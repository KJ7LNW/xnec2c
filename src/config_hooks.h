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

#ifndef CONFIG_HOOKS_H
#define CONFIG_HOOKS_H 1

#include "config/widget/config_widget.h"

/* config_hooks: xnec2c-specific change-edge hooks bound to rc_config_vars
 * rows, with the widget bindings a row shares across windows. */

void hook_polarization(void);
extern const config_refresh_t hook_polarization_refresh;

void hook_common_projection(void);
extern const config_refresh_t hook_common_projection_refresh;

void hook_common_pan(void);
extern const config_refresh_t hook_common_pan_refresh;

void hook_flow_direction(void);
extern const config_refresh_t hook_flow_direction_refresh;

void hook_color_vis(void);
extern const config_refresh_t hook_color_vis_refresh;

void hook_color_family(void);
extern const config_refresh_t hook_color_family_refresh;

/**
 * color_tone_marks_attach() - Mark and wire the family sliders at creation
 * @builder: builder holding the family slider rows
 *
 * Adds each family's snap marks labeled with the natural parameter and
 * connects the shared snap and value-format handlers with the row as
 * user data.
 */
void color_tone_marks_attach(GtkBuilder *builder);

/* The color panel's two hosts.  The animate dialog and the render settings
 * Color tab load one glade definition into their own builders, so one widget
 * id addresses a widget in each and a selection made while animating stays
 * reachable once the animate dialog closes.  color_builders[] in
 * color/color_config_hooks.c names the same windows for panel writes. */
#define COLOR_PANEL_WIDGET(id, hook) \
  CONFIG_WIDGET_TREE( .post_apply = (hook), \
    .groups = CONFIG_WIDGET_GROUPS( \
      CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder, \
        .elements = CONFIG_WIDGETS( \
          CONFIG_WIDGET( .widget_id = (id) ), NULL ) ), \
      CONFIG_WIDGET_GROUP( .builder = &render_settings_builder, \
        .elements = CONFIG_WIDGETS( \
          CONFIG_WIDGET( .widget_id = (id) ), NULL ) ), \
      NULL ) )

/* Both host groups of one color pull-down, naming the same rows and the same
 * collapsed label.  Every member the field can hold appears in each group,
 * because config_widget_label_row_declared() reports a bug for a stored value
 * no declared row expresses. */
#define COLOR_PANEL_MENU_GROUPS(label_id, ...) \
  CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder, \
    .value_label_id = (label_id), \
    .elements = CONFIG_WIDGETS( __VA_ARGS__, NULL ) ), \
  CONFIG_WIDGET_GROUP( .builder = &render_settings_builder, \
    .value_label_id = (label_id), \
    .elements = CONFIG_WIDGETS( __VA_ARGS__, NULL ) )

void hook_theme_change(void);
extern const config_refresh_t hook_theme_change_refresh;

void hook_orthographic(void);
extern const config_refresh_t hook_orthographic_refresh;

void hook_rdpat_ehfield(void);
extern const config_refresh_t hook_rdpat_ehfield_refresh;

void hook_rdpat_redraw(void);
extern const config_refresh_t hook_rdpat_redraw_refresh;

void hook_ff_overlay(void);
extern const config_refresh_t hook_ff_overlay_refresh;

void hook_ant_temp(void);
extern const config_refresh_t hook_ant_temp_refresh;

void hook_freqplots_panel_select(void);
extern const config_refresh_t hook_freqplots_panel_select_refresh;

void hook_freqplots_redraw(void);
extern const config_refresh_t hook_freqplots_redraw_refresh;

void hook_freqplots_net_gain(void);
extern const config_refresh_t hook_freqplots_net_gain_refresh;

/* Declare render-settings hooks beside every row-bound refresh;
 * settings/ owns their definitions. */
void hook_render_redraw(void);
extern const config_refresh_t hook_render_redraw_refresh;

void hook_set_renderer(void);
extern const config_refresh_t hook_set_renderer_refresh;

void hook_set_constrained(void);
extern const config_refresh_t hook_set_constrained_refresh;

void hook_set_msaa(void);
extern const config_refresh_t hook_set_msaa_refresh;

void hook_set_radius_scale(void);
extern const config_refresh_t hook_set_radius_scale_refresh;

#endif /* CONFIG_HOOKS_H */
