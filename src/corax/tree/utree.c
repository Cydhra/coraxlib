/*
    Copyright (C) 2015-2018 Tomas Flouri, Alexey Kozlov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact: Tomas Flouri <Tomas.Flouri@h-its.org>,
    Heidelberg Institute for Theoretical Studies,
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

#include "corax/corax.h"

extern int   pll_utree_lineno;
extern int   pll_utree_colstart;
extern int   pll_utree_colend;
extern FILE *pll_utree_in;

static void dealloc_data(pll_unode_t *node, void (*cb_destroy)(void *))
{
  if (node->data)
  {
    if (cb_destroy) cb_destroy(node->data);
  }
}

static void dealloc_graph_recursive(pll_unode_t *node,
                                    void (*cb_destroy)(void *),
                                    int level)
{
  if (!node->next)
  {
    /* tip node */
    dealloc_data(node, cb_destroy);
    free(node->label);
    free(node);
  }
  else
  {
    /* inner node */
    if (node->label) free(node->label);

    pll_unode_t *snode = node;
    do {
      if (node != snode || level == 0)
        dealloc_graph_recursive(snode->back, cb_destroy, level + 1);
      pll_unode_t *next = snode->next;
      dealloc_data(snode, cb_destroy);
      free(snode);
      snode = next;
    } while (snode && snode != node);
  }
}

static int indent_space = 4;

static void print_node_info(const pll_unode_t *node, int options)
{
  if (options & PLL_UTREE_SHOW_LABEL) printf(" %s", node->label);
  if (options & PLL_UTREE_SHOW_BRANCH_LENGTH) printf(" %f", node->length);
  if (options & PLL_UTREE_SHOW_CLV_INDEX) printf(" %u", node->clv_index);
  if (options & PLL_UTREE_SHOW_SCALER_INDEX) printf(" %d", node->scaler_index);
  if (options & PLL_UTREE_SHOW_PMATRIX_INDEX)
    printf(" %u", node->pmatrix_index);
  if (options & PLL_UTREE_SHOW_DATA) printf(" %p", node->data);
  printf("\n");
}

static void print_tree_recurse(pll_unode_t *node,
                               int          indent_level,
                               int *        active_node_order,
                               int          options)
{
  int i, j;

  if (!node) return;

  for (i = 0; i < indent_level; ++i)
  {
    if (active_node_order[i])
      printf("|");
    else
      printf(" ");

    for (j = 0; j < indent_space - 1; ++j) printf(" ");
  }
  printf("\n");

  for (i = 0; i < indent_level - 1; ++i)
  {
    if (active_node_order[i])
      printf("|");
    else
      printf(" ");

    for (j = 0; j < indent_space - 1; ++j) printf(" ");
  }

  printf("+");
  for (j = 0; j < indent_space - 1; ++j) printf("-");
  if (node->next) printf("+");

  print_node_info(node, options);

  if (active_node_order[indent_level - 1] == 2)
    active_node_order[indent_level - 1] = 0;

  if (node->next)
  {
    pll_unode_t *snode = node->next;
    do {
      active_node_order[indent_level] = snode->next == node ? 2 : 1;
      print_tree_recurse(
          snode->back, indent_level + 1, active_node_order, options);
      snode = snode->next;
    } while (snode != node);
  }
}

static unsigned int tree_indent_level(const pll_unode_t *node,
                                      unsigned int       indent)
{
  if (!node->next) return indent + 1;

  unsigned int ind   = 0;
  pll_unode_t *snode = node->next;
  do {
    unsigned int sind = tree_indent_level(snode->back, indent + 1);
    ind               = PLL_MAX(ind, sind);
    snode             = snode->next;
  } while (snode && snode != node);

  return ind;
}

PLL_EXPORT void pll_utree_show_ascii(const pll_unode_t *root, int options)
{
  unsigned int a, b;

  if (!root->next) root = root->back;

  a                             = tree_indent_level(root->back, 1);
  b                             = tree_indent_level(root, 0);
  unsigned int max_indent_level = (a > b ? a : b);

  int *active_node_order = (int *)malloc((max_indent_level + 1) * sizeof(int));
  if (!active_node_order)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
    return;
  }
  active_node_order[0] = 1;
  active_node_order[1] = 1;

  const pll_unode_t *node = root;
  do {
    active_node_order[0] = node->next == root ? 2 : 1;
    print_tree_recurse(node->back, 1, active_node_order, options);
    node = node->next;
  } while (node != root);
  free(active_node_order);
}

static char *newick_utree_recurse(const pll_unode_t *root,
                                  char *(*cb_serialize)(const pll_unode_t *),
                                  int level)
{
  char *newick;
  int   size_alloced = 0;
  assert(root != NULL);
  if (!root->next)
  {
    if (cb_serialize)
    {
      newick       = cb_serialize(root);
      size_alloced = (int)strlen(newick);
    }
    else
    {
      size_alloced = asprintf(&newick, "%s:%f", root->label, root->length);
    }
  }
  else
  {
    const pll_unode_t *start      = root->next;
    const pll_unode_t *snode      = start;
    char *             cur_newick = NULL;
    do {
      char *subtree =
          newick_utree_recurse(snode->back, cb_serialize, level + 1);
      if (subtree == NULL)
      {
        pll_set_error(PLL_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
        return NULL;
      }

      if (snode == start) { cur_newick = subtree; }
      else
      {
        assert(cur_newick);
        char *temp   = cur_newick;
        size_alloced = asprintf(&cur_newick, "%s,%s", temp, subtree);
        free(temp);
        free(subtree);
      }
      snode = snode->next;
    } while (snode != root);

    if (level > 0)
    {
      if (cb_serialize)
      {
        char *temp   = cb_serialize(root);
        size_alloced = asprintf(&newick, "(%s)%s", cur_newick, temp);
        free(temp);
      }
      else
      {
        size_alloced = asprintf(&newick,
                                "(%s)%s:%f",
                                cur_newick,
                                root->label ? root->label : "",
                                root->length);
      }
      free(cur_newick);
    }
    else
      newick = cur_newick;
  }

  if (size_alloced < 0)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC,
                  "memory allocation during newick export failed");
    return NULL;
  }

  return newick;
}

char *utree_export_newick(const pll_unode_t *root,
                          int                export_rooted,
                          double             root_brlen,
                          char *(*cb_serialize)(const pll_unode_t *))
{
  char *newick;
  char *subtree1;
  char *subtree2;
  int   size_alloced;

  if (!root) return NULL;

  if (!root->next) root = root->back;

  if (export_rooted)
  {
    assert(!cb_serialize);

    subtree1 = newick_utree_recurse(root->back, cb_serialize, 1);
    subtree2 = newick_utree_recurse(root, cb_serialize, 0);

    size_alloced = asprintf(&newick,
                            "(%s,(%s)%s:%f);",
                            subtree1,
                            subtree2,
                            root->label ? root->label : "",
                            root_brlen);
  }
  else
  {
    subtree1 = newick_utree_recurse(root->back, cb_serialize, 1);
    subtree2 = newick_utree_recurse(root, cb_serialize, 0);

    size_alloced = asprintf(&newick,
                            "(%s,%s)%s;",
                            subtree1,
                            subtree2,
                            root->label ? root->label : "");
  }

  free(subtree1);
  free(subtree2);

  if (size_alloced < 0)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC,
                  "memory allocation during newick export failed");
    return NULL;
  }

  //  printf("newick: %s\n", newick);

  return (newick);
}

PLL_EXPORT char *
pll_utree_export_newick(const pll_unode_t *root,
                        char *(*cb_serialize)(const pll_unode_t *))
{
  return utree_export_newick(root, 0, 0, cb_serialize);
}

PLL_EXPORT char *pll_utree_export_newick_rooted(const pll_unode_t *root,
                                                double             root_brlen)
{
  return utree_export_newick(root, 1, root_brlen, NULL);
}

PLL_EXPORT void pll_utree_create_operations(pll_unode_t *const *trav_buffer,
                                            unsigned int     trav_buffer_size,
                                            double *         branches,
                                            unsigned int *   pmatrix_indices,
                                            pll_operation_t *ops,
                                            unsigned int *   matrix_count,
                                            unsigned int *   ops_count)
{
  const pll_unode_t *node;
  unsigned int       i;

  *ops_count = 0;
  if (matrix_count) *matrix_count = 0;

  for (i = 0; i < trav_buffer_size; ++i)
  {
    node = trav_buffer[i];

    /* if the current node is the second end-point of the edge
    shared with the root node, then do not add the edge to the
    list as it will be added in the end (avoid duplicate edges
    in the list) */
    if (node != trav_buffer[trav_buffer_size - 1]->back)
    {
      if (branches) *branches++ = node->length;
      if (pmatrix_indices) *pmatrix_indices++ = node->pmatrix_index;
      if (matrix_count) *matrix_count = *matrix_count + 1;
    }

    if (node->next)
    {
      ops[*ops_count].parent_clv_index    = node->clv_index;
      ops[*ops_count].parent_scaler_index = node->scaler_index;

      ops[*ops_count].child1_clv_index    = node->next->back->clv_index;
      ops[*ops_count].child1_scaler_index = node->next->back->scaler_index;
      ops[*ops_count].child1_matrix_index = node->next->back->pmatrix_index;

      ops[*ops_count].child2_clv_index = node->next->next->back->clv_index;
      ops[*ops_count].child2_scaler_index =
          node->next->next->back->scaler_index;
      ops[*ops_count].child2_matrix_index =
          node->next->next->back->pmatrix_index;

      *ops_count = *ops_count + 1;
    }
  }
}

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
    do {
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

/* a callback function for checking tree integrity */
static int cb_check_integrity_mult(const pll_utree_t *tree,
                                   const pll_unode_t *node)
{
  unsigned int clv_index     = node->clv_index;
  int          scaler_index  = node->scaler_index;
  unsigned int pmatrix_index = node->pmatrix_index;
  char *       label         = node->label;
  double       length        = node->length;
  unsigned int subnodes      = 1;

  /* edge attributes */
  if (node->back->length != length)
  {
    pll_set_error(0,
                  "Inconsistent branch lengths: %lf != %lf",
                  length,
                  node->back->length);
    return PLL_FAILURE;
  }

  if (node->back->pmatrix_index != pmatrix_index)
  {
    pll_set_error(0,
                  "Inconsistent pmatrix indices: %u != %u",
                  pmatrix_index,
                  node->back->pmatrix_index);
    return PLL_FAILURE;
  }

  if (node->next)
  {
    /* node attributes */
    pll_unode_t *snode = node->next;
    do {
      subnodes++;

      if (tree->binary && subnodes > 3)
      {
        pll_set_error(0,
                      "Multifurcation found in a binary tree "
                      "at node with clv_index = %u",
                      snode->clv_index);
        return PLL_FAILURE;
      }

      if (subnodes > tree->tip_count)
      {
        pll_set_error(0,
                      "Multifurcation exceeding the tree size found "
                      "at node with clv_index = %u",
                      snode->clv_index);
        return PLL_FAILURE;
      }

      if (snode->clv_index != clv_index)
      {
        pll_set_error(0,
                      "Inconsistent CLV indices: %u != %u",
                      clv_index,
                      snode->clv_index);
        return PLL_FAILURE;
      }
      if (snode->scaler_index != scaler_index)
      {
        pll_set_error(0,
                      "Inconsistent scaler indices: %d != %d",
                      scaler_index,
                      snode->scaler_index);
        return PLL_FAILURE;
      }
      if (snode->label != label)
      {
        pll_set_error(
            0, "Inconsistent node labels: '%s' != '%s'", label, snode->label);
        return PLL_FAILURE;
      }
      if (!snode->next)
      {
        pll_set_error(0,
                      "Open roundabout (node->next is NULL) "
                      "at node with clv_index = %u",
                      snode->clv_index);
        return PLL_FAILURE;
      }
      snode = snode->next;
    } while (snode != node);
  }

  return 1;
}

PLL_EXPORT int pll_utree_check_integrity(const pll_utree_t *tree)
{
  return pll_utree_every_const(tree, cb_check_integrity_mult);
}

/* TODO: Memory allocation checks were not implemented in this function!!! */
static pll_unode_t *clone_node(const pll_unode_t *node)
{
  pll_unode_t *new_node = (pll_unode_t *)malloc(sizeof(pll_unode_t));
  memcpy(new_node, node, sizeof(pll_unode_t));

  if (node->label)
  {
    new_node->label = (char *)malloc(strlen(node->label) + 1);
    strcpy(new_node->label, node->label);
  }

  if (node->next)
  {
    pll_unode_t *snode     = node->next;
    pll_unode_t *new_snode = new_node;
    do {
      new_snode->next = (pll_unode_t *)malloc(sizeof(pll_unode_t));
      memcpy(new_snode->next, snode, sizeof(pll_unode_t));
      new_snode->next->label = new_node->label;
      snode                  = snode->next;
      new_snode              = new_snode->next;
    } while (snode != node);

    new_snode->next = new_node;
  }

  return new_node;
}

static void utree_recurse_clone(pll_unode_t *new_root, const pll_unode_t *root)
{
  const pll_unode_t *node = root->back;
  if (node)
  {
    new_root->back       = clone_node(node);
    new_root->back->back = new_root;

    if (node->next)
    {
      pll_unode_t *snode     = node->next;
      pll_unode_t *new_snode = new_root->back->next;
      do {
        utree_recurse_clone(new_snode, snode);
        snode     = snode->next;
        new_snode = new_snode->next;
      } while (snode && snode != node);
    }
  }
}

PLL_EXPORT pll_unode_t *pll_utree_graph_clone(const pll_unode_t *root)
{
  pll_unode_t *new_root = clone_node(root);

  const pll_unode_t *snode     = root;
  pll_unode_t *      new_snode = new_root;
  do {
    utree_recurse_clone(new_snode, snode);
    snode     = snode->next;
    new_snode = new_snode->next;
  } while (snode && snode != root);

  return new_root;
}

PLL_EXPORT pll_utree_t *pll_utree_clone(const pll_utree_t *tree)
{
  /* choose the last inner node as the starting point of the clone. It does not
    really matter which node to choose, but since the newick parser places the
    root node at the end of the list, we use the same notation here */
  pll_unode_t *root = pll_utree_graph_clone(tree->vroot);

  if (tree->binary)
    return pll_utree_wraptree(root, tree->tip_count);
  else
    return pll_utree_wraptree_multi(root, tree->tip_count, tree->inner_count);
}

PLL_EXPORT void pll_utree_create_pars_buildops(pll_unode_t *const *trav_buffer,
                                               unsigned int trav_buffer_size,
                                               pll_pars_buildop_t *ops,
                                               unsigned int *      ops_count)
{
  const pll_unode_t *node;
  unsigned int       i;

  *ops_count = 0;

  for (i = 0; i < trav_buffer_size; ++i)
  {
    node = trav_buffer[i];

    if (node->next)
    {
      ops[*ops_count].parent_score_index = node->node_index;
      ops[*ops_count].child1_score_index = node->next->back->node_index;
      ops[*ops_count].child2_score_index = node->next->next->back->node_index;

      *ops_count = *ops_count + 1;
    }
  }
}

PLL_EXPORT void pll_utree_graph_destroy(pll_unode_t *root,
                                        void (*cb_destroy)(void *))
{
  if (!root) return;

  dealloc_graph_recursive(root, cb_destroy, 0);
}

PLL_EXPORT void pll_utree_destroy(pll_utree_t *tree, void (*cb_destroy)(void *))
{
  unsigned int i;

  /* deallocate tip nodes */
  for (i = 0; i < tree->tip_count; ++i)
  {
    dealloc_data(tree->nodes[i], cb_destroy);
    if (tree->nodes[i]->label) free(tree->nodes[i]->label);
    free(tree->nodes[i]);
  }

  /* deallocate inner nodes */
  for (i = tree->tip_count; i < tree->tip_count + tree->inner_count; ++i)
  {
    pll_unode_t *first = tree->nodes[i];

    assert(first);

    if (first->label) free(first->label);

    pll_unode_t *node = first;
    do {
      pll_unode_t *next = node->next;
      dealloc_data(node, cb_destroy);
      free(node);
      node = next;
    } while (node && node != first);
  }

  /* deallocate tree structure */
  free(tree->nodes);
  free(tree);
}

static void recursive_assign_indices(pll_unode_t * node,
                                     unsigned int *tip_clv_index,
                                     unsigned int *inner_clv_index,
                                     int *         inner_scaler_index,
                                     unsigned int *inner_node_index,
                                     unsigned int  level)
{
  if (!node->next)
  {
    /* tip node */
    node->node_index    = *tip_clv_index;
    node->clv_index     = *tip_clv_index;
    node->pmatrix_index = *tip_clv_index;
    node->scaler_index  = PLL_SCALE_BUFFER_NONE;
    *tip_clv_index      = *tip_clv_index + 1;
  }
  else
  {
    /* inner node */
    pll_unode_t *snode = level ? node->next : node;
    do {
      recursive_assign_indices(snode->back,
                               tip_clv_index,
                               inner_clv_index,
                               inner_scaler_index,
                               inner_node_index,
                               level + 1);
      snode = snode->next;
    } while (snode != node);

    snode = node;
    do {
      snode->node_index   = (*inner_node_index)++;
      snode->clv_index    = *inner_clv_index;
      snode->scaler_index = *inner_scaler_index;
      if (snode == node && level > 0)
        snode->pmatrix_index = *inner_clv_index;
      else
        snode->pmatrix_index = snode->back->pmatrix_index;
      snode = snode->next;
    } while (snode != node);

    *inner_clv_index += 1;
    *inner_scaler_index += 1;
  }
}

PLL_EXPORT void pll_utree_reset_template_indices(pll_unode_t *root,
                                                 unsigned int tip_count)
{
  unsigned int tip_clv_index      = 0;
  unsigned int inner_clv_index    = tip_count;
  unsigned int inner_node_index   = tip_count;
  int          inner_scaler_index = 0;

  if (!root->next) root = root->back;

  recursive_assign_indices(root,
                           &tip_clv_index,
                           &inner_clv_index,
                           &inner_scaler_index,
                           &inner_node_index,
                           0);
}

static void fill_nodes_recursive(pll_unode_t * node,
                                 pll_unode_t **array,
                                 unsigned int  array_size,
                                 unsigned int *tip_index,
                                 unsigned int *inner_index,
                                 unsigned int  level)
{
  unsigned int index;
  if (!node->next)
  {
    /* tip node */
    index = *tip_index;
    *tip_index += 1;
  }
  else
  {
    /* inner node */
    pll_unode_t *snode = level ? node->next : node;
    do {
      fill_nodes_recursive(
          snode->back, array, array_size, tip_index, inner_index, level + 1);
      snode = snode->next;
    } while (snode != node);

    index = *inner_index;
    *inner_index += 1;
  }

  assert(index < array_size);
  array[index] = node;
}

static unsigned int utree_count_nodes_recursive(pll_unode_t * node,
                                                unsigned int *tip_count,
                                                unsigned int *inner_count,
                                                unsigned int  level)
{
  if (!node->next)
  {
    *tip_count += 1;
    return 1;
  }
  else
  {
    unsigned int count = 0;

    pll_unode_t *snode = level ? node->next : node;
    do {
      count += utree_count_nodes_recursive(
          snode->back, tip_count, inner_count, level + 1);
      snode = snode->next;
    } while (snode != node);

    *inner_count += 1;

    return count + 1;
  }
}

static unsigned int utree_count_nodes(pll_unode_t * root,
                                      unsigned int *tip_count,
                                      unsigned int *inner_count)
{
  unsigned int count = 0;

  if (tip_count) *tip_count = 0;

  if (inner_count) *inner_count = 0;

  if (!root->next && !root->back->next) return 0;

  if (!root->next) root = root->back;

  count = utree_count_nodes_recursive(root, tip_count, inner_count, 0);

  if (tip_count && inner_count) assert(count == *tip_count + *inner_count);

  return count;
}

static int unode_is_rooted(const pll_unode_t *root)
{
  return (root->next && root->next->next == root) ? 1 : 0;
}

PLL_EXPORT int pll_utree_is_rooted(const pll_utree_t *tree)
{
  return unode_is_rooted(tree->vroot);
}

static pll_utree_t *utree_wraptree(pll_unode_t *root,
                                   unsigned int tip_count,
                                   unsigned int inner_count,
                                   int          binary)
{
  unsigned int node_count;

  pll_utree_t *tree = (pll_utree_t *)malloc(sizeof(pll_utree_t));
  if (!tree)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
    return PLL_FAILURE;
  }

  if (tip_count < 3 && tip_count != 0)
  {
    pll_set_error(
        PLL_ERROR_INVALID_PARAM, "Invalid tip_count value (%u).", tip_count);
    return PLL_FAILURE;
  }

  if (!root->next) root = root->back;

  if (binary)
  {
    if (tip_count == 0)
    {
      node_count = utree_count_nodes(root, &tip_count, &inner_count);
      if (inner_count != tip_count - 2)
      {
        pll_set_error(PLL_ERROR_INVALID_PARAM,
                      "Input tree is not strictly bifurcating.");
        return PLL_FAILURE;
      }
    }
    else
    {
      inner_count = tip_count - 2;
      node_count  = tip_count + inner_count;
    }
  }
  else
  {
    if (tip_count == 0 || inner_count == 0)
      node_count = utree_count_nodes(root, &tip_count, &inner_count);
    else
      node_count = tip_count + inner_count;
  }

  if (!tip_count)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Input tree contains no inner nodes.");
    return PLL_FAILURE;
  }

  tree->nodes = (pll_unode_t **)malloc(node_count * sizeof(pll_unode_t *));
  if (!tree->nodes)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
    return PLL_FAILURE;
  }

  unsigned int tip_index   = 0;
  unsigned int inner_index = tip_count;

  fill_nodes_recursive(
      root, tree->nodes, node_count, &tip_index, &inner_index, 0);

  assert(tip_index == tip_count);
  assert(inner_index == tip_count + inner_count);

  tree->tip_count   = tip_count;
  tree->inner_count = inner_count;
  tree->edge_count  = node_count - 1;
  tree->binary = (inner_count == tip_count - (unode_is_rooted(root) ? 1 : 2));
  tree->vroot  = root;

  return tree;
}

/* wraps/encalupsates the unrooted tree graph into a tree structure
   that contains a list of nodes, number of tips and number of inner
   nodes. If 0 is passed as tip_count, then an additional recrursion
   of the tree structure is done to detect the number of tips */
PLL_EXPORT pll_utree_t *pll_utree_wraptree(pll_unode_t *root,
                                           unsigned int tip_count)
{
  return utree_wraptree(root, tip_count, 0, 1);
}

PLL_EXPORT pll_utree_t *pll_utree_wraptree_multi(pll_unode_t *root,
                                                 unsigned int tip_count,
                                                 unsigned int inner_count)
{
  return utree_wraptree(root, tip_count, inner_count, 0);
}

PLL_EXPORT pll_unode_t *pll_utree_unroot_inplace(pll_unode_t *root)
{
  /* check for a bifurcation at the root */
  if (unode_is_rooted(root))
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
