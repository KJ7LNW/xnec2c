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
 * opengl_view_fit: GL fit-to-view control-op leaf.
 *
 * Fit the batches drawn by this view using its current projection. The
 * tessellated cylinder walls carry the displayed segment radius, and the
 * fit uses the same model scale and rotation as the frame.
 */

#include "opengl_view_fit.h"
#include "opengl_view_fit_geometry.h"

#ifdef HAVE_OPENGL

#include "opengl_view.h"
#include "../opengl/opengl_structure.h"
#include "../render/render_fit.h"
#include "../view/view_core.h"
#include "../shared.h"

#include <math.h>

/*-----------------------------------------------------------------------*/

/** gl_view_fit_parallel() - Fit the rotated bounding box in parallel projection */
  static gboolean
gl_view_fit_parallel(gl_view_state_t *state, view_fit_t *fit)
{
  view_t *view = state->base.view;
  render_fit_acc_t acc = {0};
  render_fit_frame_t frame;
  float half_width = (float)view->width * 0.5f;
  float half_height = (float)view->height * 0.5f;
  float available_width = half_width - (float)RENDER_FIT_BORDER_PX;
  float available_height = half_height - (float)RENDER_FIT_BORDER_PX;
  float tangent = tanf(state->fov_rad * 0.5f);
  float aspect = (float)view->width / (float)view->height;
  float world_half_x, world_half_y, distance, pan_scale;
  float r_max = state->content.r_max;

  if( !gl_view_fit_fold_box(view, state, &acc) || !acc.any
      || !render_fit_solve(&acc, &frame) )
    return FALSE;

  world_half_x = frame.half_x * half_width / (available_width * aspect);
  world_half_y = frame.half_y * half_height / available_height;
  distance = fmaxf(world_half_x, world_half_y) / tangent;
  pan_scale = 2.0f * distance * tangent / (float)view->height;

  if( !fl_fgt(tangent, 0.0f) || !fl_fgt(distance, 0.0f)
      || !fl_fgt(r_max, 0.0f) || !fl_fgt(pan_scale, 0.0f) )
  {
    pr_warn("no frame solved: r_max=%g distance=%g proj=%d"
        " box=[%g,%g,%g,%g]\n", (double)r_max, (double)distance, 0,
        (double)acc.minx, (double)acc.miny, (double)acc.maxx, (double)acc.maxy);
    return FALSE;
  }

  fit->zoom = r_max * GL_VIEW_BASE_DISTANCE_FACTOR / distance;
  fit->pan_offset[0] = -frame.cx / pan_scale;
  fit->pan_offset[1] = -frame.cy / pan_scale;
  return TRUE;

} /* gl_view_fit_parallel() */

/** gl_view_fit_view() - Fit a GL view to its drawn geometry
 * @surface: surface presenting the structure or radiation-pattern view
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Return: FALSE when the active projection has no frame to fit.
 */
  static gboolean
gl_view_fit_view(render_surface_t *surface, view_fit_t *fit)
{
  gl_view_state_t *state;
  view_t *view;
  gboolean solved;

  if( surface == NULL || fit == NULL )
    return FALSE;

  state = gl_view_state(surface);
  view = surface->view;

  if( view == NULL
      || view->width <= 2 * RENDER_FIT_BORDER_PX
      || view->height <= 2 * RENDER_FIT_BORDER_PX
      || !fl_fgt(state->fov_rad, 0.0f) )
    return FALSE;

  // Keep the rendered batches stable while solving for the camera.
  g_rec_mutex_lock(&freq_data_lock);
  solved = rc_config.opengl_orthographic
      ? gl_view_fit_parallel(state, fit) : gl_view_fit_perspective(state, fit);
  g_rec_mutex_unlock(&freq_data_lock);

  return solved;

} /* gl_view_fit_view() */

/*-----------------------------------------------------------------------*/

/** gl_view_surface_queue_redraw() - Request a frame through the engine contract
 * @surface: surface to repaint
 *
 * Adapts the generic frame request, which names a surface, to the engine's
 * own request, which names the state producing its frames.
 */
  static void
gl_view_surface_queue_redraw(render_surface_t *surface)
{
  gl_view_queue_render( gl_view_state(surface) );

} /* gl_view_surface_queue_redraw() */

/*-----------------------------------------------------------------------*/

/* Compose the OpenGL domain protocol and active-surface operations. */
const render_engine_t gl_engine =
{
  .render = &gl_ops,
  .name = "opengl",
  .surface_free = gl_view_surface_free,
  .fit_view = gl_view_fit_view,
  .frame = gl_view_frame,
  .capture = gl_view_capture_pixbuf,
  .queue_redraw = gl_view_surface_queue_redraw,
  .surface_resize = gl_view_surface_resize,
};

#endif /* HAVE_OPENGL */
