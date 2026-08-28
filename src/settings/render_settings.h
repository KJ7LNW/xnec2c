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

#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H 1

#include "../common.h"
#include "../config_hooks.h"

/** render_settings_init - Create builder and window from glade resource */
gboolean render_settings_init(void);

/** render_settings_show - Show the settings window, creating if needed */
void render_settings_show(void);

/** render_settings_hide - Hide the settings window */
void render_settings_hide(void);

/** render_settings_sync_from_config - Update all widgets from rc_config */
void render_settings_sync_from_config(void);

#endif /* RENDER_SETTINGS_H */
