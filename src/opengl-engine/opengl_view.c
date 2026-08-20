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

#include "opengl_view.h"
#include "opengl_view_scene.h"
#include "opengl_view_overlay.h"
#include "opengl_view_render.h"
#include "opengl_view_msaa.h"
#include "opengl_view_peel.h"
#include "opengl_view_notice.h"
#include "opengl_view_fit.h"
#include "opengl_axes.h"
#include "opengl_cairo_overlay.h"
#include "opengl_ground_plane.h"
#include "../shared.h"
#include "../mem/mem.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** gl_view_gpu_release() - Release every GPU resource the context owns
 * @state: view state whose GL context is going away
 */
  void
gl_view_gpu_release(gl_view_state_t *state)
{
  if( state == NULL )
    return;

  if( state->notice_surface )
  {
    cairo_surface_destroy(state->notice_surface);
    state->notice_surface = NULL;
  }

  state->notice_surface_valid = FALSE;

  if( state->notice_overlay )
  {
    cairo_gl_overlay_free(state->notice_overlay);
    state->notice_overlay = NULL;
  }

  /* Release shared noise texture before renderables */
  if( state->noise_tex )
  {
    glDeleteTextures(1, &state->noise_tex);
    state->noise_tex = 0;
  }

  /* Destroy all renderables in reverse registration order */
  if( state->renderables )
  {
    int ri;

    for( ri = (int)state->renderables->len - 1; ri >= 0; ri-- )
    {
      gl_renderable_t *r = &g_array_index(
          state->renderables, gl_renderable_t, ri);

      if( r->destroy )
        r->destroy(r->ctx);
    }

    g_array_free(state->renderables, TRUE);
    state->renderables = NULL;
  }

  if( state->overlay )
  {
    gradient_overlay_free(state->overlay);
    state->overlay = NULL;
  }

  /* Release depth-peel FBO resources (per-resize lifecycle) */
  gl_view_peel_free(state);

  /* Release composite resources (per-realize lifecycle) */
  GL_DELETE(glDeleteBuffers, state->composite_vbo);
  GL_DELETE(glDeleteVertexArrays, state->composite_vao);
  GL_DELETE_OBJ(glDeleteProgram, state->composite_program);
  GL_DELETE_OBJ(glDeleteShader, state->composite_vs);
  GL_DELETE_OBJ(glDeleteShader, state->composite_fs);

  gl_view_msaa_free(state);

  /* A later realize rebuilds the framebuffers, so the recorded dimensions
   * must not match the resize that follows it. */
  state->msaa_width = 0;
  state->msaa_height = 0;

  /* Mark the notice inactive so a fade still armed becomes a no-op that
   * removes itself instead of requesting frames from the released context;
   * gl_view_surface_free() releases the timer with the state it carries. */
  state->notice_active = FALSE;
  state->initialized = FALSE;

} /* gl_view_gpu_release() */

/*-----------------------------------------------------------------------*/

/** gl_view_surface_free() - Release the view state a canvas held
 * @surface: surface leaving its canvas
 *
 * Reaches no widget: every teardown path finalizes the drawing widget, and
 * with it the GPU resources, before the canvas drops its surfaces. Release
 * the domain content cache at this surface-lifetime edge.
 */
  void
gl_view_surface_free(render_surface_t *surface)
{
  gl_view_state_t *state = gl_view_state(surface);

  if( state == NULL )
    return;

  state->config->content_cleanup();

  /* Stop the fade before releasing its callback payload. */
  if( state->notice_timeout_id )
  {
    g_source_remove(state->notice_timeout_id);
    state->notice_timeout_id = 0;
  }

  g_free(state->notice_text);

  mem_free(&state);

} /* gl_view_surface_free() */

/*-----------------------------------------------------------------------*/

/** gl_view_signal_init_failed() - Signal that OpenGL initialization failed
 * @state: view state carrying the failure callback
 *
 * Schedules the on_gl_init_failed callback on the main-loop idle queue
 * so the renderer falls back to Cairo.  Safe to call when no callback
 * is configured (no-op).
 */
  static void
gl_view_signal_init_failed(gl_view_state_t *state)
{
  if( state->config->on_gl_init_failed )
    g_idle_add_once(state->config->on_gl_init_failed, NULL);

} /* gl_view_signal_init_failed() */

/*-----------------------------------------------------------------------*/

/** on_realize() - GtkGLArea realize signal handler
 * @area: GL area widget
 * @user_data: view state
 */
  static void
on_realize(GtkGLArea *area, gpointer user_data)
{
  gl_view_state_t *state;
  gl_renderable_t r;

  state = (gl_view_state_t *)user_data;

  gtk_gl_area_make_current(area);

  if( gtk_gl_area_get_error(area) != NULL )
  {
    pr_err("OpenGL context error — no OpenGL implementation available on this display\n");
    gl_view_signal_init_failed(state);
    return;
  }

  /* Build renderables array */
  state->renderables = g_array_sized_new(FALSE, TRUE,
      sizeof(gl_renderable_t), 4);

  /* Create scene renderable — fatal on failure */
  r = gl_view_scene_renderable_new(state);
  if( !r.render )
  {
    pr_err("Disabling OpenGL: scene shader failed\n");
    gl_view_signal_init_failed(state);
    return;
  }
  g_array_append_val(state->renderables, r);

  /* Create overlay renderable if configured */
  r = gl_view_overlay_renderable_new(state);
  if( r.render )
  {
    g_array_append_val(state->renderables, r);
  }
  else if( state->config->overlay )
  {
    /* Overlay was configured but shader load failed */
    pr_err("Disabling OpenGL: overlay shader failed\n");
    gl_view_signal_init_failed(state);
    return;
  }

  /* Create axes renderer */
  {
    opengl_axes_t *axes;

    axes = opengl_axes_new(&state->content.axes);
    if( !axes )
    {
      pr_err("Disabling OpenGL: axes renderer failed\n");
      gl_view_signal_init_failed(state);
      return;
    }

    r = (gl_renderable_t){
      .render               = opengl_axes_render,
      .prepare              = opengl_axes_prepare,
      .destroy              = opengl_axes_free,
      .is_active            = opengl_axes_is_active,
      .far_extent           = opengl_axes_far_extent,
      .ctx                  = axes,
      .get_alpha            = opengl_axes_get_alpha,
      .origin               = {0.0f, 0.0f, 0.0f},
      .transparent_sort_order = 0
    };

    g_array_append_val(state->renderables, r);
  }

  /* Create the ground plane only when the presenting domain supplies its
   * visibility predicate. */
  if( state->config->ground_plane_is_active != NULL )
  {
    opengl_ground_plane_t *ground_plane;

    ground_plane = opengl_ground_plane_new();
    if( !ground_plane )
    {
      pr_err("Disabling OpenGL: ground plane shader failed\n");
      gl_view_signal_init_failed(state);
      return;
    }

    r = (gl_renderable_t){
      .render               = opengl_ground_plane_render,
      .prepare              = opengl_ground_plane_prepare,
      .destroy              = opengl_ground_plane_free,
      .is_active            = state->config->ground_plane_is_active,
      .far_extent           = opengl_ground_plane_far_extent,
      .ctx                  = ground_plane,
      .get_alpha            = opengl_ground_plane_get_alpha,
      .origin               = {0.0f, 0.0f, 0.0f},
      .transparent_sort_order = 2,
      .transparent_on_drag  = FALSE
    };
    g_array_append_val(state->renderables, r);
  }

  if( state->renderables->len > MAX_RENDERABLES )
  {
    pr_err("Renderable count %u exceeds MAX_RENDERABLES (%d)\n",
        state->renderables->len, MAX_RENDERABLES);
    return;
  }

  state->initialized = TRUE;

  /* Generate LIC noise texture (256x256 grayscale, shared by all renderables).
   * Created here in view state rather than in a single renderable so that
   * both scene and overlay can reference it without ownership ambiguity. */
  {
    enum { NOISE_SIZE = 256 };
    unsigned char noise_data[NOISE_SIZE * NOISE_SIZE];
    unsigned int rng = 42;
    int ni;

    for( ni = 0; ni < NOISE_SIZE * NOISE_SIZE; ni++ )
    {
      rng ^= rng << 13;
      rng ^= rng >> 17;
      rng ^= rng << 5;
      noise_data[ni] = (unsigned char)(rng & 0xFF);
    }

    glGenTextures(1, &state->noise_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, state->noise_tex);

    /* GL_RED for core profile compatibility (GL_LUMINANCE is
     * deprecated in 3.x+ core contexts created by GtkGLArea) */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NOISE_SIZE, NOISE_SIZE, 0,
        GL_RED, GL_UNSIGNED_BYTE, noise_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);
  }

  /* Compile depth-peel composite shader and create fullscreen triangle */
  {
    gl_shader_t cs = {0};
    gboolean ok;

    ok = gl_shader_load(&cs,
        "/gl/peel-composite-vertex.glsl",
        "/gl/peel-composite-fragment.glsl");

    if( ok )
    {
      static const float tri_verts[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
      };
      GLint pos_loc;

      state->composite_program = cs.program;
      state->composite_vs = cs.vertex;
      state->composite_fs = cs.fragment;
      state->composite_u_layer =
        glGetUniformLocation(cs.program, "u_layer");

      pos_loc = glGetAttribLocation(cs.program, "position");

      glGenBuffers(1, &state->composite_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, state->composite_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(tri_verts),
          tri_verts, GL_STATIC_DRAW);

      glGenVertexArrays(1, &state->composite_vao);
      glBindVertexArray(state->composite_vao);
      glEnableVertexAttribArray(pos_loc);
      glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
      pr_err("Disabling OpenGL: peel composite shader failed\n");
      gl_view_signal_init_failed(state);
      return;
    }
  }

  /* Auto-render is disabled, so the first frame of a realized context
   * comes from this request. */
  gl_view_queue_render(state);

} /* on_realize() */

/*-----------------------------------------------------------------------*/

/** on_unrealize() - GtkGLArea unrealize signal handler
 * @area: GL area widget
 * @user_data: view state
 */
  static void
on_unrealize(GtkGLArea *area, gpointer user_data)
{
  gl_view_state_t *state;

  state = (gl_view_state_t *)user_data;

  gtk_gl_area_make_current(area);

  gl_view_gpu_release(state);

} /* on_unrealize() */

/*-----------------------------------------------------------------------*/

/** on_resize() - GtkGLArea resize signal handler
 * @_area: signal source, unread: the view state carries the GL area
 * @width: new width in pixels
 * @height: new height in pixels
 * @user_data: view state
 */
  static void
on_resize(GtkGLArea *_area, int width, int height, gpointer user_data)
{
  gl_view_state_t *state;

  state = (gl_view_state_t *)user_data;

  if( state == NULL )
    return;

  /* Dimensions unchanged — skip FBO resize; the allocation handler already
   * recorded them for a redundant GTK notification. */
  if( width == state->msaa_width && height == state->msaa_height )
  {
    glViewport(0, 0, width, height);
    gl_view_queue_render(state);
    return;
  }

  /* Store dimensions for MSAA recreation */
  state->msaa_width = width;
  state->msaa_height = height;

  /* Recreate MSAA FBO at new dimensions */
  if( rc_config.opengl_msaa_samples > 0 )
    gl_view_recreate_msaa(state, rc_config.opengl_msaa_samples);

  /* Recreate depth-peel FBOs at new dimensions (only when
   * composite shader loaded successfully during realize) */
  if( state->initialized && state->composite_program )
    gl_view_peel_recreate(state, width, height, state->msaa_samples);

  if( state->overlay )
    gradient_overlay_set_viewport(state->overlay, width, height);

  glViewport(0, 0, width, height);

  /* Force redraw so the window does not remain black after resize */
  gl_view_queue_render(state);

} /* on_resize() */

/*-----------------------------------------------------------------------*/

/** on_isolator_realize() - Realize handler for the GtkGLArea wrapper
 * @wrapper: the GtkEventBox wrapping the GtkGLArea
 * @user_data: unused
 *
 * GtkGLArea in GTK3 is a no-window widget (has_window=FALSE); its
 * drawing flows into the ancestor GdkWindow that owns a native X id.
 * Without an intermediate container that carries its own native X
 * window, the GL surface shares the toplevel's backing pixmap, the
 * same drawable Compiz binds via GLX_EXT_texture_from_pixmap.  When
 * the two GLX contexts touch the same drawable, NVIDIA proprietary
 * GLX leaves the pixmap Compiz samples in an undefined state during
 * focus transitions, producing the observed blackout of every widget
 * that lives under that toplevel XID.
 *
 * Forcing the wrapper's GdkWindow native (gdk_window_ensure_native)
 * gives the GL surface its own XID, separating the drawable used by
 * client-side GLX from the one Compiz's TFP binding reads.
 */
  static void
on_isolator_realize(GtkWidget *wrapper, gpointer user_data)
{
  GdkWindow *win;

  (void)user_data;

  win = gtk_widget_get_window(wrapper);
  if( win )
    gdk_window_ensure_native(win);

} /* on_isolator_realize() */

/*-----------------------------------------------------------------------*/

/** gl_view_present_widget() - Resolve the widget the layout presents
 * @state: view state whose area produces frames
 *
 * On the X11 backend, wraps the state's area in a GtkEventBox isolator
 * carrying a native X window that separates the GL surface from the toplevel
 * drawable (see on_isolator_realize()).  On every other backend (e.g.
 * Wayland) the native child window breaks GtkGLArea offscreen compositing,
 * so the area itself is presented.
 */
  static GtkWidget *
gl_view_present_widget(gl_view_state_t *state)
{
  GtkWidget *gl_area = state->gl_area;
  GtkWidget *isolator;
  GdkDisplay *display;

  /* The native-window isolation below is an X11/Compiz/NVIDIA-GLX
   * remedy (commit f77930c).  On other backends, notably Wayland, the
   * forced native child becomes a wl_subsurface that breaks GtkGLArea
   * offscreen compositing (EGL_BAD_SURFACE).  Gate the isolator on the
   * X11 backend, detected by the default display's type name so this
   * carries no build-time dependency on the X11 headers (gdkx.h). */
  display = gdk_display_get_default();

  if( display &&
      g_strcmp0(G_OBJECT_TYPE_NAME(display), "GdkX11Display") == 0 )
  {
    /* Wrap the GtkGLArea in a GtkEventBox with a visible (native)
     * GdkWindow so the GL surface has its own XID independent of the
     * toplevel's backing pixmap.  See on_isolator_realize() above. */
    isolator = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(isolator), TRUE);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(isolator), FALSE);

    gtk_widget_set_size_request(isolator, 400, 400);
    gtk_widget_set_hexpand(isolator, TRUE);
    gtk_widget_set_vexpand(isolator, TRUE);

    gtk_container_add(GTK_CONTAINER(isolator), gl_area);
    gtk_widget_show(gl_area);

    g_signal_connect(isolator, "realize",
        G_CALLBACK(on_isolator_realize), NULL);

    return( isolator );
  }

  gtk_widget_set_size_request(gl_area, 400, 400);
  return( gl_area );

} /* gl_view_present_widget() */

/*-----------------------------------------------------------------------*/

/** gl_view_surface_new() - Build a GL surface and pack it into a container
 * @config: view configuration
 * @input: modifier scroll operations of the presenting domain, or NULL
 * @view: per-view rotation/pan/zoom/drag owner (borrowed, non-NULL)
 * @parent: container the presented widget joins
 *
 * Connects every handler before the widget is shown, so the realize and
 * resize this packing triggers reach a fully wired surface.  Pointer and
 * allocation events reach the presented widget, which carries the window
 * they are delivered to.
 */
  render_surface_t *
gl_view_surface_new(gl_view_config_t *config, const surface_input_ops_t *input,
    view_t *view, GtkContainer *parent)
{
  GtkWidget *gl_area;
  gl_view_state_t *state = NULL;

  if( config == NULL || view == NULL || parent == NULL )
    return( NULL );

  mem_new(&state);

  state->config = config;
  state->last_generation = (unsigned int)-1;
  state->fov_rad = glm_rad(60.0f);
  state->cached_camera_distance = 1.0f;

  gl_area = gtk_gl_area_new();
  state->gl_area = gl_area;

  gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);
  /* Frames are produced only on explicit request; unrelated exposes
   * present the cached frame instead of re-running the scene pass. */
  gtk_gl_area_set_auto_render(GTK_GL_AREA(gl_area), FALSE);

  gtk_widget_set_hexpand(gl_area, TRUE);
  gtk_widget_set_vexpand(gl_area, TRUE);

  g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), state);
  g_signal_connect(gl_area, "unrealize", G_CALLBACK(on_unrealize), state);
  gl_view_render_connect(state);
  g_signal_connect(gl_area, "resize", G_CALLBACK(on_resize), state);

  if( !render_surface_init(&state->base, gl_view_present_widget(state),
      &gl_engine, view, input) )
  {
    gtk_widget_destroy(gl_area);
    mem_free(&state);
    return( NULL );
  }

  gtk_container_add(parent, state->base.widget);

  return( &state->base );

} /* gl_view_surface_new() */

/*-----------------------------------------------------------------------*/

/** gl_view_queue_render() - Request a frame from a view's GtkGLArea
 * @state: view state whose area produces the frame
 *
 * The GL areas run with auto-render disabled, so a frame is produced only
 * on request.
 */
  void
gl_view_queue_render(gl_view_state_t *state)
{
  if( state == NULL )
    return;

  gtk_gl_area_queue_render(GTK_GL_AREA(state->gl_area));

} /* gl_view_queue_render() */

/*-----------------------------------------------------------------------*/

/**
 * gl_view_capture_pixbuf() - Capture the resolved OpenGL frame
 * @surface: surface holding the presented frame
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Reads the engine-owned single-sample resolve framebuffer because GTK's
 * default framebuffer has no stable format or sample-count contract.
 */
  GdkPixbuf *
gl_view_capture_pixbuf(render_surface_t *surface, int width, int height)
{
  gl_view_state_t *state = gl_view_state(surface);
  GdkPixbuf *pixbuf;
  GLint read_fbo;
  guchar *pixels = NULL;
  guchar *dst;
  size_t row_bytes;
  int row;

  if( state == NULL || width <= 0 || height <= 0 || state->resolve_fbo == 0 )
    return NULL;

  gtk_gl_area_make_current(GTK_GL_AREA(state->gl_area));
  if( gtk_gl_area_get_error(GTK_GL_AREA(state->gl_area)) != NULL )
    return NULL;

  pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
  if( pixbuf == NULL )
    return NULL;

  row_bytes = (size_t)width * 4;
  mem_alloc(&pixels, row_bytes * (size_t)height);

  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, state->resolve_fbo);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)read_fbo);

  dst = gdk_pixbuf_get_pixels(pixbuf);
  for( row = 0; row < height; row++ )
    memcpy(dst + (size_t)row * gdk_pixbuf_get_rowstride(pixbuf),
        pixels + (size_t)(height - row - 1) * row_bytes, row_bytes);

  mem_free(&pixels);
  return pixbuf;

} /* gl_view_capture_pixbuf() */

/*-----------------------------------------------------------------------*/

/** gl_view_build_mvp() - Compose model-view-projection for one frame
 * @state:       view engine state
 * @model_scale: uniform scale applied to the model; caller selects
 *               content vs overlay scale without mutating shared state
 * @mvp:         receives projection * view * model
 * @mv_dest:     receives view * model (no projection)
 *
 * Consumes view_R(view) for rotation, view->pan_offset converted from
 * screen pixels to world units via camera distance, fov and the view's
 * viewport height.  Projection planes come from state.
 */
  void
gl_view_build_mvp(gl_view_state_t *state, float model_scale,
                  mat4 mvp, mat4 mv_dest)
{
  mat4 view_mat, proj, model, trans;
  vec3 eye_pos, center_pos, up;
  view_t *view = state->base.view;
  float distance = state->cached_camera_distance;
  float aspect = (float)view->width / (float)view->height;
  float fov_rad = state->fov_rad;
  float near_plane = state->cached_near_plane;
  float far_plane = state->cached_far_plane;
  float pan_scale;
  float pan_x, pan_y;
  float (*R)[4] = view_R(view);

  glm_mat4_identity(model);
  glm_mat4_copy(R, model);
  glm_scale(model, (vec3){model_scale, model_scale, model_scale});

  /* Convert pan from screen pixels (as Cairo stores them) to world
   * units at the model plane.  Pan enters the MVP chain post-scale. */
  pan_scale = 2.0f * distance * tanf(fov_rad / 2.0f) /
              (float)view->height;
  pan_x = view->pan_offset[0] * pan_scale;
  pan_y = view->pan_offset[1] * pan_scale;

  glm_mat4_identity(trans);
  glm_translate(trans, (vec3){pan_x, pan_y, 0.0f});
  glm_mat4_mul(trans, model, model);

  glm_vec3_copy((vec3){0.0f, 0.0f, distance}, eye_pos);
  glm_vec3_zero(center_pos);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, up);

  glm_lookat(eye_pos, center_pos, up, view_mat);

  if( rc_config.opengl_orthographic )
  {
    float half_h = distance * tanf(fov_rad / 2.0f);
    glm_ortho(-half_h * aspect, half_h * aspect, -half_h, half_h,
        near_plane, far_plane, proj);
  }
  else
  {
    glm_perspective(fov_rad, aspect, near_plane, far_plane, proj);
  }

  glm_mat4_mul(view_mat, model, mv_dest);
  glm_mat4_mul(proj, view_mat, mvp);
  glm_mat4_mul(mvp, model, mvp);

} /* gl_view_build_mvp() */

#endif /* HAVE_OPENGL */
