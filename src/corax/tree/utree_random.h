#ifndef CORAX_TREE_UTREE_RANDOM_H_
#define CORAX_TREE_UTREE_RANDOM_H_

#include "corax/corax_common.h"

PLL_EXPORT pll_utree_t *pll_utree_random_create(unsigned int       taxa_count,
                                                const char *const *names,
                                                unsigned int       random_seed);

PLL_EXPORT int pll_utree_random_extend(pll_utree_t *      tree,
                                       unsigned int       ext_taxa_count,
                                       const char *const *ext_names,
                                       unsigned int       random_seed);

PLL_EXPORT pll_utree_t *
           pll_utree_random_resolve_multi(const pll_utree_t *multi_tree,
                                          unsigned int       random_seed,
                                          int *              clv_index_map);

#endif /* CORAX_TREE_UTREE_RANDOM_H_ */
