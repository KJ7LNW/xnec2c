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

#include "config/config_widget.h"

/* config_hooks: xnec2c-specific change-edge hooks bound to rc_config_vars
 * rows, plus session-only field registrations that have no persistence row. */

/** config_hooks_init - Register session-only config_widget fields
 *
 * Called once from main.c startup, before Read_Config(); rc_config_vars
 * rows register themselves separately via rc_config_register_widgets().
 */
void config_hooks_init(void);

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
 *
 * Adds each family's snap marks labeled with the natural parameter and
 * connects the shared snap and value-format handlers with the row as
 * user data.
 */
void color_tone_marks_attach(void);

void hook_theme_change(void);
extern const config_refresh_t hook_theme_change_refresh;

void hook_orthographic(void);
extern const config_refresh_t hook_orthographic_refresh;

void hook_frequency(void);

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
