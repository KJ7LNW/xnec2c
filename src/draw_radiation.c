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

#include "draw_radiation.h"
#include "shared.h"

/* For coloring rad pattern */
static double *red = NULL, *grn = NULL, *blu = NULL;

/* Buffered points in 3d (xyz) space
 * forming the radiation pattern */
static point_3d_t *point_3d = NULL;

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

/* Draw_Radiation_Pattern()
 *
 * Draws the radiation pattern as a frame of line
 * segmants joining the points defined by spherical
 * co-ordinates theta, phi and r = gain(theta, phi)
 */
  static void
Draw_Radiation_Pattern( cairo_t *cr )
{
  /* Abort if rad pattern cannot be drawn */
  if( isFlagClear(ENABLE_RDPAT) || (calc_data.fstep < 0) )
	return;

  /* Line segments to draw on Screen */
  Segment_t segm;

  int
	idx,
	nth,     /* Theta step count */
	nph,     /* Phi step count   */
	col_idx, /* Index to rad pattern color buffers */
	pts_idx; /* Index to rad pattern 3d-points buffer */


  /* Frequency step and polarization type */
  int fstep, pol;

  /* Theta and phi angles defining a rad pattern point
   * and distance of its projection from xyz origin */
  double theta, phi, r, r_min, r_range;

  /* theta and phi step in rads */
  double dth = (double)fpat.dth * (double)TORAD;
  double dph = (double)fpat.dph * (double)TORAD;

  /* Used to set text in labels */
  gchar txt[8];

  fstep = calc_data.fstep;
  pol   = calc_data.pol_type;

  /* Change drawing if newer rad pattern data */
  if( isFlagSet(DRAW_NEW_RDPAT) )
  {
	size_t mreq = ((size_t)(fpat.nth * fpat.nph)) * sizeof(point_3d_t);
	mem_realloc( (void **)&point_3d, mreq, "in draw_radiation.c" );
	mreq = (size_t)((fpat.nth-1) * fpat.nph + (fpat.nph-1) * fpat.nth);
	mreq *= sizeof(double);
	mem_realloc( (void **)&red, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&grn, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&blu, mreq, "in draw_radiation.c" );

	ClearFlag( DRAW_NEW_RDPAT );

	/* Distance of rdpattern point furthest from xyz origin */
	idx = rad_pattern[fstep].max_gain_idx[pol];
	rdpattern_proj_params.r_max = Scale_Gain(
		rad_pattern[fstep].gtot[idx], fstep, idx);

	/* Distance of rdpattern point nearest to xyz origin */
	idx = rad_pattern[fstep].min_gain_idx[pol];
	r_min = Scale_Gain(
		rad_pattern[fstep].gtot[idx], fstep, idx);

	/* Range of scaled rdpattern gain values */
	r_range = rdpattern_proj_params.r_max - r_min;

	/* Set radiation pattern projection parametrs */
	New_Projection_Parameters(
		rdpattern_width,
		rdpattern_height,
		&rdpattern_proj_params );

	/*** Convert radiation pattern values
	 * to points in 3d space in x,y,z axis ***/
	pts_idx = 0;
	phi = (double)(fpat.phis * TORAD); /* In rads */

	/* Step phi angle */
	for( nph = 0; nph < fpat.nph; nph++ )
	{
	  theta = (double)(fpat.thets * TORAD); /* In rads */

	  /* Step theta angle */
	  for( nth = 0; nth < fpat.nth; nth++ )
	  {
		/* Distance of pattern point from the xyz origin */
		r = Scale_Gain( rad_pattern[fstep].gtot[pts_idx], fstep, pts_idx );

		/* Distance of pattern point from xyz origin */
		point_3d[pts_idx].r = r;

		/* Distance of point's projection on xyz axis, from origin */
		point_3d[pts_idx].z = r * cos(theta);
		r *= sin(theta);
		point_3d[pts_idx].x = r * cos(phi);
		point_3d[pts_idx].y = r * sin(phi);

		/* Step theta in rads */
		theta += dth;

		/* Step 3d points index */
		pts_idx++;
	  } /* for( nth = 0; nth < fpat.nth; nth++ ) */

	  /* Step phi in rads */
	  phi += dph;
	} /* for( nph = 0; nph < fpat.nph; nph++ ) */

	/* Calculate RGB value for rad pattern seg.
	 * The average gain value of the two points
	 * marking each line segment is used here */

	/* Pattern segment color in theta direction */
	col_idx = pts_idx = 0;
	for( nph = 0; nph < fpat.nph; nph++ )
	{
	  for( nth = 1; nth < fpat.nth; nth++ )
	  {
		Value_to_Color(
			&red[col_idx], &grn[col_idx], &blu[col_idx],
			(point_3d[pts_idx].r+point_3d[pts_idx+1].r)/2.0-r_min,
			r_range );
		col_idx++;
		pts_idx++;

	  } /* for( nph = 0; nph < fpat.nph; nph++ ) */

	  /* Needed because of "index look-ahead" above */
	  pts_idx++;

	} /* for( nth = 1; nth < fpat.nth; nth++ ) */

	/* Pattern segment color in phi direction */
	for( nth = 0; nth < fpat.nth; nth++ )
	{
	  pts_idx = nth;
	  for( nph = 1; nph < fpat.nph; nph++ )
	  {
		Value_to_Color(
			&red[col_idx], &grn[col_idx], &blu[col_idx],
			(point_3d[pts_idx].r +
			 point_3d[pts_idx+fpat.nth].r)/2.0-r_min,
			r_range );
		col_idx++;

		/* Needed because of "index look-ahead" above */
		pts_idx += fpat.nth;

	  } /* for( nth = 0; nth < fpat.nth; nth++ ) */
	} /* for( nph = 1; nph < fpat.nph; nph++ ) */

	/* Show max gain on color code bar */
	snprintf( txt, 8, "%6.1f", rad_pattern[fstep].max_gain[pol] );
	gtk_label_set_text(	GTK_LABEL(Builder_Get_Object(
			rdpattern_window_builder, "rdpattern_colorcode_maxlabel")),
		txt );

	/* Show min gain on color code bar */
	snprintf( txt, 6, "%4.1f", rad_pattern[fstep].min_gain[pol] );
	gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
			rdpattern_window_builder, "rdpattern_colorcode_minlabel")),
		txt );

  } /* if( isFlagSet(DRAW_NEWRDPAT) ) ) */

  /* Draw xyz axes to Screen */
  Draw_XYZ_Axes( cr, rdpattern_proj_params );

  /* Overlay structure on Near Field pattern */
  if( isFlagSet(OVERLAY_STRUCT) )
  {
	/* Save structure projection params pointers */
	projection_parameters_t params = structure_proj_params;

	/* Divert structure drawing to rad pattern area */
	structure_proj_params = rdpattern_proj_params;
	structure_proj_params.r_max = params.r_max;
	structure_proj_params.xy_scale =
	  params.xy_scale1 * rdpattern_proj_params.xy_zoom;

	/* Process and draw geometry if enabled */
	Process_Wire_Segments();
	Process_Surface_Patches();
	Draw_Surface_Patches( cr, structure_segs+data.n, data.m );
	Draw_Wire_Segments( cr, structure_segs, data.n );

	/* Restore structure projection params */
	structure_proj_params = params;

  } /* if( isFlagSet(OVERLAY_STRUCT) ) */

  /*** Draw rad pattern on screen ***/
  /* Draw segments along theta direction */
  col_idx = pts_idx = 0;
  for( nph = 0; nph < fpat.nph; nph++ )
  {
	for( nth = 1; nth < fpat.nth; nth++ )
	{
	  /* Project line segment to Screen */
	  Set_Gdk_Segment(
		  &segm,
		  &rdpattern_proj_params,
		  point_3d[pts_idx].x,
		  point_3d[pts_idx].y,
		  point_3d[pts_idx].z,
		  point_3d[pts_idx+1].x,
		  point_3d[pts_idx+1].y,
		  point_3d[pts_idx+1].z );
	  pts_idx++;

	  /* Draw segment */
	  cairo_set_source_rgb( cr, red[col_idx], grn[col_idx], blu[col_idx] );
	  Cairo_Draw_Line( cr, segm.x1, segm.y1, segm.x2, segm.y2 );
	  col_idx++;

	} /* for( nth = 1; nth < fpat.nth; nth++ ) */

	pts_idx++;
  } /* for( nph = 0; nph < fpat.nph; nph++ ) */

  /* Draw segments along phi direction */
  for( nth = 0; nth < fpat.nth; nth++ )
  {
	pts_idx = nth;
	for( nph = 1; nph < fpat.nph; nph++ )
	{
	  /* Project line segment to Screen */
	  Set_Gdk_Segment(
		  &segm,
		  &rdpattern_proj_params,
		  point_3d[pts_idx].x,
		  point_3d[pts_idx].y,
		  point_3d[pts_idx].z,
		  point_3d[pts_idx+fpat.nth].x,
		  point_3d[pts_idx+fpat.nth].y,
		  point_3d[pts_idx+fpat.nth].z );

	  /* Draw segment */
	  cairo_set_source_rgb( cr, red[col_idx], grn[col_idx], blu[col_idx] );
	  Cairo_Draw_Line( cr, segm.x1, segm.y1, segm.x2, segm.y2 );
	  col_idx++;

	  /* Needed because drawing segments "looks ahead"
	   * in the 3d points buffer in the above loop */
	  pts_idx += fpat.nth;

	} /* for( nph = 1; nph < fpat.nph; nph++ ) */
  } /* for( nth = 0; nth < fpat.nth; nth++ ) */

  /* Show gain in direction of viewer */
  Show_Viewer_Gain(
	  rdpattern_window_builder,
	  "rdpattern_viewer_gain",
	  rdpattern_proj_params );

} /* Draw_Radiation_Pattern() */

/*-----------------------------------------------------------------------*/

/* Draw_Near_Field()
 *
 * Draws near E/H fields and Poynting vector
 */
  static void
Draw_Near_Field( cairo_t *cr )
{
  int idx, npts; /* Number of points to plot */
  double
	fx, fy, fz,	/* Co-ordinates of "free" end of field lines */
	fscale;		/* Scale factor for equalizing field line segments */

  /* Scale factor ref, for normalizing field strength values */
  static double dr;

  /* Co-ordinates of Poynting vectors */
  static double *pov_x = NULL, *pov_y = NULL;
  static double *pov_z = NULL, *pov_r = NULL;

  /* Range of Poynting vector values,
   * its max and min and log of max/min */
  static double pov_max = 0, max;

  /* Used to set text in labels */
  gchar txt[9];

  /* Line segments to draw on Screen */
  Segment_t segm;

  /* For coloring field lines */
  double xred = 0.0, xgrn = 0.0, xblu = 0.0;

  /* Abort if drawing a near field pattern is not possible */
  if( isFlagClear(ENABLE_NEAREH) || !near_field.valid )
	return;

  /* Initialize projection parameters */
  if( isFlagSet(DRAW_NEW_EHFIELD) )
  {
	/* Reference for scale factor used in
	 * normalizing field strength values */
	if( fpat.near ) /* Spherical co-ordinates */
	  dr = (double)fpat.dxnr;
	else /* Rectangular co-ordinates */
	  dr = sqrt(
		  (double)fpat.dxnr * (double)fpat.dxnr +
		  (double)fpat.dynr * (double)fpat.dynr +
		  (double)fpat.dznr * (double)fpat.dznr )/1.75;

	/* Set radiation pattern projection parametrs */
	/* Distance of field point furthest from xyz origin */
	rdpattern_proj_params.r_max = near_field.r_max + dr;
	New_Projection_Parameters(
		rdpattern_width,
		rdpattern_height,
		&rdpattern_proj_params );

	ClearFlag( DRAW_NEW_EHFIELD );

  } /* if( isFlagSet( DRAW_NEW_EHFIELD ) */

  /* Draw xyz axes to Screen */
  Draw_XYZ_Axes( cr, rdpattern_proj_params );

  /* Overlay structure on Near Field pattern */
  if( isFlagSet(OVERLAY_STRUCT) )
  {
	/* Save projection params pointers */
	projection_parameters_t params = structure_proj_params;

	/* Divert structure drawing to rad pattern area */
	structure_proj_params = rdpattern_proj_params;

	/* Process and draw geometry if enabled */
	Process_Wire_Segments();
	Process_Surface_Patches();
	Draw_Surface_Patches( cr, structure_segs+data.n, data.m );
	Draw_Wire_Segments( cr, structure_segs, data.n );

	/* Restore structure params */
	structure_proj_params = params;
  } /* if( isFlagSet(OVERLAY_STRUCT) ) */

  /* Step thru near field values */
  npts = fpat.nrx * fpat.nry * fpat.nrz;
  for( idx = 0; idx < npts; idx++ )
  {
	/*** Draw Near E Field ***/
	if( isFlagSet(DRAW_EFIELD) && (fpat.nfeh & NEAR_EFIELD) )
	{
	  /* Set gc attributes for segment */
	  Value_to_Color( &xred, &xgrn, &xblu,
		  near_field.er[idx], near_field.max_er );

	  /* Scale factor for each field point, to make
	   * near field direction lines equal-sized */
	  fscale = dr / near_field.er[idx];

	  /* Scaled field values are used to set one end of a
	   * line segment that represents direction of field.
	   * The other end is set by the field point co-ordinates */
	  fx = near_field.px[idx] + near_field.erx[idx] * fscale;
	  fy = near_field.py[idx] + near_field.ery[idx] * fscale;
	  fz = near_field.pz[idx] + near_field.erz[idx] * fscale;

	  /* Project new line segment of
	   * phi chain to the Screen */
	  Set_Gdk_Segment(
		  &segm, &rdpattern_proj_params,
		  near_field.px[idx], near_field.py[idx], near_field.pz[idx],
		  fx, fy, fz );

	  /* Draw segment */
	  cairo_set_source_rgb( cr, xred, xgrn, xblu );
	  Cairo_Draw_Line( cr, segm.x1, segm.y1, segm.x2, segm.y2 );

	} /* if( isFlagSet(DRAW_EFIELD) && (fpat.nfeh & NEAR_EFIELD) ) */

	/*** Draw Near H Field ***/
	if( isFlagSet(DRAW_HFIELD) && (fpat.nfeh & NEAR_HFIELD) )
	{
	  /* Set gc attributes for segment */
	  Value_to_Color( &xred, &xgrn, &xblu,
		  near_field.hr[idx], near_field.max_hr );

	  /* Scale factor for each field point, to make
	   * near field direction lines equal-sized */
	  fscale = dr / near_field.hr[idx];

	  /* Scaled field values are used to set one end of a
	   * line segment that represents direction of field.
	   * The other end is set by the field point co-ordinates */
	  fx = near_field.px[idx] + near_field.hrx[idx] * fscale;
	  fy = near_field.py[idx] + near_field.hry[idx] * fscale;
	  fz = near_field.pz[idx] + near_field.hrz[idx] * fscale;

	  /* Project new line segment of
	   * phi chain to the Screen */
	  Set_Gdk_Segment(
		  &segm, &rdpattern_proj_params,
		  near_field.px[idx], near_field.py[idx], near_field.pz[idx],
		  fx, fy, fz );

	  /* Draw segment */
	  cairo_set_source_rgb( cr, xred, xgrn, xblu );
	  Cairo_Draw_Line( cr, segm.x1, segm.y1, segm.x2, segm.y2 );

	} /* if( isFlagSet(DRAW_HFIELD) && (fpat.nfeh & NEAR_HFIELD) ) */

	/*** Draw Poynting Vector ***/
	if( isFlagSet(DRAW_POYNTING)  &&
		(fpat.nfeh & NEAR_EFIELD) &&
		(fpat.nfeh & NEAR_HFIELD) )
	{
	  int ipv; /* Mem request and index */
	  static size_t mreq = 0;

	  /* Allocate on new near field matrix size */
	  if( !mreq || isFlagSet(ALLOC_PNTING_BUFF) )
	  {
		mreq = (size_t)npts * sizeof( double );
		mem_realloc( (void **)&pov_x, mreq, "in draw_radiation.c" );
		mem_realloc( (void **)&pov_y, mreq, "in draw_radiation.c" );
		mem_realloc( (void **)&pov_z, mreq, "in draw_radiation.c" );
		mem_realloc( (void **)&pov_r, mreq, "in draw_radiation.c" );
		ClearFlag( ALLOC_PNTING_BUFF );
	  }

	  /* Calculate Poynting vector and its max and min */
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

	  /* Set gc attributes for segment */
	  Value_to_Color( &xred, &xgrn, &xblu, pov_r[idx], pov_max );

	  /* Scale factor for each field point, to make
	   * near field direction lines equal-sized */
	  fscale = dr / pov_r[idx];

	  /* Scaled field values are used to set one end of a
	   * line segment that represents direction of field.
	   * The other end is set by the field point co-ordinates */
	  fx = near_field.px[idx] + pov_x[idx] * fscale;
	  fy = near_field.py[idx] + pov_y[idx] * fscale;
	  fz = near_field.pz[idx] + pov_z[idx] * fscale;

	  /* Project new line segment of
	   * Poynting vector to the Screen */
	  Set_Gdk_Segment(
		  &segm,
		  &rdpattern_proj_params,
		  near_field.px[idx], near_field.py[idx],
		  near_field.pz[idx], fx, fy, fz );

	  /* Draw segment */
	  cairo_set_source_rgb( cr, xred, xgrn, xblu );
	  Cairo_Draw_Line( cr, segm.x1, segm.y1, segm.x2, segm.y2 );

	} /* if( isFlagSet(DRAW_POYNTING) ) */

  } /* for( idx = 0; idx < npts; idx++ ) */

  if( isFlagSet(NEAREH_ANIMATE) )
  {
	return;
  }

  /* Show max field strength on color code bar */
  if( isFlagSet(DRAW_EFIELD) )
	max = near_field.max_er;
  else if( isFlagSet(DRAW_HFIELD) )
	max = near_field.max_hr;
  else if( isFlagSet(DRAW_POYNTING) )
	max = pov_max;

  snprintf( txt, sizeof(txt), "%8.2E", max );
  gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
		  rdpattern_window_builder, "rdpattern_colorcode_maxlabel")),
	  txt );

  /* Show min field strength on color code bar */
  gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
		  rdpattern_window_builder, "rdpattern_colorcode_minlabel")),
	  "0" );

} /* Draw_Near_Field() */

/*-----------------------------------------------------------------------*/

/* Draw_Radiation()
 *
 * Draws the radiation pattern or near E/H fields
 */
  void
Draw_Radiation( cairo_t *cr )
{
  /* Abort if xnec2c may be quit by user */
  if( isFlagSet(MAIN_QUIT) ||
	  isFlagClear(ENABLE_EXCITN) )
	return;

  /* Clear drawingarea */
  cairo_set_source_rgb( cr, BLACK );
  cairo_rectangle(
	  cr, 0.0, 0.0,
	  (double)rdpattern_proj_params.width,
	  (double)rdpattern_proj_params.height);
  cairo_fill( cr );

  /* Draw rad pattern or E/H fields */
  if( isFlagSet(DRAW_GAIN) )
	Draw_Radiation_Pattern( cr );
  else if( isFlagSet(DRAW_EHFIELD) )
	  Draw_Near_Field( cr );

  /* Display frequency step */
  Display_Fstep( rdpattern_fstep_entry, calc_data.fstep );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

} /* Draw_Radiation() */

/*-----------------------------------------------------------------------*/

  gboolean
Animate_Near_Field( gpointer udata )
{
  /* omega*t = 2*pi*f*t = Time-varying phase of excitation */
  static double wt = 0.0;
  int idx, npts;

  if( isFlagClear(NEAREH_ANIMATE) )
	return( FALSE );

  /* Number of points in near fields */
  npts = fpat.nrx * fpat.nry * fpat.nrz;
  for( idx = 0; idx < npts; idx++ )
  {
	if( isFlagSet(DRAW_EFIELD) || isFlagSet(DRAW_POYNTING) )
	{
	  /* Real component of complex E field strength */
	  near_field.erx[idx] = near_field.ex[idx] *
		cos( wt + near_field.fex[idx] );
	  near_field.ery[idx] = near_field.ey[idx] *
		cos( wt + near_field.fey[idx] );
	  near_field.erz[idx] = near_field.ez[idx] *
		cos( wt + near_field.fez[idx] );

	  /* Near total electric field vector */
	  near_field.er[idx]  = sqrt(
		  near_field.erx[idx] * near_field.erx[idx] +
		  near_field.ery[idx] * near_field.ery[idx] +
		  near_field.erz[idx] * near_field.erz[idx] );
	  if( near_field.max_er < near_field.er[idx] )
		near_field.max_er = near_field.er[idx];
	}

	if( isFlagSet(DRAW_HFIELD) || isFlagSet(DRAW_POYNTING) )
	{
	  /* Real component of complex H field strength */
	  near_field.hrx[idx] = near_field.hx[idx] *
		cos( wt + near_field.fhx[idx] );
	  near_field.hry[idx] = near_field.hy[idx] *
		cos( wt + near_field.fhy[idx] );
	  near_field.hrz[idx] = near_field.hz[idx] *
		cos( wt + near_field.fhz[idx] );

	  /* Near total magnetic field vector*/
	  near_field.hr[idx]  = sqrt(
		  near_field.hrx[idx] * near_field.hrx[idx] +
		  near_field.hry[idx] * near_field.hry[idx] +
		  near_field.hrz[idx] * near_field.hrz[idx] );
	  if( near_field.max_hr < near_field.hr[idx] )
		near_field.max_hr = near_field.hr[idx];
	}

  } /* for( idx = 0; idx < npts; idx++ ) */

  /* Increment excitation phase, keep < 2pi */
  wt += near_field.anim_step;
  if( wt >= (double)M_2PI )
	wt = 0.0;

  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( rdpattern_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );

  return( TRUE );

} /* Animate_Near_Field() */

/*-----------------------------------------------------------------------*/

/* Polarization_Factor()
 *
 * Calculates polarization factor from axial
 * ratio and tilt of polarization ellipse
 */
  double
Polarization_Factor( int pol_type, int fstep, int idx )
{
  double axrt, axrt2, tilt2, polf = 1.0;

  switch( pol_type )
  {
	case POL_TOTAL:
	  polf = 1.0;
	  break;

	case POL_HORIZ:
	  axrt2  = rad_pattern[fstep].axrt[idx];
	  axrt2 *= axrt2;
	  tilt2  = sin( rad_pattern[fstep].tilt[idx] );
	  tilt2 *= tilt2;
	  polf = (axrt2 + (1.0 - axrt2) * tilt2) / (1.0 + axrt2);
	  break;

	case POL_VERT:
	  axrt2  = rad_pattern[fstep].axrt[idx];
	  axrt2 *= axrt2;
	  tilt2  = cos( rad_pattern[fstep].tilt[idx] );
	  tilt2 *= tilt2;
	  polf = (axrt2 + (1.0 - axrt2) * tilt2) / (1.0 + axrt2);
	  break;

	case POL_LHCP:
	  axrt  = rad_pattern[fstep].axrt[idx];
	  axrt2 = axrt * axrt;
	  polf  = (1.0 + 2.0 * axrt + axrt2) / 2.0 / (1.0 + axrt2);
	  break;

	case POL_RHCP:
	  axrt  = rad_pattern[fstep].axrt[idx];
	  axrt2 = axrt * axrt;
	  polf  = (1.0 - 2.0 * axrt + axrt2) / 2.0 / (1.0 + axrt2);
  }

  if( polf < 1.0E-200 ) polf = 1.0E-200;
  polf = 10.0 * log10( polf );

  return( polf );
} /* Polarization_Factor() */

/*-----------------------------------------------------------------------*/

/* Set_Polarization()
 *
 * Sets the polarization type of gain to be plotted
 */

  void
Set_Polarization( int pol )
{
  calc_data.pol_type = pol;
  Set_Window_Labels();

  /* Show gain in direction of viewer */
  Show_Viewer_Gain(
	  main_window_builder,
	  "main_gain_entry",
	  structure_proj_params );

  /* Enable redraw of rad pattern */
  SetFlag( DRAW_NEW_RDPAT );

  /* Trigger a redraw of drawingareas */
  if( isFlagSet(DRAW_ENABLED) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

  if( isFlagSet(PLOT_ENABLED) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( freqplots_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

} /* Set_Polarization() */

/*-----------------------------------------------------------------------*/

/* Set_Gain_Style()
 *
 * Sets the radiation pattern Gain scaling style
 */
  void
Set_Gain_Style( int gs )
{
  gain_style = gs;

  Set_Window_Labels();

  /* Trigger a redraw of drawingarea */
  if( isFlagSet(DRAW_ENABLED) )
  {
	/* Enable redraw of rad pattern */
	SetFlag( DRAW_NEW_RDPAT );

	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

} /* Set_Gain_Style() */

/*-----------------------------------------------------------------------*/

/*  New_Radiation_Projection_Angle()
 *
 *  Calculates new projection parameters when a
 *  structure projection angle (Wr or Wi) changes
 */
  void
New_Radiation_Projection_Angle(void)
{
  /* sin and cos of rad pattern rotation and inclination angles */
  rdpattern_proj_params.sin_wr = sin(rdpattern_proj_params.Wr/(double)TODEG);
  rdpattern_proj_params.cos_wr = cos(rdpattern_proj_params.Wr/(double)TODEG);
  rdpattern_proj_params.sin_wi = sin(rdpattern_proj_params.Wi/(double)TODEG);
  rdpattern_proj_params.cos_wi = cos(rdpattern_proj_params.Wi/(double)TODEG);

  /* Trigger a redraw of radiation drawingarea */
  if( isFlagSet(DRAW_ENABLED) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

  /* Trigger a redraw of plots drawingarea if doing "viewer" gain */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(PLOT_GVIEWER) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( freqplots_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

} /* New_Radiation_Projection_Angle() */

/*-----------------------------------------------------------------------*/

/* Redo_Radiation_Pattern()
 *
 * Refreshes radiation pattern on new frequency in spinbutton
 */
  gboolean
Redo_Radiation_Pattern( gpointer udata )
{
  /* Redo radiation pattern for a new frequency. Below
   * makes calcs use the extra buffer in rad_pattern */
  calc_data.fstep = calc_data.nfrq;
  New_Frequency();

  /* Redraw radiation pattern on screen */
  if( isFlagSet(DRAW_ENABLED) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

  return FALSE;

} /* Redo_Radiation_Pattern() */

/*-----------------------------------------------------------------------*/

/* Viewer_Gain()
 *
 * Calculate gain in direction of viewer
 * (e.g. Perpenticular to the Screen)
 */
  double
Viewer_Gain( projection_parameters_t proj_parameters, int fstep )
{
  double phi, gain;
  int nth, nph, idx;

  /* Calculate theta step from proj params */
  phi = proj_parameters.Wr;
  if( fpat.dth == 0.0 ) nth = 0;
  else
  {
	double theta;
	theta = fabs( 90.0 - proj_parameters.Wi );
	if( theta > 180.0 )
	{
	  theta = 360.0 - theta;
	  phi  -= 180.0;
	}

	if( (gnd.ksymp == 2) &&
		(theta > 90.01)  &&
		(gnd.ifar != 1) )
	  return( -999.99 );

	nth = (int)( (theta - fpat.thets) / fpat.dth + 0.5 );
	if( (nth >= fpat.nth) || (nth < 0) )
	  nth = fpat.nth-1;
  }

  /* Calculate phi step from proj params */
  if( fpat.dph == 0.0 ) nph = 0;
  else
  {
	while( phi < 0.0 ) phi += 360.0;
	nph = (int)( (phi - fpat.phis) / fpat.dph + 0.5 );
	if( (nph >= fpat.nph) || (nph < 0) )
	  nph = fpat.nph-1;
  }

  idx = nth + nph * fpat.nth;
  gain = rad_pattern[fstep].gtot[idx] +
		Polarization_Factor(calc_data.pol_type, fstep, idx);
  if( gain < -999.99 ) gain = -999.99;

  return( gain );

} /* Viewer_Gain() */

/*-----------------------------------------------------------------------*/

/* Rdpattern_Window_Killed()
 *
 * Cleans up after the rad pattern window is closed
 */
  void
Rdpattern_Window_Killed( void )
{
  rc_config.rdpattern_width  = 0;
  rc_config.rdpattern_height = 0;

  if( animate_dialog != NULL )
  {
	Gtk_Widget_Destroy( animate_dialog );
	ClearFlag( NEAREH_ANIMATE );
	if( anim_tag ) g_source_remove( anim_tag );
	anim_tag = 0;
  }

  if( isFlagSet(DRAW_ENABLED) )
  {
	ClearFlag( DRAW_FLAGS );
	rdpattern_drawingarea = NULL;
	g_object_unref( rdpattern_window_builder );
	rdpattern_window_builder = NULL;
	Free_Draw_Buffers();

	gtk_check_menu_item_set_active(	GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object( main_window_builder, "main_rdpattern")), FALSE );
  }
  rdpattern_window = NULL;
  kill_window = NULL;

} /* Rdpattern_Window_Killed() */

/*-----------------------------------------------------------------------*/

/* Set_Window_Labels()
 *
 * Sets radiation pattern window labels
 * according to what is being drawn.
 */
  void
Set_Window_Labels( void )
{
  char *pol_type[NUM_POL] =
  {
	_("Total Gain"),
	_("Horizontal Polarization"),
	_("Vertical Polarization"),
	_("RH Circular Polarization"),
	_("LH Circular Polarization")
  };

  char *scale[NUM_SCALES] =
  {
	_("Linear Power"),
	_("Linear Voltage"),
	_("ARRL Scale"),
	_("Logarithmic Scale")
  };

  char txt[64];
  size_t s = sizeof( txt );

  if( isFlagSet(DRAW_ENABLED) )
  {
	/* Set window labels */
	Strlcpy( txt, _("Radiation Patterns"), s );
	if( isFlagSet(DRAW_GAIN) )
	{
	  Strlcpy( txt, _("Radiation Pattern: - "), s );
	  Strlcat( txt, pol_type[calc_data.pol_type], s );
	  Strlcat( txt, " - ", s );
	  Strlcat( txt, scale[gain_style], s );
	}
	else if( isFlagSet(DRAW_EHFIELD) )
	{
	  Strlcpy( txt, _("Near Fields:"), s );
	  if( isFlagSet(DRAW_EFIELD) )
		Strlcat( txt, _(" - E Field"), s );
	  if( isFlagSet(DRAW_HFIELD) )
		Strlcat( txt, _(" - H Field"), s );
	  if( isFlagSet(DRAW_POYNTING) )
		Strlcat( txt, _(" - Poynting Vector"), s );
	}

	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			rdpattern_window_builder, "rdpattern_label")), txt );

  } /* if( isFlagSet(DRAW_ENABLED) ) */

  if( isFlagSet(PLOT_ENABLED) )
  {
	Strlcpy( txt, _("Frequency Data Plots - "), s );
	Strlcat( txt, pol_type[calc_data.pol_type], s );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			freqplots_window_builder, "freqplots_label")), txt );
  }

} /* Set_Window_Labels() */

/*-----------------------------------------------------------------------*/

/* Alloc_Rdpattern_Buffers
 *
 * Allocates memory to the radiation pattern buffers
 */
  void
Alloc_Rdpattern_Buffers( int nfrq, int nth, int nph )
{
  int idx;
  size_t mreq;
  static int last_nfrq = 0;

  /* Free old gain buffers first */
  for( idx = 0; idx < last_nfrq; idx++ )
  {
	free_ptr( (void **)&rad_pattern[idx].gtot );
	free_ptr( (void **)&rad_pattern[idx].max_gain );
	free_ptr( (void **)&rad_pattern[idx].min_gain );
	free_ptr( (void **)&rad_pattern[idx].max_gain_tht );
	free_ptr( (void **)&rad_pattern[idx].max_gain_phi );
	free_ptr( (void **)&rad_pattern[idx].max_gain_idx );
	free_ptr( (void **)&rad_pattern[idx].min_gain_idx );
	free_ptr( (void **)&rad_pattern[idx].axrt );
	free_ptr( (void **)&rad_pattern[idx].tilt );
	free_ptr( (void **)&rad_pattern[idx].sens );
  }
  last_nfrq = nfrq;

  /* Allocate rad pattern buffers */
  mreq = (size_t)nfrq * sizeof(rad_pattern_t);
  mem_realloc( (void **)&rad_pattern, mreq, "in draw_radiation.c" );
  for( idx = 0; idx < nfrq; idx++ )
  {
	/* Memory request for allocs */
	mreq = (size_t)(nph * nth) * sizeof(double);
	rad_pattern[idx].gtot = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].gtot), mreq, "in draw_radiation.c" );
	rad_pattern[idx].axrt = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].axrt), mreq, "in draw_radiation.c" );
	rad_pattern[idx].tilt = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].tilt), mreq, "in draw_radiation.c" );

	mreq = NUM_POL * sizeof(double);
	rad_pattern[idx].max_gain = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].max_gain), mreq, "in draw_radiation.c" );
	rad_pattern[idx].min_gain = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].min_gain), mreq, "in draw_radiation.c" );
	rad_pattern[idx].max_gain_tht = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].max_gain_tht), mreq, "in draw_radiation.c" );
	rad_pattern[idx].max_gain_phi = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].max_gain_phi), mreq, "in draw_radiation.c" );

	mreq = NUM_POL * sizeof(int);
	rad_pattern[idx].max_gain_idx = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].max_gain_idx), mreq, "in draw_radiation.c" );
	rad_pattern[idx].min_gain_idx = NULL;
	mem_alloc( (void **)&(rad_pattern[idx].min_gain_idx), mreq, "in draw_radiation.c" );

	rad_pattern[idx].sens = NULL;
	mreq = (size_t)(nph * nth) * sizeof(int);
	mem_alloc( (void **)&(rad_pattern[idx].sens), mreq, "in draw_radiation.c" );
  }

} /* Alloc_Rdpattern_Buffers() */

/*-----------------------------------------------------------------------*/

/* Alloc_Nearfield_Buffers
 *
 * Allocates memory to the radiation pattern buffers
 */
  void
Alloc_Nearfield_Buffers( int n1, int n2, int n3 )
{
  size_t mreq;

  if( isFlagClear(ALLOC_NEAREH_BUFF) ) return;
  ClearFlag( ALLOC_NEAREH_BUFF );

  /* Memory request for allocations */
  mreq = (size_t)(n1 * n2 * n3) * sizeof( double );

  /* Allocate near field buffers */
  if( fpat.nfeh & NEAR_EFIELD )
  {
	mem_realloc( (void **)&near_field.ex,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.ey,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.ez,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fex, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fey, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fez, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.erx, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.ery, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.erz, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.er,  mreq, "in draw_radiation.c" );
  }

  if( fpat.nfeh & NEAR_HFIELD )
  {
	mem_realloc( (void **)&near_field.hx,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hy,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hz,  mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fhx, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fhy, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.fhz, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hrx, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hry, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hrz, mreq, "in draw_radiation.c" );
	mem_realloc( (void **)&near_field.hr,  mreq, "in draw_radiation.c" );
  }

  mem_realloc( (void **)&near_field.px, mreq, "in draw_radiation.c" );
  mem_realloc( (void **)&near_field.py, mreq, "in draw_radiation.c" );
  mem_realloc( (void **)&near_field.pz, mreq, "in draw_radiation.c" );

} /* Alloc_Nearfield_Buffers() */

/*-----------------------------------------------------------------------*/

/* Free_Draw_Buffers()
 *
 * Frees the buffers used for drawing
 */
  void
Free_Draw_Buffers( void )
{
  free_ptr( (void **)&point_3d );
  free_ptr( (void **)&red );
  free_ptr( (void **)&grn );
  free_ptr( (void **)&blu );
}

/*-----------------------------------------------------------------------*/

