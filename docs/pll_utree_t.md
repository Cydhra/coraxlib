This will cover both the structures `pll_utree_t` and `pll_unode_t`, as well as
the concepts and design around the tree data structure in [`libpll`](libpll.md)

Concepts
================================================================================

Throughout this documentation, there is an intended difference between the terms
`pll_unode_t` and *node*. The former refers to the actual data structure that is used to
represent a vertex in a phylogenetic tree, while a *node* refers to that actual
vertex.  Each node in a phylogenetic tree is made up of 1 or more
`pll_unode_t`s. 

Each `pll_unode_t` is a collection of two pointers and some associated data. The
two pointers are the `next` and `back` pointers. If a `pll_unode_t` is an inner
node, then the `next` pointer points to the next `pll_unode_t` in the node. If
the `next` pointer is `null`, then that `pll_unode_t` represents a tip.

The `back` pointer represents edges. It points to a `pll_unode_t` associated
with another node. Suppose that we have the tree `((a,b),c,d)`, then the libpll
representation of that would what is shown in the following figure.

![test](images/libpll_utree_figure.png)

Here, the dotted arcs indicate `next` pointers, and the solid lines represent
`back` pointers.

Structures
================================================================================

The data structure is made up of two different structs. The first, `pll_utree_t`
wraps the tree. In general, when a tree is used for a function, it requires a
`pll_utree_t`. Some important things to know about this structure: the first
`inner_count` nodes in the `nodes` array are assumed to be "inner nodes". This
means that they have a non-null `next` pointer. Several functions that use
`pll_utree_t`s don't check for this, so they may fail when this assumption is
violated. To avoid this, use the [`pll_utree_wraptree`](#Notable-Functions)
function discussed below to create a `pll_utree_t`.

```
typedef struct pll_utree_s
{
  unsigned int tip_count;
  unsigned int inner_count;
  unsigned int edge_count;
  int binary;

  pll_unode_t ** nodes;
  pll_unode_t * vroot;
} pll_utree_t;
```

Fields:

- `tip_count`: Number of tips
- `inner_count`: number of inner nodes. Not the number of `pll_unode_t` that are
  part of inner nodes, but the number of interior nodes of a phylogenetic tree.
- `edge_count`: the number of edges in the tree.
- `nodes`: an array of pointers to nodes.
- `vroot`: A pointer to the virtual root. By convention, this is always an inner node.
  All tree manipulation functions such as `pll_utree_wraptree` obey to this convention.

----

It might be useful to look at the above figure when thinking about this
structure, since most of the complexity is in the behavior of the `next` and
`back` pointers.

```
typedef struct pll_unode_s
{
  char * label;
  double length;
  unsigned int node_index;
  unsigned int clv_index;
  int scaler_index;
  unsigned int pmatrix_index;
  struct pll_unode_s * next;
  struct pll_unode_s * back;

  void * data;
} pll_unode_t;
```

Fields:

- `label`: The label of the node. Optional.
- `length`: The length of the edge represented by the `back` pointer.
- `node_index`: Index of this node in the `nodes` buffer in a `pll_utree_t`. Each "super"-node shares an index. I.E. the
  index is on the "tree node" level, not on the `pll_unode_t` level.
- `clv_index`: Index of the CLVs to use when calculating a likelihood
- `scaler_index`: Index into the scaler array to represent the CLV scaler
- `pmatrix_index`: index into the array of probability matrices. These probability matrices need to be
  computed based on the length of the branch
- `next`, `back`: See the explanation in the concepts section.
- `data`: An extra pointer to store "user data". In practice, this can be used
  for any task, but existing functions might also use it, so be careful.
