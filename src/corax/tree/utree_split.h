#ifndef CORAX_TREE_UTREE_SPLIT_H_
#define CORAX_TREE_UTREE_SPLIT_H_

#include "corax/corax_common.h"

typedef unsigned int      pll_split_base_t;
typedef pll_split_base_t *pll_split_t;

typedef unsigned int hash_key_t;

typedef struct bitv_hash_entry
{
  hash_key_t    key;
  pll_split_t   bit_vector;
  unsigned int *tree_vector;
  unsigned int  tip_count;
  double        support;
  unsigned int  bip_number;

  struct bitv_hash_entry *next;
} bitv_hash_entry_t;

typedef struct
{
  unsigned int        table_size;
  bitv_hash_entry_t **table;
  unsigned int        entry_count;
  unsigned int        bit_count; /* number of bits per entry */
  unsigned int        bitv_len;  /* bitv length */
} bitv_hashtable_t;

typedef struct string_hash_entry
{
  hash_key_t                key;
  int                       node_number;
  char *                    word;
  struct string_hash_entry *next;
} string_hash_entry_t;

typedef struct
{
  char **               labels;
  unsigned int          table_size;
  string_hash_entry_t **table;
  unsigned int          entry_count;
} string_hashtable_t;

PLL_EXPORT pll_split_t *
           pllmod_utree_split_create(const pll_unode_t *tree,
                                     unsigned int       tip_count,
                                     pll_unode_t **     split_to_node_map);

PLL_EXPORT pll_split_t
pllmod_utree_split_from_tips(unsigned int *subtree_tip_ids,
                             unsigned int  subtree_size,
                             unsigned int  tip_count);

PLL_EXPORT void pllmod_utree_split_normalize_and_sort(pll_split_t *s,
                                                      unsigned int tip_count,
                                                      unsigned int n_splits,
                                                      int          keep_first);

PLL_EXPORT void pllmod_utree_split_show(pll_split_t  split,
                                        unsigned int tip_count);

PLL_EXPORT void pllmod_utree_split_destroy(pll_split_t *split_list);

PLL_EXPORT unsigned int pllmod_utree_split_lightside(pll_split_t  split,
                                                     unsigned int tip_count);

PLL_EXPORT unsigned int pllmod_utree_split_hamming_distance(
    pll_split_t s1, pll_split_t s2, unsigned int tip_count);

PLL_EXPORT int pllmod_utree_split_compatible(const pll_split_t s1,
                                             const pll_split_t s2,
                                             unsigned int      split_len,
                                             unsigned int      tip_count);

PLL_EXPORT int pllmod_utree_split_find(pll_split_t *split_list,
                                       pll_split_t  split,
                                       unsigned int tip_count);

PLL_EXPORT unsigned int pllmod_utree_split_rf_distance(pll_split_t *s1,
                                                       pll_split_t *s2,
                                                       unsigned int tip_count);
// TODO: implement Newick->splits parser
#if 0
PLL_EXPORT pll_split_t * pll_utree_split_newick_string(char * s,
                                                       unsigned int tip_count,
                                                       string_hashtable_t * names_hash);
#endif

/* split hashtable */

PLL_EXPORT
bitv_hashtable_t *pllmod_utree_split_hashtable_create(unsigned int tip_count,
                                                      unsigned int slot_count);

PLL_EXPORT bitv_hash_entry_t *pllmod_utree_split_hashtable_insert_single(
    bitv_hashtable_t *splits_hash, pll_split_t split, double support);

PLL_EXPORT bitv_hashtable_t *
           pllmod_utree_split_hashtable_insert(bitv_hashtable_t *splits_hash,
                                               pll_split_t *     splits,
                                               unsigned int      tip_count,
                                               unsigned int      split_count,
                                               const double *    support,
                                               int               update_only);

PLL_EXPORT bitv_hash_entry_t *pllmod_utree_split_hashtable_lookup(
    bitv_hashtable_t *splits_hash, pll_split_t split, unsigned int tip_count);

PLL_EXPORT
void pllmod_utree_split_hashtable_destroy(bitv_hashtable_t *hash);

#endif /* CORAX_TREE_UTREE_SPLIT_H_ */
