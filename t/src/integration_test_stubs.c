/*
 * Integration Test Stubs
 * Minimal stub implementations for GUI and runtime functions
 * These functions are called by input.c but not needed for parsing tests
 */

#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "common.h"

/* Global variable stubs */
int need_structure_redraw = 0;
int need_rdpat_redraw = 0;

/* Stub for backtrace printing */
void
print_backtrace(char *msg)
{
  /* no-op for integration tests */
}

/* Stub for GUI builder object accessor */
GtkWidget *
Builder_Get_Object(GtkBuilder *builder, const gchar *name)
{
  return NULL;
}

/* Stub for child process check */
gboolean
isChild(void)
{
  return FALSE;
}

/* Stub for window title updates */
void
Update_Window_Titles(void)
{
}

/* Stub for radiation pattern buffer allocation */
void
Alloc_Rdpattern_Buffers(int nfrq, int nth, int nph)
{
}

/* Stub for per-fstep impedance array allocation */
void
Alloc_Impedance_Buffers(int nfrq, int n_ports)
{
}

/* Stub for per-fstep impedance array free */
void
Free_Impedance_Buffers(void)
{
}

/* Stub for patch data allocation */
void
New_Patch_Data(void)
{
}

/* Stubs for prerender derivations triggered at end of command parsing;
 * the parsing test asserts raw geometry only, not prerender outputs. */
void
prerender_state_alloc(int steps_total)
{
  (void)steps_total;
}

void
compute_nf_dr_norm(void)
{
}

void
compute_excitation_center(void)
{
}

void
init_geometry_colors(void)
{
}

void
compute_trig_tables(void)
{
}

void
compute_ff_topology(void)
{
}

/* Stub for polarization factor calculation */
double
Polarization_Factor(int pol_type, int fstep, int idx)
{
  return 0.0;
}

/* Stub for widget destruction */
void
Gtk_Widget_Destroy(GtkWidget **widget)
{
}

/* Stub for per-frequency-step crnt buffer allocation */
void
Alloc_Crnt_Fstep_Buffers(int nfrq)
{
}

/* Stub for per-frequency-step crnt buffer free */
void
Free_Crnt_Fstep_Buffers(void)
{
}

/* Stub for per-frequency-step nearfield buffer allocation */
void
Alloc_Nearfield_Fstep_Buffers(int nfrq)
{
}

/* Stub for per-frequency-step nearfield buffer free */
void
Free_Nearfield_Fstep_Buffers(void)
{
}

/* Stub for frequency loop */
gboolean
Frequency_Loop(gpointer udata)
{
  return FALSE;
}

/* Stub for synchronous sweep; mathlib.c benchmark path only */
gboolean
freq_loop_run_sync(void)
{
  return FALSE;
}

/* Stub for the prompting notice; declines so the benchmark never starts */
int
Notice_Question(GtkButtonsType buttons, const char *title,
    const char *question, const char *msg_fmt, ...)
{
  return GTK_RESPONSE_NO;
}

/* Stub for the processor budget; parsing tests run single threaded */
int
xnec2c_num_procs(void)
{
  return 1;
}

/* Stub for the OpenMP thread budget */
void
xnec2c_set_omp_threads(int threads)
{
}

/* Stub for structure drawing initialization */
void
Init_Struct_Drawing(void)
{
}

/* Stub for frequency loop stop */
void
Stop_Frequency_Loop(void)
{
}

/* Stub for error dialog creation */
GtkWidget *
create_error_dialog(GtkBuilder **builder)
{
  return NULL;
}

/* Stub for Stop function */
int
Stop(int err, const char *format, ...)
{
  char mesg[1024];
  va_list args;

  va_start(args, format);
  vsnprintf(mesg, sizeof(mesg), format, args);
  va_end(args);

  fprintf(stderr, "  [STUB] Stop() called: %s\n", mesg);
  return err;
}

/* Stub for Notice function */
int
Notice(GtkButtonsType buttons, const char *title, const char *msg_fmt, ...)
{
  char message[1024];
  va_list args;

  va_start(args, msg_fmt);
  vsnprintf(message, sizeof(message), msg_fmt, args);
  va_end(args);

  printf("  [STUB] Notice() called: %s - %s\n", title, message);
  return 0;
}

/* Backtrace symbol required by the linked src/mem.c; the allocator
 * itself comes from mem.c for parity with production. */
void
_print_backtrace(char **strings)
{
  (void)strings;
}

char **
_get_backtrace(void)
{
  return NULL;
}

/* Stub for flag operations */
gboolean
isFlagSet(unsigned long long int flag)
{
  return FALSE;
}

gboolean
isFlagClear(unsigned long long int flag)
{
  return TRUE;
}

void
SetFlag(unsigned long long int flag)
{
}

void
ClearFlag(unsigned long long int flag)
{
}

/* Stub for file operations */
gboolean
Open_File(FILE **fp, char *fname, const char *mode)
{
  *fp = fopen(fname, mode);
  return (*fp != NULL);
}

void
Close_File(FILE **fp)
{
  if( fp && *fp )
  {
    fclose(*fp);
    *fp = NULL;
  }
}

/* Stub for line loading */
int
Load_Line(char *buff, FILE *pfile)
{
  if( fgets(buff, 256, pfile) == NULL )
    return EOF;
  return 0;
}

/* Locale-aware string to double conversion, mirroring src/utils.c Strtod.
 * The decimal point is read on each call so a setlocale() switch or a
 * fork() cannot leave it stale. */
double
Strtod(char *str, char **endptr)
{
  int idx;
  size_t len;
  double d = 0.0;
  char dp;
  struct lconv *lcnv;

  lcnv = localeconv();
  dp = *lcnv->decimal_point;

  len = strlen( str );
  for( idx = 0; idx < (int)len; idx++ )
    if( (str[idx] == ',') || (str[idx] == '.') )
      break;

  if( idx < (int)len ) str[idx] = dp;
  d = strtod( (const char *)str, endptr );

  return d;
}

/* Stub for string copy */
void
Strlcpy(char *dst, const char *src, size_t siz)
{
  size_t len = strlen(src);
  if( siz > 0 )
  {
    size_t copy_len = (len >= siz) ? siz - 1 : len;
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
  }
}

/* Stub for SY overrides window refresh */
void
sy_overrides_refresh(void)
{
}

/**
 * build_companion_path - real implementation for integration tests
 *
 * Duplicated here because utils.c cannot be linked due to
 * Stop/Notice symbol conflicts with integration_test_stubs.
 */
gboolean
build_companion_path(const char *src, const char *ext,
	char *buf, size_t buflen)
{
	char *dot;

	if (src == NULL || src[0] == '\0')
	{
		return FALSE;
	}

	g_strlcpy(buf, src, buflen);
	dot = strrchr(buf, '.');
	if (dot != NULL)
	{
		*dot = '\0';
	}

	g_strlcat(buf, ext, buflen);
	return TRUE;
}
