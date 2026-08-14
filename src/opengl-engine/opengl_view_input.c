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

#include "opengl_view_input.h"
#include "../shared.h"
#include "../gdk_scroll.h"

#ifdef HAVE_OPENGL

/* Forward declarations */
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static gboolean on_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);

/*-----------------------------------------------------------------------*/

/** on_button_press() - Mouse button press handler
 * @_widget: signal source, unread: the state carries the area and the view
 * @event: button press event
 * @user_data: view state
 */
  static gboolean
on_button_press(GtkWidget *_widget, GdkEventButton *event, gpointer user_data)
{
  gl_view_state_t *state;

  state = (gl_view_state_t *)user_data;

  if( state == NULL || state->base.view == NULL )
    return( FALSE );

  if( event->button == 1 || event->button == 2 )
  {
    drag_button_t button = (event->button == 1) ? VIEW_DRAG_ROTATE : VIEW_DRAG_PAN;

    view_begin_drag(state->base.view, button, (float)event->x, (float)event->y);

    state->drag_active = TRUE;
    gl_view_queue_render(state);

    return( TRUE );
  }

  return( FALSE );

} /* on_button_press() */

/*-----------------------------------------------------------------------*/

/** on_button_release() - Mouse button release handler
 * @_widget: signal source, unread: the state carries the area and the view
 * @event: button release event
 * @user_data: view state
 */
  static gboolean
on_button_release(GtkWidget *_widget, GdkEventButton *event, gpointer user_data)
{
  gl_view_state_t *state;

  state = (gl_view_state_t *)user_data;

  if( state == NULL || state->base.view == NULL )
    return( FALSE );

  view_end_drag(state->base.view);

  state->drag_active = FALSE;
  gl_view_queue_render(state);

  return( TRUE );

} /* on_button_release() */

/*-----------------------------------------------------------------------*/

/** on_motion() - Mouse motion handler
 * @_widget: signal source, unread: the state carries the area and the view
 * @event: motion event
 * @user_data: view state
 *
 * Delegates rotation and pan accumulation to view_update_drag().
 * Pixel-to-world conversion for pan happens in gl_view_build_mvp()
 * at draw time so the per-view pan_offset stores screen pixels and
 * matches the Cairo renderer.
 */
  static gboolean
on_motion(GtkWidget *_widget, GdkEventMotion *event, gpointer user_data)
{
  gl_view_state_t *state;

  state = (gl_view_state_t *)user_data;

  if( !state || !state->base.view )
    return( FALSE );

  if( state->base.view->drag_button == VIEW_DRAG_NONE )
    return( FALSE );

  view_update_drag(state->base.view, (float)event->x, (float)event->y);
  gl_view_queue_render(state);

  return( TRUE );

} /* on_motion() */

/*-----------------------------------------------------------------------*/

/** on_scroll() - Mouse scroll handler
 * @_widget: signal source, unread: the state carries the area and the view
 * @event: scroll event
 * @user_data: view state
 *
 * Modified scroll invokes the configured input operation of the presenting
 * domain. Normal scroll adjusts primary zoom via spinbutton.
 */
  static gboolean
on_scroll(GtkWidget *_widget, GdkEventScroll *event, gpointer user_data)
{
  gl_view_state_t *state;
  const gl_view_input_ops_t *input;
  const view_t *view;
  GtkSpinButton *spinbutton;
  double value, scale, zoom_percent;

  scroll_step_t s;

  state = (gl_view_state_t *)user_data;

  if( state == NULL || state->base.view == NULL )
    return( FALSE );

  view = state->base.view;

  s = scroll_step_from_deltas((GdkEvent *)event);

  input = state->config->input;

  /* Ctrl+scroll: invoke segment radius scaling handler */
  if( input != NULL && (event->state & GDK_CONTROL_MASK) &&
      input->on_ctrl_scroll )
  {
    return( input->on_ctrl_scroll(event, state) );
  }

  /* Shift+scroll: invoke the domain's shift handler */
  if( input != NULL && (event->state & GDK_SHIFT_MASK) &&
      input->on_shift_scroll )
  {
    return( input->on_shift_scroll(event, state) );
  }

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  /* Normal scroll: adjust primary zoom via the view's bound spin */
  if( !view->zoom_spin )
    return( FALSE );

  spinbutton = view->zoom_spin;
  value = gtk_spin_button_get_value(spinbutton);

  zoom_percent = value;

  scale = compute_zoom_scale(view->width, view->height, zoom_percent);

  if( s.direction == GDK_SCROLL_UP )
    value *= (1.0 + 0.1 * s.step * scale);
  else if( s.direction == GDK_SCROLL_DOWN )
    value /= (1.0 + 0.1 * s.step * scale);

  gtk_spin_button_set_value(spinbutton, value);

  return( TRUE );

} /* on_scroll() */

/*-----------------------------------------------------------------------*/

/** gl_view_input_connect() - Wire input signal handlers to a view's GL area
 * @state: view state whose area receives the pointer and scroll events
 */
  void
gl_view_input_connect(gl_view_state_t *state)
{
  gtk_widget_add_events(state->gl_area,
    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
    GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

  g_signal_connect(state->gl_area, "button-press-event",
    G_CALLBACK(on_button_press), state);
  g_signal_connect(state->gl_area, "button-release-event",
    G_CALLBACK(on_button_release), state);
  g_signal_connect(state->gl_area, "motion-notify-event",
    G_CALLBACK(on_motion), state);
  g_signal_connect(state->gl_area, "scroll-event",
    G_CALLBACK(on_scroll), state);

} /* gl_view_input_connect() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
