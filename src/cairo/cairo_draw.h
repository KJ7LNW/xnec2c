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

#ifndef CAIRO_DRAW_H
#define CAIRO_DRAW_H    1

#include "../common.h"
#include "../view/view_core.h"
#include "../prerender/prerender_state.h"
#include "../prerender/prerender_color.h"
#include "../render/render_dispatch.h"
#include "../render/render_surface.h"

#include "cairo_scenebuffer.h"

/*-----------------------------------------------------------------------
 * Cairo per-frame state, held by the surface producing the frame
 *----------------------------------------------------------------------*/

/* Deferred axis label for post-flush Pango rendering */
typedef struct
{
  int         x, y;    /* screen position (axis tip) */
  const char *text;    /* label string (points into static table) */
} axis_label_t;

#define AXIS_COUNT 3

typedef struct
{
  /* Frame resources, written at draw entry */
  cairo_t                *cr;

  /* Frame colors deposited by render() through set_colors. */
  rgb_f_t                 background;
  rgb_f_t                 view_axis;
  rgb_f_t                 view_axis_label;

  /* Render-internal fields (set during render(), consumed by render_cairo) */
  axis_label_t     axis_labels[AXIS_COUNT];
  int              n_axis_labels;
  const char      *status_message; /* deferred status text; painted after flush */
  cairo_surface_t *gradient;       /* resolved gradient legend; NULL = skip */
} cairo_render_ctx_t;

/* One Cairo drawing surface: the generic base extended with the retained
 * scene the frame deposits into and the frame state itself. */
typedef struct
{
  render_surface_t     base;
  cairo_scenebuffer_t  scenebuffer;  /* retained allocation, reset per frame */
  cairo_render_ctx_t   frame;        /* per-draw; cr written at draw entry */

} cairo_engine_surface_t;

/**
 * cairo_engine_surface() - Reach the Cairo surface object from its base
 * @surface: surface a Cairo engine produced frames for
 */
static inline cairo_engine_surface_t *
cairo_engine_surface(render_surface_t *surface)
{
  return( (cairo_engine_surface_t *)surface );
}

/*-----------------------------------------------------------------------
 * Cairo drawing primitives (cairo_project.c)
 *----------------------------------------------------------------------*/

void Cairo_Draw_Polygon(cairo_t *cr, GdkPoint *points, int npoints);
void Cairo_Draw_Line(cairo_t *cr, int x1, int y1, int x2, int y2);
void Cairo_Draw_Lines(cairo_t *cr, GdkPoint *points, int npoints);

/*-----------------------------------------------------------------------
 * 3D to 2D projection and structure data (cairo_project.c)
 *----------------------------------------------------------------------*/

extern Segment_t *structure_segs;

void Process_Wire_Segments(view_t *v, double scale);
void Process_Surface_Patches(view_t *v, double scale);

/*-----------------------------------------------------------------------
 * Axes (cairo_axes.c)
 *----------------------------------------------------------------------*/

void cairo_draw_axes(render_surface_t *surface, float extent);

/*-----------------------------------------------------------------------
 * Cairo renderer leaf functions (for render_ops_t vtable)
 *----------------------------------------------------------------------*/

gboolean cairo_draw_structure(render_surface_t *surface, float extent,
    const struct_draw_params_t *params);
gboolean cairo_draw_farfield(render_surface_t *surface, int fstep,
    const ff_draw_params_t *ff);
gboolean cairo_draw_nearfield(render_surface_t *surface,
    const near_field_point_t *origins, int npts,
    const nf_field_set_t *fields, int n_fields,
    double dr, double r_max);
void     cairo_set_status(render_surface_t *surface, const char *msg);
void     cairo_set_gradient(render_surface_t *surface,
    const gradient_result_t *result);
void     cairo_set_colors(render_surface_t *surface,
    const render_frame_colors_t *colors);

#endif
