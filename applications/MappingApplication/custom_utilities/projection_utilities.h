//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"

#if !defined(KRATOS_PROJECTION_UTILITIES_H_INCLUDED)
#define  KRATOS_PROJECTION_UTILITIES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_utilities/mapper_utilities.h"
#include "utilities/geometrical_projection_utilities.h"

namespace Kratos
{
namespace ProjectionUtilities
{

enum class PairingIndex
{
    Volume_Inside   = -1,
    Volume_Outside  = -2,
    Surface_Inside  = -3,
    Surface_Outside = -4,
    Line_Inside     = -5,
    Line_Outside    = -6,
    Closest_Point   = -7,
    Unspecified     = -8
};

typedef std::size_t SizeType;
typedef std::size_t IndexType;

typedef Geometry<Node<3>> GeometryType;

PairingIndex ProjectOnLine(const GeometryType& rGeometry,
                           const Point& rPointToProject,
                           const double LocalCoordTol,
                           Vector& rShapeFunctionValues,
                           std::vector<int>& rEquationIds,
                           double& rProjectionDistance,
                           const bool ComputeApproximation=true);

PairingIndex ProjectOnLineHermitian(const GeometryType& rGeometry,
                                    const Point& rPointToProject,
                                    const double LocalCoordTol,
                                    Vector& rHermitianShapeFunctionValues,
                                    Vector& rHermitianShapeFunctionValuesDer,
                                    double& rProjectionDistance,
                                    Point& rProjectionOfPoint);

void HermitianShapeFunctionsValues (Vector &hermitianShapeFunctions, 
                                    Vector &hermitianShapeFunctionsDer, 
                                    const array_1d<double, 3>& rCoordinates);

PairingIndex ProjectOnSurface(const GeometryType& rGeometry,
                     const Point& rPointToProject,
                     const double LocalCoordTol,
                     Vector& rShapeFunctionValues,
                     std::vector<int>& rEquationIds,
                     double& rProjectionDistance,
                     const bool ComputeApproximation=true);

PairingIndex ProjectIntoVolume(const GeometryType& rGeometry,
                               const Point& rPointToProject,
                               const double LocalCoordTol,
                               Vector& rShapeFunctionValues,
                               std::vector<int>& rEquationIds,
                               double& rProjectionDistance,
                               const bool ComputeApproximation=true);

bool ComputeProjection(const GeometryType& rGeometry,
                       const Point& rPointToProject,
                       const double LocalCoordTol,
                       Vector& rShapeFunctionValues,
                       std::vector<int>& rEquationIds,
                       double& rProjectionDistance,
                       PairingIndex& rPairingIndex,
                       const bool ComputeApproximation=true);

}  // namespace ProjectionUtilities.

}  // namespace Kratos.

#endif // KRATOS_PROJECTION_UTILITIES_H_INCLUDED  defined
