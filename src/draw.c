/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
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
 *  draw.c
 *
 *  Drawing routines for xnec2c
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "xnec2c.h"
#include "support.h"

/* Structure drawingarea */
extern GtkWidget *structure_drawingarea;

/* Some graphics contexts */
extern GdkGC
  *white_gc,
  *black_gc,
  *plot_gc;

/*-----------------------------------------------------------------------*/

/* Set_Gdk_Segment()
 *
 *  Calculates window x,y co-ordinates of a GdkSegment for
 *  drawing on Screen. Input data is a line in the xyz frame
 */
void
Set_Gdk_Segment(
	GdkSegment *segm,
	projection_parameters_t *param,
	double x1, double y1, double z1,
	double x2, double y2, double z2 )
{
  double x, y;

  /* Project end 1 of seg in xyz frame to screen frame */
  Project_on_Screen( param, x1, y1, z1, &x, &y );
  segm->x1 = (gint)(param->x_center + x*param->xy_scale);
  segm->y1 = param->pixmap_height -
	(gint)(param->y_center + y*param->xy_scale);

  /* Project end 2 of seg in xyz frame to screen frame */
  Project_on_Screen( param, x2, y2, z2, &x, &y );
  segm->x2 = (gint)(param->x_center + x*param->xy_scale);
  segm->y2 = param->pixmap_height -
	(gint)(param->y_center + y*param->xy_scale);

} /* Set_Gdk_Segment() */

/*-----------------------------------------------------------------------*/

/*  Project_on_Screen()
 *
 *  Projects a point in the x,y,z co-ordinate
 *  frame of NEC2 to the x,y frame of the Screen
 */
void
Project_on_Screen(
	projection_parameters_t *param,
	double x, double y, double z,
	double *xs, double *ys )
{
  *xs = y*param->cos_wr - x*param->sin_wr;
  *ys = z*param->cos_wi - param->sin_wi *
	(x*param->cos_wr + y*param->sin_wr);

} /* Project_on_Screen() */

/*-----------------------------------------------------------------------*/

/*  Project_XYZ_Axes()
 *
 *  Sets GdkSegment data to project xyz axes on Screen
 */
void
Project_XYZ_Axes(
	GdkPixmap *pixmap,
	projection_parameters_t *param,
	GdkSegment *segm )
{
  double x, y;
  PangoLayout *layout;

  layout = gtk_widget_create_pango_layout(
	  structure_drawingarea, "x" );

  segm->x1 = (gint)param->x_center;
  segm->y1 = param->pixmap_height - (gint)param->y_center;
  Project_on_Screen( param, param->r_max, 0.0, 0.0, &x, &y );
  segm->x2 = (gint)(param->x_center + x*param->xy_scale);
  segm->y2 = param->pixmap_height -
	(gint)(param->y_center + y*param->xy_scale);
  gdk_draw_layout( pixmap, white_gc, segm->x2, segm->y2, layout );

  (segm+1)->x1 = (gint)param->x_center;
  (segm+1)->y1 = param->pixmap_height - (gint)param->y_center;
  Project_on_Screen( param, 0.0, param->r_max, 0.0, &x, &y );
  (segm+1)->x2 = (gint)(param->x_center + x*param->xy_scale);
  (segm+1)->y2 = param->pixmap_height -
	(gint)(param->y_center + y*param->xy_scale);
  pango_layout_set_text( layout, "y", -1 );
  gdk_draw_layout( pixmap, white_gc,
	  (segm+1)->x2, (segm+1)->y2, layout );

  (segm+2)->x1 = (gint)param->x_center;
  (segm+2)->y1 = param->pixmap_height - (gint)param->y_center;
  Project_on_Screen( param, 0.0, 0.0, param->r_max, &x, &y );
  (segm+2)->x2 = (gint)(param->x_center + x*param->xy_scale);
  (segm+2)->y2 = param->pixmap_height -
	(gint)(param->y_center + y*param->xy_scale);
  pango_layout_set_text( layout, "z", -1 );
  gdk_draw_layout( pixmap, white_gc,
	  (segm+2)->x2+2, (segm+2)->y2, layout );

} /* Project_XYZ_Axes() */

/*-----------------------------------------------------------------------*/

/*  Draw_XYZ_Axes()
 *
 *  Draws the xyz axes to Screen
 */
  void
Draw_XYZ_Axes( GdkPixmap *pixmap, projection_parameters_t params )
{
  static GdkSegment seg[3];

  /* Calcualte Screen co-ordinates of xyz axes */
  Project_XYZ_Axes( pixmap, &params, seg );

  /* Draw xyz axes */
  gdk_draw_segments( pixmap, white_gc, seg, 3 );

} /* Draw_XYZ_Axes() */

/*-----------------------------------------------------------------------*/

/*  New_Projection_Parameters()
 *
 *  Calculates values for some projection parameters when
 *  a new backing pixmap is created after window resize or
 *  new wire or patch data is entered.
 */
void
New_Projection_Parameters(
	int width, int height,
	projection_parameters_t *param )
{
  double size2;

  if( width < height )
	size2 = (double)width/2.0;
  else
	size2 = (double)height/2.0;

  param->x_center = param->y_center = size2;
  if( param->r_max == 0.0 )
	param->xy_scale = 1.0;
  else
	param->xy_scale = size2 / param->r_max;
  param->pixmap_width  = width;
  param->pixmap_height = height;

} /* New_Projection_Parameters() */

/*-----------------------------------------------------------------------*/

/*  Set_GC_Attributes()
 *
 *  Sets new attributes for a GdkGc
 */
void
Set_GC_Attributes(
	GdkGC *gc,
	int red, int green, int blue,
	gint line_width,
	GtkWidget *widget )
{
  static GdkColor *color;
  static char flag = 1;

  if( flag )
  {
	color = (GdkColor *)malloc(sizeof(GdkColor));
	flag = 0;
  }

  color->red   = (red  *65535)/255;
  color->green = (green*65535)/255;
  color->blue  = (blue *65535)/255;

  gdk_colormap_alloc_color(
	  gtk_widget_get_colormap(widget), color, FALSE, TRUE );
  gdk_gc_set_foreground(gc, color);
  gdk_gc_set_line_attributes(
	  gc, line_width,
	  GDK_LINE_SOLID,
	  GDK_CAP_NOT_LAST,
	  GDK_JOIN_MITER);

} /* Set_GC_Attributes() */

/*-----------------------------------------------------------------------*/

/* Value_to_Color()
 *
 * Produces an rgb color to represent an
 * input value relative to a maximum value
 */
  void
Value_to_Color( int *red, int *grn, int *blu, double val, double max )
{
  int ival;

  /* Scale val so that normalized ival is 0-1279 */
  ival = (int)(1279.0l * val / max);

  /* Color hue according to imag value */
  switch( ival / 256 )
  {
	case 0: /* 0-255 : magenta to blue */
	  *red = 255 - ival;
	  *grn = 0;
	  *blu = 255;
	  break;

	case 1: /* 256-511 : blue to cyan */
	  *red = 0;
	  *grn = ival - 256;
	  *blu = 255;
	  break;

	case 2: /* 512-767 : cyan to green */
	  *red = 0;
	  *grn = 255;
	  *blu = 767 - ival;
	  break;

	case 3: /* 768-1023 : green to yellow */
	  *red = ival - 768;
	  *grn = 255;
	  *blu = 0;
	  break;

	case 4: /* 1024-1279 : yellow to red */
	  *red = 255;
	  *grn = 1279 - ival;
	  *blu = 0;

  } /* switch( imag / 256 ) */

} /* Value_to_Color() */

/*-----------------------------------------------------------------------*/

