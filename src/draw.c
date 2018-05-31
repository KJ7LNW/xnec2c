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

#include "draw.h"
#include "shared.h"

/*-----------------------------------------------------------------------*/

/*  Project_on_Screen()
 *
 *  Projects a point in the x,y,z co-ordinate
 *  frame of NEC2 to the x,y frame of the Screen
 */
  static void
Project_on_Screen(
	projection_parameters_t *params,
	double x, double y, double z,
	double *xs, double *ys )
{
  *xs = y*params->cos_wr - x*params->sin_wr;
  *ys = z*params->cos_wi - params->sin_wi *
	(x*params->cos_wr + y*params->sin_wr);

} /* Project_on_Screen() */

/*-----------------------------------------------------------------------*/

/* Set_Gdk_Segment()
 *
 *  Calculates window x,y co-ordinates of a Segment_t for
 *  drawing on Screen. Input data is a line in the xyz frame
 */
  void
Set_Gdk_Segment(
	Segment_t *segm,
	projection_parameters_t *params,
	double x1, double y1, double z1,
	double x2, double y2, double z2 )
{
  double x, y;

  /* Project end 1 of seg in xyz frame to screen frame */
  Project_on_Screen( params, x1, y1, z1, &x, &y );
  segm->x1 = (gint)(params->x_center + params->dx_center + x*params->xy_scale);
  segm->y1 = params->height -
	(gint)(params->y_center + params->dy_center + y*params->xy_scale);

  /* Project end 2 of seg in xyz frame to screen frame */
  Project_on_Screen( params, x2, y2, z2, &x, &y );
  segm->x2 = (gint)(params->x_center + params->dx_center + x*params->xy_scale);
  segm->y2 = params->height -
	(gint)(params->y_center + params->dy_center + y*params->xy_scale);

} /* Set_Gdk_Segment() */

/*-----------------------------------------------------------------------*/

/*  Project_XYZ_Axes()
 *
 *  Sets Segment_t data to project xyz axes on Screen
 */
  static void
Project_XYZ_Axes(
	cairo_t *cr,
	projection_parameters_t *params,
	Segment_t *segment )
{
  double x, y;
  PangoLayout *layout;
  Segment_t *segm = segment;

  /* cairo context */
  cairo_set_source_rgb( cr, WHITE );

  layout = gtk_widget_create_pango_layout( structure_drawingarea, "x" );

  segm->x1 = (gint)( params->x_center + params->dx_center );
  segm->y1 = params->height - (gint)( params->y_center + params->dy_center );
  Project_on_Screen( params, params->r_max, 0.0, 0.0, &x, &y );
  segm->x2 = (gint)(params->x_center + params->dx_center + x*params->xy_scale);
  segm->y2 = params->height -
	(gint)(params->y_center + params->dy_center + y*params->xy_scale);
  cairo_move_to( cr, (double)segm->x2, (double)segm->y2 );
  pango_cairo_show_layout( cr, layout );

  segm++;
  segm->x1 = (gint)( params->x_center + params->dx_center );
  segm->y1 = params->height - (gint)( params->y_center + params->dy_center );
  Project_on_Screen( params, 0.0, params->r_max, 0.0, &x, &y );
  segm->x2 = (gint)(params->x_center + params->dx_center + x*params->xy_scale);
  segm->y2 = params->height -
	(gint)(params->y_center + params->dy_center + y*params->xy_scale);
  pango_layout_set_text( layout, "y", -1 );
  cairo_move_to( cr, (double)segm->x2, (double)segm->y2 );
  pango_cairo_show_layout( cr, layout );

  segm++;
  segm->x1 = (gint)( params->x_center + params->dx_center );
  segm->y1 = params->height - (gint)( params->y_center + params->dy_center );
  Project_on_Screen( params, 0.0, 0.0, params->r_max, &x, &y );
  segm->x2 = (gint)(params->x_center + params->dx_center + x*params->xy_scale);
  segm->y2 = params->height -
	(gint)(params->y_center + params->dy_center + y*params->xy_scale);
  pango_layout_set_text( layout, " z", -1 );
  cairo_move_to( cr, (double)segm->x2, (double)segm->y2 );
  pango_cairo_show_layout( cr, layout );

  g_object_unref( layout );

} /* Project_XYZ_Axes() */

/*-----------------------------------------------------------------------*/

/*  Draw_XYZ_Axes()
 *
 *  Draws the xyz axes to Screen
 */
  void
Draw_XYZ_Axes( cairo_t *cr, projection_parameters_t params )
{
  static Segment_t seg[3];

  /* Calcualte Screen co-ordinates of xyz axes */
  Project_XYZ_Axes( cr, &params, seg );

  /* Draw xyz axes */
  Cairo_Draw_Segments( cr, seg, 3 );

} /* Draw_XYZ_Axes() */

/*-----------------------------------------------------------------------*/

/*  New_Projection_Parameters()
 *
 *  Calculates values for some projection parameters after
 *  a window is resized or new wire or patch data entered.
 */
  void
New_Projection_Parameters(
	int width, int height,
	projection_parameters_t *params )
{
  /* Half the size of drawing area */
  double size2;
  if( width < height )
	size2 = (double)width / 2.0;
  else
	size2 = (double)height / 2.0;

  /* This defines the center of the drawing areas. For the x co-ordinate
   * half the size of the drawing area is right (for widths of odd number
   * of pixels, so that there are even and equal number of pixels either
   * side of center). But for the y co-ordinate we need to round up as
   * y is from the top down and the y co-ordinate of center is calculated
   * from height - y_center and this leads to a 1-pixel error */
  params->x_center = (double)width / 2.0;
  params->y_center = (double)height / 2.0 + 0.5;

  if( params->r_max == 0.0 )
	params->xy_scale1 = 1.0;
  else
	params->xy_scale1 = size2 / params->r_max;
  params->xy_scale = params->xy_scale1 * params->xy_zoom;

  params->width  = width;
  params->height = height;

} /* New_Projection_Parameters() */

/*-----------------------------------------------------------------------*/

/* Value_to_Color()
 *
 * Produces an rgb color to represent an
 * input value relative to a maximum value
 */
  void
Value_to_Color( double *red, double *grn, double *blu, double val, double max )
{
  int ival;

  /* Scale val so that normalized ival is 0-1279 */
  ival = (int)(1279.0 * val / max);

  /* Color hue according to imag value */
  switch( ival/256 )
  {
	case 0: /* 0-255 : magenta to blue */
	  *red = 255.0 - (double)ival;
	  *grn = 0.0;
	  *blu = 255.0;
	  break;

	case 1: /* 256-511 : blue to cyan */
	  *red = 0.0;
	  *grn = (double)ival - 256.0;
	  *blu = 255.0;
	  break;

	case 2: /* 512-767 : cyan to green */
	  *red = 0.0;
	  *grn = 255.0;
	  *blu = 767.0 - (double)ival;
	  break;

	case 3: /* 768-1023 : green to yellow */
	  *red = (double)ival - 768.0;
	  *grn = 255.0;
	  *blu = 0.0;
	  break;

	case 4: /* 1024-1279 : yellow to red */
	  *red = 255.0;
	  *grn = 1279.0 - (double)ival;
	  *blu = 0.0;

  } /* switch( imag / 256 ) */

  /* Scale values between 0.0-1.0 */
  *red /= 255.0;
  *grn /= 255.0;
  *blu /= 255.0;

} /* Value_to_Color() */

/*-----------------------------------------------------------------------*/

/* Cairo_Draw_Polygon()
 *
 * Draws a polygon, given a number of points
 */
  void
Cairo_Draw_Polygon( cairo_t* cr, GdkPoint *points, int npoints )
{
  int idx;

  cairo_move_to( cr, (double)points[0].x, (double)points[0].y );
  for( idx = 1; idx < npoints; idx++ )
	cairo_line_to( cr, (double)points[idx].x, (double)points[idx].y );
  cairo_close_path( cr );

} /* Cairo_Draw_Polygon() */

/*-----------------------------------------------------------------------*/

/* Cairo_Draw_Segments()
 *
 * Draws a number of line segments
 */
  void
Cairo_Draw_Segments( cairo_t *cr, Segment_t *segm, int nseg )
{
  int idx;

  for( idx = 0; idx < nseg; idx++ )
  {
	cairo_move_to( cr, (double)segm[idx].x1, (double)segm[idx].y1 );
	cairo_line_to( cr, (double)segm[idx].x2, (double)segm[idx].y2 );
  }
  cairo_stroke( cr );

} /* Cairo_Draw_Segments() */

/*-----------------------------------------------------------------------*/

/* Cairo_Draw_Line()
 *
 * Draws a line between to x,y co-ordinates
 */
  void
Cairo_Draw_Line( cairo_t *cr, int x1, int y1, int x2, int y2 )
{
  cairo_move_to( cr, (double)x1, (double)y1 );
  cairo_line_to( cr, (double)x2, (double)y2 );
  cairo_stroke( cr );

} /* Cairo_Draw_Line() */

/*-----------------------------------------------------------------------*/

/* Cairo_Draw_Lines()
 *
 * Draws lines between points
 */
  void
Cairo_Draw_Lines( cairo_t *cr, GdkPoint *points, int npoints )
{
  int idx;

  cairo_move_to( cr, (double)points[0].x, (double)points[0].y );
  for( idx = 1; idx < npoints; idx++ )
	cairo_line_to( cr, (double)points[idx].x, (double)points[idx].y );
  cairo_stroke( cr );

} /* Cairo_Draw_Line() */

/*-----------------------------------------------------------------------*/

