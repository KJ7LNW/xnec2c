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
 * opengl_patch_flow: OpenGL presentation of one resolved patch-flow frame.
 *
 * The render layer resolves direction, magnitude, and world-space arrows
 * above both engines, so this module only selects the geometry a fill emits
 * and the compiled program a mark draws with.
 */
#include "opengl_patch_flow.h"
#include "../mem/mem.h"
#include "../shared.h"
#include "../prerender/prerender_state.h"

#ifdef HAVE_OPENGL

#define PATCH_OUTLINE_VERTEX_COUNT 8
#define PATCH_SOLID_VERTEX_COUNT 6

typedef struct
{
  float normal[3];
  const rgb_f_t *color;
  patch_flow_entry_t flow;
} patch_shading_t;

static void opengl_patch_solid_generate(gl_draw_batch_t *batch,
    const struct_draw_params_t *params);
static void opengl_patch_wireframe_generate(gl_draw_batch_t *batch,
    const struct_draw_params_t *params);

/* Geometry emitted by one fill, with the primitive it draws as */
typedef struct
{
  void (*generate)(gl_draw_batch_t *batch,
      const struct_draw_params_t *params);
  unsigned int draw_mode;

} patch_fill_row_t;

static const patch_fill_row_t patch_fill_rows[] = {
  [PATCH_FILL_SOLID] = {
      .generate = opengl_patch_solid_generate,
      .draw_mode = GL_TRIANGLES },
  [PATCH_FILL_WIREFRAME] = {
      .generate = opengl_patch_wireframe_generate,
      .draw_mode = GL_LINES },
};

/* Compiled program presenting each mark */
static const gl_view_program_key_t patch_flow_mark_programs[] = {
  [PATCH_FLOW_MARK_DIRECTIONAL]   = GL_PROGRAM_FLOW_DIRECTIONAL,
  [PATCH_FLOW_MARK_BIDIRECTIONAL] = GL_PROGRAM_FLOW_BIDIRECTIONAL,
  [PATCH_FLOW_MARK_LIC]           = GL_PROGRAM_FLOW_LIC,
  [PATCH_FLOW_MARK_ARROW]         = GL_PROGRAM_LIT,
};

_Static_assert(G_N_ELEMENTS(patch_fill_rows) == PATCH_FILL_COUNT,
    "every patch fill names one geometry operation");
_Static_assert(G_N_ELEMENTS(patch_flow_mark_programs) == PATCH_FLOW_MARK_COUNT,
    "every patch flow mark names one compiled program");

/** patch_normal_resolve() - Resolve one patch surface normal */
static void
patch_normal_resolve(int patch, float normal[3])
{
  normal[0] = (float)(data.patches[patch].t1y * data.patches[patch].t2z
      - data.patches[patch].t1z * data.patches[patch].t2y);
  normal[1] = (float)(data.patches[patch].t1z * data.patches[patch].t2x
      - data.patches[patch].t1x * data.patches[patch].t2z);
  normal[2] = (float)(data.patches[patch].t1x * data.patches[patch].t2y
      - data.patches[patch].t1y * data.patches[patch].t2x);
}

/** patch_shading_resolve() - Gather one patch presentation
 * @patch: patch index
 * @params: structure frame supplying the resolved patch colors
 *
 * Returns the normal and color every patch vertex carries.  Flow rides the
 * fill that reads it, so it stays zero here.
 */
static patch_shading_t
patch_shading_resolve(int patch, const struct_draw_params_t *params)
{
  patch_shading_t shading = {0};

  patch_normal_resolve(patch, shading.normal);
  shading.color = &params->patch_colors[patch];

  return shading;
}

/** patch_vertex() - Compose one patch vertex */
static structure_vertex_t
patch_vertex(const patch_shading_t *shading,
    double x, double y, double z, float u, float v)
{
  return (structure_vertex_t){
      .point = {(float)x, (float)y, (float)z},
      .normal = {shading->normal[0], shading->normal[1], shading->normal[2]},
      .color = {shading->color->r, shading->color->g, shading->color->b, 1.0f},
      .uv = {u, v},
      .direction = {shading->flow.direction[0], shading->flow.direction[1]},
      .magnitude = shading->flow.magnitude };
}

/** opengl_patch_wireframe_generate() - Emit patch outlines and arrows */
static void
opengl_patch_wireframe_generate(gl_draw_batch_t *batch,
    const struct_draw_params_t *params)
{
  structure_vertex_t *vertices;
  int arrow_count = mem_array_count(params->patch_flow.arrows);
  int vertex = 0;
  int patch;
  int arrow;

  mem_realloc(&batch->vertices,
      (size_t)(data.m * PATCH_OUTLINE_VERTEX_COUNT
        + arrow_count * ARROW_VERTEX_COUNT) * sizeof(structure_vertex_t));
  vertices = batch->vertices;

  for( patch = 0; patch < data.m; patch++ )
  {
    const patch_corners_t *corners = &geom_pre.patch_corners[patch];
    patch_shading_t shading = patch_shading_resolve(patch, params);

    vertices[vertex++] = patch_vertex(&shading,
        corners->c0x, corners->c0y, corners->c0z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c1x, corners->c1y, corners->c1z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c1x, corners->c1y, corners->c1z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c2x, corners->c2y, corners->c2z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c2x, corners->c2y, corners->c2z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c3x, corners->c3y, corners->c3z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c3x, corners->c3y, corners->c3z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c0x, corners->c0y, corners->c0z, 0.0f, 0.0f);
  }

  for( arrow = 0; arrow < arrow_count; arrow++ )
  {
    const patch_flow_arrow_t *resolved = &params->patch_flow.arrows[arrow];
    patch_shading_t shading = { .color = &resolved->color };
    int segment;

    patch_normal_resolve(resolved->patch, shading.normal);

    for( segment = 0; segment < ARROW_LINE_COUNT; segment++ )
    {
      const flow_seg_3d_t *line = &resolved->segments[segment];

      vertices[vertex++] = patch_vertex(&shading,
          line->x1, line->y1, line->z1, 0.0f, 0.0f);
      vertices[vertex++] = patch_vertex(&shading,
          line->x2, line->y2, line->z2, 0.0f, 0.0f);
    }
  }

  batch->vertex_count = vertex;
}

/** opengl_patch_solid_generate() - Emit filled patch quads carrying flow */
static void
opengl_patch_solid_generate(gl_draw_batch_t *batch,
    const struct_draw_params_t *params)
{
  const patch_flow_entry_t *flow_entries = params->patch_flow.entries;
  structure_vertex_t *vertices;
  int vertex = 0;
  int patch;

  mem_realloc(&batch->vertices,
      (size_t)data.m * PATCH_SOLID_VERTEX_COUNT * sizeof(structure_vertex_t));
  vertices = batch->vertices;

  for( patch = 0; patch < data.m; patch++ )
  {
    const patch_corners_t *corners = &geom_pre.patch_corners[patch];
    patch_shading_t shading = patch_shading_resolve(patch, params);

    shading.flow = (flow_entries != NULL)
        ? flow_entries[patch] : (patch_flow_entry_t){0};

    vertices[vertex++] = patch_vertex(&shading,
        corners->c0x, corners->c0y, corners->c0z, 1.0f, 1.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c1x, corners->c1y, corners->c1z, 0.0f, 1.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c2x, corners->c2y, corners->c2z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c0x, corners->c0y, corners->c0z, 1.0f, 1.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c2x, corners->c2y, corners->c2z, 0.0f, 0.0f);
    vertices[vertex++] = patch_vertex(&shading,
        corners->c3x, corners->c3y, corners->c3z, 1.0f, 0.0f);
  }

  batch->vertex_count = vertex;
}

/**
 * opengl_patch_flow_generate() - Build the patch batch from a resolved frame
 * @batch: patch batch receiving vertices and draw configuration
 * @params: structure frame carrying patch geometry presentation inputs
 */
void
opengl_patch_flow_generate(gl_draw_batch_t *batch,
    const struct_draw_params_t *params)
{
  patch_fill_t fill = params->patch_flow.fill;
  patch_flow_mark_t mark = params->patch_flow.mark;

  if( geom_pre.patch_corners == NULL )
  {
    batch->vertex_count = 0;
    return;
  }

  if( fill < PATCH_FILL_SOLID || fill >= PATCH_FILL_COUNT )
  {
    BUG("Patch fill %d lies outside its defined range\n", fill);
    batch->vertex_count = 0;
    return;
  }

  if( mark < PATCH_FLOW_MARK_DIRECTIONAL || mark >= PATCH_FLOW_MARK_COUNT )
  {
    BUG("Patch flow mark %d lies outside its defined range\n", mark);
    batch->vertex_count = 0;
    return;
  }

  patch_fill_rows[fill].generate(batch, params);

  batch->draw_mode = patch_fill_rows[fill].draw_mode;
  batch->program_key = patch_flow_mark_programs[mark];
  batch->scroll_phase = (float)params->patch_flow.scroll_phase;

} /* opengl_patch_flow_generate() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
