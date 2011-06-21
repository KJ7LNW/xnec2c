/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
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

#include "xnec2c.h"
#include "interface.h"
#include "support.h"

/* Pointer to input file */
extern FILE *input_fp;
/* Input file name */
extern char infile[];

/* Forked process number */
extern int nchild;

GtkWidget *error_dialog = NULL;

/*------------------------------------------------------------------------*/

/*  usage()
 *
 *  Prints usage information
 */

void usage(void)
{
  fprintf( stderr,
	  "Usage: xnec2c [-i <input-file-name>]\n"
	  "              [-j <number of processors in SMP machine>]\n"
	  "              [-h: print this usage information and exit]\n"
	  "              [-v: print nec2c version number and exit]\n" );

} /* end of usage() */

/*------------------------------------------------------------------------*/

/* Does the STOP function of fortran but with a warning dialog */
int stop( char *mesg, int err )
{
  /* For child processes */
  if( CHILD )
  {
	fprintf( stderr, "%s\n", mesg );
	if( err )
	{
	  fprintf( stderr,
		  "xnec2c: Fatal: child process %d exiting\n", nchild );
	  _exit(-1);
	}
	else return( err );

  } /* if( CHILD ) */

  /* Stop operation */
  Stop_Frequency_Loop();
  error_dialog = create_error_dialog();
  gtk_label_set_text( GTK_LABEL(
		lookup_widget(error_dialog, "error_label")), mesg );

  /* Hide ok/stop buttons according to error */
  if( err == 1 )
	gtk_widget_hide( lookup_widget(
		  error_dialog, "error_okbutton") );
  else
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

  return( err );

} /* stop */

/*------------------------------------------------------------------*/

/*  load_line()
 *
 *  loads a line from a file, aborts on failure. lines beginning
 *  with a '#' or ''' are ignored as comments. At the end of file
 *  EOF is returned.
 */

int load_line( char *buff, FILE *pfile )
{
  int
	num_chr, /* number of characters read, excluding lf/cr */
	eof = 0, /* EOF flag */
	chr;     /* character read by getc */

  num_chr = 0;

  /* clear buffer at start */
  buff[0] = '\0';

  /* ignore commented lines, white spaces and eol/cr */
  if( (chr = fgetc(pfile)) == EOF )
	return( EOF );

  while(
	  (chr == '#')	||
	  (chr == '\'') ||
	  (chr == ' ')  ||
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
	buff[num_chr++] = chr;

	/* terminate buffer as a string on EOF */
	if( (chr = fgetc(pfile)) == EOF )
	{
	  buff[num_chr] = '\0';
	  eof = EOF;
	}

  } /* end of while( num_chr < max_chr ) */

  /* Capitalize first two characters (mnemonics) */
  if( (buff[0] > 0x60) && (buff[0] < 0x79) )
	buff[0] -= 0x20;
  if( (buff[1] > 0x60) && (buff[1] < 0x79) )
	buff[1] -= 0x20;

  /* terminate buffer as a string */
  buff[num_chr] = '\0';

  return( eof );

} /* end of load_line() */

/*------------------------------------------------------------------------*/

/***  Memory allocation/freeing utils ***/
void mem_alloc( void **ptr, int req, gchar *str )
{
  gchar mesg[100];

  free_ptr( ptr );
  *ptr = malloc( req );
  if( *ptr == NULL )
  {
	strcpy( mesg, "Memory allocation denied " );
	strcat( mesg, str );
	stop( mesg, 1 );
  }

} /* End of mem_alloc() */

/*------------------------------------------------------------------------*/

void mem_realloc( void **ptr, int req, gchar *str )
{
  gchar mesg[100];

  *ptr = realloc( *ptr, req );
  if( *ptr == NULL )
  {
	strcpy( mesg, "Memory re-allocation denied " );
	strcat( mesg, str );
	stop( mesg, 1 );
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
  /* Close file path if open */
  Close_File( fp );
  if( (*fp = fopen(fname, mode)) == NULL )
  {
	char mesg[110] = "xnec2c: ";
	strncat( mesg, fname, 80 );
	perror( mesg );
	strcat( mesg, ": Failed to open file" );
	stop( mesg, 1 );
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

