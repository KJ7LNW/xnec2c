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

#ifndef __OPENGL_VIEW_PROGRAM_H
#define __OPENGL_VIEW_PROGRAM_H   1

#include "opengl_view.h"

#ifdef HAVE_OPENGL

/**
 * gl_batch_slot_t - Vertex-array resources one batch is presented through
 * @vao: vertex array recording this batch's attribute state
 * @vbo: buffer holding this batch's vertices
 * @edge: snapshot of what this slot last received
 */
typedef struct
{
  GLuint vao;
  GLuint vbo;
  gl_batch_upload_edge_t edge;

} gl_batch_slot_t;

/**
 * gl_batch_pass_t - One pass over the batches a renderable presents
 * @programs: catalog the batches select from
 * @noise_tex: line-integral-convolution texture, or zero when absent
 * @transparency_active: whether authored batch alpha reaches this pass
 * @content: batches this pass draws
 * @slots: vertex-array resources, one per batch
 * @mvp: model-view-projection matrix this pass presents with
 * @mv: model-view matrix this pass presents with
 */
typedef struct
{
  const gl_program_t *programs;
  GLuint noise_tex;
  gboolean transparency_active;
  const gl_view_content_t *content;
  const gl_batch_slot_t *slots;
  const float *mvp;
  const float *mv;

} gl_batch_pass_t;

/**
 * gl_batch_slots_init() - Create the vertex-array resources of every slot
 * @slots: slots receiving one vertex array and one buffer each
 * @count: number of slots the presenting renderable holds
 */
void gl_batch_slots_init(gl_batch_slot_t *slots, int count);

/**
 * gl_batch_slots_destroy() - Release the vertex-array resources of every slot
 * @slots: slots whose vertex arrays and buffers are released
 * @count: number of slots the presenting renderable holds
 */
void gl_batch_slots_destroy(gl_batch_slot_t *slots, int count);

/**
 * gl_program_catalog_init() - Compile every program a view presents with
 * @catalog: catalog receiving one compiled program per key
 *
 * Returns TRUE when every program links; releases the partial catalog and
 * returns FALSE otherwise.
 */
gboolean gl_program_catalog_init(gl_program_t catalog[GL_PROGRAM_COUNT]);

/**
 * gl_program_catalog_destroy() - Release every program of one catalog
 * @catalog: catalog whose programs are released
 */
void gl_program_catalog_destroy(gl_program_t catalog[GL_PROGRAM_COUNT]);

/**
 * gl_view_upload_batch() - Refresh one slot against the batch it draws
 * @slot: vertex-array resources presenting this batch
 * @program: program whose attribute locations the layout binds against
 * @layout: vertex packing the presenting domain owns
 * @batch: batch the frame presents now
 *
 * Uploads vertices when the producer advanced and rebinds attribute
 * locations when the presenting program changed, then records what the
 * slot now holds.
 */
void gl_view_upload_batch(gl_batch_slot_t *slot,
    const gl_program_t *program, const gl_vertex_layout_t *layout,
    const gl_draw_batch_t *batch);

/**
 * gl_view_draw_batches() - Draw every batch of one pass
 * @pass: batches, slots, and matrices this pass presents
 * @params: per-frame render parameters
 *
 * Binds a program once per run of batches sharing it, applies the offset,
 * uniform, and width state each batch authored, and draws it.
 */
void gl_view_draw_batches(const gl_batch_pass_t *pass,
    const gl_render_params_t *params);

#endif /* HAVE_OPENGL */
#endif /* __OPENGL_VIEW_PROGRAM_H */
