from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import KratosMultiphysics
import KratosMultiphysics.EmpireApplication

# Importing the base class
from co_simulation_steady_analysis import CoSimulationSteadyAnalysis
import json

parameter_file_name = "project_paramters_cosim_naca0012_small_fsi.json"

with open(parameter_file_name, 'r') as parameter_file:
    cosim_parameters = json.load(parameter_file)

simulation = CoSimulationSteadyAnalysis(cosim_parameters)
simulation.Run()