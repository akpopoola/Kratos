# ==============================================================================
#  KratosShapeOptimizationApplication
#
#  License:         BSD License
#                   license: ShapeOptimizationApplication/license.txt
#
#  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
#                   Geiser Armin, https://github.com/armingeiser
#
# ==============================================================================

# Making KratosMultiphysics backward compatible with python 2.6 and 2.7
from __future__ import print_function, absolute_import, division

# Kratos Core and Apps
from KratosMultiphysics import *
from KratosMultiphysics.ShapeOptimizationApplication import *

# Additional imports
from algorithm_base import OptimizationAlgorithm
import mapper_factory
import data_logger_factory
from custom_timer import Timer
from custom_variable_utilities import WriteDictionaryDataOnNodalVariable

# ==============================================================================
class AlgorithmPenalizedProjection(OptimizationAlgorithm) :
    # --------------------------------------------------------------------------
    def __init__(self, OptimizationSettings, Analyzer, Communicator, ModelPartController):
        default_algorithm_settings = Parameters("""
        {
            "name"                    : "penalized_projection",
            "correction_scaling"      : 1.0,
            "use_adaptive_correction" : true,
            "max_iterations"          : 100,
            "relative_tolerance"      : 1e-3,
            "line_search" : {
                "line_search_type"           : "manual_stepping",
                "normalize_search_direction" : true,
                "step_size"                  : 1.0
            }
        }""")
        self.algorithm_settings =  OptimizationSettings["optimization_algorithm"]
        self.algorithm_settings.RecursivelyValidateAndAssignDefaults(default_algorithm_settings)

        self.Analyzer = Analyzer
        self.Communicator = Communicator
        self.ModelPartController = ModelPartController

        self.objectives, self.equality_constraints, self.inequality_constraints = Communicator.GetInfoAboutResponses()

        self.OptimizationModelPart = ModelPartController.GetOptimizationModelPart()
        self.DesignSurface = ModelPartController.GetDesignSurface()

        self.Mapper = mapper_factory.CreateMapper(self.DesignSurface, OptimizationSettings["design_variables"]["filter"])
        self.DataLogger = data_logger_factory.CreateDataLogger(ModelPartController, Communicator, OptimizationSettings)

        self.GeometryUtilities = GeometryUtilities(self.DesignSurface)
        self.OptimizationUtilities = OptimizationUtilities(self.DesignSurface, OptimizationSettings)

        self.isDampingSpecified = OptimizationSettings["design_variables"]["damping"]["perform_damping"].GetBool()
        if self.isDampingSpecified:
            damping_regions = self.ModelPartController.GetDampingRegions()
            self.DampingUtilities = DampingUtilities(self.DesignSurface, damping_regions, OptimizationSettings)

    # --------------------------------------------------------------------------
    def CheckApplicability(self):
        if len(self.objectives) > 1:
            raise RuntimeError("Penalized projection algorithm only supports one objective function!")
        if len(self.equality_constraints)+len(self.inequality_constraints) == 0:
            raise RuntimeError("Penalized projection algorithm requires definition of a constraint!")
        if len(self.equality_constraints)+len(self.inequality_constraints) > 1:
            raise RuntimeError("Penalized projection algorithm only supports one constraint!")
    # --------------------------------------------------------------------------
    def InitializeOptimizationLoop(self):
        self.only_obj = self.objectives[0]
        self.only_con = self.equality_constraints[0] if len(self.equality_constraints)>0 else self.inequality_constraints[0]

        self.maxIterations = self.algorithm_settings["max_iterations"].GetInt() + 1
        self.relativeTolerance = self.algorithm_settings["relative_tolerance"].GetDouble()

        self.ModelPartController.InitializeMeshController()
        self.Mapper.InitializeMapping()
        self.Analyzer.InitializeBeforeOptimizationLoop()
        self.DataLogger.InitializeDataLogging()

    # --------------------------------------------------------------------------
    def RunOptimizationLoop(self):
        timer = Timer()
        timer.StartTimer()

        for self.optimizationIteration in range(1,self.maxIterations):
            print("\n>===================================================================")
            print("> ",timer.GetTimeStamp(),": Starting optimization iteration ", self.optimizationIteration)
            print(">===================================================================\n")

            timer.StartNewLap()

            self.__initializeNewShape()

            self.__analyzeShape()

            self.__computeShapeUpdate()

            self.__logCurrentOptimizationStep()

            print("\n> Time needed for current optimization step = ", timer.GetLapTime(), "s")
            print("> Time needed for total optimization so far = ", timer.GetTotalTime(), "s")

            if self.__isAlgorithmConverged():
                break
            else:
                self.__determineAbsoluteChanges()

    # --------------------------------------------------------------------------
    def FinalizeOptimizationLoop(self):
        self.DataLogger.FinalizeDataLogging()
        self.Analyzer.FinalizeAfterOptimizationLoop()

    # --------------------------------------------------------------------------
    def __initializeNewShape(self):
        self.ModelPartController.UpdateMeshAccordingInputVariable(SHAPE_UPDATE)
        self.ModelPartController.SetReferenceMeshToMesh()

    # --------------------------------------------------------------------------
    def __analyzeShape(self):
        self.Communicator.initializeCommunication()
        self.Communicator.requestValueOf(self.only_obj["identifier"].GetString())
        self.Communicator.requestGradientOf(self.only_obj["identifier"].GetString())
        self.Communicator.requestValueOf(self.only_con["identifier"].GetString())
        self.Communicator.requestGradientOf(self.only_con["identifier"].GetString())

        self.Analyzer.AnalyzeDesignAndReportToCommunicator(self.DesignSurface, self.optimizationIteration, self.Communicator)

        objGradientDict = self.Communicator.getStandardizedGradient(self.only_obj["identifier"].GetString())
        conGradientDict = self.Communicator.getStandardizedGradient(self.only_con["identifier"].GetString())

        WriteDictionaryDataOnNodalVariable(objGradientDict, self.OptimizationModelPart, DF1DX)
        WriteDictionaryDataOnNodalVariable(conGradientDict, self.OptimizationModelPart, DC1DX)

        if self.only_obj["project_gradient_on_surface_normals"].GetBool():
            self.GeometryUtilities.ComputeUnitSurfaceNormals()
            self.GeometryUtilities.ProjectNodalVariableOnUnitSurfaceNormals(DF1DX)

        if self.only_con["project_gradient_on_surface_normals"].GetBool():
            self.GeometryUtilities.ComputeUnitSurfaceNormals()
            self.GeometryUtilities.ProjectNodalVariableOnUnitSurfaceNormals(DC1DX)

        if self.isDampingSpecified:
            self.DampingUtilities.DampNodalVariable(DF1DX)
            self.DampingUtilities.DampNodalVariable(DC1DX)

        self.__ResetPossibleShapeModificationsDuringAnalysis()

    # --------------------------------------------------------------------------
    def __ResetPossibleShapeModificationsDuringAnalysis(self):
        self.ModelPartController.SetMeshToReferenceMesh()
        self.ModelPartController.SetDeformationVariablesToZero()

    # --------------------------------------------------------------------------
    def __computeShapeUpdate(self):
        self.__mapSensitivitiesToDesignSpace()
        constraint_value = self.Communicator.getStandardizedValue(self.only_con["identifier"].GetString())
        if self.__isConstraintActive(constraint_value):
            self.OptimizationUtilities.ComputeProjectedSearchDirection()
            self.OptimizationUtilities.CorrectProjectedSearchDirection(constraint_value)
        else:
            self.OptimizationUtilities.ComputeSearchDirectionSteepestDescent()
        self.OptimizationUtilities.ComputeControlPointUpdate()
        self.__mapDesignUpdateToGeometrySpace()

        if self.isDampingSpecified:
            self.DampingUtilities.DampNodalVariable(SHAPE_UPDATE)

    # --------------------------------------------------------------------------
    def __mapSensitivitiesToDesignSpace(self):
        self.Mapper.MapToDesignSpace(DF1DX, DF1DX_MAPPED)
        self.Mapper.MapToDesignSpace(DC1DX, DC1DX_MAPPED)

    # --------------------------------------------------------------------------
    def __isConstraintActive(self, constraintValue):
        if self.only_con["type"].GetString() == "=":
            return True
        elif constraintValue > 0:
            return True
        else:
            return False

    # --------------------------------------------------------------------------
    def __mapDesignUpdateToGeometrySpace(self):
        self.Mapper.MapToGeometrySpace(CONTROL_POINT_UPDATE, SHAPE_UPDATE)

    # --------------------------------------------------------------------------
    def __logCurrentOptimizationStep(self):
        self.DataLogger.LogCurrentData(self.optimizationIteration)

    # --------------------------------------------------------------------------
    def __isAlgorithmConverged(self):

        if self.optimizationIteration > 1 :

            # Check if maximum iterations were reached
            if self.optimizationIteration == self.maxIterations:
                print("\n> Maximal iterations of optimization problem reached!")
                return True

            relativeChangeOfObjectiveValue = self.DataLogger.GetValue("RELATIVE_CHANGE_OF_OBJECTIVE_VALUE")

            # Check for relative tolerance
            if abs(relativeChangeOfObjectiveValue) < self.relativeTolerance:
                print("\n> Optimization problem converged within a relative objective tolerance of ",self.relativeTolerance,"%.")
                return True

    # --------------------------------------------------------------------------
    def __determineAbsoluteChanges(self):
        self.OptimizationUtilities.AddFirstVariableToSecondVariable(CONTROL_POINT_UPDATE, CONTROL_POINT_CHANGE)
        self.OptimizationUtilities.AddFirstVariableToSecondVariable(SHAPE_UPDATE, SHAPE_CHANGE)

# ==============================================================================
