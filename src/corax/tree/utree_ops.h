#ifndef CORAX_TREE_UTREE_OPS_H_
#define CORAX_TREE_UTREE_OPS_H_

#include "corax/corax_common.h"

#define PLLMOD_TREE_ERROR_POLYPHYL_OUTGROUP 3970 // B + {10...}

PLL_EXPORT void pll_utree_set_length(pll_unode_t *edge, double length);

PLL_EXPORT void pll_utree_set_length_recursive(pll_utree_t *tree,
                                               double       length,
                                               int          missing_only);

PLL_EXPORT void pll_utree_scale_branches(pll_utree_t *tree,
                                         double       branch_length_scaler);

PLL_EXPORT void pll_utree_scale_branches_all(pll_unode_t *root,
                                             double       branch_length_scaler);

PLL_EXPORT void pll_utree_scale_subtree_branches(pll_unode_t *root,
                                                 double branch_length_scaler);

PLL_EXPORT int pll_utree_collapse_branches(pll_utree_t *tree, double min_brlen);

PLL_EXPORT pll_unode_t *pll_utree_unroot_inplace(pll_unode_t *root);

PLL_EXPORT int pll_utree_root_inplace(pll_utree_t *tree);

PLL_EXPORT int pll_utree_outgroup_root(pll_utree_t * tree,
                                       unsigned int *outgroup_tip_ids,
                                       unsigned int  outgroup_size,
                                       int           add_root_node);

PLL_EXPORT int pllmod_utree_draw_support(pll_utree_t * ref_tree,
                                         const double *support,
                                         pll_unode_t **node_map,
                                         char *(*cb_serialize)(double));

#endif /* CORAX_TREE_UTREE_OPS_H_ */
