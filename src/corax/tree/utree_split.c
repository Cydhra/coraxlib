#include "corax/corax.h"
#include "hashtable.h"

struct split_node_pair
{
  corax_split_t  split;
  corax_unode_t *node;
};

struct cb_split_params
{
  struct split_node_pair *split_nodes;
  unsigned int            tip_count;
  unsigned int            split_size;
  unsigned int            split_len;
  unsigned int            split_count; /* number of splits already set */
  int *id_to_split; /* map between node/subnode ids and splits */
};

/******************************************************************************/
/* static functions */

static inline void
merge_split(corax_split_t to, const corax_split_t from, unsigned int split_len)
{
  unsigned int i;
  for (i = 0; i < split_len; ++i) to[i] |= from[i];
}

/*
  The position of the node in the map of branches to splits is computed
  according to the node id.
 */
static unsigned int get_utree_splitmap_id(corax_unode_t *node,
                                          unsigned int   tip_count)
{
  unsigned int node_id = node->node_index;
  assert(node_id >= tip_count);
  return node_id - tip_count;
}

/**
 * Callback function for computing the splits at each branch
 * The splits will be stored in data->splits
 * at positions given by node index
 */
static int cb_get_splits(corax_unode_t *node, void *data)
{
  struct cb_split_params *split_data = (struct cb_split_params *)data;
  corax_split_t           current_split;

  unsigned int tip_count  = split_data->tip_count;
  unsigned int split_size = split_data->split_size;
  unsigned int split_len  = split_data->split_len;
  unsigned int my_split_id, child_split_id;
  unsigned int my_map_id, back_map_id;
  unsigned int tip_id, split_id;

  if (!(CORAX_UTREE_IS_TIP(node) || CORAX_UTREE_IS_TIP(node->back)))
  {
    my_map_id   = get_utree_splitmap_id(node, tip_count);
    back_map_id = get_utree_splitmap_id(node->back, tip_count);
    my_split_id = split_data->split_count;

    /* check if the split for the branch was already set */
    /* note that tree traversals visit the virtual root branch twice */
    if (split_data->id_to_split[my_map_id] >= 0) { return 1; }

    assert(my_split_id < (tip_count - 3));
    split_data->id_to_split[my_map_id]   = (int)my_split_id;
    split_data->id_to_split[back_map_id] = (int)my_split_id;

    split_data->split_nodes[my_split_id].node = node;

    /* get current split to fill */
    current_split = split_data->split_nodes[my_split_id].split;
    /* increase number of splits */
    split_data->split_count++;

    /* add the split from left branch */
    if (!CORAX_UTREE_IS_TIP(node->next->back))
    {
      child_split_id =
          (unsigned int)split_data
              ->id_to_split[get_utree_splitmap_id(node->next, tip_count)];

      memcpy(current_split,
             split_data->split_nodes[child_split_id].split,
             sizeof(corax_split_base_t) * split_len);
    }
    else
    {
      tip_id = node->next->back->node_index;
      assert(tip_id < tip_count);
      split_id = tip_id / split_size;
      tip_id %= split_size;
      current_split[split_id] = (1 << tip_id);
    }

    /* add the split from right branch */
    if (!CORAX_UTREE_IS_TIP(node->next->next->back))
    {
      child_split_id =
          (unsigned int)split_data
              ->id_to_split[get_utree_splitmap_id(node->next->next, tip_count)];
      merge_split(current_split,
                  split_data->split_nodes[child_split_id].split,
                  split_len);
    }
    else
    {
      tip_id = node->next->next->back->node_index;
      assert(tip_id < tip_count);
      split_id = tip_id / split_size;
      tip_id %= split_size;
      current_split[split_id] |= (1 << tip_id);
    }
  }

  /* continue */
  return 1;
}

/*
 * The order of the splits is not really significant, as long as the two
 * following agree.
 *
 * _cmp_splits is used for sorting.
 * compare_splits is used for comparing splits from different trees
 */
static int
compare_splits(corax_split_t s1, corax_split_t s2, unsigned int split_len)
{
  unsigned int i;

  for (i = 0; i < split_len; ++i)
  {
    if (s1[i] != s2[i]) return (int)(s1[i] > s2[i] ? 1 : -1);
  }
  return 0;
}

/*
 * Precondition: splits *must* be different.
 */
static int _cmp_splits(const void *a, const void *b)
{
  const corax_split_t *s1    = (const corax_split_t *)a;
  const corax_split_t *s2    = (const corax_split_t *)b;
  unsigned int         limit = 10000; /* max_taxa = split_size * 10^4 */
  int                  i     = 0;
  for (; ((*s1)[i] == (*s2)[i]) && limit; --limit, ++i)
    ;
  assert(limit);
  return (int)((*s1)[i] > (*s2)[i] ? 1 : -1);
}

static int _cmp_split_node_pair(const void *a, const void *b)
{
  const struct split_node_pair *s1 = (const struct split_node_pair *)a;
  const struct split_node_pair *s2 = (const struct split_node_pair *)b;

  return (_cmp_splits(&s1->split, &s2->split));
}

/*
 * Returns 1 if the split is valid (not all 0s or all 1s) and normalized;
 * returns 0 otherwise
 * */
static int split_is_valid_and_normalized(const corax_split_t bitv,
                                         unsigned int        tip_count)
{
  // this will also automatically check for all-0s case
  if (!bitv_is_normalized(bitv)) return 0;

  // now check that we don't have all 1s
  unsigned int split_size   = sizeof(corax_split_base_t) * 8;
  unsigned int split_offset = tip_count % split_size;
  unsigned int split_len    = bitv_length(tip_count);
  unsigned int i            = 0;
  unsigned int all1         = ~0u;
  unsigned int mask         = all1;
  for (i = 0; i < split_len - 1; ++i) { mask &= bitv[i]; }
  if (split_offset)
    mask &= bitv[split_len - 1] | ((1 << split_offset) - 1);
  else
    mask &= bitv[split_len - 1];

  return (mask == all1) ? 0 : 1;
}

static void truncate_splits(corax_split_set_t * split_set, unsigned int new_tip_count)
{
  unsigned int bitv_elem = split_set->split_size;
  unsigned int bitv_size = new_tip_count / bitv_elem;
  unsigned int bitv_off = new_tip_count % bitv_elem;
  if (bitv_off > 0)
    bitv_size++;

  if (split_set->tip_count > new_tip_count)
  {
    unsigned int mask = 0;
    for (unsigned int i = 0; i < (bitv_off ? bitv_off : bitv_elem); ++i)
      mask |= (1u << i);

    for (unsigned int i = 0; i < split_set->split_count; ++i)
    {
      corax_split_t last_elem = split_set->splits[i] + bitv_size - 1;
      *last_elem &= mask;
    }
  }
}

/******************************************************************************/
/* tree split functions */

CORAX_EXPORT corax_split_t
corax_utree_split_from_tips(const unsigned int *subtree_tip_ids,
                            unsigned int        subtree_size,
                            unsigned int        tip_count)
{
  size_t split_size = (sizeof(corax_split_base_t) * 8);
  size_t split_len  = (tip_count / split_size)
                     + (tip_count % (sizeof(corax_split_base_t) * 8) > 0);
  corax_split_t split =
      (corax_split_t)calloc(split_len, sizeof(corax_split_base_t));

  for (unsigned int i = 0; i < subtree_size; ++i)
  {
    unsigned int tip_id = subtree_tip_ids[i];
    unsigned int vec_id = tip_id / split_size;
    unsigned int bit_id = tip_id % split_size;
    split[vec_id] |= (1 << bit_id);
  }
  bitv_normalize(split, tip_count);
  return split;
}

/* Note: This function returns the splits according to the node indices at the
 * tips!
 *
 * split_to_node_map can be NULL
 */
CORAX_EXPORT corax_split_t *
             corax_utree_split_create(const corax_unode_t *tree,
                                      unsigned int         tip_count,
                                      corax_unode_t **     split_to_node_map)
{
  unsigned int   i;
  unsigned int   split_count, split_len, split_size;
  corax_split_t *split_list; /* array with ordered split pointers */
  corax_split_t  splits;     /* contiguous array of splits, as size is known */
  struct split_node_pair *split_nodes;
  corax_split_t           first_split;

  /* as many non-trivial splits as inner branches */
  split_count = tip_count - 3;
  split_size  = (sizeof(corax_split_base_t) * 8);
  split_len   = (tip_count / split_size)
              + (tip_count % (sizeof(corax_split_base_t) * 8) > 0);

  split_list = (corax_split_t *)malloc(split_count * sizeof(corax_split_t));
  if (!split_list)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC,
                    "Cannot allocate memory for split list\n");
    return NULL;
  }

  split_nodes = (struct split_node_pair *)malloc(
      split_count * sizeof(struct split_node_pair));
  if (!split_nodes)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC,
                    "Cannot allocate memory for split-node pairs\n");
    free(split_list);
    return NULL;
  }

  splits = (corax_split_t)calloc(split_count * split_len,
                                 sizeof(corax_split_base_t));
  if (!splits)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC,
                    "Cannot allocate memory for splits\n");
    free(split_list);
    free(split_nodes);
    return NULL;
  }

  for (i = 0; i < split_count; ++i)
  {
    split_nodes[i].split = splits + i * split_len;
    split_list[i]        = splits + i * split_len;
  }

  struct cb_split_params split_data;
  split_data.split_nodes = split_nodes;
  split_data.split_len   = split_len;
  split_data.split_size  = split_size;
  split_data.tip_count   = tip_count;
  split_data.split_count = 0;

  /* reserve positions for node and subnode ids */
  split_data.id_to_split = (int *)malloc(sizeof(int) * 3 * (tip_count - 2));

  if (!split_data.id_to_split)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC,
                    "Cannot allocate memory for splits\n");
    free(split_list);
    free(split_nodes);
    return NULL;
  }

  for (i = 0; i < 3 * (tip_count - 2); ++i) split_data.id_to_split[i] = -1;

  if (CORAX_UTREE_IS_TIP(tree)) tree = tree->back;

  /* traverse for computing the scripts */
  corax_utree_traverse_apply(
      (corax_unode_t *)tree, NULL, NULL, &cb_get_splits, &split_data);

  // TODO better handling for multifurcating trees
  assert(split_data.split_count <= split_count);

  free(split_data.id_to_split);

  for (i = 0; i < split_count; ++i) bitv_normalize(split_list[i], tip_count);

  /* sort map and split list together */
  first_split = split_nodes[0].split;
  qsort(split_nodes,
        split_count,
        sizeof(struct split_node_pair),
        _cmp_split_node_pair);

  /* if first item has changed, swap them such that the array can be deallocated
   */
  if (first_split != split_nodes[0].split)
  {
    /* find first split */
    for (i = 1; split_nodes[i].split != first_split && i < split_count; ++i)
      ;
    assert(i < split_count);

    /* swap */
    void *aux_mem = malloc(sizeof(corax_split_base_t) * split_len);
    if (!aux_mem)
    {
      corax_set_error(CORAX_ERROR_MEM_ALLOC,
                      "Cannot allocate memory for auxiliary array\n");
      free(split_list);
      free(split_nodes);
      return NULL;
    }

    memcpy(aux_mem, first_split, sizeof(corax_split_base_t) * split_len);
    memcpy(first_split,
           split_nodes[0].split,
           sizeof(corax_split_base_t) * split_len);
    memcpy(
        split_nodes[0].split, aux_mem, sizeof(corax_split_base_t) * split_len);
    free(aux_mem);
    split_nodes[i].split = split_nodes[0].split;
    split_nodes[0].split = first_split;
  }

  for (i = 0; i < split_count; ++i)
  {
    split_list[i] = split_nodes[i].split;
    assert(split_is_valid_and_normalized(split_list[i], tip_count));
  }

  /* update output arrays */
  if (split_to_node_map)
  {
    for (i = 0; i < split_count; ++i)
    {
      split_list[i]        = split_nodes[i].split;
      split_to_node_map[i] = split_nodes[i].node;
    }
  }
  else
  {
    for (i = 0; i < split_count; ++i) split_list[i] = split_nodes[i].split;
  }

  free(split_nodes);

  return split_list;
}

CORAX_EXPORT void corax_utree_split_destroy(corax_split_t *split_list)
{
  free(split_list[0]);
  free(split_list);
}

CORAX_EXPORT unsigned int corax_utree_split_lightside(const corax_split_t split,
                                                      unsigned int tip_count)
{
  return bitv_lightside(split, tip_count, 0);
}

/* This function computes a classical Hamming distance between two tree splits
 */
CORAX_EXPORT unsigned int corax_utree_split_hamming_distance(
    const corax_split_t s1, const corax_split_t s2, unsigned int tip_count)
{
  unsigned int split_len = bitv_length(tip_count);
  unsigned int hdist     = 0;
  unsigned int i;

  for (i = 0; i < split_len; ++i) { hdist += CORAX_POPCNT32(s1[i] ^ s2[i]); }

  return CORAX_MIN(hdist, tip_count - hdist);
}

CORAX_EXPORT void corax_utree_split_show(const corax_split_t split,
                                         unsigned int        tip_count)
{
  unsigned int split_size   = sizeof(corax_split_base_t) * 8;
  unsigned int split_offset = tip_count % split_size;
  unsigned int split_len    = bitv_length(tip_count);
  unsigned int i, j;

  if (!split_offset) split_offset = split_size;

  for (i = 0; i < (split_len - 1); ++i)
    for (j = 0; j < split_size; ++j)
      (split[i] & (1u << j)) ? putchar('*') : putchar('-');
  for (j = 0; j < split_offset; ++j)
    (split[i] & (1u << j)) ? putchar('*') : putchar('-');
}

/*
 * Normalize and sort.
 * Warning! first position might change, so if splits were allocated together
 * you should keep a pointer to the original first position such that you can
 * deallocate it afterwards!
 */
/**
 * normalizes and sorts a set of splits
 *
 * @param s           set of splits
 * @param tip_count   number of tips
 * @param split_count numer of splits in 's'
 * @param keep_first  do not change first pointer in 's' (i.e., s[0])
 *
 * `keep_fist` parameter is important if the set of splits were allocated in
 * a contiguous chunk of memory and you want to use s[0] to deallocate it in
 * the future.
 */
CORAX_EXPORT void corax_utree_split_normalize_and_sort(corax_split_t *s,
                                                       unsigned int   tip_count,
                                                       unsigned int split_count,
                                                       int          keep_first)
{
  unsigned int i;
  unsigned int split_len;

  corax_reset_error();

  corax_split_t first_split;
  for (i = 0; i < split_count; ++i) bitv_normalize(s[i], tip_count);

  first_split = s[0];
  qsort(s, split_count, sizeof(corax_split_t), _cmp_splits);

  if (keep_first && first_split != s[0])
  {
    split_len = bitv_length(tip_count);

    /* find first split */
    for (i = 1; s[i] != first_split && i < split_count; ++i)
      ;
    assert(i < split_count);

    /* swap */
    void *aux_mem = malloc(sizeof(corax_split_base_t) * split_len);
    if (!aux_mem)
    {
      corax_set_error(CORAX_ERROR_MEM_ALLOC,
                      "Cannot allocate memory for auxiliary array\n");
      return;
    }

    memcpy(aux_mem, first_split, sizeof(corax_split_base_t) * split_len);
    memcpy(first_split, s[0], sizeof(corax_split_base_t) * split_len);
    memcpy(s[0], aux_mem, sizeof(corax_split_base_t) * split_len);
    free(aux_mem);
    s[i] = s[0];
    s[0] = first_split;
  }
}

/*
 * Precondition: splits must be normalized and sorted!
 */
CORAX_EXPORT unsigned int corax_utree_split_rf_distance(const corax_split_t *s1,
                                                        const corax_split_t *s2,
                                                        unsigned int tip_count)
{
  unsigned int split_count = tip_count - 3;
  unsigned int split_len   = bitv_length(tip_count);
  unsigned int equal       = 0;
  unsigned int s1_idx = 0, s2_idx = 0;

  for (s1_idx = 0; s1_idx < split_count && s2_idx < split_count; ++s1_idx)
  {
    int cmp = compare_splits(s1[s1_idx], s2[s2_idx], split_len);
    if (!cmp)
    {
      equal++;
      s2_idx++;
    }
    else
    {
      if (cmp > 0)
      {
        while (++s2_idx < split_count
               && (cmp = compare_splits(s1[s1_idx], s2[s2_idx], split_len)) > 0)
          ;
        if (!cmp)
        {
          equal++;
          // s2_idx++;
        }
      }
    }
  }

  assert(equal <= (tip_count - 3));

  return 2 * (tip_count - 3 - equal);
}

CORAX_EXPORT int corax_utree_split_find(const corax_split_t *split_list,
                                        const corax_split_t  split,
                                        unsigned int         tip_count)
{
  unsigned int split_count = tip_count - 3;
  unsigned int split_len   = bitv_length(tip_count);
  for (unsigned int i = 0; i < split_count; ++i)
  {
    if (!bitv_compare(split_list[i], split, split_len)) { return i; }
  }

  return -1;
}

CORAX_EXPORT int corax_utree_split_compatible(const corax_split_t s1,
                                              const corax_split_t s2,
                                              unsigned int        split_len,
                                              unsigned int        tip_count)
{
  unsigned int i;
  unsigned int split_size   = sizeof(corax_split_base_t) * 8;
  unsigned int split_offset = tip_count % split_size;
  unsigned int mask         = split_offset ? (1u << split_offset) - 1 : ~0u;

  /* check conflicts between s1 and s2 */
  for (i = 0; i < split_len; i++)
    if (s1[i] & s2[i]) break;

  if (i == split_len) return 1;

  /* check conflicts between s1 and ~s2 */
  for (i = 0; i < split_len; i++)
    if (s1[i] & ~s2[i]) break;

  if (i == split_len) return 1;

  /* check conflicts between ~s1 and s2 */
  for (i = 0; i < split_len; i++)
    if (~s1[i] & s2[i]) break;

  if (i == split_len) return 1;

  /* check conflicts between ~s1 and ~s2 */
  for (i = 0; i < split_len - 1; i++)
    if (~s1[i] & ~s2[i]) break;

  if (i == split_len - 1 && !(~s1[i] & ~s2[i] & mask)) ++i;

  if (i == split_len)
    return 1;
  else
    return 0;
}

CORAX_EXPORT
bitv_hashtable_t *corax_utree_split_hashtable_create(unsigned int tip_count,
                                                     unsigned int slot_count)
{
  if (!slot_count) slot_count = tip_count * 10;

  return hash_init(slot_count, tip_count);
}

CORAX_EXPORT bitv_hash_entry_t *corax_utree_split_hashtable_insert_single(
    bitv_hashtable_t *splits_hash, const corax_split_t split, double support)
{
  if (!splits_hash)
  {
    corax_set_error(CORAX_ERROR_INVALID_PARAM, "splits_hash is NULL!\n");
    return NULL;
  }

  return hash_insert(
      split, splits_hash, splits_hash->entry_count, HASH_KEY_UNDEF, support, 0);
}

/**
 * Creates or updates hashtable with splits (and their support)
 *
 * @param splits_hash    hashtable to update, NULL: create new hashtable
 * @param tip_count      number of tips
 * @param split_count    number of splits in 'splits'
 * @param support        support values for the split
 * @param update_only    0: insert new values as needed,
 *                       1: only increment support for existing splits
 *
 * @returns hashtable with splits
 */
CORAX_EXPORT bitv_hashtable_t *
             corax_utree_split_hashtable_insert(bitv_hashtable_t *splits_hash,
                                                corax_split_t *   splits,
                                                unsigned int      tip_count,
                                                unsigned int      split_count,
                                                const double *    support,
                                                int               update_only)
{
  unsigned int i;

  if (!splits_hash)
  {
    /* create new hashtable */
    splits_hash = hash_init(tip_count * 10, tip_count);
    /* hashtable is empty, so update_only doesn't make sense here */
    update_only = 0;
  }

  if (!splits_hash) { return CORAX_FAILURE; }

  /* insert splits */
  for (i = 0; i < split_count; ++i)
  {
    if (update_only)
    {
      hash_update(splits[i],
                  splits_hash,
                  HASH_KEY_UNDEF,
                  support ? support[i] : 1.0,
                  0);
    }
    else
    {
      hash_insert(splits[i],
                  splits_hash,
                  splits_hash->entry_count,
                  HASH_KEY_UNDEF,
                  support ? support[i] : 1.0,
                  0);
    }
  }

  return splits_hash;
}

CORAX_EXPORT bitv_hash_entry_t *
             corax_utree_split_hashtable_lookup(bitv_hashtable_t *  splits_hash,
                                                const corax_split_t split,
                                                unsigned int        tip_count)
{
  unsigned int split_len = bitv_length(tip_count);
  hash_key_t   position =
      hash_get_key(split, split_len) % splits_hash->table_size;
  bitv_hash_entry_t *p = splits_hash->table[position];

  for (; p != NULL; p = p->next)
  {
    if (!compare_splits(p->bit_vector, split, split_len)) return p;
  }

  return 0;
}

CORAX_EXPORT
void corax_utree_split_hashtable_destroy(bitv_hashtable_t *hash)
{
  if (hash) hash_destroy(hash);
}

CORAX_EXPORT corax_split_set_t * corax_utree_splitset_create(const corax_utree_t * tree){

  corax_split_set_t * split_set = (corax_split_set_t *) calloc(1, sizeof(corax_split_set_t));

  if (!split_set)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC,
                     "Cannot allocate memory for split list\n");
    return NULL;
  }

  /* init constraint */
  split_set->tip_count = tree->tip_count;
  split_set->split_size = sizeof(corax_split_base_t) * 8;
  split_set->split_len = bitv_length(tree->tip_count);
  split_set->split_count = tree->edge_count - tree->tip_count;
  split_set->splits = corax_utree_split_create(tree->vroot, tree->tip_count, NULL);

  if (!split_set->splits)
  {
    free(split_set);
    return NULL;
  }

  return split_set;
}

CORAX_EXPORT void corax_utree_splitset_destroy(corax_split_set_t * split_set)
{
  if (split_set)
  {
    corax_utree_split_destroy(split_set->splits);
    free(split_set->id_to_split);
    free(split_set);
  }
}

CORAX_EXPORT int corax_utree_constraint_check_splits(corax_split_set_t * cons_splits, corax_split_set_t * tree_splits)
{
  int retval = CORAX_SUCCESS;

  bitv_hashtable_t* splits_hash = corax_utree_split_hashtable_insert(NULL,
                                                                      tree_splits->splits,
                                                                      cons_splits->tip_count,
                                                                      tree_splits->split_count,
                                                                      NULL,
                                                                      0);

  for (size_t i = 0; i < cons_splits->split_count; ++i)
  {
    if (!corax_utree_split_hashtable_lookup(splits_hash, cons_splits->splits[i], cons_splits->tip_count))
    {
//      pllmod_utree_split_show(cons_splits->splits[i], cons_splits->tip_count);
//      printf("\n");
      retval = CORAX_FAILURE;
      break;
    }
  }

  corax_utree_split_hashtable_destroy(splits_hash);

  return retval;
}


CORAX_EXPORT int corax_utree_constraint_check_splits_tree(corax_split_set_t * cons_splits,
                                                         const corax_utree_t * tree)
{
  int retval = CORAX_SUCCESS;

  corax_split_set_t * tree_splits =  corax_utree_splitset_create(tree);

  truncate_splits(tree_splits, cons_splits->tip_count);

  retval = corax_utree_constraint_check_splits(cons_splits, tree_splits);

  corax_utree_splitset_destroy(tree_splits);

  return retval;
}


CORAX_EXPORT int corax_utree_constraint_check_tree(const corax_utree_t * cons_tree,
                                                  const corax_utree_t * tree)
{

  int retval = CORAX_SUCCESS;

  corax_split_set_t * cons_splits =  corax_utree_splitset_create(cons_tree);

  retval = corax_utree_constraint_check_splits_tree(cons_splits, tree);

  corax_utree_splitset_destroy(cons_splits);

  return retval;
}