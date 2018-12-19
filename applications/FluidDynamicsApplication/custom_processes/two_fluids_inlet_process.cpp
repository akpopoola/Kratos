//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Simon Wenczowski
//
//

// System includes

// External includes

// Project includes
#include "utilities/openmp_utils.h"
#include "utilities/body_distance_calculation_utils.h"
#include "utilities/variable_utils.h"
#include "includes/deprecated_variables.h"

// Application includes
#include "two_fluids_inlet_process.h"
#include "fluid_dynamics_application_variables.h"


namespace Kratos
{

/* Public functions *******************************************************/

TwoFluidsInletProcess::TwoFluidsInletProcess(
    ModelPart& rModelPart,
    Parameters& rParameters)
    : Process(), mrInletModelPart(rModelPart) {

    Parameters default_parameters( R"(
    {
        "modulus_air"               : 0.0,
        "modulus_water"             : 0.0,
        "interface_normal"          : [0.0,1.0,0.0],
        "point_on_interface"        : [0.0,0.25,0.0],
        "inlet_transition_radius"   : 0.05
    }  )" );

    // validating only the specific values for two-fluid application (important here)
    rParameters["two_fluid_settings"].ValidateAndAssignDefaults(default_parameters);

    // finding the complete model the inlet model part
    // const Model& rCompleteModel = mrInletModelPart.GetModel();
    ModelPart& rRootModelPart = mrInletModelPart.GetRootModelPart();

    // setting the parameters to the private data members of the class
    mModulusAir = rParameters["two_fluid_settings"]["modulus_air"].GetDouble();
    mModulusWater = rParameters["two_fluid_settings"]["modulus_water"].GetDouble();
    mInterfaceNormal = rParameters["two_fluid_settings"]["interface_normal"].GetVector();
    mInterfacePoint = rParameters["two_fluid_settings"]["point_on_interface"].GetVector();
    mInletRadius = rParameters["two_fluid_settings"]["inlet_transition_radius"].GetDouble();

    // setting flags for the inlet on nodes and conditions
    #pragma omp parallel for
    for (int i_node = 0; i_node < static_cast<int>(mrInletModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_elem = mrInletModelPart.NodesBegin() + i_node;
        it_elem->Set( INLET, true );
    }
    #pragma omp parallel for
    for (int i_cond = 0; i_cond < static_cast<int>(mrInletModelPart.NumberOfConditions()); ++i_cond){
        // iteration over all conditions
        auto it_cond = mrInletModelPart.ConditionsBegin() + i_cond;
        it_cond->Set( INLET, true );
    }

    // Comment: The historical DISTANCE variable is used to compute a distance field that is then stored in a non-historical variable AUX_DISTANCE
    //          The functions for the distance computation do not work on non-histirical variables - this is reason for the following procedure (*)

    // (*) temporally storing the distance field as an older version of itself (it can be assured that nothing is over-written at the start)
    #pragma omp parallel for
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        it_node->GetSolutionStepValue(DISTANCE, 2) = it_node->GetSolutionStepValue(DISTANCE, 0);
    }

    // Preparing the distance computation
    VariableUtils var_utils;
    var_utils.SetNonHistoricalVariable( IS_VISITED, 0.0, rRootModelPart.Nodes() );
    var_utils.SetNonHistoricalVariable( IS_VISITED, 1.0, mrInletModelPart.Nodes() );
    var_utils.SetScalarVar( DISTANCE, 0.0, rRootModelPart.Nodes() );
    var_utils.SetScalarVar( DISTANCE, -mInletRadius, mrInletModelPart.Nodes() );

    ////// TESTING >>> works
    KRATOS_WATCH( "Initial stage" )
    int visited_counter = 0;
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        if ( it_node->GetValue(IS_VISITED) > 0.5 ){ visited_counter++; }
        KRATOS_WATCH( it_node->GetValue(IS_VISITED) );
        KRATOS_WATCH( it_node->GetValue(AUX_DISTANCE) );
        KRATOS_WATCH( it_node->GetSolutionStepValue(DISTANCE, 0) );
    }
    KRATOS_WATCH( visited_counter )
    KRATOS_WATCH( mInletRadius )

    // BodyDistanceCalculationUtils dist_utils;
    const unsigned int dim = rRootModelPart.GetProcessInfo()[DOMAIN_SIZE];
    BodyDistanceCalculationUtils distance_util;
    if ( dim == 2 ){
        distance_util.CalculateDistances<2>( rRootModelPart.Elements(), DISTANCE, 0.0 );
    } else if ( dim == 3 ){
        distance_util.CalculateDistances<3>( rRootModelPart.Elements(), DISTANCE, 0.0 );
    } else {
        KRATOS_ERROR << "Error thrown in TwoFluidsInletProcess: Dimension not valid." << std::endl;
    }

    ////// TESTING
    KRATOS_WATCH( "First stage" )
    visited_counter = 0;
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        if ( it_node->GetValue(IS_VISITED) > 0.5 ){ visited_counter++; }
        KRATOS_WATCH( it_node->GetValue(IS_VISITED) );
        KRATOS_WATCH( it_node->GetValue(AUX_DISTANCE) );
        KRATOS_WATCH( it_node->GetSolutionStepValue(DISTANCE, 0) );
    }
    KRATOS_WATCH( visited_counter )
    KRATOS_WATCH( mInletRadius )

    // scaling the distance values such that 1.0 is reached at the inlet
    const double scaling_factor = - 1.0 / mInletRadius;
    #pragma omp parallel for
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        it_node->GetSolutionStepValue(DISTANCE, 0) = scaling_factor * it_node->GetSolutionStepValue(DISTANCE, 0);
    }

    // saving the value of DISTANCE to the non-historical variable AUX_DISTANCE
    var_utils.SaveScalarVar( DISTANCE, AUX_DISTANCE, rRootModelPart.Nodes() );

    // (*) restoring the original distance field from its stored version
    #pragma omp parallel for
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        it_node->GetSolutionStepValue(DISTANCE, 0) = it_node->GetSolutionStepValue(DISTANCE, 2);
    }

    ////// TESTING
    KRATOS_WATCH( "Second stage" )
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;
        KRATOS_WATCH( it_node->GetSolutionStepValue(DISTANCE, 0) );
        KRATOS_WATCH( it_node->GetValue(IS_VISITED) );
        KRATOS_WATCH( it_node->GetValue(AUX_DISTANCE) );
    }

    // subdividing the inlet into two sub_model_part
    mrInletModelPart.CreateSubModelPart("water_inlet");
    mrInletModelPart.CreateSubModelPart("air_inlet");
    ModelPart& rWaterInlet = mrInletModelPart.GetSubModelPart("water_inlet");
    ModelPart& rAirInlet = mrInletModelPart.GetSubModelPart("air_inlet");

    // classifying nodes
    std::vector<IndexType> index_node_water;
    std::vector<IndexType> index_node_air;
    for (int i_node = 0; i_node < static_cast<int>(mrInletModelPart.NumberOfNodes()); ++i_node){
        auto it_node = mrInletModelPart.NodesBegin() + i_node;
        const double inlet_dist = ComputeNodalDistanceInInletDistanceField(it_node);
        KRATOS_WATCH(inlet_dist)
        if (inlet_dist <= 0.0){
            index_node_water.push_back( it_node->GetId() );
        } else {
            index_node_air.push_back( it_node->GetId() );
        }
    }
    KRATOS_WATCH(index_node_water);
    KRATOS_WATCH(index_node_air);

    rWaterInlet.AddNodes( index_node_water );
    rAirInlet.AddNodes( index_node_air );

    // classifying conditions
    std::vector<IndexType> index_cond_water;
    std::vector<IndexType> index_cond_air;
    for (int i_cond = 0; i_cond < static_cast<int>(mrInletModelPart.NumberOfConditions()); ++i_cond){
        auto it_cond = mrInletModelPart.ConditionsBegin() + i_cond;
        unsigned int pos_counter = 0;
        unsigned int neg_counter = 0;
        for (int i_node = 0; i_node < static_cast<int>(it_cond->GetGeometry().PointsNumber()); i_node++){
            const Node<3>& rNode = (it_cond->GetGeometry())[i_node];
            const double inlet_dist = ComputeNodalDistanceInInletDistanceField( rNode );
            if ( inlet_dist > 0 ){ pos_counter++; }
            if ( inlet_dist <= 0 ){ neg_counter++; }
        }
        // the conditions cut by the interface are neither assigned to the positive nor the negative side
        if( pos_counter == it_cond->GetGeometry().PointsNumber() ){
            index_cond_air.push_back( it_cond->GetId() );
        }
        if( neg_counter == it_cond->GetGeometry().PointsNumber() ){
            index_cond_water.push_back( it_cond->GetId() );
        }
    }
    KRATOS_WATCH(index_cond_water);
    KRATOS_WATCH(index_cond_air);

    rWaterInlet.AddConditions( index_cond_water );
    rAirInlet.AddConditions( index_cond_air );
}


void TwoFluidsInletProcess::SmoothDistanceField(){

    ModelPart& rRootModelPart = mrInletModelPart.GetRootModelPart();

    #pragma omp parallel for
    for (int i_node = 0; i_node < static_cast<int>(rRootModelPart.NumberOfNodes()); ++i_node){
        // iteration over all nodes
        auto it_node = rRootModelPart.NodesBegin() + i_node;

        // check if node is inside "inlet_transition_radius"
        if ( it_node->GetValue(AUX_DISTANCE) > 1.0e-7 ){
            // finding distance value of the node in the inlet field
            const double inlet_dist = ComputeNodalDistanceInInletDistanceField(it_node);
            // finding distance value for the node in the domain distance field
            const double domain_dist = it_node->GetSolutionStepValue( DISTANCE, 0 );
            // introducing a smooth transition based in the distance from the inlet stored in "AUX_DISTANCE"
            const double weighting_factor_inlet_field = it_node->GetValue(AUX_DISTANCE);
            const double weighting_factor_domain_field = 1.0 - weighting_factor_inlet_field;

            const double smoothed_dist = weighting_factor_inlet_field * inlet_dist + weighting_factor_domain_field * domain_dist;
            it_node->GetSolutionStepValue( DISTANCE, 0 ) = smoothed_dist;
        }

    }
}

/* Private functions ****************************************************/

double TwoFluidsInletProcess::ComputeNodalDistanceInInletDistanceField( const ModelPart::NodesContainerType::iterator node ){

    const array_1d<double,3> distance = node->Coordinates() - mInterfacePoint;
    const array_1d<double,3>& normal = mInterfaceNormal;

    const double inlet_distance =   distance[0]*normal[0] +
                                    distance[1]*normal[1] +
                                    distance[2]*normal[2];
    return inlet_distance;
}

double TwoFluidsInletProcess::ComputeNodalDistanceInInletDistanceField( const Node<3>& node ){

    const array_1d<double,3> distance = node.Coordinates() - mInterfacePoint;
    const array_1d<double,3>& normal = mInterfaceNormal;

    const double inlet_distance =   distance[0]*normal[0] +
                                    distance[1]*normal[1] +
                                    distance[2]*normal[2];
    return inlet_distance;
}


};  // namespace Kratos.