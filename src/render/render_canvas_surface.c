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
 * canvas_surface: physical widget and concrete engine binding operations.
 *
 * Receives only a selected binding and operation-specific arguments.  Canvas
 * identity, candidate inventory, active selection, and pooled handles remain
 * private to render_canvas.c.  Every operation reaches the widget and the
 * engine through the binding, so no consumer dereferences either.
 */

#include "render_canvas_surface.h"

  gboolean
canvas_surface_engine_complete(const render_engine_t *engine)
{
  return( engine != NULL && engine->fit_view != NULL &&
      engine->capture != NULL && engine->queue_redraw != NULL );

} /* canvas_surface_engine_complete() */

  gboolean
canvas_surface_has_engine(const canvas_surface_t *surface,
    const render_engine_t *engine)
{
  return( surface->engine == engine );

} /* canvas_surface_has_engine() */

  void
canvas_surface_bind(canvas_surface_t *surface, GtkWidget *widget,
    const render_engine_t *engine)
{
  surface->widget = widget;
  surface->engine = engine;

} /* canvas_surface_bind() */

  void
canvas_surface_hide(const canvas_surface_t *surface)
{
  gtk_widget_hide( surface->widget );

} /* canvas_surface_hide() */

  void
canvas_surface_show(const canvas_surface_t *surface)
{
  gtk_widget_show( surface->widget );
  gtk_widget_queue_resize( surface->widget );

} /* canvas_surface_show() */

  void
canvas_surface_queue_redraw(const canvas_surface_t *surface)
{
  surface->engine->queue_redraw( surface->widget );

} /* canvas_surface_queue_redraw() */

/**
 * canvas_surface_sync_viewport() - Record the current surface allocation
 * @surface: active view-backed surface
 * @view:    renderer-neutral view state receiving the allocation
 */
  void
canvas_surface_sync_viewport(const canvas_surface_t *surface, view_t *view)
{
  GtkAllocation allocation;

  gtk_widget_get_allocation(surface->widget, &allocation);
  view_set_viewport(view, allocation.width, allocation.height);

} /* canvas_surface_sync_viewport() */

  gboolean
canvas_surface_fit(const canvas_surface_t *surface, view_t *view,
    view_fit_t *fit)
{
  return( surface->engine->fit_view(surface->widget, view, fit) );

} /* canvas_surface_fit() */

  GdkPixbuf *
canvas_surface_capture(const canvas_surface_t *surface, int width, int height)
{
  if( width <= 0 || height <= 0 )
    return NULL;

  return( surface->engine->capture(surface->widget, width, height) );

} /* canvas_surface_capture() */

  PangoLayout *
canvas_surface_pango_layout(const canvas_surface_t *surface, const char *text)
{
  return( gtk_widget_create_pango_layout(surface->widget, text) );

} /* canvas_surface_pango_layout() */

  gboolean
canvas_surface_draw_sync(const canvas_surface_t *surface,
    int width, int height, cairo_t *cr)
{
  GtkAllocation allocation = {0};

  if( width <= 0 || height <= 0 || cr == NULL )
    return FALSE;

  allocation.width = width;
  allocation.height = height;
  gtk_widget_size_allocate(surface->widget, &allocation);
  canvas_surface_queue_redraw(surface);
  gtk_widget_draw(surface->widget, cr);

  return TRUE;

} /* canvas_surface_draw_sync() */

/*-----------------------------------------------------------------------*/
