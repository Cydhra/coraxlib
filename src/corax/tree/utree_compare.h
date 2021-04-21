#ifndef CORAX_TREE_UTREE_COMPARE_H_
#define CORAX_TREE_UTREE_COMPARE_H_

#include "corax/corax_common.h"
#include "utree_split.h"

typedef struct split_system_t
{
  pll_split_t *splits;
  double *     support;
  unsigned int split_count;
  double       max_support;
} pll_split_system_t;

typedef struct consensus_data_t
{
  pll_split_t  split;
  unsigned int bit_count;
  double       support;
} pll_consensus_data_t;

typedef struct consensus_utree_t
{
  pll_unode_t *         tree;
  pll_consensus_data_t *branch_data;
  unsigned int          tip_count;
  unsigned int          branch_count;
} pll_consensus_utree_t;

/* check that node ids and tip labels agree in both trees */
PLL_EXPORT int pllmod_utree_consistency_check(pll_utree_t *t1, pll_utree_t *t2);

/* if 2 different trees are parsed from newick node ids might have been set
   in a different order, so this function sets node ids in t2 such that
   node ids and tip labels agree in both trees */
PLL_EXPORT int pllmod_utree_consistency_set(pll_utree_t *t1, pll_utree_t *t2);

PLL_EXPORT unsigned int pllmod_utree_rf_distance(pll_unode_t *t1,
                                                 pll_unode_t *t2,
                                                 unsigned int tip_count);

PLL_EXPORT pll_consensus_utree_t *
           pllmod_utree_from_splits(const pll_split_system_t *split_system,
                                    unsigned int              tip_count,
                                    char *const *             tip_labels);

PLL_EXPORT pll_split_system_t *pllmod_utree_split_consensus(
    bitv_hashtable_t *splits_hash, unsigned int tip_count, double threshold);

PLL_EXPORT pll_consensus_utree_t *
           pllmod_utree_weight_consensus(pll_utree_t *const *trees,
                                         const double *      weights,
                                         double              threshold,
                                         unsigned int        tree_count);

// TODO: implement Newick->splits parser
#if 0
PLL_EXPORT pll_consensus_utree_t * pllmod_utree_consensus(
                                                    const char * trees_filename,
                                                    double threshold,
                                                    unsigned int * tree_count);
#endif

PLL_EXPORT void
pllmod_utree_split_system_destroy(pll_split_system_t *split_system);

PLL_EXPORT void pllmod_utree_consensus_destroy(pll_consensus_utree_t *tree);

#endif /* CORAX_TREE_UTREE_COMPARE_H_ */
