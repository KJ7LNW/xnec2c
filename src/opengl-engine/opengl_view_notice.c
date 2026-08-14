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
#include "opengl_view_notice.h"
#include "opengl_cairo_overlay.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** render_notice_text() - Create a Cairo surface with positioned text over a semi-transparent background box
 * @text: text to render
 * @width: surface width in pixels
 * @height: surface height in pixels
 * @position: GL_NOTICE_CENTER or GL_NOTICE_BOTTOM_LEFT
 *
 * Returns newly-created surface; caller owns it.
 * All content rendered at full opacity; callers apply fade via GL blend.
 */
  static cairo_surface_t*
render_notice_text(const char *text, int width, int height,
    gl_notice_position_t position)
{
  cairo_surface_t *surface;
  cairo_t *cr;
  cairo_text_extents_t extents;
  double x = 0.0, y = 0.0, padding;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cr = cairo_create(surface);

  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 16.0);

  cairo_text_extents(cr, text, &extents);

  padding = 12.0;

  /* Position text box within surface */
  switch( position )
  {
    case GL_NOTICE_BOTTOM_LEFT:
      /* Anchor with padding from edges.
       * Cairo Y increases downward; bottom = height. */
      x = padding - extents.x_bearing;
      y = height - padding - extents.height - extents.y_bearing;
      break;

    case GL_NOTICE_CENTER:
      x = (width - extents.width) / 2.0 - extents.x_bearing;
      y = (height - extents.height) / 2.0 - extents.y_bearing;
      break;

    default:
      g_error("render_notice_text: unhandled position %d", position);
      break;
  }

  /* Background box */
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.7);
  cairo_rectangle(cr,
      x + extents.x_bearing - padding,
      y + extents.y_bearing - padding,
      extents.width + 2.0 * padding,
      extents.height + 2.0 * padding);
  cairo_fill(cr);

  /* Text */
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, text);

  cairo_destroy(cr);

  return( surface );

} /* render_notice_text() */

/*-----------------------------------------------------------------------*/

/** gl_view_render_notice_surface() - Regenerate the cached Cairo surface with notice text
 * @state: view state
 * @surf_width: surface width in pixels
 * @surf_height: surface height in pixels
 *
 * Called when text or dimensions change (notice_surface_valid == FALSE).
 */
  static void
gl_view_render_notice_surface(gl_view_state_t *state, int surf_width, int surf_height)
{
  /* Release stale surface */
  if( state->notice_surface )
  {
    cairo_surface_destroy(state->notice_surface);
    state->notice_surface = NULL;
  }

  state->notice_surface = render_notice_text(
      state->notice_text, surf_width, surf_height,
      state->notice_position);
  state->notice_surf_width = surf_width;
  state->notice_surf_height = surf_height;
  state->notice_surface_valid = TRUE;

} /* gl_view_render_notice_surface() */

/*-----------------------------------------------------------------------*/

/** gl_view_render_notice() - Render notice overlay with optional fade animation
 * @state: view state
 * @surf_width: surface width in pixels
 * @surf_height: surface height in pixels
 *
 * Regenerates cached surface only when text or dimensions change.
 */
  void
gl_view_render_notice(gl_view_state_t *state, int surf_width, int surf_height)
{
  gboolean needs_upload = FALSE;

  /* Regenerate surface if invalidated or dimensions changed */
  if( !state->notice_surface_valid ||
      state->notice_surf_width != surf_width ||
      state->notice_surf_height != surf_height )
  {
    gl_view_render_notice_surface(state, surf_width, surf_height);
    needs_upload = TRUE;
  }

  /* Lazily allocate cached notice overlay */
  if( !state->notice_overlay )
    state->notice_overlay = cairo_gl_overlay_new();

  if( state->notice_overlay && state->notice_surface )
  {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    /* Modulate alpha for fade: scale source by constant alpha */
    glBlendColor(0.0f, 0.0f, 0.0f, (float)state->notice_alpha);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cairo_gl_overlay_set_size(state->notice_overlay, surf_width, surf_height);

    if( needs_upload )
      cairo_gl_overlay_upload(state->notice_overlay, state->notice_surface);

    cairo_gl_overlay_render(state->notice_overlay);

    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

} /* gl_view_render_notice() */

/*-----------------------------------------------------------------------*/

/** notice_update_callback() - Timer callback for notice fade animation
 * @user_data: view state the timer was armed with
 *
 * duration_ms == 0: persistent, no fade — holds at alpha 1.0.
 * duration_ms > 0: holds for notice_hold_ms, then fades over 500ms.
 */
  static gboolean
notice_update_callback(gpointer user_data)
{
  gl_view_state_t *state;
  gint64 current_time, elapsed_ms;
  double progress;

  state = (gl_view_state_t *)user_data;

  if( state == NULL )
    return( FALSE );

  if( !state->notice_active )
  {
    /* Returning FALSE removes the source, so drop the id the state holds. */
    state->notice_timeout_id = 0;
    return( FALSE );
  }

  current_time = g_get_monotonic_time();
  elapsed_ms = (current_time - state->notice_start_time) / 1000;

  /* Stay visible for configured hold duration */
  if( elapsed_ms < state->notice_hold_ms )
  {
    state->notice_alpha = 1.0;
    gl_view_queue_render(state);
    return( TRUE );
  }

  /* Fade over 500ms after hold period */
  progress = (double)(elapsed_ms - state->notice_hold_ms) / 500.0;

  if( progress >= 1.0 )
  {
    state->notice_active = FALSE;
    state->notice_alpha = 0.0;
    state->notice_timeout_id = 0;
    gl_view_queue_render(state);
    return( FALSE );
  }

  /* Linear fade from 1.0 to 0.0 */
  state->notice_alpha = 1.0 - progress;

  gl_view_queue_render(state);

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/** gl_view_show_notice() - Display a notice at the given position
 * @state: view state presenting the notice
 * @text: notice text
 * @duration_ms: hold duration before fade; 0 = persistent (no fade)
 * @position: GL_NOTICE_CENTER or GL_NOTICE_BOTTOM_LEFT
 */
  void
gl_view_show_notice(gl_view_state_t *state, const char *text,
    int duration_ms, gl_notice_position_t position)
{
  if( state == NULL || text == NULL )
    return;

  /* Remove existing notice timer */
  if( state->notice_timeout_id )
  {
    g_source_remove(state->notice_timeout_id);
    state->notice_timeout_id = 0;
  }

  g_free(state->notice_text);
  state->notice_text = g_strdup(text);
  state->notice_active = TRUE;
  state->notice_alpha = 1.0;
  state->notice_surface_valid = FALSE;
  state->notice_hold_ms = duration_ms;
  state->notice_position = position;
  state->notice_start_time = g_get_monotonic_time();

  /* Transient notices use a 16ms timer for smooth fade animation.
   * Persistent notices (duration_ms == 0) need no timer; they
   * remain at alpha 1.0 until explicitly hidden or replaced. */
  if( duration_ms > 0 )
    state->notice_timeout_id = g_timeout_add(16, notice_update_callback, state);

  gl_view_queue_render(state);

} /* gl_view_show_notice() */

/*-----------------------------------------------------------------------*/

/** gl_view_hide_notice() - Deactivate the current notice and cancel any fade timer
 * @state: view state whose notice ends
 */
  void
gl_view_hide_notice(gl_view_state_t *state)
{
  if( state == NULL )
    return;

  if( state->notice_timeout_id )
  {
    g_source_remove(state->notice_timeout_id);
    state->notice_timeout_id = 0;
  }

  state->notice_active = FALSE;
  state->notice_alpha = 0.0;

  gl_view_queue_render(state);

} /* gl_view_hide_notice() */

/*-----------------------------------------------------------------------*/

/** gl_view_sync_status_notice() - Synchronize notice with the frame status_message
 * @state: view state carrying the frame content
 *
 * Persistent notices track status_message changes.  Transient notices
 * (notice_hold_ms > 0) are not interrupted; the status message takes
 * over after the transient notice fades.
 */
  void
gl_view_sync_status_notice(gl_view_state_t *state)
{
  if( state->content.status_message )
  {
    /* Show or update persistent notice when no transient is active */
    if( !state->notice_active ||
        (state->notice_hold_ms == 0 &&
         g_strcmp0(state->notice_text, state->content.status_message) != 0) )
    {
      gl_view_show_notice(state, state->content.status_message,
          0, GL_NOTICE_CENTER);
    }
  }
  else if( state->notice_active && state->notice_hold_ms == 0 )
  {
    /* Status message cleared; deactivate persistent notice */
    gl_view_hide_notice(state);
  }

} /* gl_view_sync_status_notice() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
