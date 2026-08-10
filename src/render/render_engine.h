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
 * the draw path: fit-to-view, frame capture, and the frame request.  This
 * header declares the contract alone; each canvas names the vtable of the
 * engine producing its frames, so nothing here selects between engines.
 */

/* Engine control-op vtable: capabilities outside the per-frame draw path. */
typedef struct
{
  gboolean (*fit_view)(view_t *view, view_fit_t *fit);
  GdkPixbuf *(*capture)(GtkWidget *widget, int width, int height);

  /* Request a new frame from the engine's drawing widget.  A Cairo area
   * produces its frame during ::draw, while a GtkGLArea under manual render
   * mode re-presents its cached frame until a render request marks that
   * frame stale, so each engine names the primitive that suits it. */
  void (*queue_redraw)(GtkWidget *widget);

} render_engine_ops_t;

#endif /* __RENDER_ENGINE_H */
