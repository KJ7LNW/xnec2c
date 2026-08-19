/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#ifndef CAIRO_FRAME_H
#define CAIRO_FRAME_H 1

#include "cairo_draw.h"
#include "../render/render_dispatch.h"

/* Cairo backend vtable; render() gates slot calls by mode */
extern const render_ops_t cairo_ops;

/** cairo_capture_pixbuf() - Capture a Cairo drawing window into a pixbuf */
GdkPixbuf *cairo_capture_pixbuf(render_surface_t *surface,
    int width, int height);

/** cairo_surface_adopt() - Build a Cairo surface presenting a drawing area
 * @area: drawing area the surface presents through
 * @view: view the surface shows, or NULL for a surface showing none
 * @input: modifier scroll operations of the presenting domain, or NULL
 *
 * Returns a surface the caller hands to a canvas, which owns it from then on.
 */
render_surface_t *cairo_surface_adopt(GtkWidget *area, view_t *view,
    const surface_input_ops_t *input);

/** cairo_surface_free() - Release a Cairo surface and its retained scene */
void cairo_surface_free(render_surface_t *surface);

/** cairo_queue_redraw() - Request a frame from a Cairo surface */
void cairo_queue_redraw(render_surface_t *surface);

/** render_cairo() - Per-frame Cairo draw path; clear, deposit, flush, overlay */
gboolean render_cairo(render_surface_t *surface, cairo_t *cr);

#endif /* CAIRO_FRAME_H */
