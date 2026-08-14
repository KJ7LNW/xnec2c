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
 * Owns the surface objects registered for each canvas, holds which of them
 * the canvas presents, and releases them when the binding drops.  Control
 * operations reach the engine leaf through the surface, so this layer names
 * no engine and reads no renderer setting.  Each leaf owns its geometry
 * reduction and zoom/pan inversion.
 */

#include "render_canvas.h"
#include "render_surface.h"
#include "render_canvas_lifetime.h"
#include "render_redraw.h"
#include "render_canvas_surface.h"
#include "../shared.h"

/* Engines able to draw into one canvas: a Cairo drawing area and a GL area. */
#define CANVAS_SURFACES_MAX 2

/* canvas_t - the surfaces registered for one canvas and the presented one. */
typedef struct
{
  render_surface_t *surfaces[CANVAS_SURFACES_MAX];  /* NULL names a free slot */
  render_surface_t *active;           /* NULL while the canvas is unbound */
  canvas_lifetime_t *lifetime;        /* token of the current binding */

} canvas_t;

/* The canvases.  Static for the process lifetime, so a marshaled frame
 * request names its canvas through the token of the binding that queued it. */
static canvas_t canvases[CANVAS_COUNT];

/* GSourceFunc wrapper for a canvas frame request, scheduled by
 * canvas_queue_redraw().  A canvas holding a token other than the one the
 * request carries has closed its window since, so the frame is dropped. */
  static gboolean
canvas_invalidate_cb(gpointer data)
{
  const canvas_lifetime_t *lifetime = data;
  canvas_id_t id = canvas_lifetime_id( lifetime );

  if( canvases[id].lifetime == lifetime )
    canvas_invalidate( id );

  return( G_SOURCE_REMOVE );

} /* canvas_invalidate_cb() */

/**
 * canvas_slot_of_engine() - Find the slot an engine registered its surface in
 * @canvas: canvas holding the surfaces
 * @engine: engine to match
 *
 * Returns NULL when the engine registered no surface.
 */
  static render_surface_t **
canvas_slot_of_engine(canvas_t *canvas, const render_engine_t *engine)
{
  render_surface_t **found = NULL;
  int idx = 0;

  while( found == NULL && idx < CANVAS_SURFACES_MAX )
  {
    if( canvas_surface_has_engine(canvas->surfaces[idx], engine) )
      found = &canvas->surfaces[idx];

    idx++;
  }

  return( found );

} /* canvas_slot_of_engine() */

/**
 * canvas_slot_free() - Find a slot holding no surface
 * @canvas: canvas holding the surfaces
 *
 * Returns NULL when every slot is occupied.
 */
  static render_surface_t **
canvas_slot_free(canvas_t *canvas)
{
  render_surface_t **found = NULL;
  int idx = 0;

  while( found == NULL && idx < CANVAS_SURFACES_MAX )
  {
    if( canvas->surfaces[idx] == NULL )
      found = &canvas->surfaces[idx];

    idx++;
  }

  return( found );

} /* canvas_slot_free() */

/**
 * canvas_holds_view() - Report whether a canvas registered a surface for a view
 * @canvas: canvas holding the surfaces
 * @view:   view to match
 */
  static gboolean
canvas_holds_view(const canvas_t *canvas, const view_t *view)
{
  gboolean found = FALSE;
  int idx = 0;

  while( !found && idx < CANVAS_SURFACES_MAX )
  {
    if( canvas->surfaces[idx] != NULL && canvas->surfaces[idx]->view == view )
      found = TRUE;

    idx++;
  }

  return( found );

} /* canvas_holds_view() */

  void
canvas_add_surface(canvas_id_t id, render_surface_t *surface)
{
  canvas_t *canvas = &canvases[id];
  render_surface_t **slot;

  if( surface == NULL || !canvas_surface_engine_complete(surface->engine) )
  {
    BUG("canvas %d needs a surface whose engine renders, frees, fits,"
        " captures and redraws: surface %p\n", (int)id, (void *)surface);
    return;
  }

  slot = canvas_slot_of_engine( canvas, surface->engine );

  /* The engine rebinding its slot releases the surface it registered
   * before, which the canvas has owned since that registration; a
   * presented surface hands its presentation to the replacement. */
  if( slot != NULL )
  {
    if( canvas->active == *slot )
      canvas->active = surface;

    canvas_surface_free( *slot );
  }
  else
    slot = canvas_slot_free( canvas );

  if( slot == NULL )
  {
    BUG("canvas %d holds %d surfaces already\n", (int)id, CANVAS_SURFACES_MAX);
    canvas_surface_free( surface );
    return;
  }

  *slot = surface;

  if( canvas->lifetime == NULL )
    canvas->lifetime = canvas_lifetime_new( id );

} /* canvas_add_surface() */

  canvas_id_t
canvas_create(render_surface_t *surface)
{
  canvas_id_t found = CANVAS_NONE;
  canvas_id_t id = CANVAS_RESERVED_COUNT;
  const render_engine_t *engine;

  if( surface == NULL )
  {
    BUG("canvas pool needs a surface to present\n");
    return CANVAS_NONE;
  }

  /* A pooled handle is free while it presents nothing: canvas_create()
   * presents at once and canvas_clear() drops the surfaces together. */
  while( found == CANVAS_NONE && id < CANVAS_COUNT )
  {
    if( !canvas_bound(id) )
      found = id;

    id++;
  }

  if( found == CANVAS_NONE )
  {
    BUG("canvas pool holds %d created canvases already\n", CANVAS_CREATED_MAX);
    canvas_surface_free( surface );
    return CANVAS_NONE;
  }

  /* Registration takes ownership, so name the engine before handing the
   * surface over and select through that name. */
  engine = surface->engine;

  canvas_add_surface( found, surface );
  canvas_set_engine( found, engine );

  return( found );

} /* canvas_create() */

  gboolean
canvas_set_engine(canvas_id_t id, const render_engine_t *engine)
{
  canvas_t *canvas = &canvases[id];
  render_surface_t *target;
  int idx;

  if( engine == NULL )
  {
    BUG("canvas %d selected with no engine\n", (int)id);
    return FALSE;
  }

  target = canvas_surface_of( id, engine );

  if( target == NULL )
    return FALSE;

  /* Present exactly the surface whose engine produces canvas frames. */
  for( idx = 0; idx < CANVAS_SURFACES_MAX; idx++ )
  {
    render_surface_t *surface = canvas->surfaces[idx];

    if( surface == NULL || surface == target )
      continue;

    canvas_surface_hide(surface);
  }

  canvas_surface_show(target);
  canvas->active = target;

  return TRUE;

} /* canvas_set_engine() */

  void
canvas_clear(canvas_id_t id)
{
  canvas_t *canvas = &canvases[id];
  canvas_lifetime_t *lifetime = canvas->lifetime;
  int idx;

  for( idx = 0; idx < CANVAS_SURFACES_MAX; idx++ )
    canvas_surface_free( canvas->surfaces[idx] );

  *canvas = (canvas_t){ 0 };

  /* Requests queued under the closing binding hold their own references */
  canvas_lifetime_release( lifetime );

} /* canvas_clear() */

  gboolean
canvas_bound(canvas_id_t id)
{
  return( canvases[id].active != NULL );

} /* canvas_bound() */

  render_surface_t *
canvas_surface_of(canvas_id_t id, const render_engine_t *engine)
{
  render_surface_t **slot = canvas_slot_of_engine( &canvases[id], engine );

  return( (slot != NULL) ? *slot : NULL );

} /* canvas_surface_of() */

  canvas_id_t
canvas_of_view(const view_t *view)
{
  canvas_id_t found = CANVAS_NONE;
  canvas_id_t id = CANVAS_STRUCTURE;

  if( view == NULL )
    return CANVAS_NONE;

  while( found == CANVAS_NONE && id < CANVAS_COUNT )
  {
    if( canvas_holds_view(&canvases[id], view) )
      found = id;

    id++;
  }

  return( found );

} /* canvas_of_view() */

  PangoLayout *
canvas_pango_layout(canvas_id_t id, const char *text)
{
  const render_surface_t *active = canvases[id].active;

  if( active == NULL )
  {
    BUG("canvas %d presents no surface\n", (int)id);
    return NULL;
  }

  return( canvas_surface_pango_layout(active, text) );

} /* canvas_pango_layout() */

/**
 * canvas_sync_viewport() - Record the presented surface allocation in its view
 * @id: canvas presenting the view
 *
 * Returns FALSE while the canvas is unbound or presents no view.
 */
  gboolean
canvas_sync_viewport(canvas_id_t id)
{
  render_surface_t *active = canvases[id].active;

  if( active == NULL || active->view == NULL )
    return FALSE;

  canvas_surface_sync_viewport(active);

  return TRUE;

} /* canvas_sync_viewport() */

  void
canvas_invalidate(canvas_id_t id)
{
  render_surface_t *active = canvases[id].active;

  if( active == NULL )
    return;

  canvas_surface_queue_redraw(active);

} /* canvas_invalidate() */

  void
canvas_queue_redraw(canvas_id_t id, gboolean force)
{
  if( !canvas_bound(id) )
    return;

  redraw_schedule( canvas_invalidate_cb,
      canvas_lifetime_acquire(canvases[id].lifetime),
      canvas_lifetime_release, force );

} /* canvas_queue_redraw() */

  gboolean
canvas_draw_sync(canvas_id_t id, int width, int height, cairo_t *cr)
{
  render_surface_t *active = canvases[id].active;

  if( active == NULL )
    return FALSE;

  return( canvas_surface_draw_sync(active, width, height, cr) );

} /* canvas_draw_sync() */

  gboolean
canvas_fit_view(view_t *view, view_fit_t *fit)
{
  canvas_id_t id;
  render_surface_t *active;

  if( view == NULL || fit == NULL )
    return FALSE;

  id = canvas_of_view( view );

  if( id == CANVAS_NONE )
    return FALSE;

  active = canvases[id].active;

  if( active == NULL )
    return FALSE;

  return( canvas_surface_fit(active, fit) );

} /* canvas_fit_view() */

  GdkPixbuf *
canvas_capture(canvas_id_t id, int width, int height)
{
  render_surface_t *active = canvases[id].active;

  if( active == NULL )
    return NULL;

  return( canvas_surface_capture(active, width, height) );

} /* canvas_capture() */

/*-----------------------------------------------------------------------*/
