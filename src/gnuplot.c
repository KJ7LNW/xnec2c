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

#include "gnuplot.h"
#include "shared.h"
#include "prerender/prerender_state.h"
#include "chroma/chroma_nearfield.h"
#include "touchstone.h"

/*-----------------------------------------------------------------------*/

/* Save_FreqPlots_Gnuplot_Data()
 *
 * Saves frequency plots data to a file for gnuplot
 */
void Save_FreqPlots_Gnuplot_Data(char *filename)
{
	FILE *fp = NULL;
	time_t rawtime;
	struct tm *info;
	char buffer[80];

	// Abort if plot data not available
	if (!freq_sweep_complete())
	{
		Notice(GTK_BUTTONS_OK, _("Gnuplot Data"), _("Cannot save: the frequency sweep is incomplete"));
		return;
	}

	// Open gplot file, abort on error 
	if (!Open_File(&fp, filename, "w"))
		return;
	setlocale(LC_NUMERIC, "C");

	time( &rawtime );
	info = localtime( &rawtime );
	strftime(buffer, sizeof(buffer)-1, "%c (%F %H:%M:%S)", info);

	fprintf(fp, "# %s - %s\n", rc_config.input_file, buffer);
	fprintf(fp, _("# Reference impedance Z0 = %.2f Ohm\n"), calc_data.zo);
	fprintf(fp, "#\n");
	fprintf(fp, _("# To plot in gnuplot you can modify this command.  Presently it plots MHz vs zreal and zimag:\n"));
	fprintf(fp, _("#    plot for [i=2:3]  'filename.gplot' using 1:i with lines smooth bezier title columnhead(i);\n"));
	fprintf(fp, "#\n");

	g_rec_mutex_lock(&freq_data_lock);
	meas_write_header_enc(fp, "\t", "\"", "\"");
	meas_write_data(fp, "\t");
	g_rec_mutex_unlock(&freq_data_lock);

	fclose(fp);

	setlocale(LC_NUMERIC, orig_numeric_locale);
}


static void Save_FreqPlots_Touchstone(char *filename, touchstone_type_t type)
{
	FILE *fp = NULL;
	time_t rawtime;
	struct tm *info;
	char buffer[80];
	int idx;

	measurement_t meas;

	if (type < 0 || type >= TOUCHSTONE_COUNT)
	{
		BUG("This should never happen. touchstone type=%d\n", type);
		return;
	}

	// Abort if plot data not available
	if (!freq_sweep_complete())
	{
		Notice(GTK_BUTTONS_OK, _("Touchstone Data"), _("Cannot save: the frequency sweep is incomplete"));
		return;
	}

	/* Every layout writes S11 from the feedpoint impedance, and the
	 * two-port layouts reference S21 to it through the mismatch factor.
	 * The completed sweep above computes every step, so step zero reports
	 * impedance availability for the whole file. */
	if (!meas_has_impedance(0))
	{
		Notice(GTK_BUTTONS_OK, _("Touchstone Data"),
			_("Cannot save: S-parameters need a feedpoint impedance, which this excitation does not provide"));
		return;
	}

	const touchstone_layout_t *layout = &touchstone_layouts[type];

	// Open gplot file, abort on error
	if (!Open_File(&fp, filename, "w"))
		return;
	setlocale(LC_NUMERIC, "C");

	time( &rawtime );
	info = localtime( &rawtime );
	strftime(buffer, sizeof(buffer)-1, "%c (%F %H:%M:%S)", info);

	fprintf(fp, "! %s - %s\n", rc_config.input_file, buffer);
	fprintf(fp, _("! Reference impedance Z0 = %.2f Ohm\n"), calc_data.zo);
	fprintf(fp, "!\n");

	fprintf(fp, "%s", layout->comment);
	fprintf(fp, "# MHz S DB R %g\n", calc_data.zo);

	g_rec_mutex_lock(&freq_data_lock);
	for (idx = 0; idx < calc_data.steps_total; idx++)
	{
		meas_calc(&meas, idx, calc_data.ex_port);
		meas_write_format(&meas, layout->format, fp);
	}
	g_rec_mutex_unlock(&freq_data_lock);

	fclose(fp);

	setlocale(LC_NUMERIC, orig_numeric_locale);
}

void Save_FreqPlots_S1P(char *filename)
{
	Save_FreqPlots_Touchstone(filename, TOUCHSTONE_S1P);
}

void Save_FreqPlots_S2P_Max_Gain(char *filename)
{
	Save_FreqPlots_Touchstone(filename, TOUCHSTONE_S2P_MAXGAIN);
}

void Save_FreqPlots_S2P_Viewer_Gain(char *filename)
{
	Save_FreqPlots_Touchstone(filename, TOUCHSTONE_S2P_VIEWERGAIN);
}

void Save_FreqPlots_CSV(char *filename)
{
	/* Open Optimizer csv file */
	FILE *fp = NULL;

	// Abort if plot data not available
	if (!freq_sweep_complete())
	{
		Notice(GTK_BUTTONS_OK, _("CSV Data"), _("Cannot save: the frequency sweep is incomplete"));
		return;
	}

	if (!Open_File(&fp, filename, "w"))
		return;

	g_rec_mutex_lock(&freq_data_lock);
	meas_write_header(fp, ",");
	meas_write_data(fp, ",");
	g_rec_mutex_unlock(&freq_data_lock);

	fclose(fp);
}


/*-----------------------------------------------------------------------*/

/* Poynting-vector coordinate buffers, reused across
 * Save_RadPattern_Gnuplot_Data() calls. */
static double *pov_x = NULL, *pov_y = NULL, *pov_z = NULL, *pov_r = NULL;

/* gnuplot_data_free()
 *
 * Releases the Poynting-vector coordinate buffers.
 */
  void
gnuplot_data_free( void )
{
  mem_array_free( &pov_x );
  mem_array_free( &pov_y );
  mem_array_free( &pov_z );
  mem_array_free( &pov_r );

} /* gnuplot_data_free() */

/*-----------------------------------------------------------------------*/

/* Save_RadPattern_Gnuplot_Data()
 *
 * Saves radiation pattern data to a file for gnuplot
 */
  void
Save_RadPattern_Gnuplot_Data( char *filename )
{
  int idx, npts; /* Number of points to plot */

  /* Scale factor ref, for normalizing field strength values */
  double dr;

  double
    fx, fy, fz, /* Co-ordinates of "free" end of field lines */
    fscale;     /* Scale factor for equalizing field line segments */

  FILE *fp = NULL;

  int fstep = calc_data.freq_step;

  /* Draw near field pattern if possible */
  if( isFlagSet(ENABLE_NEAREH) && NF_FSTEP_AVAILABLE(fstep) )
  {
    near_field_t *nf = &near_field_fstep[fstep];
    /* Open gplot file, abort on error */
    if( !Open_File(&fp, filename, "w") )
      return;

    setlocale(LC_NUMERIC, "C");

    /* Reference for scale factor used in
     * normalizing field strength values */
    if( fpat.near ) /* Spherical co-ordinates */
      dr = (double)fpat.dxnr;
    /* Rectangular co-ordinates */
    else dr = sqrt(
        (double)fpat.dxnr * (double)fpat.dxnr +
        (double)fpat.dynr * (double)fpat.dynr +
        (double)fpat.dznr * (double)fpat.dznr )/1.75;

    npts = fpat.nrx * fpat.nry * fpat.nrz;

    /*** Draw Near E Field ***/
    if( draw_efield_active() && (fpat.nfeh & NEAR_EFIELD) )
    {
      fprintf( fp, _("# Near E field\n") );
      /* Write e-field out to file [DJS] */
      for( idx = 0; idx < npts; idx++ )
      {
        double er[3];
        double emag = nf_real_vector(&nf->points[idx], NF_CHAN_E,
            FALSE, 0.0, rc_config.nf_static_mode, er);

        fscale = dr / emag;
        fx = nf->points[idx].px + er[0] * fscale;
        fy = nf->points[idx].py + er[1] * fscale;
        fz = nf->points[idx].pz + er[2] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            nf->points[idx].px,
            nf->points[idx].py,
            nf->points[idx].pz,
            fx - nf->points[idx].px,
            fy - nf->points[idx].py,
            fz - nf->points[idx].pz);
      }
    } /* if( draw_efield_active() */

    /*** Draw Near H Field ***/
    if( draw_hfield_active() && (fpat.nfeh & NEAR_HFIELD) )
    {
      fprintf( fp, _("# Near H field\n") );
      /* Write h-field out to file [DJS] */
      for( idx = 0; idx < npts; idx++ )
      {
        double hr[3];
        double hmag = nf_real_vector(&nf->points[idx], NF_CHAN_H,
            FALSE, 0.0, rc_config.nf_static_mode, hr);

        fscale = dr / hmag;
        fx = nf->points[idx].px + hr[0] * fscale;
        fy = nf->points[idx].py + hr[1] * fscale;
        fz = nf->points[idx].pz + hr[2] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            nf->points[idx].px,
            nf->points[idx].py,
            nf->points[idx].pz,
            fx - nf->points[idx].px,
            fy - nf->points[idx].py,
            fz - nf->points[idx].pz);
      }
    } /* if( draw_hfield_active() && (fpat.nfeh & NEAR_HFIELD) ) */

    /*** Draw Poynting Vector ***/
    if( draw_poynting_active()  &&
        (fpat.nfeh & NEAR_EFIELD) &&
        (fpat.nfeh & NEAR_HFIELD) )
    {
      /* Grow the Poynting buffers when the point count exceeds the live
       * capacity; the allocator header is the single capacity record. */
      if( npts > mem_array_capacity(pov_x) )
      {
        mem_array_realloc(&pov_x, npts);
        mem_array_realloc(&pov_y, npts);
        mem_array_realloc(&pov_z, npts);
        mem_array_realloc(&pov_r, npts);
      }

      /* Poynting vector from the static real E and H vectors */
      fprintf( fp, _("# Poynting Vector\n") );
      for( idx = 0; idx < npts; idx++ )
      {
        double er[3], hr[3];

        nf_real_vector(&nf->points[idx], NF_CHAN_E, FALSE, 0.0,
            rc_config.nf_static_mode, er);
        nf_real_vector(&nf->points[idx], NF_CHAN_H, FALSE, 0.0,
            rc_config.nf_static_mode, hr);
        pov_r[idx] = nf_poynting(er, hr, &pov_x[idx], &pov_y[idx], &pov_z[idx]);

        /* Scale factor for each field point, to make
         * near field direction lines equal-sized */
        fscale = dr / pov_r[idx];

        /* Scaled field values are used to set one end of a
         * line segment that represents direction of field.
         * The other end is set by the field point co-ordinates */
        fx = nf->points[idx].px + pov_x[idx] * fscale;
        fy = nf->points[idx].py + pov_y[idx] * fscale;
        fz = nf->points[idx].pz + pov_z[idx] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            nf->points[idx].px,
            nf->points[idx].py,
            nf->points[idx].pz,
            fx - nf->points[idx].px,
            fy - nf->points[idx].py,
            fz - nf->points[idx].pz);
      } /* for( idx = 0; idx < npts; idx++ ) */

    } /* if( draw_poynting_active() ) */
  } /* if( isFlagSet(ENABLE_NEAREH) && NF_FSTEP_AVAILABLE(fstep) ) */

  /* Save radiation pattern data if possible */
  if( isFlagSet(ENABLE_RDPAT) && (calc_data.freq_step >= 0) )
  {
    int
      nth,  /* Theta step count */
      nph;  /* Phi step count   */

    /* Frequency step and polarization type */
    int fstep = calc_data.freq_step;

    /* Theta and phi angles defining a rad pattern point
     * and distance of its projection from xyz origin */
    double theta, phi, r;

    /* theta and phi step in rads */
    double dth = (double)fpat.dth * (double)TORAD;
    double dph = (double)fpat.dph * (double)TORAD;

    /* Open gplot file, abort on error */
    if( !Open_File(&fp, filename, "w") )
      return;

    setlocale(LC_NUMERIC, "C");

    fprintf( fp, _("# Radiation Pattern\n") );

    /* Distance of rdpattern point nearest to xyz origin */
    /*** Convert radiation pattern values
     * to points in 3d space in x,y,z axis ***/
    phi = (double)fpat.phis * (double)TORAD; /* In rads */

    /* Step phi angle */
    idx = 0;
    for( nph = 0; nph < fpat.nph; nph++ )
    {
      theta = (double)fpat.thets * (double)TORAD; /* In rads */

      /* Step theta angle */
      for( nth = 0; nth < fpat.nth; nth++ )
      {
        double x, y, z;

        /* Distance of pattern point from the xyz origin */
        r = Scale_Gain( rad_pattern[fstep].gtot[idx], fstep, idx );

        /* Distance of point's projection on xyz axis, from origin */
        z = r * cos(theta);
        r *= sin(theta);
        x = r * cos(phi);
        y = r * sin(phi);

        /* Print to file */
        fprintf( fp, "%12.3E %12.3E %12.3E\n", x, y, z );

        /* Step theta in rads */
        theta += dth;
        idx++;
      } /* for( nth = 0; nth < fpat.nth; nth++ ) */

      /* Step phi in rads */
      phi += dph;
    } /* for( nph = 0; nph < fpat.nph; nph++ ) */
  } /* if( isFlagSet(ENABLE_RDPAT) && (calc_data.freq_step >= 0) ) */

  setlocale(LC_NUMERIC, orig_numeric_locale);

  if( fp != NULL ) fclose(fp);
} /* Save_RadPattern_Gnuplot_Data() */


void Save_RadPattern_CSV(char *filename)
{
	FILE *fp = NULL;

	if (!Open_File(&fp, filename, "w"))
		return;

	fprintf(fp, "mhz,phi,theta,gain_total,gain_horiz,gain_vert,gain_rhcp,gain_lhcp\n");

	setlocale(LC_NUMERIC, "C");
	// Distance of rdpattern point nearest to xyz origin
	// Convert radiation pattern values to points in 3 d space in x, y, z axis

	double theta, phi, r;

    /* theta and phi step in rads */
    double dth = (double)fpat.dth * (double)TORAD;
    double dph = (double)fpat.dph * (double)TORAD;

	int calc_idx, idx, nph, nth, pol;

	g_rec_mutex_lock(&freq_data_lock);
	if (isFlagSet(ENABLE_RDPAT) && (calc_data.freq_step >= 0))
	{
		for (calc_idx = 0; calc_idx < calc_data.steps_total; calc_idx++)
		{
			// Step phi angle
			idx = 0;
			phi = (double)fpat.phis * (double)TORAD; // In rads

			for (nph = 0; nph < fpat.nph; nph++)
			{
				theta = (double) fpat.thets * (double) TORAD;	// In rads

				// Step theta angle
				for (nth = 0; nth < fpat.nth; nth++)
				{
					// Distance of pattern point from the xyz origin
					r = rad_pattern[calc_idx].gtot[idx];

					// mhz,phi,theta
					fprintf(fp, "%.6f,%.17g,%.17g,",
						save.freq[calc_idx],
						phi * TODEG,
						theta * TODEG);

					// POL_TOTAL, POL_HORIZ, POL_VERT, POL_RHCP, POL_LHCP
					for (pol = 0; pol < NUM_POL; pol++)
						fprintf(fp, "%.17g%s", 
							r + Polarization_Factor(pol, calc_idx, idx),
							(pol == NUM_POL-1 ? "\n" : ",")
							);

					// Step theta in rads
					theta += dth;
					idx++;
				} // for( nth = 0; nth < fpat.nth; nth++ )

				// Step phi in rads
				phi += dph;
			} // for( nph = 0; nph < fpat.nph; nph++ )
		}
	} // if( isFlagSet(ENABLE_RDPAT) && (calc_data.freq_step >= 0) )
	g_rec_mutex_unlock(&freq_data_lock);

	setlocale(LC_NUMERIC, orig_numeric_locale);

	fclose(fp);
}

/*-----------------------------------------------------------------------*/

/* Save_Struct_Gnuplot_Data()
 *
 * Saves antenna structure data for gnuplot
 */
  void
Save_Struct_Gnuplot_Data( char *filename )
{
  FILE *fp = NULL;

  /* Open gplot file, abort on error */
  if( !Open_File(&fp, filename, "w") )
    return;
  setlocale(LC_NUMERIC, "C");

  /* Output if patch segs and no new input pending */
  if( data.m && isFlagClear(INPUT_PENDING) )
  {
    int idx;

    /* Output segments data */
    fprintf( fp, _("# structure patch segments\n") );

    /* Output all patch rectangle edges (4 per patch).
     * Two blank lines after each edge prevent gnuplot splot
     * from connecting non-adjacent points. */
    for( idx = 0; idx < data.m; idx++ )
    {
      patch_corners_t *pc = &geom_pre.patch_corners[idx];

      /* Edge c0-c1 */
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n\n\n",
          pc->c0x, pc->c0y, pc->c0z, pc->c1x, pc->c1y, pc->c1z );

      /* Edge c1-c2 */
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n\n\n",
          pc->c1x, pc->c1y, pc->c1z, pc->c2x, pc->c2y, pc->c2z );

      /* Edge c2-c3 */
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n\n\n",
          pc->c2x, pc->c2y, pc->c2z, pc->c3x, pc->c3y, pc->c3z );

      /* Edge c3-c0 */
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n\n\n",
          pc->c3x, pc->c3y, pc->c3z, pc->c0x, pc->c0y, pc->c0z );
    } /* for( idx = 0; idx < data.m; idx++ ) */

    fprintf( fp, "\n\n" );
  } /* if( data.m && isFlagSet(INPUT_PENDING) ) */

  /* Output if wire segs and no new input pending */
  if( data.n && isFlagClear(INPUT_PENDING) )
  {
    int idx;

    /* Output segments data */
    fprintf( fp, _("# structure wire segments\n") );

    /* Output first segment outside loop to enable separation of wires */
    fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n",
        (double) data.segments[0].x1, (double) data.segments[0].y1,
        (double) data.segments[0].z1,
        (double) data.segments[0].x2, (double) data.segments[0].y2,
        (double) data.segments[0].z2);

    /* Start from second segment and check for connection of ends */
    for( idx = 1; idx < data.n; idx++ )
    {
      /* Leave a 2-line gap to next segment */
      if( (data.segments[idx].icon1 == 0) || (data.segments[idx].icon1 == (idx+1)) )
        fprintf( fp, "\n\n" );
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n",
          (double) data.segments[idx].x1, (double) data.segments[idx].y1,
          (double) data.segments[idx].z1,
          (double) data.segments[idx].x2, (double) data.segments[idx].y2,
          (double) data.segments[idx].z2);
      /* Leave a 2-line gap to next segment */
      if( (data.segments[idx].icon2 == 0) || (data.segments[idx].icon2 == (idx+1)) )
        fprintf( fp, "\n\n" );

    } /* for( idx = 1; idx < data.n; idx++ ) */
  } /* if( data.n && isFlagSet(INPUT_PENDING) ) */

  setlocale(LC_NUMERIC, orig_numeric_locale);
  fclose( fp );

  pr_notice("View with: gnuplot -e \"set terminal qt; splot '%s' with lines; pause mouse close\"\n", filename);
} /* Save_Struct_Gnuplot_Data() */

/*-----------------------------------------------------------------------*/

void Save_Currents_CSV(char *filename)
{
	FILE *fp = NULL;

	int fstep = calc_data.freq_step;
	if( !CRNT_FSTEP_AVAILABLE(fstep) )
	{
		Notice(GTK_BUTTONS_OK, _("Save Currents and Charges to CSV"),
			_("You must enable current data by clicking \"Currents\" or \"Charges\""
			"in the main xnec2c window."));
		return;
	}

	crnt_t *cf = &crnt_fstep[fstep];

	if (!Open_File(&fp, filename, "w"))
		return;

	setlocale(LC_NUMERIC, "C");

	fprintf(fp, "mhz,seg,tag,"
		"current_real,current_imag,current_mag,"  // Current in Amperes
		"charge_real,charge_imag,charge_mag,"     // Charge in Coulombs
		"x1,y1,z1,x2,y2,z2\n");

	int idx;
	double wavelength = data.wlam;
	double charge_scale = 1.0E-6 / calc_data.freq_mhz;

	for (idx = 0; idx < data.n; idx++)
	{
		fprintf(fp, "%.6f,%d,%d,"
			"%.17g,%.17g,%.17g,"  // Current (A)
			"%.17g,%.17g,%.17g,"  // Charge (C)
			"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
			calc_data.freq_mhz,
			idx+1, data.segments[idx].itag,

			// Currents
			creal(cf->cur[idx]) * wavelength,
			cimag(cf->cur[idx]) * wavelength,
			cabs(cf->cur[idx]) * wavelength,

			// Charges
			cf->bir[idx] * charge_scale,
			cf->bii[idx] * charge_scale,
			cabs(cmplx(cf->bir[idx], cf->bii[idx])) * charge_scale,

			// Segment endpoints
			data.segments[idx].x1, data.segments[idx].y1,
			data.segments[idx].z1,
			data.segments[idx].x2, data.segments[idx].y2,
			data.segments[idx].z2);
	}

	setlocale(LC_NUMERIC, orig_numeric_locale);
	fclose(fp);
}


/*-----------------------------------------------------------------------*/

/* Save_Patch_Currents_CSV()
 *
 * Saves surface patch current data to a CSV file
 */
void Save_Patch_Currents_CSV(char *filename)
{
	FILE *fp = NULL;

	int fstep = calc_data.freq_step;
	if( !CRNT_FSTEP_AVAILABLE(fstep) )
	{
		Notice(GTK_BUTTONS_OK, _("Save Patch Currents to CSV"),
			_("You must enable current data by clicking \"Currents\" or \"Charges\" "
			"in the main xnec2c window."));
		return;
	}

	crnt_t *cf = &crnt_fstep[fstep];

	if (data.m == 0)
	{
		Notice(GTK_BUTTONS_OK, _("Save Patch Currents to CSV"),
			_("No surface patches in this model."));
		return;
	}

	if (!Open_File(&fp, filename, "w"))
		return;

	setlocale(LC_NUMERIC, "C");

	fprintf(fp, "mhz,patch,area,"
		"px,py,pz,"
		"cx_real,cx_imag,cy_real,cy_imag,cz_real,cz_imag,"
		"ct1_mag,ct2_mag\n");

	int idx, j;
	complex double cx, cy, cz, ct1, ct2;

	for (idx = 0; idx < data.m; idx++)
	{
		/* Patch current stored as 3 consecutive xyz components starting at data.n */
		j = data.n + idx * 3;
		cx = cf->cur[j];
		cy = cf->cur[j + 1];
		cz = cf->cur[j + 2];

		/* Project current onto t1 and t2 tangent vectors */
		ct1 = cx * (double) data.patches[idx].t1x +
		      cy * (double) data.patches[idx].t1y +
		      cz * (double) data.patches[idx].t1z;
		ct2 = cx * (double) data.patches[idx].t2x +
		      cy * (double) data.patches[idx].t2y +
		      cz * (double) data.patches[idx].t2z;

		fprintf(fp, "%.6f,%d,%.17g,"
			"%.17g,%.17g,%.17g,"
			"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
			"%.17g,%.17g\n",
			calc_data.freq_mhz,
			idx + 1,
			(double) data.patches[idx].pbi,
			(double) data.patches[idx].px,
			(double) data.patches[idx].py,
			(double) data.patches[idx].pz,
			creal(cx), cimag(cx),
			creal(cy), cimag(cy),
			creal(cz), cimag(cz),
			cabs(ct1),
			cabs(ct2));
	}

	setlocale(LC_NUMERIC, orig_numeric_locale);
	fclose(fp);
}
