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
void hook_common_pan(void);
void hook_flow_direction(void);
void hook_color_vis(void);
void hook_color_family(void);
void hook_theme_change(void);
void hook_orthographic(void);
void hook_frequency(void);

void hook_rdpat_ehfield(void);
void hook_rdpat_redraw(void);

void hook_freqplots_panel_select(void);
void hook_freqplots_redraw(void);
void hook_freqplots_net_gain(void);
void hook_freqplots_swap_click(void);

#endif /* CONFIG_HOOKS_H */
