#ifndef CORAX_STATISTICS_BOOTSTRAP_H_
#define CORAX_STATISTICS_BOOTSTRAP_H_

#ifdef __cplusplus
extern "C" {



#endif

#include "corax/util/random.h"
#include "corax/corax_core.h"

typedef double ***ReplicateWeights;
typedef ReplicateWeights *MultiscaleReplicateWeights;

typedef double **BootstrapMatrix;
typedef BootstrapMatrix *MultiscaleBootstrapMatrices;

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
CORAX_EXPORT int corax_RELL_allocate_matrix(BootstrapMatrix matrix,
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
CORAX_EXPORT int corax_RELL_allocate_multiscale_matrices(MultiscaleBootstrapMatrices matrices,
                                                         unsigned int num_trees,
                                                         const unsigned int *num_replicates,
                                                         unsigned int num_scales);

/**
 * Initialize a matrix per scaling factor which contains the bootstrap weights.
 * That is, one weight vector for each bootstrap replicate which is multiplied onto each sequence which decides how often each
 * site is selected in the bootstrap process.
 * These weight vectors can be calculated by `corax_RELL_bootstrap` on the fly, but if the AU Test will be executed from
 * the same seed more than once, it can be benefitial to pre-calculate the weights as their generation is a substantial
 * part of the AU Test's runtime.
 *
 * The matrices are allocated and initialized at the address given as `stored_compressed_weights`.
 * If it is a nullpointer the method is a no-op.
 * The address can then be given to the multiscale bootstrap function to use the vectors rather than calculating them
 * from scratch.
 *
 * @param rstate corax random state seeded with the AU test seed
 * @param stored_compressed_weights the address where to store the matrices
 * @param num_sites_uncompressed length of the MSA without pattern compression
 * @param num_sites_compressed number of patterns in the MSA
 * @param site_weights partition weights for the MSA
 * @param scales vector containing the scaling factors for the multiscale bootstrap
 * @param num_replicates vector containing the replication factors for the multiscale bootstrap
 * @param num_scales number of scaling factors
 * @return CORAX_SUCCESS if the allocation succeeded, CORAX_FAILURE otherwise.
 */
CORAX_EXPORT int corax_RELL_initialize_weight_matrices(corax_random_state *rstate,
                                                       MultiscaleReplicateWeights stored_compressed_weights,
                                                       unsigned int num_sites_uncompressed,
                                                       unsigned int num_sites_compressed,
                                                       const unsigned int *site_weights,
                                                       const double *scales,
                                                       const unsigned int *num_replicates,
                                                       unsigned int num_scales);

/**
 * Delete the allocations for the MultiscaleReplicateWeights matrices allocated in `corax_RELL_initialize_weight_matrices`.
 * If the pointer is a nullptr, this function is a no-op.
 *
 * @param stored_compressed_weights address of multiscale bootstrap matrices allocated in `corax_RELL_initialize_weight_matrices`
 * @param num_replicates vector containing the replication factors for the multiscale bootstrap
 * @param num_scales number of scaling factors of the multiscale bootstrap
 */
void corax_RELL_delete_weight_matrices(MultiscaleReplicateWeights stored_compressed_weights,
                                       const unsigned int * num_replicates,
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
                                          unsigned int start_tree,
                                          unsigned int num_replicates);

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
 * @param stored_compressed_weights an out-parameter for a matrix of pattern-compressed replicate weights. If the
 *                                  AU-Test is to be repeated with the same seed, the generated weights can be reused.
 *                                  If this parameter is a null-ptr, weights are not stored. If it is an address to
 *                                  an array of num_replicates null-ptrs, bootstrap will store the compressed replicate
 *                                  weights in the array. If it is an address to an allocated array of replicate weight
 *                                  arrays, bootstrap will reuse this array.
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
 *
 * @return CORAX_SUCCESS on successful execution, CORAX_FAILURE when allocation fails.
 */
CORAX_EXPORT int corax_RELL_bootstrap(corax_random_state *rstate,
                                      BootstrapMatrix replicates,
                                      ReplicateWeights stored_compressed_weights,
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
* @param stored_compressed_weights an out-parameter for a matrix of pattern-compressed replicate weights. If the
 *                                 AU-Test is to be repeated with the same seed, the generated weights can be reused.
 *                                 If this parameter is a null-ptr, weights are not stored. If it is an address to
 *                                 an array of num_replicates null-ptrs, an array of pointers to per-scale arrays will
 *                                 be created.
 *                                 Each scale's bootstrap will store the compressed replicate
 *                                 weights in the provided arrays.
 *                                 If it is an address to an allocated array of replicate weight
 *                                 arrays, bootstrap will reuse this array.
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
 *
 * @return CORAX_SUCCESS on successful execution, CORAX_FAILURE when allocation fails.
 */
CORAX_EXPORT int corax_RELL_multiscale_bootstrap(corax_random_state *rstate,
                                                 MultiscaleBootstrapMatrices replicate_matrices,
                                                 MultiscaleReplicateWeights stored_compressed_weights,
                                                 const double *const *trees_persite_lnl,
                                                 const unsigned int *site_weights,
                                                 unsigned int num_sites_uncompressed,
                                                 unsigned int num_sites_compressed,
                                                 unsigned int num_trees,
                                                 const unsigned int *num_replicates,
                                                 const double *scales,
                                                 unsigned int num_scales);

/**
 * Find the maximum likelihood for each replicate in the given replicate matrix.
 * The maximum is written into an array which is allocated at the address given as `maximum`.
 * It is the callers responsibility to free the allocation inside `maximum` after the call to
 * `corax_calculate_bp_distribution`.
 * This call cannot be performed in parallel.
 * For a parallel implementation of the AU test, this call has to be performed by a single thread, after which
 * the thread group can perform `corax_calculate_bp_distribution` in parallel.
 * If a single thread is performing the calculation, the convenience-method `corax_normalize_lnl_bootstrap` performs this step
 * and the calculation of bp distributions at once.
 *
 * @param replicates replicate matrix
 * @param num_replicates number of replicates in the matrix
 * @param num_trees number of trees in the matrix
 * @param maximum out parameter for storing the maximum likelihood for each replicate
 * @return success, if allocation of the outparameter succeeded.
 */
CORAX_EXPORT int corax_find_replicate_maximum(const double * replicates,
                                              unsigned int num_replicates,
                                              unsigned int num_trees,
                                              const double **maximum);

/**
 * Calculate the bp distribution for each tree in the replicate matrix in the range [`start_tree`, `start_tree` + `num_trees`).
 * If a single thread is performing this calculation, simply give the interval [0, num_trees).
 * This calculation can be performed in parallel by giving non-overlapping intervals.
 * The results are stored in the `target` matrix, which can be the same as `replicates` for in-place calculation.
 * If a single thread is performing the calculation, the convenience-method `corax_normalize_lnl_bootstrap` performs this step
 * and the calculation of `maximum`.
 *
 * @param replicates replicate matrix
 * @param target output matrix; can be the same as replicates for in-place operation
 * @param num_replicates number of replicates in the matrix
 * @param start_tree offset within the range of trees at which to start calculating the distributions. Set to 0 and give
 *                   the number of input trees in the `num_trees` parameter to calculate all distributions. Give a range
 *                   in those two parameters
 * @param num_trees number of trees beginning from `start_tree` to include in this calculation
 * @param maximum per-replicate likelihood maximum. Calculated in `corax_find_replicate_maximum`. This state will not
 * be freed by this method.
 */
CORAX_EXPORT void corax_calculate_bp_distribution(const double *replicates,
                                                 double *target,
                                                 unsigned int num_replicates,
                                                 unsigned int start_tree,
                                                 unsigned int num_trees,
                                                 const double *maximum);

/**
 * Normalize a matrix of log-likelihood replicates in such a way that the maximum likelihood replicate of each set of
 * replicates is represented with the negative likelihood difference to the next-best replicate.
 * All suboptimal replicates have the positive difference to the maximum likelihood in their entry.
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
 * @param target the target matrix which will be filled with the likelihood difference of each replicate. It has to have
 *               the same size as the replicates matrix, but the entries will be sorted per-row.
 * @param num_replicates the number of replicates per row
 * @param num_trees the number of trees (rows) in the matrix
 *
 * @return CORAX_SUCCESS on successful execution, CORAX_FAILURE when allocation fails.
 */
CORAX_EXPORT int corax_normalize_lnl_bootstrap(const double *replicates,
                                               double *target,
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
