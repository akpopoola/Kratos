// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:             BSD License
//                                       license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

#if !defined(KRATOS_BASE_MORTAR_CRITERIA_H)
#define  KRATOS_BASE_MORTAR_CRITERIA_H

/* System includes */

/* External includes */

/* Project includes */
#include "contact_structural_mechanics_application_variables.h"
#include "custom_utilities/contact_utilities.h"
#include "utilities/mortar_utilities.h"
#include "utilities/variable_utils.h"
#include "custom_processes/aalm_adapt_penalty_value_process.h"
#include "custom_processes/compute_dynamic_factor_process.h"
#include "solving_strategies/convergencecriterias/convergence_criteria.h"

// DEBUG
#include "input_output/vtk_output.h"

namespace Kratos
{
///@addtogroup ContactStructuralMechanicsApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @class BaseMortarConvergenceCriteria
 * @ingroup ContactStructuralMechanicsApplication
 * @brief Custom convergence criteria for the mortar condition
 * @author Vicente Mataix Ferrandiz
 */
template<class TSparseSpace, class TDenseSpace>
class BaseMortarConvergenceCriteria
    : public  ConvergenceCriteria< TSparseSpace, TDenseSpace >
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of BaseMortarConvergenceCriteria
    KRATOS_CLASS_POINTER_DEFINITION( BaseMortarConvergenceCriteria );

    /// Local Flags
    KRATOS_DEFINE_LOCAL_FLAG( COMPUTE_DYNAMIC_FACTOR );
    KRATOS_DEFINE_LOCAL_FLAG( IO_DEBUG );

    /// The base class definition (and it subclasses)
    typedef ConvergenceCriteria< TSparseSpace, TDenseSpace > BaseType;
    typedef typename BaseType::TDataType                    TDataType;
    typedef typename BaseType::DofsArrayType            DofsArrayType;
    typedef typename BaseType::TSystemMatrixType    TSystemMatrixType;
    typedef typename BaseType::TSystemVectorType    TSystemVectorType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructors
    explicit BaseMortarConvergenceCriteria(
        const bool ComputeDynamicFactor = false,
        const bool IODebug = false
        )
        : ConvergenceCriteria< TSparseSpace, TDenseSpace >(),
          mpIO(nullptr)
    {
        // Set local flags
        mOptions.Set(BaseMortarConvergenceCriteria::COMPUTE_DYNAMIC_FACTOR, ComputeDynamicFactor);
        mOptions.Set(BaseMortarConvergenceCriteria::IO_DEBUG, IODebug);
    }

    ///Copy constructor
    BaseMortarConvergenceCriteria( BaseMortarConvergenceCriteria const& rOther )
      :BaseType(rOther),
       mOptions(rOther.mOptions),
       mpIO(rOther.mpIO)
    {
    }

    /// Destructor
    ~BaseMortarConvergenceCriteria() override = default;

    ///@}
    ///@name Operators
    ///@{

    /**
     * @brief Criterias that need to be called before getting the solution
     * @param rModelPart Reference to the ModelPart containing the contact problem.
     * @param rDofSet Reference to the container of the problem's degrees of freedom (stored by the BuilderAndSolver)
     * @param rA System matrix (unused)
     * @param rDx Vector of results (variations on nodal variables)
     * @param rb RHS vector (residual)
     * @return true if convergence is achieved, false otherwise
     */
    bool PreCriteria(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        const TSystemMatrixType& rA,
        const TSystemVectorType& rDx,
        const TSystemVectorType& rb
        ) override
    {
        // The current process info
        ProcessInfo& r_process_info = rModelPart.GetProcessInfo();

        // The contact model part
        ModelPart& r_contact_model_part = rModelPart.GetSubModelPart("Contact");

        // We update the normals if necessary
        const auto normal_variation = r_process_info.Has(CONSIDER_NORMAL_VARIATION) ? static_cast<NormalDerivativesComputation>(r_process_info.GetValue(CONSIDER_NORMAL_VARIATION)) : NO_DERIVATIVES_COMPUTATION;
        if (normal_variation != NO_DERIVATIVES_COMPUTATION)
            ComputeNodesMeanNormalModelPartWithPairedNormal(rModelPart); // Update normal of the conditions

        const bool adapt_penalty = r_process_info.Has(ADAPT_PENALTY) ? r_process_info.GetValue(ADAPT_PENALTY) : false;
        const bool dynamic_case = rModelPart.HasNodalSolutionStepVariable(VELOCITY);

        /* Compute weighthed gap */
        if (adapt_penalty || dynamic_case) {
            // Set to zero the weighted gap
            ResetWeightedGap(rModelPart);

            // Compute the contribution
            ContactUtilities::ComputeExplicitContributionConditions(rModelPart.GetSubModelPart("ComputingContact"));
        }

        // In dynamic case
        if ( dynamic_case && mOptions.Is(BaseMortarConvergenceCriteria::COMPUTE_DYNAMIC_FACTOR)) {
            ComputeDynamicFactorProcess compute_dynamic_factor_process( r_contact_model_part );
            compute_dynamic_factor_process.Execute();
        }

        // We recalculate the penalty parameter
        if ( adapt_penalty ) {
            AALMAdaptPenaltyValueProcess aalm_adaptation_of_penalty( r_contact_model_part );
            aalm_adaptation_of_penalty.Execute();
        }

        return true;
    }

    /**
     * @brief Compute relative and absolute error.
     * @param rModelPart Reference to the ModelPart containing the contact problem.
     * @param rDofSet Reference to the container of the problem's degrees of freedom (stored by the BuilderAndSolver)
     * @param rA System matrix (unused)
     * @param rDx Vector of results (variations on nodal variables)
     * @param rb RHS vector (residual)
     * @return true if convergence is achieved, false otherwise
     */
    bool PostCriteria(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        const TSystemMatrixType& rA,
        const TSystemVectorType& rDx,
        const TSystemVectorType& rb
        ) override
    {
        // We save the current WEIGHTED_GAP in the buffer
        auto& r_nodes_array = rModelPart.GetSubModelPart("Contact").Nodes();
        const auto it_node_begin = r_nodes_array.begin();

        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(r_nodes_array.size()); ++i) {
            auto it_node = it_node_begin + i;
            it_node->FastGetSolutionStepValue(WEIGHTED_GAP, 1) = it_node->FastGetSolutionStepValue(WEIGHTED_GAP);
        }

        // Set to zero the weighted gap
        ResetWeightedGap(rModelPart);

        // Compute the contribution
        ContactUtilities::ComputeExplicitContributionConditions(rModelPart.GetSubModelPart("ComputingContact"));

        return true;
    }

    /**
     * @brief This function initialize the convergence criteria
     * @param rModelPart The model part of interest
     */
    void Initialize(ModelPart& rModelPart) override
    {
        KRATOS_ERROR << "YOUR ARE CALLING THE BASE MORTAR CRITERIA" << std::endl;
    }

    /**
     * @brief This function initializes the solution step
     * @param rModelPart Reference to the ModelPart containing the contact problem.
     * @param rDofSet Reference to the container of the problem's degrees of freedom (stored by the BuilderAndSolver)
     * @param rA System matrix (unused)
     * @param rDx Vector of results (variations on nodal variables)
     * @param rb RHS vector (residual)
     */
    void InitializeSolutionStep(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        const TSystemMatrixType& rA,
        const TSystemVectorType& rDx,
        const TSystemVectorType& rb
        ) override
    {
        // Update normal of the conditions
        MortarUtilities::ComputeNodesMeanNormalModelPart(rModelPart.GetSubModelPart("Contact"));

        // IO for debugging
        if (mOptions.Is(BaseMortarConvergenceCriteria::IO_DEBUG)) {
            const auto& r_nodes_array = rModelPart.Nodes();
            const bool frictional_problem = rModelPart.IsDefined(SLIP) ? rModelPart.Is(SLIP) : false;

            Parameters debug_io_parameters = Parameters(R"(
            {
                "model_part_name"                    : "PLEASE_SPECIFY_MODEL_PART_NAME",
                "file_format"                        : "binary",
                "output_precision"                   : 7,
                "output_control_type"                : "nl_iteration_number",
                "output_sub_model_parts"             : false,
                "folder_name"                        : "VTK_Output",
                "custom_name_prefix"                 : "",
                "save_output_files_in_folder"        : true,
                "nodal_solution_step_data_variables" : ["DISPLACEMENT","NORMAL","WEIGHTED_GAP"],
                "nodal_data_value_variables"         : ["DYNAMIC_FACTOR","AUGMENTED_NORMAL_CONTACT_PRESSURE"],
                "nodal_flags"                        : ["INTERFACE", "ACTIVE", "SLAVE", "ISOLATED"],
                "element_data_value_variables"       : [],
                "element_flags"                      : [],
                "condition_data_value_variables"     : [],
                "condition_flags"                    : [],
                "gauss_point_variables"              : []
            })" );

            debug_io_parameters["model_part_name"].SetString(rModelPart.Name());
            debug_io_parameters["folder_name"].SetString("VTK_Output/STEP_" + std::to_string(rModelPart.GetProcessInfo()[STEP]));
            if (rModelPart.Nodes().begin()->SolutionStepsDataHas(VELOCITY_X)) {
                debug_io_parameters["nodal_solution_step_data_variables"].Append("VELOCITY");
                debug_io_parameters["nodal_solution_step_data_variables"].Append("ACCELERATION");
            }
            if (r_nodes_array.begin()->SolutionStepsDataHas(LAGRANGE_MULTIPLIER_CONTACT_PRESSURE))
                debug_io_parameters["nodal_solution_step_data_variables"].Append("LAGRANGE_MULTIPLIER_CONTACT_PRESSURE");
            else if (r_nodes_array.begin()->SolutionStepsDataHas(VECTOR_LAGRANGE_MULTIPLIER_X))
                debug_io_parameters["nodal_solution_step_data_variables"].Append("VECTOR_LAGRANGE_MULTIPLIER");
            if (frictional_problem) {
                debug_io_parameters["nodal_flags"].Append("SLIP");
                debug_io_parameters["nodal_solution_step_data_variables"].Append("WEIGHTED_SLIP");
                debug_io_parameters["nodal_data_value_variables"].Append("AUGMENTED_TANGENT_CONTACT_PRESSURE");
            }

            mpIO = Kratos::make_shared<VtkOutput>(rModelPart, debug_io_parameters);
        }
    }

    /**
     * @brief This function finalizes the non-linear iteration
     * @param rModelPart Reference to the ModelPart containing the contact problem.
     * @param rDofSet Reference to the container of the problem's degrees of freedom (stored by the BuilderAndSolver)
     * @param rA System matrix (unused)
     * @param rDx Vector of results (variations on nodal variables)
     * @param rb RHS vector (residual)
     */
    void FinalizeNonLinearIteration(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        const TSystemMatrixType& rA,
        const TSystemVectorType& rDx,
        const TSystemVectorType& rb
        ) override
    {
        // IO for debugging
        if (mOptions.Is(BaseMortarConvergenceCriteria::IO_DEBUG)) {
            mpIO->PrintOutput();
        }
    }

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Acces
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Friends
    ///@{

protected:

    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    Flags mOptions; /// Local flags

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    /**
     * @brief This method resets the weighted gap in the nodes of the problem
     * @param rModelPart Reference to the ModelPart containing the contact problem.
     */
    virtual void ResetWeightedGap(ModelPart& rModelPart)
    {
        auto& r_nodes_array = rModelPart.GetSubModelPart("Contact").Nodes();
        VariableUtils().SetScalarVar<Variable<double>>(WEIGHTED_GAP, 0.0, r_nodes_array);
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{
    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    VtkOutput::Pointer mpIO; /// The pointer to the debugging IO

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief It computes the mean of the normal in the condition in all the nodes
     * @param rModelPart The model part to compute
     */
    static inline void ComputeNodesMeanNormalModelPartWithPairedNormal(ModelPart& rModelPart) {
        MortarUtilities::ComputeNodesMeanNormalModelPart(rModelPart.GetSubModelPart("Contact"));

        // Iterate over the computing conditions
        auto& r_conditions_array = rModelPart.GetSubModelPart("ComputingContact").Conditions();
        const auto it_cond_begin = r_conditions_array.begin();

        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(r_conditions_array.size()); ++i) {
            auto it_cond = it_cond_begin + i;

            // Aux coordinates
            Point::CoordinatesArrayType aux_coords;

            // We update the paired normal
            GeometryType& this_geometry = it_cond->GetGeometry();
            aux_coords = this_geometry.PointLocalCoordinates(aux_coords, this_geometry.Center());
            it_cond->SetValue(NORMAL, this_geometry.UnitNormal(aux_coords));

            // We update the paired normal
            GeometryType::Pointer p_paired_geometry = it_cond->GetValue(PAIRED_GEOMETRY);
            aux_coords = p_paired_geometry->PointLocalCoordinates(aux_coords, p_paired_geometry->Center());
            it_cond->SetValue(PAIRED_NORMAL, p_paired_geometry->UnitNormal(aux_coords));
        }
    }

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    ///@name Private Inquiry
    ///@{
    ///@}

    ///@name Unaccessible methods
    ///@{
    ///@}

}; // Class BaseMortarConvergenceCriteria

///@name Local flags creation
///@{

/// Local Flags
template<class TSparseSpace, class TDenseSpace>
const Kratos::Flags BaseMortarConvergenceCriteria<TSparseSpace, TDenseSpace>::COMPUTE_DYNAMIC_FACTOR(Kratos::Flags::Create(0));
template<class TSparseSpace, class TDenseSpace>
const Kratos::Flags BaseMortarConvergenceCriteria<TSparseSpace, TDenseSpace>::NOT_COMPUTE_DYNAMIC_FACTOR(Kratos::Flags::Create(0, false));
template<class TSparseSpace, class TDenseSpace>
const Kratos::Flags BaseMortarConvergenceCriteria<TSparseSpace, TDenseSpace>::IO_DEBUG(Kratos::Flags::Create(1));
template<class TSparseSpace, class TDenseSpace>
const Kratos::Flags BaseMortarConvergenceCriteria<TSparseSpace, TDenseSpace>::NOT_IO_DEBUG(Kratos::Flags::Create(1, false));

}  // namespace Kratos

#endif /* KRATOS_BASE_MORTAR_CRITERIA_H  defined */

