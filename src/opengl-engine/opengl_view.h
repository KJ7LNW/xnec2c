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

#ifndef OPENGL_VIEW_H
#define OPENGL_VIEW_H 1

#include "common.h"

#ifdef HAVE_OPENGL
#include "opengl_renderer.h"
#include "opengl_gradient_overlay.h"
#include "../render/gradient_cache.h"
#include "../render/render_surface.h"

/* Maximum renderables per view (constrained by guint32 active_mask in render loop) */
#define MAX_RENDERABLES 32

/* Camera distance multiplier relative to scene r_max; chosen so the
 * default fit-to-window framing matches the Cairo renderer. */
#define GL_VIEW_BASE_DISTANCE_FACTOR 2.165f

/* Number of depth-peel passes for order-independent transparency.
 * 4 covers: near patch face, far patch face, near cylinder wall,
 * far cylinder wall — or structure front/back + radiation front/back. */
#define PEEL_PASSES 4

/* Texture unit used for the previous-pass depth texture during depth peeling */
#define PEEL_DEPTH_TEX_UNIT 2

/* Texture unit for the current layer's discovered depth (coplanar accumulation) */
#define LAYER_DEPTH_TEX_UNIT 3

/* glPolygonOffset parameters for surface-behind-wire depth ordering.
 * Factor=2.0 exceeds the peel epsilon dz coefficient (1.0), providing a
 * margin of dz+r that scales with surface slope and depth range. */
#define POLYGON_OFFSET_FACTOR 2.0f
#define POLYGON_OFFSET_UNITS  1.0f

/* Convert transparency fraction (0.0–1.0) to alpha (1.0=opaque, 0.0=invisible) */
#define TRANSPARENCY_TO_ALPHA(t) (1.0f - (t))

/** gl_batch_min_alpha() - Minimum alpha across a batch array
 * @batches: array of draw batches
 * @count: number of batches
 *
 * Used for opaque/transparent classification in depth peeling.
 */
static inline float
gl_batch_min_alpha(const gl_draw_batch_t *batches, int count)
{
  float min_alpha = 1.0f;
  int i;

  for( i = 0; i < count; i++ )
  {
    if( batches[i].alpha < min_alpha )
      min_alpha = batches[i].alpha;
  }

  return min_alpha;
}

/* Forward declaration so gl_view_input_ops_t can reference gl_view_state_t */
typedef struct gl_view_state_s gl_view_state_t;

/* Parent-prepared axes state consumed by the axes renderable. */
typedef struct
{
  gboolean active;
  float extent;

} gl_axes_content_t;

/* View content populated through the parent render protocol. */
typedef struct
{
  gl_draw_batch_t batches[GL_VIEW_MAX_BATCHES];
  int batch_count;
  int vertex_stride;
  float r_max;
  float clip_extent;
  float model_scale;
  unsigned int generation;
  gl_axes_content_t axes;

  /* Frame clear color resolved from the active theme */
  rgb_f_t background;

  /* 3D axis colors resolved from the active theme. */
  rgb_f_t view_axis;
  rgb_f_t view_axis_label;

  /* Centered text overlay rendered when no data to display; NULL = none */
  const char *status_message;

  /* Pre-resolved gradient legend from render(); surface NULL = skip.
   * GL upload compares version to avoid redundant texture transfers. */
  gradient_result_t gradient;

} gl_view_content_t;

/* Notice position within the GL viewport */
typedef enum
{
  GL_NOTICE_CENTER,
  GL_NOTICE_BOTTOM_LEFT

} gl_notice_position_t;

/* Overlay configuration for second rendering pass */
typedef struct
{
  const char *vertex_shader_path;
  const char *fragment_shader_path;
  const gl_vertex_attrib_t *attribs;
  int attrib_count;

} gl_overlay_config_t;

/* Per-frame render parameters passed to each renderable callback */
typedef struct gl_render_params_s
{
  mat4 mvp;
  mat4 mv;

  /* Carries frame-constant scene data into backend callbacks. */
  float r_max;
  rgb_f_t view_axis;
  rgb_f_t view_axis_label;

  float alpha;
  int peel_pass;

  /* When TRUE, fragment shader applies coplanar tolerance discard
   * against u_layer_depth instead of writing depth.  Set during
   * sub-pass B of coplanar accumulation. */
  int coplanar_pass;

  /* Animation phase (radians) for flow direction shaders */
  float flow_phase;

} gl_render_params_t;

/* Uniform locations for depth-peel discard logic (shared by all peel-aware shaders) */
typedef struct
{
  GLint peel_depth;
  GLint peel_pass;
  GLint layer_depth;
  GLint coplanar_pass;

} gl_peel_uniform_locs_t;

/* Renderable interface callback types */
typedef void (*gl_render_fn)(void *ctx, const gl_render_params_t *params);
typedef void (*gl_prepare_fn)(void *ctx, const gl_render_params_t *params);
typedef void (*gl_destroy_fn)(void *ctx);
typedef gboolean (*gl_active_fn)(void *ctx);
typedef float (*gl_extent_fn)(void *ctx, float r_max);

/* Called once per frame before far_extent; used for content generation
 * in renderables that must produce data before extent is known */
typedef void (*gl_generate_fn)(void *ctx);

/* Returns current effective alpha for opaque/transparent classification.
 * Called each frame so the value always reflects rc_config. */
typedef float (*gl_alpha_fn)(void *ctx);
typedef const gl_view_content_t *(*gl_content_fn)(void *ctx);

/* Unified renderable — all 3D objects implement this */
typedef struct
{
  gl_render_fn render;
  gl_prepare_fn prepare;
  gl_destroy_fn destroy;
  gl_active_fn is_active;
  gl_extent_fn far_extent;

  /* Returns model-space content for fit reduction; NULL when the
   * renderable carries no foldable geometry */
  gl_content_fn get_content;

  /* Called once per frame before far_extent; generates content for
   * renderables that must populate data before extent is known */
  gl_generate_fn generate;

  void *ctx;
  gl_alpha_fn get_alpha;
  vec3 origin;

  /* Sort priority for transparent pass (lower value renders first).
   * With depth peeling, per-pixel layering is handled by the peel
   * passes; sort_order controls renderable draw order within each
   * pass — the GL_ZERO dest blend factor means the nearest fragment
   * at each pixel wins, so order matters only at equal depth. */
  int transparent_sort_order;

  /* Whether alpha is reduced during drag interaction */
  gboolean transparent_on_drag;

  /* When TRUE, this renderable always routes through the depth-peel
   * path for coplanar accumulation, even at full opacity.  Set for
   * renderables whose batches contain coplanar geometry (polygon_offset). */
  gboolean force_peel;

} gl_renderable_t;

/* Pointer input operations the presenting domain supplies.  A NULL member
 * declines that modifier, which the generic handler then treats as an
 * unmodified event. */
typedef struct
{
  gboolean (*on_shift_scroll)(GdkEventScroll *event, gl_view_state_t *state);
  gboolean (*on_ctrl_scroll)(GdkEventScroll *event, gl_view_state_t *state);

  /* Notice advertising the ctrl+scroll capability, presented on the first
   * frame of the session that offers it */
  const char *ctrl_scroll_notice;

  /* Notice advertising the shift+scroll capability, presented on the first
   * frame of the session carrying the overlay geometry it scales */
  const char *shift_scroll_notice;

} gl_view_input_ops_t;

/* Static view configuration */
typedef struct
{
  const char *vertex_shader_path;
  const char *fragment_shader_path;
  const gl_vertex_attrib_t *attribs;
  int attrib_count;
  int vertex_stride;

  /* Pointer input operations of the presenting domain */
  const gl_view_input_ops_t *input;

  /* Second shader pass presenting overlay content; NULL when the view
   * presents primary content alone */
  const gl_overlay_config_t *overlay;

  /* Optional predicate controlling model ground-plane visibility */
  gl_active_fn ground_plane_is_active;

  /* Called via g_idle_add_once when GL context creation fails at realize time.
   * Implementations disable the OpenGL renderer and switch to Cairo fallback. */
  GSourceOnceFunc on_gl_init_failed;

  /* Releases the content caches the presenting domain owns; called as the
   * view state tears down */
  void (*content_cleanup)(void);

} gl_view_config_t;

/* View state (engine-internal): the GL engine's own drawing surface */
typedef struct gl_view_state_s
{
  /* The generic surface, which every engine extends through its first
   * member; it carries the presented widget and the borrowed view. */
  render_surface_t base;

  /* Inner GtkGLArea, resolved once at construction.  The base presents the
   * isolator wrapping it on X11 and this same area elsewhere. */
  GtkWidget *gl_area;

  GArray *renderables;
  gradient_overlay_t *overlay;
  gl_view_config_t *config;
  gl_view_content_t content;

  /* Secondary content owned by the overlay renderable; NULL while the view
   * presents primary content alone */
  gl_view_content_t *overlay_content;

  unsigned int last_generation;

  /* GL-only projection input; the viewport comes from the borrowed view */
  float fov_rad;

  /* Camera distance cached for pan pixel-to-world conversion */
  float cached_camera_distance;

  /* Cached clip planes from main render pass — shared by all renderables
   * so depth values are in the same projection space */
  float cached_near_plane;
  float cached_far_plane;

  /* Notice overlay state (unified transient + persistent) */
  gboolean notice_active;
  char *notice_text;
  double notice_alpha;
  gint64 notice_start_time;
  guint notice_timeout_id;
  int notice_hold_ms;
  gl_notice_position_t notice_position;
  cairo_gl_overlay_t *notice_overlay;
  cairo_surface_t *notice_surface;
  int notice_surf_width;
  int notice_surf_height;
  gboolean notice_surface_valid;

  /* LIC noise texture (256x256 grayscale, shared by all renderables) */
  GLuint noise_tex;

  /* Drag interaction state (transparency active during drag when on-click enabled) */
  gboolean drag_active;

  /* Per-frame: TRUE when per-type transparency is in effect.
   * Set in on_render before renderable iteration. */
  gboolean transparency_active;

  /* MSAA state */
  GLuint msaa_fbo;
  GLuint msaa_color_rbo;
  GLuint msaa_depth_rbo;
  int msaa_samples;
  int msaa_width;
  int msaa_height;
  GLuint resolve_fbo;        /* single-sample color-only resolve target */
  GLuint resolve_color_tex;  /* GL_RGBA8 texture sampled by the final copy */

  /* Depth-peel transparency state */
  GLuint peel_fbo[2];          /* ping-pong FBOs for depth peeling (single-sample resolve targets) */
  GLuint peel_depth_tex[2];    /* depth textures attached to peel FBOs */
  GLuint peel_color_tex;       /* shared color texture for current layer */
  GLuint layer_depth_tex;      /* discovered layer depth for coplanar accumulation */
  GLuint layer_depth_fbo;      /* FBO for blitting peel depth → layer_depth_tex */
  GLuint accum_fbo;            /* accumulation FBO (under-operator) */
  GLuint accum_color_tex;      /* RGBA accumulation texture */

  /* Multisampled peel FBOs — used for rasterization when MSAA is active,
   * then blit-resolved to single-sample peel_fbo[] for shader reads */
  GLuint peel_ms_fbo[2];      /* multisampled ping-pong FBOs */
  GLuint peel_ms_color_rbo[2]; /* MS color renderbuffers */
  GLuint peel_ms_depth_rbo[2]; /* MS depth renderbuffers */
  GLuint composite_program;    /* fullscreen quad shader program */
  GLuint composite_vs;         /* composite vertex shader */
  GLuint composite_fs;         /* composite fragment shader */
  GLuint composite_vao;        /* fullscreen triangle VAO */
  GLuint composite_vbo;        /* fullscreen triangle VBO (3 vertices) */
  GLint  composite_u_layer;    /* sampler uniform: current peel layer */
  int peel_width;              /* current peel texture dimensions */
  int peel_height;
  int peel_msaa_samples;       /* MSAA sample count at last peel creation */

  gboolean initialized;

} gl_view_state_t;

/** gl_view_state() - Reach the GL view state from its surface base
 * @surface: surface a GL engine produces frames for
 */
static inline gl_view_state_t *
gl_view_state(render_surface_t *surface)
{
  return( (gl_view_state_t *)surface );
}

/** gl_view_init_empty() - Initialize content for an empty scene
 * @surface: GL surface receiving the empty scene
 */
static inline void
gl_view_init_empty(render_surface_t *surface)
{
  gl_view_content_t *out = &gl_view_state(surface)->content;

  out->batch_count = 0;
  out->r_max = 1.5f;
  out->clip_extent = 1.5f;
  out->model_scale = 1.0f;
}

/** gl_view_set_status() - Set the status message on content
 * @surface: GL surface holding the frame content
 * @msg: STATUS_MSG_* string constant, or NULL
 */
static inline void
gl_view_set_status(render_surface_t *surface, const char *msg)
{
  gl_view_state(surface)->content.status_message = msg;
}

/** gl_view_set_gradient() - Store pre-resolved gradient legend result
 * @surface: GL surface holding the frame content
 * @result:  gradient legend result from gradient_cache
 */
static inline void
gl_view_set_gradient(render_surface_t *surface,
    const gradient_result_t *result)
{
  gl_view_state(surface)->content.gradient = *result;
}

/* Sorting entry for the transparent render pass */
typedef struct
{
  int index;
  int sort_order;
  float alpha;
  float depth;

} gl_trans_item_t;

/* Public API */

/** gl_view_surface_new() - Build a GL surface and pack it into a container
 * @config: view configuration
 * @view:   per-view rotation/pan/zoom/drag owner (borrowed, non-NULL)
 * @parent: container the presented widget joins
 *
 * Returns a surface the caller hands to a canvas, which owns it from then on.
 */
render_surface_t *gl_view_surface_new(gl_view_config_t *config, view_t *view,
    GtkContainer *parent);

/** gl_view_surface_free() - Release the view state a canvas held */
void gl_view_surface_free(render_surface_t *surface);

/** gl_view_gpu_release() - Release every GPU resource the context owns
 * @state: view state whose GL context is going away
 *
 * Runs at unrealize with the context current, and leaves the state ready to
 * rebuild its resources should the widget realize again.
 */
void gl_view_gpu_release(gl_view_state_t *state);

/**
 * gl_view_capture_pixbuf() - Capture the resolved OpenGL frame
 * @surface: surface holding the presented frame
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated top-down RGBA pixbuf, or NULL on failure.
 */
GdkPixbuf *gl_view_capture_pixbuf(render_surface_t *surface,
    int width, int height);

/* gl_view_queue_render() - Request a frame from a view's GtkGLArea.
 * The GL areas run with auto-render disabled, so a frame is produced only
 * on request. */
void gl_view_queue_render(gl_view_state_t *state);

/* gl_view_build_mvp() - Compose model/view/projection matrix for a frame.
 *
 * Reads rotation from view_R(state->base.view), pan from view->pan_offset
 * (converted from screen pixels to world units via camera distance, fov
 * and the view's viewport height), projection from the view's aspect and
 * state->fov_rad plus cached_near_plane / cached_far_plane.  Model scale is
 * passed explicitly so the overlay pass can select a different scale from
 * the main-content scale without temporarily mutating shared state. */
void gl_view_build_mvp(gl_view_state_t *state, float model_scale,
                       mat4 mvp, mat4 mv);
/* gl_view_setup_attribs()
 *
 * Configure vertex attribute pointers in VAO. Called once during prepare
 * when VBO data changes. VAO retains this state for subsequent renders.
 */
void gl_view_setup_attribs(
    GLuint vao,
    GLuint vbo,
    const gl_vertex_attrib_t *attribs,
    const GLint *attrib_locations,
    int attrib_count,
    int vertex_stride);


#endif /* HAVE_OPENGL */
#endif /* OPENGL_VIEW_H */
