#ifndef CORAX_STATISTICS_STATS_H_
#define CORAX_STATISTICS_STATS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "erfinv.h"

/**
 * Probability Density Function of the normal distribution with given mean and standard deviation.
 * @param x point in sample space
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 */
static inline double normal_pdf(const double x, const double mean, const double stddev) {
    return 1.0 / (stddev * sqrt(2.0 * M_PI)) * exp(-0.5 * pow((x - mean) / stddev, 2));
}

/**
 * Cumulative Density Function of the normal distribution with given mean and standard deviation.
 *
 * @param x point in sample space
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 */
static inline double normal_cdf(const double x, const double mean, const double stddev) {
    return 0.5 * (1 + erf((x - mean) / (stddev * sqrt(2.0))));
}

/**
 * Inverse CDF of the standard normal distribution (i.e., mean = 0.0, stddev = 1.0), that is, the function returns the
 * quantile of the CDF given a probability.
 *
 * @param p probability
 * @return the quantile of the standard normal distribution at the probability p, or NAN if p is not a valid probability.
 */
static inline double normal_quantile(const double p) {
    if (p < 0.0 || p > 1.0) {
        return NAN;
    }

    return -(M_SQRT2 * erfinv(2.0 * p));
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //CORAX_STATISTICS_STATS_H_