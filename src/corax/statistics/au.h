#ifndef CORAX_STATISTICS_AU_H_
#define CORAX_STATISTICS_AU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "corax/core/common.h"

CORAX_EXPORT void au_p_value(double **replicates,
                             unsigned int tree,
                             double *scales,
                             unsigned int *num_replicates,
                             unsigned int num_scales,
                             unsigned int num_trees, double initial_threshold);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //CORAX_STATISTICS_AU_H_
