#ifndef CORAX_TREE_UTREE_MOVES_H_
#define CORAX_TREE_UTREE_MOVES_H_

#include "corax/corax_common.h"

/* structures for handling topological rearrangement move rollbacks */

#define PLLMOD_TREE_REARRANGE_SPR 0
#define PLLMOD_TREE_REARRANGE_NNI 1
#define PLLMOD_TREE_REARRANGE_TBR 2

/* error codes (for this module, 3000-4000) ; B = 2^10+2^11*/
/* TBR errors (B + {2^2,2^1,2^0}) */
#define PLLMOD_TREE_ERROR_TBR_LEAF_BISECTION 3073   // B + {001}
#define PLLMOD_TREE_ERROR_TBR_OVERLAPPED_NODES 3074 // B + {010}
#define PLLMOD_TREE_ERROR_TBR_SAME_SUBTREE 3075     // B + {011}
#define PLLMOD_TREE_ERROR_TBR_MASK 3079             // B + {111}

/* NNI errors (B + {2^4,2^3}) */
#define PLLMOD_TREE_ERROR_NNI_INVALID_MOVE 3080 // B + {01...}
#define PLLMOD_TREE_ERROR_NNI_LEAF 3081         // B + {01...}
#define PLLMOD_TREE_ERROR_NNI_MASK 3096         // B + {11...}

/* SPR errors (B + {2^6,2^5}) */
#define PLLMOD_TREE_ERROR_SPR_INVALID_NODE 3104 // B + {01...}
#define PLLMOD_TREE_ERROR_SPR_MASK 3168         // B + {11...}

typedef struct pll_utree_edge
{
  pll_unode_t *parent;
  pll_unode_t *child;
  double       length;
} pll_utree_edge_t;

typedef struct
{
  int    rearrange_type;
  int    rooted;
  double likelihood;

  union
  {
    struct
    {
      pll_unode_t *prune_edge;
      pll_unode_t *regraft_edge;
      double       prune_bl;       //! length of the pruned branch
      double       prune_left_bl;  //! length of the removed branch when pruning
      double       prune_right_bl; //! length of the removed branch when pruning
      double       regraft_bl; //! length of the splitted branch when regrafting
    } SPR;
    struct
    {
      pll_unode_t *edge;
      double       left_left_bl;
      double       left_right_bl;
      double       right_left_bl;
      double       right_right_bl;
      double       edge_bl;
      int          type;
    } NNI;
    struct
    {
      pll_unode_t *    bisect_edge;
      pll_utree_edge_t reconn_edge;
      double           bisect_left_bl;
      double           bisect_right_bl;
      double           reconn_parent_left_bl;
      double           reconn_parent_right_bl;
      double           reconn_child_left_bl;
      double           reconn_child_right_bl;
    } TBR;
  };
} pll_tree_rollback_t;

PLL_EXPORT int pllmod_utree_connect_nodes(pll_unode_t *parent,
                                          pll_unode_t *child,
                                          double       length);

PLL_EXPORT int pllmod_utree_bisect(pll_unode_t * edge,
                                   pll_unode_t **parent_subtree,
                                   pll_unode_t **child_subtree);

PLL_EXPORT pll_utree_edge_t pllmod_utree_reconnect(pll_utree_edge_t *edge,
                                                   pll_unode_t *pruned_edge);

PLL_EXPORT pll_unode_t *pllmod_utree_prune(pll_unode_t *edge);

PLL_EXPORT int pllmod_utree_regraft(pll_unode_t *edge, pll_unode_t *tree);

PLL_EXPORT int pllmod_utree_interchange(pll_unode_t *edge1, pll_unode_t *edge2);

PLL_EXPORT int pllmod_utree_tbr(pll_unode_t *        b_edge,
                                pll_utree_edge_t *   r_edge,
                                pll_tree_rollback_t *rollback_info);

PLL_EXPORT int pllmod_utree_spr(pll_unode_t *        p_edge,
                                pll_unode_t *        r_edge,
                                pll_tree_rollback_t *rollback_info);

PLL_EXPORT int pllmod_utree_spr_safe(pll_unode_t *   p,
                                     pll_unode_t *   r,
                                     pll_tree_rollback_t *rollback_info);

/* type = {PLL_NNI_NEXT, PLL_NNI_NEXTNEXT} */
PLL_EXPORT int pllmod_utree_nni(pll_unode_t *        edge,
                                int                  type,
                                pll_tree_rollback_t *rollback_info);

PLL_EXPORT int pllmod_tree_rollback(pll_tree_rollback_t *rollback_info);

#endif /* CORAX_TREE_UTREE_MOVES_H_ */
