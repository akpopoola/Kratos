//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

// System includes

// External includes

// Include Base h
#include "rans_evm_epsilon_element.h"

namespace Kratos
{
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
 * Constructor.
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::RansEvmEpsilonElement(IndexType NewId)
    : StabilizedConvectionDiffusionReactionElement<TDim, TNumNodes, RansEvmEpsilonElementData>(NewId)
{
}

/**
 * Constructor using an array of nodes
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::RansEvmEpsilonElement(IndexType NewId,
                                                              const NodesArrayType& ThisNodes)
    : StabilizedConvectionDiffusionReactionElement<TDim, TNumNodes, RansEvmEpsilonElementData>(
          NewId, ThisNodes)
{
}

/**
 * Constructor using Geometry
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::RansEvmEpsilonElement(IndexType NewId,
                                                              GeometryType::Pointer pGeometry)
    : StabilizedConvectionDiffusionReactionElement<TDim, TNumNodes, RansEvmEpsilonElementData>(
          NewId, pGeometry)
{
}

/**
 * Constructor using Properties
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::RansEvmEpsilonElement(IndexType NewId,
                                                              GeometryType::Pointer pGeometry,
                                                              PropertiesType::Pointer pProperties)
    : StabilizedConvectionDiffusionReactionElement<TDim, TNumNodes, RansEvmEpsilonElementData>(
          NewId, pGeometry, pProperties)
{
}

/**
 * Copy Constructor
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::RansEvmEpsilonElement(
    RansEvmEpsilonElement<TDim, TNumNodes> const& rOther)
    : StabilizedConvectionDiffusionReactionElement<TDim, TNumNodes, RansEvmEpsilonElementData>(rOther)
{
}

/**
 * Destructor
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmEpsilonElement<TDim, TNumNodes>::~RansEvmEpsilonElement()
{
}

///@}
///@name Operators
///@{

///@}
///@name Operations
///@{

/**
 * ELEMENTS inherited from this class have to implement next
 * Create and Clone methods: MANDATORY
 */

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmEpsilonElement<TDim, TNumNodes>::Create(
    IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmEpsilonElement>(
        NewId, Element::GetGeometry().Create(ThisNodes), pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param pGeom: the geometry to be employed
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmEpsilonElement<TDim, TNumNodes>::Create(
    IndexType NewId, GeometryType::Pointer pGeom, PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmEpsilonElement>(NewId, pGeom, pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer and clones the previous element data
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmEpsilonElement<TDim, TNumNodes>::Clone(IndexType NewId,
                                                               NodesArrayType const& ThisNodes) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmEpsilonElement>(
        NewId, Element::GetGeometry().Create(ThisNodes), Element::pGetProperties());
    KRATOS_CATCH("");
}

/**
 * this determines the elemental equation ID vector for all elemental
 * DOFs
 * @param rResult: the elemental equation ID vector
 * @param rCurrentProcessInfo: the current process info instance
 */
template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::EquationIdVector(EquationIdVectorType& rResult,
                                                              ProcessInfo& CurrentProcessInfo)
{
    if (rResult.size() != TNumNodes)
        rResult.resize(TNumNodes, false);

    for (unsigned int i = 0; i < TNumNodes; i++)
        rResult[i] =
            Element::GetGeometry()[i].GetDof(TURBULENT_ENERGY_DISSIPATION_RATE).EquationId();
}

/**
 * determines the elemental list of DOFs
 * @param ElementalDofList: the list of DOFs
 * @param rCurrentProcessInfo: the current process info instance
 */
template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::GetDofList(DofsVectorType& rElementalDofList,
                                                        ProcessInfo& rCurrentProcessInfo)
{
    if (rElementalDofList.size() != TNumNodes)
        rElementalDofList.resize(TNumNodes);

    for (unsigned int i = 0; i < TNumNodes; i++)
        rElementalDofList[i] =
            Element::GetGeometry()[i].pGetDof(TURBULENT_ENERGY_DISSIPATION_RATE);
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::GetValuesVector(VectorType& rValues, int Step)
{
    this->GetFirstDerivativesVector(rValues, Step);
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::GetFirstDerivativesVector(VectorType& rValues, int Step)
{
    if (rValues.size() != TNumNodes)
        rValues.resize(TNumNodes, false);

    GeometryType& rGeom = this->GetGeometry();
    IndexType LocalIndex = 0;
    for (IndexType iNode = 0; iNode < TNumNodes; ++iNode)
    {
        rValues[LocalIndex++] = rGeom[iNode].FastGetSolutionStepValue(
            TURBULENT_ENERGY_DISSIPATION_RATE, Step);
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::GetSecondDerivativesVector(VectorType& rValues,
                                                                        int Step)
{
    if (rValues.size() != TNumNodes)
        rValues.resize(TNumNodes, false);

    GeometryType& rGeom = this->GetGeometry();
    IndexType LocalIndex = 0;
    for (IndexType iNode = 0; iNode < TNumNodes; ++iNode)
    {
        rValues[LocalIndex++] = rGeom[iNode].FastGetSolutionStepValue(
            TURBULENT_ENERGY_DISSIPATION_RATE_2, Step);
    }
}
template <unsigned int TDim, unsigned int TNumNodes>
GeometryData::IntegrationMethod RansEvmEpsilonElement<TDim, TNumNodes>::GetIntegrationMethod() const
{
    return GeometryData::GI_GAUSS_2;
}

/**
 * This method provides the place to perform checks on the completeness of the
 * input and the compatibility with the problem options as well as the
 * contitutive laws selected It is designed to be called only once (or anyway,
 * not often) typically at the beginning of the calculations, so to verify that
 * nothing is missing from the input or that no common error is found.
 * @param rCurrentProcessInfo
 * this method is: MANDATORY
 */
template <unsigned int TDim, unsigned int TNumNodes>
int RansEvmEpsilonElement<TDim, TNumNodes>::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    BaseType::Check(rCurrentProcessInfo);

    for (IndexType iNode = 0; iNode < this->GetGeometry().size(); ++iNode)
    {
        NodeType& r_node = this->GetGeometry()[iNode];

        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE_2, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_AUXILIARY_VARIABLE_2, r_node);

        KRATOS_CHECK_DOF_IN_NODE(TURBULENT_ENERGY_DISSIPATION_RATE, r_node);
    }

    return 0;

    KRATOS_CATCH("");
}

///@}
///@name Access
///@{

///@}
///@name Inquiry
///@{

///@}
///@name Input and output
///@{

/// Turn back information as a string.

template <unsigned int TDim, unsigned int TNumNodes>
std::string RansEvmEpsilonElement<TDim, TNumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "RansEvmEpsilonElement #" << Element::Id();
    return buffer.str();
}

/// Print information about this object.

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "RansEvmEpsilonElement #" << Element::Id();
}

/// Print object's data.

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::PrintData(std::ostream& rOStream) const
{
    Element::pGetGeometry()->PrintData(rOStream);
}

///@}
///@name Friends
///@{

///@}

///@name Protected static Member Variables
///@{

///@}
///@name Protected member Variables
///@{

///@}
///@name Protected Operators
///@{

///@}
///@name Protected Operations
///@{

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

///@name Static Member Variables
///@{

///@}
///@name Member Variables
///@{

///@}
///@name Private Operators
///@{

///@}
///@name Private Operations
///@{

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::CalculateConvectionDiffusionReactionData(
    RansEvmEpsilonElementData& rData,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const ProcessInfo& rCurrentProcessInfo,
    const int Step) const
{
    const double c1 = rCurrentProcessInfo[TURBULENCE_RANS_C1];
    const double c2 = rCurrentProcessInfo[TURBULENCE_RANS_C2];
    const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

    const double nu = this->EvaluateInPoint(KINEMATIC_VISCOSITY, rShapeFunctions);
    const double nu_t = this->EvaluateInPoint(TURBULENT_VISCOSITY, rShapeFunctions);
    const double tke = this->EvaluateInPoint(TURBULENT_KINETIC_ENERGY, rShapeFunctions);
    const double gamma = EvmKepsilonModelUtilities::CalculateGamma(c_mu, 1.0, tke, nu_t);

    rData.C1 = c1;
    rData.C2 = c2;
    rData.Gamma = gamma;
    rData.ShapeFunctionDerivatives = rShapeFunctionDerivatives;
    rData.TurbulentKinematicViscosity = nu_t;
    rData.KinematicViscosity = nu;
    rData.TurbulentKineticEnergy = tke;
    rData.VelocityDivergence =
        this->GetDivergenceOperator(VELOCITY, rShapeFunctionDerivatives);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmEpsilonElement<TDim, TNumNodes>::GetEffectiveKinematicViscosity(const
    RansEvmEpsilonElementData& rData,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const ProcessInfo& rCurrentProcessInfo,
    const int Step) const
{
    const double epsilon_sigma = rCurrentProcessInfo[TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA];
    return rData.KinematicViscosity + rData.TurbulentKinematicViscosity / epsilon_sigma;
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmEpsilonElement<TDim, TNumNodes>::GetScalarVariableGradientNorm(const
    RansEvmEpsilonElementData& rData,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const ProcessInfo& rCurrentProcessInfo,
    const int Step) const
{
    array_1d<double, 3> epsilon_gradient;
    this->CalculateGradient(epsilon_gradient, TURBULENT_ENERGY_DISSIPATION_RATE,
                            rShapeFunctionDerivatives);
    return norm_2(epsilon_gradient);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmEpsilonElement<TDim, TNumNodes>::GetScalarVariableRelaxedAcceleration(const
    RansEvmEpsilonElementData& rData,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const ProcessInfo& rCurrentProcessInfo,
    const int Step) const
{
    return this->EvaluateInPoint(RANS_AUXILIARY_VARIABLE_2, rShapeFunctions);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmEpsilonElement<TDim, TNumNodes>::CalculateReactionTerm(
    const RansEvmEpsilonElementData& rData, const ProcessInfo& rCurrentProcessInfo, const int Step) const
{
    return std::max(
        rData.C2 * rData.Gamma + rData.C1 * 2.0 * rData.VelocityDivergence / 3.0, 0.0);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmEpsilonElement<TDim, TNumNodes>::CalculateSourceTerm(
    const RansEvmEpsilonElementData& rData, const ProcessInfo& rCurrentProcessInfo, const int Step) const
{
    double production = 0.0;

    BoundedMatrix<double, TDim, TDim> velocity_gradient_matrix;
    this->CalculateGradient(velocity_gradient_matrix, VELOCITY, rData.ShapeFunctionDerivatives);

    production = EvmKepsilonModelUtilities::CalculateSourceTerm<TDim>(
        velocity_gradient_matrix, rData.TurbulentKinematicViscosity, rData.TurbulentKineticEnergy);

    production *= (rData.C1 * rData.Gamma);
    return production;
}

///@}
///@name Serialization
///@{

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);

    // List
    // To be completed with the class member list
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmEpsilonElement<TDim, TNumNodes>::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);

    // List
    // To be completed with the class member list
}

///@}
///@name Private  Access
///@{

///@}
///@name Private Inquiry
///@{

///@}
///@name Un accessible methods
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function

template <unsigned int TDim, unsigned int TNumNodes>
inline std::istream& operator>>(std::istream& rIStream,
                                RansEvmEpsilonElement<TDim, TNumNodes>& rThis);

/// output stream function

template <unsigned int TDim, unsigned int TNumNodes>
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansEvmEpsilonElement<TDim, TNumNodes>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << " : " << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
}

// Class template instantiation

template class RansEvmEpsilonElement<2, 3>;
template class RansEvmEpsilonElement<3, 4>;
template class RansEvmEpsilonElement<2, 4>;
template class RansEvmEpsilonElement<3, 8>;

} // namespace Kratos.
