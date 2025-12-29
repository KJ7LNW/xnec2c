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
Stop(char *mesg, int err)
{
  fprintf(stderr, "Stop called: %s (err=%d)\n", mesg, err);
  return 0;
}
