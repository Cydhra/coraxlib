 #ifndef RAXML_NG_STATISTICS_H
#define RAXML_NG_STATISTICS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "corax/util/random.h"
#include "corax/corax_core.h"

/**
 * Perform general RELL bootstrap on a set of tree log-likelihood vectors.
 * The log-likelihood vectors are sampled with replacement numReplicates * scale times, generating numReplicates
 * log-likelihood replicates per tree.
 * The replicates are stored in a matrix, which contains numReplicates replicates per row and one row per tree.
 *
 * @param replicates the out-parameter for the matrix where to store bootstrap replicates
 * @param trees_persite_lnl the matrix of per-site log-likelihoods of input trees, one row of likelihoods per tree
 * @param rstate the state of a random number generator to generate sample distributions. Note that the state has to be
 *               equal on all participating threads to generate correct bootstrap replicates.
 * @param num_sites the number of sites of the original alignment (i.e., the length of the persite lnl vectors)
 * @param num_replicates the number of replicates to generate
 * @param scale the scaling factor of the RELL bootstrap method, 1.0 being canonical RELL bootstrap generating replicates
 *              of the same sequence length.
 */
CORAX_EXPORT void corax_RELL_bootstrap(double ***replicates,
                                       double ***trees_persite_lnl,
                                       corax_random_state *rstate,
                                       int num_sites, int num_replicates, double scale);

/**
 * Normalize a matrix of log-likelihood replicates in such a way that the maximum likelihood replicate of each set of
 * replicates has likelihood 0, and all other replicates have the positive difference to the maximum likelihood in their
 * entry.
 * This is required for estimating the empirical bootstrap distribution function, i.e., the distribution of the
 * continuous BP value.
 * The canonical BP(i) value (i.e., the number of bootstrap replicates in the matrix where tree i is the maximum
 * likelihood tree) is #{L == 0} where L is the normalized likelihood.
 * The replicate vectors are then sorted per tree, meaning the BP(i) value can be simplified to a binary search for
 * the first element > 0.
 *
 * Conversely, we can calculate a quantile BP(i, t) of the empirical bootstrap distribution by searching for the first
 * element > t.
 * This represents the BP value with a margin delta-log-likelihood t, which we can use to check if the AU values converge
 * for BP(0, i).
 *
 * @param replicates the replicate matrix containing RELL-bootstrap log-likelihoods, one row per tree, numReplicates columns
 *                   per row
 * @param num_trees the number of trees (rows) in the matrix
 * @param num_replicates the number of replicates per row
 */
CORAX_EXPORT void corax_normalize_lnl_bootstrap(double ***replicates,
                                                int num_trees,
                                                int num_replicates);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //RAXML_NG_STATISTICS_H