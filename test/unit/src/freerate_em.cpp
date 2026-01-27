#include "corax/corax.h"
#include "corax/core/common.h"
#include "corax/core/partition.h"
#include "corax/core/repeats.h"
#include "corax/io/utree_io.h"
#include "corax/model/gamma.h"
#include "corax/model/invariant.h"
#include "corax/optimize/opt_generic.h"
#include "corax/optimize/opt_treeinfo.h"
#include "corax/tree/treeinfo.h"
#include "corax/tree/utree.h"
#include "corax/tree/utree_traverse.h"
#include "corax/util/compress.h"
#include "environment.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <numeric>

using testing::DoubleNear;
using testing::Each;
using testing::Combine;
using testing::Values;

class SinglePartitionedTest : public testing::Test {
protected:
    corax_msa_t *msa;
    corax_utree_t *tree;
    unsigned int *pattern_weights;
    unsigned int states;
    corax_state_t const *state_map;

    corax_treeinfo_t *treeinfo;
    corax_partition_t *part;

public:
    void LoadMSA(const char *filename, corax_state_t const *state_map) {
        msa = corax_phylip_load(filename, CORAX_TRUE);

        if (msa == nullptr) {
            msa = corax_phylip_load(filename, CORAX_FALSE);
        }
        if (msa == nullptr) {
            msa = corax_fasta_load(filename);
        }
        ASSERT_NE(msa, nullptr);

        // Compress MSA
        pattern_weights = corax_compress_site_patterns_msa(msa, state_map, nullptr);
    }

    void CreateRandomTree() {
        // Create random tree
        tree = corax_utree_random_create(msa->count, msa->label, 42);
        ASSERT_NE(tree, nullptr);
        ASSERT_EQ(msa->count, tree->tip_count);
    }

    void ResetDNASubstFreq() {
        std::array<double, 6> subst_params; subst_params.fill(1.0);
        std::array<double, 4> freq; freq.fill(0.25);

        corax_set_subst_params(part, 0, subst_params.data());
        corax_set_frequencies(part, 0, freq.data());
    }

    void SetGammaRates(double alpha = 1.0)  {
        const auto rate_cats = part->rate_cats;
        std::vector<double> category_rates(rate_cats, 0);
        std::vector<double> category_weights(rate_cats, 1.0 / rate_cats);

        corax_compute_gamma_cats(alpha, rate_cats, category_rates.data(), CORAX_GAMMA_RATES_MEAN);
        corax_set_category_rates(part, category_rates.data());
        corax_set_category_weights(part, category_weights.data());
    }

    void CreateTreeinfo(unsigned int attributes, unsigned int rate_cats, unsigned int site_limit = std::numeric_limits<unsigned int>::max()) {
        part = corax_partition_create(tree->tip_count, tree->inner_count, states, std::min(static_cast<unsigned int>(msa->length), site_limit), 1, tree->edge_count, rate_cats, tree->inner_count, attributes);

        for (auto i = 0; i < msa->count; ++i) {
            corax_set_tip_states(part, i, state_map, msa->sequence[i]);
        }
        SetGammaRates();

        treeinfo = corax_treeinfo_create(tree->vroot, tree->tip_count, 1, CORAX_BRLEN_LINKED);
        corax_treeinfo_init_partition(treeinfo, 0, part, 0, CORAX_GAMMA_RATES_MEAN, 1.0, nullptr, nullptr);
    }

    void SetAllBranchLengths(double value) {
        for (unsigned int i = 0; i < tree->edge_count; ++i) {
            corax_treeinfo_set_branch_length(treeinfo, tree->nodes[i], value);
        }
    }

    void SetupDNA() {
        states = 4;
        state_map = corax_map_nt;
        LoadMSA(env->msa_filename().c_str(), state_map);
        CreateRandomTree();
    }

    void SetupAA() {
        states = 20;
        state_map = corax_map_aa;
        LoadMSA(env->aa_msa_filename().c_str(), state_map);
        CreateRandomTree();
    }

    void TearDown() override {
        corax_partition_destroy(part);
        corax_treeinfo_destroy(treeinfo);
        corax_utree_destroy(tree, NULL);
        corax_msa_destroy(msa);
        if (pattern_weights) free(pattern_weights);
    }
};

using SitecatTestConfig = std::tuple<unsigned int, unsigned int, unsigned int, double>;
class SitecatTest : public testing::WithParamInterface<SitecatTestConfig>, public SinglePartitionedTest { };

TEST_P(SitecatTest, dna_persitecat_lh) {
    const auto simd_attributes = std::get<0>(GetParam());
    const auto opt_attributes = std::get<1>(GetParam());
    const auto scaling_attributes = std::get<2>(GetParam());
    const auto p_inv = std::get<3>(GetParam());
    const auto attributes = simd_attributes | opt_attributes | scaling_attributes;

    constexpr auto rate_cats = 3;
    SetupDNA();
    CreateTreeinfo(attributes, rate_cats);

    std::array<double, 4> frequencies; frequencies.fill(0.25);
    std::array<double, 6> subst_params; subst_params.fill(1e-9); subst_params.at(5) = 1.0;
    corax_set_frequencies(part, 0, frequencies.data());
    corax_set_subst_params(part, 0, subst_params.data());

    std::vector<double> scale_minlh(CORAX_SCALE_RATE_MAXDIFF, CORAX_SCALE_THRESHOLD);
    for (auto i = 1U; i < scale_minlh.size(); ++i) {
        scale_minlh[i] = scale_minlh[i-1] * CORAX_SCALE_THRESHOLD;
    }

    // Extremely long branches to provoke scaling
    const double brlen = attributes & CORAX_ATTRIB_RATE_SCALERS ? 1e14 : 1e18;
    SetAllBranchLengths(brlen);

    part->prop_invar[0] = p_inv;
    corax_update_invariant_sites(part);

    std::vector<double> persitecat_lh(rate_cats * msa->length, 0.0);
    double *persitecat_lh_per_part = persitecat_lh.data();

    const double lh = corax_treeinfo_compute_loglh_sitecat(treeinfo, 0, 1, &persitecat_lh_per_part);
    RecordProperty("loglh", lh);
    DBG("\t lnL = %f\n", lh);
    EXPECT_LT(lh, 0);
    EXPECT_GT(lh, -1e15);

    // Check that sitecat lh is correct by summing per-cat lh
    double summed_lh = 0.;
    size_t index = 0;
    for (auto i = 0U; i < part->sites; ++i) {
        double site_lh = 0., term_inv = 0.;
        for (auto c = 0U; c < part->rate_cats; ++c) {
            site_lh += persitecat_lh.at(index++);
        }

        if (part->invariant != NULL) {
            const auto site_state = part->invariant[i];

            for (auto c = 0U; site_state != -1 && c < part->rate_cats; ++c) {

                term_inv += part->prop_invar[0] \
                            * part->rate_weights[c] \
                            * part->frequencies[treeinfo->param_indices[0][c]][site_state];
            }
        }

        const auto parent_site_id = corax_get_site_id(part, treeinfo->root->clv_index);
        const auto child_site_id = corax_get_site_id(part, treeinfo->root->back->clv_index);
        const auto parent_scaler = treeinfo->root->scaler_index;
        const auto child_scaler = treeinfo->root->back->scaler_index;
        EXPECT_NE(parent_scaler, -1); EXPECT_NE(child_scaler, -1);

        unsigned int site_scalings;

        if (part->attributes & CORAX_ATTRIB_RATE_SCALERS) {
            site_scalings = 
                *std::min_element(
                    &part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i) * rate_cats + 0],
                    &part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i) * rate_cats + rate_cats]) + 
                *std::min_element(
                    &part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i) * rate_cats + 0],
                    &part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i) * rate_cats + rate_cats]);
        } else {
            site_scalings = 
                    part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i)] +
                    part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i)];
        }

        double site_lnL;
        if (term_inv > 0.0) {
            const auto capped_scalings = std::min(site_scalings,  static_cast<unsigned int>(CORAX_SCALE_RATE_MAXDIFF));
            const auto scale_factor = site_scalings > 0 ? scale_minlh.at(capped_scalings - 1) : 1.0;
            site_lnL = log(site_lh * scale_factor + term_inv);
        } else {
            site_lnL = log(site_lh) + site_scalings * log(CORAX_SCALE_THRESHOLD);
        }
        summed_lh += site_lnL * part->pattern_weights[i];
    }

    // For some reason, SSE causes higher LH divergence
    const double tolerance = simd_attributes & CORAX_ATTRIB_ARCH_SSE ? 1e-9 : 1e-20;

    EXPECT_NEAR(lh, summed_lh, tolerance);
    RecordProperty("summed_loglh", summed_lh);
}


TEST_P(SitecatTest, aa_persitecat_lh) {
    const auto simd_attributes = std::get<0>(GetParam());
    const auto opt_attributes = std::get<1>(GetParam());
    const auto scaling_attributes = std::get<2>(GetParam());
    const auto p_inv = std::get<3>(GetParam());
    const auto attributes = simd_attributes | opt_attributes | scaling_attributes;

    constexpr auto rate_cats = 3;
    SetupAA();
    CreateTreeinfo(attributes, rate_cats, 400);

    corax_set_frequencies(part, 0, corax_aa_freqs_lg);
    corax_set_subst_params(part, 0, corax_aa_rates_lg);

    std::vector<double> scale_minlh(CORAX_SCALE_RATE_MAXDIFF, CORAX_SCALE_THRESHOLD);
    for (auto i = 1U; i < scale_minlh.size(); ++i) {
        scale_minlh[i] = scale_minlh[i-1] * CORAX_SCALE_THRESHOLD;
    }

    // Extremely long branches to provoke scaling
    SetAllBranchLengths(0.1);

    part->prop_invar[0] = p_inv;
    corax_update_invariant_sites(part);

    std::vector<double> persitecat_lh(rate_cats * msa->length, 0.0);
    double *persitecat_lh_per_part = persitecat_lh.data();

    const double lh = corax_treeinfo_compute_loglh_sitecat(treeinfo, 0, 1, &persitecat_lh_per_part);
    DBG("\t lnL = %f\n", lh);
    EXPECT_LT(lh, 0);
    EXPECT_GT(lh, -1e15);

    // Check that sitecat lh is correct by summing per-cat lh
    double summed_lh = 0.;
    size_t index = 0;
    for (auto i = 0U; i < part->sites; ++i) {
        double site_lh = 0., term_inv = 0.;
        for (auto c = 0U; c < part->rate_cats; ++c) {
            site_lh += persitecat_lh.at(index++);
        }

        if (part->invariant != NULL) {
            const auto site_state = part->invariant[i];

            for (auto c = 0U; site_state != -1 && c < part->rate_cats; ++c) {

                term_inv += part->prop_invar[0] \
                            * part->rate_weights[c] \
                            * part->frequencies[treeinfo->param_indices[0][c]][site_state];
            }
        }

        const auto parent_site_id = corax_get_site_id(part, treeinfo->root->clv_index);
        const auto child_site_id = corax_get_site_id(part, treeinfo->root->back->clv_index);
        const auto parent_scaler = treeinfo->root->scaler_index;
        const auto child_scaler = treeinfo->root->back->scaler_index;
        EXPECT_NE(parent_scaler, -1); EXPECT_NE(child_scaler, -1);
        unsigned int site_scalings;

        if (part->attributes & CORAX_ATTRIB_RATE_SCALERS) {
            site_scalings = 
                *std::min_element(
                    &part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i) * rate_cats + 0],
                    &part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i) * rate_cats + rate_cats]) + 
                *std::min_element(
                    &part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i) * rate_cats + 0],
                    &part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i) * rate_cats + rate_cats]);
        } else {
            site_scalings = 
                    part->scale_buffer[parent_scaler][CORAX_GET_ID(parent_site_id, i)] +
                    part->scale_buffer[child_scaler][CORAX_GET_ID(child_site_id, i)];
        }
        double site_lnL;
        if (term_inv > 0.0) {
            const auto capped_scalings = std::min(site_scalings,  static_cast<unsigned int>(CORAX_SCALE_RATE_MAXDIFF));
            const auto scale_factor = site_scalings > 0 ? scale_minlh.at(capped_scalings - 1) : 1.0;
            site_lnL = log(site_lh * scale_factor + term_inv);

        } else {
            site_lnL = log(site_lh) + site_scalings * log(CORAX_SCALE_THRESHOLD);
        }
        summed_lh += site_lnL * part->pattern_weights[i];
    }
    EXPECT_NEAR(lh, summed_lh, 1e-20);
    RecordProperty("loglh", lh);
    RecordProperty("summed_loglh", summed_lh);

}

INSTANTIATE_TEST_SUITE_P(LogLHCheck, SitecatTest, Combine(
    Values(0, CORAX_ATTRIB_ARCH_SSE, CORAX_ATTRIB_ARCH_AVX, CORAX_ATTRIB_ARCH_AVX2, CORAX_ATTRIB_ARCH_AVX512),
    Values(0, CORAX_ATTRIB_PATTERN_TIP, CORAX_ATTRIB_SITE_REPEATS),
    Values(0 , CORAX_ATTRIB_RATE_SCALERS),
    Values(0.0, 0.3) /* p_invariant */
));

TEST_F(SinglePartitionedTest, em_optimization) {
    const auto sites = 600; // only take a subset of sites
    constexpr auto rate_cats = 3;

    SetupDNA();
    CreateTreeinfo(CORAX_ATTRIB_ARCH_AVX2, rate_cats, sites);

    // Restore rates and branch lengths
    const auto reset_treeinfo = [this]() {
        ResetDNASubstFreq();
        SetGammaRates();
        SetAllBranchLengths(0.1);
        part->prop_invar[0] = 0.0;
        return corax_treeinfo_compute_loglh(treeinfo, 0);
    };

    treeinfo->params_to_optimize[0] = CORAX_OPT_PARAM_FREE_RATES | CORAX_OPT_PARAM_RATE_WEIGHTS;

    const double initial_loglh = reset_treeinfo();
    RecordProperty("initial_loglh", initial_loglh);
    ASSERT_LT(initial_loglh, 0);

    double loglh_after_bfgs = -corax_algo_opt_rates_weights_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4);
    EXPECT_GT(loglh_after_bfgs, initial_loglh + 10);
    RecordProperty("loglh_after_bfgs", loglh_after_bfgs);


    ASSERT_EQ(initial_loglh, reset_treeinfo());

    // Optimize rates with EM algorithm (need multiple rounds of optimization when using Brent)
    double loglh_after_em;
    for (auto iteration = 0U; iteration < 3; ++iteration) {
        loglh_after_em = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4, true);
    }
    RecordProperty("loglh_after_em", loglh_after_em);

    EXPECT_GT(loglh_after_em, initial_loglh);
    EXPECT_NEAR(loglh_after_em, loglh_after_bfgs, 0.5);


    ASSERT_EQ(initial_loglh, reset_treeinfo());
}

TEST_F(SinglePartitionedTest, em_optimization_invar) {
    const auto sites = 600; // only take a subset of sites
    constexpr auto rate_cats = 3;

    SetupDNA();
    CreateTreeinfo(CORAX_ATTRIB_ARCH_AVX2, rate_cats, sites);
    treeinfo->params_to_optimize[0] = CORAX_OPT_PARAM_FREE_RATES | CORAX_OPT_PARAM_RATE_WEIGHTS | CORAX_OPT_PARAM_PINV;

    // Restore rates and branch lengths
    const auto reset_treeinfo = [this]() {
        ResetDNASubstFreq();
        SetGammaRates();
        SetAllBranchLengths(0.1);
        part->prop_invar[0] = 0.0;
        return corax_treeinfo_compute_loglh(treeinfo, 0);
    };
    const double initial_loglh = reset_treeinfo();
    RecordProperty("initial_loglh", initial_loglh);


    // Now with invariant
    double old_loglh;
    double loglh_after_bfgs_invar = -INFINITY, loglh_after_em_invar = -INFINITY;

    do {
        old_loglh = loglh_after_bfgs_invar;
        loglh_after_bfgs_invar = -corax_algo_opt_onedim_treeinfo(treeinfo,
                                                          CORAX_OPT_PARAM_PINV,
                                                          CORAX_OPT_MIN_PINV,
                                                          CORAX_OPT_MAX_PINV,
                                                          1e-4);
        loglh_after_bfgs_invar = -corax_algo_opt_rates_weights_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4);
        DBG("after bfgs invar: %f\n", loglh_after_bfgs_invar);
    } while(loglh_after_bfgs_invar - old_loglh > 1e-3);
    RecordProperty("loglh_after_bfgs", loglh_after_bfgs_invar);
    EXPECT_GT(loglh_after_bfgs_invar, initial_loglh);

    EXPECT_EQ(initial_loglh, reset_treeinfo());

    do {
        old_loglh = loglh_after_em_invar;
        loglh_after_em_invar = -corax_algo_opt_onedim_treeinfo(treeinfo,
                                                          CORAX_OPT_PARAM_PINV,
                                                          CORAX_OPT_MIN_PINV,
                                                          CORAX_OPT_MAX_PINV,
                                                          1e-4);
        loglh_after_em_invar = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4, true);
        DBG("after em invar: %f\n", loglh_after_em_invar);
    } while(loglh_after_em_invar - old_loglh > 1e-3);
    RecordProperty("loglh_after_em", loglh_after_em_invar);
    EXPECT_GT(loglh_after_em_invar, initial_loglh);

    EXPECT_NEAR(loglh_after_em_invar, loglh_after_bfgs_invar, 1e-2);
}
