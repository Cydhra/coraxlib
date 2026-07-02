/*
 * These tests are disabled by default.
 * They read in reference results from consel, and then execute the au-test N times (where the reference data includes
 * a value for N). Then, two one-sided t-tests are performed to reject the hypothesis that the AU-Tests of corax yield
 * a different distribution of p-values than the consel implementation.
 * This procedure can be used to safely conclude that the corax AU-test implementation is equivalent to consel.
 *
 * The tests are disabled by default because they take minutes to run each.
 * In gtest, the command line option `--gtest_also_run_disabled_tests` can be used to run them anyway.
 * Note that this does not work in ctest, because ctest assumes disabled tests can never run and never calls gtest in
 * this case.
 *
 * To force ctest to run the disabled tests, run it with the following command, which prompts ctest to print the exact
 * gtest commands, and then runs the commands manually:
 *
 * eval $(ctest -R AuTestFixture --gtest_also_run_disabled_tests -V -N | perl -ne 'if (/Test command: (.+)/) { printf "$1;";}')
 */

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "autest_fixture.hpp"


// generated approximation of the quantile function of students-t-distribution because C++ doesn't have that
static double t_critical_0_995(int df) {
    struct Point {
        int df;
        double t;
    };

    static const Point table[] = {
        {1, 63.657}, {2, 9.925}, {3, 5.841}, {4, 4.604}, {5, 4.032},
        {6, 3.707}, {7, 3.499}, {8, 3.355}, {9, 3.250}, {10, 3.169},
        {15, 2.947}, {20, 2.845}, {30, 2.750}, {50, 2.678}, {100, 2.626}
    };

    if (df <= 1) return table[0].t;
    if (df >= 100) return 2.576;

    for (size_t i = 0; i < std::size(table) - 1; ++i) {
        if (df >= table[i].df && df <= table[i + 1].df) {
            double x0 = table[i].df;
            double y0 = table[i].t;
            double x1 = table[i + 1].df;
            double y1 = table[i + 1].t;

            double t = (df - x0) / (x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }
    return 2.576;
}

void reject_hypotheses(
    const TreeStatistics &reference_statistics,
    const TreeStatistics &testing_statistics,
    const unsigned int num_samples
) {
    constexpr double EQUIVALENCE_MARGIN = 0.02;

    struct Unrejected {
        bool lower;
        bool upper;
        size_t i;
        double ref_mean;
        double ref_var;
        double test_mean;
        double test_var;
    };

    std::vector<Unrejected> unrejected;

    size_t num_trees = reference_statistics.mean.size();

    for (size_t i = 0; i < num_trees; ++i) {
        double se_ref_sq = reference_statistics.variance[i] / double(num_samples);
        double se_test_sq = testing_statistics.variance[i] / double(num_samples);
        double se_delta_sq = se_ref_sq + se_test_sq;
        double se_delta = std::sqrt(se_delta_sq);

        bool lower_bound_rejected = false;
        bool upper_bound_rejected = false;

        if (se_ref_sq == 0.0 && se_test_sq == 0.0) {
            lower_bound_rejected =
                    testing_statistics.mean[i] >= reference_statistics.mean[i] - EQUIVALENCE_MARGIN;
            upper_bound_rejected =
                    testing_statistics.mean[i] <= reference_statistics.mean[i] + EQUIVALENCE_MARGIN;
        } else {
            double df_individual = double(num_samples - 1);

            double pooled_df =
                    df_individual * (se_delta_sq * se_delta_sq) /
                    (se_ref_sq * se_ref_sq + se_test_sq * se_test_sq);

            double lower_stat =
                    (reference_statistics.mean[i] - (testing_statistics.mean[i] - EQUIVALENCE_MARGIN))
                    / se_delta;

            double upper_stat =
                    (reference_statistics.mean[i] - (testing_statistics.mean[i] + EQUIVALENCE_MARGIN))
                    / se_delta;

            int df = static_cast<int>(std::round(pooled_df));
            double critical = t_critical_0_995(df);

            lower_bound_rejected = lower_stat > critical;
            upper_bound_rejected = upper_stat < -critical;
        }

        if (!(lower_bound_rejected && upper_bound_rejected)) {
            unrejected.push_back({
                lower_bound_rejected, upper_bound_rejected,
                i,
                reference_statistics.mean[i],
                reference_statistics.variance[i],
                testing_statistics.mean[i],
                testing_statistics.variance[i]
            });
        } else {
            std::cout << "Rejected inequality: tree " << i << " is not significantly better or worse "
                    << "(consel mean: " << reference_statistics.mean[i] << ", variance: "
                    << reference_statistics.variance[i] << "; corax mean: "
                    << testing_statistics.mean[i] << ", variance: " << testing_statistics.variance[i] << ")" <<
                    std::endl;
        }
    }

    std::ostringstream oss;
    oss << "failed to reject inequality hypotheses for " << unrejected.size() << " trees" << std::endl;
    for (unsigned int i = 0; i < unrejected.size(); i++) {
        oss << "tree " << unrejected[i].i << " failed to reject "
                << (unrejected[i].lower
                        ? (unrejected[i].upper ? "both bounds" : "the lower bound")
                        : "the upper bound") << ". "
                << "(consel mean: " << unrejected[i].ref_mean << ", variance: "
                << unrejected[i].ref_var << "; corax mean: "
                << unrejected[i].test_mean << ", variance: " << unrejected[i].test_var << ")" << std::endl;
    }

    ASSERT_EQ(unrejected.size(), 0) << oss.str();
}

void run_fixture(AuTestFixture *fixture, std::string site_lh, std::string reference_file) {
    // load AU test fixture
    std::string filename = env->datapath(site_lh);
    std::string refname = env->datapath(reference_file);
    ASSERT_TRUE(fixture->LoadSiteLH(filename.c_str()));

    // load reference data
    TreeStatistics reference_statistics;
    std::ifstream ref_data(refname);
    ASSERT_TRUE(ref_data);

    unsigned int samples = 0;
    ref_data >> samples;

    for (unsigned int s = 0; s < fixture->trees; ++s) {
        double mean;
        ref_data >> mean;
        reference_statistics.mean.push_back(mean);
    }

    for (unsigned int s = 0; s < fixture->trees; ++s) {
        double variance;
        ref_data >> variance;
        reference_statistics.variance.push_back(variance);
    }

    // run the test N times
    std::cout << "Running " << samples << " au tests per tree." << std::endl;
    std::vector<std::vector<double> > all_p_values = {};
    TreeStatistics raxml_statistics;

    for (unsigned int s = 0; s < samples; s++) {
        all_p_values.emplace_back();

        fixture->Bootstrap();
        fixture->ComputeDeltas();

        std::vector<double> p_values = {};
        fixture->ComputePValues(all_p_values[s]);

        fixture->ResetStatistics();
    }

    for (unsigned int t = 0; t < fixture->trees; t++) {
        double mean = 0;
        for (unsigned int s = 0; s < samples; s++) {
            mean += all_p_values[s][t];
        }

        mean /= samples;

        double variance = 0;
        for (unsigned int s = 0; s < samples; s++) {
            variance += (mean - all_p_values[s][t]) * (mean - all_p_values[s][t]);
        }

        variance /= samples;

        raxml_statistics.mean.push_back(mean);
        raxml_statistics.variance.push_back(variance);
    }

    reject_hypotheses(reference_statistics, raxml_statistics, samples);
}

TEST_F(AuTestFixture, DISABLED_garbage) {
    run_fixture(this, "autest/garbage.siteLH", "autest/garbage.reference");
}

TEST_F(AuTestFixture, DISABLED_implausible16) {
    run_fixture(this, "autest/implausible16.siteLH", "autest/implausible16.reference");
}

TEST_F(AuTestFixture, DISABLED_optimization) {
    run_fixture(this, "autest/optimization.siteLH", "autest/optimization.reference");
}

TEST_F(AuTestFixture, DISABLED_plausible5) {
    run_fixture(this, "autest/plausible5.siteLH", "autest/plausible5.reference");
}

TEST_F(AuTestFixture, DISABLED_random6) {
    run_fixture(this, "autest/random6.siteLH", "autest/random6.reference");
}

TEST_F(AuTestFixture, DISABLED_tool_comparison) {
    run_fixture(this, "autest/tool_comparison.siteLH", "autest/tool_comparison.reference");
}
