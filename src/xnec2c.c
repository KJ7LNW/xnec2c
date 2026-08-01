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
#include "callbacks.h"
#include "shared.h"
#include "render/render_engine.h"
#include "measurements.h"
#include "prerender/prerender_color.h"
#include "prerender/prerender_farfield.h"
#include "validation_dump.h"
#include "mathlib.h"
#include "opt_ui.h"
#include "plot_freqdata.h"
#include "rdpattern_ui.h"
#include "structure_ui.h"

#define BATCH_RDPAT_DEFAULT_PX 800

typedef struct {
  int width;
  int height;
} batch_capture_size_t;

typedef struct {
  const char *button_id;
  rdpat_png_format_t format;
  int column;
  int row;
} batch_rdpattern_pane_t;

static const batch_rdpattern_pane_t batch_rdpattern_panes[] = {
  { .button_id = "rdpattern_x_axis", .format = RDPAT_PNG_FORMAT_X,
    .column = 0, .row = 0 },
  { .button_id = "rdpattern_y_axis", .format = RDPAT_PNG_FORMAT_Y,
    .column = 1, .row = 0 },
  { .button_id = "rdpattern_z_axis", .format = RDPAT_PNG_FORMAT_Z,
    .column = 0, .row = 1 },
  { .button_id = "rdpattern_default_view", .format = RDPAT_PNG_FORMAT_ISO,
    .column = 1, .row = 1 },
};

_Static_assert(G_N_ELEMENTS(batch_rdpattern_panes) + 1 == RDPAT_PNG_FORMAT_COUNT,
    "quad is the only radiation-pattern PNG format without a single pane");

/* Only nec2_eval_signal() is called from xnec2c.c; avoid pulling
 * gsl headers (via opt_simple.h) which conflict with openblas cblas. */
extern void nec2_eval_signal(void);
/* opengl_structure_invalidate() is called here; forward-declared to avoid
 * pulling opengl_structure.h (and its GL/GLEW chain) into the NEC engine file. */
extern void opengl_structure_invalidate(void);

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
			if( FREQ_EQ(calc_data.freq_mhz, freq) )
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
	{
		calc_data.freq_step = calc_data.steps_total;

		/* save.freq[steps_total] is written only by freq_loop_dispatch;
		 * CRNT_FSTEP_AVAILABLE guards consumers until data is valid. */
	}

	if (calc_data.freq_step != prev_freq_step)
		SetFlag( DRAW_NEW_RDPAT );

	// If we found the index, then no need to re-run New_Frequency because it is
	// in the index.
	return found;
}

/* Forward declaration: defined after the freq-loop helpers below */
void freq_step_update_ui( int new_step, gboolean force );

/**
 * fetch_freq_data - retrieve frequency data from cache or dispatch computation
 *
 * Sets calc_data.freq_step to the matching sweep index when freq_mhz matches
 * a cached FR-card step.  If the extra slot already holds valid data for this
 * frequency, returns immediately.  Otherwise starts the frequency loop to
 * compute the extra slot via the child dispatch path.
 *
 * Returns: TRUE when cached data is available and the caller may redraw;
 *          FALSE when computation has been dispatched (redraws follow on
 *          completion via freq_loop_finalize/redraws).
 */
gboolean
fetch_freq_data( void )
{
  /* No data loaded yet; save.freq is unallocated */
  if( save.freq == NULL )
    return FALSE;

  g_rec_mutex_lock(&freq_data_lock);

  if( set_freq_step() )
  {
    freq_step_update_ui( calc_data.freq_step, TRUE );
    g_rec_mutex_unlock(&freq_data_lock);
    return TRUE;
  }

  if( save.fstep[calc_data.steps_total] &&
      FREQ_EQ(save.freq[calc_data.steps_total], calc_data.freq_mhz) )
  {
    freq_step_update_ui( calc_data.steps_total, TRUE );
    g_rec_mutex_unlock(&freq_data_lock);
    return TRUE;
  }

  g_rec_mutex_unlock(&freq_data_lock);
  return FALSE;
}

/**
 * freq_display_update - record user-selected frequency and refresh display
 * @fmhz: frequency in MHz selected by the user
 *
 * Sets fmhz_save and calls opt_ui_update_values.  Does not trigger NEC2
 * computation.  Called by user_set_frequency before dispatching computation,
 * and directly by the spinbutton handler when Apply is unchecked.
 */
void
freq_display_update( double fmhz )
{
  calc_data.fmhz_save = fmhz;
  opt_ui_update_values();
}

/**
 * user_set_frequency - apply a user-selected frequency and trigger computation
 * @fmhz:  frequency in MHz chosen by the user
 *
 * Updates display state via freq_display_update then dispatches NEC2
 * computation.  On a cache hit, freq_step_update_ui refreshes the UI with
 * computed data.  Apply checkbox logic is handled by the caller — this
 * function always computes.
 */
void
user_set_frequency( double fmhz )
{
  freq_display_update( fmhz );
  calc_data.freq_mhz = fmhz;
  if( !fetch_freq_data() )
    Start_Frequency_Loop_Greenline();
}

/* Frequency_Scale_Geometry()
 *
 * Scales geometric parameters to frequency
 */
  void
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
      data.segments[idx].x = save.xtemp[idx] * fr;
      data.segments[idx].y = save.ytemp[idx] * fr;
      data.segments[idx].z = save.ztemp[idx] * fr;
      data.segments[idx].si = save.sitemp[idx]* fr;
      data.segments[idx].bi = save.bitemp[idx]* fr;
    }
  }

  if( data.m != 0)
  {
    double fr2= fr* fr;
    for( idx = 0; idx < data.m; idx++ )
    {
      int j;

      j = idx + data.n;
      data.patches[idx].px = save.xtemp[j] * fr;
      data.patches[idx].py = save.ytemp[j] * fr;
      data.patches[idx].pz = save.ztemp[j] * fr;
      data.patches[idx].pbi = save.bitemp[j]* fr2;
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
          Stop( ERR_STOP, _("Ground_Parameters():"
                "Error in ground parameters") );
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

/* calc_data_free()
 *
 * Releases the per-FR-card frequency-loop descriptor array.
 */
  void
calc_data_free( void )
{
  mem_array_free( &calc_data.freq_loop_data );

} /* calc_data_free() */

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
  mem_array_realloc(&smat.ssx, (smat.nop * smat.nop));

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
          xx = data.segments[idx5].x - data.segments[idx4].x;
          yy = data.segments[idx5].y - data.segments[idx4].y;
          zz = data.segments[idx5].z - data.segments[idx4].z;
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

    int n_ports = Num_Feedpoint_Ports();
    for( int p = 0; p < n_ports; p++ )
    {
      impedance_data[fstep].zreal[p]  = creal( netcx.zped_port[p] );
      impedance_data[fstep].zimag[p]  = cimag( netcx.zped_port[p] );
      impedance_data[fstep].zmagn[p]  = cabs ( netcx.zped_port[p] );
      impedance_data[fstep].zphase[p] = cang ( netcx.zped_port[p] );
    }

    if( (calc_data.iped == 1) && (n_ports > 0) &&
        (impedance_data[fstep].zmagn[calc_data.ex_port] > calc_data.zpnorm) )
      calc_data.zpnorm = impedance_data[fstep].zmagn[calc_data.ex_port];
  }

} /* Set_Network_Data() */

/*-----------------------------------------------------------------------*/

/* free_impedance_step()
 *
 * Releases one frequency step's per-port impedance sub-buffers.
 */
  static void
free_impedance_step( void *elem )
{
  impedance_data_t *ip = elem;
  mem_array_free( &ip->zreal );
  mem_array_free( &ip->zimag );
  mem_array_free( &ip->zmagn );
  mem_array_free( &ip->zphase );
}

/*-----------------------------------------------------------------------*/

/* Alloc_Impedance_Buffers()
 *
 * Sizes the per-fstep impedance array-of-structs, each element carrying one
 * value per feedpoint port.  A zero port count leaves the inner members NULL.
 */
  void
Alloc_Impedance_Buffers( int nfrq, int n_ports )
{
  mem_array_resize( &impedance_data, nfrq, free_impedance_step );

  if( n_ports > 0 )
    for( int idx = 0; idx < nfrq; idx++ )
    {
      mem_array_realloc( &impedance_data[idx].zreal,  n_ports );
      mem_array_realloc( &impedance_data[idx].zimag,  n_ports );
      mem_array_realloc( &impedance_data[idx].zmagn,  n_ports );
      mem_array_realloc( &impedance_data[idx].zphase, n_ports );
    }
}

/*-----------------------------------------------------------------------*/

/* Free_Impedance_Buffers()
 *
 * Releases every fstep's per-port sub-buffers, then the outer array-of-structs,
 * leaving impedance_data NULL so no managed block survives teardown.
 */
  void
Free_Impedance_Buffers( void )
{
  if( impedance_data == NULL )
    return;

  /* Release each fstep's per-port sub-buffers, then the outer array. */
  int nfrq = mem_array_count( impedance_data );
  for( int idx = 0; idx < nfrq; idx++ )
    free_impedance_step( &impedance_data[idx] );

  mem_array_free( &impedance_data );
}

/*-----------------------------------------------------------------------*/

/* Rescan_Zpnorm()
 *
 * Recomputes the VSWR-normalization running maximum impedance magnitude over
 * already-computed frequency steps for the selected excitation port.
 */
  void
Rescan_Zpnorm( void )
{
  if( Num_Feedpoint_Ports() <= 0 )
    return;

  calc_data.zpnorm = 0.0;
  for( int fs = 0; fs <= calc_data.steps_total; fs++ )
    if( save.fstep[fs] &&
        (impedance_data[fs].zmagn[calc_data.ex_port] > calc_data.zpnorm) )
      calc_data.zpnorm = impedance_data[fs].zmagn[calc_data.ex_port];
}

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
      fpat.ploss += 0.5* cmg* cmg* creal( zload.zarray[i])* data.segments[i].si;

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

    /* Store radiation efficiency per frequency step */
    int fstep = calc_data.freq_step;
    if (fstep >= 0 && fpat.pinr > 0.0)
    {
      rad_pattern[fstep].efficiency =
        (fpat.pinr - fpat.ploss - fpat.pnlr) / fpat.pinr;
    }
  }

} /* Radiation_Pattern() */

/*-----------------------------------------------------------------------*/

/* Near_Field_Pattern()
 *
 * Calculates near field pattern if enabled
 */
  void
Near_Field_Pattern( void )
{
  if( isFlagClear(ENABLE_NEAREH) )
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

  g_rec_mutex_lock(&freq_data_lock);

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
  Set_Network_Data();

  /* Calculate power loss */
  Power_Loss();

  /* Calculate radiation pattern */
  Radiation_Pattern();

  /* Near field calculation */
  Near_Field_Pattern();

  /* Per-fstep noise temperature table: frequency is fixed here, so all
   * sky/earth model × method combinations are deterministic and hoistable. */
  ant_temp_fill_fstep( calc_data.freq_step );

  /* Save per-step results before prerender so struct_colors_fill_fstep
   * reads current crnt_fstep / near_field_fstep data. */
  Save_Crnt_Data( calc_data.freq_step );
  Save_Nearfield_Data( calc_data.freq_step );

  /* Child-deterministic per-fstep prerender: no user-mutable inputs enter
   * these functions. */
  struct_colors_fill_fstep( calc_data.freq_step );

  if( !CHILD )
  {
    if( save.fstep != NULL && calc_data.freq_step >= 0 )
      save.fstep[calc_data.freq_step] = 1;
  }

  g_rec_mutex_unlock(&freq_data_lock);

  // Calculate elapsed time
  clock_gettime(CLOCK_MONOTONIC, &end);
  
  elapsed = (end.tv_sec + (double)end.tv_nsec/1e9) - (start.tv_sec + (double)start.tv_nsec/1e9);
  pr_info("%.6f MHz: %f seconds. (%s)\n",
			calc_data.freq_mhz, elapsed, current_mathlib->name);

} /* New_Frequency()  */

/*-----------------------------------------------------------------------*/

typedef struct
{
  int              max_step;     /* Highest dispatchable index (steps_total or steps_total-1) */
  int              next_scan;    /* Resume point for dispatch step scan */
  struct timespec  t0;           /* Wall-clock start time */
  child_proc_t   **idle_stack;   /* LIFO stack of idle child pointers */
  int              idle_top;     /* Index of top entry; -1 = empty */
} freq_loop_state_t;

/* Per-sweep state; allocated by Start_Frequency_Loop(), freed by thread/Stop */
static freq_loop_state_t *floop_state = NULL;

/*
 * green_line_class_t - disposition of the green-line frequency relative to the
 * sweep steps and the populated display extent.  Closed enum: every consumer
 * dispatches on it exhaustively.
 */
typedef enum
{
  GREEN_LINE_INACTIVE,  /* fmhz_save <= 0, NaN, or no FR cards */
  GREEN_LINE_ALIAS,     /* coincides with a sweep step within FREQ_EPSILON */
  GREEN_LINE_EXTRA,     /* in FR data range or display extent; compute extra slot */
  GREEN_LINE_STALE      /* outside every FR data range and the display extent */
} green_line_class_t;

/* Classification result; alias_step is meaningful only for GREEN_LINE_ALIAS. */
typedef struct
{
  green_line_class_t cls;
  int                alias_step;
} green_line_eval_t;

/*
 * green_line_eval - classify the green-line frequency against sweep and display
 *
 * Single authority for every judgment of calc_data.fmhz_save relative to the
 * sweep steps, the FR card data ranges, and the populated display extent.
 * Membership is the union of the geometry-free FR data range and the display
 * extent populated by freqplots_update_fscale_extents.  ALIAS precedes the
 * range tests so a sweep-step coincidence deduplicates the extra slot.
 * save.freq[0..steps_total-1] must be populated by freq_populate_steps.
 */
static green_line_eval_t
green_line_eval( void )
{
  green_line_eval_t result = { GREEN_LINE_INACTIVE, -1 };
  double fmhz = calc_data.fmhz_save;

  if( fmhz <= 0.0 || isnan(fmhz) || calc_data.FR_cards < 1 )
    return result;

  for( int i = 0; i < calc_data.steps_total; i++ )
    if( FREQ_EQ(fmhz, save.freq[i]) )
    {
      result.cls        = GREEN_LINE_ALIAS;
      result.alias_step = i;
      return result;
    }

  if( freqloop_card_of_fmhz(fmhz) >= 0 )
    result.cls = GREEN_LINE_EXTRA;
  else
    result.cls = GREEN_LINE_STALE;

  return result;
}

/**
 * freq_loop_display_step - determine the step to show in the UI
 *
 * Scans save.fstep[] for the highest completed sweep entry.  When the green-
 * line slot is active and computed it takes priority; when the green line
 * aliases a computed sweep step it routes to that matching step.
 * Returns the step index, or -1 if no data is available yet.
 */
static int
freq_loop_display_step( void )
{
  int idx, step = -1;

  g_rec_mutex_lock(&freq_data_lock);

  for( idx = 0; idx < calc_data.steps_total; idx++ )
    if( save.fstep[idx] )
      step = idx;

  /* Dispatch the green-line selection on its classification: the extra slot
   * wins when computed, an alias routes to its matching sweep step, and a
   * stale or inactive green line keeps the highest completed sweep step. */
  green_line_eval_t gl = green_line_eval();
  switch( gl.cls )
  {
    case GREEN_LINE_EXTRA:
      if( save.fstep[calc_data.steps_total] )
        step = calc_data.steps_total;
      break;

    case GREEN_LINE_ALIAS:
      if( save.fstep[gl.alias_step] )
        step = gl.alias_step;
      break;

    case GREEN_LINE_STALE:
    case GREEN_LINE_INACTIVE:
      /* Keep the highest completed sweep step found above. */
      break;
  }

  g_rec_mutex_unlock(&freq_data_lock);
  return step;
}

/**
 * freq_step_update_ui - apply a frequency step change to all UI consumers
 * @new_step: frequency step index to activate (0..steps_total)
 *
 * Sets calc_data.freq_step and all derived display state, then queues
 * redraws for every frequency-dependent window.  Unconditional and
 * idempotent.  Must be called on the GTK main thread.
 */
void
freq_step_update_ui( int new_step, gboolean force )
{
  char txt[16];

  g_rec_mutex_lock(&freq_data_lock);

  if( save.freq == NULL )
  {
    g_rec_mutex_unlock(&freq_data_lock);
    return;
  }

  calc_data.freq_step = new_step;
  calc_data.freq_mhz  = save.freq[new_step];
  SetFlag( DRAW_NEW_RDPAT );
  SetFlag( FREQ_LOOP_READY );

  /* Block value-changed callbacks during programmatic spinbutton updates;
   * only user interaction sets fmhz_save via those callbacks. */
  SIGNAL_BLOCK(mainwin_frequency, on_config_widget_changed);
  gtk_spin_button_set_value( mainwin_frequency, calc_data.freq_mhz );
  SIGNAL_UNBLOCK(mainwin_frequency, on_config_widget_changed);

  if( isFlagSet(DRAW_ENABLED) && rdpattern_frequency != NULL )
  {
    SIGNAL_BLOCK(rdpattern_frequency, on_config_widget_changed);
    gtk_spin_button_set_value( rdpattern_frequency, calc_data.freq_mhz );
    SIGNAL_UNBLOCK(rdpattern_frequency, on_config_widget_changed);
  }

  if( isFlagSet(PLOT_ENABLED) )
  {
    snprintf( txt, sizeof(txt), "%.3f", calc_data.freq_mhz );
    gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
        freqplots_window_builder, "freqplots_fmhz_entry")), txt );

    freqplots_redraw_all(force);
  }

  /* Vertex colors are baked per freq_step; invalidate so the next render
   * rebuilds them from crnt_fstep[new_step] rather than cached stale data. */
#ifdef HAVE_OPENGL
  opengl_structure_invalidate();
#endif

  xnec2_widget_queue_draw( structure_drawingarea, force );

  if( isFlagSet(DRAW_ENABLED) )
  {
    xnec2_widget_queue_draw( rdpattern_drawingarea, force );
  }

  opt_ui_update_values();

  g_rec_mutex_unlock(&freq_data_lock);
}

/* Idle wrapper: intermediate loop step — draws suppressed during optimizer */
static gboolean
freq_step_update_ui_idle( gpointer p )
{
  freq_step_update_ui( GPOINTER_TO_INT(p), FALSE );
  return G_SOURCE_REMOVE;
}

/* Idle wrapper: terminal step — draws forced through SUPPRESS gate.
 * Do not flush GTK events here — _callback_g_idle_add_once flushes
 * after we return for sync paths; flushing here would process arbitrary
 * idle sources (e.g. eval_apply_and_reload) and deadlock on pthread_join. */
static gboolean
freq_step_update_ui_idle_force( gpointer p )
{
  freq_step_update_ui( GPOINTER_TO_INT(p), TRUE );
  return G_SOURCE_REMOVE;
}

/**
 * freq_populate_steps - pre-compute save.freq[] from FR card parameters
 *
 * Fills save.freq[0..steps_total-1] with frequencies derived from each
 * FR card's min_freq, delta_freq, and ifreq (linear vs multiplicative).
 * If fmhz_save is valid, also sets save.freq[steps_total] for the green
 * line slot.  Returns the highest dispatchable step index (max_step).
 */
static int
freq_populate_steps( void )
{
  int step = 0, fr, card_start;
  double freq;
  freq_loop_data_t *fld;

  for( fr = 0; fr < calc_data.FR_cards; fr++ )
  {
    fld = &calc_data.freq_loop_data[fr];
    freq = fld->min_freq;
    card_start = step;

    for( ; step < card_start + fld->freq_steps && step < calc_data.steps_total; step++ )
    {
      if( step > card_start )
      {
        freq = (fld->ifreq == 1)
            ? freq * fld->delta_freq
            : freq + fld->delta_freq;
      }
      save.freq[step] = freq;
    }
  }

  /* Admit the extra green-line slot only when its frequency is in range or
   * visible; alias, stale, and inactive green lines exclude the slot from the
   * scan range so an out-of-range frequency is never dispatched. */
  green_line_eval_t gl = green_line_eval();
  switch( gl.cls )
  {
    case GREEN_LINE_EXTRA:
      save.freq[calc_data.steps_total] = calc_data.fmhz_save;
      return calc_data.steps_total;

    case GREEN_LINE_ALIAS:
    case GREEN_LINE_STALE:
    case GREEN_LINE_INACTIVE:
      save.fstep[calc_data.steps_total] = 0;
      return calc_data.steps_total - 1;
  }

  BUG("green_line_eval returned unknown class %d\n", gl.cls);
  return calc_data.steps_total - 1;
}

/*
 * freq_loop_collect - save per-step results into shared arrays
 * @fstep: step index to store results at
 *
 * Called under freq_data_lock.  For the forked path the caller has already
 * read raw data via Get_Freq_Data(); for the non-forked path the data is
 * already in memory from New_Frequency().
 */
static void
freq_loop_collect( int fstep )
{
  Save_Crnt_Data( fstep );
  Save_Nearfield_Data( fstep );
  save.fstep[fstep] = 1;
}

static inline gboolean
idle_stack_empty( const freq_loop_state_t *state )
{ return state->idle_top < 0; }

static inline gboolean
idle_stack_full( const freq_loop_state_t *state )
{ return state->idle_top == calc_data.num_jobs - 1; }

/* Returns TRUE if any child has an assigned step not yet collected */
static gboolean
children_dispatched( void )
{
  int i;

  for( i = 0; i < calc_data.num_jobs; i++ )
    if( child_procs[i]->assigned_step != -1 )
      return TRUE;

  return FALSE;
}

/* Returns TRUE if step is already dispatched to a child (in-flight) */
static gboolean
step_in_flight( int step )
{
  int i;

  for( i = 0; i < calc_data.num_jobs; i++ )
    if( child_procs[i]->assigned_step == step )
      return TRUE;

  return FALSE;
}

static inline child_proc_t *
idle_stack_pop( freq_loop_state_t *state )
{ return state->idle_stack[state->idle_top--]; }

static inline void
idle_stack_push( freq_loop_state_t *state, child_proc_t *child )
{ state->idle_stack[++state->idle_top] = child; }

/**
 * freq_loop_validate_result - check if a child's result is still current
 * @state: loop state; child pushed back to idle stack on stale result
 * @child: child that finished computing
 *
 * Compares the frequency dispatched to the child against the current
 * value in save.freq[].  If they differ (external invalidation raced
 * the computation), logs a notice, discards the result, and returns
 * the child to the idle stack.
 *
 * Returns TRUE if the result is valid; FALSE if stale (caller skips collect).
 * Called under freq_data_lock.
 */
static gboolean
freq_loop_validate_result( freq_loop_state_t *state, child_proc_t *child )
{
  if( FREQ_EQ(save.freq[child->assigned_step], child->assigned_freq) )
    return TRUE;

  pr_notice("step %d raced: dispatched %.6f MHz, current %.6f MHz; will recalculate\n",
      child->assigned_step, child->assigned_freq,
      save.freq[child->assigned_step]);
  child->assigned_step = -1;
  idle_stack_push( state, child );
  return FALSE;
}

/*
 * freq_loop_dispatch - send one frequency step to a child or compute inline
 * @state: loop state; idle_stack updated for non-forked path
 * @child: child process descriptor
 * @fstep: step index being dispatched
 * @freq:  frequency in MHz to compute
 * @batch: TRUE for batch mathlib, FALSE for interactive mathlib
 *
 * Forked: sets child->assigned_step and writes MATHLIB+FRQDATA to pipe.
 * Non-forked: computes inline under freq_data_lock, collects, resets
 * child->assigned_step to -1, and pushes child back onto the idle stack.
 * The COMPUTE loop detects synchronous completion via child->assigned_step == -1.
 * Non-forked path: New_Frequency acquires freq_data_lock internally.
 */
static void
freq_loop_dispatch( freq_loop_state_t *state, child_proc_t *child,
                    int fstep, double freq, gboolean batch )
{
  char   *buff;
  size_t  len;

  child->assigned_step = fstep;
  child->assigned_freq = freq;

  /* Record dispatched frequency so freq_loop_validate_result and the
   * dispatch scan can compare stored vs desired for the extra slot. */
  save.freq[fstep] = freq;

  if( FORKED )
  {
    const char *mathlib_id = batch
        ? rc_config.mathlib_batch_id
        : current_mathlib->id;

    if( batch )
      mathlib_lock_intel_batch( mathlib_id );
    else
      mathlib_lock_intel_interactive( mathlib_id );

    len = strlen( fork_commands[MATHLIB] );
    Write_Pipe( child->idx, fork_commands[MATHLIB], (ssize_t)len, TRUE );
    Write_Pipe( child->idx, (char *)mathlib_id, (ssize_t)MATHLIB_ID_LEN, TRUE );

    len = strlen( fork_commands[FRQDATA] );
    Write_Pipe( child->idx, fork_commands[FRQDATA], (ssize_t)len, TRUE );
    buff = (char *)&freq;
    Write_Pipe( child->idx, buff, (ssize_t)sizeof(double), TRUE );
    return;
  }

  /* Non-forked: write freq_mhz and freq_step for the NEC engine here;
   * freq_step_update_ui overwrites both on the GTK thread for display.
   * New_Frequency acquires freq_data_lock internally. */
  calc_data.freq_mhz  = freq;
  calc_data.freq_step = fstep;
  New_Frequency();

  /* Non-forked: computation is synchronous.  Child stays off the idle stack
   * with assigned_step set; freq_loop_collect_pending() handles collect
   * and push-back so the COMPUTE loop needs no forked/non-forked branch. */
}

/*
 * freq_loop_collect_pending - collect one round of finished forked children
 * @state: loop state; idle_stack updated in place
 *
 * Blocks in select() until at least one child pipe is readable, then
 * processes all ready children.  Forked path acquires freq_data_lock
 * internally; non-forked path is lock-free (saves done in New_Frequency).
 *
 * Returns FALSE and sets FREQ_LOOP_STOP if a pipe read fails; TRUE otherwise.
 */
static gboolean
freq_loop_collect_pending( freq_loop_state_t *state )
{
  fd_set read_fds;
  int    n = 0, sel_ret, idx;

  FD_ZERO( &read_fds );
  for( idx = 0; idx < calc_data.num_jobs; idx++ )
  {
    if( child_procs[idx]->assigned_step == -1 )
      continue;

    int rfd = child_procs[idx]->from_child[READ];
    if( rfd < 0 )
    {
      if( FORKED )
        pr_warn("child %d has invalid pipe fd during forked collect\n", idx);
      continue;
    }

    FD_SET( rfd, &read_fds );
    if( n < rfd )
      n = rfd;
  }

  /* Non-forked path: no pipe fds (n==0); dispatch() computed synchronously
   * but left the child off the idle stack with assigned_step set.
   * Collect results and return child to the idle stack. */
  /* Non-forked path: New_Frequency already saved under freq_data_lock;
   * collect results and return children to the idle stack. */
  if( n == 0 )
  {
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      if( child_procs[idx]->assigned_step == -1 )
        continue;

      if( !freq_loop_validate_result( state, child_procs[idx] ) )
        continue;

      freq_loop_collect( child_procs[idx]->assigned_step );
      child_procs[idx]->assigned_step = -1;
      idle_stack_push( state, child_procs[idx] );
    }
    return TRUE;
  }

  do
  {
    sel_ret = select( n + 1, &read_fds, NULL, NULL, NULL );
  } while( sel_ret == -1 && errno == EINTR );

  if( sel_ret == -1 )
  {
    perror( "select()" );
    _exit(0);
  }

  g_rec_mutex_lock(&freq_data_lock);
  for( idx = 0; idx < num_child_procs; idx++ )
  {
    if( child_procs[idx]->assigned_step == -1 )
      continue;

    if( !FD_ISSET(child_procs[idx]->from_child[READ], &read_fds) )
      continue;

    int child_fstep = child_procs[idx]->assigned_step;

    if( !Get_Freq_Data( idx, child_fstep ) )
    {
      pr_err("Failed to read data from forked child\n");
      SetFlag(FREQ_LOOP_STOP);
      g_rec_mutex_unlock(&freq_data_lock);
      return FALSE;
    }

    /* Invalidate parent dedup cache; Get_Freq_Data overwrote local EM arrays */
    New_Frequency_Reset_Prev();

    if( !freq_loop_validate_result( state, child_procs[idx] ) )
      continue;

    freq_loop_collect( child_fstep );
    child_procs[idx]->assigned_step = -1;
    idle_stack_push( state, child_procs[idx] );
  }
  g_rec_mutex_unlock(&freq_data_lock);

  return TRUE;
}


/**
 * fmhz_save_apply_idle - GTK idle callback to apply reset fmhz_save via
 *     user_set_frequency (the single point of truth for frequency selection).
 */
static void
fmhz_save_apply_idle(gpointer data)
{
  (void)data;
  user_set_frequency(calc_data.fmhz_save);
}

/* Cost of a populated sweep step for selection; return NAN to exclude idx.
 * The runner minimizes cost, so a maximized measure returns its negation. */
typedef double (*step_cost_fn)(int idx, double target);

/**
 * freq_best_step() - Scan populated steps and return the lowest-cost one
 * @cost:   per-step cost; NAN excludes the step from selection
 * @target: target MHz passed to @cost; ignored by costs that need none
 *
 * Caller holds freq_data_lock when concurrent sweep updates are possible.
 * Returns -1 when no populated step yields a finite cost.
 */
  static int
freq_best_step(step_cost_fn cost, double target)
{
  double best_cost = G_MAXDOUBLE;
  int best_step = -1;
  int idx;

  for( idx = 0; idx < calc_data.steps_total; idx++ )
  {
    if( !save.fstep[idx] )
      continue;

    double step_cost = cost(idx, target);
    if( isnan(step_cost) )
      continue;

    if( step_cost < best_cost )
    {
      best_cost = step_cost;
      best_step = idx;
    }
  }

  return best_step;

} /* freq_best_step() */

/*-----------------------------------------------------------------------*/

/* Selection cost: VSWR at idx; a negative or NaN VSWR excludes the step.
 * target is unused; the uniform step_cost_fn signature carries it. */
  static double
step_vswr_cost(int idx, double _target)
{
  measurement_t measurement;

  meas_calc(&measurement, idx, calc_data.ex_port);
  return (measurement.vswr >= 0.0) ? measurement.vswr : NAN;

} /* step_vswr_cost() */

/*-----------------------------------------------------------------------*/

/* Selection cost: negated maximum gain at idx so the minimizer maximizes gain.
 * target is unused; the uniform step_cost_fn signature carries it. */
  static double
step_gain_cost(int idx, double _target)
{
  measurement_t measurement;

  meas_calc(&measurement, idx, calc_data.ex_port);
  return -measurement.gain_max;

} /* step_gain_cost() */

/*-----------------------------------------------------------------------*/

/* Selection cost: distance from the target MHz. */
  static double
step_freq_distance_cost(int idx, double target)
{
  return fabs(save.freq[idx] - target);

} /* step_freq_distance_cost() */

/*-----------------------------------------------------------------------*/

/**
 * sweep_center_mhz() - Midpoint frequency of the populated sweep range
 *
 * Caller holds freq_data_lock when concurrent sweep updates are possible.
 * Returns NAN when no step is populated.
 */
  static double
sweep_center_mhz(void)
{
  double lo = G_MAXDOUBLE;
  double hi = -G_MAXDOUBLE;
  gboolean found = FALSE;
  int idx;

  for( idx = 0; idx < calc_data.steps_total; idx++ )
  {
    if( !save.fstep[idx] )
      continue;

    found = TRUE;
    if( save.freq[idx] < lo )
      lo = save.freq[idx];
    if( save.freq[idx] > hi )
      hi = save.freq[idx];
  }

  if( !found )
    return NAN;

  return (lo + hi) / 2.0;

} /* sweep_center_mhz() */

/*-----------------------------------------------------------------------*/

/* Target source: the explicit --freq-select MHz value. */
  static double
config_select_mhz(void)
{
  return rc_config.freq_select_mhz;

} /* config_select_mhz() */

/*-----------------------------------------------------------------------*/

/*
 * A post-sweep frequency selection is one argopt over the populated steps:
 * minimize a per-step cost, optionally toward a target frequency.  A NULL
 * cost is the empty selection that keeps the previous slot.
 */
typedef struct
{
  step_cost_fn cost;             /* per-step cost; NAN excludes a step */
  double     (*target)(void);    /* NULL unless the cost needs a target MHz */
} freq_select_spec_t;

/* One row per freq_select_mode_t; FREQ_SELECT_NONE is the empty policy row. */
static const freq_select_spec_t freq_select_specs[FREQ_SELECT_COUNT] = {
  [FREQ_SELECT_MIN_SWR]  = { step_vswr_cost,          NULL             },
  [FREQ_SELECT_MAX_GAIN] = { step_gain_cost,          NULL             },
  [FREQ_SELECT_CENTER]   = { step_freq_distance_cost, sweep_center_mhz  },
  [FREQ_SELECT_MHZ]      = { step_freq_distance_cost, config_select_mhz },
};

/**
 * freq_run_spec() - Resolve one selection spec to a sweep step
 * @spec: selection criterion; a NULL cost keeps the previous slot
 *
 * Resolves the optional target source, then returns the lowest-cost populated
 * step.  Caller holds freq_data_lock when concurrent sweep updates are
 * possible.  Returns -1 to keep the previous frequency slot.
 */
  static int
freq_run_spec(const freq_select_spec_t *spec)
{
  double target = 0.0;

  if( spec->cost == NULL )
    return -1;

  if( spec->target != NULL )
  {
    target = spec->target();
    if( isnan(target) )
      return -1;
  }

  return freq_best_step(spec->cost, target);

} /* freq_run_spec() */

/*-----------------------------------------------------------------------*/

/**
 * freq_default_spec() - Green-line-keyed default selection (FREQ_SELECT_NONE)
 *
 * An unusable green line defaults to the neutral sweep center: an inactive line
 * was never chosen, and a stale line fell out of range with no evidence of a
 * prior VSWR preference.  A usable alias or in-range line keeps the previous
 * slot.  Caller holds freq_data_lock when concurrent sweep updates are
 * possible.
 */
  static const freq_select_spec_t *
freq_default_spec(void)
{
  green_line_eval_t gl = green_line_eval();

  switch( gl.cls )
  {
    case GREEN_LINE_INACTIVE:
    case GREEN_LINE_STALE:
      return &freq_select_specs[FREQ_SELECT_CENTER];

    case GREEN_LINE_ALIAS:
    case GREEN_LINE_EXTRA:
      return &freq_select_specs[FREQ_SELECT_NONE];
  }

  BUG("green_line_eval returned unknown class %d\n", gl.cls);
  return &freq_select_specs[FREQ_SELECT_NONE];

} /* freq_default_spec() */

/*-----------------------------------------------------------------------*/

/**
 * fmhz_save_apply_selection - position the post-sweep selected frequency
 *
 * Resolves rc_config.freq_select_mode to a selection spec: an explicit
 * --freq-select target (minimum VSWR, sweep center, maximum gain, or the step
 * nearest a requested MHz), else the green-line-keyed default.  Sets fmhz_save
 * and queues the frequency change on the GTK main thread.  Shared by
 * interactive loads and batch capture.
 *
 * Returns TRUE when fmhz_save was set, signalling the caller to skip its
 * default display-step logic; FALSE keeps the previous frequency slot.
 */
static gboolean
fmhz_save_apply_selection(void)
{
  measurement_t measurement;
  freq_select_mode_t mode = rc_config.freq_select_mode;
  const freq_select_spec_t *spec;
  int step;

  if( mode < 0 || mode >= FREQ_SELECT_COUNT )
  {
    BUG("invalid frequency selection mode %d\n", mode);
    return FALSE;
  }

  spec = (mode == FREQ_SELECT_NONE)
      ? freq_default_spec()
      : &freq_select_specs[mode];

  step = freq_run_spec(spec);

  /* No populated step resolved; leave fmhz_save unchanged. */
  if( step < 0 )
    return FALSE;

  /* An explicit --freq-select MHz target warns when the nearest populated step
     misses it; a derived target has no user expectation to violate. */
  if( mode == FREQ_SELECT_MHZ
      && !FREQ_EQ(save.freq[step], rc_config.freq_select_mhz) )
    pr_warn("nearest step %g MHz differs from requested %g MHz\n",
        save.freq[step], rc_config.freq_select_mhz);

  meas_calc(&measurement, step, calc_data.ex_port);
  pr_notice("post-sweep frequency set to %.4f MHz (VSWR %.2f)\n",
      save.freq[step], measurement.vswr);

  calc_data.fmhz_save = save.freq[step];
  /* Queue the full UI update on the GTK main thread; the idle handler applies
     the user-selected frequency change. */
  g_idle_add_once((GSourceOnceFunc)fmhz_save_apply_idle, NULL);
  return TRUE;
}

/*
 * freq_loop_finalize - complete a finished sweep
 * @state: loop state (for elapsed-time calculation)
 *
 * Sets FREQ_LOOP_DONE, logs elapsed time, wakes the optimizer, and
 * queues final UI updates.  No locks held on entry.
 */
static void
freq_loop_finalize( freq_loop_state_t *state )
{
  struct timespec end;

  SetFlag( FREQ_LOOP_DONE );

  /* Dump the validation data tree when --write-validation-dir is set;
   * no-op otherwise.  All per-fstep arrays are populated at this point. */
  Save_Validation_Tree();

  clock_gettime(CLOCK_MONOTONIC, &end);
  pr_notice("Frequency loop elapsed time: %f seconds. (%s)\n",
    (end.tv_sec  + (double)end.tv_nsec  / 1e9) -
    (state->t0.tv_sec + (double)state->t0.tv_nsec / 1e9),
    (FORKED ? get_mathlib_by_id(rc_config.mathlib_batch_id)->name
            : current_mathlib->name));

  /* Wake optimizer thread waiting on eval_cond */
  nec2_eval_signal();

  /* Position the post-sweep selected frequency: an explicit --freq-select
   * target, else the green-line default (center when unavailable, lowest-VSWR
   * when stale).  When it applies, fmhz_save_apply_selection queues
   * user_set_frequency on the GTK main thread which handles all UI updates;
   * otherwise keep the previous frequency slot and show the highest completed
   * sweep step. */
  if( !fmhz_save_apply_selection() )
  {
    int display = freq_loop_display_step();
    if( display >= 0 )
      g_idle_add_once( (GSourceOnceFunc)freq_step_update_ui_idle_force,
                            GINT_TO_POINTER(display) );
  }

  if( (rc_config.batch_mode || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS)) &&
      opt_have_files_to_save() )
    /* Async: sync would deadlock if the optimizer queues
     * eval_apply_and_reload before this idle source is processed. */
    g_idle_add_once((GSourceOnceFunc)Write_Optimizer_Data, NULL);
}

/*-----------------------------------------------------------------------*/

/**
 * batch_force_render() - Render a batch drawing widget synchronously
 * @widget: active drawing widget
 * @width: drawing width in pixels
 * @height: drawing height in pixels
 *
 * Batch completion has no frame-clock iteration before teardown.  Drawing to
 * an image surface fills the active renderer's capture source synchronously.
 */
  static void
batch_force_render(GtkWidget *widget, int width, int height)
{
  GtkAllocation allocation = {0};
  cairo_surface_t *surface;
  cairo_t *context;

  allocation.width = width;
  allocation.height = height;
  gtk_widget_size_allocate(widget, &allocation);

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  context = cairo_create(surface);
  gtk_widget_draw(widget, context);
  cairo_destroy(context);
  cairo_surface_destroy(surface);

} /* batch_force_render() */

/*-----------------------------------------------------------------------*/

/**
 * batch_rdpattern_capture_size() - Resolve native dimensions for batch panes
 *
 * Returns configured radiation-pattern dimensions, falling back to the
 * established batch default when the configuration has no usable size.
 */
  static batch_capture_size_t
batch_rdpattern_capture_size(void)
{
  batch_capture_size_t size = {
    .width = rc_config.rdpattern_width,
    .height = rc_config.rdpattern_height,
  };

  if( size.width <= 1 || size.height <= 1 )
  {
    size.width = BATCH_RDPAT_DEFAULT_PX;
    size.height = BATCH_RDPAT_DEFAULT_PX;
  }

  return size;

} /* batch_rdpattern_capture_size() */

/*-----------------------------------------------------------------------*/

/**
 * batch_apply_rdpattern_fit() - Apply the fitted radiation-pattern view
 */
  static void
batch_apply_rdpattern_fit(void)
{
  view_fit_t fit = {0};

  if( !render_fit_view(rdpattern_view, &fit) )
    return;

  if( rdpattern_view->zoom_spin != NULL )
    SIGNAL_BLOCK(rdpattern_view->zoom_spin,
        G_CALLBACK(on_rdpattern_zoom_spinbutton_value_changed));

  view_apply_fit(rdpattern_view, &fit);

  if( rdpattern_view->zoom_spin != NULL )
    SIGNAL_UNBLOCK(rdpattern_view->zoom_spin,
        G_CALLBACK(on_rdpattern_zoom_spinbutton_value_changed));

} /* batch_apply_rdpattern_fit() */

/*-----------------------------------------------------------------------*/

/**
 * batch_prepare_rdpattern_window() - Realize the radiation-pattern capture view
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns TRUE when the radiation-pattern view is available at capture size.
 */
  static gboolean
batch_prepare_rdpattern_window(int width, int height)
{
  GtkWidget *menu_item;

  if( rdpattern_window == NULL )
  {
    menu_item = Builder_Get_Object(main_window_builder, "main_rdpattern");
    if( menu_item == NULL )
      return FALSE;

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
  }

  if( rdpattern_window == NULL || rdpattern_drawingarea == NULL
      || rdpattern_view == NULL || rdpattern_window_builder == NULL )
    return FALSE;

  gtk_window_resize(GTK_WINDOW(rdpattern_window), width, height);
  gtk_widget_show_now(rdpattern_window);

  return TRUE;

} /* batch_prepare_rdpattern_window() */

/*-----------------------------------------------------------------------*/

/**
 * batch_capture_fitted_rdpattern_pane() - Capture the fitted radiation pattern
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns an owned pixbuf from the prepared radiation-pattern view, or NULL
 * when the active renderer cannot capture it.
 */
  static GdkPixbuf *
batch_capture_fitted_rdpattern_pane(int width, int height)
{
  batch_force_render(rdpattern_drawingarea, width, height);
  batch_apply_rdpattern_fit();
  batch_force_render(rdpattern_drawingarea, width, height);

  return render_capture_widget(rdpattern_drawingarea, width, height);

} /* batch_capture_fitted_rdpattern_pane() */

/*-----------------------------------------------------------------------*/

/**
 * batch_capture_rdpattern_preset_pane() - Capture a fitted preset view
 * @button_id: builder ID for the required radiation-pattern preset button
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns an owned pixbuf, or NULL when the radiation-pattern view or active
 * renderer cannot capture the selected preset.
 */
  static GdkPixbuf *
batch_capture_rdpattern_preset_pane(const char *button_id, int width, int height)
{
  GtkWidget *button;

  if( !batch_prepare_rdpattern_window(width, height) )
    return NULL;

  button = Builder_Get_Object(rdpattern_window_builder, button_id);
  if( button == NULL )
    return NULL;

  gtk_button_clicked(GTK_BUTTON(button));

  return batch_capture_fitted_rdpattern_pane(width, height);

} /* batch_capture_rdpattern_preset_pane() */

/*-----------------------------------------------------------------------*/

/**
 * batch_prepare_rdpattern_capture() - Resolve capture size at the selected step
 * @filename: destination PNG filename for diagnostics
 * @size: destination for the resolved capture dimensions
 *
 * Reads calc_data.freq_step, the single post-sweep selection applied by
 * freq_loop_finalize; performs no reselection.  Returns TRUE when that step
 * holds computed data for the radiation-pattern view.
 */
  static gboolean
batch_prepare_rdpattern_capture(const char *filename, batch_capture_size_t *size)
{
  int step;

  g_rec_mutex_lock(&freq_data_lock);
  step = calc_data.freq_step;
  if( save.fstep == NULL || step < 0 || !save.fstep[step] )
    step = -1;
  g_rec_mutex_unlock(&freq_data_lock);

  if( step < 0 )
  {
    pr_err("radiation pattern PNG capture: no computed step for %s\n",
        filename);
    return FALSE;
  }

  *size = batch_rdpattern_capture_size();

  return TRUE;

} /* batch_prepare_rdpattern_capture() */

/*-----------------------------------------------------------------------*/

/**
 * batch_write_rdpat_png() - Write one radiation-pattern PNG target
 * @format: selected radiation-pattern view format
 * @filename: destination PNG filename
 * @size: capture image dimensions
 *
 * Runs on the GTK main thread before batch teardown.  The active renderer
 * owns capture through render_capture_widget().
 */
  static void
batch_write_rdpat_png(rdpat_png_format_t format, const char *filename,
    const batch_capture_size_t *size)
{
  GdkPixbuf *captures[G_N_ELEMENTS(batch_rdpattern_panes)] = { NULL };
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;
  gsize idx;

  if( format == RDPAT_PNG_FORMAT_QUAD )
  {
    for( idx = 0; idx < G_N_ELEMENTS(batch_rdpattern_panes); idx++ )
    {
      const batch_rdpattern_pane_t *pane = &batch_rdpattern_panes[idx];

      captures[idx] = batch_capture_rdpattern_preset_pane(pane->button_id,
          size->width, size->height);
      if( captures[idx] == NULL )
        goto out_unref;
    }

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
        size->width * 2, size->height * 2);
    if( pixbuf == NULL )
      goto out_unref;

    for( idx = 0; idx < G_N_ELEMENTS(batch_rdpattern_panes); idx++ )
    {
      const batch_rdpattern_pane_t *pane = &batch_rdpattern_panes[idx];

      gdk_pixbuf_copy_area(captures[idx], 0, 0, size->width, size->height,
          pixbuf, pane->column * size->width, pane->row * size->height);
    }
  }
  else
  {
    for( idx = 0; idx < G_N_ELEMENTS(batch_rdpattern_panes); idx++ )
    {
      const batch_rdpattern_pane_t *pane = &batch_rdpattern_panes[idx];

      if( pane->format == format )
      {
        pixbuf = batch_capture_rdpattern_preset_pane(pane->button_id,
            size->width, size->height);
      }
    }
  }

  if( pixbuf == NULL )
  {
    pr_err("radiation pattern PNG capture failed for %s\n", filename);
    goto out_unref;
  }

  if( !gdk_pixbuf_save(pixbuf, filename, "png", &error, NULL) )
  {
    pr_err("radiation pattern PNG save failed for %s: %s\n", filename,
        error->message);
    g_error_free(error);
  }
  else
    pr_notice("wrote radiation pattern PNG: %s\n", filename);

out_unref:
  if( pixbuf != NULL )
    g_object_unref(pixbuf);

  for( idx = 0; idx < G_N_ELEMENTS(captures); idx++ )
  {
    if( captures[idx] != NULL )
      g_object_unref(captures[idx]);
  }

} /* batch_write_rdpat_png() */

/*-----------------------------------------------------------------------*/

/**
 * batch_write_rdpat_pngs() - Write every requested radiation-pattern PNG
 *
 * A multi-format request derives one filename per canonical format name.
 */
  static void
batch_write_rdpat_pngs(void)
{
  const rdpat_png_format_spec_t default_format = {
    .format = RDPAT_PNG_FORMAT_ISO,
  };
  const rdpat_png_format_spec_t *formats;
  batch_capture_size_t size;
  gsize count;
  gsize idx;

  if( rc_config.filename_rdpat_png == NULL )
    return;

  if( rc_config.rdpat_png_formats == NULL )
  {
    formats = &default_format;
    count = 1;
  }
  else
  {
    formats = rc_config.rdpat_png_formats;
    count = mem_array_count(rc_config.rdpat_png_formats);
  }

  if( count != 1 && !g_str_has_suffix(rc_config.filename_rdpat_png, ".png") )
  {
    pr_err("multiple radiation pattern PNG formats require a .png filename: %s\n",
        rc_config.filename_rdpat_png);
    return;
  }

  if( !batch_prepare_rdpattern_capture(rc_config.filename_rdpat_png, &size) )
    return;

  if( count == 1 )
  {
    batch_write_rdpat_png(formats[0].format, rc_config.filename_rdpat_png,
        &size);
  }
  else
  {
    for( idx = 0; idx < count; idx++ )
    {
      char *stem = NULL;
      char *filename = NULL;

      stem = g_strndup(rc_config.filename_rdpat_png,
          strlen(rc_config.filename_rdpat_png) - strlen(".png"));
      filename = g_strdup_printf("%s-%s.png", stem, formats[idx].name);
      g_free(stem);

      batch_write_rdpat_png(formats[idx].format, filename, &size);
      g_free(filename);
    }
  }

} /* batch_write_rdpat_pngs() */

/*-----------------------------------------------------------------------*/

/**
 * batch_capture_and_quit() - Capture an optional batch PNG before teardown
 * @user_data: unused idle source data
 */
  static void
batch_capture_and_quit(gpointer user_data)
{
  (void)user_data;

  batch_write_rdpat_pngs();
  xnec2c_quit(NULL);

} /* batch_capture_and_quit() */

/**
 * batch_finish_no_steps - gracefully complete a zero-step batch deck
 *
 * Runs on the GTK main thread from a g_idle_add_once source scheduled by
 * Open_Input_File when a batch deck carries no dispatchable frequency step
 * (no FR card).  Mirrors the write-and-quit tail of freq_loop_finalize for a
 * zero-step object: the write primitives are bounded by steps_total and
 * yield header-only output at zero steps.  Deferral runs it after
 * Open_Input_File returns, so no widget is touched after xnec2c_quit
 * destroys main_window.
 */
void
batch_finish_no_steps( void )
{
  /* Sweep is trivially complete; gates Save_Validation_Tree's dump. */
  SetFlag( FREQ_LOOP_DONE );

  Save_Validation_Tree();

  if( opt_have_files_to_save() )
    Write_Optimizer_Data();

  batch_write_rdpat_pngs();
  xnec2c_quit( NULL );
}

/**
 * Frequency_Loop - single iteration of the frequency sweep state machine
 * @udata: per-sweep freq_loop_state_t* allocated by Start_Frequency_Loop()
 *
 * States: INIT -> COMPUTE -> PUBLISH -> [FINALIZE | COMPUTE]
 *
 * Returns: TRUE to request another call; FALSE when the sweep is complete
 * or has been stopped and all pending children have been drained.
 */
gboolean
Frequency_Loop( gpointer udata )
{
  freq_loop_state_t *state = (freq_loop_state_t *)udata;
  int idx;

  /* INIT: reset all iteration state for a new sweep */
  if( isFlagSet(FREQ_LOOP_INIT) )
  {
    ClearFlag( FREQ_LOOP_INIT | FREQ_LOOP_DONE );

    state->idle_top     = -1;
    state->next_scan    = 0;
    state->max_step     = freq_populate_steps();

    /* Per-step validity is managed by Start_Frequency_Loop;
     * INIT resets the dedup cache and loop infrastructure. */

    /* Reset parent dedup cache so every sweep recomputes */
    New_Frequency_Reset_Prev();

    /* Push all children onto the idle stack; mark each as idle */
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      child_procs[idx]->assigned_step = -1;
      idle_stack_push( state, child_procs[idx] );
    }

    /* freq_step is a display field owned by the GTK thread; do not reset
     * it here — stale data remains visible during the recomputation cycle
     * and freq_step_update_ui sets it on completion. */
    if( calc_data.zpnorm > 0.0 ) calc_data.iped = 2;

    clock_gettime(CLOCK_MONOTONIC, &state->t0);

    return TRUE;
  }

  /* COMPUTE: producer-consumer loop.
   *
   * Dispatch to all idle children, then block for one collect round when
   * all workers are busy.  Repeat until every step is dispatched and every
   * result collected, or until FREQ_LOOP_STOP is set.
   *
   * Non-forked path: dispatch() is synchronous; one step per Frequency_Loop()
   * call so async redraws can reach the GTK main thread between steps.
   */
  /* Dispatch phase: scan for invalid steps and dispatch to idle children */
  gboolean found_work = FALSE;
  while( !idle_stack_empty(state) && !isFlagSet(FREQ_LOOP_STOP) )
  {
    int next = -1;
    for( idx = state->next_scan; idx <= state->max_step; idx++ )
    {
      if( save.fstep[idx] != 0 || step_in_flight(idx) )
        continue;
      next = idx;
      break;
    }

    /* Wrap to catch externally invalidated steps behind next_scan */
    if( next == -1 && state->next_scan > 0 )
    {
      state->next_scan = 0;
      continue;
    }

    if( next == -1 )
      break;

    found_work = TRUE;
    state->next_scan = next + 1;
    child_proc_t *child = idle_stack_pop( state );
    gboolean batch = (next < calc_data.steps_total);
    freq_loop_dispatch( state, child, next, save.freq[next], batch );
  }


  /* Collect phase: always invoked; handles forked (select+reap) and
   * non-forked (n==0: scan dispatched children, collect, push back). */
  if( !freq_loop_collect_pending(state) )
    return FALSE;

  /* STOP: drain remaining children before exiting */
  if( isFlagSet(FREQ_LOOP_STOP) )
  {
    while( children_dispatched() )
    {
      if( !freq_loop_collect_pending( state ) )
        break;
    }
    return FALSE;
  }

  /* PUBLISH: expose highest completed step to all UI consumers */
  SetFlag( FREQ_LOOP_READY );

  /* Dispatch found nothing and all children have returned */
  if( !found_work && idle_stack_full(state) )
  {
    freq_loop_finalize( state );
    return FALSE;
  }

  if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    int display = freq_loop_display_step();
    if( display >= 0 )
      g_idle_add_once( (GSourceOnceFunc)freq_step_update_ui_idle,
                       GINT_TO_POINTER(display) );
  }
  return TRUE;
} /* Frequency_Loop() */

/*-----------------------------------------------------------------------*/


void *Frequency_Loop_Thread(void *p)
{
	freq_loop_state_t *state = (freq_loop_state_t *)p;
	gboolean batch_complete = FALSE;

	// Don't draw the green line if in batch mode
	if (rc_config.batch_mode)
		calc_data.fmhz_save = 0.0;

	// Run the loop; Frequency_Loop() returns FALSE when done or stopped
	while( Frequency_Loop(state) );

	if (isFlagSet(FREQ_LOOP_STOP))
		goto out;

	// Exit if in batch mode
	if (rc_config.batch_mode)
	{
		batch_complete = TRUE;
		goto out;
	}

	/*
		Prevent deadlock waiting for Stop_Frequency_Loop()=>pthread_join()
		in Open_Input_File() triggered by Optimizer_Output() because
		g_idle_add_once_sync won't allow this Frequency_Loop_Thread()
		thread to exit until Open_Input_File() returns for GTK to make
		progress, but Open_Input_File() is waiting for pthread_join()
		to return when this thread exits.
	*/
	if ( isFlagSet(INPUT_PENDING) )
		goto out;

out:
	ClearFlag(FREQ_LOOP_RUNNING);

	if( batch_complete )
		g_idle_add_once((GSourceOnceFunc)batch_capture_and_quit, NULL);

	return NULL;
}


/**
 * freq_loop_start_internal - allocate state and launch the loop thread
 *
 * Caller has already invalidated the desired save.fstep[] entries.
 * Returns TRUE on success, FALSE if preconditions are not met.
 */
static gboolean
freq_loop_start_internal( void )
{
  if( calc_data.freq_loop_data == NULL )
    return FALSE;

  if( isFlagSet(FREQ_LOOP_RUNNING) ||
      calc_data.FR_cards < 1       ||
      calc_data.steps_total < 1 )
    return FALSE;

  /* Join previous thread if it exited naturally but was never joined.
   * Stop_Frequency_Loop is idempotent and no-ops when pth_freq_loop is NULL. */
  Stop_Frequency_Loop();

  /* Re-check: the GTK event flush inside Stop_Frequency_Loop may have
   * re-entrantly started a new sweep via eval_apply_and_reload. */
  if( isFlagSet(FREQ_LOOP_RUNNING) )
    return FALSE;

  mem_new(&floop_state);
  floop_state->idle_top = -1;
  mem_array_alloc(&floop_state->idle_stack, calc_data.num_jobs);

  /* Populate the display extent on the GTK thread before the sweep worker
   * runs freq_populate_steps; green-line classification reads the extent via
   * fmhz_within_display_range and the worker must only read these fields. */
  freqplots_update_fscale_extents();

  SetFlag( FREQ_LOOP_INIT );
  SetFlag( FREQ_LOOP_RUNNING );

  /* Intermediate-step draws use force=FALSE and are gated by
   * SUPPRESS_INTERMEDIATE_REDRAWS inside xnec2_widget_queue_draw. */

  if( !rc_config.disable_pthread_freqloop )
  {
    mem_new(&pth_freq_loop);
    int ret = pthread_create( pth_freq_loop, NULL, Frequency_Loop_Thread, floop_state );
    if( ret != 0 )
    {
      mem_free(&pth_freq_loop);
      pr_crit("failed to start Frequency_Loop_Thread\n");
      perror( "pthread_create()" );
      exit( -1 );
    }
  }
  else
  {
    floop_tag = g_idle_add( Frequency_Loop, floop_state );
  }

  return TRUE;
}

/**
 * Start_Frequency_Loop - invalidate all steps and start a full sweep
 */
gboolean
Start_Frequency_Loop( void )
{
  if( save.fstep == NULL || calc_data.steps_total < 1 )
    return FALSE;

  g_rec_mutex_lock(&freq_data_lock);
  for( int i = 0; i <= calc_data.steps_total; i++ )
    save.fstep[i] = 0;
  g_rec_mutex_unlock(&freq_data_lock);

  return freq_loop_start_internal();
}

/**
 * Start_Frequency_Loop_Greenline - recompute only the green-line step
 *
 * Invalidates save.fstep[steps_total] so the dispatch loop recomputes
 * that slot.  Sweep steps 0..steps_total-1 remain valid.
 */
gboolean
Start_Frequency_Loop_Greenline( void )
{
  if( calc_data.fmhz_save <= 0.0 || save.fstep == NULL || calc_data.steps_total < 1 )
    return FALSE;

  g_rec_mutex_lock(&freq_data_lock);
  save.fstep[calc_data.steps_total] = 0;
  g_rec_mutex_unlock(&freq_data_lock);

  return freq_loop_start_internal();
}

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
		  mem_free(&pth_freq_loop);

		  if( floop_state != NULL )
		  {
		    mem_array_free(&floop_state->idle_stack);
		    mem_free(&floop_state);
		  }
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
	ClearFlag(FREQ_LOOP_STOP);

	/* Both paths free state here; g_idle source was removed above */
	if( floop_state != NULL )
	{
	  mem_array_free(&floop_state->idle_stack);
	  mem_free(&floop_state);
	}
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

