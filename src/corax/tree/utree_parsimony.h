#ifndef CORAX_TREE_UTREE_PARSIMONY_H_
#define CORAX_TREE_UTREE_PARSIMONY_H_

#include "corax/corax_common.h"

PLL_EXPORT void pll_utree_create_pars_buildops(pll_unode_t *const *trav_buffer,
                                               unsigned int trav_buffer_size,
                                               pll_pars_buildop_t *ops,
                                               unsigned int *      ops_count);

PLL_EXPORT
pll_utree_t *pll_utree_create_parsimony(unsigned int        taxon_count,
                                        unsigned int        seq_length,
                                        char *const *       names,
                                        char *const *       sequences,
                                        const unsigned int *site_weights,
                                        const pll_state_t * map,
                                        unsigned int        states,
                                        unsigned int        attributes,
                                        unsigned int        random_seed,
                                        unsigned int *      score);

PLL_EXPORT
pll_utree_t *
pll_utree_create_parsimony_multipart(unsigned int            taxon_count,
                                     char *const *           taxon_names,
                                     unsigned int            partition_count,
                                     pll_partition_t *const *partitions,
                                     unsigned int            random_seed,
                                     unsigned int *          score);

#endif /* CORAX_TREE_UTREE_PARSIMONY_H_ */
