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

#ifndef OPENGL_STRUCTURE_H
#define OPENGL_STRUCTURE_H 1

#include "common.h"
#include "../render/render_dispatch.h"
#include "../render/render_surface_input.h"

/* Public API - always available, stubs when no OpenGL */
void opengl_structure_invalidate(void);

/* Scale below which segments render as lines instead of cylinders with 0.001 epsilon for slider */
#define CYLINDER_SCALE_LINE_THRESHOLD 0.101

#ifdef HAVE_OPENGL
#include "../opengl-engine/opengl_renderer.h"
#include "../opengl-engine/opengl_view.h"
#include "../view/view_core.h"

/* Shared structure geometry for overlay rendering */
typedef struct
{
  gl_draw_batch_t batches[GL_VIEW_MAX_BATCHES];
  int batch_count;
  const gl_vertex_layout_t *layout;
  float view_scale;

} structure_overlay_data_t;

/* Extended vertex with UV and resolved flow for the mark shaders.
 * First 48 bytes are layout-identical to lit_color_point_t. */
typedef struct
{
  point_f_3d_t point;
  point_f_3d_t normal;
  rgba_f_t color;
  float uv[2];

  /* Tangent-plane flow the render layer resolved for this patch; the
   * magnitude is zero on every vertex carrying no flow. */
  float direction[2];
  float magnitude;

} structure_vertex_t;

/* Vertex layouts shared by radiation-pattern points and structure geometry */
extern const gl_vertex_layout_t opengl_lit_point_layout;
extern const gl_vertex_layout_t opengl_structure_layout;

/* Build the structure GL surface and pack it into @parent */
render_surface_t *opengl_structure_surface_new(GtkContainer *parent);

/* GL structure leaf renderers; exported for unified gl_ops vtable */
gboolean gl_draw_structure(render_surface_t *surface, float extent,
    const struct_draw_params_t *params);
gboolean gl_draw_structure_overlay(render_surface_t *surface, float extent,
    const struct_draw_params_t *params);

/* Unified GL backend vtable (defined in opengl_ops.c) */
extern const render_ops_t gl_ops;

/* Get/set cylinder radius display scale factor */
double opengl_structure_get_radius_scale(void);
void opengl_structure_set_radius_scale(double scale);

/* Ctrl+scroll capability adjusting cylinder radius scale.
 * Offered by any view that presents structure geometry. */
extern surface_capability_t opengl_structure_wire_radius_cap;

/* Update shared geometry buffer using dispatch-resolved draw parameters. */
void opengl_structure_update_shared_geometry_with_params(const struct_draw_params_t *params);

/* Return read-only pointer to shared geometry data */
const structure_overlay_data_t* opengl_structure_get_shared_geometry(void);

#endif

#endif /* OPENGL_STRUCTURE_H */
