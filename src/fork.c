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

#include "fork.h"
#include "shared.h"
#include "measurements.h"
#include "prerender/prerender_color.h"
#include "prerender/prerender_state.h"
#include "mathlib.h"

/*-----------------------------------------------------------------------*/

/* Child_Input_File()
 *
 * Opens NEC2 input file for child processes
 */
  static void
Child_Input_File( void )
{
  /* Close open files if any */
  Close_File( &input_fp );

  /* Open NEC2 input file */
  if( strlen(rc_config.input_file) == 0 ) return;
  Open_File( &input_fp, rc_config.input_file, "r" );

  /* Read input file */
  ClearFlag( ALL_FLAGS );
  SetFlag( INPUT_PENDING );
  Read_Comments();
  Read_Geometry();
  Read_Commands();
  ClearFlag( INPUT_PENDING );

  /* Initialize xnec2c child */
  New_Frequency_Reset_Prev();

} /* Child_Input_FIle() */

/*------------------------------------------------------------------------*/

/* Fork_Command()
 *
 * Identifies a command string
 */
  static int
Fork_Command( const char *cdstr )
{
  int idx;

  for( idx = 0; idx < NUM_FKCMNDS; idx++ )
    if( strcmp(fork_commands[idx], cdstr) == 0 )
      break;

  return( idx );

} /* Fork_Command() */

/*------------------------------------------------------------------------*/

int write_exact(int fd, char *buf, int size)
{
	int len = 0;
	int offset = 0;

	do {
		len = write(fd, buf + offset, size);

		if (len < 0 && errno == EINTR) continue;

		if (len < 0) {
			perror("write()");
			return len;
		}

		if (!len)
			size = 0;
		else {
			size -= len;
			offset += len;
		}
	} while (size);

	return offset;
}


int read_exact(int fd, char *buf, int size)
{
	int len = 0;
	int offset = 0;

	do {
		len = read(fd, buf + offset, size);

		if (len < 0 && errno == EINTR) continue;

		if (len < 0) {
			perror("read()");
			return len;
		}

		if (!len)
			size = 0;
		else {
			size -= len;
			offset += len;
		}
	} while (size);

	return offset;
}


/* close_child_command_pipes()
 *
 * Closes every forked child's command pipe so each wakes on EOF and routes
 * through pipe_fail_exit() -> child_exit(); a child mid-write drains in
 * child_procs_free() before it is reaped.  Only close() and integer work runs
 * here, so a signal handler may call it directly; the SIGKILL that teardown
 * would otherwise force denies each child its report.  A no-op in a child or
 * when unforked.
 */
  void
close_child_command_pipes( void )
{
  if( !FORKED || CHILD )
    return;

  while( num_child_procs )
  {
    num_child_procs--;
    close( child_procs[num_child_procs]->to_child[WRITE] );
  }

} /* close_child_command_pipes() */

/*------------------------------------------------------------------------*/

/* child_exit()
 *
 * Single child cleanup chokepoint.  Releases the child's mem-tracked owners
 * and emits the report through child_cleanup(), then exits.  Reached from
 * pipe_fail_exit() when the parent closes the command pipe (EOF) or on any
 * pipe error.
 */
static void child_exit( void ) __attribute__ ((noreturn));
  static void
child_exit( void )
{
  child_cleanup();
  _exit( 0 );

} /* child_exit() */

/*------------------------------------------------------------------------*/

/* pipe_fail_exit()
 *
 * Pipe teardown chokepoint.  In the forked child a broken pipe means the
 * parent is gone, so route through child_exit() to release the child's owners;
 * the parent reaches this only on an unrecoverable transfer error and exits
 * directly.
 */
  static void
pipe_fail_exit( void )
{
  if( CHILD ) child_exit();
  _exit( 0 );

} /* pipe_fail_exit() */

/*------------------------------------------------------------------------*/

/* Read_Pipe()
 *
 * Reads data from a pipe (child and parent processes)
 */
  static ssize_t
Read_Pipe( int idx, char *str, ssize_t len, gboolean err )
{
  ssize_t retval;
  int pipefd;

  do {
	  if(CHILD)
		pipefd = child_procs[idx]->to_child[READ];
	  else
		pipefd = child_procs[idx]->from_child[READ];

	  fd_set rds;
	  FD_ZERO( &rds );
	  FD_SET( pipefd, &rds );
	  retval = select( 1024, &rds, NULL, NULL, NULL );

	  if (retval == -1 && errno != EINTR)
	  {
		perror( "select()" );
		pipe_fail_exit();
	  }
  } while (retval == -1 && errno == EINTR);
  
  retval = read_exact( pipefd, str, (size_t)len );

  /* A zero-length read in the child is the parent closing the command pipe at
   * teardown: EOF is the graceful-quit signal, not a transfer error.  Route to
   * child_exit() without reporting a short read.  The parent keeps the error
   * path below so a child that dies mid-run still surfaces. */
  if( CHILD && retval == 0 ) child_exit();

  if( (retval == -1) || ((retval != len) && err ) )
  {
    perror( "read()" );
    pr_err("child %d  length %d  return %d\n", idx, (int)len, (int)retval);
    pipe_fail_exit();
  }
  return( retval );

} /* Read_Pipe() */

/*------------------------------------------------------------------------*/

/* Transfer condition for frequency-domain pipe fields */
enum freq_field_cond {
  FREQ_COND_ALWAYS,
  FREQ_COND_RDPAT,
  FREQ_COND_NEAREH
};

typedef struct {
  void *ptr;
  size_t (*get_size)(void);
  size_t size;
  int cond;
} freq_field_t;

typedef ssize_t (*pipe_fn_t)(int, char *, ssize_t, gboolean);

static size_t size_npm_dbl(void)        { return (size_t)data.npm  * sizeof(double); }
static size_t size_np3m_cdbl(void)      { return (size_t)data.np3m * sizeof(complex double); }
static size_t size_nphth_dbl(void)      { return (size_t)(fpat.nph * fpat.nth) * sizeof(double); }
static size_t size_nphth_int(void)      { return (size_t)(fpat.nph * fpat.nth) * sizeof(int); }
static size_t size_n_ports_dbl(void)    { return (size_t)Num_Feedpoint_Ports() * sizeof(double); }
static size_t size_nf_points(void)      { return (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(near_field_point_t); }
static size_t size_patch_flow(void)     { return (size_t)data.m  * 4 * sizeof(float); }

/* freq_field_active()
 *
 * Returns TRUE if the given transfer condition is satisfied by
 * the current flag state.
 */
static gboolean
freq_field_active(int cond)
{
  switch (cond)
  {
    case FREQ_COND_ALWAYS: return TRUE;
    case FREQ_COND_RDPAT:  return isFlagSet(ENABLE_RDPAT);
    case FREQ_COND_NEAREH: return isFlagSet(ENABLE_NEAREH);
    default: abort();
  }
}

/* freq_fields_xfer()
 *
 * Single schema for frequency-data pipe transfer.  Each field is piped
 * directly via pipe_fn (Write_Pipe from child, PRead_Pipe from parent).
 * Both parent and child parse the same NEC2 input file via Read_Commands(),
 * so ENABLE_NEAREH and ENABLE_RDPAT are identical on both sides; the
 * field table is therefore evaluated identically by both caller sites.
 */
static int
freq_fields_xfer(int fstep, int pipe_idx, pipe_fn_t pipe_fn)
{
  /* Local (non-static) array; runtime pointer values go directly in initializers */
  freq_field_t fields[] = {
    /* Current and charge data */
    { crnt.air,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.aii,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.bir,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.bii,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.cir,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.cii,                        size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt.cur,                        size_np3m_cdbl, 0,                        FREQ_COND_ALWAYS },
    /* Per-port impedance data (fstep=0 on child, fstep=N on parent); each
     * member spans Num_Feedpoint_Ports() doubles, identical parent and child. */
    { impedance_data[fstep].zreal,     size_n_ports_dbl, 0,                      FREQ_COND_ALWAYS },
    { impedance_data[fstep].zimag,     size_n_ports_dbl, 0,                      FREQ_COND_ALWAYS },
    { impedance_data[fstep].zmagn,     size_n_ports_dbl, 0,                      FREQ_COND_ALWAYS },
    { impedance_data[fstep].zphase,    size_n_ports_dbl, 0,                      FREQ_COND_ALWAYS },
    /* Radiation pattern data */
    { rad_pattern[fstep].gtot,         size_nphth_dbl, 0,                        FREQ_COND_RDPAT  },
    { rad_pattern[fstep].tilt,         size_nphth_dbl, 0,                        FREQ_COND_RDPAT  },
    { rad_pattern[fstep].axrt,         size_nphth_dbl, 0,                        FREQ_COND_RDPAT  },
    { rad_pattern[fstep].max_gain,     NULL,           NUM_POL * sizeof(double), FREQ_COND_RDPAT  },
    { rad_pattern[fstep].min_gain,     NULL,           NUM_POL * sizeof(double), FREQ_COND_RDPAT  },
    { rad_pattern[fstep].max_gain_tht, NULL,           NUM_POL * sizeof(double), FREQ_COND_RDPAT  },
    { rad_pattern[fstep].max_gain_phi, NULL,           NUM_POL * sizeof(double), FREQ_COND_RDPAT  },
    { rad_pattern[fstep].max_gain_idx, NULL,           NUM_POL * sizeof(int),    FREQ_COND_RDPAT  },
    { rad_pattern[fstep].min_gain_idx, NULL,           NUM_POL * sizeof(int),    FREQ_COND_RDPAT  },
    { rad_pattern[fstep].sens,         size_nphth_int, 0,                        FREQ_COND_RDPAT  },
    { &rad_pattern[fstep].efficiency,  NULL,           sizeof(double),           FREQ_COND_RDPAT  },
    /* Per-fstep noise temperature table (allocated alongside rad_pattern[]) */
    { &noise_temp[fstep],              NULL,              sizeof(noise_temp_t),  FREQ_COND_RDPAT  },
    /* Per-fstep structure colors (patch flow + cmin/cmax range scalars) */
    { struct_colors[fstep].patch_flow_data, size_patch_flow, 0,                FREQ_COND_ALWAYS },
    { &struct_colors[fstep].wire_crnt_cmin, NULL,          sizeof(float),       FREQ_COND_ALWAYS },
    { &struct_colors[fstep].wire_crnt_cmax, NULL,          sizeof(float),       FREQ_COND_ALWAYS },
    { &struct_colors[fstep].wire_chrg_cmin, NULL,          sizeof(float),       FREQ_COND_ALWAYS },
    { &struct_colors[fstep].wire_chrg_cmax, NULL,          sizeof(float),       FREQ_COND_ALWAYS },
    { &struct_colors[fstep].patch_crnt_cmin, NULL,         sizeof(float),       FREQ_COND_ALWAYS },
    { &struct_colors[fstep].patch_crnt_cmax, NULL,         sizeof(float),       FREQ_COND_ALWAYS },
    /* Near field data: the phasor and spatial extent only; color and
     * geometry derive in the parent at draw and never cross the pipe */
    { near_field.points,               size_nf_points, 0,                        FREQ_COND_NEAREH },
    { &near_field.r_max,               NULL,           sizeof(double),           FREQ_COND_NEAREH },
  };

  int nfields = (int)(sizeof(fields) / sizeof(fields[0]));

  for (int i = 0; i < nfields; i++)
  {
    if (!freq_field_active(fields[i].cond))
      continue;

    size_t sz = fields[i].get_size ? fields[i].get_size() : fields[i].size;

    if (pipe_fn(pipe_idx, fields[i].ptr, (ssize_t)sz, TRUE) < 0)
      return 0;
  }

  return 1;
}

/*------------------------------------------------------------------------*/

/* Pass_Freq_Data()
 *
 * Passes frequency-dependent data (current, charge density,
 * input impedances etc) from child processes to parent.
 */
  static void
Pass_Freq_Data( void )
{
  freq_fields_xfer(0, num_child_procs, Write_Pipe);

} /* Pass_Freq_Data() */

/*------------------------------------------------------------------------*/

/* Child_Process()
 *
 * Destination of child processes, handles data
 * transfers between parent and children via pipes
 */
void Child_Process( int num_child ) __attribute__ ((noreturn));
  void
Child_Process( int num_child )
{
  ssize_t retval;   /* Return from select()/read() etc */
  char cmnd[8];     /* Command string received from parent */
  char *buff;       /* Passes address of variables to read()/write() */
  size_t cnt;       /* Size of data buffers for read()/write() */
  int threads = 0;  /* Thread budget relayed with each mathlib command */

  /* Close unwanted pipe ends */
  close( child_procs[num_child]->to_child[WRITE] );
  close( child_procs[num_child]->from_child[READ] );

  /* Loop around select() in Read_Pipe() waiting for commands/data */
  while( TRUE )
  {
    retval = Read_Pipe( num_child, cmnd, 7, TRUE );
    cmnd[retval]='\0';

    switch( Fork_Command(cmnd) )
    {
      case MATHLIB:
        Read_Pipe( num_child,
			rc_config.mathlib_batch_id,
			MATHLIB_ID_LEN, FALSE );

        buff = (char *) &threads;
        cnt = sizeof( int );
        Read_Pipe( num_child, buff, (ssize_t)cnt, TRUE );

        // Clear the previous frequency cache to prevent false values from benchmarking:
        if (strcmp(current_mathlib->id, rc_config.mathlib_batch_id) != 0)
            New_Frequency_Reset_Prev();

        mathlib_load(get_mathlib_by_id(rc_config.mathlib_batch_id));

        mathlib_set_num_threads(current_mathlib, threads);
        break;

      case INFILE: /* Read input file */
        retval = Read_Pipe( num_child, rc_config.input_file, sizeof(rc_config.input_file), FALSE );
        rc_config.input_file[retval-1] = '\0';
        Child_Input_File();
        break;

      case FRQDATA: /* Calculate currents and pass on */
        /* Dedup cache persists across sweeps in child address space;
         * reset ensures every dispatched frequency is recomputed */
        New_Frequency_Reset_Prev();

        /* Get new frequency */
        buff = (char *) &calc_data.freq_mhz;
        cnt = sizeof( double );
        Read_Pipe( num_child, buff, (ssize_t)cnt, TRUE );

        /* Frequency buffers in children are for current frequency only */
        calc_data.freq_step = 0;

        /* Set flags */
        SetFlag( FREQ_LOOP_RUNNING );

        /* Calculate freq data */
        New_Frequency();
        Pass_Freq_Data();
        break;

    } /* switch( Command(cmnd) ) */
  } /* while( TRUE ) */

} /* Child_Process() */

/*-----------------------------------------------------------------------*/

/* Write_Pipe()
 *
 * Writes data to a pipe (child and parent processes)
 */
  ssize_t
Write_Pipe( int idx, char *str, ssize_t len, gboolean err )
{
  ssize_t retval;
  int pipefd;

  do {
	  if( CHILD )
		pipefd = child_procs[idx]->from_child[WRITE];
	  else
		pipefd = child_procs[idx]->to_child[WRITE];

	  fd_set wds;
	  FD_ZERO( &wds );
	  FD_SET( pipefd, &wds );
	  retval = select( 1024, NULL, &wds, NULL, NULL );

	  if (retval == -1 && errno != EINTR)
	  {
		perror( "select()" );
		pipe_fail_exit();
	  }

  } while (retval == -1 && errno == EINTR);


  retval = write_exact( pipefd, str, (size_t)len );
  if( (retval == -1) || ((retval != len) && err) )
  {
    perror( "write()" );
    pipe_fail_exit();
  }

  return( retval );

} /* Write_Pipe() */

/*------------------------------------------------------------------------*/

/* PRead_Pipe()
 *
 * Reads data from a pipe (used by parent process)
 */
static ssize_t PRead_Pipe(int idx, char *str, ssize_t len, gboolean err)
{
	ssize_t retval;

	// Repeat read() if not all data returned 
	retval = read_exact(child_procs[idx]->from_child[READ], str, (size_t) len);
	if (retval < 0)
	{
		perror("read()");
		pr_err("child %d  length %d  return %d\n", idx, (int)len, (int)retval);
		_exit(0);
	}

	if (retval == 0 && len > 0)
	{
		pr_err("early EOF?, child %d  length %d  return %d\n", idx, (int)len, (int)retval);
		return -1;
	}

	return (retval);
}	/* PRead_Pipe() */

/*------------------------------------------------------------------------*/

/* Get_Freq_Data()
 *
 * Gets frequency-dependent data (current, charge density,
 * input impedances etc) from child processes.
 *
 * Be sure to hold the freq_data_lock mutex when calling this function.
 */
  int
Get_Freq_Data( int idx, int fstep )
{
  if (!freq_fields_xfer(fstep, idx, PRead_Pipe))
    return 0;

  return 1;
} /* Get_Freq_Data() */

/*------------------------------------------------------------------------*/

