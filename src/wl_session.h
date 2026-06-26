/*
 *  wl_session.h - Wayland xdg-session-management window restore.
 *
 *  Binds the compositor session manager, restores each toplevel's saved
 *  geometry before its first map, and captures the per-window session
 *  cookie persisted in rc_config.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef WL_SESSION_H
#define WL_SESSION_H

#include <gtk/gtk.h>

/* Bind the xdg_session_manager_v1 global and open the app-wide session
 * using rc_config.session_id; no-op when the manager is unavailable. */
void wl_session_init( void );

/* Connect the xdg-toplevel-realized restore hook for one toplevel,
 * keyed by a stable name held in the module's static table. */
void wl_session_register_window( GtkWidget *window, const char *name );

#endif /* WL_SESSION_H */
