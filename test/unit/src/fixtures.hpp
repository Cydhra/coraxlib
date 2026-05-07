#pragma once

#include <corax/corax.h>
#include <gtest/gtest.h>
#include "environment.hpp"

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
        LoadMSA(env->datapath("example.phy").c_str(), state_map);
        CreateRandomTree();
    }

    void SetupAA() {
        states = 20;
        state_map = corax_map_aa;
        LoadMSA(env->datapath("140.phy").c_str(), state_map);
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
