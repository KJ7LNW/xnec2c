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

#include "config.h"

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include <stdarg.h>

#include "utils.h"
#include "shared.h"


/*------------------------------------------------------------------------*/

/*  usage()
 *
 *  Prints usage information
 */

  void
usage(void)
{
  fprintf(stdout, _("Usage: xnec2c [options] [<input-file-name>]\n"
		"  -i|--input         <input-file-name>\n"
		"  -c|--config        <config-file-path>     - file must exist\n"
		"  -C|--new-config    <new-config-file-path> - creates the file when missing\n"
		"  -j|--jobs  <number of processors in SMP machine> (-j0 disables forking)\n"
		"     --openblas-threads <n>  set OpenBLAS thread count (default: 1)\n"
		"     --mkl-threads <n>       set Intel MKL thread count (default: 1)\n"
		"     --omp-threads <n>       set OpenMP thread count (default: 1)\n"
		"  -b|--batch:        enable batch mode, exit after the frequency loop runs\n"
		"  -P|--no-pthreads:  disable pthreads and use the GTK loop for debugging\n"
		"  -h|--help:         print usage information and exit\n"
		"  -V|--version:      print xnec2c version number and exit\n"
		"  -v|--verbose:      increase verbosity, can be specified multiple times\n"
		"  -d|--debug:        enable debug output (-dd includes backtraces)\n"
		"  -q|--quiet:        suppress debug/verbose output\n"
		"\n"
		"  --optimize:        Activate the optimizer immediately.\n"
		"  --skip-verify:     skip geometry verification checks\n"
		"  --force-verify:    force overlap check on large models (>1000 segments)\n"
		"  --mem-report:      report managed allocator live bytes per call site\n"
		"\n"
		"The following arguments write to an output file after the frequency loop\n"
		"completes.  These are useful to combine with --batch; If you wish to specify\n"
		"filenames to write without --batch mode then enable the File->Optimizer\n"
		"Settings or the files you specify on the command line will not be written.\n"
		"\n"
		"  --write-csv             <filename>  - write CSV file of measurements\n"
		"  --write-s1p             <filename>  - write S1P file of S-parameters\n"
		"  --write-s2p-max-gain    <filename>  - write S2P file, port-2 is max-gain\n"
		"  --write-s2p-viewer-gain <filename>  - write S2P file, port-2 is viewer-gain\n"
		"  --write-rdpat           <filename>  - write CSV of the radiation pattern\n"
		"  --write-currents        <filename>  - write CSV of currents and charges\n"
		"  --write-gnuplot-structure <filename> - write gnuplot structure file\n"
		"  --write-patch-currents  <filename>  - write CSV of patch surface currents\n"
		"  --write-validation-dir <directory> - write full validation data tree\n"
		"  --write-rdpat-png      <filename>  - write PNG of the radiation pattern\n"
		"  --rdpat-png-format     <format[,format...]> - x, y, z, iso, or quad views (default iso)\n"
		"\n"
		"  --freq-select <min-vswr|center|max-gain|MHz> - post-sweep selected frequency;\n"
		"                          absent, keep the previous saved frequency, or center if none\n"));

} /* end of usage() */


// May return GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, ...
int Notice(GtkButtonsType buttons, const char *title, const char *msg_fmt, ...)
{
	char message[1024];
	va_list args;
	int response;
	int locked = 0;

	va_start(args, msg_fmt);
	vsnprintf(message, sizeof(message), msg_fmt, args);
	va_end(args);

	if (!g_rec_mutex_trylock(&freq_data_lock))
		locked = 1;
	else
		g_rec_mutex_unlock(&freq_data_lock);

	if (locked || rc_config.batch_mode)
	{
		pr_err("\n=== Notice: %s ===\n%s\n\n", title, message);

		return 0;
	}

	pr_notice("\n=== Notice: %s ===\n%s\n\n", title, message);

	GtkWidget *notice = gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
		buttons,
		"%s", title);

	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(notice), "%s", message);

	response = gtk_dialog_run(GTK_DIALOG(notice));

	gtk_widget_destroy(notice);

	return response;
}

/* Deferred Notice — formatted message delivered after current GTK
 * event processing completes, avoiding re-entrant main loop corruption
 * when called from draw callbacks. */

typedef struct
{
	GtkButtonsType buttons;
	char title[128];
	char message[1024];
} deferred_notice_t;

static void _deferred_notice_cb(gpointer user_data)
{
	deferred_notice_t *dn = (deferred_notice_t *)user_data;
	Notice(dn->buttons, dn->title, "%s", dn->message);
	mem_free(&dn);
}

/**
 * Notice_Deferred() - schedule a Notice dialog after the current event
 * @buttons:  GTK button type (e.g., GTK_BUTTONS_OK)
 * @title:    dialog title
 * @msg_fmt:  printf-style format string
 *
 * Identical interface to Notice() but defers display via g_idle_add_once.
 * Safe to call from draw callbacks and render paths.
 */
void Notice_Deferred(GtkButtonsType buttons, const char *title,
	const char *msg_fmt, ...)
{
	deferred_notice_t *dn = NULL;
	mem_new(&dn);

	dn->buttons = buttons;
	snprintf(dn->title, sizeof(dn->title), "%s", title);

	va_list args;
	va_start(args, msg_fmt);
	vsnprintf(dn->message, sizeof(dn->message), msg_fmt, args);
	va_end(args);

	g_idle_add_once(_deferred_notice_cb, dn);
}

/*------------------------------------------------------------------------*/

/* Does the STOP function of fortran but with a warning dialog */
  int
Stop( int err, const char *format, ... )
{
  char mesg[1024];
  va_list args;
  GtkBuilder *builder;

  int locked = 0;

  va_start(args, format);
  vsnprintf(mesg, sizeof(mesg), format, args);
  va_end(args);

  if (!g_rec_mutex_trylock(&freq_data_lock))
    locked = 1;
  else
    g_rec_mutex_unlock(&freq_data_lock);

  pr_err("Stop: %s\n", mesg);

  /* For child processes */
  if( CHILD )
  {
    if( err )
    {
      pr_crit("%s\n", mesg);
    }
    else
    {
      pr_err("%s\n", mesg);
    }
    return( err );

  } /* if( CHILD ) */

  /* Handle batch mode gracefully */
  if (rc_config.batch_mode)
  {
    if( err )
      pr_crit("%s\n", mesg);
    else
      pr_err("%s\n", mesg);

    SetFlag(FREQ_LOOP_STOP);
    if (!locked)
      Stop_Frequency_Loop();
    /* Marshal the completion primitive onto the main thread; runs on the
     * worker thread, so it must not enter the coordinator (see
     * src/quit.c banner). */
    g_idle_add_once_sync((GSourceOnceFunc)xnec2c_quit, NULL);
    return( err );
  }

  /* During freq loop, allow loop to complete with error state */
  if (isFlagSet(FREQ_LOOP_RUNNING))
  {
    if( err )
      pr_crit("Stop during freq loop, deadlock prevented: %s\n", mesg);
    else
      pr_err("Stop during freq loop, deadlock prevented: %s\n", mesg);

    SetFlag(FREQ_LOOP_STOP);
    if (!locked)
      Stop_Frequency_Loop();
    return( err );
  }

  SetFlag(FREQ_LOOP_STOP);

  if (!locked)
    /* Stop operation */
    Stop_Frequency_Loop();


  /* Create error dialog */
  if( !error_dialog )
  {
    error_dialog = create_error_dialog( &builder );
    gtk_label_set_text( GTK_LABEL(
          Builder_Get_Object(builder, "error_label")), mesg );

    /* Hide ok button according to error */
    if( err == TRUE )
      gtk_widget_hide( Builder_Get_Object(builder, "error_okbutton") );
    gtk_widget_show( error_dialog );
    g_object_unref( builder );
  }

  /* Loop over usleep till user decides what to do */
  /* Could not think of another way to do this :-( */
  SetFlag( ERROR_CONDX );
  while( isFlagSet(ERROR_CONDX) )
  {
	// We are out of gtk_main() so iterate manually:
	while( g_main_context_iteration(NULL, FALSE) ) {}

    if( isFlagSet(MAIN_QUIT) ) exit(-1);

    usleep(100000);
  }

  return( err );
} /* Stop() */

/*------------------------------------------------------------------*/

  gboolean
Nec2_Save_Warn( const gchar *mesg )
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
    GtkBuilder *builder;
    if( !error_dialog )
    {
      error_dialog = create_error_dialog( &builder );
      gtk_label_set_text( GTK_LABEL(
            Builder_Get_Object(builder, "error_label")), mesg );
      gtk_widget_hide( Builder_Get_Object(builder, "error_stopbutton") );
      gtk_widget_show( error_dialog );
      g_object_unref( builder );
    }

    /* Loop over usleep till user decides what to do */
    /* Could not think of another way to do this :-( */
    SetFlag( ERROR_CONDX );
    while( isFlagSet(ERROR_CONDX) )
    {
      // We are out of gtk_main() so iterate manually:
      while( g_main_context_iteration(NULL, FALSE) ) {}

      if( isFlagSet(MAIN_QUIT) ) exit(-1);

      usleep(100000);
    }

    return( FALSE );
  }

  return( TRUE );
} /* Nec2_Save_Warn() */

/*------------------------------------------------------------------*/

/*  Load_Line()
 *
 *  loads a line from a file, aborts on failure. lines beginning
 *  with a '#' or ''' are ignored as comments. At the end of file
 *  EOF is returned.
 */

  int
Load_Line( char *buff, FILE *pfile )
{
  int
    num_chr, /* number of characters read, excluding lf/cr */
    eof,     /* EOF flag */
    chr;     /* character read by getc */

  num_chr = 0;
  eof     = 0;

  /* clear buffer at start */
  buff[0] = '\0';

  /* input_fp is owned elsewhere and may close mid-read via Stop() reentry */
  if( pfile == NULL )
    return( EOF );

  /* ignore commented lines, white spaces and eol/cr */
  if( (chr = fgetc(pfile)) == EOF )
    return( EOF );

  while(
      (chr == '#')  ||
      (chr == '\'') ||
      (chr == CR )  ||
      (chr == LF ) )
  {
    /* go to the end of line (look for lf or cr) */
    while( (chr != CR) && (chr != LF) )
      if( (chr = fgetc(pfile)) == EOF )
        return( EOF );

    /* dump any cr/lf remaining */
    while( (chr == CR) || (chr == LF) )
      if( (chr = fgetc(pfile)) == EOF )
        return( EOF );

  } /* end of while( (chr == '#') || ... */

  while( num_chr < LINE_LEN )
  {
    /* if lf/cr reached before filling buffer, return */
    if( (chr == CR) || (chr == LF) )
      break;

    /* enter new char to buffer */
    buff[num_chr++] = (char)chr;

    /* terminate buffer as a string on EOF */
    if( (chr = fgetc(pfile)) == EOF )
    {
      buff[num_chr] = '\0';
      eof = EOF;
    }

  } /* end of while( num_chr < max_chr ) */

  /* terminate buffer as a string */
  buff[num_chr] = '\0';

  return( eof );
} /* end of Load_Line() */

/*------------------------------------------------------------------------*/

// Scale the OpenMP resources based on the number of parallel forked jobs.
// If you fork 25 jobs and have 100 CPUs then OMP can run 4 CPUs per fork:
void xnec2c_set_omp_cpus(void)
{
#ifdef HAVE_OPENMP
	int cpus_per_job;
	int n;

	int env_num_procs = 0;

	// Respect OMP_NUM_THREADS
	if (getenv("OMP_NUM_THREADS") != NULL)
		env_num_procs = atoi(getenv("OMP_NUM_THREADS"));

	if (env_num_procs > 0)
		n = env_num_procs;
	else
		n = omp_get_num_procs();

	if (CHILD)
	{
		cpus_per_job = n / calc_data.num_jobs;

		if (cpus_per_job <= 0)
			cpus_per_job = 1;

		omp_set_num_threads(cpus_per_job);
		pr_info("using %d OpenMP threads per job (%d/%d CPUs)\n",
			getpid(),
			cpus_per_job, omp_get_num_threads(), omp_get_num_procs());
	}
	else
		omp_set_num_threads(n);
#endif
}

void clock_print_elapsed_when(char *msg, clockid_t clk_id, struct timespec *start, float min_sec)
{
	struct timespec end;
	clock_gettime(clk_id, &end);
	double elapsed = (end.tv_sec + (double)end.tv_nsec/1e9) - (start->tv_sec + (double)start->tv_nsec/1e9);

	if (elapsed > min_sec)
		pr_debug("%s[%d]: %f seconds\n",
			msg,
			getpid(),
			(end.tv_sec + (double)end.tv_nsec/1e9) - (start->tv_sec + (double)start->tv_nsec/1e9));
}

void clock_print_elapsed(char *msg, clockid_t clk_id, struct timespec *start)
{
	clock_print_elapsed_when(msg, clk_id, start, 0);
}

/*------------------------------------------------------------------------*/

/* Open_File()
 *
 * Opens a file path, returns fp
 */
  gboolean
Open_File( FILE **fp, char *fname, const char *mode )
{
  /* Close file path if open */
  Close_File( fp );
  if( (*fp = fopen(fname, mode)) == NULL )
  {
    char mesg[MESG_SIZE];
    snprintf( mesg, sizeof(mesg), _("xnec2c[%d]: %s: Failed to open file: %s\n"), getpid(), fname, strerror(errno) );
    Stop( ERR_STOP, "%s", mesg );
    return( FALSE );
  }
  else
    pr_debug("%s: %s\n", mode, fname);

  return( TRUE );
} /* Open_File() */

/*------------------------------------------------------------------------*/

/*  Close_File()
 *
 *  Closes a file pointer
 */
  void
Close_File( FILE **fp )
{
  if( *fp != NULL )
  {
	  fsync(fileno(*fp));
	  fclose(*fp);
	  *fp = NULL;
  }
} /* Close_File() */

/*------------------------------------------------------------------------*/

/* Display_Fstep()
 *
 * Displays the current frequency step number
 */
  void
Display_Fstep( GtkEntry *entry, int fstep )
{
  char str[4];

  snprintf( str, sizeof(str), "%3d", fstep );
  gtk_entry_set_text( entry, str );
}

/*------------------------------------------------------------------------*/

/* Functions for testing and setting/clearing flow control flags
 *
 *  See xnec2c.h for definition of flow control flags
 */

/* An int variable holding the single-bit flags */
static unsigned long long int Flags = 0;

  int
isFlagSet( unsigned long long int flag )
{
  return( (Flags & flag) == flag );
}

  int
isFlagClear( unsigned long long int flag )
{
  return( (~Flags & flag) == flag );
}

  void
SetFlag( unsigned long long int flag )
{
  Flags |= flag;
}

  void
ClearFlag( unsigned long long int flag )
{
  Flags &= ~flag;
}

  void
ToggleFlag( unsigned long long int flag )
{
  Flags ^= flag;
}

  void
SaveFlag( unsigned long long int *flag, unsigned long long int mask )
{
  *flag |= (Flags & mask);
}

/*------------------------------------------------------------------------*/

/* Strlcpy()
 *
 * Copies n-1 chars from src string into dest string. Unlike other
 * such library functions, this makes sure that the dest string is
 * null terminated by copying only n-1 chars to leave room for the
 * terminating char. n would normally be the sizeof(dest) string but
 * copying will not go beyond the terminating null of src string
 */
  void
Strlcpy( char *dest, const char *src, size_t n )
{
  char ch = *src;
  int idx = 0;

  /* Leave room for terminating null in dest */
  n--;

  /* Copy till terminating null of src or to n-1 */
  while( (ch != '\0') && (n > 0) )
  {
    dest[idx] = src[idx];
    idx++;
    ch = src[idx];
    n--;
  }

  /* Terminate dest string */
  dest[idx] = '\0';

} /* Strlcpy() */

/*------------------------------------------------------------------*/

/* Strlcat()
 *
 * Concatenates at most n-1 chars from src string into dest string.
 * Unlike other such library functions, this makes sure that the dest
 * string is null terminated by copying only n-1 chars to leave room
 * for the terminating char. n would normally be the sizeof(dest)
 * string but copying will not go beyond the terminating null of src
 */
  void
Strlcat( char *dest, const char *src, size_t n )
{
  char ch = *dest;
  int idd = 0; /* dest index */
  int ids = 0; /* src  index */

  /* Find terminating null of dest */
  while( (n > 0) && (ch != '\0') )
  {
    idd++;
    ch = dest[idd];
    n--; /* Count remaining char's in dest */
  }

  /* Copy n-1 chars to leave room for terminating null */
  n--;
  ch = src[ids];
  while( (n > 0) && (ch != '\0') )
  {
    dest[idd] = src[ids];
    ids++;
    ch = src[ids];
    idd++;
    n--;
  }

  /* Terminate dest string */
  dest[idd] = '\0';

} /* Strlcat() */

/*------------------------------------------------------------------*/

/* Strtod()
 *
 * Replaces strtod() to take into account the
 * locale-dependent decimal point character
 */
double Strtod( char *nptr, char **endptr )
{
  int idx;
  size_t len;
  double d = 0.0;
  char dp;
  struct lconv *lcnv;

  /* Read the active locale decimal point on each call */
  lcnv = localeconv();
  dp = *lcnv->decimal_point;

  /* Look for a . or , decimal point character
   * in the supplied number buffer (string) */
  len = strlen( nptr );
  for( idx = 0; idx < (int)len; idx++ )
    if( (nptr[idx] == ',') || (nptr[idx] == '.') )
      break;

  /* If a decimal point character is found, replace */
  if( idx < (int)len ) nptr[idx] = dp;
  d = strtod( (const char *)nptr, endptr );

  return( d );
} /* End of Strtod() */

/*------------------------------------------------------------------*/

/* str_append()
 *
 * Append a and b, store in dst.
 *
 * For example, this writes "hello world" to d:
 *    char d[20];
 *    str_append(d, "hello ", "world", 19);
 */
char *str_append(char *dst, char *a, char *b, size_t n)
{
  Strlcpy( dst, a, n );
  Strlcat( dst, b, n );

  return dst;
}

/*------------------------------------------------------------------*/

/* Get_Dirname()
 *
 * Gets the directory name from a file path
 * including the end / and returns in dirname
 */
  void
Get_Dirname( char *fpath, char *dirname, int *fname_idx )
{
  int idx, len;

  /* Get the dirname of input file to use as working directory */
  len = (int)strlen( fpath );
  for( idx = len; idx > 0; idx-- )
    if( fpath[idx] == '/' )
      break;

  /* Include end / in directory name */
  if( dirname ) Strlcpy( dirname, fpath, (size_t)(idx+2) );

  /* Return file name if pointer given */
  if( fname_idx ) *fname_idx = idx + 1;

} /* Get_Dirname() */

typedef struct 
{
	GSourceOnceFunc function;
	gpointer data;
	GMutex lock;
	GCond cond;
	int is_locked;
	char **backtrace;

	volatile int done;
} g_idle_add_data_t;


int _callback_g_idle_add_once(g_idle_add_data_t *cbdata)
{
	cbdata->function(cbdata->data);

	// If it is locked then wait for GTK and unlock it so the caller can proceed.
	//    1. If is_locked: The caller will free the memory
	//
	//    2. If !is_locked: we free it here because this is an async call
	//       and no caller will be waiting.
	if (cbdata->is_locked)
	{
		if (cbdata->backtrace != NULL)
		{
			free(cbdata->backtrace);
			cbdata->backtrace = NULL;
		}

		// Signal the waiting thread:
		g_mutex_lock(&cbdata->lock);
		cbdata->done = 1;
		g_cond_signal(&cbdata->cond);
		g_mutex_unlock(&cbdata->lock);
	}
	else
	{
		if (cbdata->backtrace != NULL)
		{
			cbdata->backtrace = NULL;
			free(cbdata->backtrace);
		}

		mem_free(&cbdata);
	}


	return FALSE;
}

guint _g_idle_add_once(GSourceOnceFunc function, gpointer data, int lock)
{
	guint ret;
	g_idle_add_data_t *cbdata = NULL;

	mem_new(&cbdata);

	// Don't do a locked sync if -P is specified because GTK
	// is already running synchronously.
	if (rc_config.disable_pthread_freqloop)
		lock = 0;

	// A synchronous wait issued from the thread that owns the GTK main
	// context would block the dispatcher that must run the scheduled
	// callback, deadlocking.  Stop() reaches its batch teardown this way
	// from Open_Input_File, which runs as a main-thread idle callback.
	// Downgrade to async so the callback runs on a later main-loop
	// iteration once the caller unwinds.
	if (lock && g_main_context_is_owner(g_main_context_default()))
		lock = 0;

	if (lock)
	{
		g_mutex_init(&cbdata->lock);
		g_cond_init(&cbdata->cond);
	}

	cbdata->function = function;
	cbdata->data = data;
	cbdata->is_locked = lock;
	cbdata->done = 0;

	cbdata->backtrace = NULL;

	// Debug async call backtraces if you need it, but be aware that _get_backtrace() is slow.
	//cbdata->backtrace = _get_backtrace();

	ret = g_idle_add((GSourceFunc)_callback_g_idle_add_once, cbdata);

	// Wait for the lock to release and free it.
	if (lock)
	{
		g_mutex_lock(&cbdata->lock);
		while (!cbdata->done)
			g_cond_wait(&cbdata->cond, &cbdata->lock);
		g_mutex_unlock(&cbdata->lock);

		g_mutex_clear(&cbdata->lock);
		g_cond_clear(&cbdata->cond);

		mem_free(&cbdata);
	}

	return ret;
}

// Call from any thread to queue a function to run once, do not wait for it to finish.
// This was implemented in glib 2.74 so use Gnome's version if available.
#if GLIB_VERSION_CUR_STABLE < G_ENCODE_VERSION(2,74)
guint g_idle_add_once(GSourceOnceFunc function, gpointer data)
{
	return _g_idle_add_once(function, data, 0); // async
}
#endif

// Call from another thread to queue a function to run once, and wait for it to finish.
guint g_idle_add_once_sync(GSourceOnceFunc function, gpointer data)
{
	return _g_idle_add_once(function, data, 1); // sync
}


/**
 * xnec2_widget_queue_draw - schedule a widget redraw through the optimizer gate
 * @w:     widget to redraw
 * @force: TRUE for user-interaction draws that must always render;
 *         FALSE for intermediate frequency-sweep draws that are
 *         suppressed during optimizer runs to avoid screen flicker
 *
 * Centralizes redraw scheduling so SUPPRESS_INTERMEDIATE_REDRAWS applies
 * uniformly to all widgets.  All callers operating on structure_drawingarea
 * or rdpattern_drawingarea must use this function rather than calling
 * gtk_widget_queue_draw() directly: those globals are swapped to GL widgets
 * when OpenGL rendering is enabled, so a direct call on any structure or
 * rdpattern GL area bypasses this gate.
 *
 * Exception: the opengl-engine/ view layer (opengl_view.c,
 * opengl_view_input.c, opengl_view_notice.c) calls gtk_widget_queue_draw()
 * directly because it redraws its own GL area for engine-internal reasons
 * (MSAA rebuild, input event repaints) that are below this gate's
 * abstraction level.
 */
/* GSourceFunc wrapper for gtk_widget_queue_draw.
 * Used by xnec2_widget_queue_draw via g_main_context_invoke_full
 * so that main-thread callers mark the widget dirty synchronously
 * while background-thread callers marshal at GDK_PRIORITY_REDRAW. */
static gboolean queue_draw_cb(gpointer w)
{
	gtk_widget_queue_draw(GTK_WIDGET(w));
	return G_SOURCE_REMOVE;
}

void xnec2_widget_queue_draw(GtkWidget *w, gboolean force)
{
	if( !force &&
	    isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) &&
	    isFlagSet(FREQ_LOOP_RUNNING) )
	{
		pr_debug("Optimizer loop incomplete, suppressing intermediate redraw.\n");
		return;
	}

	g_main_context_invoke_full(NULL, GDK_PRIORITY_REDRAW,
			queue_draw_cb, w, NULL);
}
/*------------------------------------------------------------------*/


/*
   Obtain a backtrace and print it to stdout. 
 */
void _print_backtrace(char **strings)
{
	int i;
	pr_debug("  Backtrace:\n");
	for (i = 0; strings[i] != NULL; i++)
		pr_debug("    %d. %s\n", i, strings[i]);
}

// Return an array of backtrace strings.  The value returned must be free()'ed.
char **_get_backtrace(void)
{
#ifdef HAVE_BACKTRACE
	void *array[10];
	char **strings;
	int size;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	// This wastes an array entry, but allows _print_backtrace() to find
	// the end of the list without realloc'ing space for a NULL:
	strings[size-1] = NULL;

	return strings;
#else
	return NULL;
#endif
}

void print_backtrace(char *msg)
{
	if (msg != NULL)
		pr_debug("%s:\n", msg);

	char **strings = _get_backtrace();
	if (strings != NULL)
	{
		_print_backtrace(strings);
		free(strings);
	}
}

/*------------------------------------------------------------------------*/

/**
 * build_companion_path - replace file extension to derive companion path
 * @src: source file path (e.g., "/path/to/model.nec")
 * @ext: new extension including dot (e.g., ".opt")
 * @buf: output buffer
 * @buflen: size of output buffer
 *
 * Copies src, replaces everything after the last '.' with ext.
 * If src has no extension, appends ext.
 * Returns TRUE if src was non-empty and a path was built.
 */
gboolean build_companion_path(const char *src, const char *ext,
    char *buf, size_t buflen)
{
  char *dot;
  char *slash;

  if (src == NULL || src[0] == '\0')
  {
    return FALSE;
  }

  g_strlcpy(buf, src, buflen);
  dot = strrchr(buf, '.');
  slash = strrchr(buf, '/');

  if (dot != NULL && (slash == NULL || dot > slash))
  {
    *dot = '\0';
  }

  g_strlcat(buf, ext, buflen);
  return TRUE;
}
