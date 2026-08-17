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

/* Bounds of the notice message body: the column count the text wraps at and
 * the share of the monitor work area it may occupy before it scrolls. */
#define NOTICE_WRAP_CHARS 80
#define NOTICE_SCREEN_FRACTION 0.6

/**
 * notice_format_message() - format a message into a buffer sized to hold it
 * @msg_fmt: printf-style format string
 * @args:    argument list matching @msg_fmt
 *
 * Return: managed buffer holding the complete formatted text, released by
 * the caller.
 */
static char *notice_format_message(const char *msg_fmt, va_list args)
{
	char *message = NULL;
	va_list measure;
	int len;

	/* Measure against a copy so the caller's list stays positioned for the
	 * formatting pass below. */
	va_copy(measure, args);
	len = vsnprintf(NULL, 0, msg_fmt, measure);
	va_end(measure);

	if (len < 0)
	{
		BUG("notice_format_message: vsnprintf failed for \"%s\"\n", msg_fmt);
		len = 0;
	}

	mem_alloc(&message, (size_t)len + 1);
	vsnprintf(message, (size_t)len + 1, msg_fmt, args);

	return message;
}

/**
 * notice_longest_line() - measure the widest line of a message in characters
 * @message: text the notice presents
 *
 * Return: character count of the longest newline-delimited line.
 */
static int notice_longest_line(const char *message)
{
	const char *line = message;
	size_t longest = 0;

	while (line != NULL)
	{
		const char *end = strchr(line, '\n');
		size_t len = (end != NULL ? (size_t)(end - line) : strlen(line));

		longest = MAX(longest, len);
		line = (end != NULL ? end + 1 : NULL);
	}

	return (int)longest;
}

/**
 * notice_set_monospace() - put a fixed-width face in a widget's style context
 * @widget: widget whose face decides its character width
 *
 * Column alignment in reports such as the mathlib benchmark survives only in
 * a fixed-width face.  Styling the widget rather than its text puts that face
 * in the style context, which is where GTK reads the character width it sizes
 * a label by.
 */
static void notice_set_monospace(GtkWidget *widget)
{
	GtkCssProvider *css = gtk_css_provider_new();

	gtk_css_provider_load_from_data(css, "* { font-family: monospace; }", -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
		GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(css);
}

/**
 * notice_area_add() - place a widget in the message area of a notice
 * @dialog: dialog under construction
 * @child:  widget appended below whatever the area already holds
 */
static void notice_area_add(GtkWidget *dialog, GtkWidget *child)
{
	gtk_container_add(
		GTK_CONTAINER(gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog))),
		child);
}

/**
 * notice_body_height_limit() - height a notice body may reach before scrolling
 *
 * Measuring against the monitor rather than a fixed number of pixels lets a
 * large display show a long report whole, and keeps a small display from
 * being handed a dialog taller than itself.
 *
 * Return: height limit in pixels.
 */
static int notice_body_height_limit(void)
{
	GdkRectangle workarea;

	/* GTK 3.22 moved this query from the screen onto a monitor object and
	 * deprecated the screen form, which the 3.18 floor still needs; both
	 * spellings answer the same question, so the build takes the one its
	 * headers offer without complaint. */
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkDisplay *display = gdk_display_get_default();
	GdkMonitor *primary = gdk_display_get_primary_monitor(display);

	/* Wayland leaves the primary monitor unset; the first monitor stands in */
	GdkMonitor *monitor = (primary != NULL) ? primary : gdk_display_get_monitor(display, 0);

	gdk_monitor_get_workarea(monitor, &workarea);
#else
	GdkScreen *screen = gdk_screen_get_default();

	/* An unset primary monitor reports as monitor zero, which stands in */
	gdk_screen_get_monitor_workarea(screen, gdk_screen_get_primary_monitor(screen), &workarea);
#endif

	return (int)(workarea.height * NOTICE_SCREEN_FRACTION);
}

/**
 * notice_add_message_body() - attach the scrolling monospace body of a notice
 * @dialog:  dialog under construction
 * @message: text the notice presents
 *
 * Sizes the body from the text it holds: a message narrower than the wrap
 * limit keeps its own width and a wider one wraps at the limit, while a
 * message within the height limit is shown whole and a taller one is held at
 * the limit and scrolls.
 */
static void notice_add_message_body(GtkWidget *dialog, const char *message)
{
	GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget *label = gtk_label_new(message);
	int columns = MIN(notice_longest_line(message), NOTICE_WRAP_CHARS);
	int limit = notice_body_height_limit();
	gboolean scrolls;
	int height;

	notice_set_monospace(label);

	/* Requesting the same count as both the floor and the ceiling fixes the
	 * label at that many columns; the widest line decides the count, so a
	 * short message stays narrow and a long one stops at the wrap limit. */
	gtk_label_set_width_chars(GTK_LABEL(label), columns);
	gtk_label_set_max_width_chars(GTK_LABEL(label), columns);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);

	gtk_container_add(GTK_CONTAINER(scrolled), label);
	notice_area_add(dialog, scrolled);

	/* Height follows from the columns the label settled on, and the font
	 * deciding it belongs to the style context the label inherits, so it is
	 * measured only once the body belongs to the dialog. */
	gtk_widget_get_preferred_height(label, NULL, &height);

	scrolls = (height > limit);

	/* Wrapping leaves nothing to reach sideways, so that axis is denied
	 * outright.  A body within the limit shows every line and carries no bar
	 * at all; a taller one keeps its bar on view, where the run of the thumb
	 * reports how much text lies past the edge. */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
		GTK_POLICY_NEVER, scrolls ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER);

	/* An overlay bar fades out of sight, which would leave a body that
	 * scrolls looking like one that does not. */
	gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scrolled), FALSE);

	gtk_scrolled_window_set_min_content_height(
		GTK_SCROLLED_WINDOW(scrolled), MIN(height, limit));
}

/**
 * notice_add_question() - attach the prompt the notice buttons answer
 * @dialog:   dialog under construction
 * @question: prompt, NULL when the notice only informs
 *
 * Placed after the body so it reads directly above the buttons and stays out
 * of the scrolling region.
 */
static void notice_add_question(GtkWidget *dialog, const char *question)
{
	if (question == NULL)
		return;

	GtkWidget *label = gtk_label_new(question);

	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_max_width_chars(GTK_LABEL(label), NOTICE_WRAP_CHARS);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);

	notice_area_add(dialog, label);
}

/**
 * notice_run() - present one notice and report the button pressed
 * @buttons:  button set the dialog offers
 * @title:    heading of the dialog
 * @question: prompt the buttons answer, NULL when the notice only informs
 * @message:  body text
 *
 * Return: GTK response of the button pressed, or 0 when the notice reached
 * the terminal instead of the screen.
 */
static int notice_run(GtkButtonsType buttons, const char *title,
	const char *question, const char *message)
{
	GtkWidget *notice;
	int response;
	int locked = 0;

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

	notice = gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
		buttons,
		"%s", title);

	gtk_window_set_title(GTK_WINDOW(notice), title);

	notice_add_message_body(notice, message);
	notice_add_question(notice, question);

	gtk_widget_show_all(notice);

	response = gtk_dialog_run(GTK_DIALOG(notice));

	gtk_widget_destroy(notice);

	return response;
}

/**
 * notice_vrun() - format the body of a notice and present it
 * @buttons:  button set the dialog offers
 * @title:    heading of the dialog
 * @question: prompt the buttons answer, NULL when the notice only informs
 * @msg_fmt:  printf-style format string of the body
 * @args:     argument list matching @msg_fmt
 *
 * Return: GTK response of the button pressed.
 */
static int notice_vrun(GtkButtonsType buttons, const char *title,
	const char *question, const char *msg_fmt, va_list args)
{
	char *message = notice_format_message(msg_fmt, args);
	int response = notice_run(buttons, title, question, message);

	mem_free(&message);

	return response;
}

/**
 * Notice() - present a notice the user acknowledges
 * @buttons: button set the dialog offers
 * @title:   heading of the dialog
 * @msg_fmt: printf-style format string of the body
 *
 * Return: GTK response of the button pressed, such as GTK_RESPONSE_OK or
 * GTK_RESPONSE_CANCEL, drawn from the set @buttons names.
 */
int Notice(GtkButtonsType buttons, const char *title, const char *msg_fmt, ...)
{
	va_list args;
	int response;

	va_start(args, msg_fmt);
	response = notice_vrun(buttons, title, NULL, msg_fmt, args);
	va_end(args);

	return response;
}

/**
 * Notice_Question() - present a notice whose buttons answer a prompt
 * @buttons:  button set the dialog offers
 * @title:    heading of the dialog
 * @question: prompt placed below the body and above the buttons
 * @msg_fmt:  printf-style format string of the body
 *
 * Return: GTK response of the button pressed.
 */
int Notice_Question(GtkButtonsType buttons, const char *title,
	const char *question, const char *msg_fmt, ...)
{
	va_list args;
	int response;

	va_start(args, msg_fmt);
	response = notice_vrun(buttons, title, question, msg_fmt, args);
	va_end(args);

	return response;
}

/* Deferred Notice — formatted message delivered after current GTK
 * event processing completes, avoiding re-entrant main loop corruption
 * when called from draw callbacks. */

typedef struct
{
	GtkButtonsType buttons;
	char title[128];
	char *message;
} deferred_notice_t;

static void _deferred_notice_cb(gpointer user_data)
{
	deferred_notice_t *dn = (deferred_notice_t *)user_data;
	Notice(dn->buttons, dn->title, "%s", dn->message);
	mem_free(&dn->message);
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
	dn->message = notice_format_message(msg_fmt, args);
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
  if (freq_sweep_active())
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
  if(freq_sweep_active())
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

/**
 * xnec2c_num_procs() - Report the processors this process may compute on
 *
 * Return: processor count, at least one.
 */
int xnec2c_num_procs(void)
{
#ifdef HAVE_OPENMP
	return omp_get_num_procs();
#else
	return 1;
#endif
}

/**
 * xnec2c_threads_per_worker() - Divide the processors among concurrent workers
 * @workers: number of computations that run at the same time
 *
 * Workers and their library threads draw on the same processors, so each
 * worker is entitled to an equal share: eight workers on a thirty-two
 * processor machine receive four threads each.  The --threads option states
 * a budget of its own and overrides the division.
 *
 * Return: thread count one worker is entitled to, at least one.
 */
int xnec2c_threads_per_worker(int workers)
{
	int threads;

	if (workers < 1)
	{
		BUG("%s: worker count is %d\n", __func__, workers);
		workers = 1;
	}

	/* A zero count is the request to divide the processors. */
	if (calc_data.num_threads > 0)
		threads = calc_data.num_threads;
	else
		threads = xnec2c_num_procs() / workers;

	if (threads < 1)
		threads = 1;

	return threads;
}

/**
 * xnec2c_set_omp_threads() - Apply a thread budget to the OpenMP runtime
 * @threads: thread count this process is entitled to
 *
 * OMP_NUM_THREADS states the count of every OpenMP runtime in the process, so
 * an exported setting overrides this applier, which then sets nothing.
 */
void xnec2c_set_omp_threads(int threads)
{
#ifdef HAVE_OPENMP
	static int applied = 0;

	if (getenv("OMP_NUM_THREADS") != NULL)
		return;

	omp_set_num_threads(threads);

	/* The budget is applied for every worker the sweep dispatches, so the
	 * count is announced where it changes. */
	if (applied != threads)
	{
		applied = threads;
		pr_info("applying an OpenMP budget of %d threads\n", threads);
	}
#else
	/* This build carries no OpenMP runtime to receive the budget. */
	(void)threads;
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
