/*
 * Test stubs for isolated unit testing
 * Provides minimal implementations of xnec2c functions needed by sy_expr
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "common.h"
#include "console.h"

rc_config_t rc_config = { .verbose = PR_DEBUG };

void
print_backtrace(char *msg)
{
}

int
Stop(int err, const char *format, ...)
{
  char mesg[1024];
  va_list args;

  va_start(args, format);
  vsnprintf(mesg, sizeof(mesg), format, args);
  va_end(args);

  fprintf(stderr, "Stop called: %s (err=%d)\n", mesg, err);
  return 0;
}

double
Strtod(char *nptr, char **endptr)
{
  return strtod(nptr, endptr);
}
