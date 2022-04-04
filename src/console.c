#include "console.h"

const char *colors[] = {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_YELLOW,
	COLOR_WHITE,

	COLOR_DARKGRAY,
	COLOR_BRIGHTBLUE,
	COLOR_BRIGHTGREEN,
	COLOR_BRIGHTRED,
	COLOR_BRIGHTCYAN,
	COLOR_BRIGHTMAGENTA,
	COLOR_BRIGHTWHITE,
	COLOR_BRIGHTYELLOW,
	COLOR_NONE,
	NULL
};

const char *pr_levels[] = 
{
	[PR_BUG] = "BUG",
	[PR_ALERT] = "alert",
	[PR_CRIT] = "crit",
	[PR_ERR] = "err",
	[PR_WARN] = "warn",
	[PR_NOTICE] = "notice",
	[PR_INFO] = "info",
	[PR_DEBUG] = "debug",
	NULL
};

const char *pr_colors[] = {
	COLOR_BRIGHTRED,    	// BUG
	COLOR_BRIGHTRED,   	 	// ALERT
	COLOR_BRIGHTYELLOW, 	// CRIT
	COLOR_RED,				// ERR
	COLOR_YELLOW,			// WARN
	COLOR_BRIGHTCYAN,		// NOTICE
	COLOR_BRIGHTBLUE,		// INFO
	COLOR_BRIGHTWHITE,		// DEBUG
	NULL
};

/*
 * Provide detailed output if level <= 1 or if debug is enabled.
 * Always print if debug is enabled.
 * Print backtraces if debug >= 2.
 */
inline void _xnec2c_printf(int level, const char *file, const char *func, const int line, char *format, ...) 
{
	va_list args;
	if (rc_config.verbose < level)
		return;

	va_start(args, format);
	if (rc_config.debug || level <= PR_ALERT)
		fprintf(stderr, "%s[%d %s:%s]%s\t%s:%d: ",
			pr_colors[level], getpid(), pr_levels[level], func, 
			COLOR_NONE,
			file, line);
	else
		fprintf(stderr, "%s[%s]%s ",
			pr_colors[level], pr_levels[level], COLOR_NONE);

	// Try to use the local language if possible:
	vfprintf(stderr, _(format), args);

#ifdef HAVE_BACKTRACE
	// Temporarily set PR_DEBUG for print_backtrace.
	// print_backtrace uses PR_DEBUG so prevent recursion.
	// Always backtrace if there is a BUG.
	if (level == PR_BUG || (level != PR_DEBUG && rc_config.debug >= 2))
	{
		int oldverbose = rc_config.verbose;
		rc_config.verbose = PR_DEBUG;
		print_backtrace(NULL);
		fprintf(stderr, "\n");
		rc_config.verbose = oldverbose;
	}
#endif

	va_end(args);
}

