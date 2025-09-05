#ifndef CORAX_TREE_UTREE_PARSIMONY_H_
#define CORAX_TREE_UTREE_PARSIMONY_H_

#include "corax/tree/utree.h"

#ifdef __cplusplus
extern "C"
{
#endif

  CORAX_EXPORT void
  corax_utree_create_pars_buildops(corax_unode_t *const *trav_buffer,
                                   unsigned int          trav_buffer_size,
                                   corax_pars_buildop_t *ops,
                                   unsigned int *        ops_count);

  /**
   * Creates a maximum parsimony topology using randomized stepwise-addition
   * algorithm. All branch lengths will be set to default.
   */
  CORAX_EXPORT corax_utree_t *
  corax_utree_create_parsimony(unsigned int         taxon_count,
                               unsigned int         seq_length,
                               const char *const *  names,
                               const char *const *  sequences,
                               const unsigned int * site_weights,
                               const corax_state_t *map,
                               unsigned int         states,
                               unsigned int         attributes,
                               unsigned int         random_seed,
                               unsigned int *       score);

  /**
   * Creates a maximum parsimony topology using randomized stepwise-addition
   * algorithm. All branch lengths will be set to default.
   * This function can be used with partitioned alignments (e.g., combined DNA+AA data)
   */
  CORAX_EXPORT corax_utree_t *
  corax_utree_create_parsimony_multipart(unsigned int       taxon_count,
                                         const char *const *taxon_names,
                                         unsigned int       partition_count,
                                         corax_partition_t *const *partitions,
                                         unsigned int              random_seed,
                                         unsigned int *            score);
  
  /**
   * Creates a maximum parsimony topology using randomized stepwise-addition
   * and subsequent optimization with parsimony-based SPR rounds
   */
  CORAX_EXPORT corax_utree_t *
  corax_utree_create_parsimony_multipart_spr(unsigned int       taxon_count,
                                             const char *const *taxon_names,
                                             unsigned int       partition_count,
                                             corax_partition_t *const *partitions,
                                             unsigned int       max_spr_rounds,
                                             unsigned int       spr_cost_epsilon,
                                             unsigned int       random_seed,
                                             unsigned int *     score);

  /**
   * Resolve multifurcations in the tree randomly and optimize the resulting topology
   * with parsimony-based SPR rounds.
   * This function is used to build parsimony trees with a topological constraint.
   */
  CORAX_EXPORT corax_utree_t *
  corax_utree_resolve_parsimony_multipart(const corax_utree_t * multi_tree,
                                          unsigned int partition_count,
                                          corax_partition_t * const * partitions,
                                          const unsigned int * tip_msa_idmap,
                                          unsigned int max_spr_rounds,
                                          unsigned int random_seed,
                                          int * clv_index_map,
                                          unsigned int * score);

  /**
   * Extend a tree by inserting taxa in the maximum-parsimony position.
   * This is basically stepwsie addition with a non-trivial starting tree.
   * This function is used to build parsimony trees with a
   * partial (non-comprehensive) topological constraint.
   */
  CORAX_EXPORT int
  corax_utree_extend_parsimony_multipart(corax_utree_t * tree,
                                         unsigned int taxon_count,
                                         char * const * taxon_names,
                                         const unsigned int * tip_msa_idmap,
                                         unsigned int partition_count,
                                         corax_partition_t * const * partitions,
                                         unsigned int random_seed,
                                         unsigned int * score);

  /**
   *  Optimize topology with parsimony-based SPR rounds.
   *  Execute up to `max_spr_rounds` rounds, terminate if
   *  parsimony score improvement after a round is below `spr_cost_epsilon`.
   */
  CORAX_EXPORT int
  corax_utree_parsimony_spr(corax_utree_t *       tree,
                            unsigned int          partition_count,
                            corax_parsimony_t **  parsimony,
                            unsigned int          max_spr_rounds,
                            unsigned int          spr_cost_epsilon,
                            unsigned int          random_seed,
                            unsigned int *        score);

  /**
   *  Optimize topology with parsimony-based SPR rounds under a topological
   *  constraint defined by `clv_index_map`.
   *  Execute up to `max_spr_rounds` rounds, terminate if
   *  parsimony score improvement after a round is below `spr_cost_epsilon`.
   */
  CORAX_EXPORT int
  corax_utree_parsimony_spr_constrained(corax_utree_t *       tree,
                                        unsigned int          partition_count,
                                        corax_parsimony_t **  parsimony,
                                        const unsigned int *  tip_msa_idmap,
                                        int *                 clv_index_map,
                                        unsigned int          max_spr_rounds,
                                        unsigned int          spr_cost_epsilon,
                                        unsigned int          random_seed,
                                        unsigned int *        score);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CORAX_TREE_UTREE_PARSIMONY_H_ */
