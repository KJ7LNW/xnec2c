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

#ifndef __RENDER_CANVAS_SURFACE_H
#define __RENDER_CANVAS_SURFACE_H 1

#include "render_engine.h"

/**
 * canvas_surface_engine_complete() - Report whether an engine supplies its
 * frame protocol and every surface operation
 * @engine: candidate engine, or NULL
 */
gboolean canvas_surface_engine_complete(const render_engine_t *engine);

/**
 * canvas_surface_has_engine() - Report whether a surface binds an engine
 * @surface: registered surface object, or NULL for a slot holding none
 * @engine:  concrete engine identity to compare
 */
gboolean canvas_surface_has_engine(const render_surface_t *surface,
    const render_engine_t *engine);

/**
 * canvas_surface_free() - Release a surface through its own engine
 * @surface: surface leaving the canvas, or NULL
 */
void canvas_surface_free(render_surface_t *surface);

/**
 * canvas_surface_hide() - Hide a registered physical surface
 * @surface: surface whose widget leaves presentation
 */
void canvas_surface_hide(const render_surface_t *surface);

/**
 * canvas_surface_show() - Show and resize a registered physical surface
 * @surface: surface whose widget becomes the presentation
 */
void canvas_surface_show(const render_surface_t *surface);

/**
 * canvas_surface_queue_redraw() - Request an engine frame immediately
 * @surface: active surface object
 */
void canvas_surface_queue_redraw(render_surface_t *surface);

/**
 * canvas_surface_sync_viewport() - Record the current surface allocation
 * @surface: active view-backed surface, which receives the allocation
 */
void canvas_surface_sync_viewport(render_surface_t *surface);

/**
 * canvas_surface_fit() - Compute fitted state through the surface engine
 * @surface: active view-backed surface
 * @fit:     receives fitted zoom and pan
 *
 * Returns FALSE when the engine fits no geometry.
 */
gboolean canvas_surface_fit(render_surface_t *surface, view_fit_t *fit);

/**
 * canvas_surface_capture() - Capture pixels through the surface engine
 * @surface: active surface object
 * @width:   capture width in pixels
 * @height:  capture height in pixels
 *
 * Returns NULL for a degenerate capture size.
 */
GdkPixbuf *canvas_surface_capture(render_surface_t *surface,
    int width, int height);

/**
 * canvas_surface_pango_layout() - Create text layout from surface styling
 * @surface: active surface supplying font and resolution
 * @text:    text to lay out, or NULL for an empty caller-filled layout
 */
PangoLayout *canvas_surface_pango_layout(const render_surface_t *surface,
    const char *text);

/**
 * canvas_surface_draw_sync() - Allocate, invalidate, and draw one surface
 * @surface: active surface object
 * @width:   requested allocation width
 * @height:  requested allocation height
 * @cr:      destination Cairo context
 *
 * Returns FALSE for degenerate draw inputs.
 */
gboolean canvas_surface_draw_sync(render_surface_t *surface,
    int width, int height, cairo_t *cr);

#endif /* __RENDER_CANVAS_SURFACE_H */
