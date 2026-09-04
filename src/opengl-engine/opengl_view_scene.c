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
#include "opengl_view_program.h"
#include "opengl_gradient_overlay.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/* Scene rendering context — owns the GL buffers for primary geometry */
typedef struct
{
  gl_view_state_t *view;
  gl_batch_slot_t slots[GL_VIEW_MAX_BATCHES];

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
static void gl_scene_prepare(void *ctx, const gl_render_params_t *_params);
static void gl_scene_render(void *ctx, const gl_render_params_t *params);
static gboolean gl_scene_is_active(void *ctx);
static float gl_scene_far_extent(void *ctx, float r_max);
static void gl_scene_free(void *ctx);

/*-----------------------------------------------------------------------*/

/** gl_scene_prepare() - Upload scene vertex data on generation change
 * @ctx: scene context
 * @_params: unused uniform render parameters
 */
  static void
gl_scene_prepare(void *ctx, const gl_render_params_t *_params)
{
  gl_scene_ctx_t *sc = ctx;
  gl_view_state_t *view = sc->view;
  gl_view_content_t *c = &view->content;

  (void)_params;

  /* Upload each changed batch to its own VBO and configure its VAO */
  {
    int i;

    for( i = 0; i < c->batch_count; i++ )
    {
      const gl_draw_batch_t *batch = &c->batches[i];

      if( batch->vertex_count <= 0 )
        continue;

      gl_view_upload_batch(&sc->slots[i],
          &view->programs[batch->program_key], c->layout, batch);
    }
  }

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

  /* Present authored batch alpha while transparency is active. On-click mode
   * suppresses transparency when not dragging for transparent-on-drag
   * renderables. */
  const gl_batch_pass_t pass = {
    .programs = view->programs,
    .noise_tex = view->noise_tex,
    .transparency_active = view->transparency_active,
    .content = &view->content,
    .slots = sc->slots,
    .mvp = (const float *)params->mvp,
    .mv = (const float *)params->mv
  };

  gl_view_draw_batches(&pass, params);

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

  gl_batch_slots_destroy(sc->slots, GL_VIEW_MAX_BATCHES);

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

  sc = g_new0(gl_scene_ctx_t, 1);
  sc->view = state;

  gl_batch_slots_init(sc->slots, GL_VIEW_MAX_BATCHES);

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
