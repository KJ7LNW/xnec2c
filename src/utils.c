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
 */

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
  fprintf( stderr,
	  _("Usage: xnec2c <input-file-name>\n"
		"              [-i <input-file-name>]\n"
		"              [-j <number of processors in SMP machine>]\n"
		"              [-h: print this usage information and exit]\n"
		"              [-v: print xnec2c version number and exit]\n") );

} /* end of usage() */

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
	if( isFlagSet(MAIN_QUIT) ) exit(-1);

	/* Wait for GTK to complete its tasks */
	while( g_main_context_iteration(NULL, FALSE) );
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
	  if( isFlagSet(MAIN_QUIT) ) exit(-1);

	  /* Wait for GTK to complete its tasks */
	  while( g_main_context_iteration(NULL, FALSE) );
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
	eof,	 /* EOF flag */
	chr;     /* character read by getc */

  num_chr = 0;
  eof     = 0;

  /* clear buffer at start */
  buff[0] = '\0';

  /* ignore commented lines, white spaces and eol/cr */
  if( (chr = fgetc(pfile)) == EOF )
	return( EOF );

  while(
	  (chr == '#')	||
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

/***  Memory allocation/freeing utils ***/
static size_t cnt = 0; /* Total allocation */
  void
mem_alloc( void **ptr, size_t req, gchar *str )
{
  gchar mesg[MESG_SIZE];

  free_ptr( ptr );
  *ptr = malloc( req );
  cnt += req;
  if( *ptr == NULL )
  {
	snprintf( mesg, sizeof(mesg),
		_("Memory allocation denied %s\n"), str );
	fprintf( stderr, "%s: Total memory request %ld\n", mesg, cnt );
	Stop( mesg, ERR_STOP );
  }

} /* End of mem_alloc() */

/*------------------------------------------------------------------------*/

  void
mem_realloc( void **ptr, size_t req, gchar *str )
{
  gchar mesg[MESG_SIZE];

  *ptr = realloc( *ptr, req );
  cnt += req;
  if( *ptr == NULL )
  {
	snprintf( mesg, sizeof(mesg),
		_("Memory re-allocation denied %s\n"), str );
	fprintf( stderr, "%s: Total memory request %ld\n", mesg, cnt );
	Stop( mesg, ERR_STOP );
  }

} /* End of mem_realloc() */

/*------------------------------------------------------------------------*/

  void
free_ptr( void **ptr )
{
  if( *ptr != NULL )
	free( *ptr );
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
	snprintf( mesg, sizeof(mesg),
		_("xnec2c: %s: Failed to open file\n"), fname );
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
  sync();
  if( *fp != NULL ) fclose( *fp );
  *fp = NULL;

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
 * such library fuctions, this makes sure that the dest string is
 * null terminated by copying only n-1 chars to leave room for the
 * terminating char. n would normally be the sizeof(dest) string but
 * copying will not go beyond the terminating null of src string
 */
  void
Strlcpy( char *dest, const char *src, size_t n )
{
  char ch = src[0];
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
 * Unlike other such library fuctions, this makes sure that the dest
 * string is null terminated by copying only n-1 chars to leave room
 * for the terminating char. n would normally be the sizeof(dest)
 * string but copying will not go beyond the terminating null of src
 */
  void
Strlcat( char *dest, const char *src, size_t n )
{
  char ch = dest[0];
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

/*------------------------------------------------------------------*/

