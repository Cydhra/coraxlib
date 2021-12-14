#ifndef CORAX_TREE_UTREE_SPLIT_H_
#define CORAX_TREE_UTREE_SPLIT_H_

#include "corax/tree/utree.h"

typedef unsigned int        corax_split_base_t;
typedef corax_split_base_t *corax_split_t;

typedef unsigned int hash_key_t;

typedef struct bitv_hash_entry
{
  hash_key_t    key;
  corax_split_t bit_vector;
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

#ifdef __cplusplus
extern "C"
{
#endif

  CORAX_EXPORT corax_split_t *
               corax_utree_split_create(const corax_unode_t *tree,
                                        unsigned int         tip_count,
                                        corax_unode_t **     split_to_node_map);

  CORAX_EXPORT corax_split_t
  corax_utree_split_from_tips(const unsigned int *subtree_tip_ids,
                              unsigned int        subtree_size,
                              unsigned int        tip_count);

  CORAX_EXPORT void corax_utree_split_normalize_and_sort(corax_split_t *s,
                                                         unsigned int   tip_count,
                                                         unsigned int   n_splits,
                                                         int keep_first);

  CORAX_EXPORT void corax_utree_split_show(const corax_split_t split,
                                           unsigned int        tip_count);

  CORAX_EXPORT void corax_utree_split_destroy(corax_split_t *split_list);

  CORAX_EXPORT unsigned int corax_utree_split_lightside(const corax_split_t split,
                                                        unsigned int tip_count);

  CORAX_EXPORT unsigned int corax_utree_split_hamming_distance(
      const corax_split_t s1, const corax_split_t s2, unsigned int tip_count);

  CORAX_EXPORT int corax_utree_split_compatible(const corax_split_t s1,
                                                const corax_split_t s2,
                                                unsigned int        split_len,
                                                unsigned int        tip_count);

  CORAX_EXPORT int corax_utree_split_find(const corax_split_t *split_list,
                                          const corax_split_t  split,
                                          unsigned int         tip_count);

  CORAX_EXPORT unsigned int corax_utree_split_rf_distance(const corax_split_t *s1,
                                                          const corax_split_t *s2,
                                                          unsigned int tip_count);
  // TODO: implement Newick->splits parser
  #if 0
  CORAX_EXPORT corax_split_t * corax_utree_split_newick_string(char * s,
                                                         unsigned int tip_count,
                                                         string_hashtable_t * names_hash);
  #endif

  /* split hashtable */

  CORAX_EXPORT
  bitv_hashtable_t *corax_utree_split_hashtable_create(unsigned int tip_count,
                                                       unsigned int slot_count);

  CORAX_EXPORT bitv_hash_entry_t *corax_utree_split_hashtable_insert_single(
      bitv_hashtable_t *splits_hash, const corax_split_t split, double support);

  CORAX_EXPORT bitv_hashtable_t *
               corax_utree_split_hashtable_insert(bitv_hashtable_t *splits_hash,
                                                  corax_split_t *   splits,
                                                  unsigned int      tip_count,
                                                  unsigned int      split_count,
                                                  const double *    support,
                                                  int               update_only);

  CORAX_EXPORT bitv_hash_entry_t *
               corax_utree_split_hashtable_lookup(bitv_hashtable_t *  splits_hash,
                                                  const corax_split_t split,
                                                  unsigned int        tip_count);

  CORAX_EXPORT
  void corax_utree_split_hashtable_destroy(bitv_hashtable_t *hash);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CORAX_TREE_UTREE_SPLIT_H_ */
