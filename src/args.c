/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include "args.h"
#include "mathlib.h"
#include "rc_config.h"
#include "validation_dump.h"

#include <getopt.h>

/* Zero-based cursor columns of the usage layout: option labels begin at
 * USAGE_LABEL_COL, help text and its continuations at USAGE_HELP_COL, and no
 * line extends past USAGE_WIDTH. */
#define USAGE_LABEL_COL     2
#define USAGE_HELP_COL      33
#define USAGE_WIDTH         79

typedef struct usage_entry_t usage_entry_t;

/**
 * struct usage_entry_t - One ordered row of the command-line model
 * @name: long option spelling, or NULL for standalone usage content
 * @id: getopt return value; a short option uses its own ASCII character
 * @metavar: argument placeholder, NULL when the option takes no argument
 * @text: option help, or standalone prose when @name is NULL
 * @default_arg: argument applied before parsing and rendered as the default
 * @default_resolve: computes the default in place of @default_arg
 * @target: configuration datum the applier writes
 * @apply: writes one resolved argument into the configuration
 * @notice: message announcing the applied option
 *
 * A row with a @name projects into both getopt views and the option help
 * formatter.  A row without one is inline usage content, printing @text as
 * prose, or a blank line when @text is NULL too.
 */
struct usage_entry_t {
	const char *name;
	int id;
	const char *metavar;
	const char *text;
	char *default_arg;
	char *(*default_resolve)(void);
	void *target;
	void (*apply)(const usage_entry_t *entry, char *arg);
	const char *notice;
};

enum XNEC2C_OPTS {
	// Start at 128 after all single-digit opts:
	OPT_FIRST_OPT = 128,

	OPT_ENABLE_OPTIMIZE,

	OPT_NUM_THREADS,

	OPT_WRITE_CSV,
	OPT_WRITE_S1P,
	OPT_WRITE_S2P_MAX_GAIN,
	OPT_WRITE_S2P_VIEWER_GAIN,
	OPT_WRITE_RDPAT,
	OPT_WRITE_CURRENTS,
	OPT_WRITE_GNUPLOT_STRUCTURE,
	OPT_WRITE_PATCH_CURRENTS,
	OPT_SKIP_VERIFY,
	OPT_FORCE_VERIFY,
	OPT_MEM_REPORT,
	OPT_WRITE_VALIDATION_DIR,
	OPT_WRITE_RDPAT_PNG,
	OPT_RDPAT_PNG_FORMAT,
	OPT_FREQ_SELECT,

	OPT_MAX_OPTS
};

static char *config_path_default(void);
static char *jobs_default(void);

static void apply_string_ref(const usage_entry_t *entry, char *arg);
static void apply_filename(const usage_entry_t *entry, char *arg);
static void apply_config_file(const usage_entry_t *entry, char *arg);
static void apply_flag(const usage_entry_t *entry, char *arg);
static void apply_threads(const usage_entry_t *entry, char *arg);
static void apply_jobs(const usage_entry_t *entry, char *arg);
static void apply_verbose(const usage_entry_t *entry, char *arg);
static void apply_debug(const usage_entry_t *entry, char *arg);
static void apply_quiet(const usage_entry_t *entry, char *arg);
static void apply_help(const usage_entry_t *entry, char *arg);
static void apply_version(const usage_entry_t *entry, char *arg);
static void apply_optimize(const usage_entry_t *entry, char *arg);
static void apply_validation_dir(const usage_entry_t *entry, char *arg);
static void apply_rdpat_png_format(const usage_entry_t *entry, char *arg);
static void apply_freq_select(const usage_entry_t *entry, char *arg);

static const usage_entry_t usage_entries[] = {
	{ .text = N_("Usage: xnec2c [options] [<input-file-name>]") },
	{ 0 },

	{ .name = "input",                                  .id = 'i',
	  .metavar = "<input-file-name>",
	  .text = N_("NEC2 input file to open"),
	  .target = rc_config.input_file,                   .apply = apply_filename },
	{ .name = "config",                                 .id = 'c',
	  .metavar = "<config-file-path>",
	  .text = N_("file must exist"),
	  .target = rc_config.config_file,                  .apply = apply_config_file },
	{ .name = "new-config",                             .id = 'C',
	  .metavar = "<new-config-file-path>",
	  .text = N_("creates the file when missing"),
	  .default_resolve = config_path_default,
	  .target = rc_config.config_file,                  .apply = apply_filename },
	{ .name = "jobs",                                   .id = 'j',
	  .metavar = "<count>",
	  .text = N_("child processes; 0 disables forking"),
	  .default_resolve = jobs_default,
	  .target = &calc_data.num_jobs,                    .apply = apply_jobs },
	{ .name = "batch",                                  .id = 'b',
	  .text = N_("enable batch mode, exit after the frequency loop runs"),
	  .target = &rc_config.batch_mode,                  .apply = apply_flag,
	  .notice = N_("batch mode enabled, will exit after first loop\n") },
	{ .name = "no-pthreads",                            .id = 'P',
	  .text = N_("disable pthreads and use the GTK loop for debugging"),
	  .target = &rc_config.disable_pthread_freqloop,    .apply = apply_flag,
	  .notice = N_("pthread freqloop disabled!\n") },
	{ .name = "help",                                   .id = 'h',
	  .text = N_("print usage information and exit"),
	  .apply = apply_help },
	{ .name = "version",                                .id = 'V',
	  .text = N_("print xnec2c version number and exit"),
	  .apply = apply_version },
	{ .name = "verbose",                                .id = 'v',
	  .text = N_("increase verbosity, can be specified multiple times"),
	  .apply = apply_verbose },
	{ .name = "debug",                                  .id = 'd',
	  .text = N_("enable debug output (-dd includes backtraces)"),
	  .apply = apply_debug },
	{ .name = "quiet",                                  .id = 'q',
	  .text = N_("suppress debug/verbose output"),
	  .apply = apply_quiet },
	{ 0 },

	{ .name = "threads",                                .id = OPT_NUM_THREADS,
	  .metavar = "<n>",
	  .text = N_("math library threads per job (default: processors / jobs)"),
	  .target = &calc_data.num_threads,                 .apply = apply_threads },
	{ 0 },

	{ .name = "optimize",                               .id = OPT_ENABLE_OPTIMIZE,
	  .text = N_("activate the optimizer immediately"),
	  .apply = apply_optimize },
	{ .name = "skip-verify",                            .id = OPT_SKIP_VERIFY,
	  .text = N_("skip geometry verification checks"),
	  .target = &rc_config.skip_verify_segments,        .apply = apply_flag,
	  .notice = N_("verify segments check disabled\n") },
	{ .name = "force-verify",                           .id = OPT_FORCE_VERIFY,
	  .text = N_("force overlap check on large models (>1000 segments)"),
	  .target = &rc_config.force_verify_segments,       .apply = apply_flag,
	  .notice = N_("forcing overlap check on large models\n") },
	{ .name = "mem-report",                             .id = OPT_MEM_REPORT,
	  .text = N_("report managed allocator live bytes per call site"),
	  .target = &rc_config.mem_report_enabled,          .apply = apply_flag,
	  .notice = N_("managed allocator leak report enabled\n") },
	{ 0 },

	{ .text = N_("The following arguments write to an output file after the frequency "
	  "loop completes.  These are useful to combine with --batch; if filenames are "
	  "specified without --batch, enable File->Optimizer Settings or they will not "
	  "be written.") },
	{ 0 },

	{ .name = "write-csv",                              .id = OPT_WRITE_CSV,
	  .metavar = "<filename>",
	  .text = N_("write CSV file of measurements"),
	  .target = &rc_config.filename_csv,                .apply = apply_string_ref },
	{ .name = "write-s1p",                              .id = OPT_WRITE_S1P,
	  .metavar = "<filename>",
	  .text = N_("write S1P file of S-parameters"),
	  .target = &rc_config.filename_s1p,                .apply = apply_string_ref },
	{ .name = "write-s2p-max-gain",                     .id = OPT_WRITE_S2P_MAX_GAIN,
	  .metavar = "<filename>",
	  .text = N_("write S2P file, port-2 is max-gain"),
	  .target = &rc_config.filename_s2p_max_gain,       .apply = apply_string_ref },
	{ .name = "write-s2p-viewer-gain",                  .id = OPT_WRITE_S2P_VIEWER_GAIN,
	  .metavar = "<filename>",
	  .text = N_("write S2P file, port-2 is viewer-gain"),
	  .target = &rc_config.filename_s2p_viewer_gain,    .apply = apply_string_ref },
	{ .name = "write-rdpat",                            .id = OPT_WRITE_RDPAT,
	  .metavar = "<filename>",
	  .text = N_("write CSV of the radiation pattern"),
	  .target = &rc_config.filename_rdpat,              .apply = apply_string_ref },
	{ .name = "write-currents",                         .id = OPT_WRITE_CURRENTS,
	  .metavar = "<filename>",
	  .text = N_("write CSV of currents and charges"),
	  .target = &rc_config.filename_currents,           .apply = apply_string_ref },
	{ .name = "write-gnuplot-structure",                .id = OPT_WRITE_GNUPLOT_STRUCTURE,
	  .metavar = "<filename>",
	  .text = N_("write gnuplot structure file"),
	  .target = &rc_config.filename_gnuplot_structure,  .apply = apply_string_ref },
	{ .name = "write-patch-currents",                   .id = OPT_WRITE_PATCH_CURRENTS,
	  .metavar = "<filename>",
	  .text = N_("write CSV of patch surface currents"),
	  .target = &rc_config.filename_patch_currents,     .apply = apply_string_ref },
	{ .name = "write-validation-dir",                   .id = OPT_WRITE_VALIDATION_DIR,
	  .metavar = "<directory>",
	  .text = N_("write full validation data tree"),
	  .apply = apply_validation_dir },
	{ .name = "write-rdpat-png",                        .id = OPT_WRITE_RDPAT_PNG,
	  .metavar = "<filename>",
	  .text = N_("write PNG of the radiation pattern"),
	  .target = &rc_config.filename_rdpat_png,          .apply = apply_string_ref },
	{ .name = "rdpat-png-format",                       .id = OPT_RDPAT_PNG_FORMAT,
	  .metavar = "<format[,format...]>",
	  .text = N_("x, y, z, iso, or quad views"),
	  .default_arg = "iso",
	  .target = &rc_config.rdpat_png_formats,           .apply = apply_rdpat_png_format },
	{ 0 },

	{ .name = "freq-select",                            .id = OPT_FREQ_SELECT,
	  .metavar = "<min-vswr|center|max-gain|MHz>",
	  .text = N_("post-sweep selected frequency; absent, keep the previous saved "
	  "frequency, or center if none"),
	  .apply = apply_freq_select },
};

/*------------------------------------------------------------------------*/

/**
 * config_path_default() - Compose the per-user configuration file path
 *
 * Exits when the composed path exceeds the configuration path field.
 *
 * Returns the configuration file path.
 */
static char *config_path_default(void)
{
	static char path[FILENAME_LEN] = {0};
	char home[PATH_MAX];

	get_conf_dir(home, sizeof(home));

	if( snprintf(path, sizeof(path), "%s/%s", home, DEFAULT_CONFIG_FILE) >= (int)sizeof(path) )
	{
		pr_err("config file path too long\n");
		exit(EXIT_FAILURE);
	}

	return path;
}

/**
 * jobs_default() - Resolve the child process count from the processor count
 *
 * The count is detected once and reported once, so rendering the default in
 * the usage text repeats neither the detection nor its notice.
 *
 * Returns the number of child processes to fork.
 */
static char *jobs_default(void)
{
	static char jobs[16] = {0};

	if( jobs[0] != '\0' )
		return jobs;

	int procs = xnec2c_num_procs();

	snprintf(jobs, sizeof(jobs), "%d", procs);
	pr_info("Detected %d CPUs\n", procs);

	return jobs;
}

/*------------------------------------------------------------------------*/

/**
 * usage_entry_has_arg() - Report the getopt argument mode of one option
 * @entry: option row to inspect
 *
 * The placeholder shown in the usage label states whether the option takes
 * an argument.
 *
 * Returns required_argument when the row carries a placeholder, otherwise
 * no_argument.
 */
static int usage_entry_has_arg(const usage_entry_t *entry)
{
	return entry->metavar != NULL ? required_argument : no_argument;
}

/**
 * usage_entry_is_short() - Report whether an option also spells one letter
 * @entry: option row to inspect
 *
 * Identities below OPT_FIRST_OPT are the ASCII characters of the short
 * spellings, so the identity states the kind.
 *
 * Returns TRUE when the option carries a short spelling.
 */
static gboolean usage_entry_is_short(const usage_entry_t *entry)
{
	return entry->id < OPT_FIRST_OPT;
}

/**
 * usage_entry_default() - Resolve the default argument of one option
 * @entry: option row to resolve
 *
 * A row states its default as a literal argument or as a computation of one.
 * Both the pre-parse application and the rendered usage default read this one
 * resolution, so the shown value is the value applied.
 *
 * Returns the resolved argument, NULL when the option carries no default.
 */
static char *usage_entry_default(const usage_entry_t *entry)
{
	char *arg;

	if( entry->default_resolve != NULL )
		arg = entry->default_resolve();
	else
		arg = entry->default_arg;

	return arg;
}

/*------------------------------------------------------------------------*/

/**
 * usage_model - Projection of the option table into the getopt views
 * @optstring: short option letters with their argument markers
 * @long_options: zero-terminated long option array
 * @by_id: option row of each getopt identity, NULL where no option carries one
 *
 * Every view stays zero-filled until usage_model_build() walks the table, and
 * a row reaches one only once it validates.
 */
static struct {
	char optstring[G_N_ELEMENTS(usage_entries) * 2 + 1];
	struct option long_options[G_N_ELEMENTS(usage_entries) + 1];
	const usage_entry_t *by_id[OPT_MAX_OPTS];
} usage_model;

/**
 * usage_optstring_append() - Add one short spelling to the option string
 * @entry: named row to project
 *
 * A long-only option contributes nothing, because getopt reaches it through
 * the long option array alone.
 */
static void usage_optstring_append(const usage_entry_t *entry)
{
	size_t used;

	if( !usage_entry_is_short(entry) )
		return;

	used = strlen(usage_model.optstring);
	usage_model.optstring[used++] = (char)entry->id;

	/* getopt marks an option taking an argument with a trailing colon */
	if( usage_entry_has_arg(entry) == required_argument )
		usage_model.optstring[used] = ':';
}

/**
 * usage_long_option_append() - Add one getopt record to the long options
 * @entry: named row to project
 *
 * The array holds one slot per table row, so the standalone usage rows leave
 * the zero terminator this fill point stops at.
 */
static void usage_long_option_append(const usage_entry_t *entry)
{
	struct option *slot = usage_model.long_options;

	while( slot->name != NULL )
		slot++;

	slot->name    = entry->name;
	slot->has_arg = usage_entry_has_arg(entry);
	slot->flag    = NULL;
	slot->val     = entry->id;
}

/**
 * usage_entry_index_insert() - Index one row by its getopt identity
 * @entry: named row to project
 *
 * Returns the number of faults found, one when another row already carries
 * the identity.
 */
static int usage_entry_index_insert(const usage_entry_t *entry)
{
	int faults = 0;

	if( usage_model.by_id[entry->id] != NULL )
	{
		BUG("usage entry --%s repeats the option id of --%s\n", entry->name,
		    usage_model.by_id[entry->id]->name);
		faults++;
	}
	else
		usage_model.by_id[entry->id] = entry;

	return faults;
}

/**
 * usage_entry_lookup() - Find the row owning one getopt identity
 * @id: getopt identity to resolve
 *
 * getopt also reports identities no row carries, so an identity outside the
 * indexed range resolves to no row instead of reading past the index.
 *
 * Returns the option row carrying @id, or NULL when no row carries it.
 */
static const usage_entry_t *usage_entry_lookup(int id)
{
	if( id < 1 || id >= OPT_MAX_OPTS )
		return NULL;

	return usage_model.by_id[id];
}

/**
 * usage_entry_validate() - Report one row whose fields contradict its kind
 * @entry: row to check
 * @idx: table position, naming an unnamed row in the diagnostic
 *
 * A named row is a complete getopt record; an unnamed row leaves every
 * option-only field at its zero value.  A malformed static row is reported
 * rather than silently reinterpreted.
 *
 * Returns the number of faults found in @entry.
 */
static int usage_entry_validate(const usage_entry_t *entry, size_t idx)
{
	gboolean has_default = entry->default_arg != NULL || entry->default_resolve != NULL;
	int faults = 0;

	if( entry->name != NULL )
	{
		if( entry->id < 1 || entry->id >= OPT_MAX_OPTS )
		{
			BUG("usage entry --%s carries option id %d outside 1..%d\n",
			    entry->name, entry->id, OPT_MAX_OPTS - 1);
			faults++;
		}

		if( entry->text == NULL )
		{
			BUG("usage entry --%s carries no help text\n", entry->name);
			faults++;
		}

		if( has_default && entry->metavar == NULL )
		{
			BUG("usage entry --%s defaults an argument it does not take\n", entry->name);
			faults++;
		}

		if( entry->apply == NULL )
		{
			BUG("usage entry --%s carries no applier\n", entry->name);
			faults++;
		}
	}
	else
	{
		if( entry->id != 0 || entry->metavar != NULL || has_default ||
		    entry->target != NULL ||
		    entry->apply != NULL || entry->notice != NULL )
		{
			BUG("usage entry %zu carries option fields without a name\n", idx);
			faults++;
		}
	}

	return faults;
}

/**
 * usage_model_build() - Project the option table into the getopt views
 *
 * Every named row contributes its spelling to the short option string, its
 * getopt record to the long option array, and itself to the identity index.
 *
 * Exits after reporting every malformed row, because parsing one would
 * dereference the applier it lacks.
 */
static void usage_model_build(void)
{
	int faults = 0;
	size_t idx;

	for( idx = 0; idx < G_N_ELEMENTS(usage_entries); idx++ )
	{
		const usage_entry_t *entry = &usage_entries[idx];
		int row_faults = usage_entry_validate(entry, idx);

		faults += row_faults;

		/* a standalone usage row projects into no getopt view, and a
		 * malformed row indexes none */
		if( entry->name == NULL || row_faults != 0 )
			continue;

		usage_optstring_append(entry);
		usage_long_option_append(entry);
		faults += usage_entry_index_insert(entry);
	}

	if( faults != 0 )
		exit(EXIT_FAILURE);
}

/*------------------------------------------------------------------------*/

/**
 * print_usage_wrapped() - Break prose on whitespace inside the usage columns
 * @out: stream receiving the usage text
 * @text: prose to print with the cursor already resting at @indent
 * @indent: zero-based column that begins every continuation line
 *
 * A word wider than the remaining columns prints unbroken.
 */
static void print_usage_wrapped(FILE *out, const char *text, int indent)
{
	const char *word = text;
	int col = indent;

	while( *word != '\0' )
	{
		const char *space = strchr(word, ' ');
		int len = space != NULL ? (int)(space - word) : (int)strlen(word);
		int pad = col > indent ? 1 : 0;

		if( pad != 0 && col + pad + len > USAGE_WIDTH )
		{
			fprintf(out, "\n%*s", indent, "");
			col = indent;
			pad = 0;
		}

		fprintf(out, "%*s%.*s", pad, "", len, word);
		col += pad + len;
		word += len;

		while( *word == ' ' )
			word++;
	}
}

/**
 * print_usage_option() - Print one option label beside its wrapped help
 * @out: stream receiving the usage text
 * @entry: named row to render
 *
 * A label reaching the help column takes a line of its own so the help stays
 * aligned with every other option.
 */
static void print_usage_option(FILE *out, const usage_entry_t *entry)
{
	char *label = NULL;
	char *help = NULL;
	char *dflt = usage_entry_default(entry);
	int len;

	if( usage_entry_is_short(entry) )
		label = g_strdup_printf("-%c|--%s", (char)entry->id, entry->name);
	else
		label = g_strdup_printf("--%s", entry->name);

	if( usage_entry_has_arg(entry) != no_argument )
	{
		char *with_metavar = g_strdup_printf("%s %s", label, entry->metavar);

		g_free(label);
		label = with_metavar;
	}

	len = (int)strlen(label);

	fprintf(out, "%*s%s", USAGE_LABEL_COL, "", label);
	g_free(label);

	if( USAGE_LABEL_COL + len < USAGE_HELP_COL )
		fprintf(out, "%*s", USAGE_HELP_COL - USAGE_LABEL_COL - len, "");
	else
		fprintf(out, "\n%*s", USAGE_HELP_COL, "");

	if( dflt != NULL )
		help = g_strdup_printf(_("%s (default: %s)"), _(entry->text), dflt);
	else
		help = g_strdup(_(entry->text));

	print_usage_wrapped(out, help, USAGE_HELP_COL);
	g_free(help);

	fprintf(out, "\n");
}

/**
 * print_usage_text() - Print one standalone paragraph or blank usage line
 * @out: stream receiving the usage text
 * @entry: unnamed row to render
 */
static void print_usage_text(FILE *out, const usage_entry_t *entry)
{
	/* an absent text is one blank separator line */
	if( entry->text == NULL )
	{
		fprintf(out, "\n");
		return;
	}

	print_usage_wrapped(out, _(entry->text), 0);
	fprintf(out, "\n");
}

/**
 * usage() - Print the command-line usage in table order
 * @out: stream receiving the usage text
 */
static void usage(FILE *out)
{
	size_t idx;

	for( idx = 0; idx < G_N_ELEMENTS(usage_entries); idx++ )
	{
		const usage_entry_t *entry = &usage_entries[idx];

		if( entry->name != NULL )
			print_usage_option(out, entry);
		else
			print_usage_text(out, entry);
	}
}

/*------------------------------------------------------------------------*/

static const rdpat_png_format_spec_t rdpat_png_format_names[] = {
	{ .name = "x", .format = RDPAT_PNG_FORMAT_X },
	{ .name = "y", .format = RDPAT_PNG_FORMAT_Y },
	{ .name = "z", .format = RDPAT_PNG_FORMAT_Z },
	{ .name = "iso", .format = RDPAT_PNG_FORMAT_ISO },
	{ .name = "quad", .format = RDPAT_PNG_FORMAT_QUAD },
};

_Static_assert(G_N_ELEMENTS(rdpat_png_format_names) == RDPAT_PNG_FORMAT_COUNT,
    "every radiation-pattern PNG format needs a command-line name");

/**
 * rdpat_png_format_parse() - Convert a command-line format name to its enum
 * @name: command-line format value
 * @format: destination for the resolved format
 *
 * Returns TRUE when @name names a supported radiation-pattern PNG format.
 */
static gboolean
rdpat_png_format_parse(const char *name, rdpat_png_format_spec_t *format)
{
	size_t idx;

	for( idx = 0; idx < G_N_ELEMENTS(rdpat_png_format_names); idx++ )
	{
		if( strcmp(name, rdpat_png_format_names[idx].name) == 0 )
		{
			*format = rdpat_png_format_names[idx];
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * rdpat_png_format_list_parse() - Parse radiation-pattern PNG format names
 * @names: comma-separated radiation-pattern PNG format names
 * @formats: destination for the resolved format array
 *
 * Returns TRUE when every name names a supported radiation-pattern PNG format.
 */
static gboolean
rdpat_png_format_list_parse(const char *names,
    rdpat_png_format_spec_t **formats)
{
	char **tokens;
	gsize count;
	gsize idx;
	gboolean valid = TRUE;

	tokens = g_strsplit(names, ",", -1);
	count = g_strv_length(tokens);
	mem_array_free(formats);
	mem_array_realloc(formats, count);

	for( idx = 0; idx < count; idx++ )
	{
		if( !rdpat_png_format_parse(tokens[idx], &(*formats)[idx]) )
			valid = FALSE;
	}

	g_strfreev(tokens);

	if( !valid )
		mem_array_free(formats);

	return valid;
}

static const struct { const char *name; freq_select_mode_t mode; }
	freq_select_names[] = {
		/* min-vswr is the documented spelling; min-swr is retained as an alias. */
		{ "min-vswr", FREQ_SELECT_MIN_SWR  },
		{ "min-swr",  FREQ_SELECT_MIN_SWR  },
		{ "center",   FREQ_SELECT_CENTER   },
		{ "max-gain", FREQ_SELECT_MAX_GAIN },
	};

/**
 * freq_select_keyword() - Resolve a batch capture frequency selector keyword
 * @name: command-line selector value
 * @mode: destination for the resolved selection mode
 *
 * Returns TRUE when @name names a keyword selector, leaving @mode set; FALSE
 * leaves @mode untouched for the caller to parse a numeric MHz value.
 */
static gboolean
freq_select_keyword(const char *name, freq_select_mode_t *mode)
{
	size_t idx;

	for( idx = 0; idx < G_N_ELEMENTS(freq_select_names); idx++ )
	{
		if( strcmp(name, freq_select_names[idx].name) == 0 )
		{
			*mode = freq_select_names[idx].mode;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * freq_select_number() - Resolve a numeric MHz batch capture selector
 * @arg: command-line selector value
 * @mhz: destination for the parsed MHz target
 *
 * Exits when @arg is not a bare number.  Returns FREQ_SELECT_MHZ on success.
 */
static freq_select_mode_t
freq_select_number(const char *arg, double *mhz)
{
	char *endptr;
	double val = strtod(arg, &endptr);

	if( *endptr != '\0' || endptr == arg )
	{
		pr_crit("invalid frequency selector: %s\n", arg);
		exit(1);
	}

	*mhz = val;
	return FREQ_SELECT_MHZ;
}

/**
 * parse_count() - Parse an integer option argument against its lower bound
 * @entry: option row supplying the spelling for the diagnostic
 * @arg: argument to parse
 * @min: smallest value the option accepts
 *
 * Exits when @arg is not an integer of at least @min.
 *
 * Returns the parsed count.
 */
static int parse_count(const usage_entry_t *entry, const char *arg, int min)
{
	char *endptr;
	long val = strtol(arg, &endptr, 10);

	if( *endptr != '\0' || endptr == arg || val < min )
	{
		pr_crit("--%s requires an integer argument of %d or more\n",
		    entry->name, min);
		exit(1);
	}

	return (int)val;
}

/*------------------------------------------------------------------------*/

/**
 * apply_string_ref() - Point a configuration field at the argument
 * @entry: option row naming the destination pointer
 * @arg: argument owned by the argument vector
 */
static void apply_string_ref(const usage_entry_t *entry, char *arg)
{
	*(char **)entry->target = arg;
}

_Static_assert(sizeof(rc_config.input_file) == FILENAME_LEN &&
    sizeof(rc_config.config_file) == FILENAME_LEN,
    "filename options copy into FILENAME_LEN buffers");

/**
 * apply_filename() - Copy the argument into a fixed configuration buffer
 * @entry: option row naming the destination buffer
 * @arg: filename to copy
 *
 * Exits when @arg does not fit the destination.
 */
static void apply_filename(const usage_entry_t *entry, char *arg)
{
	if( strlen(arg) >= FILENAME_LEN )
	{
		pr_crit("--%s argument too long ( > %d char )\n", entry->name,
		    FILENAME_LEN - 1);
		exit(1);
	}

	Strlcpy( entry->target, arg, FILENAME_LEN );
}

/**
 * apply_config_file() - Adopt a configuration file that already exists
 * @entry: option row naming the configuration path buffer
 * @arg: path to an existing readable configuration file
 *
 * Exits when the file is missing or unreadable.
 */
static void apply_config_file(const usage_entry_t *entry, char *arg)
{
	if( access(arg, R_OK) < 0 )
	{
		pr_crit("config file does not exist or is not readable: %s\n", arg);
		exit(1);
	}

	apply_filename(entry, arg);
}

/**
 * apply_flag() - Raise a configuration flag and announce it
 * @entry: option row naming the flag and its notice
 * @_arg: unused, a flag option takes no argument
 */
static void apply_flag(const usage_entry_t *entry, char *_arg)
{
	pr_notice("%s", _(entry->notice));
	*(int *)entry->target = 1;
}

/**
 * apply_threads() - Set the math library thread budget of one job
 * @entry: option row naming the thread count
 * @arg: requested threads per job
 *
 * A stated budget overrides the division of the processors among the
 * concurrent workers that xnec2c_threads_per_worker() performs otherwise.
 * The row carries no default, so this applier runs only for a stated argument.
 *
 * An exported thread-count variable states the same datum, and one of the two
 * settings would silently lose, so the user resolves the conflict.
 */
static void apply_threads(const usage_entry_t *entry, char *arg)
{
	const char *env = mathlib_threads_env_conflict();

	if( env != NULL )
	{
		pr_crit("--threads and %s=%s both state a thread count, unset one\n",
			env, getenv(env));
		exit(1);
	}

	*(int *)entry->target = parse_count(entry, arg, 1);
}

/**
 * apply_jobs() - Set the number of child processes to fork
 * @entry: option row naming the job count
 * @arg: requested number of child processes
 *
 * A zero count requests no forking; main() resolves that request into the
 * single in-process worker the frequency loop runs instead.
 */
static void apply_jobs(const usage_entry_t *entry, char *arg)
{
	int *jobs = entry->target;

	*jobs = parse_count(entry, arg, 0);

	if( *jobs == 0 )
		pr_notice("Forking disabled!\n");
}

/**
 * apply_verbose() - Raise the console verbosity by one level
 * @_entry: unused, the verbosity is a single well-known field
 * @_arg: unused, the option takes no argument
 */
static void apply_verbose(const usage_entry_t *_entry, char *_arg)
{
	rc_config.verbose++;
}

/**
 * apply_debug() - Enable debug output and widen the verbosity to match
 * @_entry: unused, the option spans the debug and verbosity fields
 * @_arg: unused, the option takes no argument
 */
static void apply_debug(const usage_entry_t *_entry, char *_arg)
{
	rc_config.debug++;
	rc_config.verbose += 3;
}

/**
 * apply_quiet() - Silence debug and verbose output
 * @_entry: unused, the option spans the debug and verbosity fields
 * @_arg: unused, the option takes no argument
 */
static void apply_quiet(const usage_entry_t *_entry, char *_arg)
{
	rc_config.debug = 0;
	rc_config.verbose = 0;
}

/**
 * apply_help() - Print the usage and exit
 * @_entry: unused, the usage covers the whole table
 * @_arg: unused, the option takes no argument
 */
static void apply_help(const usage_entry_t *_entry, char *_arg)
{
	usage(stdout);
	exit(0);
}

/**
 * apply_version() - Print the package version and exit
 * @_entry: unused, the version comes from the build
 * @_arg: unused, the option takes no argument
 */
static void apply_version(const usage_entry_t *_entry, char *_arg)
{
	puts( PACKAGE_STRING );
	exit(0);
}

/**
 * apply_optimize() - Activate the optimizer for this run
 * @_entry: unused, the optimizer state lives in the global flags
 * @_arg: unused, the option takes no argument
 */
static void apply_optimize(const usage_entry_t *_entry, char *_arg)
{
	SetFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
}

/**
 * apply_validation_dir() - Aim the validation dump tree at a directory
 * @_entry: unused, the dump directory owns its own storage
 * @arg: directory to fill with validation data
 */
static void apply_validation_dir(const usage_entry_t *_entry, char *arg)
{
	validation_dump_set_dir(arg);
}

/**
 * apply_rdpat_png_format() - Replace the radiation-pattern PNG format list
 * @entry: option row naming the format list
 * @arg: comma-separated format names
 *
 * Exits when any name is not a supported format.
 */
static void apply_rdpat_png_format(const usage_entry_t *entry, char *arg)
{
	if( !rdpat_png_format_list_parse(arg, entry->target) )
	{
		pr_crit("invalid radiation pattern PNG format: %s\n", arg);
		exit(1);
	}
}

/**
 * apply_freq_select() - Select the frequency reported after the sweep
 * @_entry: unused, the selection spans a mode and a target frequency
 * @arg: keyword selector or a bare MHz value
 */
static void apply_freq_select(const usage_entry_t *_entry, char *arg)
{
	if( !freq_select_keyword(arg, &rc_config.freq_select_mode) )
		rc_config.freq_select_mode = freq_select_number(arg,
		    &rc_config.freq_select_mhz);
}

/**
 * apply_option() - Route one getopt result to the option that owns it
 * @id: getopt identity of the option
 * @arg: option argument, or NULL when the option takes none
 */
static void apply_option(int id, char *arg)
{
	const usage_entry_t *entry = usage_entry_lookup(id);

	/* getopt reports unknown and malformed input as '?', which no row owns */
	if( entry == NULL )
	{
		usage(stderr);
		exit(1);
	}

	entry->apply(entry, arg);
}

/**
 * apply_option_defaults() - Apply every option default before parsing
 *
 * Command-line values reach the same appliers afterwards, so an explicit
 * argument overwrites the default applied here.
 */
static void apply_option_defaults(void)
{
	size_t idx;

	for( idx = 0; idx < G_N_ELEMENTS(usage_entries); idx++ )
	{
		const usage_entry_t *entry = &usage_entries[idx];
		char *dflt = usage_entry_default(entry);

		if( dflt == NULL )
			continue;

		entry->apply(entry, dflt);
	}
}

/**
 * args_parse() - Resolve the command line into the runtime configuration
 * @argc: argument count from main()
 * @argv: argument vector from main()
 *
 * Leaves optind addressing the first operand for the caller to consume.
 */
void args_parse(int argc, char **argv)
{
	int option;

	usage_model_build();
	apply_option_defaults();

	// default to show warnings or more important errors.  Defaults above
	// report under the louder startup verbosity that main() sets.
	rc_config.verbose = 4;

	while( (option = getopt_long(argc, argv, usage_model.optstring,
	    usage_model.long_options, NULL)) != -1 )
		apply_option(option, optarg);
}
