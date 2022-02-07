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
	if (isFlagClear(FREQ_LOOP_DONE))
	{
		Notice(_("Gnuplot Data"), "Cannot save data while frequency loop is running", GTK_BUTTONS_OK);
		return;
	}

	// Open gplot file, abort on error 
	if (!Open_File(&fp, filename, "w"))
		return;

	time( &rawtime );
	info = localtime( &rawtime );
	strftime(buffer, sizeof(buffer)-1, "%c (%F %H:%M:%S)", info);

	fprintf(fp, "# %s - %s\n", rc_config.input_file, buffer);
	fprintf(fp, _("# Reference impedance Z0 = %.2f Ohm\n"), calc_data.zo);
	fprintf(fp, "#\n");
	fprintf(fp, _("# To plot in gnuplot you can modify this command.  Presently it plots MHz vs zreal and zimag:\n"));
	fprintf(fp, _("#    plot for [i=2:3]  'filename.gplot' using 1:i with lines smooth bezier title columnhead(i);\n"));
	fprintf(fp, "#\n");

	meas_write_header_enc(fp, "\t", "\"", "\"");
	meas_write_data(fp, "\t");

	fclose(fp);
}

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

  /* Draw near field pattern if possible */
  if( isFlagSet(ENABLE_NEAREH) && near_field.valid )
  {
    /* Open gplot file, abort on error */
    if( !Open_File(&fp, filename, "w") )
      return;

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
    if( isFlagSet(DRAW_EFIELD) && (fpat.nfeh & NEAR_EFIELD) )
    {
      fprintf( fp, _("# Near E field\n") );
      /* Write e-field out to file [DJS] */
      for( idx = 0; idx < npts; idx++ )
      {
        fscale = dr / near_field.er[idx];
        fx = near_field.px[idx] + near_field.erx[idx] * fscale;
        fy = near_field.py[idx] + near_field.ery[idx] * fscale;
        fz = near_field.pz[idx] + near_field.erz[idx] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            near_field.px[idx],
            near_field.py[idx],
            near_field.pz[idx],
            fx - near_field.px[idx],
            fy - near_field.py[idx],
            fz - near_field.pz[idx] );
      }
    } /* if( isFlagSet(DRAW_EFIELD) */

    /*** Draw Near H Field ***/
    if( isFlagSet(DRAW_HFIELD) && (fpat.nfeh & NEAR_HFIELD) )
    {
      fprintf( fp, _("# Near H field\n") );
      /* Write h-field out to file [DJS] */
      for( idx = 0; idx < npts; idx++ )
      {
        fscale = dr / near_field.hr[idx];
        fx = near_field.px[idx] + near_field.hrx[idx] * fscale;
        fy = near_field.py[idx] + near_field.hry[idx] * fscale;
        fz = near_field.pz[idx] + near_field.hrz[idx] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            near_field.px[idx],
            near_field.py[idx],
            near_field.pz[idx],
            fx - near_field.px[idx],
            fy - near_field.py[idx],
            fz - near_field.pz[idx] );
      }
    } /* if( isFlagSet(DRAW_HFIELD) && (fpat.nfeh & NEAR_HFIELD) ) */

    /*** Draw Poynting Vector ***/
    if( isFlagSet(DRAW_POYNTING)  &&
        (fpat.nfeh & NEAR_EFIELD) &&
        (fpat.nfeh & NEAR_HFIELD) )
    {
      int ipv;
      static size_t mreq = 0;

      /* Co-ordinates of Poynting vectors */
      static double *pov_x = NULL, *pov_y = NULL;
      static double *pov_z = NULL, *pov_r = NULL;

      /* Range of Poynting vector values,
       * its max and min and log of max/min */
      static double pov_max = 0;

      /* Allocate on new near field matrix size */
      if( !mreq || (mreq != (size_t)npts * sizeof(double)) )
      {
        mreq = (size_t)npts * sizeof( double );
        mem_realloc( (void **)&pov_x, mreq, "in draw_radiation.c" );
        mem_realloc( (void **)&pov_y, mreq, "in draw_radiation.c" );
        mem_realloc( (void **)&pov_z, mreq, "in draw_radiation.c" );
        mem_realloc( (void **)&pov_r, mreq, "in draw_radiation.c" );
      }

      /* Calculate Poynting vector and its max and min */
      fprintf( fp, _("# Poynting Vector\n") );
      for( idx = 0; idx < npts; idx++ )
      {
        pov_max = 0;
        for( ipv = 0; ipv < npts; ipv++ )
        {
          pov_x[ipv] =
            near_field.ery[ipv] * near_field.hrz[ipv] -
            near_field.hry[ipv] * near_field.erz[ipv];
          pov_y[ipv] =
            near_field.erz[ipv] * near_field.hrx[ipv] -
            near_field.hrz[ipv] * near_field.erx[ipv];
          pov_z[ipv] =
            near_field.erx[ipv] * near_field.hry[ipv] -
            near_field.hrx[ipv] * near_field.ery[ipv];
          pov_r[ipv] = sqrt(
              pov_x[ipv] * pov_x[ipv] +
              pov_y[ipv] * pov_y[ipv] +
              pov_z[ipv] * pov_z[ipv] );
          if( pov_max < pov_r[ipv] )
            pov_max = pov_r[ipv];

        } /* for( ipv = 0; ipv < npts; ipv++ ) */

        /* Scale factor for each field point, to make
         * near field direction lines equal-sized */
        fscale = dr / pov_r[idx];

        /* Scaled field values are used to set one end of a
         * line segment that represents direction of field.
         * The other end is set by the field point co-ordinates */
        fx = near_field.px[idx] + pov_x[idx] * fscale;
        fy = near_field.py[idx] + pov_y[idx] * fscale;
        fz = near_field.pz[idx] + pov_z[idx] * fscale;

        /* Print as x, y, z, dx, dy, dz for gnuplot */
        fprintf( fp, "%f %f %f %f %f %f\n",
            near_field.px[idx],
            near_field.py[idx],
            near_field.pz[idx],
            fx - near_field.px[idx],
            fy - near_field.py[idx],
            fz - near_field.pz[idx] );
      } /* for( idx = 0; idx < npts; idx++ ) */

    } /* if( isFlagSet(DRAW_POYNTING) ) */
  } /* if( isFlagSet(ENABLE_NEAREH) && near_field.valid ) */

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

  if( fp != NULL ) fclose(fp);
} /* Save_RadPattern_Gnuplot_Data() */

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

  /* Output if patch segs and no new input pending */
  if( data.m && isFlagClear(INPUT_PENDING) )
  {
    int idx, m2;

    /* Output segments data */
    fprintf( fp, _("# structure patch segments\n") );

    /* Output first segment outside loop to enable separation of wires */
    fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n",
        (double)data.px1[0], (double)data.py1[0], (double)data.pz1[0],
        (double)data.px2[0], (double)data.py2[0], (double)data.pz2[0] );

    /* Start from second segment and check for connection of ends */
    m2 = data.m * 2;
    for( idx = 1; idx < m2; idx++ )
    {
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n",
          (double)data.px1[idx], (double)data.py1[idx], (double)data.pz1[idx],
          (double)data.px2[idx], (double)data.py2[idx], (double)data.pz2[idx] );
    } /* for( idx = 1; idx < m2; idx++ ) */

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
        (double)data.x1[0], (double)data.y1[0], (double)data.z1[0],
        (double)data.x2[0], (double)data.y2[0], (double)data.z2[0] );

    /* Start from second segment and check for connection of ends */
    for( idx = 1; idx < data.n; idx++ )
    {
      /* Leave a 2-line gap to next segment */
      if( (data.icon1[idx] == 0) || (data.icon1[idx] == (idx+1)) )
        fprintf( fp, "\n\n" );
      fprintf( fp, "%10.3E %10.3E %10.3E\n%10.3E %10.3E %10.3E\n",
          (double)data.x1[idx], (double)data.y1[idx], (double)data.z1[idx],
          (double)data.x2[idx], (double)data.y2[idx], (double)data.z2[idx] );
      /* Leave a 2-line gap to next segment */
      if( (data.icon2[idx] == 0) || (data.icon2[idx] == (idx+1)) )
        fprintf( fp, "\n\n" );

    } /* for( idx = 1; idx < data.n; idx++ ) */
  } /* if( data.n && isFlagSet(INPUT_PENDING) ) */

  fclose( fp );
} /* Save_Struct_Gnuplot_Data() */

/*-----------------------------------------------------------------------*/

