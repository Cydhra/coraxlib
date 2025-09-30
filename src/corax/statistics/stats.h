#ifndef CORAX_STATISTICS_STATS_H_
#define CORAX_STATISTICS_STATS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "corax/core/common.h"

/**
 * Probability Density Function of the normal distribution with given mean and standard deviation.
 * @param x point in sample space
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 */
CORAX_EXPORT inline double normal_pdf(double x, double mean, double stddev);

/**
 * Cumulative Density Function of the normal distribution with given mean and standard deviation.
 *
 * @param x point in sample space
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 */
CORAX_EXPORT inline double normal_cdf(double x, double mean, double stddev);

/**
 * Inverse CDF of the standard normal distribution (i.e., mean = 0.0, stddev = 1.0), that is, the function returns the
 * quantile of the CDF given a probability.
 *
 * @param p probability
 * @return the quantile of the standard normal distribution at the probability p, or NAN if p is not a valid probability.
 */
CORAX_EXPORT inline double normal_quantile(double p);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //CORAX_STATISTICS_STATS_H_