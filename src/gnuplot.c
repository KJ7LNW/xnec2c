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

#include "gnuplot.h"
#include "shared.h"

/*-----------------------------------------------------------------------*/

/* Save_FreqPlots_Gnuplot_Data()
 *
 * Saves frequency plots data to a file for gnuplot
 */
  void
Save_FreqPlots_Gnuplot_Data( char *filename )
{
  /* Abort if plot data not available */
  if( isFlagClear(FREQ_LOOP_DONE) )
	return;

  /* Used to calculate net gain */
  double Zr, Zo, Zi;

  /* Open gplot file, abort on error */
  FILE *fp = NULL;
  if( !Open_File(&fp, filename, "w") )
	return;

  /* Plot max gain vs frequency, if possible */
  if( isFlagSet(PLOT_GMAX) && isFlagSet(ENABLE_RDPAT) )
  {
	int nth, nph, idx, pol;
	gboolean no_fbr;

	double
	  gmax,		/* Max gain buffer */
	  netgain,	/* Viewer direction net gain buffer */
	  gdir_phi,	/* Direction in phi of gain */
	  fbratio;	/* Front to back ratio */

	/* Find max gain and direction, F/B ratio */
	no_fbr = FALSE;
	netgain = 0;

	/* Polarization type and impedance */
	pol = calc_data.pol_type;
	Zo = calc_data.zo;

	/* Save data for all frequency steps that were used */
	fprintf( fp, _("# Gain and F/B Ratio vs Frequency\n") );
	for( idx = 0; idx <= calc_data.lastf; idx++ )
	{
	  double fbdir;
	  int fbidx, mgidx;

	  /* Index to gtot buffer where max gain
	   * occurs for given polarization type */
	  mgidx = rad_pattern[idx].max_gain_idx[pol];

	  /* Max gain for given polarization type */
	  gmax = rad_pattern[idx].gtot[mgidx] +
		Polarization_Factor(pol, idx, mgidx);

	  /* Net gain if selected */
	  if( isFlagSet(PLOT_NETGAIN) )
	  {
		Zr = impedance_data.zreal[idx];
		Zi = impedance_data.zimag[idx];
		netgain = gmax + 10*log10(4*Zr*Zo/(pow(Zr+Zo,2)+pow(Zi,2)));
	  }

	  /* Radiation angle/phi where max gain occurs */
	  gdir_phi = rad_pattern[idx].max_gain_phi[pol];

	  /* Find F/B direction in theta */
	  fbdir = 180.0 - rad_pattern[idx].max_gain_tht[pol];
	  if( fpat.dth == 0.0 )
		nth = 0;
	  else
		nth = (int)( fbdir/fpat.dth + 0.5 );

	  /* If the antenna is modelled over ground, then use the same
	   * theta as the max gain direction, relying on phi alone to
	   * take us to the back. Patch supplied by Rik van Riel AB1KW
	   */
	  if( (nth >= fpat.nth) || (nth < 0) )
	  {
		fbdir = rad_pattern[idx].max_gain_tht[pol];
		if( fpat.dth == 0.0 )
		  nth = 0;
		else
		  nth = (int)( fbdir/fpat.dth + 0.5 );
	  }

	  /* Find F/B direction in phi */
	  fbdir = gdir_phi + 180.0;
	  if( fbdir >= 360.0 ) fbdir -= 360.0;
	  nph = (int)( fbdir/fpat.dph + 0.5 );

	  /* No F/B calc. possible if no phi step at +180 from max gain */
	  if( (nph >= fpat.nph) || (nph < 0) )
		no_fbr = TRUE;

	  /* Index to gtot buffer for gain in back direction */
	  fbidx = nth + nph*fpat.nth;

	  /* Front to back ratio */
	  fbratio  = pow( 10.0, gmax / 10.0 );
	  fbratio /= pow( 10.0,
		  (rad_pattern[idx].gtot[fbidx] +
		   Polarization_Factor(pol, idx, fbidx)) / 10.0 );
	  fbratio = 10.0 * log10( fbratio );

	  if( no_fbr && isFlagClear(PLOT_NETGAIN) ) /* Plot max gain only */
		fprintf( fp, "%13.6E %10.3E\n", save.freq[idx], gmax );
	  else if( isFlagSet(PLOT_NETGAIN) ) /* Plot max gain and net gain */
		fprintf( fp, "%13.6E %10.3E %10.3E\n", save.freq[idx], gmax, netgain );
	  else if( !no_fbr ) /* Plot max gain and F/B ratio */
		fprintf( fp, "%13.6E %10.3E %10.3E\n", save.freq[idx], gmax, fbratio );
	} /* for( idx = 0; idx < calc_data.lastf; idx++ ) */

	/* Plot gain direction in phi and theta */
	if( isFlagSet(PLOT_GAIN_DIR) )
	{
	  fprintf( fp, "\n\n" );
	  fprintf( fp, _("# Direction of gain in theta and phi\n") );
	  for( idx = 0; idx < calc_data.lastf; idx++ )
	  {
		double gdir_tht; /* Direction in theta of gain */

		/* Radiation angle/phi where max gain occurs */
		gdir_tht = 90.0 - rad_pattern[idx].max_gain_tht[pol];
		gdir_phi = rad_pattern[idx].max_gain_phi[pol];
		fprintf( fp, "%13.6E %10.3E %10.3E\n", save.freq[idx], gdir_tht, gdir_phi );
	  } /* for( idx = 0; idx < calc_data.lastf; idx++ ) */
	} /* if( isFlagSet(PLOT_GAIN_DIR) ) */

	fprintf( fp, "\n\n" );
  } /* if( isFlagSet(PLOT_GMAX) && isFlagSet(ENABLE_RDPAT) ) */

  /* Plot VSWR vs freq */
  if( isFlagSet(PLOT_VSWR) )
  {
	int idx;
	double vswr, gamma;
	double zrpro2, zrmro2, zimag2;

	/* Calculate VSWR */
	fprintf( fp, _("# VSWR vs Frequency\n") );
	for(idx = 0; idx <= calc_data.lastf; idx++ )
	{
	  zrpro2 = impedance_data.zreal[idx] + calc_data.zo;
	  zrpro2 *= zrpro2;
	  zrmro2 = impedance_data.zreal[idx] - calc_data.zo;
	  zrmro2 *= zrmro2;
	  zimag2 = impedance_data.zimag[idx] * impedance_data.zimag[idx];
	  gamma = sqrt( (zrmro2 + zimag2)/(zrpro2 + zimag2) );
	  vswr = (1+gamma)/(1-gamma);
	  if( vswr > 10.0 ) vswr = 10.0;
	  fprintf( fp, "%13.6E %10.3E\n", save.freq[idx], vswr );
	}

	fprintf( fp, "\n\n" );
  } /* if( isFlagSet(PLOT_VSWR) ) */

  /* Plot z-real and z-imag */
  if( isFlagSet(PLOT_ZREAL_ZIMAG) )
  {
	int idx;
	fprintf( fp, _("# Z real & Z imaginary vs Frequency\n") );
	for(idx = 0; idx <= calc_data.lastf; idx++ )
	  fprintf( fp, "%13.6E %10.3E %10.3E\n",
		  save.freq[idx], impedance_data.zreal[idx], impedance_data.zimag[idx] );

	fprintf( fp, "\n\n" );
  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  /* Plot z-magn and z-phase */
  if( isFlagSet(PLOT_ZMAG_ZPHASE) )
  {
	int idx;
	fprintf( fp, _("# Z magnitude & Z phase vs Frequency\n") );
	for(idx = 0; idx <= calc_data.lastf; idx++ )
	  fprintf( fp, "%13.6E %10.3E %10.3E\n",
		  save.freq[idx], impedance_data.zmagn[idx], impedance_data.zphase[idx] );
  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  fclose(fp);
} /* Save_FreqPlots_Gnuplot_Data() */

/*-----------------------------------------------------------------------*/

/* Scale_Gain()
 *
 * Scales radiation pattern gain according to selected style
 * ( ARRL style, logarithmic or linear voltage/power )
 */
static int gain_style = GS_LINP;

  static double
Scale_Gain( double gain, int fstep, int idx )
{
  /* Scaled rad pattern gain and pol factor */
  double scaled_rad = 0.0;

  gain += Polarization_Factor( calc_data.pol_type, fstep, idx );

  switch( gain_style )
  {
	case GS_LINP:
	  scaled_rad = pow(10.0, (gain/10.0));
	  break;

	case GS_LINV:
	  scaled_rad = pow(10.0, (gain/20.0));
	  break;

	case GS_ARRL:
	  scaled_rad = exp( 0.058267 * gain );
	  break;

	case GS_LOG:
	  scaled_rad = gain;
	  if( scaled_rad < -40 )
		scaled_rad = 0.0;
	  else
		scaled_rad = scaled_rad /40.0 + 1.0;

  } /* switch( gain_style ) */

  return( scaled_rad );

} /* Scale_Gain() */

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
	fx, fy, fz,	/* Co-ordinates of "free" end of field lines */
	fscale;		/* Scale factor for equalizing field line segments */

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
  if( isFlagSet(ENABLE_RDPAT) && (calc_data.fstep >= 0) )
  {
	int
	  nth,	/* Theta step count */
	  nph;	/* Phi step count   */

	/* Frequency step and polarization type */
	int fstep = calc_data.fstep;

	/* Theta and phi angles defining a rad pattern point
	 * and distance of its projection from xyz origin */
	double theta, phi, r;

	/* theta and phi step in rads */
	double dth = (double)fpat.dth * (double)TORAD;
	double dph = (double)fpat.dph * (double)TORAD;

	/* Open gplot file, abort on error */
	if( !Open_File(&fp, filename, "w") )
	  return;
	fprintf( fp, _("# Radiation Pattern") );

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
		r = Scale_Gain(	rad_pattern[fstep].gtot[idx], fstep, idx );

		/* Distance of point's projection on xyz axis, from origin */
		z = r * cos(theta);
		r *= sin(theta);
		x = r * cos(phi);
		y = r * sin(phi);

		/* Print to file */
		fprintf( fp, "%10.3E %10.3E %10.3E\n", x, y, z );

		/* Step theta in rads */
		theta += dth;
		idx++;
	  } /* for( nth = 0; nth < fpat.nth; nth++ ) */

	  /* Step phi in rads */
	  phi += dph;
	} /* for( nph = 0; nph < fpat.nph; nph++ ) */


  } /* if( isFlagSet(ENABLE_RDPAT) && (calc_data.fstep >= 0) ) */

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
	fprintf( fp, _("# structure patch segmenets\n") );

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
	fprintf( fp, _("# structure wire segmenets\n") );

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

