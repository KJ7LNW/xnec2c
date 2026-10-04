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

#include "opengl_view_msaa.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** gl_view_scene_free() - Release scene framebuffer, color, and depth renderbuffer resources
 * @state: view state containing scene resources to free
 */
  void
gl_view_scene_free(gl_view_state_t *state)
{
  if( !state )
    return;

  if( state->scene_fbo )
  {
    glDeleteFramebuffers(1, &state->scene_fbo);
    state->scene_fbo = 0;
  }

  if( state->scene_color_rbo )
  {
    glDeleteRenderbuffers(1, &state->scene_color_rbo);
    state->scene_color_rbo = 0;
  }

  if( state->scene_depth_rbo )
  {
    glDeleteRenderbuffers(1, &state->scene_depth_rbo);
    state->scene_depth_rbo = 0;
  }

  if( state->resolve_fbo )
  {
    glDeleteFramebuffers(1, &state->resolve_fbo);
    state->resolve_fbo = 0;
  }

  if( state->resolve_color_tex )
  {
    glDeleteTextures(1, &state->resolve_color_tex);
    state->resolve_color_tex = 0;
  }

  state->msaa_samples = 0;

} /* gl_view_scene_free() */

/*-----------------------------------------------------------------------*/

/** gl_view_scene_recreate() - Resize scene resources, creating handles only on first call
 * @state: view state to update with new scene resources
 * @requested_samples: number of MSAA samples to use (clamped to hardware limit);
 *   a request below 2 allocates single-sample scene storage
 *
 * Separates handle lifecycle from storage allocation: glGen + FBO attach
 * run once, glRenderbufferStorageMultisample runs on every resize.
 */
  void
gl_view_scene_recreate(gl_view_state_t *state, int requested_samples)
{
  GLint max_samples;
  int width, height;
  int samples;

  if( !state || !state->initialized )
    return;

  /* Query hardware limit and clamp */
  glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
  samples = (requested_samples > max_samples) ? max_samples : requested_samples;

  /* Allocate single-sample storage for requests below 2 samples */
  samples = (samples < 2) ? 0 : samples;

  width = state->msaa_width;
  height = state->msaa_height;

  if( width == 0 || height == 0 )
    return;

  /* Sample count changed — delete stale handles.
   * Some drivers reject in-place glRenderbufferStorageMultisample
   * at a different sample count on an attached RBO. */
  if( state->scene_fbo && state->msaa_samples != samples )
    gl_view_scene_free(state);

  /* Create handles and FBO attachments (first call or after free) */
  if( !state->scene_fbo )
  {
    glGenRenderbuffers(1, &state->scene_color_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, state->scene_color_rbo);
    glGenRenderbuffers(1, &state->scene_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, state->scene_depth_rbo);
    glGenFramebuffers(1, &state->scene_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, state->scene_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_RENDERBUFFER, state->scene_color_rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, state->scene_depth_rbo);
  }

  /* Resize storage in place (valid on both fresh and existing RBOs) */
  glBindRenderbuffer(GL_RENDERBUFFER, state->scene_color_rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
      GL_RGBA8, width, height);

  glBindRenderbuffer(GL_RENDERBUFFER, state->scene_depth_rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
      GL_DEPTH_COMPONENT24, width, height);

  state->msaa_samples = samples;

  /* Verify completeness after resize */
  glBindFramebuffer(GL_FRAMEBUFFER, state->scene_fbo);

  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
  {
    pr_err("MSAA framebuffer incomplete\n");
    gl_view_scene_free(state);
  }

  /* Create and size the single-sample resolve target.  Guarded by
   * scene_fbo: a failed completeness check above already freed it, and
   * allocating here would orphan the resolve resources. */
  if( state->scene_fbo )
  {
    if( !state->resolve_color_tex )
    {
      glGenTextures(1, &state->resolve_color_tex);
      glBindTexture(GL_TEXTURE_2D, state->resolve_color_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    if( !state->resolve_fbo )
      glGenFramebuffers(1, &state->resolve_fbo);

    glBindTexture(GL_TEXTURE_2D, state->resolve_color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, state->resolve_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, state->resolve_color_tex, 0);

    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
      pr_err("MSAA resolve framebuffer incomplete\n");
      gl_view_scene_free(state);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

} /* gl_view_scene_recreate() */

/*-----------------------------------------------------------------------*/

/** gl_view_scene_present() - Resolve scene color to a texture, then draw it into default_fbo
 * @state: view state holding the scene and resolve resources
 * @default_fbo: GTK-provided framebuffer that receives the resolved image
 *
 * A direct multisample blit into GTK's framebuffer is rejected under
 * Wayland/EGL, so the multisample resolve targets a single-sample texture
 * and a fullscreen textured draw writes that texture into default_fbo.
 */
  void
gl_view_scene_present(gl_view_state_t *state, GLint default_fbo)
{
  int w = state->msaa_width;
  int h = state->msaa_height;

  /* Hardware resolve: multisample color → single-sample texture */
  glBindFramebuffer(GL_READ_FRAMEBUFFER, state->scene_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->resolve_fbo);
  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h,
      GL_COLOR_BUFFER_BIT, GL_NEAREST);

  /* Fullscreen textured copy of the resolved color into default_fbo */
  glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
  glViewport(0, 0, w, h);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_BLEND);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->resolve_color_tex);

  glUseProgram(state->composite_program);
  glUniform1i(state->composite_u_layer, 0);

  glBindVertexArray(state->composite_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  /* Restore the frame-entry depth contract: on_render() clears the depth
   * buffer next frame gated by the depth mask and never re-asserts it. */
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

} /* gl_view_scene_present() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
