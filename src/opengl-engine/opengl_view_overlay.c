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
#include "opengl_view_program.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

typedef struct
{
  gl_view_state_t *view;
  gl_batch_slot_t slots[GL_VIEW_MAX_BATCHES];
  gboolean initialized;
  mat4 cached_mvp;
  mat4 cached_mv;
  gl_view_content_t ovl_content;

} gl_overlay_ctx_t;

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
 * The parent render protocol deposits ovl_content before the frame survey.
 */
  static void
gl_overlay_prepare(void *ctx, const gl_render_params_t *_params)
{
  gl_overlay_ctx_t *ovl = ctx;
  gl_view_state_t *view = ovl->view;
  int i;

  (void)_params;

  if( !ovl->initialized )
    return;

  if( ovl->ovl_content.batch_count <= 0 )
    return;

  for( i = 0; i < ovl->ovl_content.batch_count; i++ )
  {
    const gl_draw_batch_t *batch = &ovl->ovl_content.batches[i];

    if( batch->vertex_count <= 0 )
      continue;

    gl_view_upload_batch(&ovl->slots[i],
        &view->programs[batch->program_key], ovl->ovl_content.layout, batch);
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

  /* The pass presents its own cached matrices, so the scene projection this
   * frame carries reaches only the peel state the bound program reads. */
  const gl_batch_pass_t pass = {
    .programs = ovl->view->programs,
    .noise_tex = ovl->view->noise_tex,
    .transparency_active = ovl->view->transparency_active,
    .content = &ovl->ovl_content,
    .slots = ovl->slots,
    .mvp = (const float *)ovl->cached_mvp,
    .mv = (const float *)ovl->cached_mv
  };

  gl_view_draw_batches(&pass, params);

} /* gl_overlay_render() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_is_active() - Report whether prepared overlay batches exist
 * @ctx: overlay context
 */
  static gboolean
gl_overlay_is_active(void *ctx)
{
  gl_overlay_ctx_t *ovl = ctx;

  return( ovl->initialized && ovl->ovl_content.batch_count > 0 );

} /* gl_overlay_is_active() */

/*-----------------------------------------------------------------------*/

/** gl_overlay_far_extent() - Return the overlay's scaled geometry extent for clip plane computation
 * @ctx: overlay context
 * @r_max: current maximum scene extent
 *
 * Reads the ovl_content the parent render protocol deposited this frame.
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

  if( ovl->view->overlay_content == &ovl->ovl_content )
    ovl->view->overlay_content = NULL;

  gl_batch_slots_destroy(ovl->slots, GL_VIEW_MAX_BATCHES);

  g_free(ovl);

} /* gl_overlay_free() */

/*-----------------------------------------------------------------------*/

/** gl_view_overlay_renderable_new() - Create overlay renderable for second-pass rendering
 * @state: view state
 */
  gl_renderable_t
gl_view_overlay_renderable_new(gl_view_state_t *state)
{
  gl_overlay_ctx_t *ovl;
  gl_renderable_t r;

  if( !state->config->presents_overlay )
    return( (gl_renderable_t){0} );

  ovl = g_new0(gl_overlay_ctx_t, 1);
  ovl->view = state;

  gl_batch_slots_init(ovl->slots, GL_VIEW_MAX_BATCHES);

  ovl->initialized = TRUE;
  state->overlay_content = &ovl->ovl_content;

  r = (gl_renderable_t){
    .render               = gl_overlay_render,
    .prepare              = gl_overlay_prepare,
    .destroy              = gl_overlay_free,
    .is_active            = gl_overlay_is_active,
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
