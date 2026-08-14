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

#include "opengl_structure.h"
#include "opengl_rdpattern.h"
#include "../shared.h"
#include "../gdk_scroll.h"

#ifdef HAVE_OPENGL

#include "opengl_structure_geometry.h"
#include "opengl_state.h"
#include "../settings/render_settings.h"
#include "../opengl-engine/opengl_view.h"
#include "../render/render_dispatch.h"
#include "../rdpattern_ui.h"
#include "../structure_ui.h"

/* Maximum allowed radius scale */
#define CYLINDER_RADIUS_SCALE_MAX 100.0

/* Mutable cylinder radius scale factor (user-adjustable via Ctrl+scroll) */
static double cylinder_radius_scale = 1.0;

/* Vertex attribute layout for lit-color shader (shared with overlay consumers) */
const gl_vertex_attrib_t opengl_structure_attribs[3] = {
  { "position", 3, 0 },
  { "normal",   3, 4 * (int)sizeof(float) },
  { "color",    4, 8 * (int)sizeof(float) }
};

/* Vertex attribute layout for chevron shader (structure_vertex_t) */
const gl_vertex_attrib_t opengl_chevron_attribs[7] = {
  { "position",   3, 0 },
  { "normal",     3, 4 * (int)sizeof(float) },
  { "color",      4, 8 * (int)sizeof(float) },
  { "uv",         2, 12 * (int)sizeof(float) },
  { "flow_data",  4, 14 * (int)sizeof(float) },
  { "tangent1",   3, 18 * (int)sizeof(float) },
  { "tangent2",   3, 21 * (int)sizeof(float) }
};

/*-----------------------------------------------------------------------*/

/** opengl_structure_get_radius_scale() - Return current cylinder radius display scale factor
 */
  double
opengl_structure_get_radius_scale(void)
{
  return( cylinder_radius_scale );

} /* opengl_structure_get_radius_scale() */

/*-----------------------------------------------------------------------*/

/** opengl_structure_set_radius_scale() - Set cylinder radius display scale factor
 * @scale: new scale value, clamped to [0, CYLINDER_RADIUS_SCALE_MAX]
 *
 * Syncs to rc_config; geometry regenerates on next render.
 */
  void
opengl_structure_set_radius_scale(double scale)
{
  if( scale < 0.0 )
  {
    scale = 0.0;
  }

  if( scale > CYLINDER_RADIUS_SCALE_MAX )
  {
    scale = CYLINDER_RADIUS_SCALE_MAX;
  }

  cylinder_radius_scale = scale;
  rc_config.opengl_cylinder_radius_scale = scale;

} /* opengl_structure_set_radius_scale() */

/*-----------------------------------------------------------------------*/

/** opengl_structure_on_ctrl_scroll() - Ctrl+scroll handler for adjusting cylinder radius scale
 * @event: scroll event
 * @state: view state of the scrolled view
 *
 * Shared by the structure and rdpattern input rows.
 */
  gboolean
opengl_structure_on_ctrl_scroll(
    GdkEventScroll *event, gl_view_state_t *state)
{
  double scale, new_scale;

  scroll_step_t s;

  if( !state )
    return( FALSE );

  s = scroll_step_from_deltas((GdkEvent *)event);

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  /* Compute increment matching zoom scroll feel */
  scale = compute_zoom_scale(
      state->base.view->width,
      state->base.view->height,
      (cylinder_radius_scale > 0.1 ? cylinder_radius_scale : 0.1) * 100.0);

  new_scale = cylinder_radius_scale;

  if( s.direction == GDK_SCROLL_UP )
  {
    /* Scroll up: thicker. If at zero, jump to threshold. */
    if( new_scale < CYLINDER_SCALE_LINE_THRESHOLD )
    {
      new_scale = CYLINDER_SCALE_LINE_THRESHOLD;
    }
    else
    {
      new_scale *= (1.0 + 0.1 * s.step * scale);
    }
  }
  else if( s.direction == GDK_SCROLL_DOWN )
  {
    /* Scroll down: thinner. Below threshold snaps to zero (line mode). */
    new_scale /= (1.0 + 0.1 * s.step * scale);

    if( new_scale < CYLINDER_SCALE_LINE_THRESHOLD )
    {
      new_scale = 0.0;
    }
  }

  /* The registered post_apply hook owns the clamp, the backend apply, and
   * the repaint of both windows */
  rc_config.opengl_cylinder_radius_scale = new_scale;
  hook_set_radius_scale();

  render_settings_sync_from_config();

  return( TRUE );

} /* opengl_structure_on_ctrl_scroll() */

/*-----------------------------------------------------------------------*/

const char opengl_structure_ctrl_scroll_notice[] = "Ctrl+Scroll: Wire Radius";

/*-----------------------------------------------------------------------*/

/**
 * gl_store_structure_content() - Populate one prepared GL content owner
 * @out: primary or overlay content owner
 * @params: parent-resolved structure presentation parameters
 */
  static void
gl_store_structure_content(gl_view_content_t *out,
    const struct_draw_params_t *params)
{
  const structure_overlay_data_t *geom;

  opengl_structure_update_shared_geometry_with_params(params);
  geom = opengl_structure_get_shared_geometry();

  memcpy(out->batches, geom->batches,
      (size_t)geom->batch_count * sizeof(geom->batches[0]));
  out->batch_count = geom->batch_count;
  out->vertex_stride = geom->vertex_stride;
  out->r_max = (geom->batch_count > 0) ? params->geometry_extent : 1.5f;
  out->clip_extent = out->r_max;
  out->model_scale = params->model_scale;
  out->generation = geom->generation;

} /* gl_store_structure_content() */

/**
 * gl_draw_structure() - Populate primary structure content
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved structure presentation parameters
 */
  gboolean
gl_draw_structure(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  gl_view_state_t *state = gl_view_state(surface);

  gl_store_structure_content(&state->content, params);

  return TRUE;

} /* gl_draw_structure() */

/**
 * gl_draw_structure_overlay() - Populate secondary structure content
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved overlay presentation parameters
 */
  gboolean
gl_draw_structure_overlay(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  gl_view_state_t *state = gl_view_state(surface);

  if( state->overlay_content == NULL )
  {
    BUG("OpenGL structure overlay has no content owner\n");
    return FALSE;
  }

  gl_store_structure_content(state->overlay_content, params);

  return TRUE;

} /* gl_draw_structure_overlay() */

/*-----------------------------------------------------------------------*/

/* gl_ops defined in opengl_ops.c; declared in opengl_structure.h */

/*-----------------------------------------------------------------------*/

/** structure_content_cleanup() - Release the structure geometry caches
 */
  static void
structure_content_cleanup(void)
{
  opengl_structure_geometry_cleanup();
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_ground_plane_is_active() - Report model ground visibility
 * @_ctx: unused renderable context
 */
  static gboolean
opengl_structure_ground_plane_is_active(void *_ctx)
{
  (void)_ctx;

  return( gnd_has_real_ground() );

} /* opengl_structure_ground_plane_is_active() */

/*-----------------------------------------------------------------------*/

/* Static view configuration */
static const gl_view_input_ops_t structure_input_ops = {
  .on_shift_scroll     = NULL,
  .on_ctrl_scroll      = opengl_structure_on_ctrl_scroll,
  .ctrl_scroll_notice  = opengl_structure_ctrl_scroll_notice
};

static gl_view_config_t structure_view_config = {
  .vertex_shader_path = "/gl/lit-color-vertex.glsl",
  .fragment_shader_path = "/gl/lit-color-fragment.glsl",
  .attribs = opengl_chevron_attribs,
  .attrib_count = 7,
  .vertex_stride = (int)sizeof(structure_vertex_t),
  .input = &structure_input_ops,
  .overlay = NULL,
  .ground_plane_is_active = opengl_structure_ground_plane_is_active,
  .on_gl_init_failed = opengl_gl_init_failed,
  .content_cleanup = structure_content_cleanup
};

/*-----------------------------------------------------------------------*/

/** opengl_structure_surface_new() - Build the structure GL surface
 * @parent: container the presented widget joins
 */
  render_surface_t *
opengl_structure_surface_new(GtkContainer *parent)
{
  /* Load persisted radius scale from config; zero means line mode */
  cylinder_radius_scale = rc_config.opengl_cylinder_radius_scale;

  return( gl_view_surface_new(&structure_view_config, structure_view,
        parent) );
}

#endif /* HAVE_OPENGL */

/*-----------------------------------------------------------------------*/

/** opengl_structure_invalidate() - Public API: mark cached geometry stale
 *
 * Forces regeneration from current NEC2 data arrays on next render.
 * Call after geometry reload.
 */
  void
opengl_structure_invalidate(void)
{
#ifdef HAVE_OPENGL
  opengl_structure_geometry_invalidate();
#endif
}

/*-----------------------------------------------------------------------*/
