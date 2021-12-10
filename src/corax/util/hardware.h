#ifndef CORAX_UTIL_HARDWARE_H_
#define CORAX_UTIL_HARDWARE_H_

#include "corax/core/common.h"

typedef struct corax_hardware_s
{
  int init;
  /* cpu features */
  int altivec_present;
  int mmx_present;
  int sse_present;
  int sse2_present;
  int sse3_present;
  int ssse3_present;
  int sse41_present;
  int sse42_present;
  int popcnt_present;
  int avx_present;
  int avx2_present;

  /* TODO: add chip,core,mem info */
} corax_hardware_t;

CORAX_EXPORT extern __thread corax_hardware_t corax_hardware;

#ifdef __cplusplus
extern "C"
{
#endif

  /* functions in hardware.c */

  CORAX_EXPORT int corax_hardware_probe(void);

  CORAX_EXPORT void corax_hardware_dump(void);

  CORAX_EXPORT void corax_hardware_ignore(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CORAX_UTIL_HARDWARE_H_ */
