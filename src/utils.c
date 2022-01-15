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

#ifndef WIN32
#include <execinfo.h>
#endif

#include "utils.h"
#include "shared.h"

char **_get_backtrace();
void _print_backtrace(char **strings);


/*------------------------------------------------------------------------*/

/*  usage()
 *
 *  Prints usage information
 */

  void
usage(void)
{
  fprintf( stderr,
      _("Usage: xnec2c <input-file-name>\n"
        "              [-i <input-file-name>]\n"
        "              [-j <number of processors in SMP machine>]\n"
        "                  -j 0 disables forking for debug.\n"
        "              [-P: debug: disable pthreads for freqloop, use GTK mainloop instead.\n"
        "              [-h: print this usage information and exit]\n"
        "              [-v: print xnec2c version number and exit]\n") );

} /* end of usage() */


// May return GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, ...
int Notice(char *title, char *message,  GtkButtonsType buttons)
{
	int response;
	GtkWidget *notice = gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
		buttons,
		"%s", title);

	printf("\n=== Notice: %s ===\n%s\n\n", title, message);
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

  /* For child processes */
  if( CHILD )
  {
    fprintf( stderr, "%s\n", mesg );
    if( err )
    {
      fprintf( stderr,
          _("xnec2c: fatal: child process %d exiting\n"), num_child_procs );
      _exit(-1);
    }
    else return( err );

  } /* if( CHILD ) */

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
	while( g_main_context_iteration(NULL, FALSE) );

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
      while( g_main_context_iteration(NULL, FALSE) );

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
 * but unfortunatly the libc realloc() call doesn't initialize the reallocated memory.
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

void mem_backtrace(void *ptr)
{
	mem_obj_t *m = (mem_obj_t *)ptr;
	if (m == NULL)
	{
		print_backtrace("mem_backtrace(NULL)");

		return;
	}

	m--;
	printf("mem_backtrace(%p):\n", ptr);
	if (m->backtrace == NULL)
		printf("  m->backtrace is null, no backtrace data\n");
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
    fprintf( stderr, "%s: Memory requested %lu\n", mesg, req );
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
#ifndef WIN32
	  fsync(fileno(*fp));
#endif
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
	GSourceFunc function;
	gpointer data;
	GMutex lock;
	int is_locked;
	char **backtrace;
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
		while (g_main_context_iteration(NULL, FALSE));
		
		if (cbdata->backtrace != NULL)
		{
			free(cbdata->backtrace);
			cbdata->backtrace = NULL;
		}

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

void _g_idle_add_once(GSourceFunc function, gpointer data, int lock)
{
	g_idle_add_data_t *cbdata = NULL;
	mem_alloc((void**)&cbdata, sizeof(g_idle_add_data_t), __LOCATION__);

	g_mutex_init(&cbdata->lock);

	cbdata->function = function;
	cbdata->data = data;
	cbdata->is_locked = lock;

	cbdata->backtrace = NULL;

	// Debug async call backtraces if you need it, but be aware that _get_backtrace() is slow.
	//cbdata->backtrace = _get_backtrace();

	if (lock)
		g_mutex_lock(&cbdata->lock);

	g_idle_add((GSourceFunc)_callback_g_idle_add_once, cbdata);

	// Wait for the lock to release and free it.
	if (lock)
	{
		g_mutex_lock(&cbdata->lock);
		g_mutex_unlock(&cbdata->lock);
		free_ptr((void**)&cbdata);
	}
}

// Call from any thread to queue a function to run once, do not wait for it to finish.
void g_idle_add_once(GSourceFunc function, gpointer data)
{
	_g_idle_add_once(function, data, 0); // async
}

// Call from another thread to queue a function to run once, and wait for it to finish.
void g_idle_add_once_sync(GSourceFunc function, gpointer data)
{
	_g_idle_add_once(function, data, 1); // sync
}


/* This is a hook to centrally control the redraw of widgets based on state
 * Also it uses g_idle_add so it is thread-safe. */
void xnec2_widget_queue_draw(GtkWidget *w)
{
	// Only redraw the rdpattern when FREQ_LOOP_DONE or it the window will flash grey:
	if (w == rdpattern_drawingarea && isFlagSet(OPTIMIZER_OUTPUT) && isFlagSet(FREQ_LOOP_RUNNING))
	{
		printf("Optimizer loop incomplete, skipping radiation pattern redraw.\n");
	}
	else
		g_idle_add_once((GSourceFunc)gtk_widget_queue_draw, w);

}
/*------------------------------------------------------------------*/


/*
   Obtain a backtrace and print it to stdout. 
 */
void _print_backtrace(char **strings)
{
	int i;
	printf("  Backtrace:\n");
	for (i = 0; strings[i] != NULL; i++)
		printf("    %d. %s\n", i, strings[i]);
}

// Return an array of backtrace strings.  The value returned must be free()'ed.
char **_get_backtrace()
{
	void *array[10];
	char **strings = NULL;
	int size;

#ifndef WIN32
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
#endif

	// This wastes an array entry, but allows _print_backtrace() to find
	// the end of the list without realloc'ing space for a NULL:
	strings[size-1] = NULL;

	return strings;
}

void print_backtrace(char *msg)
{
	if (msg != NULL)
		printf("%s:\n", msg);

	char **strings = _get_backtrace();
	if (strings != NULL)
	{
		_print_backtrace(strings);
		free(strings);
	}
}
