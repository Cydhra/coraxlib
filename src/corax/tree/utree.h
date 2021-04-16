#ifndef CORAX_TREE_UTREE_H_
#define CORAX_TREE_UTREE_H_

#include "corax/corax_common.h"

PLL_EXPORT void pll_utree_destroy(pll_utree_t *tree,
                                  void (*cb_destroy)(void *));

PLL_EXPORT void pll_utree_reset_template_indices(pll_unode_t *node,
                                                 unsigned int tip_count);

PLL_EXPORT void pll_utree_graph_destroy(pll_unode_t *root,
                                        void (*cb_destroy)(void *));

PLL_EXPORT pll_utree_t *pll_utree_wraptree(pll_unode_t *root,
                                           unsigned int tip_count);

PLL_EXPORT pll_utree_t *pll_utree_wraptree_multi(pll_unode_t *root,
                                                 unsigned int tip_count,
                                                 unsigned int inner_count);

PLL_EXPORT pll_unode_t *pll_utree_create_node(unsigned int clv_index,
                                              int          scaler_index,
                                              char *       label,
                                              void *       data);

PLL_EXPORT int pll_unode_is_rooted(const pll_unode_t *root);

PLL_EXPORT int pll_utree_is_rooted(const pll_utree_t *tree);

PLL_EXPORT void pll_utree_create_operations(pll_unode_t *const *trav_buffer,
                                            unsigned int     trav_buffer_size,
                                            double *         branches,
                                            unsigned int *   pmatrix_indices,
                                            pll_operation_t *ops,
                                            unsigned int *   matrix_count,
                                            unsigned int *   ops_count);

PLL_EXPORT int pll_utree_check_integrity(const pll_utree_t *root);

PLL_EXPORT pll_unode_t *pll_utree_graph_clone(const pll_unode_t *root);

PLL_EXPORT pll_utree_t *pll_utree_clone(const pll_utree_t *root);

PLL_EXPORT void pll_utree_create_pars_buildops(pll_unode_t *const *trav_buffer,
                                               unsigned int trav_buffer_size,
                                               pll_pars_buildop_t *ops,
                                               unsigned int *      ops_count);

#endif /* CORAX_TREE_UTREE_H_ */
