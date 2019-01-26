//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes
#include <limits>

// External includes

// Project includes
#include "testing/testing.h"
#include "containers/model.h"

// Utility includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "factories/scheme_factory.h"
#include "spaces/ublas_space.h"

namespace Kratos
{
    namespace Testing
    {
        /// Tests

        // Spaces
        using SparseSpaceType = UblasSpace<double, CompressedMatrix, Vector>;
        using LocalSpaceType = UblasSpace<double, Matrix, Vector>;

        /// The definition of the scheme
        using SchemeType = Scheme<SparseSpaceType,LocalSpaceType>;

        /// The definition of the factory
        using FactoryType = SchemeFactory<SparseSpaceType,LocalSpaceType>;

        /**
         * Checks if the ResidualBasedIncrementalUpdateStaticScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedIncrementalUpdateStaticSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedIncrementalUpdateStaticScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedIncrementalUpdateStaticScheme");
        }

        /**
         * Checks if the ResidualBasedIncrementalUpdateStaticSchemeSlip performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedIncrementalUpdateStaticSchemeSlipFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedIncrementalUpdateStaticSchemeSlip"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedIncrementalUpdateStaticSchemeSlip");
        }

        /**
         * Checks if the ResidualBasedBossakDisplacementScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedBossakDisplacementSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedBossakDisplacementScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedBossakDisplacementScheme");
        }

        /**
         * Checks if the ResidualBasedNewmarkDisplacementScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedNewmarkDisplacementSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedNewmarkDisplacementScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedNewmarkDisplacementScheme");
        }

        /**
         * Checks if the ResidualBasedPseudoStaticDisplacementScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedPseudoStaticDisplacementSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedPseudoStaticDisplacementScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedPseudoStaticDisplacementScheme");
        }

        /**
         * Checks if the ResidualBasedBDFDisplacementScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedBDFDisplacementSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedBDFDisplacementScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedBDFDisplacementScheme");
        }

        /**
         * Checks if the ResidualBasedBDFCustomScheme performs correctly the Factory
         */
        KRATOS_TEST_CASE_IN_SUITE(ResidualBasedBDFCustomSchemeFactory, KratosCoreFastSuite)
        {
            Parameters this_parameters = Parameters(R"({"scheme_type" : "ResidualBasedBDFCustomScheme"})");
            SchemeType::Pointer p_scheme = FactoryType().Create(this_parameters);
            KRATOS_CHECK_C_STRING_EQUAL((p_scheme->Info()).c_str(), "ResidualBasedBDFCustomScheme");
        }

    } // namespace Testing
}  // namespace Kratos.

