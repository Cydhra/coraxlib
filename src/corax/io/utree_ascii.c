#include "utree_io.h"

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
    do
    {
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
  do
  {
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
  do
  {
    active_node_order[0] = node->next == root ? 2 : 1;
    print_tree_recurse(node->back, 1, active_node_order, options);
    node = node->next;
  } while (node != root);
  free(active_node_order);
}
