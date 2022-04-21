#ifndef CONSOLE_H
#define CONSOLE_H

#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// DOS color order, but VT100 escape codes:
#define  COLOR_BLACK          "\x1b[0;30m"
#define  COLOR_BLUE           "\x1b[0;34m"
#define  COLOR_GREEN          "\x1b[0;32m"
#define  COLOR_CYAN           "\x1b[0;36m"
#define  COLOR_RED            "\x1b[0;31m"
#define  COLOR_MAGENTA        "\x1b[0;35m"
#define  COLOR_YELLOW         "\x1b[0;33m"
#define  COLOR_WHITE          "\x1b[0;37m"

#define  COLOR_BRIGHTBLACK    "\x1b[1;30m"
#define  COLOR_DARKGRAY       COLOR_BRIGHTBLACK

#define  COLOR_BRIGHTBLUE     "\x1b[1;34m"
#define  COLOR_BRIGHTGREEN    "\x1b[1;32m"
#define  COLOR_BRIGHTRED      "\x1b[1;31m"
#define  COLOR_BRIGHTCYAN     "\x1b[1;36m"
#define  COLOR_BRIGHTMAGENTA  "\x1b[1;35m"
#define  COLOR_BRIGHTWHITE    "\x1b[1;37m"
#define  COLOR_BRIGHTYELLOW   "\x1b[1;33m"

#define  COLOR_NONE           "\x1b[0m"

// Thanks to @RichardHansen: https://stackoverflow.com/a/11172679/14055985
// This provides a '##' free version for macro comma separation:
#define _VA_FIRST(...) _VA_FIRST_HELPER(__VA_ARGS__, throwaway)
#define _VA_FIRST_HELPER(first, ...) first

#define _VA_REST(...) _VA_REST_HELPER(_VA_NUM(__VA_ARGS__), __VA_ARGS__)
#define _VA_REST_HELPER(qty, ...) _VA_REST_HELPER2(qty, __VA_ARGS__)
#define _VA_REST_HELPER2(qty, ...) _VA_REST_HELPER_##qty(__VA_ARGS__)
#define _VA_REST_HELPER_ONE(first)
#define _VA_REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define _VA_NUM(...) \
    _VA_SELECT_32ND(__VA_ARGS__, \
			TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, \
			TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, \
			TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, \
			TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, \
			TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, \
			ONE, throwaway)
#define _VA_SELECT_32ND(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
			a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
			a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, ...) a32

// Basically follow the Linux kernel's pr_ message values, but pr_emerg is BUG.
// rc_config.verbose = 0 will suppress all output except for BUGs.
#define BUG(...)       _xnec2c_printf(PR_BUG,    __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_alert(...)  _xnec2c_printf(PR_ALERT,  __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_crit(...)   _xnec2c_printf(PR_CRIT,   __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_err(...)    _xnec2c_printf(PR_ERR,    __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_warn(...)   _xnec2c_printf(PR_WARN,   __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_notice(...) _xnec2c_printf(PR_NOTICE, __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_info(...)   _xnec2c_printf(PR_INFO,   __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))
#define pr_debug(...)  _xnec2c_printf(PR_DEBUG,  __FILE__, __func__, __LINE__, _VA_FIRST(__VA_ARGS__)  _VA_REST(__VA_ARGS__))

// Funny {} braces are to deal with else cases that generate compiler warnings:
#define BUG_ON(expr, ...) { if (expr) BUG(__VA_ARGS__) }

enum 
{
	PR_BUG,
	PR_ALERT,
	PR_CRIT,
	PR_ERR,
	PR_WARN,
	PR_NOTICE,
	PR_INFO,
	PR_DEBUG
};

extern const char *colors[];
extern const char *pr_levels[];
extern const char *pr_colors[];

extern rc_config_t rc_config;

void _xnec2c_printf(int level, const char *file, const char *func, const int line, char *format, ...);
#endif
