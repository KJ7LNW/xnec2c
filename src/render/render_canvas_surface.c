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
 * canvas_surface: operations on one engine-owned drawing surface.
 *
 * Receives only a selected surface and operation-specific arguments.  Canvas
 * identity, candidate inventory, active selection, and pooled handles remain
 * private to render_canvas.c.  Every operation reaches the widget, the view
 * and the engine through the surface, so no consumer dereferences any of them.
 */

#include "render_canvas_surface.h"
#include "render_surface.h"

  gboolean
canvas_surface_engine_complete(const render_engine_t *engine)
{
  return( engine != NULL && engine->render != NULL &&
      engine->surface_free != NULL && engine->fit_view != NULL &&
      engine->capture != NULL && engine->queue_redraw != NULL );

} /* canvas_surface_engine_complete() */

  gboolean
canvas_surface_has_engine(const render_surface_t *surface,
    const render_engine_t *engine)
{
  return( surface != NULL && surface->engine == engine );

} /* canvas_surface_has_engine() */

  void
canvas_surface_free(render_surface_t *surface)
{
  if( surface == NULL )
    return;

  if( surface->engine == NULL || surface->engine->surface_free == NULL )
    return;

  surface->engine->surface_free( surface );

} /* canvas_surface_free() */

  void
canvas_surface_hide(const render_surface_t *surface)
{
  if( surface == NULL || surface->widget == NULL )
    return;

  gtk_widget_hide( surface->widget );

} /* canvas_surface_hide() */

  void
canvas_surface_show(const render_surface_t *surface)
{
  if( surface == NULL || surface->widget == NULL )
    return;

  gtk_widget_show( surface->widget );
  gtk_widget_queue_resize( surface->widget );

} /* canvas_surface_show() */

  void
canvas_surface_queue_redraw(render_surface_t *surface)
{
  if( surface == NULL || surface->engine == NULL
      || surface->engine->queue_redraw == NULL )
    return;

  surface->engine->queue_redraw( surface );

} /* canvas_surface_queue_redraw() */

/**
 * canvas_surface_sync_viewport() - Record the current surface allocation
 * @surface: active view-backed surface, which receives the allocation
 */
  void
canvas_surface_sync_viewport(render_surface_t *surface)
{
  GtkAllocation allocation;

  if( surface == NULL || surface->widget == NULL || surface->view == NULL )
    return;

  gtk_widget_get_allocation(surface->widget, &allocation);
  view_set_viewport(surface->view, allocation.width, allocation.height);

} /* canvas_surface_sync_viewport() */

  gboolean
canvas_surface_fit(render_surface_t *surface, view_fit_t *fit)
{
  if( surface == NULL || fit == NULL || surface->engine == NULL
      || surface->engine->fit_view == NULL )
    return FALSE;

  return( surface->engine->fit_view(surface, fit) );

} /* canvas_surface_fit() */

  GdkPixbuf *
canvas_surface_capture(render_surface_t *surface, int width, int height)
{
  if( width <= 0 || height <= 0 )
    return NULL;

  if( surface == NULL || surface->engine == NULL
      || surface->engine->capture == NULL )
    return NULL;

  return( surface->engine->capture(surface, width, height) );

} /* canvas_surface_capture() */

  PangoLayout *
canvas_surface_pango_layout(const render_surface_t *surface, const char *text)
{
  if( surface == NULL || surface->widget == NULL )
    return NULL;

  return( gtk_widget_create_pango_layout(surface->widget, text) );

} /* canvas_surface_pango_layout() */

  gboolean
canvas_surface_draw_sync(render_surface_t *surface,
    int width, int height, cairo_t *cr)
{
  GtkAllocation allocation = {0};

  if( width <= 0 || height <= 0 || cr == NULL )
    return FALSE;

  if( surface == NULL || surface->widget == NULL )
    return FALSE;

  allocation.width = width;
  allocation.height = height;
  gtk_widget_size_allocate(surface->widget, &allocation);
  canvas_surface_queue_redraw(surface);
  gtk_widget_draw(surface->widget, cr);

  return TRUE;

} /* canvas_surface_draw_sync() */

/*-----------------------------------------------------------------------*/
