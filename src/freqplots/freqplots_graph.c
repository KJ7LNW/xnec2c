/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

/*
 * freqplots_graph: Shared XY drawing primitives.
 *
 * Draw_Plotting_Frame deposits the grid, draw_poly deposits point-marker
 * outlines, and Draw_Graph deposits the trace polyline plus markers.  All
 * geometry enters the depth-sorted scenebuffer at the caller-supplied z
 * layer; min/max labels are deferred for painting on top.
 */

#include "freqplots_internal.h"
#include "../shared.h"

/* Draw_Plotting_Frame()
 *
 * Draws a graph plotting frame, including
 * horizontal and vertical divisions
 */
  void
Draw_Plotting_Frame(
    fp_render_t *fp,
    const fp_style_t *style,
    gchar **title,
    GdkRectangle *rect,
    int nhor, int nvert )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  int idx, xpw, xps, yph, yps;
  fp_stroke_t grid = { .color = th->colors[THEME_ROLE_GRID], .width = w->widths[FP_W_GRID] * density.stroke, .z_mid = FP_Z_GRID };
  fp_stroke_t box  = { .color = th->colors[THEME_ROLE_GRID], .width = w->widths[FP_W_AXIS] * density.stroke, .z_mid = FP_Z_GRID };

  /* title is unused; the frame draws only grid lines and the bounding box */
  (void)title;

  /* Move to plot box and divisions */
  xpw = rect->x + rect->width;
  yph = rect->y + rect->height;

  /* Draw vertical divisions */
  nvert--;
  for( idx = 1; idx <= nvert; idx++ )
  {
    xps = rect->x + (idx * rect->width) / nvert;
    fp_add_line( fp, xps, rect->y, xps, yph, grid );
  }

  /* Draw horizontal divisions */
  nhor--;
  for( idx = 1; idx <= nhor; idx++ )
  {
    yps = rect->y + (idx * rect->height) / nhor;
    fp_add_line( fp, rect->x, yps, xpw, yps, grid );
  }

  /* Draw outer box */
  fp_add_rect( fp, rect->x, rect->y, rect->width, rect->height, box );

} /* Draw_Plotting_Frame() */

/*-----------------------------------------------------------------------*/

  static void
draw_poly(
    fp_render_t *fp,
	int x, int y,
	int side, int size,
	float z, rgb_f_t c)
{
    if( side == LEFT )
      fp_add_filled_square( fp, x, y, size, z, c );
    else
      fp_add_filled_diamond( fp, x, y, size, z, c );
}

/*-----------------------------------------------------------------------*/

/* Draw_Graph()
 *
 * Plots a graph of a vs b
 * nval: number of points to plot
 * nval_max: number of points in the whole graph in case nval<nval_max
 */
  void
Draw_Graph(
    fp_render_t *fp,
    const fp_style_t *style,
    rgb_f_t trace_c,
    GdkRectangle *rect,
    double *a, double *b,
    double amax, double amin,
    double bmax, double bmin,
    int nval, int nval_max, int side )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  double ra;
  int idx;
  GdkPoint *points = NULL;
  char s[23];
  int marker_size, circle_padding;

  const int MARKER_SIZE_SINGLE_POINT = 10;
  const int MARKER_SIZE_MULTI_POINT = 6;
  const int CIRCLE_PADDING_PX = 3;

  /* Depth layer for this axis side; the side circle reads the trace color. */
  float   z       = (side == LEFT) ? FP_Z_LEFT : FP_Z_RIGHT;

  (void)nval_max;

  /* Range of values to plot */
  ra = amax - amin;

  marker_size = (nval < 3) ? MARKER_SIZE_SINGLE_POINT : MARKER_SIZE_MULTI_POINT;
  circle_padding = CIRCLE_PADDING_PX;

  /* Calculate points to plot */
  mem_array_alloc(&points, nval);
  if( points == NULL )
  {
    pr_err("memory allocation for points failed\n");
    Stop( ERR_OK, _("Draw_Graph():"
          "Memory allocation for points failed") );
    return;
  }

  int min_idx = 0, max_idx = 0;
  for( idx = 0; idx < nval; idx++ )
  {
    points[idx].x = fp_axis_pixel_x(rect, b[idx], bmin, bmax);
    points[idx].y = rect->y + (int)( (double)rect->height *
        (amax-a[idx]) / ra + 0.5 );

	if (a[idx] < a[min_idx])
		min_idx = idx;

	if (a[idx] > a[max_idx])
		max_idx = idx;

  }

  /* Draw the graph */
  fp_add_polyline( fp, points, nval,
      (fp_stroke_t){ .color = trace_c, .width = w->widths[FP_W_TRACE] * density.stroke, .z_mid = z } );

  /* Plot a small rectangle (left scale) or polygon (right scale) at point */
  for( idx = 0; idx < nval; idx++ )
  {
	// Circle behind enlarged markers provides visible axis color when no trace line
	if (marker_size == MARKER_SIZE_SINGLE_POINT)
	{
		fp_add_filled_circle( fp, points[idx].x, points[idx].y,
			marker_size/2 + circle_padding, z, trace_c );
	}

	rgb_f_t marker_c;
	if (idx == min_idx || idx == max_idx)
		marker_c = th->colors[THEME_ROLE_MARKER_EXTREME];
    else
		marker_c = trace_c;

	draw_poly(fp, points[idx].x, points[idx].y, side, marker_size, z, marker_c);
  }

  // Min/max labels:
  if (rc_config.freqplots_min_max)
  {
	  snprintf(s, sizeof(s)-1, _("min:%.2f\nmax:%.2f"), a[min_idx], a[max_idx]);

	  if (side == LEFT)
		  fp_add_text(fp, rect->x + 5, rect->y + 5, 1.0f, s,
                              JUSTIFY_LEFT, trace_c);
	  else
		  fp_add_text(fp, rect->x + rect->width - 5, rect->y + 5,
                              1.0f, s, JUSTIFY_RIGHT, trace_c);
  }

  mem_array_free(&points);

} /* Draw_Graph() */
