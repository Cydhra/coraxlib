/*
    Copyright (C) 2015 Tomas Flouri, Diego Darriba

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

#include "utree_moves.h"
#include "corax/corax.h"

static int utree_find(pll_unode_t *start, pll_unode_t *target)
{
  /* checks whether the subtree rooted at 'start' (in the direction of
     start->next and start->next->next) contains the node 'target' */

  if (!start) return 0;

  if (start == target) return 1;

  if (start->next)
  {
    if (start->next == target) return 1;
    if (utree_find(start->next->back, target)) return 1;
  }
  else
    return 0;

  if (start->next->next == target) return 1;
  if (utree_find(start->next->next->back, target)) return 1;

  return 0;
}

static void utree_link(pll_unode_t *a,
                       pll_unode_t *b,
                       double       length,
                       unsigned int pmatrix_index)
{
  a->back   = b;
  b->back   = a;
  a->length = length;
  b->length = length;

  a->pmatrix_index = b->pmatrix_index = pmatrix_index;
}

static void utree_swap(pll_unode_t *t1, pll_unode_t *t2)
{
  /* swaps the positions of trees t1 and t2. The two trees retain the branch
  lengths from their root to their respective parent nodes, and retain their
  pmatrix indices (i.e. no updating of pmatrices is required) */

  pll_unode_t *temp = t1->back;

  utree_link(t1, t2->back, t2->back->length, t2->back->pmatrix_index);
  utree_link(t2, temp, temp->length, temp->pmatrix_index);
}

PLL_EXPORT int pll_utree_nni(pll_unode_t *p, int type, pll_utree_rb_t *rb)
{
  pll_unode_t *subtree1;
  pll_unode_t *subtree2;

  if ((type != PLL_UTREE_MOVE_NNI_LEFT) && (type != PLL_UTREE_MOVE_NNI_RIGHT))
  {
    pll_set_error(PLL_ERROR_NNI_INVALIDMOVE, "Invalid NNI move type");
    return PLL_FAILURE;
  }

  /* check if selected node p is edge  */
  if (!(p->next) || !(p->back->next))
  {
    pll_set_error(PLL_ERROR_NNI_TERMINALBRANCH, "Specified terminal branch");
    return PLL_FAILURE;
  }

  /* check if rollback buffer is provided, and fill it up */
  if (rb)
  {
    rb->move_type    = PLL_UTREE_MOVE_NNI;
    rb->nni.p        = p;
    rb->nni.nni_type = type;
  }

  subtree1 = p->next;
  subtree2 =
      (type == PLL_UTREE_MOVE_NNI_LEFT) ? p->back->next : p->back->next->next;

  utree_swap(subtree1, subtree2);

  return PLL_SUCCESS;
}

static int utree_nni_rollback(pll_utree_rb_t *rb)
{
  /* restore the tree topology from a previous SPR */
  return pll_utree_nni(rb->nni.p, rb->nni.nni_type, NULL);
}

PLL_EXPORT int pll_utree_spr(pll_unode_t *   p,
                             pll_unode_t *   r,
                             pll_utree_rb_t *rb,
                             double *        branch_lengths,
                             unsigned int *  matrix_indices)
{
  /* given nodes p and r, perform an SPR move in the following way,
     i.e. prune subtree C and make it adjacent to subtree D:

      A           B          C             D           A          B
     ____        ____       ____          ____        ____       ____
     \  /        \  /       \  /          \  /        \  /       \  /
      \/          \/         \/            \/          \/         \/
       *          *          * p'           *          *          *
        \         |     q   /                \         |         /
         *'*_____.*._____*'* p     --->       *'*_____.*._____*'*
         '*'     *.*     '*'                  '*'     *.*     '*'
         / r       u    q' \                  /                 \
     r' *                   * v              *                   *
       /\                   /\              /\                   /\
      /__\                 /__\            /__\                 /__\

       D                    E               C                    E

     node p must be part of an inner node (i.e. node with ->next set). The
     procedure prunes the subtree rooted at the opposite end-point of p
     (subtree C in our case) and regrafts it on the edge r'<->r. It is done
     in the following way:

     (a) prune the subtree rooted at the opposite end-point of p (p' on figure)
         by breaking the edges q<->u and q'<->v

     (b) connect node u with node v

     (c) break edge r<->r' by connecting node r with node q, and node r' with
         node q'

     Node r must not be part of the subtree to be pruned (C in this case). Note
     that for speed reasons, the function *does not* check this property to save
     a tree traversal. A safer (albeit slower) function that checks this
     property is pll_utree_spr_safe
  */

  int k = 0;

  if ((!branch_lengths && matrix_indices)
      || (branch_lengths && !matrix_indices))
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Parameters 4,5 must be both NULL or both set");
    return PLL_FAILURE;
  }

  /* if p is a tip node then prompt an error */
  if (!p->next)
  {
    pll_set_error(PLL_ERROR_SPR_TERMINALBRANCH,
                  "Prune edge must be defined by an inner node");
    return PLL_FAILURE;
  }

  /* check whether the move will result in the same tree */
  if (r == p || r == p->back || r == p->next || r == p->next->back
      || r == p->next->next || r == p->next->next->back)
  {
    pll_set_error(PLL_ERROR_SPR_NOCHANGE, "Proposed move yields the same tree");
    return PLL_FAILURE;
  }

  /* check if rollback buffer is provided, and fill it up */
  if (rb)
  {
    rb->move_type    = PLL_UTREE_MOVE_SPR;
    rb->spr.p        = p;
    rb->spr.r        = r;
    rb->spr.rb       = r->back;
    rb->spr.r_len    = r->length;
    rb->spr.pnb      = p->next->back;
    rb->spr.pnb_len  = p->next->length;
    rb->spr.pnnb     = p->next->next->back;
    rb->spr.pnnb_len = p->next->next->length;
  }

  /* (b) connect u and v */
  pll_unode_t *u = p->next->back;
  pll_unode_t *v = p->next->next->back;
  utree_link(u, v, u->length + v->length, u->pmatrix_index);
  /* if requested, store the new branch length for the corresponding
     pmatrix index */
  if (branch_lengths)
  {
    branch_lengths[k] = u->length;
    matrix_indices[k] = u->pmatrix_index;
  }

  /* (a) prune subtree C */
  p->next->back = p->next->next->back = NULL;

  /* (c) regraft C at r<->r' */
  double length = r->length / 2;

  /* r' <-> q' */
  utree_link(r->back, p->next->next, length, p->next->next->pmatrix_index);
  /* if requested, store the new branch length for the corresponding
     pmatrix index */
  if (branch_lengths)
  {
    ++k;
    branch_lengths[k] = length;
    matrix_indices[k] = p->next->next->pmatrix_index;
  }

  /* r<->q */
  utree_link(r, p->next, length, r->pmatrix_index);
  /* if requested, store the new branch length for the corresponding
     pmatrix index */
  if (branch_lengths)
  {
    ++k;
    branch_lengths[k] = length;
    matrix_indices[k] = r->pmatrix_index;
  }

  return PLL_SUCCESS;
}

static int utree_spr_rollback(pll_utree_rb_t *rb,
                              double *        branch_lengths,
                              unsigned int *  matrix_indices)
{
  if ((!branch_lengths && matrix_indices)
      || (branch_lengths && !matrix_indices))
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Parameters 4,5 must be both NULL or both set");
    return PLL_FAILURE;
  }

  int k = 0;

  /* restore the tree topology from a previous SPR */
  utree_link(rb->spr.pnb,
             rb->spr.p->next,
             rb->spr.pnb_len,
             rb->spr.pnb->pmatrix_index);
  if (branch_lengths)
  {
    branch_lengths[k] = rb->spr.pnb_len;
    matrix_indices[k] = rb->spr.pnb->pmatrix_index;
  }

  utree_link(rb->spr.pnnb,
             rb->spr.p->next->next,
             rb->spr.pnnb_len,
             rb->spr.p->next->next->pmatrix_index);
  if (branch_lengths)
  {
    branch_lengths[++k] = rb->spr.pnnb_len;
    matrix_indices[k]   = rb->spr.p->next->next->pmatrix_index;
  }

  utree_link(rb->spr.r, rb->spr.rb, rb->spr.r_len, rb->spr.r->pmatrix_index);
  if (branch_lengths)
  {
    branch_lengths[++k] = rb->spr.r_len;
    matrix_indices[k]   = rb->spr.r->pmatrix_index;
  }

  return PLL_SUCCESS;
}

/* this is a safer (but slower) function for performing an spr move, than
   pll_utree_spr(). See the last paragraph in the comments section of the
   pll_utree_spr() function for more details */
PLL_EXPORT int pll_utree_spr_safe(pll_unode_t *   p,
                                  pll_unode_t *   r,
                                  pll_utree_rb_t *rb,
                                  double *        branch_lengths,
                                  unsigned int *  matrix_indices)
{
  /* check all possible scenarios of failure */
  if (!p)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Node p is set to NULL");
    return PLL_FAILURE;
  }

  if (!r)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Node r is set to NULL");
    return PLL_FAILURE;
  }

  if (!p->next)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Prune edge must be defined by an inner node");
    return PLL_FAILURE;
  }

  /* check whether the move results in the same tree */
  if (r == p || r == p->back || r == p->next || r == p->next->back
      || r == p->next->next || r == p->next->next->back)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Proposed move yields the same tree");
    return PLL_FAILURE;
  }

  /* node r must not be in the same subtree as the one that is to be pruned */
  if (utree_find(p->back, r))
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM,
                  "Node r is part of the subtree to be pruned");
    return PLL_FAILURE;
  }

  return pll_utree_spr(p, r, rb, branch_lengths, matrix_indices);
}

PLL_EXPORT int pll_utree_rollback(pll_utree_rb_t *rollback,
                                  double *        branch_lengths,
                                  unsigned int *  matrix_indices)
{
  if (!rollback)
  {
    pll_set_error(PLL_ERROR_INVALID_PARAM, "Provide a rollback");
    return PLL_FAILURE;
  }

  if (rollback->move_type == PLL_UTREE_MOVE_SPR)
    return utree_spr_rollback(rollback, branch_lengths, matrix_indices);
  else if (rollback->move_type == PLL_UTREE_MOVE_NNI)
    return utree_nni_rollback(rollback);

  pll_set_error(PLL_ERROR_INVALID_PARAM, "Invalid move type");
  return PLL_FAILURE;
}

/* PLLMOD moves */

/******************************************************************************/
/* Topological operations */

static int utree_find_node_in_subtree(pll_unode_t *root, pll_unode_t *node)
{
  if (root == node) { return PLL_SUCCESS; }

  if (root->next)
  {
    if (root->next == node || root->next->next == node) { return PLL_SUCCESS; }

    return utree_find_node_in_subtree(root->next->back, node)
           || utree_find_node_in_subtree(root->next->next->back, node);
  }

  return PLL_FAILURE;
}

/**
 * @brief Connects 2 nodes and sets the pmatrix index and branch length
 *
 * connects `back` pointers of `child` and `parent`
 * pmatrix index for `child` is set to the one in `parent`
 *
 * @param[in,out] parent the parent node
 * @param[in,out] child  the child node
 * @param[in] length     the branch length
 *
 */
PLL_EXPORT int pllmod_utree_connect_nodes(pll_unode_t *parent,
                                          pll_unode_t *child,
                                          double       length)
{
  if (!(parent && child)) return PLL_FAILURE;

  parent->back = child;
  child->back  = parent;
  pll_utree_set_length(parent, length);

  /* PMatrix index is set to parent node */
  child->pmatrix_index = parent->pmatrix_index;

  return PLL_SUCCESS;
}

/**
 * @brief Bisects the tree by removing one edge
 *
 * Removes the edge \p edge and frees the nodes defining that edge.
 * Reconnects the subtrees at the sides of the edge (figure below).
 * The branch lengths of the new edges are the sum of the removed ones.
 * The join branch contains the pmatrix index of the parent edges
 * The removed pmatrix indices are returned in the field
 *     'additional_pmatrix_index' of both output subtrees
 *
 * Returns the new parent and child edges, where parent is the closest to \p
 * edge.
 *
 *   A            C              A        C
 *    \___edge___/       ---->   |        |
 *    /          \               |        |
 *   B            D              B        D
 *   A,B,C,D are subtrees
 *
 * @param[in] edge            edge to remove
 * @param[out] parent_subtree edge corresponding to the 'edge' subtree
 * @param[out] child_subtree  edge corresponding to the 'edge->back' subtree
 * @return PLL_SUCCESS if OK
 */
PLL_EXPORT int pllmod_utree_bisect(pll_unode_t * edge,
                                   pll_unode_t **parent_subtree,
                                   pll_unode_t **child_subtree)
{
  assert(parent_subtree);
  assert(child_subtree);

  pll_unode_t *aux_tree;

  if (!edge->next) return PLL_FAILURE;

  pll_unode_t *c_edge = edge->back;

  /* connect parent subtree */
  (*parent_subtree) = edge->next->back;
  aux_tree          = edge->next->next->back;

  pllmod_utree_connect_nodes(
      *parent_subtree, aux_tree, (*parent_subtree)->length + aux_tree->length);

  edge->next->pmatrix_index = edge->next->next->pmatrix_index;

  /* connect child subtree */
  (*child_subtree) = c_edge->next->back;
  aux_tree         = c_edge->next->next->back;

  pllmod_utree_connect_nodes(
      *child_subtree, aux_tree, (*child_subtree)->length + aux_tree->length);

  c_edge->next->pmatrix_index = c_edge->next->next->pmatrix_index;

  return PLL_SUCCESS;
}

/**
 * Reconnects two subtrees by adding 2 new nodes and 1 edge.
 *
 * Adds 1 new edge connecting edges \p edge.parent and \p edge.child with
 * length \p edge.length.
 *
 *   A       C         A              C
 *   |       |  ---->   \            /
 *                       e1--edge--e2
 *   |       |          /            \
 *   B       D         B              D
 *   A,B,C,D are subtrees
 *
 * @param edge                 new edge (edge structure)
 * @param pruned_edge          edge to prune, defined by a tree node
 *
 * @return the new created edge
 */
PLL_EXPORT pll_utree_edge_t pllmod_utree_reconnect(pll_utree_edge_t *edge,
                                                   pll_unode_t *pruned_edge)
{
  /* create and connect 2 new nodes */
  pll_unode_t *parent_node, *child_node;
  assert(pruned_edge->back);

  parent_node = pruned_edge;
  child_node  = pruned_edge->back;
  assert(parent_node->back == child_node && child_node->back == parent_node);

  assert(!PLL_UTREE_IS_TIP(parent_node));
  assert(!PLL_UTREE_IS_TIP(child_node));

  pll_utree_edge_t new_edge;
  new_edge.child  = child_node;
  new_edge.length = edge->length;

  /* set length */
  pll_utree_set_length(parent_node, edge->length);

  /* reconnect parent close to edge.parent */
  pllmod_utree_connect_nodes(
      parent_node->next->next, edge->parent->back, edge->parent->back->length);

  pllmod_utree_connect_nodes(edge->parent, parent_node->next, 0);

  /* reconnect child close to edge.child */
  pllmod_utree_connect_nodes(
      child_node->next->next, edge->child->back, edge->child->back->length);

  pllmod_utree_connect_nodes(edge->child, child_node->next, 0);

  return new_edge;
}

/**
 * @brief Prunes a subtree in an unrooted tree
 *
 * Disconnecs an edge (e1) and connects the adjacent nodes. New branch (A-B)
 * length is set to the sum of lengths of previous branch (e1-A + e1-B)
 *
 *   A              C              A                   C
 *    \            /               |                  /
 *     e1--edge--e2        --->    |  +   e1--edge--e2
 *    /            \               |                  \
 *   B              D              B                   D
 *   A,B,C,D are subtrees
 *
 *  Note that `edge` is disconnected after the operation
 *
 * @param edge the edge to prune
 * @return the new connected edge, if the operation was applied correctly
 */
PLL_EXPORT pll_unode_t *pllmod_utree_prune(pll_unode_t *edge)
{
  pll_unode_t *edge1, *edge2;

  assert(edge);
  if (!edge->next)
  {
    /* invalid node */
    pll_set_error(PLLMOD_TREE_ERROR_SPR_INVALID_NODE,
                  "Attempting to prune a tip node");
    return NULL;
  }

  /* connect adjacent subtrees together */
  edge1 = edge->next->back;
  edge2 = edge->next->next->back;
  pllmod_utree_connect_nodes(edge1, edge2, edge1->length + edge2->length);

  /* disconnect pruned edge */
  edge->next->back = edge->next->next->back = NULL;

  return edge1;
}

/**
 * @brief Regrafts an edge into a tree
 *
 * Connects a disconnected edge (provided by `e2` in the graph below)
 * into a tree
 *
 *  A                    C         A              C
 *   \                   |          \            /
 *    e1--edge--e2   +   |   --->    e1--edge--e2
 *   /                   |          /            \
 *  B                    D         B              D
 *   A,B,C,D are subtrees
 *
 *  The length of the new branches (e2-C and e2-D) are set to half the length
 *  of the removed branch (C-D)
 *
 * @param edge the edge to regraft
 * @param tree the tree to connect `edge` to
 * @return PLL_SUCCESS if the operation was applied correctly,
 *         PLL_FAILURE otherwise (check pll_errmsg for details)
 */
PLL_EXPORT int pllmod_utree_regraft(pll_unode_t *edge, pll_unode_t *tree)
{
  pll_unode_t *edge1, *edge2;
  double       new_length;

  assert(edge && tree);
  if (!edge->next)
  {
    /* invalid node */
    pll_set_error(PLLMOD_TREE_ERROR_SPR_INVALID_NODE,
                  "Attempting to regraft a tip node");
    return PLL_FAILURE;
  }
  if (edge->next->back || edge->next->next->back)
  {
    /* invalid node */
    pll_set_error(PLLMOD_TREE_ERROR_SPR_INVALID_NODE,
                  "Attempting to regraft a connected node");
    return PLL_FAILURE;
  }

  /* connect tree with edge, splitting the branch designed by tree */
  edge1      = tree;
  edge2      = tree->back;
  new_length = tree->length / 2;
  pllmod_utree_connect_nodes(edge1, edge->next, new_length);
  pllmod_utree_connect_nodes(edge->next->next, edge2, new_length);

  return PLL_SUCCESS;
}

/**
 * Performs one TBR move by applying a bisection and a reconnection.
 * The CLV, scaler and pmatrix indices are updated.
 *
 * @param[in] b_edge bisection point
 * @param[in] r_edge reconnection point
 * @param[out] rollback_info Rollback information for undoing this move.
 *                           If it is NULL, rollback information is ignored.
 *
 * @return PLL_SUCCESS if the move was applied correctly,
 *         PLL_FAILURE otherwise (check pll_errmsg for details)
 */
PLL_EXPORT int pllmod_utree_tbr(pll_unode_t *        b_edge,
                                pll_utree_edge_t *   r_edge,
                                pll_tree_rollback_t *rollback_info)
{
  pll_unode_t *parent, *child;

  /* validate if the move can be applied */

  /* 1. bisection point must not be a leaf branch */
  if (!(b_edge->next && b_edge->back->next))
  {
    pll_set_error(PLLMOD_TREE_ERROR_TBR_LEAF_BISECTION,
                  "attempting to bisect at a leaf node");
    return PLL_FAILURE;
  }

  /* 2. reconnection edges are different from bisection point */
  if (b_edge == r_edge->parent || b_edge == r_edge->parent->back
      || b_edge == r_edge->child || b_edge == r_edge->child->back
      || b_edge->back == r_edge->parent || b_edge->back == r_edge->parent->back
      || b_edge->back == r_edge->child || b_edge->back == r_edge->child->back)
  {
    pll_set_error(PLLMOD_TREE_ERROR_TBR_OVERLAPPED_NODES,
                  "TBR nodes are overlapped");
    return PLL_FAILURE;
  }

  /* 3. reconnection edges must belong to different subtrees rooted at b_edge
   *    and b_edge->back
   */
  if (!(utree_find_node_in_subtree(b_edge, r_edge->parent)
        && utree_find_node_in_subtree(b_edge->back, r_edge->child))
      && !(utree_find_node_in_subtree(b_edge->back, r_edge->parent)
           && utree_find_node_in_subtree(b_edge, r_edge->child)))
  {
    pll_set_error(PLLMOD_TREE_ERROR_TBR_SAME_SUBTREE,
                  "TBR reconnection in same subtree");
    return PLL_FAILURE;
  }

  /* save rollback information */
  if (rollback_info)
  {
    rollback_info->rearrange_type         = PLLMOD_TREE_REARRANGE_TBR;
    rollback_info->rooted                 = 0;
    rollback_info->TBR.bisect_edge        = b_edge;
    rollback_info->TBR.reconn_edge.parent = b_edge->next->next;
    rollback_info->TBR.reconn_edge.child  = b_edge->back->next->next;
    rollback_info->TBR.reconn_edge.length = b_edge->length;

    rollback_info->TBR.bisect_left_bl  = r_edge->parent->length;
    rollback_info->TBR.bisect_right_bl = r_edge->child->length;

    rollback_info->TBR.reconn_parent_left_bl  = b_edge->next->length;
    rollback_info->TBR.reconn_parent_right_bl = b_edge->next->next->length;
    rollback_info->TBR.reconn_child_left_bl   = b_edge->back->next->length;
    rollback_info->TBR.reconn_child_right_bl = b_edge->back->next->next->length;
  }

  /* bisect at b_edge */
  pllmod_utree_bisect(b_edge, &parent, &child);

  /* reconnect at r_edge */
  pllmod_utree_reconnect(r_edge, b_edge);

  return PLL_SUCCESS;
}

/**
 * Performs one SPR move
 * The CLV, scaler and pmatrix indices are updated.
 *
 * @param[in] p_edge Edge to be pruned
 * @param[in] r_edge Edge to be regrafted
 * @param[out] rollback_info Rollback information for undoing this move.
 *                           If it is NULL, rollback information is ignored.
 *
 * @return PLL_SUCCESS if the move was applied correctly,
 *         PLL_FAILURE otherwise (check pll_errmsg for details)
 */
PLL_EXPORT int pllmod_utree_spr(pll_unode_t *        p_edge,
                                pll_unode_t *        r_edge,
                                pll_tree_rollback_t *rollback_info)
{
  int retval;

  if (PLL_UTREE_IS_TIP(p_edge))
  {
    /* invalid move */
    pll_set_error(PLLMOD_TREE_ERROR_SPR_INVALID_NODE,
                  "Attempting to prune a leaf branch");
    return PLL_FAILURE;
  }

  /* save rollback information */
  if (rollback_info)
  {
    rollback_info->rearrange_type     = PLLMOD_TREE_REARRANGE_SPR;
    rollback_info->rooted             = 0;
    rollback_info->SPR.prune_edge     = p_edge;
    rollback_info->SPR.regraft_edge   = p_edge->next->back;
    rollback_info->SPR.prune_bl       = p_edge->length;
    rollback_info->SPR.prune_left_bl  = p_edge->next->length;
    rollback_info->SPR.prune_right_bl = p_edge->next->next->length;
    rollback_info->SPR.regraft_bl     = r_edge->length;
  }

  retval = pll_utree_spr(p_edge, r_edge, 0, 0, 0);

  return retval;
}

/**
 * Performs one NNI move
 * The CLV, scaler and pmatrix indices are updated.
 *
 * @param[in] edge NNI interchange edge
 * @param[in] type move type: PLL_NNI_LEFT, PLL_NNI_RIGHT
 * @param[out] rollback_info Rollback information for undoing this move.
 *                           If it is NULL, rollback information is ignored.
 *
 * @return PLL_SUCCESS if the move was applied correctly,
 *         PLL_FAILURE otherwise (check pll_errmsg for details)
 */
PLL_EXPORT int pllmod_utree_nni(pll_unode_t *        edge,
                                int                  type,
                                pll_tree_rollback_t *rollback_info)
{
  /* validate preconditions */
  assert(edge && edge->back);

  if (!(type == PLL_UTREE_MOVE_NNI_LEFT || type == PLL_UTREE_MOVE_NNI_RIGHT))
  {
    /* invalid move */
    pll_set_error(PLLMOD_TREE_ERROR_NNI_INVALID_MOVE, "Invalid NNI move type");
    return PLL_FAILURE;
  }
  if (PLL_UTREE_IS_TIP(edge) || PLL_UTREE_IS_TIP(edge->back))
  {
    /* invalid move */
    pll_set_error(PLLMOD_TREE_ERROR_NNI_LEAF,
                  "Attempting to apply NNI on a leaf branch");
    return PLL_FAILURE;
  }

  /* save rollback information */
  if (rollback_info)
  {
    rollback_info->rearrange_type     = PLLMOD_TREE_REARRANGE_NNI;
    rollback_info->rooted             = 0;
    rollback_info->NNI.edge           = edge;
    rollback_info->NNI.type           = type;
    rollback_info->NNI.left_left_bl   = edge->next->length;
    rollback_info->NNI.left_right_bl  = edge->next->next->length;
    rollback_info->NNI.right_left_bl  = edge->back->next->length;
    rollback_info->NNI.right_right_bl = edge->back->next->next->length;
    rollback_info->NNI.edge_bl        = edge->length;
  }

  if (!pll_utree_nni(edge, type, 0)) return PLL_FAILURE;

  return PLL_SUCCESS;
}

static int utree_rollback_tbr(pll_tree_rollback_t *rollback_info)
{
  assert(!rollback_info->rooted);
  assert(rollback_info->rearrange_type == PLLMOD_TREE_REARRANGE_TBR);

  pll_unode_t *p             = rollback_info->TBR.bisect_edge;
  pll_unode_t *q             = p->next->back;
  pll_unode_t *r             = p->back->next->back;
  double       reconn_length = rollback_info->TBR.reconn_edge.length;

  /* undo move */
  if (!pllmod_utree_tbr(p, &(rollback_info->TBR.reconn_edge), 0))
    return PLL_FAILURE;

  /* reset branches */
  pll_utree_set_length(p, reconn_length);
  pll_utree_set_length(q, rollback_info->TBR.bisect_left_bl);
  pll_utree_set_length(r, rollback_info->TBR.bisect_right_bl);
  pll_utree_set_length(p->next, rollback_info->TBR.reconn_parent_left_bl);
  pll_utree_set_length(p->next->next,
                       rollback_info->TBR.reconn_parent_right_bl);
  pll_utree_set_length(p->back->next, rollback_info->TBR.reconn_child_left_bl);
  pll_utree_set_length(p->back->next->next,
                       rollback_info->TBR.reconn_child_right_bl);

  return PLL_SUCCESS;
}

static int utree_rollback_spr(pll_tree_rollback_t *rollback_info)
{
  assert(!rollback_info->rooted);
  assert(rollback_info->rearrange_type == PLLMOD_TREE_REARRANGE_SPR);

  pll_unode_t *p  = rollback_info->SPR.prune_edge;
  pll_unode_t *r  = rollback_info->SPR.regraft_edge;
  pll_unode_t *z1 = p->next->back;
  pll_unode_t *z2 = r->back;

  /* undo move */
  if (!pllmod_utree_spr(p, r, 0)) return PLL_FAILURE;

  /* reset branches */
  pll_utree_set_length(z1, rollback_info->SPR.regraft_bl);
  pll_utree_set_length(p, rollback_info->SPR.prune_bl);
  pll_utree_set_length(r, rollback_info->SPR.prune_left_bl);
  pll_utree_set_length(z2, rollback_info->SPR.prune_right_bl);

  return PLL_SUCCESS;
}

static int utree_rollback_nni(pll_tree_rollback_t *rollback_info)
{
  assert(!rollback_info->rooted);
  assert(rollback_info->rearrange_type == PLLMOD_TREE_REARRANGE_NNI);

  pll_unode_t *p = rollback_info->NNI.edge;
  pll_unode_t *q = p->back;

  /* undo move */
  if (!pllmod_utree_nni(p, rollback_info->NNI.type, 0)) return PLL_FAILURE;

  /* reset branches */

  pll_utree_set_length(p, rollback_info->NNI.edge_bl);
  pll_utree_set_length(p->next, rollback_info->NNI.left_left_bl);
  pll_utree_set_length(p->next->next, rollback_info->NNI.left_right_bl);
  pll_utree_set_length(q->next, rollback_info->NNI.right_left_bl);
  pll_utree_set_length(q->next->next, rollback_info->NNI.right_right_bl);

  // assert(UNIMPLEMENTED);
  return PLL_SUCCESS;
}

/**
 * Rollback the previous move
 * @param  rollback_info the rollback info returned by the previous move
 * @return PLL_SUCCESS if the rollback move was applied correctly,
 *         PLL_FAILURE otherwise (check pll_errmsg for details)
 */
PLL_EXPORT int pllmod_tree_rollback(pll_tree_rollback_t *rollback_info)
{
  int retval = PLL_FAILURE;
  switch (rollback_info->rearrange_type)
  {
  case PLLMOD_TREE_REARRANGE_TBR:
  {
    retval = utree_rollback_tbr(rollback_info);
  }
  break;
  case PLLMOD_TREE_REARRANGE_SPR:
  {
    retval = utree_rollback_spr(rollback_info);
  }
  break;
  case PLLMOD_TREE_REARRANGE_NNI:
  {
    retval = utree_rollback_nni(rollback_info);
  }
  break;
  default:
    /* unimplemented */
    assert(0);
    break;
  }
  return retval;
}
