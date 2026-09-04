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

#ifndef ANIM_PHASE_CONTROL_H
#define ANIM_PHASE_CONTROL_H    1

#include "../common.h"

/**
 * anim_phase_control_stop() - End playback and release its timing sources
 *
 * Sole owner of stopping playback: clears the playback state and removes the
 * tick and delayed cadence sources.  Leaves the phase context untouched.
 */
void anim_phase_control_stop(void);

/** anim_phase_slider_attach() - Wire the phase slider drag handler */
void anim_phase_slider_attach(void);

/**
 * on_animate_phase_slider_change_value() - Apply a keyboard phase scrub
 * @range: emitting phase scale
 * @scroll: scroll action type
 * @value: proposed phase in degrees
 * @_user_data: unused
 */
gboolean on_animate_phase_slider_change_value(GtkRange *range,
    GtkScrollType scroll, gdouble value, gpointer _user_data);

/**
 * Animate_Phase() - Advance the phase by one playback step
 * @_user_data: unused
 *
 * Returns G_SOURCE_REMOVE once playback has stopped and G_SOURCE_CONTINUE
 * while it runs.
 */
gboolean Animate_Phase(gpointer _user_data);

/**
 * on_animate_spinbutton_value_changed() - Restart playback timing after edits
 * @spinbutton: emitting cadence spin button
 * @_user_data: unused
 */
void on_animate_spinbutton_value_changed(GtkSpinButton *spinbutton,
    gpointer _user_data);

/**
 * on_animate_spinbutton_focus_out_event() - Apply cadence edits on focus loss
 * @_widget:   emitting cadence spin button, unused
 * @_event:    focus event, unused
 * @_user_data: unused
 *
 * Returns FALSE so the event continues to propagate.
 */
gboolean on_animate_spinbutton_focus_out_event(GtkWidget *_widget,
    GdkEventFocus *_event, gpointer _user_data);

/**
 * on_animation_applybutton_clicked() - Start playback
 * @_button:    emitting button, unused
 * @_user_data: unused
 */
void on_animation_applybutton_clicked(GtkButton *_button,
    gpointer _user_data);

/**
 * on_animation_cancelbutton_clicked() - Stop playback and return to zero phase
 * @_button:    emitting button, unused
 * @_user_data: unused
 */
void on_animation_cancelbutton_clicked(GtkButton *_button,
    gpointer _user_data);

#endif /* ANIM_PHASE_CONTROL_H */
