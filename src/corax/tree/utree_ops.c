#include "utree_ops.h"
#include "corax/corax.h"

static pll_unode_t *unode_prev(pll_unode_t *node)
{
  if (node->next)
  {
    pll_unode_t *prev = node;
    while (prev->next != node) prev = prev->next;
    return prev;
  }
  else
    return NULL;
}

/* This function removes a branch between lnode and lnode->back by
 * "dissolving" a roundabout ("inner node triplet") that contains lnode->back,
 * and merging its remainders into the "left" roundabout as show below:
 *
 *     *-l2-*          *-r2-*                  *-l2-*
      /      \        /      \                /      \
 * --l1  x1  l3------r1  x2  r3--   ---->  --l1  x1  r2--
 *    \      /        \      /                \      /
 *     *----*          *----*                  *-r3-*
 *
 *  where l3 = lnode, r1 = lnode->back
 */
static int remove_branch(pll_unode_t *lnode)
{
  pll_unode_t *rnode = lnode->back;

  /* can only remove a branch between two inner nodes */
  if (!lnode->next || !rnode->next) return PLL_FAILURE;

  pll_unode_t *lnode_prev = unode_prev(lnode);
  pll_unode_t *lnode_next = lnode->next;
  pll_unode_t *rnode_prev = unode_prev(rnode);
  pll_unode_t *rnode_next = rnode->next;

  /* merge remaining subnodes of left and right nodes */
  lnode_prev->next = rnode_next;
  rnode_prev->next = lnode_next;

  /* update clv_index and scaler_index in right node remainder */
  while (rnode_next != lnode_next)
  {
    rnode_next->clv_index    = lnode_prev->clv_index;
    rnode_next->scaler_index = lnode_prev->scaler_index;
    rnode_next->label        = lnode_prev->label;
    rnode_next               = rnode_next->next;
  }

  /* destroy both subnodes adjacent to the deleted branch */
  free(lnode);
  free(rnode->label);
  free(rnode);

  return PLL_SUCCESS;
}

static char *default_support_fmt(double support)
{
  char *sup_str;
  int   size_alloced = asprintf(&sup_str, "%lf", support);

  return size_alloced >= 0 ? sup_str : NULL;
}

PLL_EXPORT void pll_utree_set_length(pll_unode_t *edge, double length)
{
  edge->length = edge->back->length = length;
}

PLL_EXPORT void pll_utree_set_length_recursive(pll_utree_t *tree,
                                               double       length,
                                               int          missing_only)
{
  /* set branch lengths */
  unsigned int i;
  unsigned int tip_count   = tree->tip_count;
  unsigned int inner_count = tree->inner_count;
  for (i = 0; i < tip_count + inner_count; ++i)
  {
    pll_unode_t *node = tree->nodes[i];
    if (!node->length || !missing_only) pll_utree_set_length(node, length);
    if (node->next)
    {
      if (!node->next->length || !missing_only)
        pll_utree_set_length(node->next, length);
      if (!node->next->next->length || !missing_only)
        pll_utree_set_length(node->next->next, length);
    }
  }
}

PLL_EXPORT void pll_utree_scale_branches(pll_utree_t *tree,
                                         double       branch_length_scaler)
{
  /* scale branch lengths */
  unsigned int  i;
  unsigned int  tip_count   = tree->tip_count;
  unsigned int  inner_count = tree->inner_count;
  pll_unode_t **nodes       = tree->nodes;
  for (i = 0; i < tip_count; ++i) { nodes[i]->length *= branch_length_scaler; }
  for (i = tip_count; i < tip_count + inner_count; ++i)
  {
    nodes[i]->length *= branch_length_scaler;
    nodes[i]->next->length *= branch_length_scaler;
    nodes[i]->next->next->length *= branch_length_scaler;
  }
}

PLL_EXPORT void pll_utree_scale_branches_all(pll_unode_t *root,
                                             double       branch_length_scaler)
{
  double root_length;

  /* scale all branches in a tree */
  pll_utree_scale_subtree_branches(root, branch_length_scaler);
  root_length = root->length;
  pll_utree_scale_subtree_branches(root->back, branch_length_scaler);

  /* undo duplicated scaling */
  root->length = root->back->length = root_length;
}

PLL_EXPORT void pll_utree_scale_subtree_branches(pll_unode_t *root,
                                                 double branch_length_scaler)
{
  /* scale all branches in a subtree rooted at node */
  root->length *= branch_length_scaler;
  root->back->length *= branch_length_scaler;

  if (root->next)
  {
    pll_utree_scale_subtree_branches(root->next->back, branch_length_scaler);
    pll_utree_scale_subtree_branches(root->next->next->back,
                                     branch_length_scaler);
  }
}

PLL_EXPORT int pll_utree_collapse_branches(pll_utree_t *tree, double min_brlen)
{
  if (!tree || !tree->vroot)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Empty tree specified!");
    return PLL_FAILURE;
  }

  double        brlen_cutoff  = min_brlen + PLL_ONE_EPSILON;
  unsigned int  tip_count     = tree->tip_count;
  unsigned int  inner_count   = tree->inner_count;
  unsigned int  node_count    = inner_count + tip_count;
  unsigned int  removed_count = 0;
  unsigned int *clv2pos_map =
      (unsigned int *)calloc(inner_count, sizeof(unsigned int));

  if (!clv2pos_map)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC,
                  "Cannot allocate memory for clv2pos map!");
    return PLL_FAILURE;
  }

  /* to avoid making assumptions about node ordering in tree->nodes,
   * we build this map indexed based on clv_index */
  for (unsigned int i = tip_count; i < node_count; ++i)
  {
    unsigned int inner_clv_idx = tree->nodes[i]->clv_index - tip_count;
    clv2pos_map[inner_clv_idx] = i;
  }

  for (unsigned int i = tip_count; i < node_count; ++i)
  {
    pll_unode_t *node = tree->nodes[i];

    /* this node has been removed in a previous iteration -> skip */
    if (!node) continue;

    assert(!PLL_UTREE_IS_TIP(node));

    pll_unode_t *start_node = NULL;
    do
    {
      pll_unode_t *anode = node->back;
      if (PLL_UTREE_IS_TIP(anode) || node->length > brlen_cutoff)
      {
        if (!start_node) start_node = node;
        node = node->next;
      }
      else
      {
        /* remove branch and merge adjacent inner nodes */
        pll_unode_t *prev = unode_prev(node);
        if (tree->vroot == node || tree->vroot == anode) tree->vroot = prev;

        /* find out position of to-be-removed node in the tree->nodes array,
         * and earmark it for deletion by setting respective entry to NULL */
        unsigned int anode_pos = clv2pos_map[anode->clv_index - tip_count];
        assert(anode_pos >= tip_count && anode_pos < node_count);
        tree->nodes[anode_pos] = NULL;
        tree->nodes[i]         = prev;

        remove_branch(node);
        removed_count++;

        node = prev->next;
      }
    } while (node && node != start_node);
  }

  if (removed_count > 0)
  {
    /* compress tree->nodes array by excluding removed inner nodes */
    unsigned int idx            = tip_count;
    unsigned int new_node_count = node_count - removed_count;
    for (unsigned int i = tip_count; i < node_count; ++i)
    {
      pll_unode_t *node = tree->nodes[i];
      if (node) tree->nodes[idx++] = node;
    }
    assert(idx == new_node_count);

    /* update pll_utree_t metadata */
    tree->inner_count -= removed_count;
    tree->edge_count -= removed_count;
    tree->binary = 0;
    tree->nodes  = (pll_unode_t **)realloc(
        tree->nodes, new_node_count * sizeof(pll_unode_t *));
  }

  free(clv2pos_map);

  return PLL_SUCCESS;
}

PLL_EXPORT pll_unode_t *pll_utree_unroot_inplace(pll_unode_t *root)
{
  /* check for a bifurcation at the root */
  if (pll_unode_is_rooted(root))
  {
    if (root->next == root)
    {
      pll_set_error(PLL_ERROR_NEWICK_SYNTAX, "Unifurcation detected at root");
      return PLL_FAILURE;
    }
    pll_unode_t *left  = root->back;
    pll_unode_t *right = root->next->back;

    if (root->label) free(root->label);
    free(root->next);
    free(root);

    double new_length = left->length + right->length;
    left->back        = right;
    right->back       = left;
    left->length = right->length = new_length;
    left->pmatrix_index          = right->pmatrix_index =
        PLL_MIN(left->pmatrix_index, right->pmatrix_index);

    return left->next ? left : right;
  }
  else
    return root;
}

PLL_EXPORT int pllmod_utree_root_inplace(pll_utree_t *tree)
{
  if (!tree)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Empty tree specified!");
    return PLL_FAILURE;
  }

  /* check if tree is already rooted */
  if (tree->vroot->next && tree->vroot->next->next == tree->vroot)
    return PLL_SUCCESS;

  pll_unode_t *root           = tree->vroot;
  pll_unode_t *root_back      = root->back;
  pll_unode_t *root_left      = (pll_unode_t *)calloc(1, sizeof(pll_unode_t));
  pll_unode_t *root_right     = (pll_unode_t *)calloc(1, sizeof(pll_unode_t));
  root_left->next             = root_right;
  root_right->next            = root_left;
  double       root_brlen     = root->length / 2.;
  unsigned int last_clv_index = 0;
  int          last_scaler_index  = 0;
  unsigned int last_node_index    = 0;
  unsigned int last_pmatrix_index = 0;
  unsigned int node_count         = tree->inner_count + tree->tip_count;

  for (unsigned int i = 0; i < node_count; ++i)
  {
    const pll_unode_t *node = tree->nodes[i];
    last_clv_index          = PLL_MAX(last_clv_index, node->clv_index);
    last_scaler_index       = PLL_MAX(last_scaler_index, node->scaler_index);
    do
    {
      last_node_index    = PLL_MAX(last_node_index, node->node_index);
      last_pmatrix_index = PLL_MAX(last_pmatrix_index, node->pmatrix_index);
      node               = node->next;
    } while (node && node != tree->nodes[i]);
  }

  root_left->clv_index = root_right->clv_index = ++last_clv_index;
  root_left->scaler_index = root_right->scaler_index = ++last_scaler_index;
  root_left->node_index                              = ++last_node_index;
  root_right->node_index                             = ++last_node_index;
  root_right->pmatrix_index                          = ++last_pmatrix_index;

  pllmod_utree_connect_nodes(root, root_left, root_brlen);
  pllmod_utree_connect_nodes(root_right, root_back, root_brlen);

  tree->vroot = root_left;
  tree->inner_count++;
  tree->edge_count++;
  node_count++;

  tree->nodes =
      (pll_unode_t **)realloc(tree->nodes, node_count * sizeof(pll_unode_t *));
  tree->nodes[node_count - 1] = root_left;

  return PLL_SUCCESS;
}

PLL_EXPORT int pll_utree_outgroup_root(pll_utree_t * tree,
                                       unsigned int *outgroup_tip_ids,
                                       unsigned int  outgroup_size,
                                       int           add_root_node)
{
  pll_unode_t **split_to_node_map = NULL;
  pll_split_t * tree_splits       = NULL;
  pll_unode_t * new_root          = NULL;
  unsigned int  tip_count;
  unsigned int  split_count;

  if (!tree || !outgroup_tip_ids || !outgroup_size)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Empty tree and/or outgroup specified!");
    return PLL_FAILURE;
  }

  if (outgroup_size == 1)
  {
    // special case single-taxon outgroup: just find a tip by node_index
    for (unsigned int i = 0; i < tree->tip_count; ++i)
    {
      const pll_unode_t *node = tree->nodes[i];
      if (node->node_index == outgroup_tip_ids[0])
      {
        new_root = node->back;
        break;
      }
    }
  }
  else
  {
    tip_count   = tree->tip_count;
    split_count = tip_count - 3;

    split_to_node_map =
        (pll_unode_t **)calloc(split_count, sizeof(pll_unode_t *));

    if (!split_to_node_map)
    {
      pll_set_error(PLL_ERROR_MEM_ALLOC,
                    "Cannot allocate memory for split->node map!");
      return PLL_FAILURE;
    }

    tree_splits = pllmod_utree_split_create(
        tree->vroot, tree->tip_count, split_to_node_map);

    if (!tree_splits)
    {
      assert(pll_errno);
      free(split_to_node_map);
      return PLL_FAILURE;
    }

    // create outgroup split
    pll_split_t outgroup_split = pllmod_utree_split_from_tips(
        outgroup_tip_ids, outgroup_size, tip_count);

    // check if this split is in the tree
    int root_idx =
        pllmod_utree_split_find(tree_splits, outgroup_split, tip_count);
    if (root_idx >= 0) new_root = split_to_node_map[root_idx];

    pllmod_utree_split_destroy(tree_splits);
    free(split_to_node_map);
    free(outgroup_split);
  }

  // set tree->vroot to the outgroup split node
  if (new_root)
  {
    tree->vroot = new_root;
    if (add_root_node)
      return pllmod_utree_root_inplace(tree);
    else
      return PLL_SUCCESS;
  }
  else
  {
    pll_set_error(PLLMOD_TREE_ERROR_POLYPHYL_OUTGROUP,
                  "Outgroup is not monophyletic!");
    return PLL_FAILURE;
  }
}

PLL_EXPORT int pllmod_utree_draw_support(pll_utree_t * ref_tree,
                                         const double *support,
                                         pll_unode_t **node_map,
                                         char *(*cb_serialize)(double))
{
  if (!ref_tree || !support)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Parameter is NULL!\n");
    return PLL_FAILURE;
  }

  unsigned int split_count = ref_tree->edge_count - ref_tree->tip_count;
  for (size_t i = 0; i < split_count; ++i)
  {
    pll_unode_t *node =
        node_map ? node_map[i] : ref_tree->nodes[ref_tree->tip_count + i];

    /* this has to be an inner node! */
    assert(node->next);

    if (node->label) free(node->label);

    node->label = node->next->label = node->next->next->label =
        cb_serialize ? cb_serialize(support[i])
                     : default_support_fmt(support[i]);
  }

  return PLL_SUCCESS;
}
