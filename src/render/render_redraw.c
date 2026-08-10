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
 * render_redraw: the optimizer suppression gate and the main-thread marshal
 * shared by every repaint request, canvas frame requests among them.
 */

#include "render_redraw.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/**
 * redraw_suppressed() - Report whether the optimizer gate holds this redraw
 * @force: TRUE for user-interaction draws that must always render
 *
 * Intermediate frequency-sweep draws are dropped while an optimizer run
 * owns the frequency loop, so the display does not flicker through every
 * evaluated candidate.
 */
  static gboolean
redraw_suppressed(gboolean force)
{
  gboolean suppressed;

  suppressed = !force &&
      isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) &&
      isFlagSet(FREQ_LOOP_RUNNING);

  if( suppressed )
    pr_debug("Optimizer loop incomplete, suppressing intermediate redraw.\n");

  return( suppressed );

} /* redraw_suppressed() */

/*-----------------------------------------------------------------------*/

/* GSourceFunc wrapper for a plain widget repaint, scheduled by
 * xnec2_widget_queue_draw(). */
  static gboolean
queue_draw_cb(gpointer w)
{
  gtk_widget_queue_draw( GTK_WIDGET(w) );

  return( G_SOURCE_REMOVE );

} /* queue_draw_cb() */

/*-----------------------------------------------------------------------*/

/**
 * redraw_schedule() - Schedule repaint work through the optimizer gate
 * @callback: repaint work, run once on the GTK main thread
 * @data:     argument handed to @callback, which must outlive the marshal
 * @force:    bypass the intermediate-redraw suppression gate
 */
  void
redraw_schedule(GSourceFunc callback, gpointer data, gboolean force)
{
  if( redraw_suppressed(force) )
    return;

  g_main_context_invoke_full( NULL, GDK_PRIORITY_REDRAW,
      callback, data, NULL );

} /* redraw_schedule() */

/*-----------------------------------------------------------------------*/

/**
 * xnec2_widget_queue_draw() - Schedule a widget repaint through the gate
 * @w:     widget presented by no canvas
 * @force: bypass the intermediate-redraw suppression gate
 *
 * Serves the colorcode legend strips and the animate dialog.  The
 * opengl-engine/ view layer requests its own frames
 * through gl_view_queue_render() for engine-internal reasons (MSAA rebuild,
 * input event repaints, notice fade) that sit below this gate's abstraction
 * level.
 */
  void
xnec2_widget_queue_draw(GtkWidget *w, gboolean force)
{
  redraw_schedule( queue_draw_cb, w, force );

} /* xnec2_widget_queue_draw() */

/*-----------------------------------------------------------------------*/
