/*
 * Integration Test Stubs
 * Minimal stub implementations for GUI and runtime functions
 * These functions are called by input.c but not needed for parsing tests
 */

#include <stdio.h>
#include <stdarg.h>
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
Builder_Get_Object(GtkBuilder *builder, gchar *name)
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

/* Stub for patch data allocation */
void
New_Patch_Data(void)
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

/* Stub for nearfield buffer allocation */
void
Alloc_Nearfield_Buffers(int nrx, int nry, int nrz)
{
}

/* Stub for frequency loop */
gboolean
Frequency_Loop(gpointer udata)
{
  return FALSE;
}

/* Stub for frequency reset */
void
New_Frequency_Reset_Prev(void)
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

/* Stub for memory allocation */
void
mem_alloc(void **ptr, size_t req, gchar *str)
{
  *ptr = malloc(req);
}

/* Stub for memory reallocation */
void
mem_realloc(void **ptr, size_t req, gchar *str)
{
  *ptr = realloc(*ptr, req);
}

/* Stub for pointer freeing */
void
free_ptr(void **ptr)
{
  if( ptr && *ptr )
  {
    free(*ptr);
    *ptr = NULL;
  }
}

/* Stub for flag operations */
gboolean
isFlagSet(unsigned long long int flag)
{
  return FALSE;
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

/* Stub for string to double conversion */
double
Strtod(char *str, char **endptr)
{
  return strtod(str, endptr);
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
