//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    @{KRATOS_APP_AUTHOR}
//

#include "rans_constitutive_laws_application_variables.h"

namespace Kratos
{
KRATOS_CREATE_VARIABLE( double, TURBULENT_KINETIC_ENERGY )
KRATOS_CREATE_VARIABLE( double, TURBULENT_ENERGY_DISSIPATION_RATE )
KRATOS_CREATE_VARIABLE( double, TURBULENT_KINETIC_ENERGY_RATE )
KRATOS_CREATE_VARIABLE( double, TURBULENT_ENERGY_DISSIPATION_RATE_2 )
KRATOS_CREATE_VARIABLE( int, RANS_MODELLING_PROCESS_STEP )
KRATOS_CREATE_VARIABLE( int, RANS_TIME_STEP )
KRATOS_CREATE_VARIABLE( double, RESIDUAL )
KRATOS_CREATE_VARIABLE( double, TANGENTIAL_VELOCITY )
KRATOS_CREATE_VARIABLE( double, NORMAL_VELOCITY )
KRATOS_CREATE_VARIABLE( double, RANS_Y_PLUS )
KRATOS_CREATE_VARIABLE( double, RANS_AUXILIARY_VARIABLE_1 )
KRATOS_CREATE_VARIABLE( double, RANS_AUXILIARY_VARIABLE_2 )

// Turbulence model constants
KRATOS_CREATE_VARIABLE( double, WALL_SMOOTHNESS_BETA )
KRATOS_CREATE_VARIABLE( double, WALL_VON_KARMAN )
KRATOS_CREATE_VARIABLE( double, TURBULENCE_RANS_C_MU )
KRATOS_CREATE_VARIABLE( double, TURBULENCE_RANS_C1 )
KRATOS_CREATE_VARIABLE( double, TURBULENCE_RANS_C2 )
KRATOS_CREATE_VARIABLE( double, TURBULENT_MIXING_LENGTH )
KRATOS_CREATE_VARIABLE( double, TURBULENT_VISCOSITY_MIN )
KRATOS_CREATE_VARIABLE( double, TURBULENT_VISCOSITY_MAX )
KRATOS_CREATE_VARIABLE( double, TURBULENT_KINETIC_ENERGY_SIGMA )
KRATOS_CREATE_VARIABLE( double, TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA )
KRATOS_CREATE_VARIABLE( Element::WeakPointer, PARENT_ELEMENT )
KRATOS_CREATE_VARIABLE( std::vector<int>, GAUSS_POINT_INDICES)

}
