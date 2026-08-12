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
 * render_engine: one concrete renderer identity.
 *
 * The nested render protocol receives parent-prepared domain arguments.  The
 * remaining operations act on the active physical surface selected by canvas.
 * Canvas code dispatches only the surface operations and never invokes or
 * inspects the domain protocol.
 */

/* Named by reference alone; render_dispatch.h defines the domain protocol
 * and the argument types its operations carry. */
typedef struct render_ops_s render_ops_t;

typedef struct
{
  const render_ops_t *render;
  gboolean (*fit_view)(GtkWidget *surface, view_t *view, view_fit_t *fit);
  GdkPixbuf *(*capture)(GtkWidget *widget, int width, int height);

  /* Request a new frame from the engine's drawing widget.  A Cairo area
   * produces its frame during ::draw, while a GtkGLArea under manual render
   * mode re-presents its cached frame until a render request marks that
   * frame stale, so each engine names the primitive that suits it. */
  void (*queue_redraw)(GtkWidget *widget);

} render_engine_t;

#endif /* __RENDER_ENGINE_H */
