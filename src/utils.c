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

#include "utils.h"
#include "shared.h"

char **_get_backtrace(void);
void _print_backtrace(char **strings);


/*------------------------------------------------------------------------*/

/*  usage()
 *
 *  Prints usage information
 */

  void
usage(void)
{
  fprintf(stdout, "Usage: xnec2c [options] [<input-file-name>]\n"
		"  -i|--input <input-file-name>\n"
		"  -j|--jobs  <number of processors in SMP machine> (-j0 disables forking)\n"
		"  -b|--batch:        enable batch mode, exit after the frequency loop runs\n"
		"     --optimize:     Activate the optimizer immediately.\n"
		"  -P|--no-pthreads:  disable pthreads and use the GTK loop for debugging\n"
		"  -h|--help:         print usage information and exit\n"
		"  -V|--version:      print xnec2c version number and exit\n"
		"  -v|--verbose:      increase verbosity, can be specified multiple times\n"
		"  -d|--debug:        enable debug output (-dd includes backtraces)\n"
		"  -d|--quiet:        suppress debug/verbose output\n"
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
		"  --write-currents        <filename>  - write CSV of currents and charges\n");

} /* end of usage() */


// May return GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, ...
int Notice(char *title, char *message,  GtkButtonsType buttons)
{
	int response;
	int locked = 0;

	if (!g_mutex_trylock(&freq_data_lock))
		locked = 1;
	else
		g_mutex_unlock(&freq_data_lock);

	if (!g_mutex_trylock(&global_lock))
		locked = 1;
	else
		g_mutex_unlock(&global_lock);

	if (locked)
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

/*------------------------------------------------------------------------*/

/* Does the STOP function of fortran but with a warning dialog */
  int
Stop( char *mesg, int err )
{
  GtkBuilder *builder;

  int locked = 0;

  if (!g_mutex_trylock(&freq_data_lock))
    locked = 1;
  else
    g_mutex_unlock(&freq_data_lock);

  if (!g_mutex_trylock(&global_lock))
    locked = 1;
  else
    g_mutex_unlock(&global_lock);

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

/* xnec2c uses mem_realloc() very often to resize buffers in the code.
 * There are cases where uninitialized memory buffers can lead to incorrect behavior
 * but unfortunately the libc realloc() call doesn't initialize the reallocated memory.
 * 
 * Since there is not a portable way to discover the amount of memory allocated by the
 * previous realloc/malloc() call we cannot call memset() on the extended portion.
 *
 * To solve this mem_obj_t stores a couple sizes and a pointer:
 *    1. size: the total allocated size requested by the caller
 *             Note that the actual allocated size is greater by sizeof(mem_obj_t)
 *    2. used: The amount actually used by the caller
 *         a. If mem_realloc() is called with a smaller amount of memory requested
 *            then it will shrink the `used` size without reallocating.  
 *            
 *         b. If mem_realloc() is called requesting more than `used` but less than `size`
 *            then it is grown without reallocating.
 *
 *         c. If mem_realloc() is called requesting more than `size` then realloc() is
 *            called and both `size` and `used` are updated.
 *
 *    3. ptr: a pointer to the memory used by the caller
 *          
 * A mem_obj_t object is allocated in excess of the structure size by the amount
 * of memory requested by the caller.  When ptr_free or mem_realloc are called
 * we use decrement by sizeof(mem_obj_t) to access the original
 * structure pointer.
 */
typedef struct mem_obj_t
{
	size_t size;
	size_t used;
	char **backtrace;

	void *ptr;
} mem_obj_t;

void mem_obj_dump(void *ptr)
{
	mem_obj_t *m = (mem_obj_t*)ptr;
	m--;

	pr_debug("mem_obj_t at %p:\n"
		"  size: %lu\n"
		"  used: %lu\n"
		"  addr: %p\n", (void *)m, (unsigned long)m->size,  (unsigned long)m->used, m->ptr);


}

void mem_backtrace(void *ptr)
{
	mem_obj_t *m = (mem_obj_t *)ptr;
	if (m == NULL)
	{
		print_backtrace("mem_backtrace(NULL)");

		return;
	}

	m--;
	pr_debug("mem_backtrace(%p):\n", ptr);
	if (m->backtrace == NULL)
		pr_debug("  m->backtrace is null, no backtrace data\n");
	else
		_print_backtrace(m->backtrace);

}

inline void mem_alloc( void **ptr, size_t req, gchar *str )
{
	mem_realloc(ptr, req, str);
} /* End of mem_alloc() */

/*------------------------------------------------------------------------*/

void mem_realloc( void **ptr, size_t req, gchar *str )
{
  gchar mesg[MESG_SIZE];
  size_t prev_used;

  // Get the original mem_obj_t object:
  mem_obj_t *m = (mem_obj_t *)*ptr;

  if (m == NULL)
  {
	  prev_used = 0;
	  m = realloc( m, req+sizeof(mem_obj_t) );

	  if (m != NULL)
	  {
		  m->size = req;
		  m->used = req;
		  m->backtrace = NULL;
	  }
  }
  else
  {
	  // Adjust pointer to struct location:
	  m--;

	  if (m->backtrace != NULL)
	  {
		  free(m->backtrace);
		  m->backtrace = NULL;
	  }
	  prev_used = m->used;

	  if (req <= m->size)
	  {
		m->used = req;
	  }
	  else 
	  {
		m = realloc( m, req+sizeof(mem_obj_t) );

		if (m != NULL)
		{
			m->size = req;
			m->used = req;
		}
	  }
  }

  if( m == NULL )
  {
    snprintf( mesg, sizeof(mesg),
        _("Memory re-allocation denied %s\n"), str );
    pr_err("%s: Memory requested %lu\n", mesg, (unsigned long)req);
    Stop( mesg, ERR_STOP );
	return;
  }

  m->ptr = (void*)(m+1);
  *ptr = m->ptr;

  // Get a backtrace for the allocation.  Use mem_backtrace(ptr) to display it.
  // Debug only if you need it, this is very slow:
  //m->backtrace = _get_backtrace();

  if (m->used > prev_used)
	  memset(((uint8_t*)*ptr)+prev_used, 0x00, m->used - prev_used);

} /* End of mem_realloc() */

/*------------------------------------------------------------------------*/

  void
free_ptr( void **ptr )
{
  if( *ptr != NULL )
  {
	mem_obj_t *m = (mem_obj_t *)*ptr;
	m--;
    free( m );
  }
  *ptr = NULL;

} /* End of free_ptr() */

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
    Stop( mesg, ERR_STOP );
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
  static gboolean first_call = TRUE;
  static char dp = '.';


  /* Find locale-dependent decimal point character */
  if( first_call )
  {
    struct lconv *lcnv;
    lcnv = localeconv();
    dp = *lcnv->decimal_point;
    first_call = FALSE;
  }

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
		// Wait for GTK
		while (g_main_context_iteration(NULL, FALSE)) {}
		
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

		free_ptr((void**)&cbdata);
	}


	return FALSE;
}

guint _g_idle_add_once(GSourceOnceFunc function, gpointer data, int lock)
{
	guint ret;
	g_idle_add_data_t *cbdata = NULL;

	mem_alloc((void**)&cbdata, sizeof(g_idle_add_data_t), __LOCATION__);

	// Don't do a locked sync if -P is specified because GTK
	// is already running synchronously.
	if (rc_config.disable_pthread_freqloop)
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

		free_ptr((void**)&cbdata);
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


/* This is a hook to centrally control the redraw of widgets based on state
 * Also it uses g_idle_add so it is thread-safe. */
void xnec2_widget_queue_draw(GtkWidget *w)
{
	// Only redraw the rdpattern when FREQ_LOOP_DONE or it the window will flash grey:
	if (w == rdpattern_drawingarea &&
	    isFlagSet(OPTIMIZER_OUTPUT) &&
	    isFlagSet(FREQ_LOOP_RUNNING) &&
	    !need_rdpat_redraw)
	{
		pr_debug("Optimizer loop incomplete, skipping radiation pattern redraw.\n");
	}
	else if (w == structure_drawingarea &&
	    isFlagSet(OPTIMIZER_OUTPUT) &&
	    isFlagSet(FREQ_LOOP_RUNNING) &&
	    !need_structure_redraw)
	{
		pr_debug("Optimizer loop incomplete, skipping structure redraw.\n");
	}
	else
		g_idle_add_once((GSourceOnceFunc)gtk_widget_queue_draw, w);

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
