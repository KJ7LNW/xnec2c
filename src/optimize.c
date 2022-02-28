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

#include "shared.h"
#include <poll.h>
#include <sys/inotify.h>

/*------------------------------------------------------------------------*/

/* Writes out frequency-dependent
 * data for the external Optimizer */
  void
_Write_Optimizer_Data( void )
{
  char csv_file[FILENAME_LEN];
  size_t s = sizeof( csv_file );

  int i;

  measurement_t meas;

  /* Create a file name for the Optimizer csv file */
  Strlcpy( csv_file, rc_config.input_file, s );
  Strlcat( csv_file, ".csv", s );

  /* Open Optimizer csv file */
  FILE *fp = NULL;
  if( !Open_File(&fp, csv_file, "w") )
  {
    if( error_dialog == NULL )
    {
      GtkBuilder *builder;
      error_dialog = create_error_dialog( &builder );
      gtk_label_set_text( GTK_LABEL(Builder_Get_Object(builder, "error_label")),
          "Failed to open Optimizer CSV file for writing" );
      gtk_widget_hide( Builder_Get_Object(builder, "error_okbutton") );
      gtk_widget_show( error_dialog );
      g_object_unref( builder );
    }
    perror( "xnec2c: Open_File()" );

    return;
  } // if( !Open_File(&fp, csv_file, "w") )

  // Print names
  for (i = 0; i < MEAS_COUNT; i++)
	  fprintf(fp, "%s,", meas_names[i]);
  fprintf(fp, "\n");

  /* Print frequency-dependent data corresponding
   * to graphs in plot of frequency-dependent data FIXME */
  for( int idx = 0; idx < calc_data.steps_total; idx++ )
  {
	meas_calc(&meas, idx);
	for (i = 0; i < MEAS_COUNT; i++)
	{
		fprintf( fp, "%.17g,", meas.a[i] );
	}
    fprintf(fp, "\n");
  }

  fclose( fp );
} // Write_Optimizer_Data()

void Write_Optimizer_Data( void )
{
	g_mutex_lock(&freq_data_lock);
	_Write_Optimizer_Data();
	g_mutex_unlock(&freq_data_lock);
}

/*------------------------------------------------------------------------*/

int inotify_open(struct pollfd *pfd)
{
  int wd, fd;

  /* Create the file descriptor for accessing the inotify API. */
  fd = inotify_init1( IN_NONBLOCK );
  if( fd == -1 )
  {
    perror( "xnec2c: inotify_init1" );
    exit( -1 );
  }

  /* Create a file watch descriptor */
  wd = inotify_add_watch( fd, rc_config.input_file, IN_CLOSE_WRITE );
  if( wd == -1 )
  {
    fprintf(stderr, "xnec2c: cannot watch '%s': %s\n",
        rc_config.input_file, strerror(errno));
    exit( -1 );
  }

  pfd->fd     = fd;     /* Inotify input */
  pfd->events = POLLIN;

  return fd;
}

// Watches the NEC2 (.nec) input file for a save using the
// inotify system and re-reads the input file for re-processing
  void *
Optimizer_Output( void *arg )
{
  int fd = -1, poll_num;
  int job_num, num_busy_procs;
  struct pollfd pfd;
  char buf[256] __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;

  char prev_input_file[sizeof(rc_config.input_file)] = {0};

  /* Wait for watch events */
  while( TRUE )
  {
	// Re-open the inotify handle if the filename changes.  This works
	// for the first iteration too because prev_input_file is empty:
	if (strcmp(prev_input_file, rc_config.input_file))
	{
	  if (fd > 0)
		close(fd);

	  fd = inotify_open(&pfd);
	  strncpy(prev_input_file, rc_config.input_file, sizeof(prev_input_file));
	}

    // Exit thread if optimizer output has been cancelled
    if( isFlagClear(OPTIMIZER_OUTPUT) ||
        isFlagClear(PLOT_ENABLED) )
      break;

    if ( CHILD )
	{
		printf("optimize.c: exiting because we are a child process.\n");
		break;
	}

    // Poll inotify file descriptor, timeout 1 sec
    poll_num = poll( &pfd, 1, 1000 );

    if (poll_num == -1)
    {
      if( errno == EINTR ) continue;
      perror( "xnec2c: poll" );
      exit( -1 );
    }

    if( poll_num > 0 )
    {
      if( pfd.revents & POLLIN )
      {
        /* Inotify events are available. Read some events. */
        len = read( fd, buf, sizeof(buf) );
        if( (len == -1) && (errno != EAGAIN) )
        {
          perror( "xnec2c: read" );
          exit( -1 );
        }

		if ( isFlagSet(FREQ_LOOP_RUNNING | FREQ_LOOP_INIT | INPUT_PENDING)|| isFlagClear(FREQ_LOOP_DONE))
			continue;

		num_busy_procs = 0;
		for (job_num = 0; job_num < calc_data.num_jobs; job_num++)
			if (forked_proc_data != NULL && forked_proc_data[job_num] != NULL && forked_proc_data[job_num]->busy) 
				num_busy_procs++;

		if (num_busy_procs)
		{
			printf("warning: %d child jobs are running, skipping optimization\n", num_busy_procs);
			usleep(100000);
			continue;
		}

        event = (const struct inotify_event *) buf;

        /* Read input file and re-process */
        if( event->mask & IN_CLOSE_WRITE )
        {
          gboolean flag = FALSE;

		  // Prevent queuing a file change while locked:
		  g_mutex_lock(&global_lock);
          g_idle_add( Open_Input_File, (gpointer) &flag );
		  g_mutex_unlock(&global_lock);

        }
      }
    } // if( poll_num > 0 )
  } // while( TRUE )

  /* Close inotify file descriptor. */
  close( fd );
  return( NULL );
} // Optimizer_Output()

/*------------------------------------------------------------------------*/

