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

  int pol, mgidx;
  double max_gain;

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

  /* Print data column titles */
  fprintf( fp, "%s,", "Freq-MHz" );
  fprintf( fp, "%s,%s,", "Z-real", "Z-imag" );
  fprintf( fp, "%s,%s,", "Z-magn", "Z-phase" );
  fprintf( fp, "%s,", "VSWR" );
  fprintf( fp, "%s,%s,", "Gain-max", "F/B Ratio" );
  fprintf( fp, "%s,%s,", "Direct-tht", "Direct-phi" );
  fprintf( fp, "%s,", "Gain-viewer" );
  fprintf( fp, "%s,", "Gain-net" );
  fprintf( fp, "\n" );

  /* Print frequency-dependent data corresponding
   * to graphs in plot of frequency-dependent data FIXME */
  for( int idx = 0; idx < calc_data.steps_total; idx++ )
  {
    /* Print the frequency in MHz */
    fprintf( fp, "%g,", (double)save.freq[idx] );

    /* Print Z-real and Z-imag of input impedance */
	fprintf( fp, "%g,%g,",
		impedance_data.zreal[idx], impedance_data.zimag[idx] );

    /* Plot Z-magnitude and Z-phase */
	fprintf( fp, "%g,%g,",
		impedance_data.zmagn[idx], impedance_data.zphase[idx] );

    /* Print VSWR */
	double zrpro2 = impedance_data.zreal[idx] + calc_data.zo;
	zrpro2 *= zrpro2;
	double zrmro2 = impedance_data.zreal[idx] - calc_data.zo;
	zrmro2 *= zrmro2;
	double zimag2 = impedance_data.zimag[idx] * impedance_data.zimag[idx];
	double gamma = sqrt( (zrmro2 + zimag2) / (zrpro2 + zimag2) );
	double vswr = (1 + gamma) / (1 - gamma);
	fprintf( fp, "%g,", vswr );

    /* Print Max gain for given polarization type and direction if enabled */
	pol = calc_data.pol_type;
	mgidx = rad_pattern[idx].max_gain_idx[pol];
	max_gain = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	fprintf( fp, "%g,%g,", max_gain, rad_pattern[idx].fbratio );

	fprintf( fp, "%g,%g,",
		90.0 - rad_pattern[idx].max_gain_tht[pol],
		rad_pattern[idx].max_gain_phi[pol] );

    /* Print gain in viewer's direction */
	double viewer_gain = Viewer_Gain( structure_proj_params, idx );
	fprintf( fp, "%g,", viewer_gain );

    /* Print Net gain in max gain case */
	pol = calc_data.pol_type;
	mgidx = rad_pattern[idx].max_gain_idx[pol];
	max_gain = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	double Zr = impedance_data.zreal[idx];
	double Zi = impedance_data.zimag[idx];
	double Zo = calc_data.zo;
	double net_gain = max_gain +
	  10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );
	fprintf( fp, "%g,", net_gain );

    fprintf( fp, "\n" );
  } //for( int idx = 0; idx < calc_data.steps_total; idx++ )

  fclose( fp );
} // Write_Optimizer_Data()

void Write_Optimizer_Data( void )
{
	g_mutex_lock(&freq_data_lock);
	_Write_Optimizer_Data();
	g_mutex_unlock(&freq_data_lock);
}

/*------------------------------------------------------------------------*/

// Watches the NEC2 (.nec) input file for a save using the
// inotify system and re-reads the input file for re-processing
  void *
Optimizer_Output( void *arg )
{
  int fd, poll_num;
  int wd, job_num, num_busy_procs;
  struct pollfd pfd;
  char buf[256] __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;

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

  pfd.fd     = fd;     /* Inotify input */
  pfd.events = POLLIN;

  /* Wait for watch events */
  while( TRUE )
  {
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

