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

#include "opengl_structure.h"
#include "opengl_rdpattern.h"
#include "../shared.h"
#include "../gdk_scroll.h"

#ifdef HAVE_OPENGL

#include "opengl_structure_geometry.h"
#include "opengl_state.h"
#include "../settings/render_settings.h"
#include "../opengl-engine/opengl_view.h"
#include "../themes/theme.h"
#include "../opengl-engine/opengl_view_notice.h"
#include "../render/render_dispatch.h"
#include "../rdpattern_ui.h"
#include "../structure_ui.h"

/* Maximum allowed radius scale */
#define CYLINDER_RADIUS_SCALE_MAX 100.0

/* Mutable cylinder radius scale factor (user-adjustable via Ctrl+scroll) */
static double cylinder_radius_scale = 1.0;


/* Widget pointer for queue_redraw */
static GtkWidget *structure_gl_widget = NULL;

/* Vertex attribute layout for lit-color shader (shared with overlay consumers) */
const gl_vertex_attrib_t opengl_structure_attribs[3] = {
  { "position", 3, 0 },
  { "normal",   3, 4 * (int)sizeof(float) },
  { "color",    4, 8 * (int)sizeof(float) }
};

/* Vertex attribute layout for chevron shader (structure_vertex_t) */
const gl_vertex_attrib_t opengl_chevron_attribs[7] = {
  { "position",   3, 0 },
  { "normal",     3, 4 * (int)sizeof(float) },
  { "color",      4, 8 * (int)sizeof(float) },
  { "uv",         2, 12 * (int)sizeof(float) },
  { "flow_data",  4, 14 * (int)sizeof(float) },
  { "tangent1",   3, 18 * (int)sizeof(float) },
  { "tangent2",   3, 21 * (int)sizeof(float) }
};

/*-----------------------------------------------------------------------*/

/** opengl_structure_get_radius_scale() - Return current cylinder radius display scale factor
 */
  double
opengl_structure_get_radius_scale(void)
{
  return( cylinder_radius_scale );

} /* opengl_structure_get_radius_scale() */

/*-----------------------------------------------------------------------*/

/** opengl_structure_set_radius_scale() - Set cylinder radius display scale factor
 * @scale: new scale value, clamped to [0, CYLINDER_RADIUS_SCALE_MAX]
 *
 * Syncs to rc_config; geometry regenerates on next render.
 */
  void
opengl_structure_set_radius_scale(double scale)
{
  if( scale < 0.0 )
  {
    scale = 0.0;
  }

  if( scale > CYLINDER_RADIUS_SCALE_MAX )
  {
    scale = CYLINDER_RADIUS_SCALE_MAX;
  }

  cylinder_radius_scale = scale;
  rc_config.opengl_cylinder_radius_scale = scale;

} /* opengl_structure_set_radius_scale() */

/*-----------------------------------------------------------------------*/

/** opengl_structure_on_ctrl_scroll() - Ctrl+scroll handler for adjusting cylinder radius scale
 * @widget: source scroll widget
 * @event: scroll event
 * @view_state: pointer to gl_view_state_t
 *
 * Shared by structure and rdpattern scene providers.
 */
  gboolean
opengl_structure_on_ctrl_scroll(
    GtkWidget *widget, GdkEventScroll *event, gpointer view_state)
{
  gl_view_state_t *state;
  double scale, new_scale;

  scroll_step_t s;

  state = (gl_view_state_t *)view_state;

  if( !state )
    return( FALSE );

  s = scroll_step_from_deltas((GdkEvent *)event);

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  /* Compute increment matching zoom scroll feel */
  scale = compute_zoom_scale(
      (int)(state->viewport_height * state->aspect),
      (int)state->viewport_height,
      (cylinder_radius_scale > 0.1 ? cylinder_radius_scale : 0.1) * 100.0);

  new_scale = cylinder_radius_scale;

  if( s.direction == GDK_SCROLL_UP )
  {
    /* Scroll up: thicker. If at zero, jump to threshold. */
    if( new_scale < CYLINDER_SCALE_LINE_THRESHOLD )
    {
      new_scale = CYLINDER_SCALE_LINE_THRESHOLD;
    }
    else
    {
      new_scale *= (1.0 + 0.1 * s.step * scale);
    }
  }
  else if( s.direction == GDK_SCROLL_DOWN )
  {
    /* Scroll down: thinner. Below threshold snaps to zero (line mode). */
    new_scale /= (1.0 + 0.1 * s.step * scale);

    if( new_scale < CYLINDER_SCALE_LINE_THRESHOLD )
    {
      new_scale = 0.0;
    }
  }

  opengl_structure_set_radius_scale(new_scale);
  render_settings_sync_from_config();

  /* Queue redraw on the event source widget */
  xnec2_widget_queue_draw(widget, TRUE);

  /* Queue redraw on cross-window GL areas */
  if( structure_gl_widget && widget != structure_gl_widget )
    xnec2_widget_queue_draw(structure_gl_widget, TRUE);

  {
    GtkWidget *rdpat_w = opengl_rdpattern_get_widget();
    if( rdpat_w && widget != rdpat_w )
      xnec2_widget_queue_draw(rdpat_w, TRUE);
  }

  return( TRUE );

} /* opengl_structure_on_ctrl_scroll() */

/*-----------------------------------------------------------------------*/

/** opengl_structure_show_ctrl_notice() - Show ctrl+scroll notice once per session
 * @widget: GL area widget to display notice on
 *
 * Shared guard across all views so the message appears only once
 * regardless of which view renders first.
 */
  void
opengl_structure_show_ctrl_notice(GtkWidget *widget)
{
  static gboolean shown = FALSE;

  if( shown || !widget )
    return;

  gl_view_show_notice(widget, "Ctrl+Scroll: Wire Radius",
      2500, GL_NOTICE_BOTTOM_LEFT);
  shown = TRUE;

} /* opengl_structure_show_ctrl_notice() */

/*-----------------------------------------------------------------------*/

/** gl_draw_structure() - Structure leaf: update shared geometry and populate batches
 * @ctx:    gl_view_state_s (passed as void* through render_ops_t)
 * @extent: content half-extent for projection scaling
 * @params: dispatch-resolved draw parameters (precomputed colors)
 *
 * Always returns TRUE; sets status_message when no geometry is loaded.
 */
  gboolean
gl_draw_structure(void *ctx, float extent, const struct_draw_params_t *params)
{
  gl_view_state_t *state = (gl_view_state_t *)ctx;
  gl_view_content_t *out = &state->content;
  const structure_overlay_data_t *geom;

  opengl_structure_update_shared_geometry_with_params(params);
  geom = opengl_structure_get_shared_geometry();

  memcpy(out->batches, geom->batches,
      (size_t)geom->batch_count * sizeof(geom->batches[0]));
  out->batch_count = geom->batch_count;
  out->vertex_stride = geom->vertex_stride;
  out->r_max = (geom->batch_count > 0) ? extent : 1.5f;
  out->clip_extent = out->r_max;
  out->model_scale = 1.0f;
  out->generation = geom->generation;

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/* gl_ops defined in opengl_ops.c; declared in opengl_structure.h */

/*-----------------------------------------------------------------------*/

/** structure_scene_generate() - Scene provider generate callback
 * @state: view state; scene writes into state->content
 *
 * Delegates to unified render() dispatch with state as ctx.
 */
  static gboolean
structure_scene_generate(gl_view_state_t *state)
{
  const theme_t *active_theme = theme_active();

  state->content.status_message = NULL;
  state->content.gradient = (gradient_result_t){NULL, 0};
  state->content.background = active_theme->colors[THEME_ROLE_BACKGROUND];
  state->content.view_axis = active_theme->colors[THEME_ROLE_VIEW_AXIS];
  state->content.view_axis_label =
      active_theme->colors[THEME_ROLE_VIEW_AXIS_LABEL];

  opengl_structure_show_ctrl_notice(structure_gl_widget);

  return render((void *)state, &gl_ops, structure_view);
}

/*-----------------------------------------------------------------------*/

/** structure_scene_cleanup() - Scene provider cleanup callback
 */
  static void
structure_scene_cleanup(void)
{
  opengl_structure_geometry_cleanup();
}

/*-----------------------------------------------------------------------*/

/** structure_scene_post_render() - Scene provider post-render callback */
  static void
structure_scene_post_render(void)
{
}

/*-----------------------------------------------------------------------*/

/* Static scene configuration and provider */
static gl_view_config_t structure_view_config = {
  .vertex_shader_path = "/gl/lit-color-vertex.glsl",
  .fragment_shader_path = "/gl/lit-color-fragment.glsl",
  .attribs = opengl_chevron_attribs,
  .attrib_count = 7,
  .vertex_stride = (int)sizeof(structure_vertex_t),
  .on_gl_init_failed = opengl_gl_init_failed
};

/** opengl_structure_ground_plane_is_active() - Report whether ground plane renderable is visible
 * @_ctx: unused context pointer
 *
 * Delegates to gnd_has_real_ground() — parse-time immutable predicate.
 */
  static gboolean
opengl_structure_ground_plane_is_active(void *_ctx)
{
  (void)_ctx;

  return( gnd_has_real_ground() );

} /* opengl_structure_ground_plane_is_active() */

/*-----------------------------------------------------------------------*/

static gl_scene_provider_t structure_scene_provider = {
  .generate                 = structure_scene_generate,
  .post_render              = structure_scene_post_render,
  .cleanup                  = structure_scene_cleanup,
  .overlay_config           = NULL,
  .overlay_generate         = NULL,
  .overlay_cleanup          = NULL,
  .on_ctrl_scroll           = opengl_structure_on_ctrl_scroll,
  .ground_plane_is_active   = opengl_structure_ground_plane_is_active
};

/*-----------------------------------------------------------------------*/

/** opengl_structure_create_widget_impl() - Create the OpenGL structure widget using the generic view engine
 *
 * Returns existing widget if already created.
 */
  static GtkWidget*
opengl_structure_create_widget_impl(void)
{
  /* Return existing widget if already created */
  if( structure_gl_widget )
    return( structure_gl_widget );

  /* Load persisted radius scale from config; zero means line mode */
  cylinder_radius_scale = rc_config.opengl_cylinder_radius_scale;

  if( structure_view == NULL )
    return( NULL );

  structure_gl_widget = gl_view_create_widget(
      &structure_view_config,
      &structure_scene_provider,
      structure_view );

  return( structure_gl_widget );
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_get_widget() - Return the structure GL widget
 */
  GtkWidget*
opengl_structure_get_widget(void)
{
  return( structure_gl_widget );
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_cleanup_impl() - Cleanup structure GL resources
 */
  static void
opengl_structure_cleanup_impl(void)
{
  structure_scene_cleanup();
  structure_gl_widget = NULL;
}

#endif /* HAVE_OPENGL */

/*-----------------------------------------------------------------------*/

/** opengl_structure_create_widget() - Public API: create structure GL widget
 */
  GtkWidget*
opengl_structure_create_widget(void)
{
#ifdef HAVE_OPENGL
  return( opengl_structure_create_widget_impl() );
#else
  return( NULL );
#endif
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_cleanup() - Public API: cleanup structure resources
 */
  void
opengl_structure_cleanup(void)
{
#ifdef HAVE_OPENGL
  opengl_structure_cleanup_impl();
#endif
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_invalidate() - Public API: mark cached geometry stale
 *
 * Forces regeneration from current NEC2 data arrays on next render.
 * Call after geometry reload.
 */
  void
opengl_structure_invalidate(void)
{
#ifdef HAVE_OPENGL
  opengl_structure_geometry_invalidate();
#endif
}

/*-----------------------------------------------------------------------*/
