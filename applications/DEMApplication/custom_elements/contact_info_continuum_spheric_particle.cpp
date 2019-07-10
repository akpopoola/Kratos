//
// Author: Joaquín Irazábal jirazabal@cimne.upc.edu
//

// System includes
#include <string>
#include <iostream>
#include <cmath>

#include <fstream>

// External includes

// Project includes
#include "contact_info_continuum_spheric_particle.h"

namespace Kratos
{

ContactInfoContinuumSphericParticle::ContactInfoContinuumSphericParticle()
    : SphericContinuumParticle() {}

ContactInfoContinuumSphericParticle::ContactInfoContinuumSphericParticle(IndexType NewId, GeometryType::Pointer pGeometry)
    : SphericContinuumParticle(NewId, pGeometry) {}

ContactInfoContinuumSphericParticle::ContactInfoContinuumSphericParticle(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties)
    : SphericContinuumParticle(NewId, pGeometry, pProperties) {}

ContactInfoContinuumSphericParticle::ContactInfoContinuumSphericParticle(IndexType NewId, NodesArrayType const& ThisNodes)
    : SphericContinuumParticle(NewId, ThisNodes) {}

ContactInfoContinuumSphericParticle::ContactInfoContinuumSphericParticle(Element::Pointer p_continuum_spheric_particle)
{
    GeometryType::Pointer p_geom = p_continuum_spheric_particle->pGetGeometry();
    PropertiesType::Pointer pProperties = p_continuum_spheric_particle->pGetProperties();
    ContactInfoContinuumSphericParticle(p_continuum_spheric_particle->Id(), p_geom, pProperties);
}

ContactInfoContinuumSphericParticle& ContactInfoContinuumSphericParticle::operator=(const ContactInfoContinuumSphericParticle& rOther) {

    SphericParticle::operator=(rOther);

    mNeighbourContactRadius = rOther.mNeighbourContactRadius;
    mNeighbourIndentation = rOther.mNeighbourIndentation;
    mNeighbourTgOfFriAng = rOther.mNeighbourTgOfFriAng;
    mNeighbourContactStress = rOther.mNeighbourContactStress;
    mNeighbourCohesion = rOther.mNeighbourCohesion;
    mNeighbourRigidContactRadius = rOther.mNeighbourRigidContactRadius;
    mNeighbourRigidIndentation = rOther.mNeighbourRigidIndentation;
    mNeighbourRigidTgOfFriAng = rOther.mNeighbourRigidTgOfFriAng;
    mNeighbourRigidContactStress = rOther.mNeighbourRigidContactStress;
    mNeighbourRigidCohesion = rOther.mNeighbourRigidCohesion;

    return *this;
}

Element::Pointer ContactInfoContinuumSphericParticle::Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
{
    GeometryType::Pointer p_geom = GetGeometry().Create(ThisNodes);

    return Element::Pointer(new ContactInfoContinuumSphericParticle(NewId, p_geom, pProperties));
}

void ContactInfoContinuumSphericParticle::ComputeNewNeighboursHistoricalData(DenseVector<int>& temp_neighbours_ids,
                                                         std::vector<array_1d<double, 3> >& temp_neighbour_elastic_contact_forces)
{
    std::vector<array_1d<double, 3> > temp_neighbour_elastic_extra_contact_forces;
    std::vector<double> temp_neighbour_contact_radius;
    std::vector<double> temp_neighbour_indentation;
    std::vector<double> temp_neighbour_tg_of_fri_ang;
    std::vector<double> temp_neighbour_contact_stress;
    std::vector<double> temp_neighbour_cohesion;
    unsigned int new_size = mNeighbourElements.size();
    array_1d<double, 3> vector_of_zeros = ZeroVector(3);
    temp_neighbours_ids.resize(new_size, false);
    temp_neighbour_elastic_contact_forces.resize(new_size);
    temp_neighbour_elastic_extra_contact_forces.resize(new_size);
    temp_neighbour_contact_radius.resize(new_size);
    temp_neighbour_indentation.resize(new_size);
    temp_neighbour_tg_of_fri_ang.resize(new_size);
    temp_neighbour_contact_stress.resize(new_size);
    temp_neighbour_cohesion.resize(new_size);

    DenseVector<int>& vector_of_ids_of_neighbours = GetValue(NEIGHBOUR_IDS);

    for (unsigned int i = 0; i < new_size; i++) {
        noalias(temp_neighbour_elastic_contact_forces[i]) = vector_of_zeros;
        noalias(temp_neighbour_elastic_extra_contact_forces[i]) = vector_of_zeros;
        temp_neighbour_contact_radius[i] = 0.0;
        temp_neighbour_indentation[i] = 0.0;
        temp_neighbour_tg_of_fri_ang[i] = 1e20;
        temp_neighbour_contact_stress[i] = 0.0;
        temp_neighbour_cohesion[i] = 0.0;

        if (mNeighbourElements[i] == NULL) { // This is required by the continuum sphere which reorders the neighbors
            temp_neighbours_ids[i] = -1;
            continue;
        }

        temp_neighbours_ids[i] = mNeighbourElements[i]->Id();

        for (unsigned int j = 0; j < vector_of_ids_of_neighbours.size(); j++) {
            if (int(temp_neighbours_ids[i]) == vector_of_ids_of_neighbours[j] && vector_of_ids_of_neighbours[j] != -1) {
                noalias(temp_neighbour_elastic_contact_forces[i]) = mNeighbourElasticContactForces[j];
                noalias(temp_neighbour_elastic_extra_contact_forces[i]) = mNeighbourElasticExtraContactForces[j]; //TODO: remove this from discontinuum!!
                temp_neighbour_contact_radius[i] = mNeighbourContactRadius[j];
                temp_neighbour_indentation[i] = mNeighbourIndentation[j];
                temp_neighbour_tg_of_fri_ang[i] = mNeighbourTgOfFriAng[j];
                temp_neighbour_contact_stress[i] = mNeighbourContactStress[j];
                temp_neighbour_cohesion[i] = mNeighbourCohesion[j];
                break;
            }
        }
    }

    vector_of_ids_of_neighbours.swap(temp_neighbours_ids);
    mNeighbourElasticContactForces.swap(temp_neighbour_elastic_contact_forces);
    mNeighbourElasticExtraContactForces.swap(temp_neighbour_elastic_extra_contact_forces);
    mNeighbourContactRadius.swap(temp_neighbour_contact_radius);
    mNeighbourIndentation.swap(temp_neighbour_indentation);
    mNeighbourTgOfFriAng.swap(temp_neighbour_tg_of_fri_ang);
    mNeighbourContactStress.swap(temp_neighbour_contact_stress);
    mNeighbourCohesion.swap(temp_neighbour_cohesion);
}

void ContactInfoContinuumSphericParticle::ComputeNewRigidFaceNeighboursHistoricalData()
{
    array_1d<double, 3> vector_of_zeros = ZeroVector(3);
    std::vector<DEMWall*>& rNeighbours = this->mNeighbourRigidFaces;
    unsigned int new_size              = rNeighbours.size();
    std::vector<int> temp_neighbours_ids(new_size); //these two temporal vectors are very small, saving them as a member of the particle loses time (usually they consist on 1 member).
    std::vector<array_1d<double, 3> > temp_neighbours_elastic_contact_forces(new_size);
    std::vector<array_1d<double, 3> > temp_neighbours_contact_forces(new_size);
    std::vector<double> temp_contact_radius(new_size);
    std::vector<double> temp_indentation(new_size);
    std::vector<double> temp_tg_of_fri_ang(new_size);
    std::vector<double> temp_contact_stress(new_size);
    std::vector<double> temp_cohesion(new_size);

    for (unsigned int i = 0; i<rNeighbours.size(); i++){

        noalias(temp_neighbours_elastic_contact_forces[i]) = vector_of_zeros;
        noalias(temp_neighbours_contact_forces[i]) = vector_of_zeros;
        temp_contact_radius[i] = 0.0;
        temp_indentation[i] = 0.0;
        temp_tg_of_fri_ang[i] = 1e20;
        temp_contact_stress[i] = 0.0;
        temp_cohesion[i] = 0.0;

        if (rNeighbours[i] == NULL) { // This is required by the continuum sphere which reorders the neighbors
            temp_neighbours_ids[i] = -1;
            continue;
        }

        temp_neighbours_ids[i] = static_cast<int>(rNeighbours[i]->Id());

        for (unsigned int j = 0; j != mFemOldNeighbourIds.size(); j++) {
            if (static_cast<int>(temp_neighbours_ids[i]) == mFemOldNeighbourIds[j] && mFemOldNeighbourIds[j] != -1) {
                noalias(temp_neighbours_elastic_contact_forces[i]) = mNeighbourRigidFacesElasticContactForce[j];
                noalias(temp_neighbours_contact_forces[i]) = mNeighbourRigidFacesTotalContactForce[j];
                temp_contact_radius[i] = mNeighbourRigidContactRadius[j];
                temp_indentation[i] = mNeighbourRigidIndentation[j];
                temp_tg_of_fri_ang[i] = mNeighbourRigidTgOfFriAng[j];
                temp_contact_stress[i] = mNeighbourRigidContactStress[j];
                temp_cohesion[i] =  mNeighbourRigidCohesion[j];
                break;
            }
        }
    }

    mFemOldNeighbourIds.swap(temp_neighbours_ids);
    mNeighbourRigidFacesElasticContactForce.swap(temp_neighbours_elastic_contact_forces);
    mNeighbourRigidFacesTotalContactForce.swap(temp_neighbours_contact_forces);
    mNeighbourRigidContactRadius.swap(temp_contact_radius);
    mNeighbourRigidIndentation.swap(temp_indentation);
    mNeighbourRigidTgOfFriAng.swap(temp_tg_of_fri_ang);
    mNeighbourRigidContactStress.swap(temp_contact_stress);
    mNeighbourRigidCohesion.swap(temp_cohesion);
}

double ContactInfoContinuumSphericParticle::GetParticleInitialCohesion()            { return SphericParticle::GetFastProperties()->GetParticleInitialCohesion();            }
double ContactInfoContinuumSphericParticle::GetAmountOfCohesionFromStress()         { return SphericParticle::GetFastProperties()->GetAmountOfCohesionFromStress();         }
double ContactInfoContinuumSphericParticle::GetParticleConicalDamageContactRadius() { return SphericParticle::GetFastProperties()->GetParticleConicalDamageContactRadius(); }
double ContactInfoContinuumSphericParticle::GetParticleConicalDamageMaxStress()     { return SphericParticle::GetFastProperties()->GetParticleConicalDamageMaxStress();     }
double ContactInfoContinuumSphericParticle::GetParticleConicalDamageGamma()         { return SphericParticle::GetFastProperties()->GetParticleConicalDamageGamma();         }
double ContactInfoContinuumSphericParticle::GetLevelOfFouling()                     { return SphericParticle::GetFastProperties()->GetLevelOfFouling();                     }

void   ContactInfoContinuumSphericParticle::SetParticleInitialCohesionFromProperties(double* particle_initial_cohesion)          { SphericParticle::GetFastProperties()->SetParticleInitialCohesionFromProperties( particle_initial_cohesion);  }
void   ContactInfoContinuumSphericParticle::SetAmountOfCohesionFromStressFromProperties(double* amount_of_cohesion_from_stress)  { SphericParticle::GetFastProperties()->SetAmountOfCohesionFromStressFromProperties( amount_of_cohesion_from_stress);  }
void   ContactInfoContinuumSphericParticle::SetParticleConicalDamageContactRadiusFromProperties(double* particle_contact_radius) { SphericParticle::GetFastProperties()->SetParticleConicalDamageContactRadiusFromProperties( particle_contact_radius); }
void   ContactInfoContinuumSphericParticle::SetParticleConicalDamageMaxStressFromProperties(double* particle_max_stress)         { SphericParticle::GetFastProperties()->SetParticleConicalDamageMaxStressFromProperties( particle_max_stress);         }
void   ContactInfoContinuumSphericParticle::SetParticleConicalDamageGammaFromProperties(double* particle_gamma)                  { SphericParticle::GetFastProperties()->SetParticleConicalDamageGammaFromProperties( particle_gamma);                  }
void   ContactInfoContinuumSphericParticle::SetLevelOfFoulingFromProperties(double* level_of_fouling)                            { SphericParticle::GetFastProperties()->SetLevelOfFoulingFromProperties( level_of_fouling);                            }

double ContactInfoContinuumSphericParticle::SlowGetParticleInitialCohesion()            { return GetProperties()[PARTICLE_INITIAL_COHESION]; }
double ContactInfoContinuumSphericParticle::SlowGetAmountOfCohesionFromStress()         { return GetProperties()[AMOUNT_OF_COHESION_FROM_STRESS]; }
double ContactInfoContinuumSphericParticle::SlowGetParticleConicalDamageContactRadius() { return GetProperties()[CONICAL_DAMAGE_CONTACT_RADIUS];  }
double ContactInfoContinuumSphericParticle::SlowGetParticleConicalDamageMaxStress()     { return GetProperties()[CONICAL_DAMAGE_MAX_STRESS];      }
double ContactInfoContinuumSphericParticle::SlowGetParticleConicalDamageGamma()         { return GetProperties()[CONICAL_DAMAGE_GAMMA];           }
double ContactInfoContinuumSphericParticle::SlowGetLevelOfFouling()                     { return GetProperties()[LEVEL_OF_FOULING];               }

}  // namespace Kratos.