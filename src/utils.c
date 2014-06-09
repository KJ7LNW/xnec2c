/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *
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

/* misc.c
 *
 * Miscellaneous support functions for xnec2c.c
 */

#include "utils.h"

/*------------------------------------------------------------------------*/

/*  usage()
 *
 *  Prints usage information
 */

void usage(void)
{
  fprintf( stderr,
	  _("Usage: xnec2c <input-file-name>\n"\
		"              [-i <input-file-name>]\n"\
		"              [-j <number of processors in SMP machine>]\n"\
		"              [-h: print this usage information and exit]\n"\
		"              [-v: print xnec2c version number and exit]\n") );

} /* end of usage() */

/*------------------------------------------------------------------------*/

/* Does the STOP function of fortran but with a warning dialog */
  int
stop( char *mesg, int err )
{
  /* For child processes */
  if( CHILD )
  {
	fprintf( stderr, "%s\n", mesg );
	if( err )
	{
	  fprintf( stderr,
		  _("xnec2c: fatal: child process %d exiting\n"),
		  num_child_procs );
	  _exit(-1);
	}
	else
	  return( err );

  } /* if( CHILD ) */

  /* Stop operation */
  Stop_Frequency_Loop();
  error_dialog = create_error_dialog();
  gtk_label_set_text( GTK_LABEL(
		lookup_widget(error_dialog, "error_label")), mesg );

  /* Hide ok button according to error */
  if( err == TRUE )
	gtk_widget_hide( lookup_widget(
		  error_dialog, "error_okbutton") );
  gtk_widget_show( error_dialog );

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
} /* stop */

/*------------------------------------------------------------------*/

  gboolean
Nec2_Save_Warn( const gchar *mesg )
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	error_dialog = create_error_dialog();
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(error_dialog, "error_label")), mesg );
	gtk_widget_hide( lookup_widget(
		  error_dialog, "error_stopbutton") );
	gtk_widget_show( error_dialog );

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

int Load_Line( char *buff, FILE *pfile )
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

  /* Capitalize first two characters (mnemonics) */
  if( (buff[0] > 0x60) && (buff[0] < 0x79) )
	buff[0] = (char)toupper( (int)buff[1] );
  if( (buff[1] > 0x60) && (buff[1] < 0x79) )
	buff[1] = (char)toupper( (int)buff[1] );

  /* terminate buffer as a string */
  buff[num_chr] = '\0';

  return( eof );
} /* end of Load_Line() */

/*------------------------------------------------------------------------*/

/***  Memory allocation/freeing utils ***/
static size_t cnt = 0; /* Total allocation */
void mem_alloc( void **ptr, size_t req, gchar *str )
{
  gchar mesg[100];

  free_ptr( ptr );
  *ptr = malloc( req );
  cnt += req;
  if( *ptr == NULL )
  {
	snprintf( mesg, 99, _("Memory allocation denied %s\n"), str );
	mesg[99] = '\0';
	fprintf( stderr, "%s: Total memory request %ld\n", mesg, cnt );
	stop( mesg, ERR_STOP );
  }

} /* End of mem_alloc() */

/*------------------------------------------------------------------------*/

void mem_realloc( void **ptr, size_t req, gchar *str )
{
  gchar mesg[100];

  *ptr = realloc( *ptr, req );
  cnt += req;
  if( *ptr == NULL )
  {
	snprintf( mesg, 99, _("Memory re-allocation denied %s\n"), str );
	mesg[99] = '\0';
	fprintf( stderr, "%s: Total memory request %ld\n", mesg, cnt );
	stop( mesg, ERR_STOP );
  }

} /* End of mem_realloc() */

/*------------------------------------------------------------------------*/

void free_ptr( void **ptr )
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
  /* Abort if file name is blank */
  if( strlen(infile) == 0 ) return( TRUE );

  /* Close file path if open */
  Close_File( fp );
  if( (*fp = fopen(fname, mode)) == NULL )
  {
	char mesg[110];
	snprintf( mesg, 109,
		_("xnec2c: %s: Failed to open file\n"), fname );
	mesg[109] = '\0';
	stop( mesg, ERR_STOP );
	return( FALSE );
  }

  return(TRUE);
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
	fclose( *fp );
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

  snprintf( str, 4, "%3d", fstep );
  str[3] = '\0';
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
  while( (ch != '\0') )
  {
	idd++;
	ch = dest[idd];
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
