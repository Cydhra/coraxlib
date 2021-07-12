#ifndef CORAX_TREE_UTREE_TRAVERSE_H_
#define CORAX_TREE_UTREE_TRAVERSE_H_

#include "corax/corax_common.h"

/** 
 * Creates a list of nodes from a traversal, starting at root. The order of the
 * traversal can be controlled with the traversal argument, which accepts either
 * `PLL_TREE_TRAVERSE_POSTORDER `or `PLL_TREE_TRAVERSE_PREORDER`. The callback
 * function controls which nodes are traversed by returning true for a node node
 * which should be added to `outbuffer`. If false is returned instead, then the
 * traversal is halted for that subtree, and the node which returned it is not
 * added to `outbuffer`. By doing this the traversal of the tree can be halted
 * early, which is useful for partial traversals. `trav_size `is an out
 * parameter.  Returns `PLL_SUCCESS `on a traversal without errors, and
 * `PLL_FAILURE `if there was an error.
 *
 * If a full traversal is desired, the callback should return true for all
 * inputs.
 *
 * The intended use of this function is to build a traversal buffer, which is to
 * be used in later computations. For example, the `outbuffer `parameter is used
 * to build the operations for likelihood computations (see
 * `pll_utree_create_operations`). The function will fail if `nullptr `is passed
 * in for `outbuffer`. If the behavior of this function is needed, but the
 * buffer is not needed, please use `pll_utree_traverse_apply`.
 *
 * @param root Node which the traversal will start from.
 *
 * @param traversal Type of traversal. Can be `PLL_TREE_TRAVERSE_POSTORDER` or
 * `PLL_TREE_TRAVERSE_PREORDER`
 *
 * @param cbtrav Callback function which can be used to control the traversal.
 * Please see the general function documentation for more detail.
 *
 * @param[out] outbuffer Buffer which will contain the traversed nodes in order.
 *
 * @param[out] trav_size Pointer a buffer which will contain the final traversal
 * size.
 *
 * @return `PLL_SUCCESS` on a traversal without errors. `PLL_FAILURE` otherwise.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT int pll_utree_traverse(pll_unode_t *root,
                                  int          traversal,
                                  int (*cbtrav)(pll_unode_t *),
                                  pll_unode_t **outbuffer,
                                  unsigned int *trav_size);

PLL_EXPORT int pll_utree_traverse_subtree(pll_unode_t *root,
                                          int          traversal,
                                          int (*cbtrav)(pll_unode_t *),
                                          pll_unode_t **outbuffer,
                                          unsigned int *trav_size);
PLL_EXPORT int pll_utree_every(pll_utree_t *tree,
                               int (*cb)(const pll_utree_t *,
                                         const pll_unode_t *));

PLL_EXPORT int pll_utree_every_const(const pll_utree_t *tree,
                                     int (*cb)(const pll_utree_t *tree,
                                               const pll_unode_t *));

PLL_EXPORT int
pll_utree_traverse_apply(pll_unode_t *root,
                         int (*cb_pre_trav)(pll_unode_t *, void *),
                         int (*cb_in_trav)(pll_unode_t *, void *),
                         int (*cb_post_trav)(pll_unode_t *, void *),
                         void *data);

PLL_EXPORT int pll_utree_nodes_at_node_dist(pll_unode_t * node,
                                            pll_unode_t **outbuffer,
                                            unsigned int *node_count,
                                            unsigned int  min_distance,
                                            unsigned int  max_distance);

PLL_EXPORT int pll_utree_nodes_at_edge_dist(pll_unode_t * edge,
                                            pll_unode_t **outbuffer,
                                            unsigned int *node_count,
                                            unsigned int  min_distance,
                                            unsigned int  max_distance);

#endif /* CORAX_TREE_UTREE_TRAVERSE_H_ */
