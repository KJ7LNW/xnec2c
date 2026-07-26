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

#include "opengl_view_scene.h"
#include "opengl_view_peel.h"
#include "opengl_gradient_overlay.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/* Scene rendering context — owns shader and GL resources for primary geometry */
typedef struct
{
  gl_view_state_t *view;
  gl_shader_t shader;
  GLuint vao[GL_VIEW_MAX_BATCHES];
  GLuint vbo[GL_VIEW_MAX_BATCHES];
  GLint mvp_location;
  GLint u_mv_location;
  GLint u_alpha_location;
  GLint u_color_dim_location;
  GLint flow_mode_location;
  GLint u_phase_location;
  GLint u_cos_phase_location;
  GLint u_sin_phase_location;
  GLint noise_tex_location;
  gl_peel_uniform_locs_t peel_locs;
  GLint *attrib_locations;

} gl_scene_ctx_t;

/** gl_scene_get_alpha() - Classification alpha for scene renderable
 *
 * Returns minimum batch alpha so the scene enters the depth-peeled
 * transparent pass when any batch has transparency.
 */
static float
gl_scene_get_alpha(void *ctx)
{
  gl_scene_ctx_t *sc = ctx;

  return gl_batch_min_alpha(sc->view->content.batches,
      sc->view->content.batch_count);
}

/** gl_scene_get_content() - Primary model-space content for fit reduction */
static const gl_view_content_t *
gl_scene_get_content(void *ctx)
{
  gl_scene_ctx_t *sc = ctx;

  return &sc->view->content;
}

/* Forward declarations for callbacks */
static void gl_scene_prepare(void *ctx, float r_max);
static void gl_scene_render(void *ctx, const gl_render_params_t *params);
static gboolean gl_scene_is_active(void *ctx);
static float gl_scene_far_extent(void *ctx, float r_max);
static void gl_scene_free(void *ctx);

/*-----------------------------------------------------------------------*/

/** gl_scene_prepare() - Upload scene vertex data on generation change
 * @ctx: scene context
 * @r_max: unused
 */
  static void
gl_scene_prepare(void *ctx, float r_max)
{
  gl_scene_ctx_t *sc = ctx;
  gl_view_state_t *view = sc->view;
  gl_view_content_t *c = &view->content;

  (void)r_max;

  if( c->generation == view->last_generation )
    return;

  /* Upload each batch to its own VBO and configure its VAO */
  {
    int i;

    for( i = 0; i < c->batch_count; i++ )
    {
      if( c->batches[i].vertex_count > 0 )
      {
        glBindBuffer(GL_ARRAY_BUFFER, sc->vbo[i]);
        glBufferData(GL_ARRAY_BUFFER,
            c->batches[i].vertex_count * c->vertex_stride,
            c->batches[i].vertices,
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        gl_view_setup_attribs(sc->vao[i], sc->vbo[i],
            view->config->attribs, sc->attrib_locations,
            view->config->attrib_count, c->vertex_stride);
      }
    }
  }

  view->last_generation = c->generation;

} /* gl_scene_prepare() */

/*-----------------------------------------------------------------------*/

/** gl_scene_render() - Render scene geometry using generic draw pass
 * @ctx: scene context
 * @params: per-frame render parameters
 */
  static void
gl_scene_render(void *ctx, const gl_render_params_t *params)
{
  gl_scene_ctx_t *sc = ctx;
  gl_view_state_t *view = sc->view;

  /* Set uniforms before draw pass */
  glUseProgram(sc->shader.program);
  glUniformMatrix4fv(sc->u_mv_location, 1, GL_FALSE,
      (const float *)params->mv);

  /* Flow direction mode and phase animation offset.
   * Locations are -1 for shaders without these uniforms (no-op). */
  glUniform1i(sc->flow_mode_location, rc_config.current_flow_visualization_mode);
  glUniform1f(sc->u_phase_location, params->flow_phase);
  glUniform1f(sc->u_cos_phase_location, cosf(params->flow_phase));
  glUniform1f(sc->u_sin_phase_location, sinf(params->flow_phase));

  /* Bind LIC noise texture to unit 1 */
  if( view->noise_tex != 0 )
  {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, view->noise_tex);
    glUniform1i(sc->noise_tex_location, 1);
    glActiveTexture(GL_TEXTURE0);
  }

  gl_view_set_peel_uniforms(&sc->peel_locs, params);

  glUniformMatrix4fv(sc->mvp_location, 1, GL_FALSE,
      (const float *)params->mvp);

  /* Per-batch alpha: use batch.alpha when transparency is active,
   * otherwise 1.0 (opaque).  On-click mode suppresses transparency
   * when not dragging for renderables with transparent_on_drag. */
  {
    gboolean use_batch_alpha = view->transparency_active;
    int i;

    for( i = 0; i < view->content.batch_count; i++ )
    {
      if( view->content.batches[i].vertex_count > 0 )
      {
        float batch_alpha = use_batch_alpha
            ? view->content.batches[i].alpha : 1.0f;

        glBindVertexArray(sc->vao[i]);

        /* Per-batch polygon offset: pushes filled surfaces behind
         * lines/wires at hardware level (slope-scaled + depth-step).
         * Factor=2.0 exceeds peel epsilon dz coefficient (1.0),
         * providing margin of dz+r at all zoom levels. */
        if( view->content.batches[i].polygon_offset )
        {
          glEnable(GL_POLYGON_OFFSET_FILL);
          glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
        }
        else
        {
          glDisable(GL_POLYGON_OFFSET_FILL);
        }

        glUniform1f(sc->u_alpha_location, batch_alpha);
        glUniform1f(sc->u_color_dim_location,
            view->content.batches[i].color_dim);

        /* Apply per-batch line width unconditionally so it never inherits
         * leftover global GL state from a prior batch or renderable.
         * Triangle batches pin to their own width, keeping line width a
         * per-batch single source of truth. */
        glLineWidth(view->content.batches[i].line_width > 0.0f
            ? view->content.batches[i].line_width : 1.0f);

        glDrawArrays(view->content.batches[i].draw_mode, 0,
            view->content.batches[i].vertex_count);
      }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindVertexArray(0);
  }

} /* gl_scene_render() */

/*-----------------------------------------------------------------------*/

/** gl_scene_is_active() - Returns TRUE when scene has vertex data to render
 * @ctx: scene context
 */
  static gboolean
gl_scene_is_active(void *ctx)
{
  gl_scene_ctx_t *sc = ctx;

  return( sc->view->content.batch_count > 0 );

} /* gl_scene_is_active() */

/*-----------------------------------------------------------------------*/

/** gl_scene_far_extent() - Returns the scene geometry extent for clip plane calculation
 * @ctx: scene context
 * @r_max: default extent
 *
 * Uses clip_extent when set (accounts for translation offsets), falls back to r_max.
 */
  static float
gl_scene_far_extent(void *ctx, float r_max)
{
  gl_scene_ctx_t *sc = ctx;
  float result, clip_ext;

  result = r_max;
  clip_ext = sc->view->content.clip_extent;

  if( clip_ext > r_max )
  {
    result = clip_ext;
  }

  return( result );

} /* gl_scene_far_extent() */

/*-----------------------------------------------------------------------*/

/** gl_scene_free() - Free scene rendering context and GL resources
 * @ctx: scene context
 */
  static void
gl_scene_free(void *ctx)
{
  gl_scene_ctx_t *sc = ctx;

  if( !sc )
    return;

  if( sc->view->scene && sc->view->scene->cleanup )
    sc->view->scene->cleanup();

  glDeleteBuffers(GL_VIEW_MAX_BATCHES, sc->vbo);
  glDeleteVertexArrays(GL_VIEW_MAX_BATCHES, sc->vao);

  gl_shader_destroy(&sc->shader);

  g_free(sc->attrib_locations);
  g_free(sc);

} /* gl_scene_free() */

/*-----------------------------------------------------------------------*/

/** gl_view_scene_renderable_new() - Create scene renderable for primary geometry rendering
 * @state: view state
 */
  gl_renderable_t
gl_view_scene_renderable_new(gl_view_state_t *state)
{
  gl_scene_ctx_t *sc;
  gl_renderable_t r;
  gboolean ok;
  int i;

  sc = g_new0(gl_scene_ctx_t, 1);
  sc->view = state;

  ok = gl_shader_load(&sc->shader,
      state->config->vertex_shader_path,
      state->config->fragment_shader_path);

  if( !ok )
  {
    pr_err("Failed to load shaders\n");
    g_free(sc);

    return( (gl_renderable_t){0} );
  }

  sc->mvp_location = glGetUniformLocation(sc->shader.program, "mvp");
  sc->u_mv_location = glGetUniformLocation(sc->shader.program, "u_mv");
  sc->u_alpha_location = glGetUniformLocation(sc->shader.program, "u_alpha");
  sc->u_color_dim_location = glGetUniformLocation(sc->shader.program, "u_color_dim");
  sc->flow_mode_location = glGetUniformLocation(sc->shader.program, "flow_mode");
  sc->u_phase_location = glGetUniformLocation(sc->shader.program, "u_phase");
  sc->u_cos_phase_location = glGetUniformLocation(sc->shader.program, "u_cos_phase");
  sc->u_sin_phase_location = glGetUniformLocation(sc->shader.program, "u_sin_phase");
  sc->noise_tex_location = glGetUniformLocation(sc->shader.program, "noise_tex");
  gl_view_peel_locs_init(&sc->peel_locs, sc->shader.program);

  glGenVertexArrays(GL_VIEW_MAX_BATCHES, sc->vao);
  glGenBuffers(GL_VIEW_MAX_BATCHES, sc->vbo);

  sc->attrib_locations = g_new(GLint, state->config->attrib_count);

  for( i = 0; i < state->config->attrib_count; i++ )
  {
    sc->attrib_locations[i] = glGetAttribLocation(
        sc->shader.program,
        state->config->attribs[i].name);
  }

  /* Override default generic value for flow_data attribute.
   * OpenGL defaults unbound vec4 attributes to (0,0,0,1); the w=1
   * causes mag_sq=1.0 in the fragment shader, falsely activating
   * the flow/chevron/LIC block for non-patch vertices (rdpattern
   * shell, wire segments using 3-attrib config). */
  {
    GLint flow_loc = glGetAttribLocation(sc->shader.program, "flow_data");

    if( flow_loc >= 0 )
      glVertexAttrib4f(flow_loc, 0.0f, 0.0f, 0.0f, 0.0f);
  }

  r = (gl_renderable_t){
    .render               = gl_scene_render,
    .prepare              = gl_scene_prepare,
    .destroy              = gl_scene_free,
    .is_active            = gl_scene_is_active,
    .far_extent           = gl_scene_far_extent,
    .get_content          = gl_scene_get_content,
    .ctx                  = sc,
    .get_alpha            = gl_scene_get_alpha,
    .origin               = {0.0f, 0.0f, 0.0f},
    .transparent_sort_order = 1,
    .transparent_on_drag  = TRUE,
    .force_peel           = TRUE
  };

  return( r );

} /* gl_view_scene_renderable_new() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
