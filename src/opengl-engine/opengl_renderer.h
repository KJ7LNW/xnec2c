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

#ifndef __OPENGL_RENDERER_H
#define __OPENGL_RENDERER_H 1

#include "common.h"

#ifdef HAVE_OPENGL
#include <epoxy/gl.h>
#include "opengl_math.h"

/* Maximum independent draw batches per view content
 * (structure: segments, patches, network/transmission-line outlines,
 * two-port network polygon fills, node/antinode glyph overlay) */
#define GL_VIEW_MAX_BATCHES 5

/* Compiled program a batch selects.  Each key names one shading technique
 * over the shared lit-color source; vertex packing is an independent axis
 * the presenting domain owns through gl_vertex_layout_t. */
typedef enum
{
  GL_PROGRAM_LIT = 0,
  GL_PROGRAM_FLOW_DIRECTIONAL,
  GL_PROGRAM_FLOW_BIDIRECTIONAL,
  GL_PROGRAM_FLOW_LIC,
  GL_PROGRAM_COUNT

} gl_view_program_key_t;

/* Vertex attribute descriptor */
typedef struct
{
  const char *name;
  int components;
  int offset;

} gl_vertex_attrib_t;

/* Packing of one vertex shape, owned by the domain that fills those
 * vertices.  The attribute list terminates on a NULL name. */
typedef struct
{
  const gl_vertex_attrib_t *attribs;
  int stride;

} gl_vertex_layout_t;

/** gl_draw_batch_t - Self-contained vertex batch for a single glDrawArrays call
 * @vertices: owned vertex allocation (caller manages lifetime)
 * @vertex_count: number of vertices to draw
 * @draw_mode: GL primitive mode (GL_TRIANGLES, GL_LINES, etc.)
 * @program_key: compiled program presenting this batch
 * @generation: version its producer stamped, gating this batch's upload
 */
typedef struct
{
  void *vertices;
  int vertex_count;
  unsigned int draw_mode;
  /* When TRUE, glPolygonOffset pushes this batch behind non-offset
   * batches (e.g. surface triangles behind wireframe lines). */
  gboolean polygon_offset;

  /* Per-batch RGB brightness multiplier (0.0=black, 1.0=full) */
  float color_dim;

  /* Per-batch transparency (0.0=invisible, 1.0=opaque) */
  float alpha;

  /* GL_LINES draw width in pixels; <=0 falls back to 1.0 */
  float line_width;

  gl_view_program_key_t program_key;

  unsigned int generation;

  /* Texture translation of the convolution mark; unread by other marks */
  float scroll_phase;

} gl_draw_batch_t;

/**
 * gl_batch_upload_edge_t - Record one vertex array's uploaded batch identity
 * @vertices: source allocation copied into the buffer
 * @layout: vertex packing the recorded attribute pointers describe
 * @generation: producer version of the source bytes
 * @program_key: program whose attribute locations the array records
 * @valid: whether this snapshot describes an upload
 *
 * Record the source allocation with its producer-local version because
 * independent producers share vertex-array slots.
 */
typedef struct
{
  const void *vertices;
  const gl_vertex_layout_t *layout;
  unsigned int generation;
  int vertex_count;
  gl_view_program_key_t program_key;
  gboolean valid;

} gl_batch_upload_edge_t;

/* Shader container */
typedef struct
{
  GLuint program;
  GLuint vertex;
  GLuint fragment;

} gl_shader_t;

/* Shader sources composing one program.  A fragment prologue selects a
 * compile-time variant of one shared source, so the runtime stage carries
 * no mode branch. */
typedef struct
{
  const char *vertex_path;
  const char *fragment_path;
  const char *fragment_prologue; /* NULL compiles the source unmodified */

} gl_shader_spec_t;

/* Delete a GL resource (array-style API: glDelete*(GLsizei, const GLuint*))
 * if non-zero, then zero the handle */
#define GL_DELETE(fn, id) \
  do { if( (id) ) { fn(1, &(id)); (id) = 0; } } while(0)

/* Delete a GL resource (single-handle API: glDelete*(GLuint))
 * if non-zero, then zero the handle */
#define GL_DELETE_OBJ(fn, id) \
  do { if( (id) ) { fn(id); (id) = 0; } } while(0)

/* Shader functions */
gboolean gl_shader_load(gl_shader_t *shader, const gl_shader_spec_t *spec);
void gl_shader_destroy(gl_shader_t *shader);


#endif /* HAVE_OPENGL */
#endif /* __OPENGL_RENDERER_H */
