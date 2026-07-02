#include <gtest/gtest.h>

#include "autest_fixture.hpp"
#include "environment.hpp"

const std::string PROXY_ERROR = "The proxy test for AU bit-compatibility failed. "
            "This is purely a regression test designed to notify about changes to the AU implementation. "
            "The test can fail for reasons like: change of the random number generator, "
            "change of the order of operations in the AU test, or general changes to the AU test. "
            "If this test is failing, run the expensive AU tests to verify the AU test is still working "
            "as expected and then adjust this proxy test (see autest.cpp).";

// This is a proxy test for the expensive TOST procedures that verify the correctness of the AU test.
// It verifies that AU test returns the exact p-values as pre-calculated on a fixed seed.
// Many benign changes to the code or the standard library could change the outcome of this test.
// If this test fails, run the TOST procedures for the AU test (autest.cpp).
// If they are still green, the AU test is still correct, and this test case can be adjusted to new expected p-values.
TEST_F(AuTestFixture, p_values1) {
    std::string filename = env->datapath("autest/random6.siteLH");
    ASSERT_TRUE(LoadSiteLH(filename.c_str()));

    Bootstrap(); // seeded with 1
    ComputeDeltas();

    std::vector<double> p_values = {};
    ComputePValues(p_values);

    ASSERT_TRUE(fabs(p_values[0] - 0.999881063400786) < 1E-10 &&
                fabs(p_values[1] - 2.14223655846113e-05) < 1E-10 &&
                fabs(p_values[2] - 0.0001188625294515355) < 1E-10 &&
                fabs(p_values[3] - 9.602775657535645e-06) < 1E-10 &&
                fabs(p_values[4] - 0.000154608297170622) < 1E-10 &&
                fabs(p_values[5] - 2.982922097122254e-05) < 1E-10 &&
                fabs(p_values[6] - 1.123892542920224e-07) < 1E-10) << PROXY_ERROR;
}

TEST_F(AuTestFixture, p_values2) {
    std::string filename = env->datapath("autest/tool_comparison.siteLH");
    ASSERT_TRUE(LoadSiteLH(filename.c_str()));

    Bootstrap(); // seeded with 1
    ComputeDeltas();

    std::vector<double> p_values = {};
    ComputePValues(p_values);

    ASSERT_TRUE(fabs(p_values[0] - 0.5246497758823337) < 1E-10 &&
                fabs(p_values[1] - 0.4753971649435209) < 1E-10 &&
                fabs(p_values[2] - 0.5236497758823337) < 1E-10) << PROXY_ERROR;
}
