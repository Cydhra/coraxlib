#include "corax/corax.h"

PLL_EXPORT void pll_utree_create_pars_buildops(pll_unode_t *const *trav_buffer,
                                               unsigned int trav_buffer_size,
                                               pll_pars_buildop_t *ops,
                                               unsigned int *      ops_count)
{
  const pll_unode_t *node;
  unsigned int       i;

  *ops_count = 0;

  for (i = 0; i < trav_buffer_size; ++i)
  {
    node = trav_buffer[i];

    if (node->next)
    {
      ops[*ops_count].parent_score_index = node->node_index;
      ops[*ops_count].child1_score_index = node->next->back->node_index;
      ops[*ops_count].child2_score_index = node->next->next->back->node_index;

      *ops_count = *ops_count + 1;
    }
  }
}

/**
 * Creates a maximum parsimony topology using randomized stepwise-addition
 * algorithm. All branch lengths will be set to default.
 */
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
                                        unsigned int *      score)
{
  size_t       i;
  pll_utree_t *tree = NULL;

  pll_partition_t *partition = pll_partition_create(taxon_count,
                                                    0, /* number of CLVs */
                                                    states,
                                                    seq_length,
                                                    1,
                                                    1, /* pmatrix count */
                                                    1, /* rate_cats */
                                                    0, /* scale buffers */
                                                    attributes);

  if (!partition)
  {
    assert(pll_errno);
    return NULL;
  }

  /* set pattern weights and free the weights array */
  if (site_weights) pll_set_pattern_weights(partition, site_weights);

  /* find sequences in hash table and link them with the corresponding taxa */
  for (i = 0; i < taxon_count; ++i)
    pll_set_tip_states(partition, i, map, sequences[i]);

  tree = pll_utree_create_parsimony_multipart(
      taxon_count, names, 1, &partition, random_seed, score);

  /* destroy all structures allocated for the concrete PLL partition instance */
  pll_partition_destroy(partition);

  return tree;
}

/**
 * Creates a maximum parsimony topology using randomized stepwise-addition
 * algorithm. All branch lengths will be set to default.
 * This function can be used with partitioned alignments (e.g., combined DNA+AA
 * data)
 */
PLL_EXPORT
pll_utree_t *
pll_utree_create_parsimony_multipart(unsigned int            taxon_count,
                                     char *const *           taxon_names,
                                     unsigned int            partition_count,
                                     pll_partition_t *const *partitions,
                                     unsigned int            random_seed,
                                     unsigned int *          score)
{
  pll_utree_t *tree = NULL;
  unsigned int i;

  pll_parsimony_t **parsimony =
      (pll_parsimony_t **)calloc(partition_count, sizeof(pll_parsimony_t *));

  if (!parsimony)
  {
    pll_set_error(PLL_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
    return NULL;
  }

  for (i = 0; i < partition_count; ++i)
  {
    assert(taxon_count == partitions[i]->tips);
    parsimony[i] = pll_fastparsimony_init(partitions[i]);
    if (!parsimony[i])
    {
      assert(pll_errno);
      goto cleanup;
    }
  }

  tree = pll_fastparsimony_stepwise(
      parsimony, taxon_names, score, partition_count, random_seed);

  if (tree)
  {
    /* update pmatrix/scaler/node indices */
    pll_utree_reset_template_indices(
        tree->nodes[tree->tip_count + tree->inner_count - 1], tree->tip_count);

    /* set default branch lengths */
    pll_utree_set_length_recursive(tree, PLL_TREE_DEFAULT_BRANCH_LENGTH, 0);
  }
  else
    assert(pll_errno);

cleanup:
  /* destroy parsimony */
  for (i = 0; i < partition_count; ++i)
  {
    if (parsimony[i]) pll_parsimony_destroy(parsimony[i]);
  }

  free(parsimony);

  return tree;
}
