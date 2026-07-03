#ifndef CORAX_STATISTICS_AU_H_
#define CORAX_STATISTICS_AU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "corax/core/common.h"

/**
 * Returned by statistics function if theory collapses.
 */
#define AU_MATH_ERROR 2

/**
 * Perform the AU test on a single tree, identified by its index in the `replicates` matrix.
 * This method automatically applies a bias correction to ensure the solution is stable.
 *
 * @param replicates normalized and sorted bootstrap replicate statistics
 * @param tree the index of the test tree in the replicate matrix
 * @param scales the bootstrap scaling factors
 * @param num_replicates how many bootstrap replicates where created per scaling factor
 * @param num_scales how many scaling factors are in the `scales` array
 * @param initial_threshold the threshold where to begin the convergence test
 * @param d out-parameter for the signed distance parameter estimate
 * @param c out-parameter for the curvature estimate
 * @param p_value out-parameter for the AU p-value
 *
 * @return CORAX_SUCCESS if the AU test completed successfully, AU_MATH_ERROR if no bias correction was found to
 * stabilize the problem. The p_value parameter will be set in both cases.
 * Returns CORAX_FAILURE if memory allocation failed.
 */
CORAX_EXPORT int corax_au_p_value(double **replicates,
                                  unsigned int tree,
                                  const double *scales,
                                  const unsigned int *num_replicates,
                                  unsigned int num_scales,
                                  double initial_threshold,
                                  double *d,
                                  double *c,
                                  double *p_value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //CORAX_STATISTICS_AU_H_
