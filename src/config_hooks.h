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
void hook_common_projection(void);
void hook_flow_direction(void);
void hook_color_vis(void);
void hook_color_family(void);
void hook_theme_change(void);
void hook_orthographic(void);
void hook_frequency(void);

void hook_rdpat_e_field(void);
void hook_rdpat_h_field(void);
void hook_rdpat_poynting(void);
void hook_rdpat_overlay(void);
void hook_rdpat_gradient_key(void);
void hook_rdpat_draw_style(void);

void hook_freqplots_gmax(void);
void hook_freqplots_gdir(void);
void hook_freqplots_gviewer(void);
void hook_freqplots_vswr(void);
void hook_freqplots_zrlzim(void);
void hook_freqplots_zmgzph(void);
void hook_freqplots_smith(void);
void hook_freqplots_ant_temp(void);
void hook_freqplots_agt(void);
void hook_freqplots_net_gain(void);
void hook_freqplots_min_max(void);
void hook_freqplots_clamp_vswr(void);
void hook_freqplots_show_ant_temp(void);
void hook_freqplots_round_x_axis(void);
void hook_freqplots_swap_click(void);
void hook_freqplots_s11(void);

#endif /* CONFIG_HOOKS_H */
