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

double Scale_Gain(double gain, int fstep, int idx);
double Inverse_Scale_Gain(double scaled_val);

/**
 * rdpattern_overlay_shift_scroll() - Adjust overlay structure scale from scroll input
 * @dir:      GDK_SCROLL_UP or GDK_SCROLL_DOWN
 * @vp_w:     viewport width in pixels
 * @vp_h:     viewport height in pixels
 * @zoom_pct: current overlay scale as percentage (rdpattern_overlay_scale_adj * 100.0)
 *
 * Gates on cached dispatch result: overlay_active and RENDER_MODE_FARFIELD.
 * Mutates rc_config.rdpattern_overlay_scale_adj and queues a rdpattern redraw.
 * Returns TRUE when the event was consumed, FALSE otherwise.
 */
void rdpattern_view_changed_cb(view_t *v, gpointer user_data);

gboolean rdpattern_overlay_shift_scroll(GdkScrollDirection dir,
    int vp_w, int vp_h, double zoom_pct);

/**
 * rdpattern_shift_scroll() - Scale the overlay structure from a scroll event
 * @event: scroll event carrying the shift modifier
 * @surface: surface of the scrolled view, naming the viewport the scale reads
 *
 * Serves the shift+scroll row of every engine presenting the pattern view.
 */
gboolean rdpattern_shift_scroll(GdkEventScroll *event,
    render_surface_t *surface);

/* Notice advertising the shift+scroll overlay-scale capability */
extern const char rdpattern_shift_scroll_notice[];

/* Modifier scroll operations the Cairo radiation-pattern surface offers */
extern const surface_input_ops_t rdpattern_cairo_input;

#endif

