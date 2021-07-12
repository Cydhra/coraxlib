/**
 * @file utree.h
 *
 * @brief This header file contains functions related to the pll_utree_t struct
 *
 * @author whoever
 */
#ifndef CORAX_TREE_UTREE_H_
#define CORAX_TREE_UTREE_H_

#include "corax/corax_common.h"

/** 
 * Deallocate the memory associated with a utree. `cb_destroy` is used to delete
 * the user data allocated in data.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT void pll_utree_destroy(pll_utree_t *tree,
                                  void (*cb_destroy)(void *));

PLL_EXPORT void pll_utree_reset_template_indices(pll_unode_t *node,
                                                 unsigned int tip_count);

PLL_EXPORT void pll_utree_graph_destroy(pll_unode_t *root,
                                        void (*cb_destroy)(void *));

/**
 * Takes a tree, represented by a node, and optionally a tip count. Will produce
 * a pll_utree_t that contains that tree. The pointer to the original node is
 * not invalidated.
 *
 * @param root Pointer to the virtual root. Should be an "inner node".
 *
 * @param tip_count Number of tips in contained in the tree represented by
 * `root`
 *
 * @ingroup pll_utree_t
 */
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

/**
 * Given the `pll_unode_t**` from a traversal using `pll_utree_traverse`, this
 * will create a list of `pll_operation_t`.
 *
 * @param trav_buffer
 *
 * @param trav_buffer_size
 *
 * @param[out] branches A buffer to store the branch length parameters used in
 * the operations. Optional.
 *
 * @param[out] pmatrix_indices A buffer to store the pmatrix indices used in the
 * operations. Optional.
 *
 * @param[out] ops Buffer to store the created ops. For a full traversal, the
 * allocated size should be equal to the number of the number of branches in the
 * tree.
 *
 * @param[out] matrix_count Out parameter indicating the number matrices
 * required to perform the operations
 *
 * @param[out] ops_count Out parameter indicating the actual number of
 * operations.
 *
 * @ingroup pll_utree_t
 * @ingroup pll_operation_t
 */
PLL_EXPORT void pll_utree_create_operations(pll_unode_t *const *trav_buffer,
                                            unsigned int     trav_buffer_size,
                                            double *         branches,
                                            unsigned int *   pmatrix_indices,
                                            pll_operation_t *ops,
                                            unsigned int *   matrix_count,
                                            unsigned int *   ops_count);

PLL_EXPORT int pll_utree_check_integrity(const pll_utree_t *root);

PLL_EXPORT pll_unode_t *pll_utree_graph_clone(const pll_unode_t *root);

/**
 * Clone a tree. This is a semi-deep copy. The fields `label` and the pointers
 * `next` and `back` are deep copied, but the data field is shallowly copied.
 *
 * @param root The tree to clone.
 */
PLL_EXPORT pll_utree_t *pll_utree_clone(const pll_utree_t *root);

PLL_EXPORT int pll_utree_set_clv_minimal(pll_unode_t *root,
                                         unsigned int tip_count);

#endif /* CORAX_TREE_UTREE_H_ */
