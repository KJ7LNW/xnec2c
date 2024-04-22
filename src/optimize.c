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

/*------------------------------------------------------------------------*/

int opt_have_files_to_save(void)
{
	return (
		// Flags
		rc_config.opt_write_csv ||
        rc_config.opt_write_s1p ||
        rc_config.opt_write_s2p_max_gain ||
        rc_config.opt_write_s2p_viewer_gain ||
		rc_config.opt_write_rdpat ||
		rc_config.opt_write_currents ||

		// Files:
		rc_config.filename_csv ||
		rc_config.filename_s1p ||
		rc_config.filename_s2p_max_gain ||
		rc_config.filename_s2p_viewer_gain ||
		rc_config.filename_rdpat ||
		rc_config.filename_currents
	);
}

/* Writes out frequency-dependent
 * data for the external Optimizer */
  void
_Write_Optimizer_Data( void )
{
  char filename[FILENAME_LEN];
  size_t n = sizeof( filename );

  // from the menu:
  if (rc_config.opt_write_csv)
	  Save_FreqPlots_CSV(str_append(filename, rc_config.input_file, ".csv", n));

  if (rc_config.opt_write_s1p)
	  Save_FreqPlots_S1P(str_append(filename, rc_config.input_file, ".s1p", n));

  if (rc_config.opt_write_s2p_max_gain)
	  Save_FreqPlots_S2P_Max_Gain(str_append(filename, rc_config.input_file, "-maxgain.s2p", n));

  if (rc_config.opt_write_s2p_viewer_gain)
	  Save_FreqPlots_S2P_Viewer_Gain(str_append(filename, rc_config.input_file, "-viewergain.s2p", n));

  if (rc_config.opt_write_rdpat)
	  Save_RadPattern_CSV(str_append(filename, rc_config.input_file, "-radpattern.csv", n));

  if (rc_config.opt_write_currents)
	  Save_Currents_CSV(str_append(filename, rc_config.input_file, "-currents.csv", n));


  // from the cmdline:
  if (rc_config.filename_csv)
	  Save_FreqPlots_CSV(rc_config.filename_csv);

  if (rc_config.filename_s1p)
	  Save_FreqPlots_S1P(rc_config.filename_s1p);

  if (rc_config.filename_s2p_max_gain)
	  Save_FreqPlots_S2P_Max_Gain(rc_config.filename_s2p_max_gain);

  if (rc_config.filename_s2p_viewer_gain)
	  Save_FreqPlots_S2P_Viewer_Gain(rc_config.filename_s2p_viewer_gain);

  if (rc_config.filename_rdpat)
  {
	pr_debug("saving rdpat: %s\n", rc_config.filename_rdpat);
	Save_RadPattern_CSV(rc_config.filename_rdpat);
  }

  if (rc_config.filename_currents)
  {
	pr_debug("saving currents: %s\n", rc_config.filename_currents);
	Save_Currents_CSV(rc_config.filename_currents);
  }

} // Write_Optimizer_Data()

void Write_Optimizer_Data( void )
{
	g_mutex_lock(&freq_data_lock);
	_Write_Optimizer_Data();
	g_mutex_unlock(&freq_data_lock);
}


// If inotify was not detected then create stubs:
#ifndef HAVE_INOTIFY

void *Optimizer_Output( void *arg ) { pr_err("xnec2c was built without inotify.\n"); return NULL; }

// Otherwise, normal optimize code:
#else

#include <poll.h>
#include <sys/inotify.h>
#include <sys/stat.h>

int inotify_open(struct pollfd *pfd)
{
  int wd, fd;

  /* Create the file descriptor for accessing the inotify API. */
  fd = inotify_init1( IN_NONBLOCK );
  if( fd == -1 )
  {
    perror( "inotify_init1" );
    exit( -1 );
  }

  /* Create a file watch descriptor */
  wd = inotify_add_watch( fd, rc_config.input_file, IN_CLOSE_WRITE );
  if( wd == -1 )
  {
    pr_err("Unable to configure file modification detection to %s: %s\n", rc_config.input_file,
           strerror(errno));
    return -1;
  }
  else
	  pr_debug("Monitoring rc_config.input_file: %s\n", rc_config.input_file);

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
      if (fd > 0) close( fd );

      fd = inotify_open( &pfd );
      if (fd < 0)
      {
        ClearFlag( OPTIMIZER_OUTPUT );
        return( NULL );
      }

      Strlcpy(prev_input_file, rc_config.input_file, sizeof(prev_input_file));
	}

    // Exit thread if optimizer output has been cancelled
    if( isFlagClear(OPTIMIZER_OUTPUT))
    {
      pr_debug("Exited optimizer thread.\n");
      break;
    }

    if ( CHILD )
	{
		pr_info("optimize.c: exit because this is a child process.\n");
		break;
	}

    // Poll inotify file descriptor, timeout 1 sec
    poll_num = poll( &pfd, 1, 1000 );

    if (poll_num == -1)
    {
      if( errno == EINTR ) continue;
      perror( "poll" );
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
          perror( "read" );
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
			pr_warn("%d child jobs are running, skipping optimization\n",
                                num_busy_procs);
			usleep(100000);
			continue;
		}

		int done = 0;
		struct stat st;

		// This fixes a crash when inotify reports the file is closed but the
		// file content is empty.  Interestingly, just calling `stat` on the
		// file seems to be enough to prevent the bug, but the code is here
		// just in case:
		do
		{
			errno = 0;
			int stat_ret = stat(rc_config.input_file, &st);
			done = (stat_ret == 0 && st.st_size > 0);
			if (!done)
			{
				pr_warn("%s: file is zero bytes, retrying (%s)\n", strerror(errno));
				usleep(100000);
			}
		} while (!done);

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
  if( fd >= 0) close( fd );
  return( NULL );
} // Optimizer_Output()

#endif // HAVE_INOTIFY

/*------------------------------------------------------------------------*/

