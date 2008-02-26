/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2006 N. Kyriazis <neoklis<at>mailspeed.net>
 *
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

/*
 *  draw_structure.c
 *
 *  Structure drawing routines for xnec2c
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "xnec2c.h"
#include "support.h"

/* Current data */
extern crnt_t crnt;

/* Geometry data */
extern data_t data;

/* Network data */
extern netcx_t netcx;

/* Excitation data */
extern vsorc_t vsorc;

/* Loading data */
extern zload_t zload;

/* Saved data buffer */
extern save_t save;

/* Ground data */
extern gnd_t gnd;

/* Field pattern data */
extern fpat_t fpat;

/* Data needed during prog execution */
extern calc_data_t calc_data;

/* Structure drawing area widget */
extern GtkWidget *structure_drawingarea;
extern GtkWidget *main_window;

/* Pixmap for structure drawing */
extern GdkPixmap *structure_pixmap;
extern int structure_pixmap_width, structure_pixmap_height;

/* Parameters for projecting structure and rad pattern to Screen */
extern projection_parameters_t structure_proj_params;
extern projection_parameters_t rdpattern_proj_params;

/* Plots and Radiation pattern drawingareas */
extern GtkWidget *rdpattern_window;

/* Pixmap for drawing plots */
extern GdkPixmap *freqplots_pixmap;
extern int freqplots_pixmap_width, freqplots_pixmap_height;

/* Pixmap for drawing radiation patterns */
extern GdkPixmap *rdpattern_pixmap;
extern int rdpattern_pixmap_width, rdpattern_pixmap_height;

/* Some graphics contexts */
extern GdkGC
  *white_gc,
  *black_gc,
  *plot_gc;

/* Segments for drawing structure */
GdkSegment *structure_segs = NULL;

/* Magnitude of seg/patch current/charge */
double *cmag = NULL, *ct1m = NULL, *ct2m = NULL;

/*-----------------------------------------------------------------------*/

/*  Draw_Structure()
 *
 *  Draws xyz axes, wire segments and patches
 */
  void
Draw_Structure( GtkWidget *drawingarea, GdkGC *gc )
{
  /* Abort if xnec2c may be quit by user */
  if( isFlagSet(MAIN_QUIT) )
	return;

  /* Clear pixmap */
  gdk_draw_rectangle(
	  structure_pixmap,
	  black_gc,
	  TRUE, 0, 0,
	  structure_proj_params.pixmap_width,
	  structure_proj_params.pixmap_height);

  /* Process and draw geometry if available, else clear screen */
  Process_Wire_Segments();
  Process_Surface_Patches();
  Draw_XYZ_Axes( structure_pixmap, structure_proj_params );
  Draw_Surface_Patches( drawingarea, gc, structure_segs+data.n, data.m );
  Draw_Wire_Segments( drawingarea, gc, structure_segs, data.n );

  /* Show gain in direction of viewer */
  Show_Viewer_Gain( main_window, "main_gain_entry", structure_proj_params );

  /* Render pixmap to screen */
  gdk_window_set_back_pixmap( drawingarea->window, structure_pixmap, FALSE );
  gdk_window_clear( drawingarea->window );

  /* Reset "new current data" flag */
  crnt.newer = 0;

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

} /* Draw_Structure() */

/*-----------------------------------------------------------------------*/

/*  New_Wire_Data()
 *
 *  Calculates some projection parameters
 *  when new wire segment data is created
 */
  void
New_Wire_Data( void )
{
  double
	r,	   /* Distance of a point from XYZ origin */
	r_max; /* Maximum value of above */

  int idx;


  /* Abort if no wire data */
  if( data.n == 0 )	return;

  /* Find segment end furthest from xyz axes origin */
  r_max = 0.0;
  for( idx = 0; idx < data.n; idx++ )
  {
	r = sqrt(
		(double)data.x1[idx] * (double)data.x1[idx] +
		(double)data.y1[idx] * (double)data.y1[idx] +
		(double)data.z1[idx] * (double)data.z1[idx] );
	if( r > r_max )
	  r_max = r;

	r = sqrt(
		(double)data.x2[idx] * (double)data.x2[idx] +
		(double)data.y2[idx] * (double)data.y2[idx] +
		(double)data.z2[idx] * (double)data.z2[idx] );
	if( r > r_max )
	  r_max = r;

  } /* for( idx = 0; idx < data.n; idx++ ) */

  /* Max value of segment r saved if appropriate */
  if( r_max > structure_proj_params.r_max )
	structure_proj_params.r_max = r_max;

  /* Redraw structure on screen */
  New_Projection_Parameters(
	  structure_pixmap_width,
	  structure_pixmap_height,
	  &structure_proj_params );

} /* New_Wire_Data() */

/*-----------------------------------------------------------------------*/

/*  New_Patch_Data()
 *
 *  Calculates some projection parameters
 *  when new surface patch data is created
 */
  void
New_Patch_Data( void )
{
  double
	s,		/* Side/2 of a square that will represent a patch  */
	sx, sy, sz, /* Length of components of s in the X, Y, Z axes */
	r,		/* Distance of points in patch from XYZ co-ordinates */
	r_max;	/* Maximum value of above */

  int idx, i;
  size_t mreq;


  /* Abort if no patch data */
  if( data.m == 0 )	return;

  /* Allocate memory for patch line segments */
  mreq = 2 * data.m * sizeof(double);
  mem_realloc( (void *)&data.px1, mreq, "in draw_structure.c" );
  mem_realloc( (void *)&data.py1, mreq, "in draw_structure.c" );
  mem_realloc( (void *)&data.pz1, mreq, "in draw_structure.c" );
  mem_realloc( (void *)&data.px2, mreq, "in draw_structure.c" );
  mem_realloc( (void *)&data.py2, mreq, "in draw_structure.c" );
  mem_realloc( (void *)&data.pz2, mreq, "in draw_structure.c" );

  /* Find point furthest from xyz axes origin */
  r_max = 0.0;
  for( idx = 0; idx < data.m; idx++ )
  {
	i = 2 * idx;

	/* Side/2 of square representing a patch (sqrt of patch area) */
	s = sqrt( data.pbi[idx] ) / 2.0;

	/* Projection of s on xyz components of t1 */
	sx = s * (double)data.t1x[idx];
	sy = s * (double)data.t1y[idx];
	sz = s * (double)data.t1z[idx];

	/* End 1 of line seg parallel to t1 vector */
	data.px1[i] = (double)data.px[idx] + sx;
	data.py1[i] = (double)data.py[idx] + sy;
	data.pz1[i] = (double)data.pz[idx] + sz;

	/* Its distance from XYZ origin */
	r = sqrt(
		data.px1[i]*data.px1[i] +
		data.py1[i]*data.py1[i] +
		data.pz1[i]*data.pz1[i] );
	if( r > r_max )
	  r_max = r;

	/* End 2 of line seg parallel to t1 vector */
	data.px2[i] = (double)data.px[idx] - sx;
	data.py2[i] = (double)data.py[idx] - sy;
	data.pz2[i] = (double)data.pz[idx] - sz;

	/* Its distance from XYZ origin */
	r = sqrt(
		data.px2[i]*data.px2[i] +
		data.py2[i]*data.py2[i] +
		data.pz2[i]*data.pz2[i] );
	if( r > r_max )
	  r_max = r;

	i++;

	/* Projection of s on xyz components of t2 */
	sx = s * (double)data.t2x[idx];
	sy = s * (double)data.t2y[idx];
	sz = s * (double)data.t2z[idx];

	/* End 1 of line parallel to t2 vector */
	data.px1[i] = (double)data.px[idx] + sx;
	data.py1[i] = (double)data.py[idx] + sy;
	data.pz1[i] = (double)data.pz[idx] + sz;

	/* Its distance from XYZ origin */
	r = sqrt(
		data.px1[i]*data.px1[i] +
		data.py1[i]*data.py1[i] +
		data.pz1[i]*data.pz1[i] );
	if( r > r_max )
	  r_max = r;

	/* End 2 of line parallel to t2 vector */
	data.px2[i] = (double)data.px[idx] - sx;
	data.py2[i] = (double)data.py[idx] - sy;
	data.pz2[i] = (double)data.pz[idx] - sz;

	/* Its distance from XYZ origin */
	r = sqrt(
		data.px2[i]*data.px2[i] +
		data.py2[i]*data.py2[i] +
		data.pz2[i]*data.pz2[i] );
	if( r > r_max )
	  r_max = r;

  } /* for( idx = 0; idx < data.m; idx++ ) */

  /* Max value of patch r saved if appropriate */
  if( r_max > structure_proj_params.r_max )
	structure_proj_params.r_max = r_max;

  /* Redraw structure on screen */
  New_Projection_Parameters(
	  structure_pixmap_width,
	  structure_pixmap_height,
	  &structure_proj_params );

} /* New_Patch_Data() */

/*-----------------------------------------------------------------------*/

/*  Process_Wire_Segments()
 *
 *  Processes wire segment data so they can be drawn on Screen
 */
  void
Process_Wire_Segments( void )
{
  int idx;

  /* Project all wire segs from xyz frame to screen frame */
  for( idx = 0; idx < data.n; idx++ )
	Set_Gdk_Segment(
		&structure_segs[idx],
		&structure_proj_params,
		(double)data.x1[idx],
		(double)data.y1[idx],
		(double)data.z1[idx],
		(double)data.x2[idx],
		(double)data.y2[idx],
		(double)data.z2[idx] );

} /* Process_Wire_Segments() */

/*-----------------------------------------------------------------------*/

/*  Process_Surface_Patches()
 *
 *  Processes surface patch data so they can be drawn on Screen
 */
  void
Process_Surface_Patches( void )
{
  int idx, m2;

  /* Project all patch segs from xyz frame to screen frame */
  /* Patches are represented by 2 line segs parallel to t1 */
  /* and t2 vectors. Length of segs is sqrt of patch area  */
  m2 = data.m * 2;
  for( idx = 0; idx < m2; idx++ )
	Set_Gdk_Segment(
		&structure_segs[idx+data.n],
		&structure_proj_params,
		data.px1[idx],
		data.py1[idx],
		data.pz1[idx],
		data.px2[idx],
		data.py2[idx],
		data.pz2[idx] );

} /* Process_Surface_Patches() */

/*-----------------------------------------------------------------------*/

/*  Draw_Wire_Segments()
 *
 *  Draws all wire segments of the input structure
 */

void
Draw_Wire_Segments(
	GtkWidget *drawingarea,
	GdkGC *gc,
	GdkSegment *segm,
	gint nseg )
{
  int idx, i;

  /* Abort if no wire segs or new input pending */
  if( !nseg || isFlagSet(INPUT_PENDING) )
	return;

  /* Draw networks */
  for( idx = 0; idx < netcx.nonet; idx++ )
  {
	int i1, i2;

	i1 = netcx.iseg1[idx]-1;
	i2 = netcx.iseg2[idx]-1;

	switch( netcx.ntyp[idx] )
	{
	  case 1: /* Two-port network */
		{
		  GdkPoint points[4];

		  /* Set gc attributes for networks */
		  Set_GC_Attributes( gc, MAGENTA, 0, drawingarea );

		  /* Draw a box between segs to represent two-port network */
		  points[0].x = segm[i1].x1 + (segm[i2].x1 - segm[i1].x1)/3;
		  points[0].y = segm[i1].y1 + (segm[i2].y1 - segm[i1].y1)/3;
		  points[1].x = segm[i2].x1 + (segm[i1].x1 - segm[i2].x1)/3;
		  points[1].y = segm[i2].y1 + (segm[i1].y1 - segm[i2].y1)/3;
		  points[2].x = segm[i2].x2 + (segm[i1].x2 - segm[i2].x2)/3;
		  points[2].y = segm[i2].y2 + (segm[i1].y2 - segm[i2].y2)/3;
		  points[3].x = segm[i1].x2 + (segm[i2].x2 - segm[i1].x2)/3;
		  points[3].y = segm[i1].y2 + (segm[i2].y2 - segm[i1].y2)/3;
		  gdk_draw_polygon( structure_pixmap, gc, TRUE, points, 4  );

		  /* Draw connecting lines */
		  gdk_draw_line(
			  structure_pixmap, gc,
			  segm[i1].x1,
			  segm[i1].y1,
			  segm[i2].x1,
			  segm[i2].y1 );

		  gdk_draw_line(
			  structure_pixmap, gc,
			  segm[i1].x2,
			  segm[i1].y2,
			  segm[i2].x2,
			  segm[i2].y2 );
		  break;
		}

	  case 2: /* Straight transmission line */
		/* Set gc attributes for networks */
		Set_GC_Attributes( gc, CYAN, 0, drawingarea );
		gdk_draw_line(
			structure_pixmap, gc,
			segm[i1].x1,
			segm[i1].y1,
			segm[i2].x1,
			segm[i2].y1 );

		gdk_draw_line(
			structure_pixmap, gc,
			segm[i1].x2,
			segm[i1].y2,
			segm[i2].x2,
			segm[i2].y2 );
		break;

	  case 3: /* Crossed transmisson line */
		/* Set gc attributes for networks */
		Set_GC_Attributes( gc, CYAN, 0, drawingarea );
		gdk_draw_line(
			structure_pixmap, gc,
			segm[i1].x1,
			segm[i1].y1,
			segm[i2].x2,
			segm[i2].y2 );

		gdk_draw_line(
			structure_pixmap, gc,
			segm[i1].x2,
			segm[i1].y2,
			segm[i2].x1,
			segm[i2].y1 );

	} /* switch( netcx.ntyp ) */

  } /* for( idx = 0; idx < netcx.nonet; idx++ ) */

  /* Draw currents/charges if enabled, return */
  /* Current or charge calculations do not contain wavelength */
  /* factors, since they are drawn normalized to their max value */
  if(
	  (isFlagSet(DRAW_CURRENTS) ||
	   isFlagSet(DRAW_CHARGES)) &&
	  crnt.valid )
  {
	static double cmax; /* Max of seg current/charge */
	int red, grn, blu;  /* To color structure segs   */
	char label[11];

	/* Loop over all wire segs, find max current/charge */
	if( crnt.newer )
	{
	  cmax = 0.0;
	  for( idx = 0; idx < nseg; idx++ )
	  {
		if( isFlagSet(DRAW_CURRENTS) )
		  /* Calculate segment current magnitude */
		  cmag[idx] = (double)cabsl( crnt.cur[idx] );
		else
		  /* Calculate segment charge density */
		  cmag[idx] = (double)cabsl( cmplx(crnt.bii[idx], crnt.bir[idx]) );

		/* Find max current/charge magnitude */
		if( cmag[idx] > cmax )
		  cmax = cmag[idx];
	  }

	  /* Show max value in color code label */
	  if( isFlagSet(DRAW_CURRENTS) )
		snprintf( label, 11, "%10.3E", cmax * (double)data.wlam );
	  else
		snprintf( label, 11, "%10.3E", cmax * 1.0E-6/(double)calc_data.fmhz );
	  gtk_label_set_text(
		  GTK_LABEL(lookup_widget(structure_drawingarea,
			  "main_colorcode_maxlabel")), label );

	} /* if( crnt.newer ) */

	/* Draw segments in color code according to current */
	for( idx = 0; idx < nseg; idx++ )
	{
	  /* Calculate RGB value for seg current */
	  Value_to_Color( &red, &grn, &blu, cmag[idx], cmax );

	  /* Set gc attributes for segment */
	  Set_GC_Attributes( gc, red, grn, blu, 2, drawingarea );

	  /* Draw segment */
	  gdk_draw_line(
		  structure_pixmap, gc,
		  segm[idx].x1,
		  segm[idx].y1,
		  segm[idx].x2,
		  segm[idx].y2 );

	} /* for( idx = 0; idx < nseg; idx++ ) */

	return;

  } /* if( isFlagSet(DRAW_CURRENTS) || isFlagSet(DRAW_CHARGES) ) */

  /* Draw segs if not all loaded */
  if( zload.nldseg != nseg )
  {
	/* Set gc attributes for segments */
	if( isFlagSet(OVERLAY_STRUCT) && isFlagSet(DRAW_EHFIELD) )
	  Set_GC_Attributes( gc, WHITE, 2, drawingarea );
	else
	  Set_GC_Attributes( gc, BLUE, 2, drawingarea );

	/* Draw wire segments */
	gdk_draw_segments( structure_pixmap, gc, segm, nseg );
  }

  /* Draw loaded segments */
  for( idx = 0; idx < zload.nldseg; idx++ )
  {
	/* Set gc attributes for loaded segs */
	if( zload.ldtype[idx] == 5 ) /* Finite wire resistivity */
	  Set_GC_Attributes( gc, YELLOW, 2, drawingarea );
	else /* Lumped loads */
	  Set_GC_Attributes( gc, YELLOW, 4, drawingarea );

	i = zload.ldsegn[idx]-1;
	gdk_draw_line(
		structure_pixmap, gc,
		segm[i].x1,
		segm[i].y1,
		segm[i].x2,
		segm[i].y2 );
  }

  /* Set gc attributes for excitation */
  Set_GC_Attributes( gc, RED, 2, drawingarea );

  /* Draw excitation sources */
  for( idx = 0; idx < vsorc.nsant; idx++ )
  {
	i = vsorc.isant[idx]-1;
	gdk_draw_line(
		structure_pixmap, gc,
		segm[i].x1,
		segm[i].y1,
		segm[i].x2,
		segm[i].y2 );
  }

  for( idx = 0; idx < vsorc.nvqd; idx++ )
  {
	i = vsorc.ivqd[idx]-1;
	gdk_draw_line(
		structure_pixmap, gc,
		segm[i].x1,
		segm[i].y1,
		segm[i].x2,
		segm[i].y2 );
  }

} /* Draw_Wire_Segments() */

/*-----------------------------------------------------------------------*/

/*  Draw_Surface_Patches()
 *
 *  Draws the line segments that represent surface patches
 */
void
Draw_Surface_Patches(
	GtkWidget *drawingarea,
	GdkGC *gc,
	GdkSegment *segm,
	gint npatch )
{
  /* Abort if no patches */
  if( ! npatch )
	return;

  /* Draw currents if enabled, return */
  if( isFlagSet(DRAW_CURRENTS) && crnt.valid )
  {
	/* Buffers for t1,t2 currents below */
	static double cmax;

	/* Current along x,y,z and t1,t2 vector directions */
	complex double cx, cy, cz, ct1, ct2;

	int red, grn, blu, i, j;

	/* Find max value of patch current magnitude */
	if( crnt.newer )
	{
	  j= data.n;
	  cmax = 0.0;

	  for( i = 0; i < npatch; i++ )
	  {
		/* Calculate current along x,y,z vectors */
		cx = (complex double)crnt.cur[j];
		cy = (complex double)crnt.cur[j+1];
		cz = (complex double)crnt.cur[j+2];

		/* Calculate current along t1 and t2 tangent vectors */
		ct1 = cx*(double)data.t1x[i] +
		  cy*(double)data.t1y[i] +
		  cz*(double)data.t1z[i];
		ct2 = cx*(double)data.t2x[i] +
		  cy*(double)data.t2y[i] +
		  cz*(double)data.t2z[i];

		/* Save current magnitudes */
		ct1m[i] = (double)cabsl( ct1 );
		ct2m[i] = (double)cabsl( ct2 );

		/* Find current magnitude max */
		if( ct1m[i] > cmax ) cmax = ct1m[i];
		if( ct2m[i] > cmax ) cmax = ct2m[i];

		j += 3;

	  } /* for( i = 0; i < npatch; i++ ) */

	} /* if( crnt.newer ) */

	/* Draw patches in color code according to current */
	for( i = 0; i < npatch; i++ )
	{
	  j = 2 * i;

	  /* Calculate RGB value for patch t1 current */
	  Value_to_Color( &red, &grn, &blu, ct1m[i], cmax );

	  /* Set gc attributes for patch t1 */
	  Set_GC_Attributes( gc, red, grn, blu, 2, drawingarea );

	  /* Draw patch t1 */
	  gdk_draw_line(
		  structure_pixmap, gc,
		  segm[j].x1,
		  segm[j].y1,
		  segm[j].x2,
		  segm[j].y2 );

	  /* Calculate RGB value for patch t2 current */
	  Value_to_Color( &red, &grn, &blu, ct2m[i], cmax );

	  /* Set gc attributes for patch t2 */
	  Set_GC_Attributes( gc, red, grn, blu, 2, drawingarea );

	  /* Draw patch t2 */
	  j++;
	  gdk_draw_line(
		  structure_pixmap, gc,
		  segm[j].x1,
		  segm[j].y1,
		  segm[j].x2,
		  segm[j].y2 );

	} /* for( idx = 0; idx < npatch; idx++ ) */

  } /* if( isFlagSet(DRAW_CURRENTS) ) */
  else
  {
	/* Set gc attributes for patches */
	if( isFlagSet(OVERLAY_STRUCT) && isFlagSet(DRAW_EHFIELD) )
	  Set_GC_Attributes( gc, WHITE, 2, drawingarea );
	else
	  Set_GC_Attributes( gc, BLUE, 2, drawingarea );

	/* Draw patch segments */
	gdk_draw_segments( structure_pixmap, gc, segm, 2*npatch );
  }

} /* Draw_Surface_Patches() */

/*-----------------------------------------------------------------------*/

/* Redo_Currents()
 *
 * Refreshes plots on new frequency in spinbutton
 */
  gboolean
Redo_Currents( gpointer userdata )
{
  /* Abort if no geometry data */
  if( ((data.n == 0) && (data.m == 0)) ||
	  isFlagClear(ENABLE_EXCITN) )
	return FALSE;

  /* Makes calcs use the extra buffer in rad_pattern */
  calc_data.fstep = calc_data.nfrq;
  save.last_freq = 0.0l;
  New_Frequency();

  /* Display freq data in entry widgets */
  if( isFlagSet(PLOT_FREQ_LINE) )
	Plot_Frequency_Data();

  /* Redraw structure on screen */
  if( (structure_drawingarea != NULL) &&
	  (isFlagSet(DRAW_CURRENTS) || isFlagSet(DRAW_CHARGES)) )
	Draw_Structure( structure_drawingarea, plot_gc );

  return FALSE;

} /* Redo_Currents() */

/*-----------------------------------------------------------------------*/

/*  New_Structure_Projection_Angle()
 *
 *  Calculates new projection parameters when a
 *  structure projection angle (Wr or Wi) changes
 */
  void
New_Structure_Projection_Angle(void)
{
  /* sin and cos of structure rotation and inclination angles */
  structure_proj_params.sin_wr = sin(structure_proj_params.Wr/TD);
  structure_proj_params.cos_wr = cos(structure_proj_params.Wr/TD);
  structure_proj_params.sin_wi = sin(structure_proj_params.Wi/TD);
  structure_proj_params.cos_wi = cos(structure_proj_params.Wi/TD);

  /* Trigger a redraw of structure drawingarea */
  Draw_Structure( structure_drawingarea, plot_gc );

  /* Trigger a redraw of plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(PLOT_GVIEWER) )
	Plot_Frequency_Data();

} /* New_Structure_Projection_Angle() */

/*-----------------------------------------------------------------------*/

/*  Init_Struct_Drawing()
 *
 *  Initializes drawing parameters after geometry input
 */
  void
Init_Struct_Drawing( void )
{
  /* We need n segs for wires + 2m for patces */
  mem_realloc( (void *)&structure_segs,
	  (data.n+2*data.m) * sizeof(GdkSegment), "in draw_structure.c" );
  New_Wire_Data();
  New_Patch_Data();
}

/*-----------------------------------------------------------------------*/

/* Show_Viewer_Gain()
 *
 * Shows gain in direction of viewer
 */
void
Show_Viewer_Gain(
	GtkWidget *window,
	const char *widget,
	projection_parameters_t proj_params )
{
  if( isFlagSet(DRAW_CURRENTS) ||
	  isFlagSet(DRAW_CHARGES)  ||
	  isFlagSet(DRAW_GAIN) )
  {
	char txt[8];

	if( isFlagSet(ENABLE_RDPAT) && (calc_data.fstep >=0) )
	  snprintf( txt, 8, "%7.2f",
		  Viewer_Gain(proj_params, calc_data.fstep) );
	else
	  strcpy( txt, "   -----" );

	gtk_entry_set_text( GTK_ENTRY(lookup_widget(window, widget)), txt );
  }

} /* Show_Viewer_Gain() */

/*-----------------------------------------------------------------------*/

