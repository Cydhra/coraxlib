#include "corax/corax.h"

CORAX_EXPORT void
corax_utree_create_pars_buildops(corax_unode_t *const *trav_buffer,
                                 unsigned int          trav_buffer_size,
                                 corax_pars_buildop_t *ops,
                                 unsigned int *        ops_count)
{
  const corax_unode_t *node;
  unsigned int         i;

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
                                            unsigned int *       score)
{
  size_t         i;
  corax_utree_t *tree = NULL;

  corax_partition_t *partition = corax_partition_create(taxon_count,
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
    assert(corax_errno);
    return NULL;
  }

  /* set pattern weights and free the weights array */
  if (site_weights) corax_set_pattern_weights(partition, site_weights);

  /* find sequences in hash table and link them with the corresponding taxa */
  for (i = 0; i < taxon_count; ++i)
    corax_set_tip_states(partition, i, map, sequences[i]);

  tree = corax_utree_create_parsimony_multipart(
      taxon_count, names, 1, &partition, random_seed, score);

  /* destroy all structures allocated for the concrete PLL partition instance */
  corax_partition_destroy(partition);

  return tree;
}

/**
 * Creates a maximum parsimony topology using randomized stepwise-addition
 * algorithm. All branch lengths will be set to default.
 * This function can be used with partitioned alignments (e.g., combined DNA+AA
 * data)
 */
CORAX_EXPORT
corax_utree_t *
corax_utree_create_parsimony_multipart(unsigned int       taxon_count,
                                       const char *const *taxon_names,
                                       unsigned int       partition_count,
                                       corax_partition_t *const *partitions,
                                       unsigned int       random_seed,
                                       unsigned int *     score)
{
  return corax_utree_create_parsimony_multipart_spr(taxon_count, taxon_names,
                                                    partition_count, partitions,
                                                    0, 0,
                                                    random_seed, score);
}

/**
 * Creates a maximum parsimony topology using randomized stepwise-addition
 * and subsequent optimization with SPR rounds
 */
CORAX_EXPORT
corax_utree_t *
corax_utree_create_parsimony_multipart_spr(unsigned int       taxon_count,
                                           const char *const *taxon_names,
                                           unsigned int       partition_count,
                                           corax_partition_t *const *partitions,
                                           unsigned int       max_spr_rounds,
                                           unsigned int       spr_cost_epsilon,
                                           unsigned int       random_seed,
                                           unsigned int *     score)
{
  corax_utree_t *tree = NULL;
  unsigned int   i;

  corax_parsimony_t **parsimony = (corax_parsimony_t **)calloc(
      partition_count, sizeof(corax_parsimony_t *));

  if (!parsimony)
  {
    corax_set_error(CORAX_ERROR_MEM_ALLOC, "Unable to allocate enough memory.");
    return NULL;
  }

  for (i = 0; i < partition_count; ++i)
  {
    assert(taxon_count == partitions[i]->tips);
    parsimony[i] = corax_fastparsimony_init(partitions[i]);
    if (!parsimony[i])
    {
      assert(corax_errno);
      goto cleanup;
    }
  }

  tree = corax_fastparsimony_stepwise(
      parsimony, taxon_names, score, partition_count, random_seed);

  /* apply SPR moves to improve parsimony score */
  corax_utree_parsimony_spr(tree, partition_count, parsimony,
                            max_spr_rounds, spr_cost_epsilon, random_seed, score);

  if (tree)
  {
    /* update pmatrix/scaler/node indices */
    corax_utree_reset_template_indices(
        tree->nodes[tree->tip_count + tree->inner_count - 1], tree->tip_count);

    /* set default branch lengths */
    corax_utree_set_length_recursive(tree, CORAX_TREE_DEFAULT_BRANCH_LENGTH, 0);
  }
  else
    assert(corax_errno);

  CORAX_UNUSED(taxon_count);

cleanup:
  /* destroy parsimony */
  for (i = 0; i < partition_count; ++i)
  {
    if (parsimony[i]) corax_parsimony_destroy(parsimony[i]);
  }

  free(parsimony);

  return tree;
}

CORAX_EXPORT corax_utree_t *
corax_utree_resolve_parsimony_multipart(const corax_utree_t * multi_tree,
                                        unsigned int partition_count,
                                        corax_partition_t * const * partitions,
                                        const unsigned int * tip_msa_idmap,
                                        unsigned int max_spr_rounds,
                                        unsigned int random_seed,
                                        int * clv_index_map,
                                        unsigned int * score)
{
  int retval = CORAX_FAILURE;
  unsigned int i;

  corax_utree_t * tree = NULL;

  corax_parsimony_t ** parsimony =
      (corax_parsimony_t **) calloc(partition_count, sizeof(corax_parsimony_t *));

  if (!parsimony)
  {
    corax_errno = CORAX_ERROR_MEM_ALLOC;
    snprintf(corax_errmsg, 200, "Unable to allocate enough memory.");
    return NULL;
  }

  for (i = 0; i < partition_count; ++i)
  {
    parsimony[i] = corax_fastparsimony_init(partitions[i]);
    if (!parsimony[i])
    {
      assert(corax_errno);
      goto cleanup;
    }
  }

  
  /* first, resolve multifurcations randomly */
  tree = corax_utree_random_resolve_multi(multi_tree, random_seed, clv_index_map);

  if (!tree)
    goto cleanup;

  /* if constraint tree was not fully resolved, apply SPR moves to improve parsimony score */
  if (!multi_tree->binary)
  {
    retval = corax_utree_parsimony_spr_constrained(tree, partition_count, parsimony,
                                                   tip_msa_idmap, clv_index_map,
                                                   max_spr_rounds, 0,
                                                   random_seed, score);
  }
  else
    retval = CORAX_SUCCESS;

  if (retval)
  {
//    /* update pmatrix/scaler/node indices */
//    pll_utree_reset_template_indices(tree->nodes[tree->tip_count +
//                                                 tree->inner_count - 1],
//                                     tree->tip_count);

     /* set default branch lengths */
    corax_utree_set_length_recursive(tree,
                                      CORAX_TREE_DEFAULT_BRANCH_LENGTH,
                                      0);
  }
  else
    assert(corax_errno);

  cleanup:
    /* destroy parsimony */
    for (i = 0; i < partition_count; ++i)
    {
      if (parsimony[i])
        corax_parsimony_destroy(parsimony[i]);
    }

    free(parsimony);

    if (!retval && tree)
      corax_utree_destroy(tree, NULL);

  return tree;
}

CORAX_EXPORT int
corax_utree_extend_parsimony_multipart(corax_utree_t * tree,
                                       unsigned int taxon_count,
                                       char * const * taxon_names,
                                       const unsigned int * tip_msa_idmap,
                                       unsigned int partition_count,
                                       corax_partition_t * const * partitions,
                                       unsigned int random_seed,
                                       unsigned int * score)
{
  int retval = CORAX_FAILURE;
  unsigned int i;
  unsigned int total_tip_count = tree->tip_count + taxon_count;

  corax_parsimony_t ** parsimony =
      (corax_parsimony_t **) calloc(partition_count, sizeof(corax_parsimony_t *));

  if (!parsimony)
  {
    corax_errno = CORAX_ERROR_MEM_ALLOC;
    snprintf(corax_errmsg, 200, "Unable to allocate enough memory.");
    return retval;
  }

  for (i = 0; i < partition_count; ++i)
  {
    assert(total_tip_count == partitions[i]->tips);
    parsimony[i] = corax_fastparsimony_init(partitions[i]);
    if (!parsimony[i])
    {
      assert(corax_errno);
      goto cleanup;
    }
  }
  
  
  retval = corax_fastparsimony_stepwise_extend(tree, parsimony, partition_count,
                                             taxon_names, tip_msa_idmap,
                                             random_seed, score);

  if (retval)
  {
    /* update pmatrix/scaler/node indices */
    corax_utree_reset_template_indices(tree->nodes[tree->tip_count +
                                                 tree->inner_count - 1],
                                     tree->tip_count);

    assert(tree->tip_count == total_tip_count);

    /* set default branch lengths */
    corax_utree_set_length_recursive(tree,
                                      CORAX_TREE_DEFAULT_BRANCH_LENGTH,
                                      0);
  }
  else
    assert(corax_errno);

  CORAX_UNUSED(total_tip_count);

cleanup:
  /* destroy parsimony */
  for (i = 0; i < partition_count; ++i)
  {
    if (parsimony[i])
      corax_parsimony_destroy(parsimony[i]);
  }

  free(parsimony);

  return retval;
}


CORAX_EXPORT int
corax_utree_parsimony_spr(corax_utree_t * tree,
                          unsigned int          partition_count,
                          corax_parsimony_t **  parsimony,
                          unsigned int          max_spr_rounds,
                          unsigned int          spr_cost_epsilon,
                          unsigned int          random_seed,
                          unsigned int *        score)
{
  return corax_utree_parsimony_spr_constrained(tree, partition_count, parsimony,
                                               NULL, NULL,
                                               max_spr_rounds, spr_cost_epsilon,
                                               random_seed, score);
}

CORAX_EXPORT int
corax_utree_parsimony_spr_constrained(corax_utree_t * tree,
                                      unsigned int          partition_count,
                                      corax_parsimony_t **  parsimony,
                                      const unsigned int *  tip_msa_idmap,
                                      int *                 clv_index_map,
                                      unsigned int          max_spr_rounds,
                                      unsigned int          spr_cost_epsilon,
                                      unsigned int          random_seed,
                                      unsigned int *        score)
{
  int retval = CORAX_SUCCESS;

  if (max_spr_rounds)
  {
    unsigned int spr_round = 0;
    unsigned int best_score;

//    printf("max spr_rounds: %u, cost epsilon: %u, cost: %u\n", max_spr_rounds, spr_cost_epsilon, *score);

    *score = ~0;
    do
    {
      best_score = *score;
      retval = corax_fastparsimony_stepwise_spr_round(tree, parsimony, partition_count,
                                                      tip_msa_idmap, random_seed,
                                                      clv_index_map, score);
      ++spr_round;
//      printf("spr_round: %u, cost: %u\n", spr_round, *score);
    }
    while (retval && spr_round < max_spr_rounds && *score < (best_score - spr_cost_epsilon));
  }

  return retval;
}

