#include "stats.h"

#include <math.h>
#include "erfinv.h"

CORAX_EXPORT inline double normal_pdf(const double x, const double mean, const double stddev) {
    return 1.0 / (stddev * sqrt(2.0 * M_PI)) * exp(-0.5 * pow((x - mean) / stddev, 2));
}

CORAX_EXPORT inline double normal_cdf(const double x, const double mean, const double stddev) {
    return 0.5 * (1 + erf((x - mean) / (stddev * sqrt(2.0))));
}

CORAX_EXPORT inline double normal_quantile(const double p) {
    if (p < 0.0 || p > 1.0) {
        return NAN;
    }

    return -(M_SQRT2 * erfinv(2.0 * p));
}