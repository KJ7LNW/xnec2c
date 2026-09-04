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

#include "anim_phase_control.h"
#include "anim_class.h"
#include "anim_dialog.h"
#include "anim_phase.h"
#include "../callbacks.h"
#include "../shared.h"

/* Delay before an edited cadence restarts the tick, in milliseconds */
#define ANIM_CADENCE_APPLY_MS 500

/* Playback advances the phase; the phase context stays open without it. */
static gboolean playback_running;

/* Playback tick source, held only while playback runs */
static guint animation_timer_source_id;

/* Deferred cadence-apply source, coalescing rapid spin-button edits */
static guint animation_apply_timer;

/**
 * phase_source_remove() - Retire one owned GLib source
 * @source: source identifier to remove and clear
 */
static void
phase_source_remove(guint *source)
{
  if( *source == 0 )
    return;

  g_source_remove(*source);
  *source = 0;
}

/** phase_wrap() - Wrap a value into a half-open phase span */
static gdouble
phase_wrap(gdouble value, gdouble lower, gdouble span)
{
  value = fmod(value - lower, span);
  if( dl_flt(value, DL_EPS) )
    value += span;

  return value + lower;
}

/** on_animate_phase_slider_change_value() - Apply a keyboard phase scrub */
gboolean
on_animate_phase_slider_change_value(GtkRange *range, GtkScrollType scroll,
    gdouble value, gpointer _user_data)
{
  GtkAdjustment *adj;
  gdouble lower;
  gdouble span;


  if( scroll == GTK_SCROLL_JUMP )
    return TRUE;

  adj = gtk_range_get_adjustment(range);
  lower = gtk_adjustment_get_lower(adj);
  span = gtk_adjustment_get_upper(adj) - lower;

  if( !dl_fgt(span, -DL_EPS) )
    return TRUE;

  value = phase_wrap(value, lower, span);
  gtk_range_set_value(range, value);
  anim_phase_set(value * TORAD);
  anim_dispatch_redraw();
  anim_dialog_readout_update();

  return TRUE;
}

/** on_phase_slider_motion_notify() - Apply a pointer-drag phase scrub */
static gboolean
on_phase_slider_motion_notify(GtkWidget *widget, GdkEventMotion *event,
    gpointer _user_data)
{
  static gdouble prev_x = NAN;
  GtkRange *range;
  GtkAdjustment *adj;
  GdkRectangle rect;
  gdouble lower;
  gdouble span;
  gdouble value;


  if( !(event->state & GDK_BUTTON1_MASK) )
  {
    prev_x = NAN;
    return FALSE;
  }

  range = GTK_RANGE(widget);
  adj = gtk_range_get_adjustment(range);
  lower = gtk_adjustment_get_lower(adj);
  span = gtk_adjustment_get_upper(adj) - lower;
  gtk_range_get_range_rect(range, &rect);

  if( rect.width <= 0 || !dl_fgt(span, -DL_EPS) )
    return FALSE;

  if( isnan(prev_x) )
  {
    prev_x = event->x;
    return FALSE;
  }

  value = gtk_range_get_value(range)
      + (event->x - prev_x) * span / (gdouble)rect.width;
  prev_x = event->x;
  value = phase_wrap(value, lower, span);

  SIGNAL_BLOCK(range, on_animate_phase_slider_change_value);
  gtk_range_set_value(range, value);
  SIGNAL_UNBLOCK(range, on_animate_phase_slider_change_value);

  anim_phase_set(value * TORAD);
  anim_dispatch_redraw();
  anim_dialog_readout_update();

  return FALSE;
}

/** anim_phase_slider_attach() - Wire the phase slider drag handler */
void
anim_phase_slider_attach(void)
{
  g_signal_connect(Builder_Get_Object(animate_dialog_builder,
        "animate_phase_slider"), "motion-notify-event",
      G_CALLBACK(on_phase_slider_motion_notify), NULL);
}

/** anim_phase_slider_reset() - Return the slider to zero degrees */
static void
anim_phase_slider_reset(void)
{
  GtkRange *slider;

  if( animate_dialog == NULL )
    return;

  slider = GTK_RANGE(Builder_Get_Object(animate_dialog_builder,
        "animate_phase_slider"));
  SIGNAL_BLOCK(slider, on_animate_phase_slider_change_value);
  gtk_range_set_value(slider, 0.0);
  SIGNAL_UNBLOCK(slider, on_animate_phase_slider_change_value);
}

/** anim_phase_slider_sync_sensitivity() - Project playback onto the slider */
static void
anim_phase_slider_sync_sensitivity(void)
{
  if( animate_dialog == NULL )
    return;

  gtk_widget_set_sensitive(Builder_Get_Object(animate_dialog_builder,
        "animate_phase_slider"), !playback_running);
}

/** Animate_Phase() - Advance the phase by one playback step
 * @_user_data: unused
 *
 * Returns G_SOURCE_REMOVE once playback has stopped.  Otherwise advances the
 * animation phase by one step, folds the animating classes into one frame
 * request per canvas, then refreshes the phase readout.
 */
  gboolean
Animate_Phase(gpointer _user_data)
{
  if( !playback_running )
  {
    animation_timer_source_id = 0;
    return( G_SOURCE_REMOVE );
  }

  anim_phase_advance();

  anim_dispatch_redraw();
  anim_dialog_readout_update();

  return( G_SOURCE_CONTINUE );

} /* Animate_Phase() */

/** update_animation_parameters() - Rebuild the tick from the cadence controls
 *
 * Reads the cycle frequency and frame rate, resolves the per-tick phase step,
 * then replaces the tick source while playback runs.
 */
  static void
update_animation_parameters(void)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, fps;

  spinbutton = GTK_SPIN_BUTTON(
      Builder_Get_Object(animate_dialog_builder, "animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON(
      Builder_Get_Object(animate_dialog_builder, "animate_steps_spinbutton") );
  fps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / fps);
  anim_phase_set_step( (double)M_2PI * freq / fps );

  phase_source_remove(&animation_timer_source_id);

  if( playback_running )
    animation_timer_source_id = g_timeout_add( intval, Animate_Phase, NULL );
}

/** anim_phase_control_stop() - End playback and release its timing sources */
  void
anim_phase_control_stop(void)
{
  playback_running = FALSE;
  phase_source_remove(&animation_timer_source_id);
  phase_source_remove(&animation_apply_timer);
}

/** apply_animation_delayed() - Apply a settled cadence edit
 * @_udata: unused
 *
 * Returns G_SOURCE_REMOVE, having retired its own source.
 */
  static gboolean
apply_animation_delayed(gpointer _udata)
{
  if( playback_running )
    update_animation_parameters();

  animation_apply_timer = 0;
  return( G_SOURCE_REMOVE );
}

  void
on_animate_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         _user_data)
{
  gtk_spin_button_update( spinbutton );

  /* Skip live update if no animation is active */
  if( !playback_running )
    return;

  phase_source_remove(&animation_apply_timer);
  animation_apply_timer = g_timeout_add( ANIM_CADENCE_APPLY_MS,
      apply_animation_delayed, NULL );
}

  gboolean
on_animate_spinbutton_focus_out_event(
    GtkWidget       *_widget,
    GdkEventFocus   *_event,
    gpointer         _user_data)
{
  phase_source_remove(&animation_apply_timer);

  if( playback_running )
    update_animation_parameters();

  return( FALSE );
}

  void
on_animation_applybutton_clicked(
    GtkButton       *_button,
    gpointer         _user_data)
{
  /* Validate near-field setup only when no structure content can animate */
  if( !anim_domain_available(ANIM_DOMAIN_STRUCTURE) &&
      rdpat_ehfield_active() && !Validate_Nearfield_Animation() )
    return;

  playback_running = TRUE;
  anim_phase_slider_sync_sensitivity();
  update_animation_parameters();

}

  void
on_animation_cancelbutton_clicked(
    GtkButton       *_button,
    gpointer         _user_data)
{
  anim_phase_control_stop();
  anim_phase_set( 0.0 );

  /* Re-enable manual scrubbing and return the slider and readout to the reset
   * phase. */
  anim_phase_slider_reset();
  anim_phase_slider_sync_sensitivity();
  anim_dispatch_redraw();
  anim_dialog_readout_update();
}
