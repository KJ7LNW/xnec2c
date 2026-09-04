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

#ifndef ANIM_DIALOG_H
#define ANIM_DIALOG_H   1

#include "../common.h"

/**
 * anim_dialog_readout_update() - Present the displayed phase
 *
 * Projects the phase context onto the dialog readout without moving the
 * slider, whose position stays put while playback runs.  Does nothing while
 * the dialog is closed.
 */
void anim_dialog_readout_update(void);

/** anim_panel_sensitivity() - Project class truth onto animation controls */
void anim_panel_sensitivity(void);

/**
 * on_animate_dialog_activate() - Open the animation window
 * @_menuitem:  emitting menu item, unused
 * @_user_data: unused
 *
 * Serves the structure and radiation-pattern menu entries alike, since the
 * dialog presents every animated class the model admits.
 */
void on_animate_dialog_activate(GtkMenuItem *_menuitem,
    gpointer _user_data);

/**
 * on_animation_okbutton_clicked() - Close the animation window
 * @_button:    emitting button, unused
 * @_user_data: unused
 */
void on_animation_okbutton_clicked(GtkButton *_button, gpointer _user_data);

/**
 * on_animate_dialog_destroy() - Release the animation window and its context
 * @_object:    emitting dialog, unused
 * @_user_data: unused
 */
void on_animate_dialog_destroy(GObject *_object, gpointer _user_data);

#endif /* ANIM_DIALOG_H */
