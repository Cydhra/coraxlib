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
#include "environment.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <numeric>

using testing::DoubleNear;
using testing::Each;

TEST(coraxlib_freerate_em, test1) {
    const auto sites = 600; // only take a subset of sites

    corax_msa_t* msa = corax_phylip_load(env->msa_filename().c_str(), CORAX_TRUE);
    ASSERT_NE(msa, nullptr);
    ASSERT_LT(sites, msa->length);

    auto tree = corax_utree_random_create(msa->count, msa->label, 42);
    ASSERT_NE(tree, nullptr);
    ASSERT_EQ(msa->count, tree->tip_count);

    constexpr auto rate_cats = 3;
    auto part = corax_partition_create(tree->tip_count, tree->inner_count, 4, sites, 1, tree->edge_count, rate_cats, tree->inner_count, CORAX_ATTRIB_ARCH_AVX);

    std::array<double, 4> frequencies; frequencies.fill(0.25);
    std::array<double, 6> subst_params; subst_params.fill(1);
    std::array<unsigned int, rate_cats> params_indices; params_indices.fill(0);
    std::array<double, rate_cats> category_rates; category_rates.fill(0);
    std::array<double, rate_cats> category_weights; category_weights.fill(1.0 / rate_cats);

    corax_compute_gamma_cats(1.0, rate_cats, category_rates.data(), CORAX_GAMMA_RATES_MEAN);

    corax_set_frequencies(part, 0, frequencies.data());
    corax_set_subst_params(part, 0, subst_params.data());
    corax_set_category_rates(part, category_rates.data());
    corax_set_category_weights(part, category_weights.data());

    for (auto i = 0; i < msa->count; ++i) {
        corax_set_tip_states(part, i, corax_map_nt, msa->sequence[i]);
    }

    auto treeinfo = corax_treeinfo_create(tree->vroot, tree->tip_count, 1, CORAX_BRLEN_LINKED);

    corax_treeinfo_init_partition(treeinfo, 0, part, CORAX_OPT_PARAM_RATE_WEIGHTS | CORAX_OPT_PARAM_FREE_RATES, CORAX_GAMMA_RATES_MEAN, 1.0, params_indices.data(), NULL);

    double initial_loglh = corax_treeinfo_compute_loglh(treeinfo, 0);
    ASSERT_LT(initial_loglh, 0);

    double loglh_after_bfgs = -corax_algo_opt_rates_weights_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4);
    EXPECT_GT(loglh_after_bfgs, initial_loglh + 10);


    // Restore rates and branch lengths
    const auto reset_treeinfo = [treeinfo, &category_rates, &category_weights, &part, &tree]() {
        corax_set_category_rates(part, category_rates.data());
        part->prop_invar[0] = 0.0;
        corax_set_category_weights(part, category_weights.data());
        for (unsigned int i = 0; i < tree->edge_count; ++i) {
            corax_treeinfo_set_branch_length(treeinfo, tree->nodes[i], 0.1);
        }
        return corax_treeinfo_compute_loglh(treeinfo, 0);
    };

    ASSERT_EQ(initial_loglh, reset_treeinfo());

    // Optimize rates with EM algorithm (need multiple rounds of optimization when using Brent)
    double loglh_after_em;
    for (auto iteration = 0U; iteration < 3; ++iteration) {
        loglh_after_em = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4);
    }

    EXPECT_GT(loglh_after_em, initial_loglh);
    EXPECT_NEAR(loglh_after_em, loglh_after_bfgs, 0.5);


    ASSERT_EQ(initial_loglh, reset_treeinfo());

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
        printf("after bfgs invar: %f\n", loglh_after_bfgs_invar);
    } while(loglh_after_bfgs_invar - old_loglh > 1e-3);
    EXPECT_GT(loglh_after_bfgs_invar, initial_loglh);

    do {
        old_loglh = loglh_after_em_invar;
        loglh_after_em_invar = -corax_algo_opt_onedim_treeinfo(treeinfo,
                                                          CORAX_OPT_PARAM_PINV,
                                                          CORAX_OPT_MIN_PINV,
                                                          CORAX_OPT_MAX_PINV,
                                                          1e-4);
        loglh_after_em_invar = -corax_algo_opt_rates_weights_em_treeinfo(treeinfo, CORAX_OPT_MIN_RATE, CORAX_OPT_MAX_RATE, CORAX_OPT_MIN_BRANCH_LEN, CORAX_OPT_MAX_BRANCH_LEN, 0, 1e-4);
        printf("after em invar: %f\n", loglh_after_em_invar);
    } while(loglh_after_em_invar - old_loglh > 1e-3);
    EXPECT_GT(loglh_after_em_invar, initial_loglh);

    EXPECT_NEAR(loglh_after_em_invar, loglh_after_bfgs_invar, 0.5);


    corax_partition_destroy(part);
    corax_treeinfo_destroy(treeinfo);

    corax_utree_destroy(tree, NULL);

    corax_msa_destroy(msa);
}

TEST(coraxlib_freerate_em, persitecat_lh) {
    corax_msa_t* msa = corax_phylip_load(env->msa_filename().c_str(), CORAX_TRUE);
    ASSERT_NE(msa, nullptr);

    auto tree = corax_utree_random_create(msa->count, msa->label, 42);
    ASSERT_NE(tree, nullptr);
    ASSERT_EQ(msa->count, tree->tip_count);

    constexpr auto rate_cats = 3;

    std::array<double, 4> frequencies; frequencies.fill(0.25);
    std::array<double, 6> subst_params; subst_params.fill(1e-9); subst_params.at(5) = 1.0;
    std::array<unsigned int, rate_cats> params_indices; params_indices.fill(0);
    std::array<double, rate_cats> category_rates; category_rates.fill(0);
    std::array<double, rate_cats> category_weights; category_weights.fill(1.0 / rate_cats);

    corax_compute_gamma_cats(1.0, rate_cats, category_rates.data(), CORAX_GAMMA_RATES_MEAN);


    const std::vector<unsigned int> attrib_simd_levels {0, CORAX_ATTRIB_ARCH_SSE, CORAX_ATTRIB_ARCH_AVX, CORAX_ATTRIB_ARCH_AVX2, CORAX_ATTRIB_ARCH_AVX512};
    const std::vector<unsigned int> attrib_tree_opt {0, CORAX_ATTRIB_PATTERN_TIP, CORAX_ATTRIB_SITE_REPEATS};
    const std::vector<unsigned int> attrib_scaling {0}; //, CORAX_ATTRIB_RATE_SCALERS};
    const std::vector<double> prop_invar_vals {0.0, 0.3};

    std::vector<double> scale_minlh(CORAX_SCALE_RATE_MAXDIFF, CORAX_SCALE_THRESHOLD);
    for (auto i = 1U; i < scale_minlh.size(); ++i) {
        scale_minlh[i] = scale_minlh[i-1] * CORAX_SCALE_THRESHOLD;
    }

    // Test all variants of optimizations 
    for (auto scaling : attrib_scaling) {
    for (auto prop_invar : prop_invar_vals) {
        // TODO: figure out why 1e18 crashes on per-rate scalers
        const double brlen = scaling & CORAX_ATTRIB_RATE_SCALERS ? 1e14 : 1e18;

        std::vector<double> lnLs;
        for (auto simd : attrib_simd_levels) {
        for (auto tree_opt : attrib_tree_opt) {
            const auto attrib = simd | tree_opt | scaling;
            printf("Testing %u\n", attrib);

            auto part = corax_partition_create(tree->tip_count, tree->inner_count, 4, msa->length, 1, tree->edge_count, rate_cats, tree->inner_count, attrib);
            corax_set_frequencies(part, 0, frequencies.data());
            corax_set_subst_params(part, 0, subst_params.data());
            corax_set_category_rates(part, category_rates.data());
            corax_set_category_weights(part, category_weights.data());

            for (auto i = 0; i < msa->count; ++i) {
                corax_set_tip_states(part, i, corax_map_nt, msa->sequence[i]);
            }

            EXPECT_EQ(part->prop_invar[0], 0.0);
            part->prop_invar[0] = prop_invar;
            corax_update_invariant_sites(part);

            auto treeinfo = corax_treeinfo_create(tree->vroot, tree->tip_count, 1, CORAX_BRLEN_LINKED);

            corax_treeinfo_init_partition(treeinfo, 0, part, 0, CORAX_GAMMA_RATES_MEAN, 1.0, params_indices.data(), NULL);

            // Extremely long branches to provoke scaling
            for (unsigned int i = 0; i < tree->edge_count; ++i) {
                corax_treeinfo_set_branch_length(treeinfo, tree->nodes[i], brlen);
            }


            // Compute LogLH multiple times with different kernels
            std::vector<double> persitecat_lh(rate_cats * msa->length, 0.0);
            double *persitecat_lh_per_part = persitecat_lh.data();

            const double lh = corax_treeinfo_compute_loglh_sitecat(treeinfo, 0, 1, &persitecat_lh_per_part);
            printf("\t lnL = %f\n", lh);
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
                const unsigned int site_scalings = 
                            part->scale_buffer[treeinfo->root->scaler_index][CORAX_GET_ID(parent_site_id, i)] +
                            part->scale_buffer[treeinfo->root->back->scaler_index][CORAX_GET_ID(child_site_id, i)];
                double site_lnL;
                if (term_inv > 0.0) {
                    site_lnL = log(site_lh * scale_minlh.at(std::min(site_scalings,  static_cast<unsigned int>(CORAX_SCALE_RATE_MAXDIFF -1))) \
                                   + term_inv);

                } else {
                    site_lnL = log(site_lh) + site_scalings * log(CORAX_SCALE_THRESHOLD);
                }
                summed_lh += site_lnL * part->pattern_weights[i];
            }
            EXPECT_NEAR(lh, summed_lh, 1e-24);
            lnLs.push_back(summed_lh);



            corax_partition_destroy(part);
            corax_treeinfo_destroy(treeinfo);
        }
        }

        EXPECT_GE(lnLs.size(), 2);
        EXPECT_THAT(lnLs, Each(DoubleNear(lnLs[0], 1e-3)));
    }
    }

    corax_utree_destroy(tree, NULL);

    corax_msa_destroy(msa);
}
