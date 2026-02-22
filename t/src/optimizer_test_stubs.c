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

/* Simplified memory stubs for test isolation.
 * The real implementation in utils.c prepends a tracking header;
 * tests use plain allocators since no tracking is needed. */

void
mem_alloc(void **ptr, size_t req, gchar *str)
{
	(void)str;
	*ptr = calloc(1, req);
}

void
mem_realloc(void **ptr, size_t req, gchar *str)
{
	(void)str;
	*ptr = realloc(*ptr, req);
}

void
free_ptr(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}
