/*
 * Test stubs for optimizer unit tests
 * Provides minimal implementations of xnec2c functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "console.h"

rc_config_t rc_config = { .verbose = PR_DEBUG };

void
print_backtrace(char *msg)
{
	(void)msg;
}

int
Stop(int err, const char *format, ...)
{
	(void)err;
	(void)format;
	return 0;
}
