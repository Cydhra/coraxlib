/**
 * @file utree.h
 *
 * @brief This header file contains functions related to the corax_utree_t
 * struct
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
 * @ingroup corax_utree_t
 */
CORAX_EXPORT void corax_utree_destroy(corax_utree_t *tree,
                                      void (*cb_destroy)(void *));

CORAX_EXPORT void corax_utree_reset_template_indices(corax_unode_t *node,
                                                     unsigned int   tip_count);

CORAX_EXPORT void corax_utree_graph_destroy(corax_unode_t *root,
                                            void (*cb_destroy)(void *));

/**
 * Takes a tree, represented by a node, and optionally a tip count. Will produce
 * a corax_utree_t that contains that tree. The pointer to the original node is
 * not invalidated.
 *
 * @param root Pointer to the virtual root. Should be an "inner node".
 *
 * @param tip_count Number of tips in contained in the tree represented by
 * `root`
 *
 * @ingroup corax_utree_t
 */
CORAX_EXPORT corax_utree_t *corax_utree_wraptree(corax_unode_t *root,
                                                 unsigned int   tip_count);

CORAX_EXPORT corax_utree_t *corax_utree_wraptree_multi(
    corax_unode_t *root, unsigned int tip_count, unsigned int inner_count);

CORAX_EXPORT corax_unode_t *corax_utree_create_node(unsigned int clv_index,
                                                    int          scaler_index,
                                                    char *       label,
                                                    void *       data);

CORAX_EXPORT int corax_unode_is_rooted(const corax_unode_t *root);

CORAX_EXPORT int corax_utree_is_rooted(const corax_utree_t *tree);

/**
 * Given the `corax_unode_t**` from a traversal using `corax_utree_traverse`,
 * this will create a list of `corax_operation_t`.
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
 * @ingroup corax_utree_t
 * @ingroup corax_operation_t
 */
CORAX_EXPORT void
corax_utree_create_operations(const corax_unode_t *const *trav_buffer,
                              unsigned int                trav_buffer_size,
                              double *                    branches,
                              unsigned int *              pmatrix_indices,
                              corax_operation_t *         ops,
                              unsigned int *              matrix_count,
                              unsigned int *              ops_count);

CORAX_EXPORT int corax_utree_check_integrity(const corax_utree_t *root);

CORAX_EXPORT corax_unode_t *corax_utree_graph_clone(const corax_unode_t *root);

/**
 * Clone a tree. This is a semi-deep copy. The fields `label` and the pointers
 * `next` and `back` are deep copied, but the data field is shallowly copied.
 *
 * @param root The tree to clone.
 */
CORAX_EXPORT corax_utree_t *corax_utree_clone(const corax_utree_t *root);

CORAX_EXPORT int corax_utree_set_clv_minimal(corax_unode_t *root,
                                             unsigned int   tip_count);

#endif /* CORAX_TREE_UTREE_H_ */
