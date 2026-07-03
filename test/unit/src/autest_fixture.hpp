//
// Created by hengstjs on 7/2/2026.
//

#ifndef RAXML_NG_AUTEST_FIXTURE_HPP
#define RAXML_NG_AUTEST_FIXTURE_HPP

#include <fstream>
#include <vector>

#include "../../../src/corax/statistics/bootstrap.h"
#include "../../../src/corax/statistics/au.h"
#include "../../../src/corax/util/random.h"

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
    corax_random_state *rstate = nullptr;
    const double *const *trees_persite_lnl = nullptr;
    double **test_statistics = nullptr;


    bool LoadSiteLH(const char *filename) {
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

#endif //RAXML_NG_AUTEST_FIXTURE_HPP
