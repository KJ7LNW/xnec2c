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
 * Folds the model-space vertices of the batches this view drew — the
 * tessellated cylinder walls carry the displayed segment radius, and
 * model_scale is baked in by the same rotation the frame uses — into the
 * generic reducer, then inverts the framing into the view's zoom and pan
 * through the shared currency (zoom spin and pan offset).
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

/** gl_view_fit_view() - Fit a GL view to its drawn geometry
 * @surface: surface presenting the structure or radiation-pattern view
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Folds the GL content batches this view drew (radius scaling and model_scale
 * baked in) into the generic reducer, then inverts the framing into the view's
 * zoom and pan.  Returns FALSE when no geometry is present.
 */
  static gboolean
gl_view_fit_view(render_surface_t *surface, view_fit_t *fit)
{
  gl_view_state_t *state;
  view_t *view;
  render_fit_acc_t acc = {0};
  render_fit_frame_t frame;
  render_proj_t projection;
  float viewport_height;
  float r_max = 0.0f;
  float distance = 0.0f;
  float zoom = 0.0f;
  float pan_x = 0.0f;
  float pan_y = 0.0f;
  gboolean solved = FALSE;

  if( surface == NULL || fit == NULL )
    return FALSE;

  state = gl_view_state(surface);
  view = surface->view;

  if( view == NULL
      || view->width <= 2 * RENDER_FIT_BORDER_PX
      || view->height <= 2 * RENDER_FIT_BORDER_PX
      || !fl_fgt(state->fov_rad, 0.0f) )
    return FALSE;

  viewport_height = (float)view->height;

  projection.kind = rc_config.opengl_orthographic
      ? RENDER_PROJ_PARALLEL : RENDER_PROJ_PERSPECTIVE;
  projection.fov_rad = state->fov_rad;
  projection.aspect = (float)view->width / viewport_height;

  g_rec_mutex_lock(&freq_data_lock);
  r_max = state->content.r_max;

  if( gl_view_fit_fold_box(view, state, &acc) && acc.any )
  {
    if( projection.kind == RENDER_PROJ_PERSPECTIVE )
      gl_view_fit_fold_perspective(view, state, &projection, &acc,
          0.5f * (acc.minx + acc.maxx),
          0.5f * (acc.miny + acc.maxy));

    solved = render_fit_solve(&acc, &projection, view->height, &frame);
  }

  if( solved )
  {
    switch( projection.kind )
    {
      case RENDER_PROJ_PARALLEL:
      {
        float half_width = (float)view->width * 0.5f;
        float half_height = (float)view->height * 0.5f;
        float available_width = half_width - (float)RENDER_FIT_BORDER_PX;
        float available_height = half_height - (float)RENDER_FIT_BORDER_PX;
        float tangent = tanf(projection.fov_rad * 0.5f);
        float world_half_x = frame.half_x * half_width
            / (available_width * projection.aspect);
        float world_half_y = frame.half_y * half_height / available_height;

        if( fl_fgt(tangent, 0.0f) )
          distance = fmaxf(world_half_x, world_half_y) / tangent;
        else
          solved = FALSE;
        break;
      }

      case RENDER_PROJ_PERSPECTIVE:
        distance = frame.dist;
        break;

      case RENDER_PROJ_COUNT:
        BUG("gl_view_fit_view: invalid projection kind %d\n", projection.kind);
        solved = FALSE;
        break;
    }
  }

  if( solved && fl_fgt(distance, 0.0f) && fl_fgt(r_max, 0.0f) )
  {
    zoom = r_max * GL_VIEW_BASE_DISTANCE_FACTOR / distance;

    switch( projection.kind )
    {
      case RENDER_PROJ_PARALLEL:
      {
        float pan_scale = 2.0f * distance
            * tanf(projection.fov_rad * 0.5f) / viewport_height;

        if( fl_fgt(pan_scale, 0.0f) )
        {
          pan_x = -frame.cx / pan_scale;
          pan_y = -frame.cy / pan_scale;
        }
        else
        {
          solved = FALSE;
        }
        break;
      }

      case RENDER_PROJ_PERSPECTIVE:
      {
        render_fit_acc_t ndc = {0};

        if( gl_view_fit_fold_ndc(view, state, &projection, distance, &ndc)
            && ndc.any )
        {
          pan_x = -0.5f * (ndc.minx + ndc.maxx)
              * viewport_height * projection.aspect * 0.5f;
          pan_y = -0.5f * (ndc.miny + ndc.maxy)
              * viewport_height * 0.5f;
        }
        else
        {
          solved = FALSE;
        }
        break;
      }

      case RENDER_PROJ_COUNT:
        BUG("gl_view_fit_view: invalid projection kind %d\n", projection.kind);
        solved = FALSE;
        break;
    }
  }
  else
  {
    solved = FALSE;
  }

  g_rec_mutex_unlock(&freq_data_lock);

  if( acc.any && !solved )
    pr_warn("gl_view_fit_view: no frame solved: r_max=%g distance=%g proj=%d"
        " box=[%g,%g,%g,%g]\n", (double)r_max, (double)distance,
        (int)projection.kind, (double)acc.minx, (double)acc.miny,
        (double)acc.maxx, (double)acc.maxy);

  if( solved )
  {
    fit->zoom = zoom;
    fit->pan_offset[0] = pan_x;
    fit->pan_offset[1] = pan_y;
  }

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
  .surface_free = gl_view_surface_free,
  .fit_view = gl_view_fit_view,
  .capture = gl_view_capture_pixbuf,
  .queue_redraw = gl_view_surface_queue_redraw,
};

#endif /* HAVE_OPENGL */
