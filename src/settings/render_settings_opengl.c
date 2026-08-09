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
#include "../opengl/opengl_structure.h"
#include "../opengl/opengl_msaa.h"
#include "../structure_ui.h"
#include "../rdpattern_ui.h"

/*------------------------------------------------------------------------*/

/* Compile-time width checks for the fields bound to OpenGL-tab widgets */
CONFIG_FIELD_INT_ASSERT(rc_config.opengl_msaa_samples);
CONFIG_FIELD_INT_ASSERT(rc_config.opengl_transparent_on_click);
CONFIG_FIELD_INT_ASSERT(rc_config.rdpattern_draw_style);
CONFIG_FIELD_INT_ASSERT(rc_config.opengl_orthographic);
CONFIG_FIELD_DBL_ASSERT(rc_config.opengl_cylinder_radius_scale);

CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_segments);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_patches);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_rdpat_surface);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_rdpat_wire);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_nearfield);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_ground_plane);
CONFIG_FIELD_FLT_ASSERT(rc_config.brightness_axes);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_segments);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_patches);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_rdpat_surface);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_rdpat_wire);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_nearfield);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_ground_plane);
CONFIG_FIELD_FLT_ASSERT(rc_config.transparency_axes);

/*------------------------------------------------------------------------*/

/** hook_set_msaa - Apply the MSAA sample count, then redraw */
void
hook_set_msaa(void)
{
  Set_MSAA_Samples(rc_config.opengl_msaa_samples);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}

/** hook_set_radius_scale - Apply the cylinder radius scale, then redraw */
void
hook_set_radius_scale(void)
{
  opengl_structure_set_radius_scale(rc_config.opengl_cylinder_radius_scale);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}

/*------------------------------------------------------------------------*/

/* OpenGL tab reset-field list: orthographic projection, per-type brightness
 * and transparency, cylinder scale, transparent-on-click, MSAA, draw style. */
void *const opengl_tab_fields[] = {
  &rc_config.opengl_orthographic,
  &rc_config.brightness_segments,
  &rc_config.brightness_patches,
  &rc_config.brightness_rdpat_surface,
  &rc_config.brightness_rdpat_wire,
  &rc_config.brightness_nearfield,
  &rc_config.brightness_ground_plane,
  &rc_config.brightness_axes,
  &rc_config.transparency_segments,
  &rc_config.transparency_patches,
  &rc_config.transparency_rdpat_surface,
  &rc_config.transparency_rdpat_wire,
  &rc_config.transparency_nearfield,
  &rc_config.transparency_ground_plane,
  &rc_config.transparency_axes,
  &rc_config.opengl_cylinder_radius_scale,
  &rc_config.opengl_transparent_on_click,
  &rc_config.opengl_msaa_samples,
  &rc_config.rdpattern_draw_style,
  NULL,
};

/*------------------------------------------------------------------------*/

/** on_opengl_tab_reset_clicked - Per-tab Reset button handler for OpenGL tab
 *
 * Restores orthographic projection, brightness, transparency, cylinder scale,
 * MSAA, and draw style to compiled-in defaults.  config_reset_tab_user syncs
 * peer widgets and runs each changed field's hook; the invalidate and redraws
 * refresh the OpenGL views.
 */
void
on_opengl_tab_reset_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  config_reset_tab_user(SETTINGS_TAB_OPENGL);
  Queue_Structure_Rebuild( TRUE );
  Queue_Radiation_Redraw(TRUE);
}
