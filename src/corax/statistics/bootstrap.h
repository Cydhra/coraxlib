#ifndef RAXML_NG_STATISTICS_H
#define RAXML_NG_STATISTICS_H

#ifdef __cplusplus
extern "C" {



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
 * @param num_sites the number of sites of the original alignment (i.e., the length of the per-site lnl vectors)
 * @param num_replicates the number of replicates to generate
 * @param num_trees
 * @param scale the scaling factor of the RELL bootstrap method, 1.0 being canonical RELL bootstrap generating replicates
 *              of the same sequence length.
 */
CORAX_EXPORT void corax_RELL_bootstrap(double **replicates,
                                       double **trees_persite_lnl,
                                       corax_random_state *rstate,
                                       unsigned int num_sites, unsigned int num_replicates, unsigned int num_trees,
                                       double scale);

/**
 * Perform general RELL bootstrap on a set of tree log-likelihood vectors.
 * The log-likelihood vectors are sampled with replacement numReplicates * scale times, generating numReplicates
 * log-likelihood replicates per tree per scale.
 * The replicates are stored in matrices, which contain numReplicates replicates per row and one row per tree.
 * There is one matrix per tree.
 *
 * @param replicate_matrices the out-parameter for an array of matrices of bootstrap replicates
 * @param trees_persite_lnl the matrix of per-site log-likelihoods of input trees, one row of likelihoods per tree
 * @param rstate the state of a random number generator to generate sample distributions. Note that the state has to be
 *               equal on all participating threads to generate correct bootstrap replicates.
 * @param num_sites the number of sites of the original alignment (i.e., the length of the per-site lnl vectors)
 * @param num_trees the number of trees in the persite_lnl array
 * @param num_replicates an array defining how many bootstrap replicates to generate per scale
 * @param scales an array of scaling factors for the multiscale bootstrap
 * @param num_scales
 */
CORAX_EXPORT void corax_RELL_multiscale_bootstrap(double ***replicate_matrices,
                                                  double **trees_persite_lnl,
                                                  corax_random_state *rstate,
                                                  unsigned int num_sites, unsigned int num_trees,
                                                  unsigned int *num_replicates,
                                                  double *scales, unsigned int num_scales);

/**
 * Normalize a matrix of log-likelihood replicates in such a way that the maximum likelihood replicate of each set of
 * replicates has likelihood 0, and all other replicates have the positive difference to the maximum likelihood in their
 * entry.
 * This is required for estimating the empirical bootstrap distribution function, i.e., the distribution of the
 * continuous BP value.
 * The canonical BP(i) value (i.e., the number of bootstrap replicates in the matrix where tree i is the maximum
 * likelihood tree) is #{L == 0} where L is the normalized likelihood.
 *
 * The replicate vectors are then sorted per tree, meaning the BP(i) value can be simplified to a binary search for
 * the first element > 0.
 * Analogously, we can calculate a quantile BP(i, t) of the empirical bootstrap distribution by searching for the first
 * element > t.
 * This represents the BP value with a margin delta-log-likelihood t, which we can use to check if the AU values converge
 * for BP(0, t).
 *
 * @param replicates the replicate matrix containing RELL-bootstrap log-likelihoods, one row per tree, numReplicates columns
 *                   per row
 * @param num_replicates the number of replicates per row
 * @param num_trees the number of trees (rows) in the matrix
 */
CORAX_EXPORT

void corax_normalize_lnl_bootstrap(double *replicates,
                                   unsigned int num_replicates,
                                   unsigned int num_trees);

/**
 * Calculate the bootstrap count (i.e., the number of bootstrap replicates where the given tree is the maximum
 * likelihood trees) with a delta log-likelihood margin threshold. Every bootstrap replicate which is not smaller than
 * the maximum likelihood tree in that replicate by more than the threshold still counts towards the bootstrap count.
 *
 * To obtain the canonical BP value, calculate the bootstrap count at threshold 0.0, and divide by the number of
 * replicates.
 *
 * Note that this function returns a floating point number, because it interpolates the discrete empirical distribution
 * function induced by the bootstrap replicates into a continuous function to avoid numerical issues with the AU
 * algorithm.
 *
 * @param replicates sorted, normalized bootstrap replicates matrix
 * @param num_replicates number of replicates in the matrix
 * @param tree the index of the tree in the bootstrap matrix
 * @param threshold the delta log-likelihood margin where trees still count as maximum likelihood.
 */
CORAX_EXPORT double corax_empirical_bootstrap_count(double *replicates,
                                                    unsigned int num_replicates,
                                                    unsigned int tree,
                                                    double threshold);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //RAXML_NG_STATISTICS_H
