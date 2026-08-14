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

#ifndef __RENDER_CANVAS_H
#define __RENDER_CANVAS_H 1

#include "render_engine.h"

/*
 * render_canvas: the drawing surfaces and the engines producing their frames.
 *
 * A canvas is a place in the layout that shows rendered frames.  Every engine
 * able to draw there builds the surface object it draws into and hands it to
 * the canvas, which owns it until the binding drops.  Selecting an engine
 * presents that engine's surface while hiding the others, so a renderer swap
 * names an engine and never a widget.  Consumers ask the canvas for its fit,
 * its capture, its text layout and its frames; none infers the producing
 * engine from a widget's GTK class or from the renderer setting.
 *
 * A surface carries the view it shows, or NULL when it shows none: the
 * frequency plots register such a surface and take part in text layout,
 * capture and frame requests alone.  Windows built at runtime take a handle
 * from canvas_create() and carry it for their lifetime.
 */

/* canvas_id_t - handle naming one drawing surface set.  The windows present
 * from program start hold reserved handles; canvas_create() serves the rest. */
typedef enum
{
  CANVAS_NONE = 0,                    /* a zeroed record names no canvas */
  CANVAS_STRUCTURE,
  CANVAS_RDPATTERN,
  CANVAS_FREQPLOTS,
  CANVAS_RESERVED_COUNT               /* first handle canvas_create() takes */

} canvas_id_t;

/* Handles canvas_create() holds at once, one per runtime-built window. */
#define CANVAS_CREATED_MAX 16

/* Canvas array bound: the reserved handles and the created pool. */
#define CANVAS_COUNT (CANVAS_RESERVED_COUNT + CANVAS_CREATED_MAX)

/**
 * canvas_add_surface() - Register the surface an engine draws into
 * @id:      canvas the surface occupies
 * @surface: engine-built surface the canvas takes ownership of
 *
 * Called by the code creating the surface, once per engine able to back the
 * canvas.  Registration alone presents nothing; canvas_set_engine() selects
 * which registered surface the canvas shows.
 */
void canvas_add_surface(canvas_id_t id, render_surface_t *surface);

/**
 * canvas_create() - Register a surface under a handle from the pool
 * @surface: engine-built surface the canvas takes ownership of
 *
 * Called by a window built at runtime, which holds the returned handle for
 * its lifetime and hands it back through canvas_clear() at teardown.  The
 * surface is presented at once, so the window draws without a further call.
 * Returns CANVAS_NONE when the pool is exhausted, having released @surface.
 */
canvas_id_t canvas_create(render_surface_t *surface);

/**
 * canvas_set_engine() - Present the surface of the named engine
 * @id:     canvas to switch
 * @engine: engine whose registered surface becomes the visible one
 *
 * Shows that engine's surface, hides every other surface registered for the
 * canvas, and makes the engine the producer of its frames.  Returns FALSE
 * when the engine registered no surface for this canvas, which is the state
 * of a canvas whose window is closed.
 */
gboolean canvas_set_engine(canvas_id_t id, const render_engine_t *engine);

/**
 * canvas_clear() - Release every surface registered for a canvas
 * @id: canvas whose window is being destroyed
 *
 * Leaves the canvas unbound, so frame requests and captures against it are
 * no-ops until its window is built again.
 */
void canvas_clear(canvas_id_t id);

/**
 * canvas_bound() - Report whether a canvas presents a surface
 * @id: canvas to test
 *
 * A canvas is unbound before its window is built and after it is destroyed.
 */
gboolean canvas_bound(canvas_id_t id);

/**
 * canvas_surface_of() - Return the surface an engine registered
 * @id:     canvas holding the surface
 * @engine: engine that built the surface
 *
 * Serves a domain reaching its own engine's surface whether or not the
 * canvas presents it.  Returns NULL when that engine registered none.
 */
render_surface_t *canvas_surface_of(canvas_id_t id,
                                    const render_engine_t *engine);

/**
 * canvas_of_view() - Find the canvas registering a surface for a view
 * @view: view whose scene the canvas presents
 *
 * Returns CANVAS_NONE when no canvas registers a surface showing @view.
 */
canvas_id_t canvas_of_view(const view_t *view);

/**
 * canvas_pango_layout() - Lay out text in the font of a canvas
 * @id: canvas whose presented surface supplies the font and resolution
 * @text: string to lay out, or NULL for an empty layout the caller fills
 *
 * Serves drawing code measuring or painting text on a canvas, so a producer
 * of text names its canvas instead of resolving a surface.  Returns a layout
 * the caller owns and releases with g_object_unref().
 */
PangoLayout *canvas_pango_layout(canvas_id_t id, const char *text);

/**
 * canvas_sync_viewport() - Record the presented surface allocation in its view
 * @id: canvas presenting the view
 *
 * Returns FALSE while the canvas is unbound or presents no view.
 */
gboolean canvas_sync_viewport(canvas_id_t id);

/**
 * canvas_invalidate() - Request a frame from a canvas immediately
 * @id: canvas to repaint
 *
 * Runs on the GTK main thread and marshals nothing, so a synchronous draw
 * that follows in the same function reads freshly produced pixels.  No-op
 * while the canvas is unbound.
 */
void canvas_invalidate(canvas_id_t id);

/**
 * canvas_draw_sync() - Allocate, invalidate, and draw a canvas synchronously
 * @id: canvas to draw
 * @width: requested allocation width
 * @height: requested allocation height
 * @cr: destination Cairo context
 *
 * Runs on the GTK main thread and returns FALSE while the canvas is unbound
 * or the requested draw inputs are invalid.
 */
gboolean canvas_draw_sync(canvas_id_t id, int width, int height, cairo_t *cr);

/**
 * canvas_queue_redraw() - Schedule a canvas frame through the optimizer gate
 * @id:    canvas to repaint
 * @force: TRUE for user-interaction draws that must always render;
 *         FALSE for intermediate frequency-sweep draws that are
 *         suppressed during optimizer runs to avoid screen flicker
 *
 * Callers on any thread reach the canvas through this entry: main-thread
 * callers mark the canvas dirty synchronously while background-thread
 * callers marshal at GDK_PRIORITY_REDRAW.
 */
void canvas_queue_redraw(canvas_id_t id, gboolean force);

/**
 * canvas_fit_view() - Resolve fitted view state through the bound engine
 * @view: view whose drawn content defines the fit
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when the view's canvas is unbound or its engine has no
 * geometry to fit.
 */
gboolean canvas_fit_view(view_t *view, view_fit_t *fit);

/**
 * canvas_capture() - Capture a canvas frame through the bound engine
 * @id:     canvas to capture
 * @width:  capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated pixbuf, or NULL when the canvas is unbound or
 * its engine cannot capture the rendered frame.
 */
GdkPixbuf *canvas_capture(canvas_id_t id, int width, int height);

#endif /* __RENDER_CANVAS_H */
