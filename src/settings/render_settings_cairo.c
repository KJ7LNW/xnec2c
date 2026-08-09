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
#include "render_settings_common.h"
#include "render_settings_internal.h"
#include "../structure_ui.h"

/*------------------------------------------------------------------------*/

/* Compile-time width checks for the fields bound to Cairo-tab widgets */
CONFIG_FIELD_INT_ASSERT(rc_config.cairo_antialias);
CONFIG_FIELD_INT_ASSERT(rc_config.cairo_color_quant);
CONFIG_FIELD_INT_ASSERT(rc_config.cairo_line_cap);
CONFIG_FIELD_INT_ASSERT(rc_config.cairo_depth_bins);

/*------------------------------------------------------------------------*/

/* Cairo tab reset-field list: anti-aliasing, color quantization, line cap,
 * and depth bins. */
void *const cairo_tab_fields[] = {
  &rc_config.cairo_antialias,
  &rc_config.cairo_color_quant,
  &rc_config.cairo_line_cap,
  &rc_config.cairo_depth_bins,
  NULL,
};

/*------------------------------------------------------------------------*/

/** on_cairo_tab_reset_clicked - Per-tab Reset button handler for Cairo tab
 *
 * Restores Cairo rendering settings to compiled-in defaults;
 * config_reset_tab_user syncs peer widgets and runs each changed field's
 * hook, then the Cairo drawing areas redraw.
 */
void
on_cairo_tab_reset_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  config_reset_tab_user(SETTINGS_TAB_CAIRO);
  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}
