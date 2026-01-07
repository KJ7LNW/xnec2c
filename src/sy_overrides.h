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

#ifndef SY_OVERRIDES_H
#define SY_OVERRIDES_H    1

#include "common.h"

/* Initialize the symbol overrides window */
gboolean sy_overrides_init(void);

/* Show the symbol overrides window */
void sy_overrides_show(void);

/* Hide the symbol overrides window */
void sy_overrides_hide(void);

/* Refresh the symbol overrides display from symbol table */
void sy_overrides_refresh(void);

/* Cleanup resources */
void sy_overrides_cleanup(void);

/* Signal handlers called from glade */
void on_show_sy_overrides_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_sy_overrides_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_sy_overrides_apply_clicked(GtkButton *button, gpointer user_data);
void on_sy_overrides_cancel_clicked(GtkButton *button, gpointer user_data);
void on_sy_overrides_close_clicked(GtkButton *button, gpointer user_data);

#endif
