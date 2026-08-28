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

/*
 * surface_input: pointer, scroll and allocation behavior of every drawing
 * surface, held once.
 *
 * The surface travels as the signal user data, so each handler reaches the
 * view it mutates, the modifier operations of its domain, and the engine
 * producing its frames without naming any of them.  Engine identity reaches
 * behavior through that binding alone.
 */

#include "render_surface_input.h"
#include "render_surface.h"
#include "render_canvas_surface.h"
#include "../gdk_scroll.h"
#include "../shared.h"

/* GDK modifier bit carried by each member of the modifier axis, so dispatch
 * reads an ordinal rather than naming a modifier */
static const guint modifier_masks[SURFACE_MOD_COUNT] = {
  [SURFACE_MOD_CTRL]  = GDK_CONTROL_MASK,
  [SURFACE_MOD_SHIFT] = GDK_SHIFT_MASK
};

/**
 * on_button_press() - Begin a drag session from a pointer button
 * @_widget: signal source, unread: the surface carries the widget
 * @event: button press event
 * @user_data: surface presenting the pressed widget
 *
 * Button 1 rotates and button 2 pans; every other button is left to the
 * default handlers.
 */
  static gboolean
on_button_press(GtkWidget *_widget, GdkEventButton *event, gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;
  view_t *view = surface->view;
  drag_button_t button;

  if( event->button != 1 && event->button != 2 )
    return( FALSE );

  button = (event->button == 1) ? VIEW_DRAG_ROTATE : VIEW_DRAG_PAN;

  view_begin_drag(view, button, (float)event->x, (float)event->y);
  canvas_surface_queue_redraw(surface);

  return( TRUE );

} /* on_button_press() */

/*-----------------------------------------------------------------------*/

/**
 * on_button_release() - End the drag session a button press opened
 * @_widget: signal source, unread: the surface carries the widget
 * @event: button release event
 * @user_data: surface presenting the released widget
 */
  static gboolean
on_button_release(GtkWidget *_widget, GdkEventButton *event,
    gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;
  view_t *view = surface->view;

  if( event->button != 1 && event->button != 2 )
    return( FALSE );

  if( view->drag_button == VIEW_DRAG_NONE )
    return( FALSE );

  view_end_drag(view);
  canvas_surface_queue_redraw(surface);

  return( TRUE );

} /* on_button_release() */

/*-----------------------------------------------------------------------*/

/**
 * on_motion() - Accumulate rotation or pan from pointer movement
 * @_widget: signal source, unread: the surface carries the widget
 * @event: motion event
 * @user_data: surface presenting the widget the pointer moved over
 *
 * The drag button records the session, so movement outside one is ignored.
 * Pixel-to-world conversion for pan happens at draw time, which leaves
 * pan_offset in screen pixels for every engine.
 */
  static gboolean
on_motion(GtkWidget *_widget, GdkEventMotion *event, gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;
  view_t *view = surface->view;

  if( view->drag_button == VIEW_DRAG_NONE )
    return( FALSE );

  view_update_drag(view, (float)event->x, (float)event->y);
  canvas_surface_queue_redraw(surface);

  return( TRUE );

} /* on_motion() */

/*-----------------------------------------------------------------------*/

/**
 * scroll_capability() - Select the capability a modifier state requests
 * @input: modifier capabilities of the scrolled surface, or NULL when none
 * @state: modifier mask carried by the scroll event
 *
 * Walks the axis in ordinal order and answers the first capability the row
 * offers for a modifier the event carries, so a chord holding both requests
 * the ctrl capability and falls to shift where the row declines ctrl.
 * Returns NULL when the surface declines the modifier, leaving the event to
 * the zoom path.
 */
  static const surface_capability_t *
scroll_capability(const surface_input_ops_t *input, guint state)
{
  const surface_capability_t *cap = NULL;
  surface_modifier_t m;

  if( input == NULL )
    return( NULL );

  for( m = 0; m < SURFACE_MOD_COUNT && cap == NULL; m++ )
  {
    if( (state & modifier_masks[m]) == 0 )
      continue;

    cap = input->by_modifier[m];
  }

  return( cap );

} /* scroll_capability() */

/*-----------------------------------------------------------------------*/

/**
 * surface_notice_capabilities() - Advertise capabilities acting on a subject
 * @surface: surface whose engine has resolved the subject
 * @subject: subject the calling engine brought into a reportable state
 *
 * The guard lives on the capability, so rows borrowing one capability
 * advertise it once between them.  An engine declining the notice leaves the
 * guard down, which offers the capability again on a later frame.
 */
  void
surface_notice_capabilities(render_surface_t *surface,
    surface_cap_subject_t subject)
{
  surface_capability_t *cap;
  surface_modifier_t m;

  if( surface == NULL || surface->input == NULL )
    return;

  for( m = 0; m < SURFACE_MOD_COUNT; m++ )
  {
    cap = surface->input->by_modifier[m];

    if( cap == NULL || cap->notice_shown || cap->subject != subject )
      continue;

    cap->notice_shown =
      canvas_surface_notice_capability(surface, subject, cap->notice);
  }

} /* surface_notice_capabilities() */

/*-----------------------------------------------------------------------*/

/**
 * surface_zoom_scroll() - Adjust the primary zoom from a scroll event
 * @view: view whose bound spin button owns the zoom
 * @event: scroll event
 *
 * Zoom travels through the view's bound spin button, whose value-changed
 * handler owns the view zoom.  Returns TRUE when the event was consumed.
 */
  static gboolean
surface_zoom_scroll(const view_t *view, GdkEventScroll *event)
{
  double value, scale;
  scroll_step_t s;

  s = scroll_step_from_deltas((GdkEvent *)event);

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  if( view->zoom_spin == NULL )
    return( FALSE );

  value = gtk_spin_button_get_value(view->zoom_spin);
  scale = compute_zoom_scale(view->width, view->height, value);

  if( s.direction == GDK_SCROLL_UP )
    value *= (1.0 + 0.1 * s.step * scale);
  else if( s.direction == GDK_SCROLL_DOWN )
    value /= (1.0 + 0.1 * s.step * scale);

  gtk_spin_button_set_value(view->zoom_spin, value);

  return( TRUE );

} /* surface_zoom_scroll() */

/*-----------------------------------------------------------------------*/

/**
 * on_scroll() - Apply a modifier capability or adjust the primary zoom
 * @_widget: signal source, unread: the surface carries the widget
 * @event: scroll event
 * @user_data: surface presenting the scrolled widget
 *
 * A modifier the surface declines leaves the event to the zoom path.
 */
  static gboolean
on_scroll(GtkWidget *_widget, GdkEventScroll *event, gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;
  const surface_capability_t *cap =
    scroll_capability(surface->input, event->state);

  return( cap != NULL ? cap->handler(event, surface)
                      : surface_zoom_scroll(surface->view, event) );

} /* on_scroll() */

/*-----------------------------------------------------------------------*/

/**
 * on_size_allocate() - Own the surface geometry edge for every engine
 * @_widget: signal source, unread: the surface carries the widget
 * @allocation: new logical allocation
 * @user_data: surface whose widget was allocated
 *
 * One geometry event discharges three obligations in order: the view records
 * the drawable in logical units, the engine sizes its own resources in device
 * pixels, and the frame presenting the new size is requested.
 */
  static void
on_size_allocate(GtkWidget *_widget, GtkAllocation *allocation,
    gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;
  int scale = gtk_widget_get_scale_factor(surface->widget);

  view_set_viewport(surface->view, allocation->width, allocation->height);
  canvas_surface_resize(surface, allocation->width * scale,
      allocation->height * scale);
  canvas_surface_queue_redraw(surface);

} /* on_size_allocate() */

/*-----------------------------------------------------------------------*/

/**
 * surface_input_connect() - Wire input and allocation signals to a surface
 * @surface: initialized surface whose widget receives the signals
 */
  void
surface_input_connect(render_surface_t *surface)
{
  if( surface == NULL || surface->widget == NULL || surface->view == NULL )
    return;

  gtk_widget_add_events(surface->widget,
    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
    GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

  g_signal_connect(surface->widget, "button-press-event",
    G_CALLBACK(on_button_press), surface);
  g_signal_connect(surface->widget, "button-release-event",
    G_CALLBACK(on_button_release), surface);
  g_signal_connect(surface->widget, "motion-notify-event",
    G_CALLBACK(on_motion), surface);
  g_signal_connect(surface->widget, "scroll-event",
    G_CALLBACK(on_scroll), surface);
  g_signal_connect(surface->widget, "size-allocate",
    G_CALLBACK(on_size_allocate), surface);

} /* surface_input_connect() */

/*-----------------------------------------------------------------------*/
