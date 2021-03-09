#include <stdarg.h>

#include "corax.h"

/**
 * @brief Set pll error (pll_errno and pll_errmsg)
 *
 * @param[in] errno the error code
 * @param[in] errmsg_fmt formatted error message
 */
__attribute__((format(printf, 2, 3))) void
pll_set_error(int errno, const char *errmsg_fmt, ...)
{
  pll_errno = errno;

  va_list args;
  va_start(args, errmsg_fmt);
  vsnprintf(pll_errmsg, PLL_ERRMSG_LEN, errmsg_fmt, args);
  va_end(args);
}

/**
 * Reset pll error and messages.
 *
 * Call this function within operations whose error status depends on
 * `pll_errno` such that no error leaks in from previous operations.
 */
void pll_reset_error()
{
  pll_errno = 0;
  strcpy(pll_errmsg, "");
}
