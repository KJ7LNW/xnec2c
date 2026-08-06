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

/* Pipe primitives are defined below their first use in this file */
static ssize_t Write_Pipe( int idx, char *str, ssize_t len );

/* Wire names of the parent/child commands, indexed by enum P2CH_COMND.
 * The row width holds every tag to FORK_CMD_LEN bytes, so a wider name
 * fails to compile. */
static const char fork_cmd_names[NUM_FKCMNDS][FORK_CMD_LEN + 1] = {
  [INFILE]  = "inpfile",
  [FRQDATA] = "frqdata",
};

/* One command payload field: the address transferred and its width */
typedef struct {
  void   *ptr;
  size_t  size;
} fork_field_t;

typedef ssize_t (*pipe_fn_t)(int, char *, ssize_t);

/* fork_fields_xfer()
 *
 * Transfers @nfields payload fields over child @idx's pipe in the direction
 * named by @pipe_fn.  Parent and child walk the same description, so neither
 * side can deviate from the other's byte stream.  A failed transfer never
 * returns here: both directions exit the process at the fault.
 */
static void
fork_fields_xfer( int idx, const fork_field_t *fields, int nfields,
                  pipe_fn_t pipe_fn )
{
  for( int i = 0; i < nfields; i++ )
    pipe_fn( idx, fields[i].ptr, (ssize_t)fields[i].size );
}

/* fork_xfer_infile()
 *
 * Transfers the INFILE payload over child @idx's pipe: the NEC2 input file
 * path, held by rc_config on both sides.
 */
static void
fork_xfer_infile( int idx, pipe_fn_t pipe_fn )
{
  fork_field_t fields[] = {
    { rc_config.input_file, sizeof(rc_config.input_file) },
  };

  fork_fields_xfer( idx, fields, (int)G_N_ELEMENTS(fields), pipe_fn );
}

/* fork_xfer_frqdata()
 *
 * Transfers the FRQDATA payload in @frq over child @idx's pipe: math library
 * id, thread budget, and the frequency to solve.
 */
static void
fork_xfer_frqdata( int idx, fork_frqdata_t *frq, pipe_fn_t pipe_fn )
{
  fork_field_t fields[] = {
    { frq->mathlib_id, sizeof(frq->mathlib_id) },
    { &frq->threads,   sizeof(frq->threads)    },
    { &frq->freq_mhz,  sizeof(frq->freq_mhz)   },
  };

  fork_fields_xfer( idx, fields, (int)G_N_ELEMENTS(fields), pipe_fn );
}

/* fork_send_cmd()
 *
 * Writes the wire tag of command @cmd to child @idx, ahead of its payload.
 */
  static void
fork_send_cmd( int idx, enum P2CH_COMND cmd )
{
  Write_Pipe( idx, (char *)fork_cmd_names[cmd], FORK_CMD_LEN );

} /* fork_send_cmd() */

/*------------------------------------------------------------------------*/

/* fork_send_infile()
 *
 * Sends the INFILE command tag and its payload to child @idx.
 */
  void
fork_send_infile( int idx )
{
  fork_send_cmd( idx, INFILE );
  fork_xfer_infile( idx, Write_Pipe );

} /* fork_send_infile() */

/*------------------------------------------------------------------------*/

/* fork_send_frqdata()
 *
 * Sends the FRQDATA command tag and the payload in @frq to child @idx.
 */
  void
fork_send_frqdata( int idx, fork_frqdata_t *frq )
{
  fork_send_cmd( idx, FRQDATA );
  fork_xfer_frqdata( idx, frq, Write_Pipe );

} /* fork_send_frqdata() */

/*------------------------------------------------------------------------*/

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
    if( strcmp(fork_cmd_names[idx], cdstr) == 0 )
      break;

  return( idx );

} /* Fork_Command() */

/*------------------------------------------------------------------------*/

static int write_exact(int fd, char *buf, int size)
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


static int read_exact(int fd, char *buf, int size)
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
Read_Pipe( int idx, char *str, ssize_t len)
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

  if( (retval == -1) || (retval != len ) )
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
    { crnt_fstep[fstep].air,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].aii,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].bir,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].bii,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].cir,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].cii,           size_npm_dbl,   0,                        FREQ_COND_ALWAYS },
    { crnt_fstep[fstep].cur,           size_np3m_cdbl, 0,                        FREQ_COND_ALWAYS },
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
    { near_field_fstep[fstep].points,  size_nf_points, 0,                        FREQ_COND_NEAREH },
    { &near_field_fstep[fstep].r_max,  NULL,           sizeof(double),           FREQ_COND_NEAREH },
  };

  int nfields = (int)(sizeof(fields) / sizeof(fields[0]));

  for (int i = 0; i < nfields; i++)
  {
    if (!freq_field_active(fields[i].cond))
      continue;

    size_t sz = fields[i].get_size ? fields[i].get_size() : fields[i].size;

    if (pipe_fn(pipe_idx, fields[i].ptr, (ssize_t)sz) < 0)
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
  char cmnd[FORK_CMD_LEN + 1];  /* Command string received from parent */
  fork_frqdata_t frq = { 0 };   /* FRQDATA payload received from parent */

  /* Close unwanted pipe ends */
  close( child_procs[num_child]->to_child[WRITE] );
  close( child_procs[num_child]->from_child[READ] );

  /* Loop around select() in Read_Pipe() waiting for commands/data */
  while( TRUE )
  {
    Read_Pipe( num_child, cmnd, FORK_CMD_LEN);
    cmnd[FORK_CMD_LEN] = '\0';

    switch( Fork_Command(cmnd) )
    {
      case INFILE: /* Read input file */
        fork_xfer_infile( num_child, Read_Pipe );
        rc_config.input_file[sizeof(rc_config.input_file) - 1] = '\0';
        Child_Input_File();
        break;

      case FRQDATA: /* Adopt the dispatched library, calculate currents and pass on */
        fork_xfer_frqdata( num_child, &frq, Read_Pipe );

        /* The budget arrives with the library it configures, so both land
         * before this frequency is solved. */
        mathlib_load( get_mathlib_by_id(frq.mathlib_id) );
        mathlib_set_num_threads( current_mathlib, frq.threads );

        /* Dedup cache persists across sweeps in child address space;
         * reset ensures every dispatched frequency is recomputed */
        New_Frequency_Reset_Prev();

        calc_data.freq_mhz = frq.freq_mhz;

        /* Frequency buffers in children are for current frequency only */
        calc_data.freq_step = 0;

        /* Set flags */
        SetFlag( FREQ_LOOP_RUNNING );

        /* Calculate freq data */
        New_Frequency();
        Pass_Freq_Data();
        break;

      default:
        BUG( "unrecognized command \"%s\" from parent\n", cmnd );
        break;

    } /* switch( Command(cmnd) ) */
  } /* while( TRUE ) */

} /* Child_Process() */

/*-----------------------------------------------------------------------*/

/* Write_Pipe()
 *
 * Writes data to a pipe (child and parent processes)
 */
  static ssize_t
Write_Pipe( int idx, char *str, ssize_t len )
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
  if( (retval == -1) || (retval != len) )
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
static ssize_t PRead_Pipe(int idx, char *str, ssize_t len)
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

  /* Parent publication point: the child's counter never crosses the pipe,
   * so the parent stamps its own token once the slot content is complete */
  if( isFlagSet(ENABLE_NEAREH) )
    near_field_fstep[fstep].content_generation = ++near_field_generation;

  return 1;
} /* Get_Freq_Data() */

/*------------------------------------------------------------------------*/

