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

#include "opengl_view_overlay.h"
#include "opengl_view_peel.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/* Overlay rendering context — owns shader, GL resources, and cached MVP */
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
  unsigned int last_generation;
  gboolean initialized;
  mat4 cached_mvp;
  mat4 cached_mv;
  gl_view_content_t ovl_content;

} gl_overlay_ctx_t;

/* Forward declarations for callbacks */
static void gl_overlay_prepare(void *ctx, const gl_render_params_t *_params);
static void gl_overlay_render(void *ctx, const gl_render_params_t *params);
static gboolean gl_overlay_is_active(void *ctx);
static void gl_overlay_generate(void *ctx);
static float gl_overlay_far_extent(void *ctx, float r_max);
static void gl_overlay_free(void *ctx);

/*-----------------------------------------------------------------------*/

/** gl_overlay_get_alpha() - Classification alpha for overlay renderable
 *
 * Returns the minimum batch alpha so the overlay enters the
 * depth-peeled transparent pass when any batch has transparency.
 */
static float
gl_overlay_get_alpha(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;

  return gl_batch_min_alpha(ovl->ovl_content.batches,
      ovl->ovl_content.batch_count);
}

/*-----------------------------------------------------------------------*/

/** gl_overlay_get_content() - Overlay model-space content for fit reduction */
static const gl_view_content_t *
gl_overlay_get_content(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;

  return &ovl->ovl_content;
}

/*-----------------------------------------------------------------------*/

/** gl_overlay_prepare() - Upload overlay VBO and cache own MVP
 * @ctx: overlay context
 * @_params: unused uniform render parameters
 *
 * ovl_content is pre-populated by gl_overlay_generate_and_extent during
 * the active survey — no overlay_generate call here.
 */
  static void
gl_overlay_prepare(void *ctx, const gl_render_params_t *_params)
{
  gl_overlay_ctx_t *ovl = ctx;
  gl_view_state_t *view = ovl->view;

  (void)_params;

  if( !ovl->initialized )
    return;

  if( ovl->ovl_content.batch_count <= 0 )
    return;

  /* Upload overlay vertices on generation change */
  if( ovl->ovl_content.generation != ovl->last_generation )
  {
    const gl_overlay_config_t *ocfg = view->scene->overlay_config;
    int i;

    for( i = 0; i < ovl->ovl_content.batch_count; i++ )
    {
      if( ovl->ovl_content.batches[i].vertex_count > 0 )
      {
        glBindBuffer(GL_ARRAY_BUFFER, ovl->vbo[i]);
        glBufferData(GL_ARRAY_BUFFER,
            ovl->ovl_content.batches[i].vertex_count * ovl->ovl_content.vertex_stride,
            ovl->ovl_content.batches[i].vertices,
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        gl_view_setup_attribs(ovl->vao[i], ovl->vbo[i],
            ocfg->attribs, ovl->attrib_locations,
            ocfg->attrib_count, ovl->ovl_content.vertex_stride);
      }
    }

    ovl->last_generation = ovl->ovl_content.generation;
  }

  /* Compute and cache own MVP with user-adjusted model scale.
   * Projection parameters (near/far) shared with main render pass
   * so all renderables produce comparable depth values; only the
   * uniform model scale differs from the main-content pass. */
  gl_view_build_mvp(view, ovl->ovl_content.model_scale,
      ovl->cached_mvp, ovl->cached_mv);

} /* gl_overlay_prepare() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_render() - Render overlay using cached MVP (ignores params->mvp)
 * @ctx: overlay context
 * @params: per-frame render parameters (mvp/mv ignored — overlay uses own projection)
 */
  static void
gl_overlay_render(void *ctx, const gl_render_params_t *params)
{
  gl_overlay_ctx_t *ovl = ctx;

  if( ovl->ovl_content.batch_count <= 0 )
    return;

  /* Set uniforms before draw pass.
   * Overlay uses its own cached MV (different model_scale). */
  glUseProgram(ovl->shader.program);
  glUniformMatrix4fv(ovl->u_mv_location, 1, GL_FALSE,
      (float *)ovl->cached_mv);
  glUniform1i(ovl->flow_mode_location, rc_config.current_flow_visualization_mode);
  glUniform1f(ovl->u_phase_location, params->flow_phase);
  glUniform1f(ovl->u_cos_phase_location, cosf(params->flow_phase));
  glUniform1f(ovl->u_sin_phase_location, sinf(params->flow_phase));

  /* Bind LIC noise texture to unit 1 (shared with scene via view state) */
  if( ovl->view->noise_tex != 0 )
  {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ovl->view->noise_tex);
    glUniform1i(ovl->noise_tex_location, 1);
    glActiveTexture(GL_TEXTURE0);
  }

  gl_view_set_peel_uniforms(&ovl->peel_locs, params);

  glUniformMatrix4fv(ovl->mvp_location, 1, GL_FALSE,
      (const float *)ovl->cached_mvp);

  /* Per-batch alpha and brightness for overlay (structure on rdpattern) */
  {
    gboolean use_batch_alpha = ovl->view->transparency_active;
    int i;

    for( i = 0; i < ovl->ovl_content.batch_count; i++ )
    {
      if( ovl->ovl_content.batches[i].vertex_count > 0 )
      {
        float batch_alpha = use_batch_alpha
            ? ovl->ovl_content.batches[i].alpha : 1.0f;

        glBindVertexArray(ovl->vao[i]);

        /* Per-batch polygon offset for overlay structure patches */
        if( ovl->ovl_content.batches[i].polygon_offset )
        {
          glEnable(GL_POLYGON_OFFSET_FILL);
          glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
        }
        else
        {
          glDisable(GL_POLYGON_OFFSET_FILL);
        }

        glUniform1f(ovl->u_alpha_location, batch_alpha);
        glUniform1f(ovl->u_color_dim_location,
            ovl->ovl_content.batches[i].color_dim);

        /* Apply per-batch line width unconditionally so it never inherits
         * leftover global GL state from a prior batch or renderable.
         * Triangle batches pin to their own width, keeping line width a
         * per-batch single source of truth. */
        glLineWidth(ovl->ovl_content.batches[i].line_width > 0.0f
            ? ovl->ovl_content.batches[i].line_width : 1.0f);

        glDrawArrays(ovl->ovl_content.batches[i].draw_mode, 0,
            ovl->ovl_content.batches[i].vertex_count);
      }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindVertexArray(0);
  }

} /* gl_overlay_render() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_is_active() - Returns TRUE when overlay is initialized and provider exists
 * @ctx: overlay context
 */
  static gboolean
gl_overlay_is_active(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;

  return( ovl->initialized &&
          ovl->view->scene->overlay_generate != NULL );

} /* gl_overlay_is_active() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_generate() - Regenerate overlay content via overlay_generate, storing the result in ovl_content
 * @ctx: overlay context
 *
 * Resets vertex_count first so stale data does not persist when overlay_generate
 * declines (e.g. the structure overlay toggle unchecked).
 * Called during the active survey before far_extent is queried.
 */
  static void
gl_overlay_generate(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;
  gl_view_state_t *view = ovl->view;

  /* Reset before generation attempt */
  ovl->ovl_content.batch_count = 0;

  if( !ovl->initialized )
    return;

  if( !view->scene->overlay_generate )
    return;

  view->scene->overlay_generate(&view->content, &ovl->ovl_content);

} /* gl_overlay_generate() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_far_extent() - Return the overlay's scaled geometry extent for clip plane computation
 * @ctx: overlay context
 * @r_max: current maximum scene extent
 *
 * Reads ovl_content populated by gl_overlay_generate during the same survey pass.
 */
  static float
gl_overlay_far_extent(void *ctx, float r_max)
{
  gl_overlay_ctx_t *ovl = ctx;
  float ovl_model_scale, scaled_extent;

  if( ovl->ovl_content.batch_count <= 0 )
    return( r_max );

  ovl_model_scale = ovl->ovl_content.model_scale;

  scaled_extent = ovl->ovl_content.r_max * ovl_model_scale;

  if( scaled_extent > r_max )
    return( scaled_extent );

  return( r_max );

} /* gl_overlay_far_extent() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_free() - Free overlay rendering context and GL resources
 * @ctx: overlay context
 */
  static void
gl_overlay_free(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;

  if( !ovl )
    return;

  if( ovl->view->scene && ovl->view->scene->overlay_cleanup )
    ovl->view->scene->overlay_cleanup();

  glDeleteBuffers(GL_VIEW_MAX_BATCHES, ovl->vbo);
  glDeleteVertexArrays(GL_VIEW_MAX_BATCHES, ovl->vao);

  gl_shader_destroy(&ovl->shader);

  g_free(ovl->attrib_locations);
  g_free(ovl);

} /* gl_overlay_free() */

/*-----------------------------------------------------------------------*/

/** gl_view_overlay_renderable_new() - Create overlay renderable for second-pass rendering
 * @state: view state
 */
  gl_renderable_t
gl_view_overlay_renderable_new(gl_view_state_t *state)
{
  const gl_overlay_config_t *ocfg;
  gl_overlay_ctx_t *ovl;
  gl_renderable_t r;
  gboolean ok;
  int i;

  if( !state->scene->overlay_config )
  {
    return( (gl_renderable_t){0} );
  }

  ocfg = state->scene->overlay_config;

  ovl = g_new0(gl_overlay_ctx_t, 1);
  ovl->view = state;

  ok = gl_shader_load(&ovl->shader,
      ocfg->vertex_shader_path,
      ocfg->fragment_shader_path);

  if( !ok )
  {
    pr_err("Failed to load overlay shaders\n");
    g_free(ovl);

    return( (gl_renderable_t){0} );
  }

  ovl->mvp_location =
    glGetUniformLocation(ovl->shader.program, "mvp");
  ovl->u_mv_location =
    glGetUniformLocation(ovl->shader.program, "u_mv");
  ovl->u_alpha_location =
    glGetUniformLocation(ovl->shader.program, "u_alpha");
  ovl->u_color_dim_location =
    glGetUniformLocation(ovl->shader.program, "u_color_dim");
  ovl->flow_mode_location =
    glGetUniformLocation(ovl->shader.program, "flow_mode");
  ovl->u_phase_location =
    glGetUniformLocation(ovl->shader.program, "u_phase");
  ovl->u_cos_phase_location =
    glGetUniformLocation(ovl->shader.program, "u_cos_phase");
  ovl->u_sin_phase_location =
    glGetUniformLocation(ovl->shader.program, "u_sin_phase");
  ovl->noise_tex_location =
    glGetUniformLocation(ovl->shader.program, "noise_tex");
  gl_view_peel_locs_init(&ovl->peel_locs, ovl->shader.program);

  glGenVertexArrays(GL_VIEW_MAX_BATCHES, ovl->vao);
  glGenBuffers(GL_VIEW_MAX_BATCHES, ovl->vbo);

  ovl->attrib_locations = g_new(GLint, ocfg->attrib_count);
  for( i = 0; i < ocfg->attrib_count; i++ )
  {
    ovl->attrib_locations[i] = glGetAttribLocation(
        ovl->shader.program,
        ocfg->attribs[i].name);
  }

  /* Override default generic value for flow_data attribute.
   * See gl_view_scene_renderable_new() for rationale. */
  {
    GLint flow_loc = glGetAttribLocation(ovl->shader.program, "flow_data");

    if( flow_loc >= 0 )
      glVertexAttrib4f(flow_loc, 0.0f, 0.0f, 0.0f, 0.0f);
  }

  ovl->last_generation = (unsigned int)-1;
  ovl->initialized = TRUE;

  r = (gl_renderable_t){
    .render               = gl_overlay_render,
    .prepare              = gl_overlay_prepare,
    .destroy              = gl_overlay_free,
    .is_active            = gl_overlay_is_active,
    .generate             = gl_overlay_generate,
    .far_extent           = gl_overlay_far_extent,
    .get_content          = gl_overlay_get_content,
    .ctx                  = ovl,
    .get_alpha            = gl_overlay_get_alpha,
    .origin               = {0.0f, 0.0f, 0.0f},
    .transparent_sort_order = 1,
    .transparent_on_drag  = TRUE,
    .force_peel           = TRUE
  };

  return( r );

} /* gl_view_overlay_renderable_new() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
