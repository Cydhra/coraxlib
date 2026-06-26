#include "corax/corax.h"

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "../../../src/corax/statistics/bootstrap.h"
#include "../../../src/corax/statistics/au.h"
#include "../../../src/corax/util/random.h"

#include <stdexcept>
#include <cmath>
#include <sstream>

const std::vector<double> AU_DEFAULT_SCALES = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4};
const std::vector<unsigned int> AU_DEFAULT_REPS = {
    10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000
};

struct TreeStatistics {
    std::vector<double> mean;
    std::vector<double> variance;
};

class AuTestFixture : public testing::Test {
public:
    unsigned int trees = 0, sites = 0;
    unsigned int samples = 0;
    corax_random_state *rstate = nullptr;
    const double *const *trees_persite_lnl = nullptr;
    double **test_statistics = nullptr;
    TreeStatistics reference_statistics;

    bool LoadSiteLH(const char *filename, const char* reference) {
        std::ifstream file(filename);
        if (!file) {
            return false;
        }

        file >> trees >> sites;
        std::cout << "Loading " << trees << " trees with " << sites << " sites." << std::endl;

        auto data = new double *[trees];
        for (unsigned int t = 0; t < trees; ++t) {
            data[t] = new double[sites];
        }

        std::string label;
        for (unsigned int t = 0; t < trees; ++t) {
            file >> label; // skip label until whitespace

            for (unsigned int s = 0; s < sites; ++s) {
                file >> data[t][s];
            }
        }

        trees_persite_lnl = const_cast<const double *const *>(data);

        std::ifstream ref_data(reference);
        if (!ref_data) {
            return false;
        }

        ref_data >> samples;

        for (unsigned int s = 0; s < trees; ++s) {
            double mean;
            ref_data >> mean;
            reference_statistics.mean.push_back(mean);
        }

        for (unsigned int s = 0; s < trees; ++s) {
            double variance;
            ref_data >> variance;
            reference_statistics.variance.push_back(variance);
        }

        return true;
    }

    void Bootstrap() {
        auto site_weights = new unsigned int[sites];
        for (unsigned int i = 0; i < sites; ++i) {
            site_weights[i] = 1;
        }

        corax_RELL_multiscale_bootstrap(rstate, &test_statistics, trees_persite_lnl, site_weights, sites, sites, trees,
                                        AU_DEFAULT_REPS.data(), AU_DEFAULT_SCALES.data(), AU_DEFAULT_SCALES.size());

        delete[] site_weights;
    }

    void ComputeDeltas() {
        for (unsigned int id_scale = 0; id_scale < AU_DEFAULT_SCALES.size(); id_scale++) {
            corax_normalize_lnl_bootstrap(test_statistics[id_scale], test_statistics[id_scale],
                                          AU_DEFAULT_REPS[id_scale], trees);
        }
    }

    void ComputePValues(std::vector<double> &p_values) {
        constexpr unsigned int SCALE_CLOSEST_TO_ONE = 5;

        for (unsigned int tree = 0; tree < trees; tree++) {
            double d, c;
            double p_value = 0.0;
            corax_au_p_value(test_statistics,
                             tree,
                             AU_DEFAULT_SCALES.data(),
                             AU_DEFAULT_REPS.data(),
                             AU_DEFAULT_SCALES.size(),
                             corax_bootstrap_expectation(test_statistics[SCALE_CLOSEST_TO_ONE], AU_DEFAULT_REPS[SCALE_CLOSEST_TO_ONE], tree),
                             &d,
                             &c,
                             &p_value);

            p_values.push_back(p_value);
        }
    }

    void ResetStatistics() {
        for (unsigned int scale = 0; scale < AU_DEFAULT_SCALES.size(); scale++) {
            memset(test_statistics[scale], 0, sizeof(double) * trees * AU_DEFAULT_REPS[scale]);
        }
    }

    void SetUp() override {
        rstate = corax_random_create(1);
    }

    void TearDown() override {
        for (unsigned int t = 0; t < trees; ++t) {
            delete[] trees_persite_lnl[t];
        }
        delete[] trees_persite_lnl;
        corax_random_destroy(rstate);
    }
};

// generated approximation of the quantile function of students-t-distribution because C++ doesn't have that
static double t_critical_0_995(int df) {
    struct Point { int df; double t; };

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
    const TreeStatistics& reference_statistics,
    const TreeStatistics& testing_statistics,
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
            unrejected.push_back({lower_bound_rejected, upper_bound_rejected,
                                  i,
                                  reference_statistics.mean[i],
                                  reference_statistics.variance[i],
                                  testing_statistics.mean[i],
                                  testing_statistics.variance[i]});
        } else {
            std::cout << "Rejected inequality: tree " << i << " is not significantly better or worse "
                << "(consel mean: " << reference_statistics.mean[i] << ", variance: "
                << reference_statistics.variance[i] << "; corax mean: "
            << testing_statistics.mean[i] << ", variance: " << testing_statistics.variance[i] << ")" << std::endl;
        }
    }

    std::ostringstream oss;
    oss << "failed to reject inequality hypotheses for " << unrejected.size() << " trees" << std::endl;
    for (unsigned int i = 0; i < unrejected.size(); i++) {
        oss << "tree " << unrejected[i].i << " failed to reject "
        << (unrejected[i].lower ? (unrejected[i].upper ? "both bounds" : "the lower bound") : "the upper bound") << ". "
        << "(consel mean: " << unrejected[i].ref_mean << ", variance: "
        << unrejected[i].ref_var << "; corax mean: "
        << unrejected[i].test_mean << ", variance: " << unrejected[i].test_var << ")" << std::endl;
    }

    ASSERT_EQ(unrejected.size(), 0) << oss.str();
}

void run_fixture(AuTestFixture *fixture, std::string site_lh, std::string reference_file) {
    std::string filename = env->datapath(site_lh);
    std::string refname = env->datapath(reference_file);
    ASSERT_TRUE(fixture->LoadSiteLH(filename.c_str(), refname.c_str()));

    std::cout << "Running " << fixture->samples << " au tests per tree." << std::endl;

    std::vector<std::vector<double>> all_p_values = {};
    TreeStatistics raxml_statistics;

    for (unsigned int s = 0; s < fixture->samples; s++) {
        all_p_values.emplace_back();

        fixture->Bootstrap();
        fixture->ComputeDeltas();

        std::vector<double> p_values = {};
        fixture->ComputePValues(all_p_values[s]);

        fixture->ResetStatistics();
    }

    for (unsigned int t = 0; t < fixture->trees; t++) {
        double mean = 0;
        for (unsigned int s = 0; s < fixture->samples; s++) {
            mean += all_p_values[s][t];
        }

        mean /= fixture->samples;

        double variance = 0;
        for (unsigned int s = 0; s < fixture->samples; s++) {
            variance += (mean - all_p_values[s][t]) * (mean - all_p_values[s][t]);
        }

        variance /= fixture->samples;

        raxml_statistics.mean.push_back(mean);
        raxml_statistics.variance.push_back(variance);
    }

    reject_hypotheses(fixture->reference_statistics, raxml_statistics, fixture->samples);

    ASSERT_EQ(1.0, 1.0);
}

TEST_F(AuTestFixture, garbage) {
    run_fixture(this, "autest/garbage.siteLH", "autest/garbage.reference");
}
