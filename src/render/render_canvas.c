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

/*
 * render_canvas: the canvas registry and canvas-scoped control-op dispatch.
 *
 * Holds the surfaces registered for each canvas, which of them the canvas
 * presents, and the engine producing its frames.  Control operations reach
 * the engine leaf through that record, so this layer names no engine and
 * reads no renderer setting.  Each leaf owns its geometry reduction and
 * zoom/pan inversion.
 */

#include "render_canvas.h"
#include "render_redraw.h"
#include "../shared.h"

/* Engines able to draw into one canvas: a Cairo drawing area and a GL area. */
#define CANVAS_SURFACES_MAX 2

/* canvas_surface_t - the surface one engine draws into. */
typedef struct
{
  GtkWidget *widget;                  /* surface in the layout */
  const render_engine_ops_t *engine;  /* engine drawing into it; NULL when the slot is free */

} canvas_surface_t;

/* canvas_t - the surfaces registered for one canvas and the presented one. */
typedef struct
{
  canvas_surface_t surfaces[CANVAS_SURFACES_MAX];
  const canvas_surface_t *active;     /* NULL while the canvas is unbound */

} canvas_t;

/*-----------------------------------------------------------------------*/

/* The canvases.  Static for the process lifetime, so a marshaled frame
 * request carries a canvas id rather than a record pointer. */
static canvas_t canvases[CANVAS_COUNT];

/* Projection of view identity onto canvas identity.  Read only through
 * canvas_of_view(); CANVAS_FREQPLOTS drives no view and appears nowhere. */
static const canvas_id_t view_canvas[] =
{
  [VIEW_STRUCTURE] = CANVAS_STRUCTURE,
  [VIEW_RDPATTERN] = CANVAS_RDPATTERN,
};

/*-----------------------------------------------------------------------*/

/**
 * canvas_surface_of_engine() - Find the surface an engine registered
 * @canvas: canvas holding the surfaces
 * @engine: engine to match, or NULL to take a free slot
 *
 * Returns NULL when the engine registered no surface, or when no slot is
 * free for a NULL @engine.
 */
  static canvas_surface_t *
canvas_surface_of_engine(canvas_t *canvas, const render_engine_ops_t *engine)
{
  canvas_surface_t *found = NULL;
  int idx = 0;

  while( found == NULL && idx < CANVAS_SURFACES_MAX )
  {
    if( canvas->surfaces[idx].engine == engine )
      found = &canvas->surfaces[idx];

    idx++;
  }

  return( found );

} /* canvas_surface_of_engine() */

/*-----------------------------------------------------------------------*/

/* GSourceFunc wrapper for a canvas frame request, scheduled by
 * canvas_queue_redraw(). */
  static gboolean
canvas_invalidate_cb(gpointer id)
{
  canvas_invalidate( (canvas_id_t)GPOINTER_TO_INT(id) );

  return( G_SOURCE_REMOVE );

} /* canvas_invalidate_cb() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_add_surface() - Register the surface an engine draws into
 * @id:     canvas the surface occupies
 * @widget: surface taking part in layout and capture
 * @engine: control-op vtable of the engine drawing into @widget
 *
 * An engine registering again replaces its own surface; a new engine takes
 * a free slot.
 */
  void
canvas_add_surface(canvas_id_t id, GtkWidget *widget,
    const render_engine_ops_t *engine)
{
  canvas_t *canvas = &canvases[id];
  canvas_surface_t *surface;

  if( widget == NULL || engine == NULL )
  {
    BUG("canvas %d surface incomplete: widget %p engine %p\n",
        (int)id, (void *)widget, (const void *)engine);
    return;
  }

  surface = canvas_surface_of_engine( canvas, engine );

  if( surface == NULL )
    surface = canvas_surface_of_engine( canvas, NULL );

  if( surface == NULL )
  {
    BUG("canvas %d holds %d surfaces already\n", (int)id, CANVAS_SURFACES_MAX);
    return;
  }

  surface->widget = widget;
  surface->engine = engine;

} /* canvas_add_surface() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_set_engine() - Present the surface of the named engine
 * @id:     canvas to switch
 * @engine: engine whose registered surface becomes the visible one
 *
 * Returns FALSE when the engine registered no surface for this canvas.
 */
  gboolean
canvas_set_engine(canvas_id_t id, const render_engine_ops_t *engine)
{
  canvas_t *canvas = &canvases[id];
  canvas_surface_t *target;
  int idx;

  if( engine == NULL )
  {
    BUG("canvas %d selected with no engine\n", (int)id);
    return FALSE;
  }

  target = canvas_surface_of_engine( canvas, engine );

  if( target == NULL )
    return FALSE;

  /* Exactly one registered surface is visible: the one whose engine
   * produces the canvas frames. */
  for( idx = 0; idx < CANVAS_SURFACES_MAX; idx++ )
    if( canvas->surfaces[idx].engine != NULL &&
        &canvas->surfaces[idx] != target )
      gtk_widget_hide( canvas->surfaces[idx].widget );

  gtk_widget_show( target->widget );

  /* Force aspect ratio update after showing the surface */
  gtk_widget_queue_resize( target->widget );

  canvas->active = target;

  return TRUE;

} /* canvas_set_engine() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_clear() - Drop every surface registered for a canvas
 * @id: canvas whose window is being destroyed
 */
  void
canvas_clear(canvas_id_t id)
{
  canvases[id] = (canvas_t){ 0 };

} /* canvas_clear() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_bound() - Report whether a canvas presents a surface
 * @id: canvas to test
 */
  gboolean
canvas_bound(canvas_id_t id)
{
  return( canvases[id].active != NULL );

} /* canvas_bound() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_of_view() - Project a view identity onto its canvas identity
 * @type: view whose scene the canvas presents
 */
  canvas_id_t
canvas_of_view(view_type_t type)
{
  return( view_canvas[type] );

} /* canvas_of_view() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_widget() - Return the surface a canvas presents
 * @id: canvas to resolve
 *
 * Reading the surface of an unbound canvas is a producer error: the caller
 * ran before the window was built or after it was destroyed.  Sites reached
 * across that lifetime test canvas_bound() first.
 */
  GtkWidget *
canvas_widget(canvas_id_t id)
{
  const canvas_t *canvas = &canvases[id];

  if( canvas->active == NULL )
  {
    BUG("canvas %d presents no surface\n", (int)id);
    return NULL;
  }

  return( canvas->active->widget );

} /* canvas_widget() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_invalidate() - Request a frame from a canvas immediately
 * @id: canvas to repaint
 *
 * Dispatches through the presenting engine, which owns the frame primitive
 * its surface needs.  No-op while the canvas is unbound.
 */
  void
canvas_invalidate(canvas_id_t id)
{
  const canvas_surface_t *active = canvases[id].active;

  if( active == NULL )
    return;

  active->engine->queue_redraw( active->widget );

} /* canvas_invalidate() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_queue_redraw() - Schedule a canvas frame through the optimizer gate
 * @id:    canvas to repaint
 * @force: bypass the intermediate-redraw suppression gate
 *
 * An unbound canvas has no frame to produce and marshals nothing.
 */
  void
canvas_queue_redraw(canvas_id_t id, gboolean force)
{
  if( !canvas_bound(id) )
    return;

  redraw_schedule( canvas_invalidate_cb, GINT_TO_POINTER((int)id), force );

} /* canvas_queue_redraw() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_fit_view() - Resolve fitted view state through the bound engine
 * @view: view whose drawn content defines the fit
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when the view's canvas is unbound or its engine has no
 * geometry to fit.
 */
  gboolean
canvas_fit_view(view_t *view, view_fit_t *fit)
{
  const canvas_surface_t *active;

  if( view == NULL || fit == NULL )
    return FALSE;

  active = canvases[ canvas_of_view(view->type) ].active;

  if( active == NULL )
    return FALSE;

  if( active->engine->fit_view == NULL )
    return FALSE;

  return( active->engine->fit_view(view, fit) );

} /* canvas_fit_view() */

/*-----------------------------------------------------------------------*/

/**
 * canvas_capture() - Capture a canvas frame through the bound engine
 * @id:     canvas to capture
 * @width:  capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated pixbuf, or NULL when the canvas is unbound or
 * its engine cannot capture the rendered frame.
 */
  GdkPixbuf *
canvas_capture(canvas_id_t id, int width, int height)
{
  const canvas_surface_t *active = canvases[id].active;

  if( active == NULL || width <= 0 || height <= 0 )
    return NULL;

  if( active->engine->capture == NULL )
    return NULL;

  return( active->engine->capture(active->widget, width, height) );

} /* canvas_capture() */

/*-----------------------------------------------------------------------*/
