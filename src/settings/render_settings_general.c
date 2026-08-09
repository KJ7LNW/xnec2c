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

#include "../shared.h"
#include "../callbacks.h"
#include "render_settings.h"
#include "render_settings_common.h"
#include "render_settings_internal.h"

#include "../structure_ui.h"
#include "../rdpattern_ui.h"


/*------------------------------------------------------------------------*/

/* Compile-time width checks for the fields bound to General-tab widgets */
CONFIG_FIELD_INT_ASSERT(rc_config.use_opengl_renderer);
CONFIG_FIELD_INT_ASSERT(rc_config.view_drag_constrained);

/*------------------------------------------------------------------------*/

/** hook_set_renderer - Switch the active renderer, then redraw
 *
 * Change-edge hook for the OpenGL-renderer toggle.
 */
void
hook_set_renderer(void)
{
  opengl_set_renderer(rc_config.use_opengl_renderer);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}

/** hook_set_constrained - Apply constrained-rotation mode, then redraw */
void
hook_set_constrained(void)
{
  opengl_set_constrained_rotation(rc_config.view_drag_constrained);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}

/*------------------------------------------------------------------------*/

/* General tab reset-field list: renderer toggle and constrained rotation.
 * Overlay scale is programmatic session state (shift+scroll) with no widget,
 * so it is excluded from the settings reset. */
void *const general_tab_fields[] = {
  &rc_config.use_opengl_renderer,
  &rc_config.view_drag_constrained,
  NULL,
};

/*------------------------------------------------------------------------*/

/** on_general_reset_clicked - Per-tab Reset button handler for General tab
 *
 * Resets the renderer toggle and constrained rotation to compiled-in
 * defaults; config_reset_tab_user syncs peer widgets and runs each changed
 * field's hook.
 */
void
on_general_reset_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  config_reset_tab_user(SETTINGS_TAB_GENERAL);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}
