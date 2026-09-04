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
 * opengl_view_program: The compiled programs a view draws its batches with.
 *
 * One catalog serves the scene pass and the overlay pass, so neither holds
 * program state of its own.  Each key names a shading technique over one
 * shared source; vertex packing is an independent axis the presenting
 * domain owns through gl_vertex_layout_t.
 */
#include "opengl_view_program.h"
#include "opengl_view_peel.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/* Every catalog program compiles one shared lit-color source pair */
#define LIT_VERTEX_PATH   "/gl/lit-color-vertex.glsl"
#define LIT_FRAGMENT_PATH "/gl/lit-color-fragment.glsl"

/* Fragment definition selecting the mark each catalog program compiles */
static const gl_shader_spec_t gl_program_specs[] = {
  [GL_PROGRAM_LIT] = {
      .vertex_path = LIT_VERTEX_PATH,
      .fragment_path = LIT_FRAGMENT_PATH },
  [GL_PROGRAM_FLOW_DIRECTIONAL] = {
      .vertex_path = LIT_VERTEX_PATH,
      .fragment_path = LIT_FRAGMENT_PATH,
      .fragment_prologue = "#define FLOW_MARK_DIRECTIONAL 1" },
  [GL_PROGRAM_FLOW_BIDIRECTIONAL] = {
      .vertex_path = LIT_VERTEX_PATH,
      .fragment_path = LIT_FRAGMENT_PATH,
      .fragment_prologue = "#define FLOW_MARK_BIDIRECTIONAL 1" },
  [GL_PROGRAM_FLOW_LIC] = {
      .vertex_path = LIT_VERTEX_PATH,
      .fragment_path = LIT_FRAGMENT_PATH,
      .fragment_prologue = "#define FLOW_MARK_LIC 1" },
};

_Static_assert(G_N_ELEMENTS(gl_program_specs) == GL_PROGRAM_COUNT,
    "every program key names one shader specification");

/**
 * gl_program_set_magnitude_default() - Zero the generic flow magnitude
 * @program: linked program whose unbound magnitude attribute receives zero
 *
 * The generic attribute value is context state every program shares, so each
 * program establishes the zero a vertex carrying no flow reads.
 */
  static void
gl_program_set_magnitude_default(const gl_program_t *program)
{
  GLint location;

  location = glGetAttribLocation(program->shader.program, "magnitude");

  if( location < 0 )
    return;

  glVertexAttrib1f(location, 0.0f);

} /* gl_program_set_magnitude_default() */

/*-----------------------------------------------------------------------*/

/**
 * gl_program_init() - Compile one program and resolve its locations
 * @program: program record to populate
 * @spec: shader sources and the fragment variant this program compiles
 *
 * Returns TRUE when the program links and its locations resolve.
 */
  static gboolean
gl_program_init(gl_program_t *program, const gl_shader_spec_t *spec)
{
  if( !gl_shader_load(&program->shader, spec) )
    return( FALSE );

  program->mvp = glGetUniformLocation(program->shader.program, "mvp");
  program->u_mv = glGetUniformLocation(program->shader.program, "u_mv");
  program->u_alpha = glGetUniformLocation(program->shader.program, "u_alpha");
  program->u_color_dim =
    glGetUniformLocation(program->shader.program, "u_color_dim");
  program->u_scroll_phase =
    glGetUniformLocation(program->shader.program, "u_scroll_phase");
  program->noise_tex =
    glGetUniformLocation(program->shader.program, "noise_tex");
  gl_view_peel_locs_init(&program->peel, program->shader.program);

  /* A layout without flow leaves magnitude unbound, and every mark skips
   * a vertex whose magnitude is zero. */
  gl_program_set_magnitude_default(program);

  return( TRUE );

} /* gl_program_init() */

/*-----------------------------------------------------------------------*/

/**
 * gl_program_catalog_destroy() - Release every program of one catalog
 * @catalog: catalog whose programs are released
 */
void
gl_program_catalog_destroy(gl_program_t catalog[GL_PROGRAM_COUNT])
{
  gl_view_program_key_t key;

  /* Clearing the record leaves every handle zero, so a catalog released
   * after a partial load releases nothing twice. */
  for( key = GL_PROGRAM_LIT; key < GL_PROGRAM_COUNT; key++ )
  {
    gl_shader_destroy(&catalog[key].shader);
    catalog[key] = (gl_program_t){0};
  }

} /* gl_program_catalog_destroy() */

/*-----------------------------------------------------------------------*/

/**
 * gl_program_catalog_init() - Compile every program a view presents with
 * @catalog: catalog receiving one compiled program per key
 *
 * Returns TRUE when every program links; releases the partial catalog and
 * returns FALSE otherwise.
 */
gboolean
gl_program_catalog_init(gl_program_t catalog[GL_PROGRAM_COUNT])
{
  gl_view_program_key_t key;
  gboolean loaded;

  loaded = TRUE;

  for( key = GL_PROGRAM_LIT; key < GL_PROGRAM_COUNT && loaded; key++ )
    loaded = gl_program_init(&catalog[key], &gl_program_specs[key]);

  if( !loaded )
    gl_program_catalog_destroy(catalog);

  return( loaded );

} /* gl_program_catalog_init() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_setup_attribs() - Point one vertex array at a layout within a program
 * @vao: vertex array object receiving the attribute state
 * @vbo: buffer holding the vertices the layout describes
 * @program: program whose attribute locations the layout binds against
 * @layout: vertex packing the presenting domain owns
 *
 * The vertex array retains this state for subsequent draws, so only a
 * change of presenting program requires resolving these locations again.
 */
static void
gl_view_setup_attribs(GLuint vao, GLuint vbo, const gl_program_t *program,
    const gl_vertex_layout_t *layout)
{
  int idx;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  for( idx = 0; layout->attribs[idx].name != NULL; idx++ )
  {
    GLint location = glGetAttribLocation(program->shader.program,
        layout->attribs[idx].name);

    /* A layout may describe vertices richer than the bound program reads. */
    if( location < 0 )
      continue;

    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
        location,
        layout->attribs[idx].components,
        GL_FLOAT,
        GL_FALSE,
        layout->stride,
        (void *)(long)layout->attribs[idx].offset);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

} /* gl_view_setup_attribs() */

/*-----------------------------------------------------------------------*/

/**
 * gl_batch_slots_init() - Create the vertex-array resources of every slot
 * @slots: slots receiving one vertex array and one buffer each
 * @count: number of slots the presenting renderable holds
 */
void
gl_batch_slots_init(gl_batch_slot_t *slots, int count)
{
  int i;

  for( i = 0; i < count; i++ )
  {
    glGenVertexArrays(1, &slots[i].vao);
    glGenBuffers(1, &slots[i].vbo);
  }

} /* gl_batch_slots_init() */

/*-----------------------------------------------------------------------*/

/**
 * gl_batch_slots_destroy() - Release the vertex-array resources of every slot
 * @slots: slots whose vertex arrays and buffers are released
 * @count: number of slots the presenting renderable holds
 */
void
gl_batch_slots_destroy(gl_batch_slot_t *slots, int count)
{
  int i;

  for( i = 0; i < count; i++ )
  {
    glDeleteBuffers(1, &slots[i].vbo);
    glDeleteVertexArrays(1, &slots[i].vao);
  }

} /* gl_batch_slots_destroy() */

/*-----------------------------------------------------------------------*/

/**
 * gl_batch_data_current() - Report whether the uploaded vertex store stands
 * @edge: snapshot recorded by the last upload of this vertex array
 * @layout: vertex packing determining the uploaded byte count
 * @batch: batch the frame presents now
 */
  static gboolean
gl_batch_data_current(const gl_batch_upload_edge_t *edge,
    const gl_vertex_layout_t *layout, const gl_draw_batch_t *batch)
{
  return( edge->valid
      && edge->vertices == batch->vertices
      && edge->layout == layout
      && edge->generation == batch->generation
      && edge->vertex_count == batch->vertex_count );

} /* gl_batch_data_current() */

/*-----------------------------------------------------------------------*/

/**
 * gl_batch_layout_current() - Report whether the recorded attributes stand
 * @edge: snapshot recorded by the last upload of this vertex array
 * @layout: vertex packing the presenting domain owns
 * @batch: batch the frame presents now
 *
 * The recorded pointers carry both the packing they describe and the
 * locations the presenting program resolved, so either changing re-resolves.
 */
  static gboolean
gl_batch_layout_current(const gl_batch_upload_edge_t *edge,
    const gl_vertex_layout_t *layout, const gl_draw_batch_t *batch)
{
  return( edge->valid && edge->layout == layout
      && edge->program_key == batch->program_key );

} /* gl_batch_layout_current() */

/*-----------------------------------------------------------------------*/

/**
 * gl_batch_upload_capture() - Snapshot the state a batch upload would record
 * @layout: vertex packing the presenting domain owns
 * @batch: batch about to be uploaded
 *
 * Records the source allocation with its producer-local version because
 * independent producers share vertex-array slots.
 */
  static gl_batch_upload_edge_t
gl_batch_upload_capture(const gl_vertex_layout_t *layout,
    const gl_draw_batch_t *batch)
{
  return( (gl_batch_upload_edge_t){
      .vertices = batch->vertices,
      .layout = layout,
      .generation = batch->generation,
      .vertex_count = batch->vertex_count,
      .program_key = batch->program_key,
      .valid = TRUE } );

} /* gl_batch_upload_capture() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_upload_batch_data() - Refresh one slot's vertex store
 * @slot: vertex-array resources presenting this batch
 * @layout: vertex packing determining the uploaded byte count
 * @batch: batch the frame presents now
 */
  static void
gl_view_upload_batch_data(gl_batch_slot_t *slot,
    const gl_vertex_layout_t *layout, const gl_draw_batch_t *batch)
{
  if( likely(gl_batch_data_current(&slot->edge, layout, batch)) )
    return;

  glBindBuffer(GL_ARRAY_BUFFER, slot->vbo);
  glBufferData(GL_ARRAY_BUFFER,
      batch->vertex_count * layout->stride,
      batch->vertices,
      GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

} /* gl_view_upload_batch_data() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_upload_batch_layout() - Refresh one slot's vertex attributes
 * @slot: vertex-array resources presenting this batch
 * @program: program whose attribute locations the layout binds against
 * @layout: vertex packing the presenting domain owns
 * @batch: batch the frame presents now
 */
  static void
gl_view_upload_batch_layout(gl_batch_slot_t *slot,
    const gl_program_t *program, const gl_vertex_layout_t *layout,
    const gl_draw_batch_t *batch)
{
  if( likely(gl_batch_layout_current(&slot->edge, layout, batch)) )
    return;

  gl_view_setup_attribs(slot->vao, slot->vbo, program, layout);

} /* gl_view_upload_batch_layout() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_upload_batch() - Refresh one slot against the batch it draws
 * @slot: vertex-array resources presenting this batch
 * @program: program whose attribute locations the layout binds against
 * @layout: vertex packing the presenting domain owns
 * @batch: batch the frame presents now
 *
 * Replacing the store behind a bound buffer name leaves the vertex array's
 * recorded format, stride, offsets, and buffer binding untouched, so a
 * producer advance uploads alone while a layout or program change alone
 * re-resolves attribute locations.
 */
void
gl_view_upload_batch(gl_batch_slot_t *slot, const gl_program_t *program,
    const gl_vertex_layout_t *layout, const gl_draw_batch_t *batch)
{
  gl_view_upload_batch_data(slot, layout, batch);
  gl_view_upload_batch_layout(slot, program, layout, batch);
  slot->edge = gl_batch_upload_capture(layout, batch);

} /* gl_view_upload_batch() */

/*-----------------------------------------------------------------------*/

/**
 * gl_program_bind_noise() - Bind the optional line-integral-convolution texture
 * @program: program whose sampler receives texture unit one
 * @noise_tex: texture to bind, or zero when the view carries none
 */
  static void
gl_program_bind_noise(const gl_program_t *program, GLuint noise_tex)
{
  if( noise_tex == 0 || program->noise_tex < 0 )
    return;

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, noise_tex);
  glUniform1i(program->noise_tex, 1);
  glActiveTexture(GL_TEXTURE0);

} /* gl_program_bind_noise() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_set_batch_uniforms() - Apply the uniforms one batch presents with
 * @program: program bound for this batch
 * @batch: batch about to draw
 * @transparency_active: whether this pass presents the authored batch alpha
 *
 * A location absent from the bound program is -1, which every setter here
 * treats as a no-op.  The authored alpha stays on the batch because other
 * consumers read it independently of presentation.
 */
  static void
gl_view_set_batch_uniforms(const gl_program_t *program,
    const gl_draw_batch_t *batch, gboolean transparency_active)
{
  glUniform1f(program->u_alpha,
      transparency_active ? batch->alpha : 1.0f);
  glUniform1f(program->u_color_dim, batch->color_dim);
  glUniform1f(program->u_scroll_phase, batch->scroll_phase);

} /* gl_view_set_batch_uniforms() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_bind_batch_program() - Bind one batch's program on transition
 * @pass: matrices and view resources this pass presents
 * @params: per-frame render parameters
 * @key: catalog program the batch selects
 * @bound: program currently bound for this pass
 *
 * Returns the program bound after this transition.
 */
  static const gl_program_t *
gl_view_bind_batch_program(const gl_batch_pass_t *pass,
    const gl_render_params_t *params, gl_view_program_key_t key,
    const gl_program_t *bound)
{
  const gl_program_t *program = &pass->programs[key];

  if( likely(program == bound) )
    return bound;

  glUseProgram(program->shader.program);
  glUniformMatrix4fv(program->u_mv, 1, GL_FALSE, pass->mv);

  gl_program_bind_noise(program, pass->noise_tex);

  gl_view_set_peel_uniforms(&program->peel, params);

  glUniformMatrix4fv(program->mvp, 1, GL_FALSE, pass->mvp);

  return program;

} /* gl_view_bind_batch_program() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_draw_batches() - Draw every batch of one pass
 * @pass: batches, slots, and matrices this pass presents
 * @params: per-frame render parameters
 *
 * Binds a program once per run of batches sharing it, applies the offset,
 * uniform, and width state each batch authored, and draws it.
 */
void
gl_view_draw_batches(const gl_batch_pass_t *pass,
    const gl_render_params_t *params)
{
  const gl_program_t *bound = NULL;
  int i;

  if( unlikely(pass->content->batch_count <= 0) )
    return;

  for( i = 0; i < pass->content->batch_count; i++ )
  {
    const gl_draw_batch_t *batch = &pass->content->batches[i];
    const gl_program_t *program;

    if( unlikely(batch->vertex_count <= 0) )
      continue;

    program = gl_view_bind_batch_program(pass, params,
        batch->program_key, bound);
    bound = program;

    glBindVertexArray(pass->slots[i].vao);

    /* Per-batch polygon offset: pushes filled surfaces behind
     * lines/wires at hardware level (slope-scaled + depth-step).
     * Factor=2.0 exceeds peel epsilon dz coefficient (1.0),
     * providing margin of dz+r at all zoom levels. */
    if( batch->polygon_offset )
    {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
    }
    else
    {
      glDisable(GL_POLYGON_OFFSET_FILL);
    }

    gl_view_set_batch_uniforms(program, batch, pass->transparency_active);

    /* Apply per-batch line width unconditionally so it never inherits
     * leftover global GL state from a prior batch or renderable.
     * Triangle batches pin to their own width, keeping line width a
     * per-batch single source of truth. */
    glLineWidth(fl_fgt(batch->line_width, -FL_EPS)
        ? batch->line_width : 1.0f);

    glDrawArrays(batch->draw_mode, 0, batch->vertex_count);
  }

  glDisable(GL_POLYGON_OFFSET_FILL);
  glBindVertexArray(0);

} /* gl_view_draw_batches() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
