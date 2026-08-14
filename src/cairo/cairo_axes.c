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

/*
 * cairo_axes: XYZ axis drawing for Cairo rendering.
 */
#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/* Axis direction vectors and labels */
static const struct
{
  double ox, oy, oz;
  const char *label;
} axis_table[3] =
{
  { 1.0, 0.0, 0.0, "x"  },
  { 0.0, 1.0, 0.0, "y"  },
  { 0.0, 0.0, 1.0, "z" }
};

/**
 * cairo_draw_axes() - Deposit axis segments and defer label rendering
 * @surface: Cairo surface receiving the deposit
 * @extent:  half-extent of primary content for axis length
 *
 * Projects each axis from origin to tip via Set_Gdk_Segment, deposits
 * the line into the scenebuffer with real camera-axis depth, and stores
 * label positions on the render context for post-flush Pango rendering.
 */
  void
cairo_draw_axes(render_surface_t *surface, float extent)
{
  cairo_engine_surface_t *cs = cairo_engine_surface(surface);
  cairo_render_ctx_t *cc = &cs->frame;
  view_t *v = surface->view;
  double scale = view_projection_scale(v, extent, v->zoom);
  Segment_t seg;
  int idx;

  for( idx = 0; idx < 3; idx++ )
  {
    Set_Gdk_Segment(&seg, v, scale,
        0.0, 0.0, 0.0,
        axis_table[idx].ox * (double)extent,
        axis_table[idx].oy * (double)extent,
        axis_table[idx].oz * (double)extent,
        &seg.z_mid);

    seg_set_color(&seg, cc->view_axis);
    seg.width = 1.0f;
    scenebuffer_add(&cs->scenebuffer, &seg);

    cc->axis_labels[idx].x    = seg.x2;
    cc->axis_labels[idx].y    = seg.y2;
    cc->axis_labels[idx].text = axis_table[idx].label;
  }
  cc->n_axis_labels = AXIS_COUNT;
}

/*-----------------------------------------------------------------------*/
