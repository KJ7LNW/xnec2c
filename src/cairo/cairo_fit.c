/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <math.h>

#include "cairo_fit.h"
#include "cairo_frame.h"
#include "../render/render_fit.h"
#include "../render/render_geometry.h"
#include "../shared.h"
#include "../view/view_core.h"

typedef struct
{
  view_t           *view;
  render_fit_acc_t *acc;

} cairo_fit_ctx_t;

/** cairo_fit_sink() - Fold one drawn world-space point into the reducer
 * @user:  Cairo fit context supplying the view basis and accumulator
 * @x:     world-space x coordinate
 * @y:     world-space y coordinate
 * @z:     world-space z coordinate
 * @scale: model scale applied by the renderer
 */
  static void
cairo_fit_sink(void *user, double x, double y, double z, float scale)
{
  cairo_fit_ctx_t *ctx = user;
  double screen_x;
  double screen_y;
  float screen_z;

  Project_on_Screen(ctx->view, x, y, z, &screen_x, &screen_y, &screen_z);
  render_fit_add(ctx->acc, (float)screen_x * scale,
      (float)screen_y * scale);

} /* cairo_fit_sink() */

/** cairo_fit_view() - Fit a Cairo view to its drawn geometry
 * @view: structure or radiation-pattern view
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when no geometry is available or the viewport is degenerate.
 */
  gboolean
cairo_fit_view(view_t *view, view_fit_t *fit)
{
  render_fit_acc_t acc = {0};
  render_fit_frame_t frame;
  const render_proj_t projection =
  {
    .kind = RENDER_PROJ_PARALLEL,
    .fov_rad = 0.0f,
    .aspect = 0.0f
  };
  cairo_fit_ctx_t ctx = { .view = view, .acc = &acc };
  float extent = 0.0f;
  double zoom = 0.0;
  double pan_x = 0.0;
  double pan_y = 0.0;
  gboolean solved = FALSE;

  if( view == NULL || fit == NULL )
    return FALSE;

  if( view->width <= 2 * RENDER_FIT_BORDER_PX
      || view->height <= 2 * RENDER_FIT_BORDER_PX )
    return FALSE;

  g_rec_mutex_lock(&freq_data_lock);

  extent = render_geom_walk(view, cairo_fit_sink, &ctx);
  if( acc.any )
    solved = render_fit_solve(&acc, &projection, view->height, &frame);

  if( solved && fl_fgt(extent, 0.0f) )
  {
    double base = view_projection_scale(view, extent, 1.0f);
    double border = (double)RENDER_FIT_BORDER_PX;
    double available_x = (double)view->width / 2.0 - border;
    double available_y = (double)view->height / 2.0 - border;
    double pixels_x = (double)frame.half_x * base;
    double pixels_y = (double)frame.half_y * base;

    if( !dl_fgt(pixels_x, 0.0) && !dl_fgt(pixels_y, 0.0) )
    {
      solved = FALSE;
    }
    else
    {
      double zoom_x = dl_fgt(pixels_x, 0.0)
          ? available_x / pixels_x : G_MAXDOUBLE;
      double zoom_y = dl_fgt(pixels_y, 0.0)
          ? available_y / pixels_y : G_MAXDOUBLE;
      double final_scale;

      zoom = fmin(zoom_x, zoom_y);
      final_scale = view_projection_scale(view, extent, (float)zoom);
      pan_x = -(double)frame.cx * final_scale;
      pan_y = -(double)frame.cy * final_scale;
    }
  }
  else
  {
    solved = FALSE;
  }

  g_rec_mutex_unlock(&freq_data_lock);

  if( acc.any && !solved )
    pr_warn("cairo_fit_view: no frame solved: extent=%g box=[%g,%g,%g,%g]"
        " view=%dx%d\n", (double)extent, (double)acc.minx, (double)acc.miny,
        (double)acc.maxx, (double)acc.maxy, view->width, view->height);

  if( solved )
  {
    fit->zoom = (float)zoom;
    fit->pan_offset[0] = (float)pan_x;
    fit->pan_offset[1] = (float)pan_y;
  }

  return solved;

} /* cairo_fit_view() */

/* Cairo engine control-operation vtable. */
const render_engine_ops_t cairo_engine_ops =
{
  .fit_view = cairo_fit_view,
  .capture = cairo_capture_pixbuf,
  .queue_redraw = gtk_widget_queue_draw,
};
