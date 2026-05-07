#include "fixtures.hpp"
#include "corax/core/common.h"
#include "corax/core/partition.h"
#include "corax/model/invariant.h"
#include "corax/optimize/opt_generic.h"
#include "corax/optimize/opt_treeinfo.h"
#include "corax/tree/treeinfo.h"
#include "corax/tree/utree.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::Combine;
using testing::Values;


using SitecatTestConfig = std::tuple<unsigned int, unsigned int, unsigned int, double>;
class SitecatTest : public testing::WithParamInterface<SitecatTestConfig>, public SinglePartitionedTest { };

void place_vroot_at_tip(corax_treeinfo_t *treeinfo)
{
    auto new_vroot = treeinfo->tree->nodes[0];
    ASSERT_TRUE(CORAX_UTREE_IS_TIP(new_vroot))
        << "Please adapt vroot placement to tip node, to ensure tip inner optimization is actually called";

    auto parent_clv_index = new_vroot->clv_index;
    auto child_clv_index = new_vroot->back->clv_index;
    ASSERT_TRUE((parent_clv_index < treeinfo->tip_count) || (child_clv_index < treeinfo->tip_count));

    corax_treeinfo_set_root(treeinfo, new_vroot);
}

TEST_P(SitecatTest, dna_persitecat_lh) {
    const auto simd_attributes = std::get<0>(GetParam());
    const auto opt_attributes = std::get<1>(GetParam());
    const auto scaling_attributes = std::get<2>(GetParam());
    const auto p_inv = std::get<3>(GetParam());
    const auto attributes = simd_attributes | opt_attributes | scaling_attributes;

    constexpr auto rate_cats = 3;
    SetupDNA();
    CreateTreeinfo(attributes, rate_cats);

    std::array<double, 4> frequencies {0.25, 0.25, 0.25, 0.25};
    std::array<double, 6> subst_params {1e-9, 1e-9, 1e-9, 1e-9, 1e-9, 1.0};
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

    // Update virtual root so that tip-inner optimimzation can actually be applied
    if (attributes & CORAX_ATTRIB_PATTERN_TIP) {
        place_vroot_at_tip(treeinfo);
    }

    const double lh = corax_treeinfo_compute_loglh_sitecat(treeinfo, 0, 1, &persitecat_lh_per_part);
    RecordProperty("loglh", lh);
    DBG("\t lnL = %f\n", lh);
    EXPECT_LT(lh, 0);
    EXPECT_GT(lh, -1e15);

    // Check that sitecat lh is correct by summing per-cat lh
    double summed_lh = 0.;
    for (auto i = 0U; i < part->sites; ++i) {
        double terma = 0., term_inv = 0.;

        if (part->invariant != nullptr) {
            const auto site_state = part->invariant[i];

            for (auto c = 0U; site_state != -1 && c < part->rate_cats; ++c) {

                term_inv += part->prop_invar[0] \
                            * part->rate_weights[c] \
                            * part->frequencies[treeinfo->param_indices[0][c]][site_state];
            }
        }

        unsigned int site_scalings = UINT_MAX;
        for (auto c = 0U; c < part->rate_cats; ++c) {
            site_scalings = CORAX_MIN(corax_retrieve_root_edge_scalings(treeinfo, part, i, c), site_scalings);
        }

        for (auto c = 0U; c < part->rate_cats; ++c) {
            const unsigned int category_scalings = corax_retrieve_root_edge_scalings(treeinfo, part, i, c) - site_scalings;
            const double terma_r = (1 - part->prop_invar[0]) * part->rate_weights[c] * persitecat_lh[i * part->rate_cats + c];
            const auto scaler = pow(CORAX_SCALE_THRESHOLD, CORAX_MIN(CORAX_SCALE_RATE_MAXDIFF, category_scalings));
            terma += scaler * terma_r;
        }

        double site_lnL;
        if (term_inv > 0.0) {
            const auto capped_scalings = std::min(site_scalings,  static_cast<unsigned int>(CORAX_SCALE_RATE_MAXDIFF));
            const auto scale_factor = site_scalings > 0 ? scale_minlh.at(capped_scalings - 1) : 1.0;
            site_lnL = log(terma * scale_factor + term_inv);
        } else {
            site_lnL = log(terma) + site_scalings * log(CORAX_SCALE_THRESHOLD);
        }
        summed_lh += site_lnL * part->pattern_weights[i];
    }

    EXPECT_DOUBLE_EQ(lh, summed_lh);
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

    // Update virtual root so that tip-inner optimimzation can actually be applied
    if (attributes & CORAX_ATTRIB_PATTERN_TIP) {
        place_vroot_at_tip(treeinfo);
    }

    const double lh = corax_treeinfo_compute_loglh_sitecat(treeinfo, 0, 1, &persitecat_lh_per_part);
    DBG("\t lnL = %f\n", lh);
    EXPECT_LT(lh, 0);
    EXPECT_GT(lh, -1e15);

    // Check that sitecat lh is correct by summing per-cat lh
    double summed_lh = 0.;
    for (auto i = 0U; i < part->sites; ++i) {
        double terma = 0., term_inv = 0.;

        if (part->invariant != nullptr) {
            const auto site_state = part->invariant[i];

            for (auto c = 0U; site_state != -1 && c < part->rate_cats; ++c) {

                term_inv += part->prop_invar[0] \
                            * part->rate_weights[c] \
                            * part->frequencies[treeinfo->param_indices[0][c]][site_state];
            }
        }

        unsigned int site_scalings = UINT_MAX;
        for (auto c = 0U; c < part->rate_cats; ++c) {
            site_scalings = CORAX_MIN(corax_retrieve_root_edge_scalings(treeinfo, part, i, c), site_scalings);
        }

        for (auto c = 0U; c < part->rate_cats; ++c) {
            const unsigned int category_scalings = corax_retrieve_root_edge_scalings(treeinfo, part, i, c) - site_scalings;
            const double terma_r = (1 - part->prop_invar[0]) * part->rate_weights[c] * persitecat_lh[i * part->rate_cats + c];
            const auto scaler = pow(CORAX_SCALE_THRESHOLD, CORAX_MIN(CORAX_SCALE_RATE_MAXDIFF, category_scalings));
            terma += scaler * terma_r;
        }

        double site_lnL;
        if (term_inv > 0.0) {
            const auto capped_scalings = std::min(site_scalings,  static_cast<unsigned int>(CORAX_SCALE_RATE_MAXDIFF));
            const auto scale_factor = site_scalings > 0 ? scale_minlh.at(capped_scalings - 1) : 1.0;
            site_lnL = log(terma * scale_factor + term_inv);
        } else {
            site_lnL = log(terma) + site_scalings * log(CORAX_SCALE_THRESHOLD);
        }
        summed_lh += site_lnL * part->pattern_weights[i];
    }

    EXPECT_DOUBLE_EQ(lh, summed_lh);
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

    double loglh_after_bfgs = -corax_algo_opt_rates_weights_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 0.001, 1e-4);
    EXPECT_GT(loglh_after_bfgs, initial_loglh + 10);
    RecordProperty("loglh_after_bfgs", loglh_after_bfgs);


    ASSERT_EQ(initial_loglh, reset_treeinfo());

    // Optimize rates with EM algorithm (need multiple rounds of optimization when using Brent)
    double loglh_after_em;
    for (auto iteration = 0U; iteration < 3; ++iteration) {
        loglh_after_em = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 0.001, 1e-4, true);
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
        loglh_after_bfgs_invar = -corax_algo_opt_rates_weights_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 0.001, 1e-4);
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
        loglh_after_em_invar = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 0.001, 1e-4, true);
        DBG("after em invar: %f\n", loglh_after_em_invar);
    } while(loglh_after_em_invar - old_loglh > 1e-3);
    RecordProperty("loglh_after_em", loglh_after_em_invar);
    EXPECT_GT(loglh_after_em_invar, initial_loglh);

    EXPECT_NEAR(loglh_after_em_invar, loglh_after_bfgs_invar, 1e-2);
}
