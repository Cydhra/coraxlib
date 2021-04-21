#ifndef CORAX_TREE_UTREE_TBE_H_
#define CORAX_TREE_UTREE_TBE_H_

#include "corax/corax_common.h"

typedef struct refsplit_info
{
  unsigned int p;
  bool         subtree_res;
  unsigned int left_leaf_idx;
  unsigned int right_leaf_idx;
} pllmod_tbe_split_info_t;

PLL_EXPORT
pllmod_tbe_split_info_t *
pllmod_utree_tbe_nature_init(pll_unode_t *       ref_root,
                             unsigned int        tip_count,
                             const pll_unode_t **split_to_node_map);

/* Compute Transfer Support (Lemoine et al., Nature 2018) for every split in
 * ref_splits. Sarahs implementation of the algorithm from the Nature paper. */
PLL_EXPORT int pllmod_utree_tbe_nature(pll_split_t *            ref_splits,
                                       pll_split_t *            bs_splits,
                                       pll_unode_t *            bs_root,
                                       unsigned int             tip_count,
                                       double *                 support,
                                       pllmod_tbe_split_info_t *split_info);

/* This is an old, naive and rather inefficient TBE computation method by
 * Alexey. Keep it here just in case */
PLL_EXPORT int pllmod_utree_tbe_naive(pll_split_t *ref_splits,
                                      pll_split_t *bs_splits,
                                      unsigned int tip_count,
                                      double *     support);

#endif /* CORAX_TREE_UTREE_TBE_H_ */
