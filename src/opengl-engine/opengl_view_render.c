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

#include "opengl_view_render.h"
#include "opengl_view_peel.h"
#include "opengl_view_msaa.h"
#include "opengl_view_notice.h"
#include "opengl_gradient_overlay.h"
#include "opengl_view_fit.h"
#include "../render/render_dispatch.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/** gl_view_setup_attribs() - Configure vertex attribute pointers in VAO
 */
  void
gl_view_setup_attribs(
    GLuint vao,
    GLuint vbo,
    const gl_vertex_attrib_t *attribs,
    const GLint *attrib_locations,
    int attrib_count,
    int vertex_stride)
{
  int i;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  for( i = 0; i < attrib_count; i++ )
  {
    glEnableVertexAttribArray(attrib_locations[i]);
    glVertexAttribPointer(
        attrib_locations[i],
        attribs[i].components,
        GL_FLOAT,
        GL_FALSE,
        vertex_stride,
        (void *)(long)attribs[i].offset);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

} /* gl_view_setup_attribs() */

/*-----------------------------------------------------------------------*/

/* gl_trans_item_t defined in opengl_view.h */

/*-----------------------------------------------------------------------*/

/** gl_view_notice_once() - Present a capability notice the first time offered
 * @state:  view state the notice appears on
 * @notice: text advertising the capability, or NULL when none is offered
 * @shown:  guard the caller holds for the session
 */
  static void
gl_view_notice_once(gl_view_state_t *state, const char *notice,
    gboolean *shown)
{
  if( *shown || notice == NULL )
    return;

  gl_view_show_notice(state, notice, GL_VIEW_NOTICE_HOLD_MS,
      GL_NOTICE_BOTTOM_LEFT);

  *shown = TRUE;

} /* gl_view_notice_once() */

/*-----------------------------------------------------------------------*/

/** gl_view_show_ctrl_notice() - Advertise the ctrl+scroll capability
 * @state: view state naming the input operations of the presenting domain
 *
 * One guard serves every view, so the notice appears on the first frame of
 * the session offering the capability.  The capability stands on the view
 * itself, so this precedes the frame body.
 */
  static void
gl_view_show_ctrl_notice(gl_view_state_t *state)
{
  static gboolean ctrl_shown = FALSE;
  const gl_view_input_ops_t *input = state->config->input;

  if( input == NULL )
    return;

  gl_view_notice_once(state, input->ctrl_scroll_notice, &ctrl_shown);

} /* gl_view_show_ctrl_notice() */

/*-----------------------------------------------------------------------*/

/** gl_view_show_scale_notice() - Advertise the shift+scroll capability
 * @state: view state holding the content this frame deposited
 *
 * Structure scaling acts on the overlay geometry, so this follows the frame
 * body and presents only while the frame carries that geometry.
 */
  static void
gl_view_show_scale_notice(gl_view_state_t *state)
{
  static gboolean scale_shown = FALSE;
  const gl_view_input_ops_t *input = state->config->input;
  const gl_view_content_t *overlay = state->overlay_content;

  if( input == NULL || overlay == NULL || overlay->batch_count <= 0 )
    return;

  gl_view_notice_once(state, input->shift_scroll_notice, &scale_shown);

} /* gl_view_show_scale_notice() */

/*-----------------------------------------------------------------------*/

/** gl_view_gradient_overlay_create() - Build the gradient legend HUD
 * @state: view state receiving the overlay
 *
 * Deferred to the first frame carrying legend content, where the GL context
 * of the presenting area is current.
 */
  static void
gl_view_gradient_overlay_create(gl_view_state_t *state)
{
  state->overlay = gradient_overlay_new();

  if( state->overlay == NULL )
    return;

  gradient_overlay_set_viewport(state->overlay,
      state->base.view->width, state->base.view->height);

} /* gl_view_gradient_overlay_create() */

/*-----------------------------------------------------------------------*/

/** gl_view_frame_content_reset() - Clear the content each frame rebuilds
 * @state: view state whose frame content resets
 *
 * The parent render protocol deposits the domain content of every frame,
 * including the colors its leaves paint with.
 */
  static void
gl_view_frame_content_reset(gl_view_state_t *state)
{
  state->content.axes = (gl_axes_content_t){0};
  state->content.status_message = NULL;
  state->content.gradient = (gradient_result_t){NULL, 0};

  if( state->overlay_content != NULL )
    state->overlay_content->batch_count = 0;

} /* gl_view_frame_content_reset() */

/*-----------------------------------------------------------------------*/

/** on_render() - GtkGLArea render signal handler
 * @area: GL area widget being rendered
 * @context: GL context
 * @user_data: pointer to gl_view_state_t
 */
  static gboolean
on_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
  gl_view_state_t *state;
  gl_render_params_t render_params = {0};
  mat4 mvp, mv;
  float camera_distance;
  GLint default_fbo = 0;
  guint32 active_mask;
  float eff_alphas[MAX_RENDERABLES];
  guint i;

  state = (gl_view_state_t *)user_data;

  if( !state || !state->initialized )
    return( FALSE );

  gl_view_frame_content_reset(state);

  gl_view_show_ctrl_notice(state);

  if( !render(&state->base) )
    return( FALSE );

  gl_view_show_scale_notice(state);

  camera_distance = state->content.r_max * GL_VIEW_BASE_DISTANCE_FACTOR /
                    state->base.view->zoom;
  state->cached_camera_distance = camera_distance;

  /* Active survey — build mask and compute far extent in one pass */
  {
    float effective_far, nearest_point, farthest_point, near_plane, far_plane, ext;

    active_mask = 0;
    effective_far = 0.0f;
    state->transparency_active =
        !rc_config.opengl_transparent_on_click || state->drag_active;

    for( i = 0; i < state->renderables->len; i++ )
    {
      gl_renderable_t *r = &g_array_index(
          state->renderables, gl_renderable_t, i);

      if( r->is_active != NULL && !r->is_active(r->ctx) )
        continue;

      active_mask |= (1u << i);

      /* Per-type transparency: each renderable's get_alpha() returns
       * classification alpha from rc_config or batch min-alpha.
       * The on-click toggle suppresses transparency when not dragging. */
      {
        float base_alpha = r->get_alpha(r->ctx);

        if( r->transparent_on_drag )
          eff_alphas[i] = state->transparency_active ? base_alpha : 1.0f;
        else
          eff_alphas[i] = base_alpha;
      }

      /* Generate content before extent is queried — allows renderables
       * that produce data as a side effect of extent calculation to
       * do so in a dedicated, clearly-named step */
      if( r->generate )
        r->generate(r->ctx);

      if( r->far_extent )
      {
        ext = r->far_extent(r->ctx, state->content.r_max);

        if( ext > effective_far )
          effective_far = ext;
      }
    }

    /* clip_extent accounts for translation offsets; all providers set it */
    if( effective_far < state->content.clip_extent )
    {
      effective_far = state->content.clip_extent;
    }

    nearest_point = camera_distance - state->content.clip_extent;
    farthest_point = camera_distance + effective_far;

    /* Extra margin provides window-space headroom for per-patch
     * gl_FragDepth bias without pushing fragments past 1.0. */
    far_plane = farthest_point * 1.5f;

    if( rc_config.opengl_orthographic )
    {
      /* Orthographic near plane set negative so depth clipping does not
       * advance with camera_distance as zoom increases.  The camera
       * position along Z is nominal in parallel projection; the full
       * scene depth must remain visible at any zoom level. */
      near_plane = -far_plane;
    }
    else if( nearest_point > 0.0f )
    {
      near_plane = nearest_point * 0.8f;
    }
    else
    {
      /* Bound near/far ratio to ~10000:1 for usable depth precision.
       * Fixed 0.001f caused extreme ratios with large scenes. */
      near_plane = far_plane / 10000.0f;
    }

    state->cached_near_plane = near_plane;
    state->cached_far_plane = far_plane;

    gl_view_build_mvp(state, state->content.model_scale, mvp, mv);
  }

  glm_mat4_copy(mvp, render_params.mvp);
  glm_mat4_copy(mv, render_params.mv);
  render_params.r_max = state->content.r_max;
  render_params.view_axis = state->content.view_axis;
  render_params.view_axis_label = state->content.view_axis_label;
  render_params.flow_phase = flow_phase;

  /* Framebuffer setup */
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_fbo);

  if( state->msaa_fbo )
    glBindFramebuffer(GL_FRAMEBUFFER, state->msaa_fbo);

  glClearColor(state->content.background.r, state->content.background.g,
      state->content.background.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  /* Opaque pass — depth buffer fully populated */
  for( i = 0; i < state->renderables->len; i++ )
  {
    gl_renderable_t *r = &g_array_index(
        state->renderables, gl_renderable_t, i);

    if( !(active_mask & (1u << i)) )
      continue;

    float eff_alpha = eff_alphas[i];

    if( eff_alpha < 1.0f || r->force_peel )
      continue;

    /* Opaque pass: peel_pass=0 (no discard in shader) */
    render_params.alpha = eff_alpha;
    render_params.peel_pass = 0;
    r->prepare(r->ctx, &render_params);
    r->render(r->ctx, &render_params);
  }

  /* Depth-peeled transparent pass — order-independent transparency
   * via front-to-back depth peeling with under-operator compositing.
   * Each pass extracts the next-nearest depth layer from all
   * transparent renderables, then composites it into the
   * accumulation buffer.  Final result is blended over the opaque
   * framebuffer. */
  {
    gl_trans_item_t items[MAX_RENDERABLES];
    int trans_count, j, k;

    trans_count = 0;

    for( i = 0; i < state->renderables->len; i++ )
    {
      gl_renderable_t *r = &g_array_index(
          state->renderables, gl_renderable_t, i);

      if( !(active_mask & (1u << i)) )
        continue;

      float eff_alpha = eff_alphas[i];

      if( eff_alpha >= 1.0f && !r->force_peel )
        continue;

      items[trans_count].alpha = eff_alpha;
      items[trans_count].sort_order = r->transparent_sort_order;

      /* Depth from effective extent: smaller extent (inner geometry)
       * sorts first so it renders before outer geometry.  This
       * adapts to user scaling of the overlay without a static
       * sort_order assumption about containment direction. */
      if( r->far_extent )
      {
        items[trans_count].depth = r->far_extent(r->ctx, state->content.r_max);
      }
      else
      {
        items[trans_count].depth = 0.0f;
      }
      items[trans_count].index = (int)i;
      trans_count++;
    }

    /* Insertion sort: ascending sort_order, then ascending depth */
    for( j = 1; j < trans_count; j++ )
    {
      gl_trans_item_t key = items[j];

      k = j - 1;

      while( k >= 0 &&
             (items[k].sort_order > key.sort_order ||
              (items[k].sort_order == key.sort_order &&
               items[k].depth > key.depth)) )
      {
        items[k + 1] = items[k];
        k--;
      }

      items[k + 1] = key;
    }

    if( trans_count > 0 && state->peel_fbo[0] )
    {
      GLuint active_fbo;

      active_fbo = state->msaa_fbo
        ? state->msaa_fbo : (GLuint)default_fbo;

      gl_view_peel_render(state, active_fbo, render_params,
          items, trans_count);
    }
    else if( trans_count > 0 )
    {
      pr_warn("Transparent renderables skipped: peel FBOs unavailable\n");
    }
  }

  /* 2D HUD — gradient legend overlay (set by render() via set_gradient) */
  if( state->content.gradient.surface != NULL )
  {
    if( state->overlay == NULL )
      gl_view_gradient_overlay_create(state);

    if( state->overlay != NULL )
    {
      gradient_overlay_upload_surface(state->overlay,
          state->content.gradient.surface,
          state->content.gradient.version);
      gradient_overlay_render(state->overlay);
    }
  }

  /* Surface dimensions for 2D notice overlay */
  {
    int surf_width, surf_height;

    surf_width = state->base.view->width;
    surf_height = state->base.view->height;

    /* Synchronize persistent notice with scene status_message */
    gl_view_sync_status_notice(state);

    /* Render notice if active */
    if( state->notice_active && state->notice_text )
      gl_view_render_notice(state, surf_width, surf_height);
  }

  /* Resolve MSAA into the default FBO through a single-sample texture copy.
   * A direct multisample blit into GTK's framebuffer is rejected under
   * Wayland/EGL, so the resolve and the default-FBO write are split. */
  if( state->msaa_fbo )
    gl_view_msaa_resolve(state, default_fbo);

  return( TRUE );

} /* on_render() */

/*-----------------------------------------------------------------------*/

/** gl_view_render_connect() - Wire the render signal handler to a view's area
 * @state: view state whose area produces frames and receives on_render()
 */
  void
gl_view_render_connect(gl_view_state_t *state)
{
  g_signal_connect(state->gl_area, "render", G_CALLBACK(on_render), state);

} /* gl_view_render_connect() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
