#ifndef CORAX_OPTIMIZE_BRANCHES_H_
#define CORAX_OPTIMIZE_BRANCHES_H_

#include "opt_generic.h"

/* Custom parameters structure provided by PLL for the
 * high level optimization functions (Newton-Raphson). */
typedef struct
{
  pll_partition_t *   partition;
  pll_unode_t *       tree;
  const unsigned int *params_indices;
  double *            sumtable;
  double              branch_length_min;
  double              branch_length_max;
  double              tolerance;
  int                 max_newton_iters;
  int                 opt_method; /* see PLLMOD_OPT_BLO_* constants above */
} pll_newton_tree_params_t;

typedef struct
{
  pll_unode_t *     tree;
  unsigned int      partition_count;
  pll_partition_t **partitions;
  unsigned int **   params_indices;
  double **         precomp_buffers;
  double **         brlen_buffers;
  double *          brlen_orig;
  double *          brlen_guess;
  int *             converged;
  double *          brlen_scalers;
  double            branch_length_min;
  double            branch_length_max;
  double            tolerance;
  int               max_newton_iters;
  int               opt_method; /* see PLLMOD_OPT_BLO_* constants above */
  int               brlen_linkage;
  void *            parallel_context;
  void (*parallel_reduce_cb)(void *, double *, size_t, int);
} pll_newton_tree_params_multi_t;

PLL_EXPORT void pllmod_opt_derivative_func(void *  parameters,
                                           double  proposal,
                                           double *df,
                                           double *ddf);

/* high level optimization functions */

PLL_EXPORT double
pllmod_opt_optimize_branch_lengths_iterative(pll_partition_t *   partition,
                                             pll_unode_t *       tree,
                                             const unsigned int *params_indices,
                                             double branch_length_min,
                                             double branch_length_max,
                                             double tolerance,
                                             int    smoothings,
                                             int    keep_update);

PLL_EXPORT double
pllmod_opt_optimize_branch_lengths_local(pll_partition_t *   partition,
                                         pll_unode_t *       tree,
                                         const unsigned int *params_indices,
                                         double              branch_length_min,
                                         double              branch_length_max,
                                         double              tolerance,
                                         int                 smoothings,
                                         int                 radius,
                                         int                 keep_update);

PLL_EXPORT double pllmod_opt_optimize_branch_lengths_local_multi(
    pll_partition_t **partitions,
    size_t            partition_count,
    pll_unode_t *     tree,
    unsigned int **   params_indices,
    double **         sumtable_buffers,
    double **         brlen_buffers,
    double *          brlen_scalers,
    double            branch_length_min,
    double            branch_length_max,
    double            lh_epsilon,
    int               max_iters,
    int               radius,
    int               keep_update,
    int               opt_method,
    int               brlen_linkage,
    void *            parallel_context,
    void (*parallel_reduce_cb)(void *, double *, size_t, int));
#endif /* CORAX_OPTIMIZE_BRANCHES_H_ */
