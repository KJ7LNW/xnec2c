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

#ifndef __RENDER_REDRAW_H
#define __RENDER_REDRAW_H 1

#include <gtk/gtk.h>

/*
 * render_redraw: repaint scheduling.
 *
 * Owns the optimizer suppression gate and the main-thread marshal that
 * every repaint request passes through, whichever surface it targets.
 * Widgets that a canvas presents are repainted through the canvas layer,
 * which schedules its frame request here; widgets that belong to no canvas
 * take xnec2_widget_queue_draw().
 */

/**
 * redraw_schedule() - Schedule repaint work through the optimizer gate
 * @callback: repaint work, run once on the GTK main thread
 * @data:     argument handed to @callback, which must outlive the marshal
 * @destroy:  releases owned @data once the request retires, or NULL when
 *            the caller lends data it owns for longer than the request
 * @force:    TRUE for user-interaction draws that must always render;
 *            FALSE for intermediate frequency-sweep draws that are
 *            suppressed during optimizer runs to avoid screen flicker
 *
 * Main-thread callers run @callback synchronously, while callers on any
 * other thread marshal it at GDK_PRIORITY_REDRAW, as the GTK drawing calls
 * are confined to the main thread.  A suppressed request retires at once,
 * so owned data is released on every outcome.
 */
void redraw_schedule(GSourceFunc callback, gpointer data,
    GDestroyNotify destroy, gboolean force);

/**
 * xnec2_widget_queue_draw() - Schedule a widget repaint through the gate
 * @w:     widget presented by no canvas
 * @force: TRUE for user-interaction draws that must always render;
 *         FALSE for intermediate frequency-sweep draws that are
 *         suppressed during optimizer runs to avoid screen flicker
 *
 * Serves the colorcode legend strips and the animate dialog.  A canvas
 * surface takes canvas_queue_redraw() instead, so
 * that a GL area receives a render request rather than a draw of the frame
 * it already holds.
 */
void xnec2_widget_queue_draw(GtkWidget *w, gboolean force);

#endif /* __RENDER_REDRAW_H */
