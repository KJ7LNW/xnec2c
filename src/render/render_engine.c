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
 * render_engine: engine control-op dispatch.
 *
 * Selects the active engine's control-op vtable from the renderer setting and
 * dispatches control operations to the engine leaf.  Each leaf owns its
 * geometry reduction and zoom/pan inversion; this layer owns selection.
 */

#include "render_engine.h"
#include "../cairo/cairo_fit.h"
#include "../shared.h"
#ifdef HAVE_OPENGL
#include "../opengl-engine/opengl_view_fit.h"
#endif

/*-----------------------------------------------------------------------*/

/** render_engine_active() - Return the active engine capability table */
  static const render_engine_ops_t *
render_engine_active(void)
{
#ifdef HAVE_OPENGL
  if( rc_config.use_opengl_renderer )
    return( &gl_engine_ops );
#endif

  return( &cairo_engine_ops );

} /* render_engine_active() */

/*-----------------------------------------------------------------------*/

/**
 * render_fit_view() - Resolve fitted view state through the active engine
 * @view: view whose drawn content defines the fit
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when the active engine has no geometry to fit.
 */
  gboolean
render_fit_view(view_t *view, view_fit_t *fit)
{
  const render_engine_ops_t *ops;

  if( view == NULL || fit == NULL )
    return FALSE;

  ops = render_engine_active();

  if( ops->fit_view == NULL )
    return FALSE;

  return( ops->fit_view(view, fit) );

} /* render_fit_view() */

/*-----------------------------------------------------------------------*/

/** render_capture_widget() - Capture the active engine frame
 * @widget: active engine drawing widget
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated pixbuf, or NULL when the active engine cannot
 * capture the widget's rendered frame.
 */
  GdkPixbuf *
render_capture_widget(GtkWidget *widget, int width, int height)
{
  const render_engine_ops_t *ops;

  if( widget == NULL || width <= 0 || height <= 0 )
    return NULL;

  ops = render_engine_active();

  if( ops->capture == NULL )
    return NULL;

  return ops->capture(widget, width, height);

} /* render_capture_widget() */

/*-----------------------------------------------------------------------*/
