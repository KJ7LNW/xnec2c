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

/* Flow direction visualization mode for patch currents */
typedef enum
{
  FLOW_DIR_REFERENCE_PHASE = 0,
  FLOW_DIR_POLARIZATION_TILT,
  FLOW_DIR_PEAK_MAGNITUDE,
  FLOW_DIR_LIC,
  FLOW_DIR_WIREFRAME

} flow_direction_mode_t;

/* Shared structure geometry for overlay rendering */
typedef struct
{
  gl_draw_batch_t batches[GL_VIEW_MAX_BATCHES];
  int batch_count;
  int vertex_stride;
  float view_scale;
  unsigned int generation;

} structure_overlay_data_t;

/* Public API - always available, stubs when no OpenGL */
void opengl_structure_invalidate(void);

/* Scale below which segments render as lines instead of cylinders with 0.001 epsilon for slider */
#define CYLINDER_SCALE_LINE_THRESHOLD 0.101

#ifdef HAVE_OPENGL
#include "../opengl-engine/opengl_renderer.h"
#include "../opengl-engine/opengl_view.h"
#include "../view/view_core.h"

/* Extended vertex with UV and flow data for chevron shader.
 * First 48 bytes are layout-identical to lit_color_point_t. */
typedef struct
{
  point_f_3d_t point;
  point_f_3d_t normal;
  rgba_f_t color;
  float uv[2];
  float flow_data[4];   /* Re(ct1), Im(ct1), Re(ct2), Im(ct2) */

  /* Patch tangent frame for GPU-driven arrow rotation.
   * When non-zero, the vertex shader interprets:
   *   point  = patch center (not world position)
   *   uv     = arrow template position in UV space (not texture coords)
   * and computes world_pos = center + (2*rot_uv - 1) · tangent_frame
   * where rot_uv is the template UV rotated by the phase-derived flow angle.
   * When zero, point and uv retain their standard meanings. */
  float tangent1[3];    /* s * t1 (half-side scaled tangent vector 1) */
  float tangent2[3];    /* s * t2 (half-side scaled tangent vector 2) */

} structure_vertex_t;

/* Vertex attribute layout for lit-color shader (structure rendering) */
extern const gl_vertex_attrib_t opengl_structure_attribs[3];

/* Vertex attribute layout for chevron shader (structure_vertex_t: 7 attribs) */
extern const gl_vertex_attrib_t opengl_chevron_attribs[7];

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

/* Ctrl+scroll handler for adjusting cylinder radius scale.
 * Usable by any view that presents structure geometry. */
gboolean opengl_structure_on_ctrl_scroll(
    GdkEventScroll *event, render_surface_t *surface);

/* Notice advertising the ctrl+scroll radius capability */
extern const char opengl_structure_ctrl_scroll_notice[];

/* Update shared geometry buffer using dispatch-resolved draw parameters. */
void opengl_structure_update_shared_geometry_with_params(const struct_draw_params_t *params);

/* Return read-only pointer to shared geometry data */
const structure_overlay_data_t* opengl_structure_get_shared_geometry(void);

#endif

#endif /* OPENGL_STRUCTURE_H */
