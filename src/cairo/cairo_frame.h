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
GdkPixbuf *cairo_capture_pixbuf(GtkWidget *widget, int width, int height);

/** cairo_frame_get_scenebuffer() - Return per-view scenebuffer
 * @type: VIEW_STRUCTURE or VIEW_RDPATTERN
 */
cairo_scenebuffer_t *cairo_frame_get_scenebuffer(view_type_t type);

/** cairo_frame_destroy() - Release scenebuffer allocations at shutdown */
void cairo_frame_destroy(void);

/** render_cairo() - Per-frame Cairo draw path; clear, deposit, flush, overlay */
gboolean render_cairo(cairo_render_ctx_t *ctx);

#endif /* CAIRO_FRAME_H */
