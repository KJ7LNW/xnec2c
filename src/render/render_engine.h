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

#ifndef __RENDER_ENGINE_H
#define __RENDER_ENGINE_H 1

#include "../view/view_core.h"

/*
 * render_engine: engine control-op vtable, distinct from the per-frame
 * render_ops_t drawing vtable.  Control ops are one-shot capabilities outside
 * the draw path; fit-to-view is the first.  render_engine_active() selects the
 * active engine's vtable from rc_config.use_opengl_renderer.
 */

/* Engine control-op vtable: capabilities outside the per-frame draw path. */
typedef struct
{
  gboolean (*fit_view)(view_t *view, view_fit_t *fit);
  GdkPixbuf *(*capture)(GtkWidget *widget, int width, int height);

} render_engine_ops_t;

/**
 * render_fit_view() - Resolve fitted state through the active engine
 * @view: view whose drawn content defines the fit
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when the active engine has no geometry to fit.
 */
gboolean render_fit_view(view_t *view, view_fit_t *fit);

/**
 * render_capture_widget() - Capture the active engine frame
 * @widget: active engine drawing widget
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated pixbuf, or NULL when capture is unavailable.
 */
GdkPixbuf *render_capture_widget(GtkWidget *widget, int width, int height);

#endif /* __RENDER_ENGINE_H */
