//
//   Project Name:        KratosConstitutiveModelsApplication $
//   Created by:          $Author:                    LHauser $
//   Last modified by:    $Co-Author:     LMonforte, MCiantia $
//   Date:                $Date:                    July 2018 $
//   Revision:            $Revision:                      0.0 $
//
//

#if !defined(KRATOS_SOIL_NON_ASSOCIATIVE_MODEL_H_INCLUDED )
#define      KRATOS_SOIL_NON_ASSOCIATIVE_MODEL_H_INCLUDED


// Clay And Sand Model (CASM)

// System includes

// External includes
#include <iostream>
#include <fstream>

// Project includes
#include "custom_models/plasticity_models/non_associative_plasticity_model.hpp"



//***** the hardening law associated to this Model has ... variables
// Variables
// 0 Plastic Multiplier
// 1 Volumetric Plastic Strain
// 2 Dev Plastic Strain
// 3 Abs Value Volumetric Plastic Strain
// 4 B (bounding)
// 5 pc preconsolidation

// 8 NonLocal Plastic Vol Def
// 9 Constrained Modulus (not correct, to be corrected)

namespace Kratos
{
   ///@addtogroup ConstitutiveModelsApplication
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

   /// Short class definition.
   /** Detail class definition.
    */
   template<class TElasticityModel, class TYieldSurface>
   class KRATOS_API(CONSTITUTIVE_MODELS_APPLICATION) SoilNonAssociativeModel : public NonAssociativePlasticityModel<TElasticityModel, TYieldSurface >
   {
      public:

         ///@name Type Definitions
         ///@{

         //elasticity model
         typedef TElasticityModel                               ElasticityModelType;

         //yield surface
         typedef TYieldSurface                                     YieldSurfaceType;


         // derived type
         typedef NonAssociativePlasticityModel<ElasticityModelType,YieldSurfaceType>  DerivedType;

         //base type
         typedef PlasticityModel<ElasticityModelType,YieldSurfaceType>  BaseType;

         //common types
         typedef typename BaseType::Pointer                         BaseTypePointer;
         typedef typename BaseType::SizeType                               SizeType;
         typedef typename BaseType::VoigtIndexType                   VoigtIndexType;
         typedef typename BaseType::MatrixType                           MatrixType;
         typedef typename BaseType::VectorType                           VectorType;
         typedef typename BaseType::ModelDataType                     ModelDataType;
         typedef typename BaseType::MaterialDataType               MaterialDataType;
         typedef typename BaseType::PlasticDataType                 PlasticDataType;
         typedef typename BaseType::InternalVariablesType     InternalVariablesType;

         
         /// Pointer definition of SoilNonAssociativeModel
         KRATOS_CLASS_POINTER_DEFINITION( SoilNonAssociativeModel );

         ///@}
         ///@name Life Cycle
         ///@{

         /// Default constructor.
         SoilNonAssociativeModel() : DerivedType() { mInitialized = false; }

         /// Copy constructor.
         SoilNonAssociativeModel(SoilNonAssociativeModel const& rOther) : DerivedType(rOther), mInitialized(rOther.mInitialized) {}

         /// Assignment operator.
         SoilNonAssociativeModel& operator=(SoilNonAssociativeModel const& rOther)
         {
            DerivedType::operator=(rOther);
            this->mInitialized = rOther.mInitialized;
            return *this;
         }

         /// Clone.
         ConstitutiveModel::Pointer Clone() const override
         {
            return ( SoilNonAssociativeModel::Pointer(new SoilNonAssociativeModel(*this)) );
         }

         /// Destructor.
         virtual ~SoilNonAssociativeModel() {}


         ///@}
         ///@name Operators
         ///@{


         ///@}
         ///@name Operations
         ///@{

         

         /**
          * Check
          */    
         virtual int Check(const Properties& rMaterialProperties, const ProcessInfo& rCurrentProcessInfo) override
         {
            KRATOS_TRY

            //LMV: to be implemented. but should not enter in the base one

            return 0;

            KRATOS_CATCH("")
         }

         ///@}
         ///@name Access
         ///@{

         /**
          * Has Values
          */   
         virtual bool Has(const Variable<double>& rThisVariable) override
         {
            if(rThisVariable == PLASTIC_STRAIN || rThisVariable == DELTA_PLASTIC_STRAIN )
               return true;

            return false;
         }


         ///@}
         ///@name Inquiry
         ///@{


         ///@}
         ///@name Input and output
         ///@{

         /// Turn back information as a string.
         virtual std::string Info() const override
         {
            std::stringstream buffer;
            buffer << "SoilNonAssociativeModel" ;
            return buffer.str();
         }

         /// Print information about this object.
         virtual void PrintInfo(std::ostream& rOStream) const override
         {
            rOStream << "SoilNonAssociativeModel";
         }

         /// Print object's data.
         virtual void PrintData(std::ostream& rOStream) const override
         {
            rOStream << "SoilNonAssociativeModel Data";
         }


         ///@}
         ///@name Friends
         ///@{


         ///@}

      protected:
         ///@name Protected static Member Variables
         ///@{


         ///@}
         ///@name Protected member Variables
         ///@{

         bool mInitialized; 

         ///@}
         ///@name Protected Operators
         ///@{


         ///@}
         ///@name Protected Operations
         ///@{

         
         
            //***************************************************************************************
            //***************************************************************************************
            // Compute Elasto Plastic Matrix
            void ComputeElastoPlasticTangentMatrix( ModelDataType & rValues, PlasticDataType & rVariables, Matrix & rEPMatrix) override
            {

               KRATOS_TRY

               // evaluate constitutive matrix and plastic flow
               Matrix ElasticMatrix(6,6);
               noalias(ElasticMatrix) = ZeroMatrix(6,6);
               this->mElasticityModel.CalculateConstitutiveTensor( rValues, ElasticMatrix);

               VectorType DeltaStressYieldCondition = this->mYieldSurface.CalculateDeltaStressYieldCondition( rVariables, DeltaStressYieldCondition);
               VectorType PlasticPotentialDerivative = this->mYieldSurface.CalculateDeltaPlasticPotential( rVariables, PlasticPotentialDerivative);


               MatrixType PlasticPotDerTensor;
               PlasticPotDerTensor = ConstitutiveModelUtilities::StrainVectorToTensor( PlasticPotentialDerivative, PlasticPotDerTensor);
               double H = this->mYieldSurface.GetHardeningRule().CalculateDeltaHardening( rVariables, H, PlasticPotDerTensor);

               VectorType AuxF = prod( trans(DeltaStressYieldCondition), rEPMatrix);
               VectorType AuxG = prod( rEPMatrix, PlasticPotentialDerivative);

               Matrix PlasticUpdateMatrix(6,6);
               noalias(PlasticUpdateMatrix) = ZeroMatrix(6,6);
               double denom = 0;
               for (unsigned int i = 0; i < 6; i++) {
                  denom += AuxF(i)*PlasticPotentialDerivative(i);
                  for (unsigned int j = 0; j < 6; j++) {
                     PlasticUpdateMatrix(i,j) = AuxG(i) * AuxF(j);
                  }
               }

               rEPMatrix -= PlasticUpdateMatrix / ( H + denom);


               KRATOS_CATCH("")
            }
            //***********************************************************************************
            //***********************************************************************************
            // Compute one step of the elasto-plastic problem
            void ComputeOneStepElastoPlasticProblem( ModelDataType & rValues, PlasticDataType & rVariables, const MatrixType & rDeltaDeformationMatrix) override
            {
               KRATOS_TRY
            

               MatrixType StressMatrix;
               // evaluate constitutive matrix and plastic flow
               double & rPlasticMultiplier = rVariables.Internal.Variables[0];
               double & rPlasticVolDef = rVariables.Internal.Variables[1]; 
               double & rPlasticDevDef = rVariables.Internal.Variables[2];

               Matrix ElasticMatrix(6,6);
               noalias(ElasticMatrix) = ZeroMatrix(6,6);
               this->mElasticityModel.CalculateConstitutiveTensor( rValues, ElasticMatrix);

               VectorType DeltaStressYieldCondition = this->mYieldSurface.CalculateDeltaStressYieldCondition( rVariables, DeltaStressYieldCondition);
               VectorType PlasticPotentialDerivative = this->mYieldSurface.CalculateDeltaPlasticPotential( rVariables, PlasticPotentialDerivative);

               MatrixType PlasticPotDerTensor;
               PlasticPotDerTensor = ConstitutiveModelUtilities::StrainVectorToTensor( PlasticPotentialDerivative, PlasticPotDerTensor);
               double H = this->mYieldSurface.GetHardeningRule().CalculateDeltaHardening( rVariables, H, PlasticPotDerTensor);

               MatrixType StrainMatrix = prod( rDeltaDeformationMatrix, trans( rDeltaDeformationMatrix) );
               VectorType StrainVector; 
               this->ConvertCauchyGreenTensorToHenckyVector( StrainMatrix, StrainVector);

               VectorType AuxVector;
               AuxVector = prod( ElasticMatrix, StrainVector);
               double DeltaGamma;
               DeltaGamma = MathUtils<double>::Dot( AuxVector, DeltaStressYieldCondition);

               double Denominador = H + MathUtils<double>::Dot( DeltaStressYieldCondition, prod(ElasticMatrix, PlasticPotentialDerivative) );

               DeltaGamma /= Denominador;

               if ( DeltaGamma < 0)
                  DeltaGamma = 0;

               MatrixType UpdateMatrix;
               this->ConvertHenckyVectorToCauchyGreenTensor( -DeltaGamma * PlasticPotentialDerivative / 2.0, UpdateMatrix);
               UpdateMatrix = prod( rDeltaDeformationMatrix, UpdateMatrix);


               rValues.StrainMatrix = prod( UpdateMatrix, rValues.StrainMatrix);
               rValues.StrainMatrix = prod( rValues.StrainMatrix, trans(UpdateMatrix));

               this->mElasticityModel.CalculateStressTensor( rValues, StressMatrix);

               rPlasticMultiplier += DeltaGamma;
               double VolPlasticIncr = 0.0;
               for (unsigned int i = 0; i < 3; i++)
                  VolPlasticIncr += DeltaGamma * PlasticPotentialDerivative(i);
               rPlasticVolDef += VolPlasticIncr;

               double DevPlasticIncr = 0.0;
               for (unsigned int i = 0; i < 3; i++)
                  DevPlasticIncr += pow( DeltaGamma * PlasticPotentialDerivative(i) - VolPlasticIncr/3.0, 2.0);
               for (unsigned int i = 3; i < 6; i++)
                  DevPlasticIncr += 2.0 * pow( DeltaGamma *  PlasticPotentialDerivative(i) /2.0 , 2.0);
               DevPlasticIncr = sqrt(DevPlasticIncr);
               rPlasticDevDef += DevPlasticIncr;



               KRATOS_CATCH("")
            }
            // ****************************************************************************
            //  compute the stress state by using implex
            void  CalculateImplexPlasticStep(ModelDataType& rValues, PlasticDataType&  rVariables, MatrixType&  rStressMatrix, const MatrixType & rDeltaDeformationMatrix) override
            {
               KRATOS_TRY


               const double & rPlasticMultiplierOld = this->mPreviousInternal.Variables[0];
               double & rPlasticMultiplier    = rVariables.Internal.Variables[0];
               double  DeltaPlasticMultiplier = (rPlasticMultiplier - rPlasticMultiplierOld);

               if ( DeltaPlasticMultiplier < 0)
                  DeltaPlasticMultiplier = 0;

               
               this->mElasticityModel.CalculateStressTensor(rValues,rStressMatrix);

               VectorType PlasticPotentialDerivative = this->mYieldSurface.CalculateDeltaPlasticPotential( rVariables, PlasticPotentialDerivative);


               MatrixType UpdateMatrix;
               this->ConvertHenckyVectorToCauchyGreenTensor( -DeltaPlasticMultiplier * PlasticPotentialDerivative / 2.0, UpdateMatrix);
               UpdateMatrix = prod( rDeltaDeformationMatrix, UpdateMatrix);


               rValues.StrainMatrix = prod( UpdateMatrix, rValues.StrainMatrix);
               rValues.StrainMatrix = prod( rValues.StrainMatrix, trans(UpdateMatrix));

               this->mElasticityModel.CalculateStressTensor( rValues, rStressMatrix);


               KRATOS_CATCH("")
            }

            //********************************************************************
            //********************************************************************
            // UpdateInternalVariables
            virtual void UpdateInternalVariables(ModelDataType& rValues, PlasticDataType& rVariables, const MatrixType& rStressMatrix) override
            {
               KRATOS_TRY

               this->mUltraPreviousInternal = this->mPreviousInternal;

               this->mPreviousInternal.Variables[3] = this->mInternal.Variables[3];
               this->mInternal.Variables[3] = this->mInternal.Variables[3] + fabs( rVariables.Internal.Variables[1] - this->mInternal.Variables[1]);
               for (unsigned int i = 0; i < 10; i++) {
                     if (  (i != 3) && (i != 8)  ) {
                     double & rCurrentPlasticVariable = rVariables.Internal.Variables[i]; 
                     double & rPreviousPlasticVariable    = this->mInternal.Variables[i];

                     this->mPreviousInternal.Variables[i] = rPreviousPlasticVariable;
                     rPreviousPlasticVariable = rCurrentPlasticVariable;
                  }
               }


               KRATOS_CATCH("")
            }


            //***************************************************************************************
            //***************************************************************************************
            // Correct Yield Surface Drift According to 
            virtual void ReturnStressToYieldSurface( ModelDataType & rValues, PlasticDataType & rVariables) override
            {
               KRATOS_TRY

               double Tolerance = 1e-8;

               MatrixType StressMatrix;
               this->mElasticityModel.CalculateStressTensor( rValues, StressMatrix);
               double YieldSurface = this->mYieldSurface.CalculateYieldCondition( rVariables, YieldSurface);

               if ( fabs(YieldSurface) < Tolerance)
                  return;

               const ModelDataType & rModelData = rVariables.GetModelData();

               double & rPlasticVolDef = rVariables.Internal.Variables[1]; 
               double & rPlasticDevDef = rVariables.Internal.Variables[2];

               for (unsigned int i = 0; i < 150; i++) {

                  Matrix ElasticMatrix(6,6);
                  noalias(ElasticMatrix) = ZeroMatrix(6,6);
                  this->mElasticityModel.CalculateConstitutiveTensor( rValues, ElasticMatrix);

                  VectorType DeltaStressYieldCondition = this->mYieldSurface.CalculateDeltaStressYieldCondition( rVariables, DeltaStressYieldCondition);
                  VectorType PlasticPotentialDerivative = this->mYieldSurface.CalculateDeltaPlasticPotential( rVariables, PlasticPotentialDerivative);

                  MatrixType PlasticPotDerTensor;
                  PlasticPotDerTensor = ConstitutiveModelUtilities::StrainVectorToTensor( PlasticPotentialDerivative, PlasticPotDerTensor);
                  double H = this->mYieldSurface.GetHardeningRule().CalculateDeltaHardening( rVariables, H, PlasticPotDerTensor);

                  double DeltaGamma = YieldSurface;
                  DeltaGamma /= ( H + MathUtils<double>::Dot( DeltaStressYieldCondition, prod(ElasticMatrix, PlasticPotentialDerivative) ) );

                  MatrixType UpdateMatrix;
                  this->ConvertHenckyVectorToCauchyGreenTensor( -DeltaGamma * PlasticPotentialDerivative / 2.0, UpdateMatrix);

                  rValues.StrainMatrix = prod( UpdateMatrix, rValues.StrainMatrix);
                  rValues.StrainMatrix = prod( rValues.StrainMatrix, trans(UpdateMatrix));

                  this->mElasticityModel.CalculateStressTensor( rValues, StressMatrix);

                  double VolPlasticIncr = 0.0;
                  for (unsigned int i = 0; i < 3; i++)
                     VolPlasticIncr += DeltaGamma * PlasticPotentialDerivative(i);
                  rPlasticVolDef += VolPlasticIncr;

                  double DevPlasticIncr = 0.0;
                  for (unsigned int i = 0; i < 3; i++)
                     DevPlasticIncr += pow( DeltaGamma * PlasticPotentialDerivative(i) - VolPlasticIncr/3.0, 2.0);
                  for (unsigned int i = 3; i < 6; i++)
                     DevPlasticIncr += 2.0 * pow( DeltaGamma *  DeltaStressYieldCondition(i) /2.0 , 2.0);
                  DevPlasticIncr = DeltaGamma/fabs(DeltaGamma) * sqrt(DevPlasticIncr);
                  rPlasticDevDef += DevPlasticIncr;



                  YieldSurface = this->mYieldSurface.CalculateYieldCondition( rVariables, YieldSurface);


                  if ( fabs( YieldSurface) < Tolerance) {
                     return;
                  }
               }
               std::cout << " theStressPointDidNotReturnedCorrectly " << YieldSurface << std::endl;

               KRATOS_CATCH("")
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


         ///@}
         ///@name Private Operators
         ///@{


         ///@}
         ///@name Private Operations
         ///@{


         ///@}
         ///@name Private  Access
         ///@{


         ///@}
         ///@name Private Inquiry
         ///@{


         ///@}
         ///@name Serialization
         ///@{    
         friend class Serializer;

         virtual void save(Serializer& rSerializer) const override
         {
            KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, DerivedType )
            rSerializer.save("Initialized", mInitialized);
         }

         virtual void load(Serializer& rSerializer) override
         {
            KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, DerivedType )
            rSerializer.load("Initialized", mInitialized);
         }

         ///@}
         ///@name Un accessible methods
         ///@{


         ///@}

   }; // Class SoilNonAssociativeModel

   ///@}

   ///@name Type Definitions
   ///@{


   ///@}
   ///@name Input and output
   ///@{


   ///@} 
   ///@name Input and output 
   ///@{


   ///@}

   ///@} addtogroup block


}  // namespace Kratos.

#endif // KRATOS_SOIL_NON_ASSOCIATIVE__MODEL_H_INCLUDED  defined 

