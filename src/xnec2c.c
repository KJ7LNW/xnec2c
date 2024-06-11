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

#include <pthread.h>

#include "xnec2c.h"
#include "shared.h"
#include "mathlib.h"

static pthread_t *pth_freq_loop = NULL;

/* Left-overs from fortran code :-( */
static double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;

/*-----------------------------------------------------------------------*/

/* Set the calc_data.freq_step if it matches calc_data.freq_mhz.
 * Redo radiation pattern for a new frequency.
 *
 * If it doesn't, return 0 so the caller can run New_Frequency() and
 * use the extra buffer (in rad_pattern and other structures). */
int set_freq_step(void)
{
	int fr, step;
	double freq;

	int prev_freq_step = calc_data.freq_step;

	int idx = 0;
	int found = 0;
	for (fr = 0; !found && fr < calc_data.FR_cards && save.fstep[idx]; fr++)
	{
		freq = calc_data.freq_loop_data[fr].min_freq;
		for (step = 0; !found && step < calc_data.freq_loop_data[fr].freq_steps && save.fstep[idx]; step++)
		{
			// if calc_data.freq_mhz =~ freq, +/- 1 Hz for rounding error:
			if (calc_data.freq_mhz > freq - 1e-6 && calc_data.freq_mhz < freq + 1e-6)
			{
				calc_data.freq_step = idx;
				found = 1;
			}
			else
			{
				if (calc_data.freq_loop_data[fr].ifreq == 1)
					freq *= calc_data.freq_loop_data[fr].delta_freq;
				else
					freq += calc_data.freq_loop_data[fr].delta_freq;

				idx++;
			}

		}
	}

	// If we didn't find the frequency, then use the "extra" frequency
	// allocated as +1 at the end of all per-frequency data indexes:
	if (!found)
		calc_data.freq_step = calc_data.steps_total;

	if (calc_data.freq_step != prev_freq_step)
		SetFlag( DRAW_NEW_RDPAT );

	// If we found the index, then no need to re-run New_Frequency because it is
	// in the index.
	return found;
}
/* Frequency_Scale_Geometry()
 *
 * Scales geometric parameters to frequency
 */
  static void
Frequency_Scale_Geometry(void)
{
  double fr;
  int idx;

  /* Calculate wavelength */
  data.wlam= CVEL / calc_data.freq_mhz;

  /* frequency scaling of geometric parameters */
  fr = calc_data.freq_mhz / CVEL;
  if( data.n != 0)
  {
    for( idx = 0; idx < data.n; idx++ )
    {
      data.x[idx] = save.xtemp[idx] * fr;
      data.y[idx] = save.ytemp[idx] * fr;
      data.z[idx] = save.ztemp[idx] * fr;
      data.si[idx]= save.sitemp[idx]* fr;
      data.bi[idx]= save.bitemp[idx]* fr;
    }
  }

  if( data.m != 0)
  {
    double fr2= fr* fr;
    for( idx = 0; idx < data.m; idx++ )
    {
      int j;

      j = idx + data.n;
      data.px[idx] = save.xtemp[j] * fr;
      data.py[idx] = save.ytemp[j] * fr;
      data.pz[idx] = save.ztemp[j] * fr;
      data.pbi[idx]= save.bitemp[j]* fr2;
    }
  }

} /* Frequency_Scale_Geometry() */

/*-----------------------------------------------------------------------*/

/* Struct_Impedance_Loading()
 *
 * Calculates structure (segment) impedance loading
 */
  static void
Structure_Impedance_Loading( void )
{
  /* Calculate some loading parameters */
  if( zload.nload != 0)
    load(
        calc_data.ldtyp,  calc_data.ldtag,
        calc_data.ldtagf, calc_data.ldtagt,
        calc_data.zlr,    calc_data.zli,
        calc_data.zlc );

} /* Struct_Impedance_Loading() */

/*-----------------------------------------------------------------------*/

/* Ground_Parameters()
 *
 * Calculates ground parameters (antenna environment)
 */
  static void
Ground_Parameters( void )
{
  complex double epsc;

  if( gnd.ksymp != 1)
  {
    gnd.frati = CPLX_10;

    if( gnd.iperf != 1)
    {
      if( save.sig < 0.0 )
        save.sig = -save.sig / (59.96 * data.wlam);

      epsc = cmplx( save.epsr, -save.sig * data.wlam * 59.96 );
      gnd.zrati = 1.0 / csqrt( epsc);
      gwav.u = gnd.zrati;
      gwav.u2 = gwav.u * gwav.u;

      if( gnd.nradl > 0 )
      {
        gnd.scrwl = save.scrwlt / data.wlam;
        gnd.scrwr = save.scrwrt / data.wlam;
        gnd.t1 = CPLX_01 * 2367.067/ (double)gnd.nradl;
        gnd.t2 = gnd.scrwr * (double)gnd.nradl;
      } /* if( gnd.nradl > 0 ) */

      if( gnd.iperf == 2)
      {
        somnec( save.epsr, save.sig, calc_data.freq_mhz );
        gnd.frati =( epsc - 1.0) / ( epsc + 1.0);
        if( cabs(( ggrid.epscf - epsc) / epsc) >= 1.0e-3 )
        {
          pr_err("complex dielectric constant from file: %12.5E%+12.5Ej, requested: %12.5E%+12.5Ej\n",
                 creal(ggrid.epscf), cimag(ggrid.epscf),
				 creal(epsc), cimag(epsc));
          Stop( _("Ground_Parameters():"
                "Error in ground parameters"), ERR_STOP );
        }
      } /* if( gnd.iperf != 2) */
    } /* if( gnd.iperf != 1) */
    else
    {
      gnd.scrwl = 0.0;
      gnd.scrwr = 0.0;
      gnd.t1 = 0.0;
      gnd.t2 = 0.0;
    }
  } /* if( gnd.ksymp != 1) */

  return;
} /* Ground_Parameters() */

/*-----------------------------------------------------------------------*/

/* Set_Interaction_Matrix()
 *
 * Sets and factors the interaction matrix
 */
  static void
Set_Interaction_Matrix( void )
{
  /* Memory allocation for symmetry array */
  smat.nop = netcx.neq/netcx.npeq;
  size_t mreq = (size_t)(smat.nop * smat.nop) * sizeof( complex double);
  mem_realloc( (void **)&smat.ssx, mreq, "in xnec2c.c" );

  /* irngf is not used (NGF function not implemented) */
  int iresrv = data.np2m * (data.np + 2 * data.mp);
  if( matpar.imat == 0)
    fblock( netcx.npeq, netcx.neq, iresrv, data.ipsym);

  cmset( netcx.neq, cm, calc_data.rkh, calc_data.iexk );
  factrs( netcx.npeq, netcx.neq, cm, save.ip );
  netcx.ntsol = 0;

} /* Set_Interaction_Matrix() */

/*-----------------------------------------------------------------------*/

/* Set_Excitation()
 *
 * Sets the excitation part of the matrix
 */
  static void
Set_Excitation( void )
{
  if( (fpat.ixtyp >= 1) && (fpat.ixtyp <= 4) )
  {
    tmp4= TORAD* calc_data.xpr4;
    tmp5= TORAD* calc_data.xpr5;

    if( fpat.ixtyp == 4)
    {
      tmp1= calc_data.xpr1/ data.wlam;
      tmp2= calc_data.xpr2/ data.wlam;
      tmp3= calc_data.xpr3/ data.wlam;
      tmp6= calc_data.xpr6/( data.wlam* data.wlam);
    }
    else
    {
      tmp1= TORAD* calc_data.xpr1;
      tmp2= TORAD* calc_data.xpr2;
      tmp3= TORAD* calc_data.xpr3;
      tmp6= calc_data.xpr6;
    } /* if( fpat.ixtyp == 4) */

  } /* if( (fpat.ixtyp >= 1) && (fpat.ixtyp <= 4) ) */

  /* fills e field right-hand matrix */
  etmns( tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, fpat.ixtyp, crnt.cur );

} /* Set_Excitation() */

/*-----------------------------------------------------------------------*/

/* Set_Network_Data()
 *
 * Sets up network data and solves for currents
 */
  static void
Set_Network_Data( void )
{
  if( netcx.nonet != 0 )
  {
    int i, j, itmp1, itmp2, itmp3;

    itmp3=0;
    itmp1= netcx.ntyp[0];
    for( i = 0; i < 2; i++ )
    {
      if( itmp1 == 3) itmp1=2;

      for( j = 0; j < netcx.nonet; j++)
      {
        itmp2= netcx.ntyp[j];

        if( (itmp2/itmp1) != 1 ) itmp3 = itmp2;
        else if( (itmp2 >= 2) && (netcx.x11i[j] <= 0.0) )
        {
          double xx, yy, zz;
          int idx4, idx5;

          idx4 = netcx.iseg1[j]-1;
          idx5 = netcx.iseg2[j]-1;
          xx = data.x[idx5]- data.x[idx4];
          yy = data.y[idx5]- data.y[idx4];
          zz = data.z[idx5]- data.z[idx4];
          netcx.x11i[j] = data.wlam* sqrt( xx*xx + yy*yy + zz*zz );
        }

      } /* for( j = 0; j < netcx.nonet; j++) */

      if( itmp3 == 0) break;

      itmp1= itmp3;

    } /* for( i = 0; i < 2; i++ ) */

  } /* if( netcx.nonet != 0 ) */

  /* Set network data */
  netwk( cm, save.ip, crnt.cur );
  netcx.ntsol = 1;

  /* Save impedance data for normalization */
  int fstep = calc_data.freq_step;
  if (fstep < 0 || fstep > calc_data.steps_total)
	return;

  if( ((calc_data.steps_total > 1) &&
        isFlagSet(FREQ_LOOP_RUNNING)) ||
		CHILD ||
		fstep == calc_data.steps_total)
  {

    impedance_data.zreal[fstep] = (double)creal( netcx.zped);
    impedance_data.zimag[fstep] = (double)cimag( netcx.zped);
    impedance_data.zmagn[fstep] = (double)cabs( netcx.zped);
    impedance_data.zphase[fstep]= (double)cang( netcx.zped);

    if( (calc_data.iped == 1) &&
        ((double)impedance_data.zmagn[fstep] > calc_data.zpnorm) )
      calc_data.zpnorm = (double)impedance_data.zmagn[fstep];
  }

} /* Set_Network_Data() */

/*-----------------------------------------------------------------------*/

/* Power_Loss()
 *
 * Calculate power loss due to segment loading
 */
  static void
Power_Loss( void )
{
  int i;
  double cmg;
  complex double curi;


  /* No wire/segments in structure */
  if( data.n == 0) return;

  fpat.ploss = 0.0;
  /* Loop over all wire segs */
  for( i = 0; i < data.n; i++ )
  {
    /* Calculate segment current (mag/phase) */
    curi= crnt.cur[i]* data.wlam;
    cmg= cabs( curi);

    /* Calculate power loss in segment */
    if( (zload.nload != 0) &&
        (fabs(creal(zload.zarray[i])) >= 1.0e-20) )
      fpat.ploss += 0.5* cmg* cmg* creal( zload.zarray[i])* data.si[i];

  } /* for( i = 0; i < n; i++ ) */

} /* Power_Loss() */

/*-----------------------------------------------------------------------*/

/* Radiation_Pattern()
 *
 * Calculates far field (radiation) pattern
 */
  static void
Radiation_Pattern( void )
{
  if( (gnd.ifar != 1) && isFlagSet(ENABLE_RDPAT) )
  {
    fpat.pinr= netcx.pin;
    fpat.pnlr= netcx.pnls;
    rdpat();
  }

} /* Radiation_Pattern() */

/*-----------------------------------------------------------------------*/

/* Near_Field_Pattern()
 *
 * Calculates near field pattern if enabled/needed
 */
  void
Near_Field_Pattern( void )
{
  if( near_field.valid ||
      isFlagClear(DRAW_EHFIELD) ||
      isFlagClear(ENABLE_NEAREH) )
    return;

  if( fpat.nfeh & NEAR_EFIELD )
    nfpat(0);

  if( fpat.nfeh & NEAR_HFIELD )
    nfpat(1);

} /* Near_Field_Pattern() */

/*-----------------------------------------------------------------------*/

/* New_Frequency_Reset_Prev()
 *
 * Resets the previous frequency state to force New_Frequency() to recalculate if the
 * same frequency is called.
 *
 * save.last_freq variable stores the previous MHz value that was used when
 * calling New_Frequency() so it can exit early if that frequency
 * has already been calculated.  Reset_Prev_New_Frequency() needs
 * to be called to reset this when a file is opened or when a benchmark
 * is being run.
 */
void New_Frequency_Reset_Prev(void)
{
	save.last_freq = 0;
}

/* New_Frequency()
 *
 * (Re)calculates all frequency-dependent parameters
 */
  void
New_Frequency( void )
{
  struct timespec start, end;
  double elapsed;

  /* Abort if freq has not really changed, as when changing
   * between current or charge density structure coloring */
  if( (save.last_freq == calc_data.freq_mhz) ||
      isFlagClear(ENABLE_EXCITN) )
    return;

  g_mutex_lock(&freq_data_lock);

  save.last_freq = calc_data.freq_mhz;

  // Only show this if you manually change frequencies:
  clock_gettime(CLOCK_MONOTONIC, &start);

  /* Frequency scaling of geometric parameters */
  Frequency_Scale_Geometry();

  /* Structure segment loading */
  Structure_Impedance_Loading();

  /* Calculate ground parameters */
  Ground_Parameters();

  /* Fill and factor primary interaction matrix */
  Set_Interaction_Matrix();

  /* Fill excitation part of matrix */
  Set_Excitation();

  /* Matrix solving (netwk calls solves) */
  crnt.valid = 0;
  Set_Network_Data();

  /* Calculate power loss */
  Power_Loss();

  /* Calculate radiation pattern */
  Radiation_Pattern();

  /* Near field calculation */
  near_field.valid = 0;
  Near_Field_Pattern();

  g_mutex_unlock(&freq_data_lock);

  // Calculate elapsed time
  clock_gettime(CLOCK_MONOTONIC, &end);
  
  elapsed = (end.tv_sec + (double)end.tv_nsec/1e9) - (start.tv_sec + (double)start.tv_nsec/1e9);
  pr_info("%.6f MHz: %f seconds. (%s)\n",
			calc_data.freq_mhz, elapsed, current_mathlib->name);

} /* New_Frequency()  */

/*-----------------------------------------------------------------------*/

static gboolean retval; /* Function's return value */

int update_freqplots_fmhz_entry(gpointer p)
{
    /* Display current frequency in plots entry */
    char txt[16];
    snprintf( txt, sizeof(txt)-1, "%.3f", calc_data.freq_mhz );
    gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
            freqplots_window_builder, "freqplots_fmhz_entry")), txt );

	return FALSE;
}

// Set the specified widget to the value of calc_data.freq_mhz
int update_freq_mhz_spin_button_value(GtkSpinButton *w)
{
	gtk_spin_button_set_value(w, (gdouble)calc_data.freq_mhz );

	return FALSE;
}

int update_fmhz_save_spin_button_value(GtkSpinButton *w)
{
	gtk_spin_button_set_value(w, (gdouble)calc_data.fmhz_save );

	return FALSE;
}

/* Frequency_Loop()
 *
 * Loops over frequency if calculations over a frequency range is
 * requested, dividing the job between child processes if forked
 *
 * This function does the following:

	0. If FREQ_LOOP_INIT, set initial state
	1. For each job:
		a. Set "freq" to the right value from FR_cards for the next job to assign
		b. If FREQ_LOOP_STOP, then cleanup and disable loop
		c. Increment frequency, set save.freq[fstep] = (double)freq;
		d. Find an available child and give it the fstep
	5. Wait until a child has frequency data ready.
		- For each available child:
			a. Load the data via Get_Freq_Data
			     into the fstep'th index that was handed to that particular child
			b. Set save.fstep[child_fstep]=1 to indicate the data is ready
			c. Clear the child's busy flag
	6. Set calc_data.freq_step to the highest available step with data. 
		This is an index value, not a count.
	7. Trigger a redraw if still have busy children
	8. If no more busy children:
		Reset the saved frequency (green line) to what it was.
	9. Save optimizer output if enabled
 */

gboolean Frequency_Loop( gpointer udata )
{
  /* Value of frequency and step num in the loop */
  static double freq;

  static int
    fstep,           /* Current frequency step */
    fsteps_total,    /* Total number of frequency steps processed */
    num_busy_procs;  /* Number of busy child processes */

  int idx, job_num = 0;
  size_t len;
  char *buff;      /* Used to pass on structure poiners */
  fd_set read_fds; /* Read file descriptors for select() */

  // Total freqloop time:
  static struct timespec start, end;


  /* (Re) Initialize freq loop */
  if( isFlagSet(FREQ_LOOP_INIT) )
  {
    g_mutex_lock(&freq_data_lock);

    /* Clear global flags */
    ClearFlag( FREQ_LOOP_INIT | FREQ_LOOP_DONE );

    /* (Re)-enable freq loop (back to start freq) */
    freq = calc_data.freq_loop_data[0].min_freq;

    /* Step back frequency and step count since incrementing
     * is done at start of frequency loop calculations. This
     * is done using data from the first (index 0) FR card */
    fstep = -1;
    if( calc_data.freq_loop_data[0].ifreq == 1)
      freq /= calc_data.freq_loop_data[0].delta_freq;
    else
      freq -= calc_data.freq_loop_data[0].delta_freq;

    /* Clear list of "valid" (processed) loop steps */
    for( idx = 0; idx < calc_data.steps_total; idx++ )
      save.fstep[idx] = 0;

    /* Clear the index to current FR card and steps total */
    calc_data.FR_index = 0;

    /* Initialize frequency steps totalizer to first FR card steps */
    fsteps_total = calc_data.freq_loop_data[0].freq_steps;

    /* Clear "last-used-frequency" buffer */
    New_Frequency_Reset_Prev();

    /* Zero num of busy processes */
    num_busy_procs = 0;

    /* Signal global freq step "illegal" FIXME */
    calc_data.freq_step = -1;

    /* Inherited from NEC2 */
    if( calc_data.zpnorm > 0.0 ) calc_data.iped = 2;

	// Start the timer:
	clock_gettime(CLOCK_MONOTONIC, &start);

    g_mutex_unlock(&freq_data_lock);

    /* Continue iterating this function.  (Returning FALSE would discontinue the frequency loop.) */
    retval = TRUE;
    return ( retval );

  } /* isFlagSet(INIT_FREQ_LOOP) */
  ClearFlag( FREQ_LOOP_INIT );

  // Prevent the optimizer from running this function in parallel:
  g_mutex_lock(&global_lock);
  
  /* Repeat freq stepping over number of child processes
   * if forked. calc_data.num_jobs = 1 for non-forked runs.
   * If not forked (no multi-threading), following block will
   * execute only once, since only one instance is running */
  for( idx = 0; idx < calc_data.num_jobs; idx++ )
  {
    /* Up frequency step count. Note that this is initialized at -1 above so it
	 * really does start the first loop with fstep==0 */
    fstep++;

    /* Set `freq` for use below
     * If all steps of current FR card are processed, go to the next */
    if( fstep >= fsteps_total )
    {
      calc_data.FR_index++;
      if( calc_data.FR_index < calc_data.FR_cards )
      {
        /* Add steps of new FR card range to total */
        fsteps_total += calc_data.freq_loop_data[calc_data.FR_index].freq_steps;

        /* Update loop frequency from new FR card
         * FIXME:  We shrink it here just so we cang row it down
         * below where it is commented "Increment frequency" */
        freq = calc_data.freq_loop_data[calc_data.FR_index].min_freq;
        if( calc_data.freq_loop_data[calc_data.FR_index].ifreq == 1)
          freq /= calc_data.freq_loop_data[calc_data.FR_index].delta_freq;
        else
          freq -= calc_data.freq_loop_data[calc_data.FR_index].delta_freq;
      }
      else
      {
        calc_data.FR_index--;   /* keep it in range, avoid off-by-1 error */
      }
    }

    /* Frequency loop was paused by user */
    if( isFlagSet(FREQ_LOOP_STOP) )
    {
      /* Points to last buffer in rad_pattern filled by loop */
      fstep = calc_data.freq_step;

      /* Re-enable pausing of freq loop */
      ClearFlag( FREQ_LOOP_STOP );

      /* Cancel idle callbacks on exit */
      retval = FALSE;

      break;
    } /* if( isFlagSet(FREQ_LOOP_STOP) ) */

    /* Increment frequency:
	 * ifreq is "IFRQ (I1) from the FR card specification:
	 *    0 - linear stepping
	 *    1 - multiplicative stepping */
    if( calc_data.freq_loop_data[calc_data.FR_index].ifreq == 1)
      freq *= calc_data.freq_loop_data[calc_data.FR_index].delta_freq;
    else
      freq += calc_data.freq_loop_data[calc_data.FR_index].delta_freq;

    /* Save frequencies for plotting */
    if (fstep < calc_data.steps_total)
		save.freq[fstep] = (double)freq;

    /* Delegate calculations to child processes if forked */
    if( FORKED && fstep < calc_data.steps_total )
    {
      /* Look for an idle process */
      for( job_num = 0; job_num < calc_data.num_jobs; job_num++ )
      {
        /* If an idle process is found, give it a job and
         * then step the frequency loop by breaking out */
        if( ! forked_proc_data[job_num]->busy )
        {
          /* Signal and count busy processes */
          forked_proc_data[job_num]->busy  = TRUE;
          forked_proc_data[job_num]->fstep = fstep;
          num_busy_procs++;

          // Send the mathlib to use, try to lock it if it is Intel MKL.
		  mathlib_lock_intel_batch(rc_config.mathlib_batch_idx);
          Write_Pipe( idx, fork_commands[MATHLIB], (ssize_t)strlen(fork_commands[MATHLIB]), TRUE );
          Write_Pipe( idx, (char*)&rc_config.mathlib_batch_idx,
			  (ssize_t)sizeof(rc_config.mathlib_batch_idx), TRUE );

          /* Tell process to calculate freq dependent data */
          len = strlen( fork_commands[FRQDATA] );
          Write_Pipe( job_num, fork_commands[FRQDATA], (ssize_t)len, TRUE );

          /* Give it the next frequency */
          buff = (char *)&freq;
          len = sizeof( double );
          Write_Pipe( job_num, buff, (ssize_t)len, TRUE );
          break;
        }
      } /* for( job_num = 0; job_num < calc_data.num_jobs; job_num++ ) */

    } /* if( FORKED ) */
    else if ( fstep < calc_data.steps_total ) /* Calculate freq dependent data (no fork) */
    {
      g_mutex_lock(&freq_data_lock);
      calc_data.freq_mhz  = freq;
      calc_data.freq_step = fstep;
      calc_data.last_step = fstep;
      g_mutex_unlock(&freq_data_lock);

      New_Frequency();

      // Be sure to exit if this was the last iteration:
      if (fstep >= calc_data.steps_total-1)
             retval = 0;

      break;
    }

    /* All idle processes are given a job */
    if( num_busy_procs >= calc_data.num_jobs )
      break;

  } /* for( idx = 0; idx < calc_data.num_jobs; idx++ ) */

  /* Receive results from forked children */
  if( FORKED && num_busy_procs )
    do
    {
      int n = 0;

      /* Set read fd's to watch for child writes */
      FD_ZERO( &read_fds );
      for( idx = 0; idx < calc_data.num_jobs; idx++ )
      {
        FD_SET( forked_proc_data[idx]->child2pnt_pipe[READ], &read_fds );
        if( n < forked_proc_data[idx]->child2pnt_pipe[READ] )
          n = forked_proc_data[idx]->child2pnt_pipe[READ];
      }

      /* Wait for data from finished child processes */
      if( select( n+1, &read_fds, NULL, NULL, NULL ) == -1 )
      {
        if (errno == EINTR)
			continue;
        perror( "select()" );
        _exit(0);
      }

      /* Check for finished child processes */
      g_mutex_lock(&freq_data_lock);

      for( idx = 0; idx < num_child_procs; idx++ )
      {
        if( FD_ISSET(forked_proc_data[idx]->child2pnt_pipe[READ], &read_fds) )
        {
          /* Read data from finished child process */
          if (!Get_Freq_Data( idx, forked_proc_data[idx]->fstep ))
          {
            pr_err("Failed to read data from forked child\n");
            SetFlag(FREQ_LOOP_STOP);
            g_mutex_unlock(&freq_data_lock);
            g_mutex_unlock(&global_lock);
            return FALSE;
          }

          /* Clear "last-used-frequency" buffer, the local version of the data is
           * no longer what New_Frequency() set it to: */
          New_Frequency_Reset_Prev(); 

          /* Mark freq step in list of processed steps */
          save.fstep[forked_proc_data[idx]->fstep] = 1;

          /* Mark finished child process as ready for next job */
          forked_proc_data[idx]->busy = FALSE;

          /* Count down number of busy processes */
          num_busy_procs--;
        }
      } /* for( idx = 0; idx < num_child_procs; idx++ ) */

      /* Find highest freq step that has no steps below it
       * that have not been processed by a child process */
      for( idx = 0; idx < calc_data.steps_total; idx++ )
      {
        if( save.fstep[idx] ) calc_data.freq_step = idx;
        else break;
      }

      if (calc_data.freq_step >= calc_data.steps_total-1)
	  {
		  /* Cancel idle callbacks on exit */
		  retval = FALSE;
		  if (num_busy_procs != 0)
		  {
			  pr_debug("Frequency_Loop is done with all steps but num_busy_procs=%d (calc_data.freq_step=%d calc_data.steps_total=%d)?\n",
                              num_busy_procs, calc_data.freq_step,
                              calc_data.steps_total);
				for( job_num = 0; job_num < calc_data.num_jobs; job_num++ )
				{
					if (forked_proc_data[job_num]->busy)
					{
						pr_debug("  busy job[%d]: fstep=%d freq=%f\n",
							job_num,
							forked_proc_data[job_num]->fstep,
							save.freq[forked_proc_data[job_num]->fstep]);
					}
				}
		  }
	  }

      g_mutex_unlock(&freq_data_lock);

    } /* do. Loop terminated and busy children */
    while( !retval && num_busy_procs );

  /* Return if freq step 0 not ready yet, no frequency data is populated yet. */
  if( calc_data.freq_step < 0 )
  {
	  g_mutex_unlock(&global_lock);
      return( retval );
  }

  /* Set frequency and step to global variables
   * FIXME: These should move within the &freq_data_lock section just above ^^ */
  g_mutex_lock(&freq_data_lock);
  calc_data.last_step = calc_data.freq_step;
  calc_data.freq_mhz = (double)save.freq[calc_data.freq_step];
  g_mutex_unlock(&freq_data_lock);

  SetFlag( FREQ_LOOP_READY );

  if (retval || num_busy_procs)
  {
	/* Trigger a redraw of open drawingareas */
	/* Plot frequency-dependent data */
	if( isFlagSet(PLOT_ENABLED) )
	{
	  /* Display current frequency in plots entry */
	  g_idle_add_once((GSourceOnceFunc)update_freqplots_fmhz_entry, NULL);

	  if( isFlagClear(OPTIMIZER_OUTPUT) || freqplots_click_pending())
	  {
		xnec2_widget_queue_draw( freqplots_drawingarea );
	  }
	}

	/* Set main window frequency spinbutton */
	g_idle_add_once((GSourceOnceFunc)update_freq_mhz_spin_button_value, mainwin_frequency);

	/* Set Radiation pattern window frequency spinbutton */
	if( isFlagSet(DRAW_ENABLED) )
		  g_idle_add_once((GSourceOnceFunc)update_freq_mhz_spin_button_value, rdpattern_frequency);

	xnec2_widget_queue_draw( structure_drawingarea );
  }

  /* Change flags at exit if loop is done */
  if( !retval && !num_busy_procs )
  {
    ClearFlag( FREQ_LOOP_RUNNING );
    SetFlag( FREQ_LOOP_DONE );

	clock_gettime(CLOCK_MONOTONIC, &end);
	pr_notice("Frequency loop elapsed time: %f seconds. (%s)\n",
                (end.tv_sec + (double)end.tv_nsec / 1e9) - (start.tv_sec + (double)start.tv_nsec / 1e9),
				(FORKED ? get_mathlib_by_idx(rc_config.mathlib_batch_idx)->name : current_mathlib->name));

    /* After the loop is finished, re-set the saved frequency
     * that the user clicked on in the frequency plots window */
    double max_freq = calc_data.freq_loop_data[calc_data.FR_cards-1].max_freq;
    double min_freq = calc_data.freq_loop_data[0].min_freq;
    if( (int)calc_data.fmhz_save &&
		calc_data.fmhz_save >= min_freq &&
		calc_data.fmhz_save <= max_freq)
    {
	  // There are multiple changes here that will trigger New_Frequency() but 
	  // the New_Frequency() function is smart enough to calculate only once.
      calc_data.freq_mhz = calc_data.fmhz_save;
      
	  // Call this from the Frequency_Loop thread to keep it from happening in the GTK thread:
	  //  -- Actually this causes the rdpattern to draw the wrong frequency when the optimizer
	  //  is turned on and there is a fmhz_save frequency selected.  It draws the first freq
	  //  instead of the selected freq.  Why?
	  //New_Frequency();

      /* Set main window frequency spinbutton.
	   * This will trigger New_Frequency() via Redo_Currents(). */
      g_idle_add_once_sync((GSourceOnceFunc)update_freq_mhz_spin_button_value, mainwin_frequency);

      /* Set Radiation pattern window frequency spinbutton.
	   * This will trigger New_Frequency() via Redo_Radiation_Pattern,. */
      if( isFlagSet(DRAW_ENABLED) )
        g_idle_add_once_sync((GSourceOnceFunc)update_fmhz_save_spin_button_value, rdpattern_frequency);

      if( isFlagSet(PLOT_ENABLED) )
      {
        SetFlag( PLOT_FREQ_LINE );
        g_idle_add_once_sync((GSourceOnceFunc)update_freqplots_fmhz_entry, NULL);
      }
    }

    /* Re-draw drawing areas at end of loop */
    if( isFlagSet(PLOT_ENABLED) )
      xnec2_widget_queue_draw( freqplots_drawingarea );
    if( isFlagSet(DRAW_ENABLED) )
      xnec2_widget_queue_draw( rdpattern_drawingarea );

    /* Write out frequency loop data for the optimizer if optimization is
     * active.  This is a sync call, so only call this if we have files flagged
     * for writing (opt_have_files_to_save) based on the checkboxes in
     * File->Optimization Settings */
    if( (rc_config.batch_mode || isFlagSet(OPTIMIZER_OUTPUT)) && opt_have_files_to_save() )
    {
      g_idle_add_once_sync((GSourceOnceFunc)Write_Optimizer_Data, NULL);
    }
  } // if( !retval && !num_busy_procs )

  g_mutex_unlock(&global_lock);

  return( retval );
} /* Frequency_Loop() */

/*-----------------------------------------------------------------------*/


void *Frequency_Loop_Thread(void *p)
{
	// Don't draw the green line if in batch mode
	if (rc_config.batch_mode)
		calc_data.fmhz_save = 0.0;

	// Run the loop
	while (isFlagSet(FREQ_LOOP_RUNNING) && Frequency_Loop(NULL));

	ClearFlag(FREQ_LOOP_RUNNING);

	if (isFlagSet(FREQ_LOOP_STOP))
		return NULL;

	// Exit if in batch mode
	if (rc_config.batch_mode)
	{
		g_idle_add_once_sync((GSourceOnceFunc)Gtk_Quit, NULL);
		return NULL;
	}

	SetFlag(DRAW_NEW_RDPAT);

	/*
		Prevent deadlock waiting for Stop_Frequency_Loop()=>pthread_join()
		in Open_Input_File() triggered by Optimizer_Output() because
		g_idle_add_once_sync won't allow this Frequency_Loop_Thread()
		thread to exit until Open_Input_File() returns for GTK to make
		progress, but Open_Input_File() is waiting for pthread_join()
		to return when this thread exits.
	*/
	if ( isFlagSet(INPUT_PENDING) )
		return NULL;

	/*
	   Re-draw drawing areas at end of loop 
	 */
	if (isFlagSet(PLOT_ENABLED))
		g_idle_add_once_sync((GSourceOnceFunc) gtk_widget_queue_draw, freqplots_drawingarea);

	if (isFlagSet(DRAW_ENABLED))
	{
		g_idle_add_once_sync((GSourceOnceFunc)update_freq_mhz_spin_button_value, rdpattern_frequency);
		g_idle_add_once_sync((GSourceOnceFunc)update_freq_mhz_spin_button_value, mainwin_frequency);
		g_idle_add_once_sync((GSourceOnceFunc)Redo_Currents, NULL);

		need_rdpat_redraw = 1;
		need_structure_redraw = 1;
		g_idle_add_once_sync((GSourceOnceFunc)gtk_widget_queue_draw, structure_drawingarea);
		g_idle_add_once_sync((GSourceOnceFunc)gtk_widget_queue_draw, rdpattern_drawingarea);
	}

	return NULL;
}


/* Start_Frequency_Loop()
 *
 * Starts frequency loop
 */
  gboolean
Start_Frequency_Loop( void )
{
  if( calc_data.freq_loop_data == NULL )
    return( FALSE );

  if( isFlagClear(FREQ_LOOP_RUNNING) &&
      (calc_data.FR_cards > 0 )      &&
      (calc_data.steps_total > 1) )
  {
    retval = TRUE;
    SetFlag( FREQ_LOOP_INIT );
    SetFlag( FREQ_LOOP_RUNNING );

	if (!rc_config.disable_pthread_freqloop)
	{
		mem_alloc((void**)&pth_freq_loop, sizeof(pthread_t), __LOCATION__);
		int ret = pthread_create( pth_freq_loop, NULL, Frequency_Loop_Thread, NULL );
		if( ret != 0 )
		{
			free_ptr((void**)&pth_freq_loop);
			pr_crit("failed to start Frequency_Loop_Thread\n");
			perror( "pthread_create()" );
			exit( -1 );
		}
	}
	else
	{
		floop_tag = g_idle_add( Frequency_Loop, NULL );
	}
    return( TRUE );
  }
  else return( FALSE );

} /* Start_Frequency_Loop() */

/*-----------------------------------------------------------------------*/

/* Stop_Frequency_Loop()
 *
 * Stops and resets freq loop
 */
  void
Stop_Frequency_Loop( void )
{
  // Clearing this flag will cause the Frequency_Loop pthread to exit when it is done:
  ClearFlag( FREQ_LOOP_RUNNING );
  SetFlag(FREQ_LOOP_STOP);

  if (!rc_config.disable_pthread_freqloop)
  {
	  // Wait for the thread to exit:
	  if (pth_freq_loop != NULL)
	  {
		  pthread_join(*pth_freq_loop, NULL);
		  free_ptr((void**)&pth_freq_loop);
	  }

	  ClearFlag(FREQ_LOOP_STOP);

	  // Flush any pending GTK events. This is critical because any pending
	  // events that may work upon GtkWidget's that change (or close) upon exit
	  // from this function will fail.
	  while( g_main_context_iteration(NULL, FALSE) ) {}
  }
  else if( floop_tag > 0 )
  {
	g_source_remove( floop_tag );
	floop_tag = 0;
  }
} /* Stop_Frequency_Loop() */

/*-----------------------------------------------------------------------*/

/* Incident_Field_Loop()
 *
 * Loops over incident field directions if
 * receiving pattern calculations are requested
 */
  void
Incident_Field_Loop( void )
{
  int phi_step, theta_step;

  /* Frequency scaling of geometric parameters */
  Frequency_Scale_Geometry();

  /* Structure segment loading */
  Structure_Impedance_Loading();

  /* Calculate ground parameters */
  Ground_Parameters();

  /* Fill and factor primary interaction matrix */
  Set_Interaction_Matrix();

  /* Loop over incident field angles */
  netcx.nprint=0;
  /* Loop over phi */
  for( phi_step = 0; phi_step < calc_data.nphi; phi_step++ )
  {
    /* Loop over theta */
    for( theta_step = 0; theta_step < calc_data.nthi; theta_step++ )
    {
      /* Fill excitation part of matrix */
      Set_Excitation();

      /* Matrix solving (netwk calls solves) */
      Set_Network_Data();

      /* Calculate power loss */
      Power_Loss();

      calc_data.xpr1 += calc_data.xpr4;

    } /* for( theta_step = 0; theta_step < calc_data.nthi.. */

    calc_data.xpr1= calc_data.thetis;
    calc_data.xpr2= calc_data.xpr2+ calc_data.xpr5;

  } /* for( phi_step = 0; phi_step < calc_data.nphi.. */

  calc_data.xpr2  = calc_data.phiss;

} /* Incident_Field_Loop() */

/*-----------------------------------------------------------------------*/

