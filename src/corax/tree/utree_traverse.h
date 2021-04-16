#ifndef CORAX_TREE_UTREE_TRAVERSE_H_
#define CORAX_TREE_UTREE_TRAVERSE_H_

#include "corax/corax_common.h"

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
