//    |  /           | 
//    ' /   __| _` | __|  _ \   __| 
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/ 
//                   Multi-Physics  
//
//  License:		 BSD License 
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//


// System includes

// External includes


// Project includes
#include "includes/define_python.h"
#include "includes/c2c_variables.h"
#include "python/add_c2c_variables_to_python.h"




namespace Kratos
{

namespace Python
{
using namespace pybind11;

void  AddC2CVariablesToPython(pybind11::module& m)
{
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  SOLID_TEMPERATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  FLUID_TEMPERATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AVERAGE_TEMPERATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  INLET_TEMPERATURE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_AVERAGE_TEMPERATURE)

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,   LAST_AIR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,   PRESSURES)

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_DENSITY)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_SPECIFIC_HEAT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_THICKNESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_SFACT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_VFACT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_CONDUCTIVITY)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_HTC_ENVIRONMENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_TEMPERATURE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,  MOLD_INNER_TEMPERATURE)

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HTC)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOLIDFRACTION )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOLIDFRACTION_RATE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOLIDIF_TIME  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOLIDIF_MODULUS  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FILLTIME)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MACRO_POROSITY  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHRINKAGE_POROSITY  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAX_VEL  )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRONT_MEETING)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LIQUID_TIME)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLOW_LENGTH)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLOW_LENGTH2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COOLING_RATE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LIQUID_TO_SOLID_TIME)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TIME_CRT)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,SINKMARK)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,COLD_SHUTS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,NIYAMA)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,TEMPERATURE_GRADIENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,INITIAL_TEMPERATURE)
}
}  // namespace Python.
} // Namespace Kratos
