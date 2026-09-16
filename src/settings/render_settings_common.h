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

#ifndef RENDER_SETTINGS_COMMON_H
#define RENDER_SETTINGS_COMMON_H 1

#include "../common.h"
#include "../config/widget/config_widget.h"

/*------------------------------------------------------------------------*/

/* Settings tab indices for dispatch */
typedef enum {
  SETTINGS_TAB_GENERAL = 0,
  SETTINGS_TAB_OPENGL,
  SETTINGS_TAB_CAIRO,
  SETTINGS_TAB_COUNT
} settings_tab_t;

/*------------------------------------------------------------------------*/

/** config_reset_tab_user - Reset one tab and invoke post_apply hooks
 * @tab: which tab to reset
 *
 * Applies each field's compiled-in default from rc_config_vars and invokes
 * post_apply for each entry whose value changed.  For user-initiated resets
 * only (widgets must exist).
 */
void config_reset_tab_user(settings_tab_t tab);

/** render_settings_load_glade - Load all three settings glade resources
 *
 * Creates the builder, loads render_settings.glade, opengl_settings.glade,
 * and cairo_settings.glade, connects signals, and extracts the window widget.
 * Returns TRUE on success; on failure, logs the error and returns FALSE.
 * Caller is responsible for setting render_settings_window on success.
 */
gboolean render_settings_load_glade(void);

#endif /* RENDER_SETTINGS_COMMON_H */
