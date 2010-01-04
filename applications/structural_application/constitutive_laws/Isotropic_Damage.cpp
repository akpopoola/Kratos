/*
==============================================================================
KratosStructuralApplication 
A library based on:
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi, Janosch Stascheit, Felix Nagel 
pooyan@cimne.upc.edu 
rrossi@cimne.upc.edu
janosch.stascheit@rub.de
nagel@sd.rub.de
- CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain
- Ruhr-University Bochum, Institute for Structural Mechanics, Germany


Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNERS.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  variablesIS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/
/* *********************************************************   
*          
*   Last Modified by:    $Author: nelson $
*   Date:                $Date: 2008-09-03 
\*   Revision:            $Revision: 1.1 $
*
* ***********************************************************/


// System includes 
#include <iostream>

// External includes 
#include<cmath>

// Project includes 
#include "includes/define.h"
#include "constitutive_laws/Isotropic_Damage.h"
#include "includes/constitutive_law.h"
#include "utilities/math_utils.h"
#include "custom_utilities/sd_math_utils.h"
#include "custom_utilities/tensor_utils.h"
#include "includes/variables.h"
#include "includes/process_info.h"
#include "includes/properties.h"


namespace Kratos
{
    namespace Isotropic_Damage_Auxiliaries
    {
        boost::numeric::ublas::bounded_matrix<double,2,2> mstemp;
	#ifdef _OPENMP
	#pragma omp threadprivate(mstemp)
	#endif
        boost::numeric::ublas::bounded_matrix<double,2,2> msaux;
	#ifdef _OPENMP
	#pragma omp threadprivate(msaux)
	#endif
	Matrix ConstitutiveMatrixAux(3,3);
        #ifdef _OPENMP
        #pragma omp threadprivate(ConstitutiveMatrixAux)
        #endif
        Vector StrainVectorPerturbation(3);
	#ifdef _OPENMP
        #pragma omp threadprivate(StrainVectorPerturbation)
        #endif
	Vector StressVectorPerturbation(3);
	#ifdef _OPENMP
        #pragma omp threadprivate(StressVectorPerturbation)
        #endif
	Vector StrainVectorPerturbation_aux(3);
	#ifdef _OPENMP
        #pragma omp threadprivate(StrainVectorPerturbation_aux)
        #endif
	Vector StressVectorPerturbation_aux(3);
	#ifdef _OPENMP
        #pragma omp threadprivate(StressVectorPerturbation_aux)
        #endif



     /* // Ver pagina 57 " Metodologia de Evaluacion de Estructuras de Hormigon Armado"								
	Matrix C(0,0);
	#pragma omp threadprivate(C)
	Matrix D(0,0);
	#pragma omp threadprivate(D)
	Matrix V(0,0);
	#pragma omp threadprivate(V)
        Vector d_Sigma(0);
	#pragma omp threadprivate(d_Sigma)
      */
    } 


    using namespace Isotropic_Damage_Auxiliaries;

	/**
	 *	TO BE TESTED!!!
	 */

         Isotropic_Damage::Isotropic_Damage() 
	: ConstitutiveLaw< Node<3> >()
	
	{
	  KRATOS_ERROR(std::logic_error,"Calling the empty constructor for Isotropic Damage","");
	}

	 Isotropic_Damage::Isotropic_Damage(FluencyCriteriaPointer FluencyCriteria, SofteningHardeningCriteriaPointer SofteningBehavior, PropertiesPointer Property) 
	: ConstitutiveLaw< Node<3> >()
	{
	      mpFluencyCriteria   = FluencyCriteria;
              mpSofteningBehavior = SofteningBehavior;
              mpProperties        = Property;
	      //KRATOS_WATCH(*mProperties)
	}
	/**
	 *	TO BE TESTED!!!
	 */
	Isotropic_Damage::~Isotropic_Damage()
	{
	}
	
	
	bool Isotropic_Damage::Has( const Variable<double>& rThisVariable )
	{
		return false;
	}
	
	bool Isotropic_Damage::Has( const Variable<Vector>& rThisVariable )
	{
		return false;
	}
	
	bool Isotropic_Damage::Has( const Variable<Matrix>& rThisVariable )
	{
		return false;
	}
	
	double Isotropic_Damage::GetValue( const Variable<double>& rThisVariable )
	{
	  if( rThisVariable == DAMAGE)
	  {			
	  //KRATOS_WATCH(md)
	  return md_new;

	  }
	  else return 0.00;
	    //KRATOS_ERROR(std::logic_error, "double Variable case not considered", "");
	}
	

	Vector Isotropic_Damage::GetValue( const Variable<Vector>& rThisVariable )
	{
	    KRATOS_ERROR(std::logic_error, "Vector Variable case not considered", "");
	}
	
	Matrix Isotropic_Damage::GetValue( const Variable<Matrix>& rThisVariable )
	{
	    KRATOS_ERROR(std::logic_error, "Matrix Variable case not considered", "");
	}

    void Isotropic_Damage::SetValue( const Variable<double>& rThisVariable, const double& rValue, 
                                const ProcessInfo& rCurrentProcessInfo )
	{
	}
	
    void Isotropic_Damage::SetValue( const Variable<Vector>& rThisVariable, const Vector& rValue, 
                                const ProcessInfo& rCurrentProcessInfo )
	{
	}
	
    void Isotropic_Damage::SetValue( const Variable<Matrix>& rThisVariable, const Matrix& rValue, 
                                const ProcessInfo& rCurrentProcessInfo )
	{
	}
    
    void Isotropic_Damage::Calculate(const Variable<Matrix >& rVariable, Matrix& rResult, 
                   const ProcessInfo& rCurrentProcessInfo)
    {
    }


//***********************************************************************************************
//***********************************************************************************************

	void Isotropic_Damage::InitializeMaterial( const Properties& props,
	const GeometryType& geom,
	const Vector& ShapeFunctionsValues )
	
	{

	// Nota: Estas Variables no seran necesarias almacenarlas por cada elemento.
	// Basta con llamarlas con sus propiedades.
	mFc    = (*mpProperties)[FC];
	mFt    = (*mpProperties)[FT];
	mEc    = (*mpProperties)[CONCRETE_YOUNG_MODULUS_C];
	mEt    = (*mpProperties)[CONCRETE_YOUNG_MODULUS_T];
	mNU    = (*mpProperties)[POISSON_RATIO];
	mGE    = (*mpProperties)[FRACTURE_ENERGY];
	ml     = sqrt(fabs(geom.Area()));   // longitud del elemento
        mDE    = (*mpProperties)[DENSITY];

	 mpFluencyCriteria->InitializeMaterial(*mpProperties);
        //KRATOS_WATCH((*mpProperties))

         // la resistencia a traccion  o compresion del hormigon va evolucionando a medida que el daño se hace efectivo.
         // Por esta razon Sigma_y deberia ir evolucionado. No obstante, el modelo  
         mr_old = mpFluencyCriteria->mSigma_y;   //mFt/sqrt(mEc);
	 mr_o   = mr_old; 
         md_old = 0.00;
         md_new = 0.00;
	}

		

//***********************************************************************************************
//***********************************************************************************************

void Isotropic_Damage::InitializeSolutionStep( const Properties& props,
                    const GeometryType& geom,
                    const Vector& ShapeFunctionsValues ,
                    const ProcessInfo& CurrentProcessInfo)
{
				
}

//***********************************************************************************************
//***********************************************************************************************


	void Isotropic_Damage::CalculateNoDamageElasticMatrix(Matrix& C, const double E, const double NU)
	{ 
                if (C.size1()!=3) {C.resize(3,3, false);}
		double c1 = E / (1.00 - NU*NU);
		double c2 = c1 * NU;
		double c3 = 0.5*E / (1.00 + NU);

		C(0,0) = c1;	    C(0,1) = c2;	    C(0,2) = 0.0;
		C(1,0) = c2;	    C(1,1) = c1;	    C(1,2) = 0.0;
		C(2,0) = 0.0;	    C(2,1) = 0.0;	    C(2,2) = c3;
		
	}
	

//***********************************************************************************************
//***********************************************************************************************


      void Isotropic_Damage::CalculateConstitutiveMatrix(const Vector& StrainVector, Matrix& ConstitutiveMatrix)
      {
              //Isotropic_Damage::CalculateNoDamageElasticMatrix(ConstitutiveMatrix,mEc,mNU);
	     
              Vector Aux(3);
              CalculateStressAndTangentMatrix(Aux, StrainVector , ConstitutiveMatrix);
              //KRATOS_WATCH(ConstitutiveMatrix)
      }


//***********************************************************************************************
//***********************************************************************************************


void Isotropic_Damage::CalculateStressAndDamage(Vector& StressVector, const Vector& StrainVector )

{ 
       
	double Tau           = 0.00;
        double ElasticDomain = 0.00;
	double A   	     = 0.00;
        //double delta_d       = 0.00;

        Vector No_Damage_Stress = ZeroVector(3); 
 
	A  = 1.00/((mGE*mEc)/(ml*mFt*mFt)-0.5);
	if (A < 0.00)
	{    
	A  = 0.00;
	std::cout<< "Warning: A is less than zero"<<std::endl;
	}
 
	CalculateNoDamageStress(StrainVector, StressVector);
        mpFluencyCriteria->CalculateEquivalentUniaxialStressViaInvariants(StressVector,ElasticDomain);
	if(ElasticDomain<0.00)
          {
	      Tau  =  mr_old;
          }
        else
          {
              Tau  =  mpFluencyCriteria->mSigma_e;
          }
        
        mr_new  =  std::max(mr_old,Tau);
        md_new  =  mpSofteningBehavior->FunctionSofteningHardeningBehavior(A,mr_o,mr_new);
                 
}



//***********************************************************************************************
//***********************************************************************************************


void  Isotropic_Damage::FinalizeSolutionStep( const Properties& props,
                                               const GeometryType& geom, 
                                               const Vector& ShapeFunctionsValues ,
                                             const ProcessInfo& CurrentProcessInfo)

			{ 
                            md_old = md_new;
                            mr_old = mr_new;
                            mpFluencyCriteria->mSigma_y = mpFluencyCriteria->mSigma_e;
                            
			}


//***********************************************************************************************
//***********************************************************************************************

void Isotropic_Damage::CalculateNoDamageStress(const Vector& StrainVector, Vector& StressVector)
	{
		
		
		double c1 = mEc / (1.00 - mNU*mNU);
		double c2 = c1 * mNU;
		double c3 = 0.5*mEc / (1.00 + mNU);
	    
		
		  
		StressVector[0] = c1*StrainVector[0] + c2 * (StrainVector[1])	;
		StressVector[1] = c1*StrainVector[1] + c2 * (StrainVector[0])	;
		StressVector[2] = c3*StrainVector[2];
	}
	

//***********************************************************************************************
//***********************************************************************************************
	

 void Isotropic_Damage::CalculateStress( const Vector& StrainVector, Vector& StressVector)
            
	{

	    double ElasticDomain;
            CalculateStressAndDamage(StressVector, StrainVector);
            noalias(StressVector) = (1.00 - md_new)*StressVector; 
//             KRATOS_WATCH(md_new);
            //KRATOS_WATCH(StressVector)
            //mpFluencyCriteria->CalculateEquivalentUniaxialStressViaInvariants(StressVector,ElasticDomain);

        }


//***********************************************************************************************
//***********************************************************************************************	

  void Isotropic_Damage::CalculateCauchyStresses(
		Vector& rCauchy_StressVector,
		const Matrix& rF,
		const Vector& rPK2_StressVector,
		const Vector& rGreenLagrangeStrainVector)
    {
		Matrix S = MathUtils<double>::StressVectorToTensor( rPK2_StressVector );

		double J = MathUtils<double>::Det2( rF );

		noalias(mstemp) = prod(rF,S);
		noalias(msaux)  = prod(mstemp,trans(rF));
		msaux *= J;

		if(rCauchy_StressVector.size() != 3)
			rCauchy_StressVector.resize(3);
		
		rCauchy_StressVector[0] = msaux(0,0);
		rCauchy_StressVector[1] = msaux(1,1);
		rCauchy_StressVector[2] = msaux(1,2);
  }


//***********************************************************************************************
//**********************************************************************************************
/*
	 void Isotropic_Damage::CalculateStressAndTangentMatrix(Vector& StressVector,
                    const Vector& StrainVector,
                    Matrix& algorithmicTangent)
                         {
                       
			  double DevSigma = 0.00;
			  double A   	  = 0.00;
                          Vector DerivateFluencyCriteria = ZeroVector(6);
                          Vector DerivateFluencyCriteria_aux = ZeroVector(3);
                          Vector NoDamageStressVector = ZeroVector(3);
                          identity_matrix<double> Unit(3);
                          Matrix Aux = ZeroMatrix(6,6);
                          Matrix C; 
                          algorithmicTangent.resize(3,3,false);
                          C.resize(3,3, false);
                          
                          
                          // computo matriz elastica
 			  if((mpFluencyCriteria->mElasticDomain) < 0.00)
			    {
                               CalculateNoDamageElasticMatrix(algorithmicTangent, mEc,mNU);
			    }
                         // computo la matriz tangente
                          else 
                          {
			  A  = 1.00/((mGE*mEc)/(ml*mFt*mFt)-0.5);
                          CalculateNoDamageStress(StrainVector, NoDamageStressVector);
                          CalculateNoDamageElasticMatrix(C, mEc,mNU);
                          //if(norm_2(NoDamageStressVector)<1E-5) {NoDamageStressVector = ZeroVector(3);} 

                          mpFluencyCriteria->CalculateDerivateFluencyCriteria(NoDamageStressVector, DerivateFluencyCriteria);
                          //KRATOS_WATCH(DerivateFluencyCriteria) 

                          // Lo coloco en un vetor de 3
                          DerivateFluencyCriteria_aux(0) =  DerivateFluencyCriteria(0);
                          DerivateFluencyCriteria_aux(1) =  DerivateFluencyCriteria(1);
                          DerivateFluencyCriteria_aux(2) =  DerivateFluencyCriteria(3);


			  DevSigma = (1.00-md_new)*( (1.00/mr_new) + A/(mr_o));
			  Aux = md_new*Unit + DevSigma*outer_prod(NoDamageStressVector, DerivateFluencyCriteria_aux);
                          //KRATOS_WATCH(Aux)

                         
                          noalias(algorithmicTangent) = prod(Matrix(Unit -Aux), C);  
                          KRATOS_WATCH(algorithmicTangent)   
                          //CalculateNoDamageElasticMatrix(algorithmicTangent, mEc,mNU);        
                          //KRATOS_WATCH(algorithmicTangent)         
                          }
                          
                         }
*/
//***********************************************************************************************
//***********************************************************************************************	
  void Isotropic_Damage::CalculateStressAndTangentMatrix(Vector& StressVector,
                    const Vector& StrainVector,
                    Matrix& algorithmicTangent)
                         {
			 // Using perturbation methods
                         long double delta_strain =  0.00;
			 long double factor       =  1E-15;
                         long double max          =  1E-16;
                         double last_damage       =  md_new;
			 double last_r            =  mr_new;
                         
			 
			 StrainVectorPerturbation.resize(3, false);
			 StressVectorPerturbation.resize(3, false);
			 StrainVectorPerturbation_aux.resize(3, false);
			 StressVectorPerturbation_aux.resize(3, false);
			 
			 // diferencia con caso Damage-3D
			 noalias(StrainVectorPerturbation)     = StrainVector;
			 noalias(StrainVectorPerturbation_aux) = StrainVector;  			  
               
                         algorithmicTangent = ZeroMatrix(3,3);
                         

			 for (unsigned int i=0;i<StrainVectorPerturbation.size();i++)
			    {
				  
				 if  (fabs(StrainVector(i))<1E-10)
				    {
				     delta_strain = (*std::min_element(StrainVector.begin(),StrainVector.end()))*factor;
				     if (delta_strain==0.00)
					 {
					    delta_strain = factor;  //perturbacion
					 }
				    } 
				 else
				    {
				      delta_strain = StrainVector(i)*factor;
				    }
					
				   if (delta_strain < max) {delta_strain=max;}
                                   
                                     
				     StrainVectorPerturbation(i)     += delta_strain;
                                     StrainVectorPerturbation_aux(i) -= delta_strain;
                                     
				     Isotropic_Damage::CalculateStress(StrainVectorPerturbation, StressVectorPerturbation);
				     
                                     Isotropic_Damage::CalculateStress(StrainVectorPerturbation_aux, StressVectorPerturbation_aux);
				     
     
                                     // Antiguo procedimiento
			             //StressVectorPerturbation = StressVectorPerturbation-StressVector;
				     //KRATOS_WATCH(StressVectorPerturbation);
				     //KRATOS_WATCH(StressVectorPerturbation_aux);
				     noalias(StressVectorPerturbation) = StressVectorPerturbation-StressVectorPerturbation_aux;
				     //KRATOS_WATCH(StressVectorPerturbation);
                                     //noalias(StressVectorPerturbation) = StressVectorPerturbation/delta_strain;
				     noalias(StressVectorPerturbation) = StressVectorPerturbation/(2.00*delta_strain);
				    
                                   			      
				    for (unsigned int j = 0; j<StrainVectorPerturbation.size(); j++)
					 {
					    algorithmicTangent(j,i) = StressVectorPerturbation(j); 
					 }
				  
				    md_new     = last_damage;
				    mr_new = last_r;
				    StrainVectorPerturbation = StrainVector; 
				    StrainVectorPerturbation_aux = StrainVector; 
                                    
			         }
                              //KRATOS_WATCH(algorithmicTangent);                                           
                         }
//***********************************************************************************************
//***********************************************************************************************	
void Isotropic_Damage::Calculate(const Variable<double>& rVariable, 
                                    double& Output, 
                                    const ProcessInfo& rCurrentProcessInfo)
   {
    double local_damage =  fabs(1.00-md_new); 
    Output = sqrt(local_damage*mEc/mDE);
    if(Output==0){Output = 1;}
   }

}




