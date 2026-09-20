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

#ifndef WINDOW_LIFECYCLE_H
#define WINDOW_LIFECYCLE_H  1

#include "common.h"

gboolean window_is_open(window_t type);
GtkBuilder *window_builder(window_t type);
view_t *window_view(window_t type);
window_t window_from_widget(GtkWidget *widget);
void window_capture_geometry(GtkWidget *toplevel);
void window_default_origin(window_t type, gint *x, gint *y);
void window_release(window_t type);

#endif
