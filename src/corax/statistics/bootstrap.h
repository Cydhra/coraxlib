#ifndef CORAX_STATISTICS_BOOTSTRAP_H_
#define CORAX_STATISTICS_BOOTSTRAP_H_

#ifdef __cplusplus
extern "C" {



#endif

#include "corax/util/random.h"
#include "corax/corax_core.h"

/**
 * Allocate a test-statistics matrix for the designated number of trees and replicates.
 * This method can be used when pre-allocating matrices for a designated number of trees that is larger than individual
 * calls to `corax_RELL_bootstrap`.
 * If the number of trees `corax_RELL_bootstrap` is called with matches the number of trees in the matrix,
 * `corax_RELL_bootstrap` can allocate the memory itself.
 *
 * @param matrix the output parameter for the newly allocated matrix
 * @param num_trees the number of trees (rows) of the matrix
 * @param num_replicates the number of bootstrap replicates (columns) of the matrix
 */
CORAX_EXPORT int corax_RELL_allocate_matrix(double **matrix,
                                            unsigned int num_trees,
                                            unsigned int num_replicates);

/**
 * Allocate a list of pointers which will hold the matrices of the multiscale RELL bootstrap procedure.
 * Allocates enough space to hold exactly `num_scales` pointers, and then allocates the matrices using
 * the num_replicates array to determine how many columns each matrix has.
 * This method can be used when pre-allocating matrices for a designated number of trees that is larger than arguments
 * to individual calls to `corax_RELL_bootstrap`.
 *
 * @param matrices the output parameter for the newly allocated list which will hold the matrices.
 * @param num_trees the number of input trees
 * @param num_replicates the number of replicates for each individual scaling factor
 * @param num_scales the number of scaling factors, which will determined how many matrices need to be held in the list
 */
CORAX_EXPORT int corax_RELL_allocate_multiscale_matrices(double ***matrices,
                                                         unsigned int num_trees,
                                                         const unsigned int *num_replicates,
                                                         unsigned int num_scales);

/**
 * Extract a matrix view starting at a row offset, splitting the original matrix into submatrices.
 *
 * @param matrix the test statistics matrix containing bootstrap replicates
 * @param start_tree the id of the tree in the matrix where the new matrix view should start
 * @param num_replicates the number of bootstrap replicates in the matrix
 * @return a pointer to a view of the matrix starting at the `start_tree`-th row
 */
CORAX_EXPORT double *corax_RELL_submatrix(double *matrix,
                                          unsigned int start_tree, unsigned int num_replicates);

/**
 * Perform general RELL bootstrap on a set of tree log-likelihood vectors.
 * The log-likelihood vectors are sampled with replacement numReplicates * scale times, generating numReplicates
 * log-likelihood replicates per tree.
 * The replicates are stored in a matrix, which contains numReplicates replicates per row and one row per tree.
 *
 * @param rstate the state of a random number generator to generate sample distributions. Note that the state has to be
 *               equal on all participating threads to generate correct bootstrap replicates.
 * @param replicates the out-parameter for the matrix where to store bootstrap replicates. The matrix needs space for
 *                   num_replicates * num_trees double precision numbers. Alternatively a nullptr can be passed to the
 *                   function, which will allocate the space for the caller. In any case, it is the caller's responsibility
 *                   to free the memory.
 * @param trees_persite_lnl the matrix of per-site log-likelihoods of input trees, one row of likelihoods per tree
 * @param site_weights the per-site weights of pattern compression
 * @param num_sites_uncompressed the number of sites of the original alignment (i.e., the length of the per-site lnl
 *                               vectors before pattern compression)
 * @param num_sites_compressed the number of sites in the alignment after pattern compression (i.e., the length of the
 *                             site_weights vectors
 * @param num_replicates the number of replicates to generate
 * @param num_trees the number of input trees
 * @param scale the scaling factor of the RELL bootstrap method, 1.0 being canonical RELL bootstrap generating replicates
 *              of the same sequence length.
 */
CORAX_EXPORT void corax_RELL_bootstrap(corax_random_state *rstate,
                                       double **replicates,
                                       const double *const *trees_persite_lnl,
                                       const unsigned int *site_weights,
                                       unsigned int num_sites_uncompressed,
                                       unsigned int num_sites_compressed,
                                       unsigned int num_replicates,
                                       unsigned int num_trees,
                                       double scale);

/**
 * Perform general RELL bootstrap on a set of tree log-likelihood vectors.
 * The log-likelihood vectors are sampled with replacement numReplicates * scale times, generating numReplicates
 * log-likelihood replicates per tree per scale.
 * The replicates are stored in matrices, which contain numReplicates replicates per row and one row per tree.
 * There is one matrix per tree.
 *
 * @param rstate the state of a random number generator to generate sample distributions. Note that the state has to be
 *               equal on all participating threads to generate correct bootstrap replicates.
 * @param replicate_matrices the out-parameter for an array of matrices of bootstrap replicates. Each matrix needs space
 *                           for num_replicates[i] * num_trees double precision numbers. Alternatively a nullptr can be
 *                           passed to the function, which will allocate the space for the caller. In any case, it is
 *                           the caller's responsibility to free both the matrices, and the pointer array.
 * @param trees_persite_lnl the matrix of per-site log-likelihoods of input trees, one row of likelihoods per tree
 * @param site_weights the per-site weights of pattern compression
 * @param num_sites_uncompressed the number of sites of the original alignment (i.e., the length of the per-site lnl
 *                               vectors before pattern compression)
 * @param num_sites_compressed the number of sites in the alignment after pattern compression (i.e., the length of the
 *                             site_weights vectors
 * @param num_trees the number of trees in the persite_lnl array
 * @param num_replicates an array defining how many bootstrap replicates to generate per scale
 * @param scales an array of scaling factors for the multiscale bootstrap
 * @param num_scales the number of entries in the scales array.
 */
CORAX_EXPORT void corax_RELL_multiscale_bootstrap(corax_random_state *rstate,
                                                  double ***replicate_matrices,
                                                  const double *const *trees_persite_lnl,
                                                  const unsigned int *site_weights,
                                                  unsigned int num_sites_uncompressed,
                                                  unsigned int num_sites_compressed,
                                                  unsigned int num_trees,
                                                  const unsigned int *num_replicates,
                                                  const double *scales,
                                                  unsigned int num_scales);

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
 * Return the median normalized bootstrap count for the given tree, which is the expected bootstrap count under the
 * assumption that the distribution is equivalent to a normal distribution.
 * This method assumes the replicate matrix has been normalized with `corax_normalize_lnl_bootstrap`.
 *
 * @param replicates replicate array for a given scaling factor
 * @param num_replicates number of replicates at that scaling factor
 * @param tree id of the tree
 * @return the median, normalized, smoothed number of maximum likelihood trees for the tree at that scale
 */
CORAX_EXPORT double corax_bootstrap_expectation(const double *replicates,
                                                unsigned int num_replicates,
                                                unsigned int tree);

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

#endif //CORAX_STATISTICS_BOOTSTRAP_H_
