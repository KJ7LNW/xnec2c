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

#include "opengl_view_peel.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** gl_view_peel_free() - Release depth-peel FBO, texture, accumulation, and composite resources
 * @state: view state containing peel resources to free
 */
  void
gl_view_peel_free(gl_view_state_t *state)
{
  int i;

  if( !state )
    return;

  for( i = 0; i < 2; i++ )
  {
    GL_DELETE(glDeleteFramebuffers, state->peel_fbo[i]);
    GL_DELETE(glDeleteTextures, state->peel_depth_tex[i]);

    /* Multisampled peel resources (no-op when MSAA inactive) */
    GL_DELETE(glDeleteFramebuffers, state->peel_ms_fbo[i]);
    GL_DELETE(glDeleteRenderbuffers, state->peel_ms_color_rbo[i]);
    GL_DELETE(glDeleteRenderbuffers, state->peel_ms_depth_rbo[i]);
  }

  GL_DELETE(glDeleteTextures, state->peel_color_tex);
  GL_DELETE(glDeleteTextures, state->layer_depth_tex);
  GL_DELETE(glDeleteFramebuffers, state->layer_depth_fbo);
  GL_DELETE(glDeleteFramebuffers, state->accum_fbo);
  GL_DELETE(glDeleteTextures, state->accum_color_tex);

  state->peel_width = 0;
  state->peel_height = 0;
  state->peel_msaa_samples = 0;

} /* gl_view_peel_free() */

/*-----------------------------------------------------------------------*/

/** create_peel_texture_handle() - Create a 2D texture with nearest filter and clamp-to-edge wrap
 *
 * Allocates the GL name and sets sampling parameters.  Does not allocate
 * storage — call resize_peel_texture() afterward.
 *
 * Returns texture handle; leaves texture bound to GL_TEXTURE_2D.
 */
  static GLuint
create_peel_texture_handle(void)
{
  GLuint tex;

  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return tex;

} /* create_peel_texture_handle() */

/*-----------------------------------------------------------------------*/

/** resize_peel_texture() - Reallocate texture storage at given dimensions
 * @tex: texture handle (must already exist)
 * @ifmt: internal format (e.g. GL_RGBA8, GL_DEPTH_COMPONENT24)
 * @width: texture width in pixels
 * @height: texture height in pixels
 * @fmt: pixel format (e.g. GL_RGBA, GL_DEPTH_COMPONENT)
 * @type: pixel type (e.g. GL_UNSIGNED_BYTE, GL_FLOAT)
 */
  static void
resize_peel_texture(GLuint tex, GLenum ifmt, int width, int height,
    GLenum fmt, GLenum type)
{
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, type, NULL);

} /* resize_peel_texture() */

/*-----------------------------------------------------------------------*/

/** gl_view_peel_recreate() - Resize depth-peel FBO resources, creating handles only on first call
 * @state: view state to update with new peel resources
 * @width: viewport width in pixels
 * @height: viewport height in pixels
 * @msaa_samples: MSAA sample count (0 = single-sample peel FBOs)
 *
 * Separates handle lifecycle from storage allocation.  Texture handles,
 * FBOs, and FBO attachments are created once.  Storage (glTexImage2D,
 * glRenderbufferStorageMultisample) is resized in place on every call.
 *
 * Multisampled renderbuffer handles are created/destroyed only when
 * transitioning between MSAA 0 and >0, tracked via peel_msaa_samples.
 */
  void
gl_view_peel_recreate(gl_view_state_t *state, int width, int height,
    int msaa_samples)
{
  int i;
  int eff_msaa;

  if( !state )
    return;

  if( width == 0 || height == 0 )
  {
    gl_view_peel_free(state);
    return;
  }

  eff_msaa = (msaa_samples >= 2) ? msaa_samples : 0;

  /* Create texture handles and FBO attachments once */
  if( !state->peel_color_tex )
  {
    state->peel_color_tex = create_peel_texture_handle();

    for( i = 0; i < 2; i++ )
      state->peel_depth_tex[i] = create_peel_texture_handle();

    state->accum_color_tex = create_peel_texture_handle();

    /* Layer depth texture for coplanar accumulation sub-pass.
     * Receives a depth blit after depth discovery so the fragment
     * shader can sample it without feedback on the active FBO. */
    state->layer_depth_tex = create_peel_texture_handle();

    /* Single-sample FBOs with texture attachments */
    glGenFramebuffers(2, state->peel_fbo);

    for( i = 0; i < 2; i++ )
    {
      glBindFramebuffer(GL_FRAMEBUFFER, state->peel_fbo[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
          GL_TEXTURE_2D, state->peel_depth_tex[i], 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D, state->peel_color_tex, 0);
    }

    /* Layer depth FBO: depth-only target for blit from peel depth */
    glGenFramebuffers(1, &state->layer_depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, state->layer_depth_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, state->layer_depth_tex, 0);

    /* Accumulation FBO */
    glGenFramebuffers(1, &state->accum_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, state->accum_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, state->accum_color_tex, 0);
  }

  /* Resize texture storage in place */
  state->peel_width = width;
  state->peel_height = height;

  resize_peel_texture(state->peel_color_tex,
      GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE);

  for( i = 0; i < 2; i++ )
  {
    resize_peel_texture(state->peel_depth_tex[i],
        GL_DEPTH_COMPONENT24, width, height, GL_DEPTH_COMPONENT, GL_FLOAT);
  }

  resize_peel_texture(state->layer_depth_tex,
      GL_DEPTH_COMPONENT24, width, height, GL_DEPTH_COMPONENT, GL_FLOAT);

  resize_peel_texture(state->accum_color_tex,
      GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE);

  /* MSAA transition or sample-count change: destroy stale MS handles.
   * Some drivers reject in-place glRenderbufferStorageMultisample
   * at a different sample count on an attached RBO. */
  if( state->peel_msaa_samples > 0 && eff_msaa != state->peel_msaa_samples )
  {
    for( i = 0; i < 2; i++ )
    {
      GL_DELETE(glDeleteFramebuffers, state->peel_ms_fbo[i]);
      GL_DELETE(glDeleteRenderbuffers, state->peel_ms_color_rbo[i]);
      GL_DELETE(glDeleteRenderbuffers, state->peel_ms_depth_rbo[i]);
    }
  }

  /* Create MS handles when transitioning to MSAA */
  if( !state->peel_ms_fbo[0] && eff_msaa > 0 )
  {
    /* Transition to MSAA: create MS handles and attach */
    glGenFramebuffers(2, state->peel_ms_fbo);

    for( i = 0; i < 2; i++ )
    {
      glGenRenderbuffers(1, &state->peel_ms_color_rbo[i]);
      glBindRenderbuffer(GL_RENDERBUFFER, state->peel_ms_color_rbo[i]);
      glGenRenderbuffers(1, &state->peel_ms_depth_rbo[i]);
      glBindRenderbuffer(GL_RENDERBUFFER, state->peel_ms_depth_rbo[i]);

      glBindFramebuffer(GL_FRAMEBUFFER, state->peel_ms_fbo[i]);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_RENDERBUFFER, state->peel_ms_color_rbo[i]);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
          GL_RENDERBUFFER, state->peel_ms_depth_rbo[i]);
    }
  }
  /* Resize MS renderbuffer storage in place when MSAA active */
  if( eff_msaa > 0 && state->peel_ms_fbo[0] )
  {
    for( i = 0; i < 2; i++ )
    {
      glBindRenderbuffer(GL_RENDERBUFFER, state->peel_ms_color_rbo[i]);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, eff_msaa,
          GL_RGBA8, width, height);

      glBindRenderbuffer(GL_RENDERBUFFER, state->peel_ms_depth_rbo[i]);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, eff_msaa,
          GL_DEPTH_COMPONENT24, width, height);
    }
  }

  state->peel_msaa_samples = eff_msaa;

  /* Verify completeness of all FBOs after resize */
  for( i = 0; i < 2; i++ )
  {
    glBindFramebuffer(GL_FRAMEBUFFER, state->peel_fbo[i]);

    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
      pr_err("Peel FBO %d incomplete\n", i);
      gl_view_peel_free(state);
      return;
    }

    if( eff_msaa > 0 && state->peel_ms_fbo[i] )
    {
      glBindFramebuffer(GL_FRAMEBUFFER, state->peel_ms_fbo[i]);

      if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
      {
        pr_err("Peel MSAA FBO %d incomplete\n", i);
        gl_view_peel_free(state);
        return;
      }
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, state->accum_fbo);

  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
  {
    pr_err("Peel accumulation FBO incomplete\n");
    gl_view_peel_free(state);
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, state->layer_depth_fbo);

  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
  {
    pr_err("Layer depth FBO incomplete\n");
    gl_view_peel_free(state);
    return;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

} /* gl_view_peel_recreate() */

/*-----------------------------------------------------------------------*/

/** gl_view_peel_locs_init() - Look up depth-peel uniform locations for a shader program
 * @locs: output location struct
 * @program: GL shader program handle
 */
  void
gl_view_peel_locs_init(gl_peel_uniform_locs_t *locs, GLuint program)
{
  locs->peel_depth = glGetUniformLocation(program, "u_peel_depth");
  locs->peel_pass = glGetUniformLocation(program, "u_peel_pass");
  locs->layer_depth = glGetUniformLocation(program, "u_layer_depth");
  locs->coplanar_pass = glGetUniformLocation(program, "u_coplanar_pass");

} /* gl_view_peel_locs_init() */

/*-----------------------------------------------------------------------*/

/** gl_view_set_peel_uniforms() - Set depth-peel uniforms for the current peel pass
 * @locs: uniform locations obtained from gl_view_peel_locs_init()
 * @params: per-frame render parameters (peel_pass, viewport dimensions)
 */
  void
gl_view_set_peel_uniforms(const gl_peel_uniform_locs_t *locs,
    const gl_render_params_t *params)
{
  glUniform1i(locs->peel_pass, params->peel_pass);
  glUniform1i(locs->peel_depth, PEEL_DEPTH_TEX_UNIT);
  glUniform1i(locs->coplanar_pass, params->coplanar_pass);
  glUniform1i(locs->layer_depth, LAYER_DEPTH_TEX_UNIT);

} /* gl_view_set_peel_uniforms() */

/*-----------------------------------------------------------------------*/

/** gl_view_peel_render() - Run depth-peel transparency passes
 * @state: view state with peel FBO resources
 * @active_fbo: opaque-pass FBO (source for depth blit, target for final composite)
 * @render_params: frame parameter snapshot copied for transparent pass mutation
 * @items: sorted transparent items (index, alpha, sort_order, depth)
 * @count: number of transparent items
 *
 * Prepares each transparent renderable, then runs PEEL_PASSES front-to-back
 * depth-peel iterations.  Each pass extracts the next-nearest depth layer,
 * composites it into an accumulation buffer via the under operator, then
 * blends the final accumulated color over the opaque framebuffer.
 */
  void
gl_view_peel_render(gl_view_state_t *state, GLuint active_fbo,
    gl_render_params_t render_params,
    const gl_trans_item_t *items, int count)
{
  gboolean use_ms;
  int pw, ph;
  int j, k;

  pw = state->peel_width;
  ph = state->peel_height;
  use_ms = (state->peel_ms_fbo[0] != 0);

  /* Prepare all transparent renderables once */
  for( j = 0; j < count; j++ )
  {
    gl_renderable_t *r = &g_array_index(
        state->renderables, gl_renderable_t, items[j].index);

    render_params.alpha = items[j].alpha;
    render_params.peel_pass = 0;
    r->prepare(r->ctx, &render_params);
  }

  /* Clear accumulation buffer to fully transparent */
  glBindFramebuffer(GL_FRAMEBUFFER, state->accum_fbo);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  /* Depth-peel passes with coplanar accumulation.
   *
   * Each peel iteration has two sub-passes:
   *
   * Sub-pass A (depth discovery): color write OFF, depth write ON,
   * GL_LESS.  Populates the depth buffer with this layer's nearest
   * surviving depth without writing color.
   *
   * After sub-pass A, the discovered depth is blit to layer_depth_tex
   * so sub-pass B can sample it without GL feedback.
   *
   * Sub-pass B (coplanar accumulation): color write ON, depth write
   * OFF, depth test disabled.  Fragment shader's peel discard
   * rejects previous layers; coplanar tolerance test rejects
   * fragments outside dz-based tolerance of layer_depth_tex.  Alpha-over
   * blend composites all same-depth fragments (stacked gel model).
   * Hardware depth test is disabled because GL_LEQUAL would reject
   * coplanar fragments differing by even 1 ULP from the sub-pass A
   * winner before the shader's tolerance check could run.
   *
   * When MSAA is active, both sub-passes target multisampled peel
   * FBOs.  Depth resolves to single-sample between sub-passes for
   * the layer_depth blit; full resolve after sub-pass B for the
   * under-composite. */
  for( k = 0; k < PEEL_PASSES; k++ )
  {
    int cur = k % 2;
    int prev = (k + 1) % 2;
    GLuint render_fbo;

    render_fbo = use_ms ? state->peel_ms_fbo[cur] : state->peel_fbo[cur];

    /* Blit opaque depth into this pass's render FBO */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, active_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fbo);
    glBlitFramebuffer(0, 0, pw, ph, 0, 0, pw, ph,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    /* Clear color — depth was just blitted from opaque */
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Bind previous pass's depth texture for peel discard */
    glActiveTexture(GL_TEXTURE0 + PEEL_DEPTH_TEX_UNIT);
    glBindTexture(GL_TEXTURE_2D, state->peel_depth_tex[prev]);
    glActiveTexture(GL_TEXTURE0);

    /* Select this layer for both sub-passes. */
    render_params.peel_pass = k;

    /* --- Sub-pass A: depth discovery ---
     * Color mask OFF, depth write ON, GL_LESS.
     * Finds the nearest surviving fragment per pixel. */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_BLEND);

    render_params.coplanar_pass = 0;

    for( j = 0; j < count; j++ )
    {
      gl_renderable_t *r = &g_array_index(
          state->renderables, gl_renderable_t, items[j].index);

      render_params.alpha = items[j].alpha;
      r->render(r->ctx, &render_params);
    }

    /* Resolve MSAA depth for layer_depth blit */
    if( use_ms )
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, state->peel_ms_fbo[cur]);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->peel_fbo[cur]);
      glBlitFramebuffer(0, 0, pw, ph, 0, 0, pw, ph,
          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }

    /* Blit discovered depth to layer_depth_tex (avoids GL feedback
     * when sub-pass B samples layer depth while rendering to the
     * same peel FBO) */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, state->peel_fbo[cur]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->layer_depth_fbo);
    glBlitFramebuffer(0, 0, pw, ph, 0, 0, pw, ph,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    /* Bind layer depth texture for coplanar tolerance test */
    glActiveTexture(GL_TEXTURE0 + LAYER_DEPTH_TEX_UNIT);
    glBindTexture(GL_TEXTURE_2D, state->layer_depth_tex);
    glActiveTexture(GL_TEXTURE0);

    /* --- Sub-pass B: coplanar accumulation ---
     * Color mask ON, depth write OFF, depth test disabled.
     * The shader's peel discard rejects previous layers and
     * the coplanar tolerance test rejects non-layer fragments,
     * so the hardware depth test must not further restrict.
     * GL_LEQUAL would reject coplanar fragments that are even
     * 1 ULP farther than sub-pass A's winner. */
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    render_params.coplanar_pass = 1;

    for( j = 0; j < count; j++ )
    {
      gl_renderable_t *r = &g_array_index(
          state->renderables, gl_renderable_t, items[j].index);

      render_params.alpha = items[j].alpha;
      r->render(r->ctx, &render_params);
    }

    /* Re-enable depth test for next iteration (sub-pass A needs GL_LESS) */
    glEnable(GL_DEPTH_TEST);

    /* Resolve multisampled color + depth to single-sample.
     * Color for the under-composite; depth for next pass's
     * peel discard texture read. */
    if( use_ms )
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, state->peel_ms_fbo[cur]);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->peel_fbo[cur]);
      glBlitFramebuffer(0, 0, pw, ph, 0, 0, pw, ph,
          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }

    /* Under-composite this peel layer into accumulation buffer.
     * Under operator: dst = src * (1 - dst.a) + dst */
    glBindFramebuffer(GL_FRAMEBUFFER, state->accum_fbo);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state->peel_color_tex);

    glUseProgram(state->composite_program);
    glUniform1i(state->composite_u_layer, 0);

    glBindVertexArray(state->composite_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

  } /* for each peel pass */

  /* Final composite: blend accumulated transparent color
   * over the opaque framebuffer using premultiplied alpha.
   * dst = src * 1 + dst * (1 - src.a) */
  glBindFramebuffer(GL_FRAMEBUFFER, active_fbo);
  glViewport(0, 0, pw, ph);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->accum_color_tex);

  glUseProgram(state->composite_program);
  glUniform1i(state->composite_u_layer, 0);

  glBindVertexArray(state->composite_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  /* Restore GL state for subsequent passes (notice overlay, MSAA blit) */
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glClearColor(state->content.background.r, state->content.background.g,
      state->content.background.b, 1.0f);

} /* gl_view_peel_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
