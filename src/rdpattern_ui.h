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

#ifndef RDPATTERN_UI_H
#define RDPATTERN_UI_H  1

#include "common.h"
#include "prerender/prerender_state.h"
#include "render/render_surface_input.h"

void ant_temp_entry_set_kelvin(GtkWidget *entry, double kelvin);
void ant_temp_entry_set_unresolved(GtkWidget *entry);

const char *pol_type_name(int pol);
double Scale_Gain(double gain, int fstep, int idx);
double Inverse_Scale_Gain(double scaled_val);

void rdpattern_view_changed_cb(view_t *v, gpointer user_data);

/**
 * nf_static_menu_sync() - Project phase liveness onto static-field controls
 *
 * The near-field static-baseline menu items choose the peak or instantaneous
 * vector drawn while the animation window is closed. While it is open the
 * phase slider drives the vectors, so the selection has no effect.
 */
void nf_static_menu_sync(void);

/* Shift+scroll capability scaling the structure overlay, offered by every
 * engine presenting the radiation-pattern view */
extern surface_capability_t rdpattern_overlay_scale_cap;

/* Modifier scroll operations the Cairo radiation-pattern surface offers */
extern const surface_input_ops_t rdpattern_cairo_input;

#endif

