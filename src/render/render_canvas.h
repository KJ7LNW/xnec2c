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
 * able to draw there registers the surface it draws into, and selecting an
 * engine presents that surface while hiding the others, so a renderer swap
 * names an engine and never a widget.  Consumers ask the canvas for its
 * surface, its fit, its capture and its frames; none infers the producing
 * engine from a widget's GTK class or from the renderer setting.
 *
 * Two canvases carry a view_t scene; the frequency plots carry none and are
 * registered for capture and frame requests alone.  Windows built at runtime
 * take a handle from canvas_create() and carry it for their lifetime.
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
 * @id:     canvas the surface occupies
 * @widget: surface taking part in layout and capture
 * @engine: control-op vtable of the engine drawing into @widget
 *
 * Called by the code creating the widget, once per engine able to back the
 * canvas.  Registration alone presents nothing; canvas_set_engine() selects
 * which registered surface the canvas shows.
 */
void canvas_add_surface(canvas_id_t id, GtkWidget *widget,
                        const render_engine_ops_t *engine);

/**
 * canvas_create() - Register a surface under a handle from the pool
 * @widget: surface taking part in layout and capture
 * @engine: control-op vtable of the engine drawing into @widget
 *
 * Called by a window built at runtime, which holds the returned handle for
 * its lifetime and hands it back through canvas_clear() at teardown.  The
 * surface is presented at once, so the window draws without a further call.
 * Returns CANVAS_NONE when the pool is exhausted.
 */
canvas_id_t canvas_create(GtkWidget *widget,
                          const render_engine_ops_t *engine);

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
gboolean canvas_set_engine(canvas_id_t id, const render_engine_ops_t *engine);

/**
 * canvas_clear() - Drop every surface registered for a canvas
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
 * canvas_of_view() - Project a view identity onto its canvas identity
 * @type: view whose scene the canvas presents
 *
 * Returns the canvas driven by the named view.
 */
canvas_id_t canvas_of_view(view_type_t type);

/**
 * canvas_widget() - Return the surface a canvas presents
 * @id: canvas to resolve
 *
 * The canvas must be bound; an unbound canvas is a producer error and is
 * reported as a bug.  Sites reached while a window may be closed test
 * canvas_bound() first.
 */
GtkWidget *canvas_widget(canvas_id_t id);

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
