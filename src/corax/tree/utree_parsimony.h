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

  CORAX_EXPORT
  corax_utree_t *corax_utree_create_parsimony(unsigned int         taxon_count,
                                              unsigned int         seq_length,
                                              const char *const *  names,
                                              const char *const *  sequences,
                                              const unsigned int * site_weights,
                                              const corax_state_t *map,
                                              unsigned int         states,
                                              unsigned int         attributes,
                                              unsigned int         random_seed,
                                              unsigned int *       score);

  CORAX_EXPORT
  corax_utree_t *
  corax_utree_create_parsimony_multipart(unsigned int       taxon_count,
                                         const char *const *taxon_names,
                                         unsigned int       partition_count,
                                         corax_partition_t *const *partitions,
                                         unsigned int              random_seed,
                                         unsigned int *            score);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CORAX_TREE_UTREE_PARSIMONY_H_ */
