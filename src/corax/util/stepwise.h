#ifndef CORAX_UTIL_STEPWISE_H_
#define CORAX_UTIL_STEPWISE_H_

#include "corax/corax_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* functions in stepwise.c */

  CORAX_EXPORT corax_utree_t *
               corax_fastparsimony_stepwise(corax_parsimony_t **list,
                                            const char *const * labels,
                                            unsigned int *      score,
                                            unsigned int        count,
                                            unsigned int        seed);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CORAX_UTIL_STEPWISE_H_ */
