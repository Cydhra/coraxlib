#ifndef CORAX_TREE_TREEINFO_H_
#define CORAX_TREE_TREEINFO_H_

#include "corax/corax_common.h"

#define PLLMOD_TREEINFO_PARTITION_ALL -1

typedef struct treeinfo_edge
{
  unsigned int left_index;
  unsigned int right_index;
  unsigned int pmatrix_index;
  double       brlen;
} pllmod_treeinfo_edge_t;

typedef struct treeinfo_topology
{
  unsigned int            edge_count;
  unsigned int            brlen_set_count;
  unsigned int            root_index;
  pllmod_treeinfo_edge_t *edges;
  double **               branch_lengths;
} pllmod_treeinfo_topology_t;

typedef struct treeinfo
{
  // dimensions
  unsigned int tip_count;
  unsigned int partition_count;

  /* 0 = linked/shared, 1 = linked with scaler, 2 = unlinked */
  int     brlen_linkage;
  double *linked_branch_lengths;

  corax_unode_t *root;
  corax_utree_t *tree;

  unsigned int  subnode_count;
  corax_unode_t **subnodes;

  // partitions & partition-specific stuff
  corax_partition_t **partitions;
  double *          alphas;
  int *gamma_mode; /* discrete GAMMA rates computation mode (mean, median) */
  unsigned int **param_indices;
  int **         subst_matrix_symmetries;
  double **      branch_lengths;
  double *       brlen_scalers;
  double *       partition_loglh;
  int *          params_to_optimize;

  // partition that have been initialized (useful for parallelization)
  unsigned int      init_partition_count;
  unsigned int *    init_partition_idx;
  corax_partition_t **init_partitions;

  /* tree topology constraint */
  unsigned int *constraint;

  /* precomputation buffers for derivatives (aka "sumtable") */
  double **deriv_precomp;

  // invalidation flags
  char **clv_valid;
  char **pmatrix_valid;

  // buffers
  corax_unode_t **   travbuffer;
  unsigned int *   matrix_indices;
  corax_operation_t *operations;

  // partition on which all operations should be performed
  int active_partition;

  // general-purpose counter
  unsigned int counter;

  // parallelization stuff
  void *parallel_context;
  void (*parallel_reduce_cb)(void *, double *, size_t, int);
} pllmod_treeinfo_t;

typedef struct
{
  unsigned int  node_count;
  corax_unode_t **nodes;

  unsigned int  partition_count;
  unsigned int *partition_indices;

  corax_utree_t *tree;
  double **    probs;
} pllmod_ancestral_t;

/** @defgroup pllmod_treeinfo_t pllmod_treeinfo_t
 */

/**
 * Create a pllmod_treeinfo_t from an existing tree.
 *
 * @param root A pointer to the virtual root of the unrooted tree.
 *
 * @param tips: Number of tips in the tree. Almost always this will also be the
 * number of taxa.
 *
 * @param partitions: Number of partitions that will be used in the full
 * analysis.
 *
 * @param brlen_linkage Which branch length linking method to use. Options are:
 * - `PLLMOD_COMMON_BRLEN_UNLINKED`: The branch lengths for one partition have
 *   no relation to any other partition.
 * - `CORAX_BRLEN_SCALED`: The branch lengths are scaled per partition.
 * - `CORAX_BRLEN_LINKED`: The branch lengths are all equal for all
 *   partitions.
 *
 * @ingroup pllmod_treeinfo_t
 */
CORAX_EXPORT pllmod_treeinfo_t *pllmod_treeinfo_create(corax_unode_t *root,
                                                     unsigned int tips,
                                                     unsigned int partitions,
                                                     int brlen_linkage);

CORAX_EXPORT
int pllmod_treeinfo_set_parallel_context(
    pllmod_treeinfo_t *treeinfo,
    void *             parallel_context,
    void (*parallel_reduce_cb)(void *, double *, size_t, int op));

/**
 * Initialize a partition in a treeinfo.
 *
 * @param partition_index Index of the partition to initialize.
 *
 * @param partition The pointer to the partition itself. This partition needs to
 * be initialized before this.
 *
 * @param params_to_optimize Which paramters to optimize. Options are:
 * - PLLMOD_OPT_PARAM_ALL
 * - PLLMOD_OPT_PARAM_SUBST_RATES
 * - PLLMOD_OPT_PARAM_ALPHA
 * - PLLMOD_OPT_PARAM_PINV
 * - PLLMOD_OPT_PARAM_FREQUENCIES
 * - PLLMOD_OPT_PARAM_BRANCHES_SINGLE
 * - PLLMOD_OPT_PARAM_BRANCHES_ALL
 * - PLLMOD_OPT_PARAM_BRANCHES_ITERATIVE
 * - PLLMOD_OPT_PARAM_TOPOLOGY
 * - PLLMOD_OPT_PARAM_FREE_RATES
 * - PLLMOD_OPT_PARAM_RATE_WEIGHTS
 * - PLLMOD_OPT_PARAM_BRANCH_LEN_SCALAR
 * - PLLMOD_OPT_PARAM_USER: Uses user defined code.
 * Any of these can be combined via the `|` operation.
 *
 * @param gamma_mode Controls the gamma rate discretization methods. Options
 * are:
 * - CORAX_GAMMA_RATES_MEAN
 * - CORAX_GAMMA_RATES_MEDIAN
 *
 * @param alpha Initial alpha to use in the model.
 *
 * @param param_indices Specify the parameter indices to ... do stuff. Can be
 * set to null, at which point the defaults are used.
 *
 * @param subst_matrix_symmetries A list of symmetries in the model. Can be set
 * to `nullptr` as well. If set to null, indicates that there are no symmetries
 * in the model. If there are symmetries, the order is "left to right". For
 * example, if our symmetry array was {0,0,0,1,1,1}, then the two symmetry
 * groups would be the top row, and the reamaining triangle
 *
 * @ingroup pllmod_treeinfo_t
 */
CORAX_EXPORT int
pllmod_treeinfo_init_partition(pllmod_treeinfo_t * treeinfo,
                               unsigned int        partition_index,
                               corax_partition_t *   partition,
                               int                 params_to_optimize,
                               int                 gamma_mode,
                               double              alpha,
                               const unsigned int *param_indices,
                               const int *         subst_matrix_symmetries);

CORAX_EXPORT int pllmod_treeinfo_set_active_partition(pllmod_treeinfo_t *treeinfo,
                                                    int partition_index);

CORAX_EXPORT int pllmod_treeinfo_set_root(pllmod_treeinfo_t *treeinfo,
                                        corax_unode_t *      root);

CORAX_EXPORT
int pllmod_treeinfo_get_branch_length_all(const pllmod_treeinfo_t *treeinfo,
                                          const corax_unode_t *      edge,
                                          double *                 lengths);

CORAX_EXPORT int pllmod_treeinfo_set_branch_length(pllmod_treeinfo_t *treeinfo,
                                                 corax_unode_t *      edge,
                                                 double             length);

CORAX_EXPORT
int pllmod_treeinfo_set_branch_length_all(pllmod_treeinfo_t *treeinfo,
                                          corax_unode_t *      edge,
                                          const double *     lengths);

CORAX_EXPORT
int pllmod_treeinfo_set_branch_length_partition(pllmod_treeinfo_t *treeinfo,
                                                corax_unode_t *      edge,
                                                int    partition_index,
                                                double length);

CORAX_EXPORT
corax_utree_t *
pllmod_treeinfo_get_partition_tree(const pllmod_treeinfo_t *treeinfo,
                                   int                      partition_index);

CORAX_EXPORT
pllmod_treeinfo_topology_t *
pllmod_treeinfo_get_topology(const pllmod_treeinfo_t *   treeinfo,
                             pllmod_treeinfo_topology_t *topol);

CORAX_EXPORT
int pllmod_treeinfo_set_topology(pllmod_treeinfo_t *               treeinfo,
                                 const pllmod_treeinfo_topology_t *topol);

CORAX_EXPORT
int pllmod_treeinfo_destroy_topology(pllmod_treeinfo_topology_t *topol);

CORAX_EXPORT int pllmod_treeinfo_destroy_partition(pllmod_treeinfo_t *treeinfo,
                                                 unsigned int partition_index);

CORAX_EXPORT void pllmod_treeinfo_destroy(pllmod_treeinfo_t *treeinfo);

CORAX_EXPORT int pllmod_treeinfo_update_prob_matrices(pllmod_treeinfo_t *treeinfo,
                                                    int update_all);

CORAX_EXPORT void pllmod_treeinfo_invalidate_all(pllmod_treeinfo_t *treeinfo);

CORAX_EXPORT int pllmod_treeinfo_validate_clvs(pllmod_treeinfo_t *treeinfo,
                                             corax_unode_t **     travbuffer,
                                             unsigned int travbuffer_size);

CORAX_EXPORT void pllmod_treeinfo_invalidate_pmatrix(pllmod_treeinfo_t *treeinfo,
                                                   const corax_unode_t *edge);

CORAX_EXPORT void pllmod_treeinfo_invalidate_clv(pllmod_treeinfo_t *treeinfo,
                                               const corax_unode_t *edge);

CORAX_EXPORT double pllmod_treeinfo_compute_loglh(pllmod_treeinfo_t *treeinfo,
                                                int                incremental);

CORAX_EXPORT double pllmod_treeinfo_compute_loglh_flex(
    pllmod_treeinfo_t *treeinfo, int incremental, int update_pmatrices);

CORAX_EXPORT double pllmod_treeinfo_compute_loglh_persite(
    pllmod_treeinfo_t *treeinfo, int incremental, double **persite_lnl);

CORAX_EXPORT
int pllmod_treeinfo_scale_branches_all(pllmod_treeinfo_t *treeinfo,
                                       double             scaler);

CORAX_EXPORT
int pllmod_treeinfo_scale_branches_partition(pllmod_treeinfo_t *treeinfo,
                                             unsigned int       partition_idx,
                                             double             scaler);

CORAX_EXPORT
int pllmod_treeinfo_normalize_brlen_scalers(pllmod_treeinfo_t *treeinfo);

CORAX_EXPORT int pllmod_treeinfo_set_tree(pllmod_treeinfo_t *treeinfo,
                                        corax_utree_t *      tree);

CORAX_EXPORT int
pllmod_treeinfo_set_constraint_clvmap(pllmod_treeinfo_t *treeinfo,
                                      const int *        clv_index_map);

CORAX_EXPORT int
pllmod_treeinfo_set_constraint_tree(pllmod_treeinfo_t *treeinfo,
                                    const corax_utree_t *cons_tree);

CORAX_EXPORT int pllmod_treeinfo_check_constraint(pllmod_treeinfo_t *treeinfo,
                                                corax_unode_t *      subtree,
                                                corax_unode_t *regraft_edge);

CORAX_EXPORT pllmod_ancestral_t *
           pllmod_treeinfo_compute_ancestral(pllmod_treeinfo_t *treeinfo);

CORAX_EXPORT void
pllmod_treeinfo_destroy_ancestral(pllmod_ancestral_t *ancestral);

#endif /* CORAX_TREE_TREEINFO_H_ */
