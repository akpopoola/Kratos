//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Marc Nunez, Inigo Lopez based on R.Rossi and V.Mataix work
//
//
//

/* System includes */

/* External includes */

/* Project includes */
#include "utilities/variable_utils.h"
#include "compute_nodal_value_process.h"
#include "processes/calculate_nodal_area_process.h"


namespace Kratos
{

// Default constructor
ComputeNodalValueProcess::ComputeNodalValueProcess(
    ModelPart& rModelPart,
    const std::vector<std::string>& rVariableStringArray)
    :mrModelPart(rModelPart)
{
    KRATOS_TRY

    KRATOS_ERROR_IF( rVariableStringArray.size() < 1 ) <<
    " ComputeNodalValueProcess: The variables list is empty " << std::endl;

    StoreVariableList(rVariableStringArray);

    InitializeNodalVariables();

    KRATOS_CATCH("")
}

void ComputeNodalValueProcess::Execute()
{
    KRATOS_TRY;

    // Set nodal values to zero
    ClearNodalValues();

    CalculateNodalAreaProcess<false> area_calculator(mrModelPart, mrModelPart.GetProcessInfo()[DOMAIN_SIZE]);
    area_calculator.Execute();

    for (std::size_t i_var = 0; i_var < mArrayVariablesList.size(); i_var++){
        const auto& r_array_var = *mArrayVariablesList[i_var];
        AddElementsContribution<array_1d<double, 3>,3>(r_array_var);
    }
    for (std::size_t i_var = 0; i_var < mDoubleVariablesList.size(); i_var++){
        const auto& r_double_var = *mDoubleVariablesList[i_var];
        AddElementsContribution<double, 1>(r_double_var);
    }

    PonderateNodalValues();

    KRATOS_CATCH("")
}

void ComputeNodalValueProcess::StoreVariableList(const std::vector<std::string>& rVariableStringArray)
{
    // Storing variable list
    for (std::size_t i_variable=0; i_variable < rVariableStringArray.size(); i_variable++){
        if (KratosComponents<Variable<double>>::Has(rVariableStringArray[i_variable])) {
            const auto& r_double_var  = KratosComponents<Variable<double>>::Get(rVariableStringArray[i_variable]);
            mDoubleVariablesList.push_back(&r_double_var);
        }
        else if (KratosComponents<Variable<array_1d<double,3>>>::Has(rVariableStringArray[i_variable])){
            const auto& r_array_var = KratosComponents<Variable<array_1d<double,3>>>::Get(rVariableStringArray[i_variable]);
            mArrayVariablesList.push_back(&r_array_var);
        }
        else {
            KRATOS_ERROR << "The variable defined in the list is not a double variable nor an array variable. Given variable: " << rVariableStringArray[i_variable] << std::endl;
        }
    }

}

void ComputeNodalValueProcess::InitializeNodalVariables()
{
    // In case the variables are not initialized we initialize them
    auto& r_nodes = mrModelPart.Nodes();
    for (std::size_t i_var = 0; i_var < mArrayVariablesList.size(); i_var++){
        const auto& r_array_var = *mArrayVariablesList[i_var];
        if (!r_nodes.begin()->Has(r_array_var)){
            const array_1d<double,3> zero_vector = ZeroVector(3);
            VariableUtils().SetNonHistoricalVariable(r_array_var, zero_vector, r_nodes);
        }
    }
    for (std::size_t i_var = 0; i_var < mDoubleVariablesList.size(); i_var++){
        const auto& r_double_var = *mDoubleVariablesList[i_var];
        if (!r_nodes.begin()->Has(r_double_var)){
            VariableUtils().SetNonHistoricalVariable(r_double_var, 0.0, r_nodes);
        }
    }
}

void ComputeNodalValueProcess::ClearNodalValues()
{
    const array_1d<double, 3> aux_zero_vector = ZeroVector(3);
    for (std::size_t i_var = 0; i_var < mArrayVariablesList.size(); i_var++){
        const auto& r_array_var = *mArrayVariablesList[i_var];
        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
            auto it_node=mrModelPart.NodesBegin()+i;
            it_node->SetValue(r_array_var, aux_zero_vector);
        }
    }

    for (std::size_t i_var = 0; i_var < mDoubleVariablesList.size(); i_var++){
        const auto& r_double_var = *mDoubleVariablesList[i_var];
        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i) {
            auto it_node=mrModelPart.NodesBegin()+i;
            it_node->SetValue(r_double_var, 0.0);
        }
    }
}

template< typename TValueType, std::size_t TSize >
void ComputeNodalValueProcess::AddElementsContribution(const Variable<TValueType>& rVariable){
    // Auxiliar container
    Vector N;

    const ProcessInfo& r_current_process_info = mrModelPart.GetProcessInfo();

    // Current domain size
    const std::size_t dimension = mrModelPart.GetProcessInfo()[DOMAIN_SIZE];
    KRATOS_ERROR_IF(dimension != 2 && dimension !=3) << "Dimension has to be either 2 or 3! Current dimension: " << dimension << std::endl;

    // First element iterator
    const auto it_element_begin = mrModelPart.ElementsBegin();

    // Iterate over the elements
    #pragma omp parallel for firstprivate(N)
    for(int i_elem=0; i_elem<static_cast<int>(mrModelPart.Elements().size()); ++i_elem) {
        auto it_elem = it_element_begin + i_elem;
        auto& r_geometry = it_elem->GetGeometry();

        // Current geometry information
        const std::size_t number_of_nodes = r_geometry.PointsNumber();

        // Resize if needed
        if (N.size() != number_of_nodes)
            N.resize(number_of_nodes);

        // The integration points
        const auto& r_integration_method = r_geometry.GetDefaultIntegrationMethod();
        const auto& r_integration_points = r_geometry.IntegrationPoints(r_integration_method);
        const std::size_t number_of_integration_points = r_integration_points.size();

        // The containers of the shape functions and the local gradients
        const Matrix& rNmatrix = r_geometry.ShapeFunctionsValues(r_integration_method);

        // std::vector<TValueType> element_values(number_of_integration_points);
        // it_elem->GetValueOnIntegrationPoints(rVariable, element_values, r_current_process_info);
        auto element_values = GetElementValue<TValueType, TSize>(number_of_integration_points, it_elem, rVariable, r_current_process_info);

        for ( IndexType i_gauss = 0; i_gauss < number_of_integration_points; ++i_gauss ) {
            // Getting the shape functions
            noalias(N) = row(rNmatrix, i_gauss);

            Vector detJ0;
            GeometryData::ShapeFunctionsGradientsType DN_DX;
            r_geometry.ShapeFunctionsIntegrationPointsGradients(DN_DX, detJ0, r_integration_method);

            const auto gauss_point_value = element_values[i_gauss];

            const double gauss_point_volume = r_integration_points[i_gauss].Weight() * detJ0[i_gauss];

            for(std::size_t i_node=0; i_node<number_of_nodes; ++i_node) {
                //UpdateNodalValue(r_geometry[i_node], rVariable, N[i_node], gauss_point_volume, gauss_point_value);
                auto& value = GetNodalValue<TValueType, TSize>(r_geometry[i_node], rVariable);
                for(std::size_t k=0; k<TSize; ++k) {
                    #pragma omp atomic
                    value[k] += N[i_node]* gauss_point_volume * gauss_point_value[k];
                }
            }
        }
    }
}

template< typename TValueType, std::size_t TSize >
array_1d<double, TSize>& ComputeNodalValueProcess::GetNodalValue(Element::NodeType& rNode,
    const Variable<TValueType>& rVariable)
{
    array_1d<double, TSize>& val = rNode.GetValue(rVariable);
    return val;
}

template<>
array_1d<double, 1>& ComputeNodalValueProcess::GetNodalValue(Element::NodeType& rNode,
    const Variable<double>& rVariable)
{
    double& val = rNode.GetValue(rVariable);
    array_1d<double, 1> val_array = {val};
    //array_1d<double, 1>& val_array[0] = rNode.GetValue(rVariable);
    return val_array;
}

template< typename TValueType, std::size_t TSize >
std::vector<array_1d<double, TSize>> ComputeNodalValueProcess::GetElementValue(const std::size_t& rNumberOfIntegrationPoints,
    ModelPart::ElementIterator rItElem,
    const Variable<TValueType>& rVariable,
    const ProcessInfo& rCurrentProcessInfo)
{
    std::vector<TValueType> element_values(rNumberOfIntegrationPoints);
    rItElem->GetValueOnIntegrationPoints(rVariable, element_values, rCurrentProcessInfo);

    return element_values;
}

template<>
std::vector<array_1d<double, 1>> ComputeNodalValueProcess::GetElementValue(const std::size_t& rNumberOfIntegrationPoints,
    ModelPart::ElementIterator rItElem,
    const Variable<double>& rVariable,
    const ProcessInfo& rCurrentProcessInfo)
{
    std::vector<double> element_values(rNumberOfIntegrationPoints);
    rItElem->GetValueOnIntegrationPoints(rVariable, element_values, rCurrentProcessInfo);
    std::vector<array_1d<double, 1>> element_value_array(rNumberOfIntegrationPoints);
    for ( IndexType i_gauss = 0; i_gauss < rNumberOfIntegrationPoints; ++i_gauss ) {
        element_value_array[i_gauss][0] = element_values[i_gauss];
    }

    return element_value_array;
}

void ComputeNodalValueProcess::UpdateNodalValue(Element::NodeType& rNode,
    const Variable<array_1d<double, 3>>& rVariable,
    const double& rN,
    const double& rGaussPointVolume,
    const array_1d<double, 3>& rGaussPointValue)
{
    auto& r_gradient = rNode.GetValue(rVariable);
    for(std::size_t k=0; k<r_gradient.size(); ++k) {
        #pragma omp atomic
        r_gradient[k] += rN * rGaussPointVolume * rGaussPointValue[k];
    }
}

void ComputeNodalValueProcess::UpdateNodalValue(Element::NodeType& rNode,
    const Variable<double>& rVariable,
    const double& rN,
    const double& rGaussPointVolume,
    const double& rGaussPointValue)
{
    double& value = rNode.GetValue(rVariable);
    #pragma omp atomic
    value += rN * rGaussPointVolume * rGaussPointValue;
}

void ComputeNodalValueProcess::PonderateNodalValues()
{
    for (std::size_t i_var = 0; i_var < mArrayVariablesList.size(); i_var++){
        const auto& r_array_var = *mArrayVariablesList[i_var];
        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i){
            auto it_node=mrModelPart.NodesBegin()+i;
            it_node->GetValue(r_array_var) /= it_node->GetValue(NODAL_AREA);
        }
    }
    for (std::size_t i_var = 0; i_var < mDoubleVariablesList.size(); i_var++){
        const auto& r_double_var = *mDoubleVariablesList[i_var];
        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(mrModelPart.Nodes().size()); ++i){
            auto it_node=mrModelPart.NodesBegin()+i;
            it_node->GetValue(r_double_var) /= it_node->GetValue(NODAL_AREA);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template instantiation

template void ComputeNodalValueProcess::AddElementsContribution<array_1d<double, 3>, 3>(const Variable<array_1d<double, 3>>&);
template void ComputeNodalValueProcess::AddElementsContribution<double, 1>(const Variable<double>& rVariable);

template std::vector<array_1d<double, 3>> ComputeNodalValueProcess::GetElementValue<array_1d<double, 3>, 3>(const std::size_t& rNumberOfIntegrationPoints,
    ModelPart::ElementIterator rItElem,
    const Variable<array_1d<double, 3>>& rVariable,
    const ProcessInfo& rCurrentProcessInfo);

template array_1d<double, 3>& ComputeNodalValueProcess::GetNodalValue(Element::NodeType& rNode,
    const Variable<array_1d<double, 3>>& rVariable);

// template std::vector<array_1d<double, 1>> ComputeNodalValueProcess::GetElementValue<double, 1>(const std::size_t& rNumberOfIntegrationPoints,
//     ModelPart::ElementIterator rItElem,
//     const Variable<double>& rVariable,
//     const ProcessInfo& rCurrentProcessInfo);
} /* namespace Kratos.*/
