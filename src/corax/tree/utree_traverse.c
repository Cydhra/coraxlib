#include "utree_traverse.h"

PLL_EXPORT int pll_utree_every(pll_utree_t *tree,
                               int (*cb)(const pll_utree_t *,
                                         const pll_unode_t *))
{
  unsigned int i;
  int          rc = 1;

  for (i = 0; i < tree->tip_count + tree->inner_count; ++i)
    rc &= cb(tree, tree->nodes[i]);

  return (rc ? PLL_SUCCESS : PLL_FAILURE);
}

PLL_EXPORT int pll_utree_every_const(const pll_utree_t *tree,
                                     int (*cb)(const pll_utree_t *,
                                               const pll_unode_t *))
{
  unsigned int i;
  int          rc = 1;

  for (i = 0; i < tree->tip_count + tree->inner_count; ++i)
    rc &= cb(tree, tree->nodes[i]);

  return (rc ? PLL_SUCCESS : PLL_FAILURE);
}

static void utree_traverse_recursive(pll_unode_t *node,
                                     int          traversal,
                                     int (*cbtrav)(pll_unode_t *),
                                     unsigned int *index,
                                     pll_unode_t **outbuffer)
{
  if (!cbtrav(node)) return;

  if (traversal == PLL_TREE_TRAVERSE_PREORDER)
  {
    outbuffer[*index] = node;
    *index            = *index + 1;
  }

  if (node->next)
  {
    pll_unode_t *snode = node->next;
    do
    {
      utree_traverse_recursive(
          snode->back, traversal, cbtrav, index, outbuffer);
      snode = snode->next;
    } while (snode && snode != node);
  }

  if (traversal == PLL_TREE_TRAVERSE_POSTORDER)
  {
    outbuffer[*index] = node;
    *index            = *index + 1;
  }
}

PLL_EXPORT int pll_utree_traverse_subtree(pll_unode_t *root,
                                          int          traversal,
                                          int (*cbtrav)(pll_unode_t *),
                                          pll_unode_t **outbuffer,
                                          unsigned int *trav_size)
{
  *trav_size = 0;
  if (!root->next) return PLL_FAILURE;

  if (traversal == PLL_TREE_TRAVERSE_POSTORDER
      || traversal == PLL_TREE_TRAVERSE_PREORDER)
  {
    /* Unlike the other function, we only recurse on the subtree induced by root
     * This means that for the tree
     *             2
     *            / next
     *     1 ----*
     *       back \ next
     *             3
     *
     * Only 2 and 3 will be traversed, because the noce associated with the back
     * pointer will not be used.
     */

    utree_traverse_recursive(root, traversal, cbtrav, trav_size, outbuffer);
  }
  else
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Invalid traversal value.");
    return PLL_FAILURE;
  }

  return PLL_SUCCESS;
}

PLL_EXPORT int pll_utree_traverse(pll_unode_t *root,
                                  int          traversal,
                                  int (*cbtrav)(pll_unode_t *),
                                  pll_unode_t **outbuffer,
                                  unsigned int *trav_size)
{
  *trav_size = 0;
  if (!root->next) return PLL_FAILURE;

  if (traversal == PLL_TREE_TRAVERSE_POSTORDER
      || traversal == PLL_TREE_TRAVERSE_PREORDER)
  {

    /* we will traverse an unrooted tree in the following way

                2
              /
        1  --*
              \
                3

       at each node the callback function is called to decide whether we
       are going to traversing the subtree rooted at the specific node */

    utree_traverse_recursive(
        root->back, traversal, cbtrav, trav_size, outbuffer);
    utree_traverse_recursive(root, traversal, cbtrav, trav_size, outbuffer);
  }
  else
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Invalid traversal value.");
    return PLL_FAILURE;
  }

  return PLL_SUCCESS;
}

static int utree_traverse_apply(pll_unode_t *node,
                                int (*cb_pre_trav)(pll_unode_t *, void *),
                                int (*cb_in_trav)(pll_unode_t *, void *),
                                int (*cb_post_trav)(pll_unode_t *, void *),
                                void *data)
{
  int          retval     = 1;
  pll_unode_t *child_tree = 0;

  if (cb_pre_trav && !cb_pre_trav(node, data)) return PLL_FAILURE;

  if (PLL_UTREE_IS_TIP(node))
  {
    if (cb_in_trav) retval &= cb_in_trav(node, data);
    if (cb_post_trav) retval &= cb_post_trav(node, data);
    return retval;
  }

  child_tree = node->next;
  while (child_tree != node)
  {
    retval &= utree_traverse_apply(
        child_tree->back, cb_pre_trav, cb_in_trav, cb_post_trav, data);

    if (cb_in_trav && child_tree->next != node && !cb_in_trav(child_tree, data))
      return PLL_FAILURE;

    child_tree = child_tree->next;
  }

  if (cb_post_trav) retval &= cb_post_trav(node, data);

  return retval;
}

PLL_EXPORT int
pll_utree_traverse_apply(pll_unode_t *root,
                         int (*cb_pre_trav)(pll_unode_t *, void *),
                         int (*cb_in_trav)(pll_unode_t *, void *),
                         int (*cb_post_trav)(pll_unode_t *, void *),
                         void *data)
{
  int retval = 1;

  assert(root);

  if (PLL_UTREE_IS_TIP(root)) return PLL_FAILURE;

  retval &= utree_traverse_apply(
      root->back, cb_pre_trav, cb_in_trav, cb_post_trav, data);
  retval &=
      utree_traverse_apply(root, cb_pre_trav, cb_in_trav, cb_post_trav, data);

  return retval;
}

static void utree_nodes_at_dist(pll_unode_t * node,
                                pll_unode_t **outbuffer,
                                unsigned int *index,
                                unsigned int  min_distance,
                                unsigned int  max_distance,
                                unsigned int  depth)
{
  if (depth >= min_distance && depth <= max_distance)
  {
    outbuffer[*index] = node;
    *index            = *index + 1;
  }

  if (depth >= max_distance || !(node->next)) return;

  utree_nodes_at_dist(node->next->back,
                      outbuffer,
                      index,
                      min_distance,
                      max_distance,
                      depth + 1);
  utree_nodes_at_dist(node->next->next->back,
                      outbuffer,
                      index,
                      min_distance,
                      max_distance,
                      depth + 1);
}

/**
 * Returns the list of nodes at a distance between \p min_distance and
 * \p max_distance from a specified node
 *
 * @param[in] node the root node
 * @param[out] outbuffer the list of nodes. Outbuffer should be allocated
 * @param[out] node_count the number of nodes returned in \p outbuffer
 * @param[in] min_distance the minimum distance to check
 * @param[in] max_distance the maximum distance to check
 */
PLL_EXPORT int pll_utree_nodes_at_node_dist(pll_unode_t * node,
                                            pll_unode_t **outbuffer,
                                            unsigned int *node_count,
                                            unsigned int  min_distance,
                                            unsigned int  max_distance)
{
  if (!node->next)
  {
    pll_set_error(PLL_ERROR_INVALID_NODE_TYPE,
                  "Internal node expected, but tip node was provided");
    return PLL_FAILURE;
  }

  if (max_distance < min_distance)
  {
    pll_set_error(
        PLL_ERROR_INVALID_RANGE,
        "Invalid distance range: %d..%d (max_distance < min_distance)",
        min_distance,
        max_distance);
    return PLL_FAILURE;
  }

  *node_count = 0;

  /* we will traverse an unrooted tree in the following way

               1
             /
          --*
             \
               2
    */

  utree_nodes_at_dist(
      node, outbuffer, node_count, min_distance, max_distance, 0);

  return PLL_SUCCESS;
}

/**
 * Returns the list of nodes at a distance between \p min_distance and
 * \p max_distance from a specified edge
 *
 * @param[in] edge the root edge
 * @param[out] outbuffer the list of nodes. Outbuffer should be allocated
 * @param[out] node_count the number of nodes returned in \p outbuffer
 * @param[in] min_distance the minimum distance to check
 * @param[in] max_distance the maximum distance to check
 */

PLL_EXPORT int pll_utree_nodes_at_edge_dist(pll_unode_t * edge,
                                            pll_unode_t **outbuffer,
                                            unsigned int *node_count,
                                            unsigned int  min_distance,
                                            unsigned int  max_distance)
{
  unsigned int depth = 0;

  if (!edge->next)
  {
    pll_set_error(PLL_ERROR_INVALID_NODE_TYPE,
                  "Internal node expected, but tip node was provided");
    return PLL_FAILURE;
  }

  if (max_distance < min_distance)
  {
    pll_set_error(
        PLL_ERROR_INVALID_RANGE,
        "Invalid distance range: %d..%d (max_distance < min_distance)",
        min_distance,
        max_distance);
    return PLL_FAILURE;
  }

  *node_count = 0;

  /* we will traverse an unrooted tree in the following way

       3          1
        \        /
         * ---- *
        /        \
       4          2
   */

  utree_nodes_at_dist(
      edge->back, outbuffer, node_count, min_distance, max_distance, depth + 1);
  utree_nodes_at_dist(
      edge, outbuffer, node_count, min_distance, max_distance, depth);

  return PLL_SUCCESS;
}
