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

#ifdef HAVE_OPENGL

#include "opengl_structure_geometry.h"
#include "opengl_state.h"
#include "../opengl-engine/opengl_view.h"
#include "../render/render_dispatch.h"
#include "../render/render_wire_radius.h"
#include "../structure_ui.h"

/* Vertex attribute layout shared by radiation-pattern point batches */
static const gl_vertex_attrib_t opengl_lit_point_attribs[] = {
  { "position", 3, (int)offsetof(lit_color_point_t, point) },
  { "normal",   3, (int)offsetof(lit_color_point_t, normal) },
  { "color",    4, (int)offsetof(lit_color_point_t, color) },
  { NULL, 0, 0 }
};

/* Vertex attribute layout for structure_vertex_t */
static const gl_vertex_attrib_t opengl_structure_attribs[] = {
  { "position",   3, (int)offsetof(structure_vertex_t, point) },
  { "normal",     3, (int)offsetof(structure_vertex_t, normal) },
  { "color",      4, (int)offsetof(structure_vertex_t, color) },
  { "uv",         2, (int)offsetof(structure_vertex_t, uv) },
  { "direction",  2, (int)offsetof(structure_vertex_t, direction) },
  { "magnitude",  1, (int)offsetof(structure_vertex_t, magnitude) },
  { NULL, 0, 0 }
};

const gl_vertex_layout_t opengl_lit_point_layout = {
  .attribs = opengl_lit_point_attribs,
  .stride = (int)sizeof(lit_color_point_t)
};

const gl_vertex_layout_t opengl_structure_layout = {
  .attribs = opengl_structure_attribs,
  .stride = (int)sizeof(structure_vertex_t)
};

/*-----------------------------------------------------------------------*/

/**
 * gl_store_structure_content() - Populate one prepared GL content owner
 * @out: primary or overlay content owner
 * @params: parent-resolved structure presentation parameters
 * @wire_geometry: segment geometry of the selected wire form
 */
  static void
gl_store_structure_content(gl_view_content_t *out,
    const struct_draw_params_t *params,
    const gl_wire_geometry_t *wire_geometry)
{
  const structure_overlay_data_t *geom;

  opengl_structure_update_shared_geometry_with_params(params, wire_geometry);
  geom = opengl_structure_get_shared_geometry();

  memcpy(out->batches, geom->batches,
      (size_t)geom->batch_count * sizeof(geom->batches[0]));
  out->batch_count = geom->batch_count;
  out->layout = geom->layout;
  out->r_max = (geom->batch_count > 0) ? params->geometry_extent : 1.5f;
  out->clip_extent = out->r_max;
  out->model_scale = params->model_scale;

} /* gl_store_structure_content() */

/**
 * gl_draw_structure() - Populate primary structure content
 * @surface: GL surface holding the frame content
 * @params: parent-resolved structure presentation parameters
 * @wire_geometry: segment geometry of the selected wire form
 */
  static gboolean
gl_draw_structure(render_surface_t *surface,
    const struct_draw_params_t *params,
    const gl_wire_geometry_t *wire_geometry)
{
  gl_view_state_t *state = gl_view_state(surface);

  gl_store_structure_content(&state->content, params, wire_geometry);

  return TRUE;

} /* gl_draw_structure() */

/**
 * gl_draw_structure_overlay() - Populate secondary structure content
 * @surface: GL surface holding the frame content
 * @params: parent-resolved overlay presentation parameters
 * @wire_geometry: segment geometry of the selected wire form
 */
  static gboolean
gl_draw_structure_overlay(render_surface_t *surface,
    const struct_draw_params_t *params,
    const gl_wire_geometry_t *wire_geometry)
{
  gl_view_state_t *state = gl_view_state(surface);

  if( state->overlay_content == NULL )
  {
    BUG("OpenGL structure overlay has no content owner\n");
    return FALSE;
  }

  gl_store_structure_content(state->overlay_content, params, wire_geometry);

  return TRUE;

} /* gl_draw_structure_overlay() */

/*-----------------------------------------------------------------------*/

/**
 * gl_draw_structure_line() - Populate primary structure content as lines
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved structure presentation parameters
 */
  gboolean
gl_draw_structure_line(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  return( gl_draw_structure(surface, params, &gl_wire_geometry_line) );
}

/**
 * gl_draw_structure_solid() - Populate primary structure content as cylinders
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved structure presentation parameters
 */
  gboolean
gl_draw_structure_solid(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  return( gl_draw_structure(surface, params, &gl_wire_geometry_solid) );
}

/**
 * gl_draw_structure_overlay_line() - Populate secondary structure content as lines
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved overlay presentation parameters
 */
  gboolean
gl_draw_structure_overlay_line(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  return( gl_draw_structure_overlay(surface, params,
        &gl_wire_geometry_line) );
}

/**
 * gl_draw_structure_overlay_solid() - Populate secondary structure content as cylinders
 * @surface: GL surface holding the frame content
 * @_extent: Cairo projection extent unused by OpenGL
 * @params: parent-resolved overlay presentation parameters
 */
  gboolean
gl_draw_structure_overlay_solid(render_surface_t *surface, float _extent,
    const struct_draw_params_t *params)
{
  return( gl_draw_structure_overlay(surface, params,
        &gl_wire_geometry_solid) );
}

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

/* Scroll capabilities of the structure domain */
static surface_capability_t *const structure_input_caps[] = {
  &wire_radius_cap,
  NULL
};

/* Static view configuration */
static gl_view_config_t structure_view_config = {
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
  return( gl_view_surface_new(&structure_view_config, structure_input_caps,
        structure_view, parent) );
}

#endif /* HAVE_OPENGL */

