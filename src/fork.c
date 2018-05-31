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

#include "fork.h"
#include "shared.h"

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
  if( strlen(rc_config.infile) == 0 ) return;
  Open_File( &input_fp, rc_config.infile, "r" );

  /* Read input file */
  ClearFlag( ALL_FLAGS );
  SetFlag( INPUT_PENDING );
  Read_Comments();
  Read_Geometry();
  Read_Commands();
  ClearFlag( INPUT_PENDING );

  /* Initialize xnec2c child */
  save.last_freq = 0.0;
  crnt.newer = crnt.valid = 0;

} /* Child_Input_FIle() */

/*------------------------------------------------------------------------*/

/* Fork_Command()
 *
 * Identifies a command srting
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

/* Read_Pipe()
 *
 * Reads data from a pipe (child and parent processes)
 */
  static ssize_t
Read_Pipe( int idx, char *str, ssize_t len, gboolean err )
{
  ssize_t retval;
  int pipefd;

  if(CHILD)
	pipefd = forked_proc_data[idx]->pnt2child_pipe[READ];
  else
	pipefd = forked_proc_data[idx]->child2pnt_pipe[READ];

  retval = select( 1024, &forked_proc_data[idx]->read_fds, NULL, NULL, NULL );
  if( retval == -1 )
  {
	perror( "xnec2c: select()" );
	_exit(0);
  }

  retval = read( pipefd, str, (size_t)len );
  if( (retval == -1) || ((retval != len) && err ) )
  {
	perror( "xnec2c: Read_Pipe(): read()" );
	fprintf( stderr, "xnec2c: Read_Pipe(): child %d  length %d  return %d\n",
		idx, (int)len, (int)retval );
	_exit(0);
  }
  return( retval );

} /* Read_Pipe() */

/*------------------------------------------------------------------------*/

/* Mem_Copy()
 *
 * Copies between buffers using memcpy()
 */
  static void
Mem_Copy( char *buff, char *var, size_t cnt, gboolean wrt )
{
  static int idx;

  /* Clear idx to buffer */
  if( !cnt )
  {
	idx = 0;
	return;
  }

  /* If child process writing data */
  if( wrt )
	memcpy( &buff[idx], var, cnt );
  else /* Parent reading data */
	memcpy( var, &buff[idx], cnt );
  idx += (int)cnt;

} /* Mem_Copy() */

/*------------------------------------------------------------------------*/

/* Pass_Freq_Data()
 *
 * Passes frequency-dependent data (current, charge density,
 * input impedances etc) from child processes to parent.
 */
  static void
Pass_Freq_Data( void )
{
  char *buff = NULL, flag;
  size_t cnt, buff_size;

  /*** Total of bytes to read/write thru pipe ***/
  buff_size =
	/* Current & charge data (a, b, c, ir & ii) */
	(size_t)(6 * data.npm) * sizeof( double ) +
	/* Complex current (crnt.cur) */
	(size_t)data.np3m * sizeof( complex double ) +
	/* newer and valid flags */
	2 * sizeof(char) +
	/* Impedance data */
	4 * sizeof(double) +
	/* Network data */
	sizeof(complex double);

  /* Radiation pattern data if enabled */
  if( isFlagSet(ENABLE_RDPAT) )
  {
	buff_size +=
	  /* Gain total, tilt, axial ratio */
	  (size_t)(3 * fpat.nph * fpat.nth) * sizeof(double) +
	  /* max & min gain, tht & phi angles */
	  (size_t)(4 * NUM_POL) * sizeof(double) +
	  /* max and min gain index */
	  (size_t)(2 * NUM_POL) * sizeof(int) +
	  /* Polarization sens */
	  (size_t)(fpat.nph * fpat.nth) * sizeof(int) +
	  /* New pattern flag */
	  sizeof( char );
  }

  /* Near field data if enabled */
  if( isFlagSet(DRAW_EHFIELD) )
  {
	/* Notify parent to read near field data */
	Write_Pipe( num_child_procs, "nfeh", 4, TRUE );

	/* Near E field data */
	if( fpat.nfeh & NEAR_EFIELD )
	  buff_size +=
		(size_t)( 10 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double);
	/* Near H field data */
	if( fpat.nfeh & NEAR_HFIELD )
	  buff_size +=
		(size_t)( 10 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double);
	/* Co-ordinates of field points */
	buff_size +=
	  (size_t)( 3 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double) +
	  /* newer & valid flags */
	  2 * sizeof(char);
  }
  else /* Notify parent not to read near field data */
	Write_Pipe( num_child_procs, "noeh", 4, TRUE );

  /* Allocate data buffers */
  mem_alloc( (void **)&buff, buff_size, "in fork.c" );

  /* Clear buffer index in this function */
  Mem_Copy( buff, buff, 0, WRITE );

  /* Pass on current and charge data */
  cnt =  (size_t)data.npm * sizeof( double );
  Mem_Copy( buff, (char *)crnt.air, cnt, WRITE );
  Mem_Copy( buff, (char *)crnt.aii, cnt, WRITE );
  Mem_Copy( buff, (char *)crnt.bir, cnt, WRITE );
  Mem_Copy( buff, (char *)crnt.bii, cnt, WRITE );
  Mem_Copy( buff, (char *)crnt.cir, cnt, WRITE );
  Mem_Copy( buff, (char *)crnt.cii, cnt, WRITE );

  cnt = (size_t)data.np3m * sizeof( complex double );
  Mem_Copy( buff, (char *)crnt.cur, cnt, WRITE );

  cnt = sizeof( char );
  Mem_Copy( buff, &crnt.newer, cnt, WRITE );
  Mem_Copy( buff, &crnt.valid, cnt, WRITE );

  /* Impedance data */
  cnt = sizeof(double);
  Mem_Copy( buff, (char *)&impedance_data.zreal[0],  cnt, WRITE );
  Mem_Copy( buff, (char *)&impedance_data.zimag[0],  cnt, WRITE );
  Mem_Copy( buff, (char *)&impedance_data.zmagn[0],  cnt, WRITE );
  Mem_Copy( buff, (char *)&impedance_data.zphase[0], cnt, WRITE );

  /* Network data */
  cnt = sizeof(complex double);
  Mem_Copy( buff, (char *)&netcx.zped, cnt, WRITE );

  /* Pass on radiation pattern data if enabled */
  if( isFlagSet(ENABLE_RDPAT) )
  {
	cnt = (size_t)(fpat.nph * fpat.nth) * sizeof(double);
	Mem_Copy( buff, (char *)rad_pattern[0].gtot, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].tilt, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].axrt, cnt, WRITE );

	cnt = (size_t)NUM_POL * sizeof(double);
	Mem_Copy( buff, (char *)rad_pattern[0].max_gain, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].min_gain, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].max_gain_tht, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].max_gain_phi, cnt, WRITE );

	cnt = (size_t)NUM_POL * sizeof(int);
	Mem_Copy( buff, (char *)rad_pattern[0].max_gain_idx, cnt, WRITE );
	Mem_Copy( buff, (char *)rad_pattern[0].min_gain_idx, cnt, WRITE );

	cnt = (size_t)(fpat.nph * fpat.nth) * sizeof(int);
	Mem_Copy( buff, (char *)rad_pattern[0].sens, cnt, WRITE );

	if( isFlagSet(DRAW_NEW_RDPAT) )
	  flag = 1;
	else
	  flag = 0;
	cnt = sizeof( char );
	Mem_Copy( buff, &flag, cnt, WRITE );
  }

  /* Near field data */
  if( isFlagSet(DRAW_EHFIELD) )
  {
	/* Magnitude and phase of E field */
	if( fpat.nfeh & NEAR_EFIELD )
	{
	  cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	  Mem_Copy( buff, (char *)near_field.ex, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.ey, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.ez, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fex, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fey, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fez, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.erx, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.ery, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.erz, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.er, cnt, WRITE );
	  cnt = sizeof(double);
	  Mem_Copy( buff, (char *)&near_field.max_er, cnt, WRITE );
	}

	/* Magnitude and phase of H fields */
	if( fpat.nfeh & NEAR_HFIELD )
	{
	  cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	  Mem_Copy( buff, (char *)near_field.hx, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hy, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hz, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fhx, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fhy, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.fhz, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hrx, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hry, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hrz, cnt, WRITE );
	  Mem_Copy( buff, (char *)near_field.hr, cnt, WRITE );
	  cnt = sizeof(double);
	  Mem_Copy( buff, (char *)&near_field.max_hr, cnt, WRITE );
	}

	/* Co-ordinates of field points */
	cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	Mem_Copy( buff, (char *)near_field.px, cnt, WRITE );
	Mem_Copy( buff, (char *)near_field.py, cnt, WRITE );
	Mem_Copy( buff, (char *)near_field.pz, cnt, WRITE );
	cnt = sizeof(double);
	Mem_Copy( buff, (char *)&near_field.r_max, cnt, WRITE );

	cnt = sizeof( char );
	Mem_Copy( buff, &near_field.newer, cnt, WRITE );
	Mem_Copy( buff, &near_field.valid, cnt, WRITE );

  } /* if( isFlagSet(DRAW_EHFIELD) ) */

  /* Pass data accumulated in buffer if child */
  Write_Pipe( num_child_procs, buff, (ssize_t)buff_size, TRUE );

  free_ptr( (void **)&buff );

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
  ssize_t retval;	/* Return from select()/read() etc */
  char cmnd[8];		/* Command string received from parent */
  char *buff;		/* Passes address of variables to read()/write() */
  size_t cnt;		/* Size of data buffers for read()/write() */

  /* Close unwanted pipe ends */
  close( forked_proc_data[num_child]->pnt2child_pipe[WRITE] );
  close( forked_proc_data[num_child]->child2pnt_pipe[READ] );

  /* Watch read/write pipe for i/o */
  FD_ZERO( &forked_proc_data[num_child]->read_fds );

  FD_SET( forked_proc_data[num_child]->pnt2child_pipe[READ],
	  &forked_proc_data[num_child]->read_fds );

  FD_ZERO( &forked_proc_data[num_child]->write_fds );

  FD_SET( forked_proc_data[num_child]->child2pnt_pipe[WRITE],
	  &forked_proc_data[num_child]->write_fds );

  /* Loop around select() in Read_Pipe() waiting for commands/data */
  while( TRUE )
  {
	retval = Read_Pipe( num_child, cmnd, 7, TRUE );
	cmnd[retval]='\0';

	switch( Fork_Command(cmnd) )
	{
	  case INFILE: /* Read input file */
		retval = Read_Pipe( num_child, rc_config.infile, 80, FALSE );
		rc_config.infile[retval] = '\0';
		Child_Input_File();
		break;

	  case FRQDATA: /* Calculate currents and pass on */
		/* Get new frequency */
		buff = (char *)&calc_data.fmhz;
		cnt = sizeof( double );
		Read_Pipe( num_child, buff, (ssize_t)cnt, TRUE );

		/* Frequency buffers in children
		 * are for current frequency only */
		calc_data.fstep = 0;

		/* Clear "last-used-frequency" buffer */
		save.last_freq = 0.0;

		/* Set flags */
		SetFlag( FREQ_LOOP_RUNNING );

		/* Calculate freq data and pass to parent */
		New_Frequency();
		Pass_Freq_Data();
		break;

	  case EHFIELD: /* Calcualte near field E/H data */
		{
		  /* Get near field flags */
		  char flag;

		  /* Set near field flags */
		  cnt = sizeof( flag );
		  Read_Pipe( num_child, &flag, (ssize_t)cnt, TRUE );

		  if( flag & E_HFIELD )
			SetFlag( DRAW_EHFIELD );
		  else
			ClearFlag( DRAW_EHFIELD );

		  if( flag & SNAPSHOT )
			SetFlag( NEAREH_SNAPSHOT );
		  else
			ClearFlag( NEAREH_SNAPSHOT );

		  if( flag & EFIELD )
			SetFlag( DRAW_EFIELD );
		  else
			ClearFlag( DRAW_EFIELD );

		  if( flag & HFIELD )
			SetFlag( DRAW_HFIELD );
		  else
			ClearFlag( DRAW_HFIELD );
		}

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

  if( CHILD )
	pipefd = forked_proc_data[idx]->child2pnt_pipe[WRITE];
  else
	pipefd = forked_proc_data[idx]->pnt2child_pipe[WRITE];

  retval = select( 1024, NULL, &forked_proc_data[idx]->write_fds, NULL, NULL );
  if( retval == -1 )
  {
	perror( "xnec2c: select()" );
	_exit(0);
  }

  retval = write( pipefd, str, (size_t)len );
  if( (retval == -1) || ((retval != len) && err) )
  {
	perror( "xnec2c: write()" );
	_exit(0);
  }

  usleep(2);
  return( retval );

} /* Write_Pipe() */

/*------------------------------------------------------------------------*/

/* PRead_Pipe()
 *
 * Reads data from a pipe (used by parent process)
 */
  static ssize_t
PRead_Pipe( int idx, char *str, ssize_t len, gboolean err )
{
  ssize_t retval;

  retval = read( forked_proc_data[idx]->child2pnt_pipe[READ], str, (size_t)len );
  if( (retval == -1) || ((retval != len) && err ) )
  {
	perror( "xnec2c: PRead_Pipe(): read()" );
	_exit(0);
  }
  return( retval );

} /* PRead_Pipe() */

/*------------------------------------------------------------------------*/

/* Get_Freq_Data()
 *
 * Gets frequency-dependent data (current, charge density,
 * input impedances etc) from child processes.
 */
  void
Get_Freq_Data( int idx, int fstep )
{
  char *buff = NULL, flag;
  char nfeh[5];
  size_t cnt, buff_size;

  /*** Total of bytes to read/write thru pipe ***/
  buff_size =
	/* Current & charge data (a, b, c ir & ii) */
	(size_t)(6 * data.npm) * sizeof( double ) +
	/* Complex current (crnt.cur) */
	(size_t)data.np3m * sizeof( complex double ) +
	/* newer and valid flags */
	2 * sizeof(char) +
	/* Impedance data */
	4 * sizeof(double) +
	/* Network data */
	sizeof(complex double);

  /* Radiation pattern data if enabled */
  if( isFlagSet(ENABLE_RDPAT) )
  {
	buff_size +=
	  /* Gain total, tilt, axial ratio */
	  (size_t)(3 * fpat.nph * fpat.nth) * sizeof(double) +
	  /* max & min gain, tht & phi angles */
	  (size_t)(4 * NUM_POL) * sizeof(double) +
	  /* max and min gain index */
	  (size_t)(2 * NUM_POL) * sizeof(int) +
	  /* Polarization sens */
	  (size_t)(fpat.nph * fpat.nth) * sizeof(int) +
	  /* New pattern flag */
	  sizeof( char );
  }

  /* Notification to read near field data */
  PRead_Pipe( idx, nfeh, 4, TRUE );
  nfeh[4] = '\0';

  /* Get near field data if enabled */
  if( strcmp(nfeh, "nfeh") == 0 )
  {
	/* Near E field data */
	if( fpat.nfeh & NEAR_EFIELD )
	  buff_size +=
		(size_t)( 10 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double);
	/* Near H field data */
	if( fpat.nfeh & NEAR_HFIELD )
	  buff_size +=
		(size_t)( 10 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double);
	/* Co-ordinates of field points */
	buff_size +=
	  (size_t)( 3 * fpat.nrx * fpat.nry * fpat.nrz + 1 ) * sizeof(double) +
	  /* newer & valid flags */
	  2 * sizeof(char);
  }

  /* Allocate data buffer */
  mem_alloc( (void **)&buff, buff_size, "in fork.c" );

  /* Clear buffer index in this function */
  Mem_Copy( buff, buff, 0, READ );

  /* Get data accumulated in buffer if child */
  PRead_Pipe( idx, buff, (ssize_t)buff_size, TRUE );

  /* Get current and charge data */
  cnt =  (size_t)data.npm * sizeof( double );
  Mem_Copy( buff, (char *)crnt.air, cnt, READ );
  Mem_Copy( buff, (char *)crnt.aii, cnt, READ );
  Mem_Copy( buff, (char *)crnt.bir, cnt, READ );
  Mem_Copy( buff, (char *)crnt.bii, cnt, READ );
  Mem_Copy( buff, (char *)crnt.cir, cnt, READ );
  Mem_Copy( buff, (char *)crnt.cii, cnt, READ );

  cnt = (size_t)data.np3m * sizeof( complex double );
  Mem_Copy( buff, (char *)crnt.cur, cnt, READ );

  cnt = sizeof( char );
  Mem_Copy( buff, &crnt.newer, cnt, READ );
  Mem_Copy( buff, &crnt.valid, cnt, READ );

  /* Get impedance data */
  cnt = sizeof(double);
  Mem_Copy( buff, (char *)&impedance_data.zreal[fstep], cnt, READ );
  Mem_Copy( buff, (char *)&impedance_data.zimag[fstep], cnt, READ );
  Mem_Copy( buff, (char *)&impedance_data.zmagn[fstep], cnt, READ );
  Mem_Copy( buff, (char *)&impedance_data.zphase[fstep], cnt, READ );

  /* Get network data */
  cnt = sizeof(complex double);
  Mem_Copy( buff, (char *)&netcx.zped, cnt, READ );

  /* Get radiation pattern data if enabled */
  if( isFlagSet(ENABLE_RDPAT) )
  {
	cnt = (size_t)(fpat.nph * fpat.nth) * sizeof(double);
	Mem_Copy( buff, (char *)rad_pattern[fstep].gtot, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].tilt, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].axrt, cnt, READ );

	cnt = (size_t)NUM_POL * sizeof(double);
	Mem_Copy( buff, (char *)rad_pattern[fstep].max_gain, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].min_gain, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].max_gain_tht, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].max_gain_phi, cnt, READ );

	cnt = (size_t)NUM_POL * sizeof(int);
	Mem_Copy( buff, (char *)rad_pattern[fstep].max_gain_idx, cnt, READ );
	Mem_Copy( buff, (char *)rad_pattern[fstep].min_gain_idx, cnt, READ );

	cnt = (size_t)(fpat.nph * fpat.nth) * sizeof(int);
	Mem_Copy( buff, (char *)rad_pattern[fstep].sens, cnt, READ );

	Mem_Copy( buff, &flag, sizeof(flag), READ );
	if( flag ) SetFlag( DRAW_NEW_RDPAT );
  }

  /* Get near field data if signaled by child */
  if( strcmp(nfeh, "nfeh") == 0 )
  {
	/* Magnitude and phase of E field */
	if( fpat.nfeh & NEAR_EFIELD )
	{
	  cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	  Mem_Copy( buff, (char *)near_field.ex, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.ey, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.ez, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fex, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fey, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fez, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.erx, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.ery, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.erz, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.er, cnt, READ );
	  cnt = sizeof(double);
	  Mem_Copy( buff, (char *)&near_field.max_er, cnt, READ );
	}

	/* Magnitude and phase of H fields */
	if( fpat.nfeh & NEAR_HFIELD )
	{
	  cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	  Mem_Copy( buff, (char *)near_field.hx, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hy, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hz, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fhx, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fhy, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.fhz, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hrx, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hry, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hrz, cnt, READ );
	  Mem_Copy( buff, (char *)near_field.hr, cnt, READ );
	  cnt = sizeof(double);
	  Mem_Copy( buff, (char *)&near_field.max_hr, cnt, READ );
	}

	/* Co-ordinates of field points */
	cnt = (size_t)(fpat.nrx * fpat.nry * fpat.nrz) * sizeof(double);
	Mem_Copy( buff, (char *)near_field.px, cnt, READ );
	Mem_Copy( buff, (char *)near_field.py, cnt, READ );
	Mem_Copy( buff, (char *)near_field.pz, cnt, READ );
	cnt = sizeof(double);
	Mem_Copy( buff, (char *)&near_field.r_max, cnt, READ );

	cnt = sizeof( char );
	Mem_Copy( buff, &near_field.newer, cnt, READ );
	Mem_Copy( buff, &near_field.valid, cnt, READ );

  } /*if( isFlagSet(DRAW_EHFIELD) ) */

  free_ptr( (void **)&buff );

} /* Get_Freq_Data() */

/*------------------------------------------------------------------------*/

