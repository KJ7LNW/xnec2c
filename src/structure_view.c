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
 * structure_view: Structure view creation and change handling.
 *
 * Creates structure_view over the main window spin buttons and
 * refreshes the readouts and canvas it feeds when the view changes.
 */
#include "structure_view.h"
#include "structure_ui.h"
#include "shared.h"
#include "window_lifecycle.h"
#include "render/render_canvas.h"
#include "view/view_core.h"

/*-----------------------------------------------------------------------*/

/** structure_view_changed_cb() - view_t change callback for structure view
 * @v:           view that changed
 * @_user_data:  unused
 *
 * Invoked by view_notify_change() whenever rotation, pan, zoom, or extent
 * changes.  Refreshes the WR/WI spin display and viewer readout, then queues
 * a structure redraw and the view-dependent frequency plots.  The
 * frequency-step readouts are view-independent and stay untouched here.
 * Propagation to the rdpattern view under
 * common-projection sharing is handled internally by view_t via the
 * rotation_follower link established by view_share_master().  Bound as
 * the main window descriptor's changed_cb, which window_view_create()
 * passes to view_new().
 */
  void
structure_view_changed_cb(view_t *v, gpointer _user_data)
{
  /* The viewer-gain panel plots the gain in the view direction, so a view
   * change moves its trace */
  static const fp_panel_t view_panels[] = { FP_PANEL_VIEWER, FP_PANEL_COUNT };

  (void)_user_data;

  view_update_spin_display( v );
  structure_viewer_readout();
  canvas_queue_redraw(CANVAS_STRUCTURE, TRUE);
  freqplots_redraw_if_showing( view_panels );

} /* structure_view_changed_cb() */

/*-----------------------------------------------------------------------*/

/**
 * structure_view_create() - Create the structure view over its spin buttons
 *
 * Binds the main window rotate, incline and zoom spin buttons and the
 * frequency-step entry, creates structure_view through the main window
 * descriptor, and seeds its angles from the spins.
 */
  void
structure_view_create( void )
{
  /* Spin buttons and entry backing the structure view */
  window_view_spins_bind( MAIN_WINDOW );
  structure_fstep_entry = GTK_ENTRY(
      Builder_Get_Object(main_window_builder, "structure_fstep_entry") );

  /* Create the view over the spin buttons, then seed its angles from them */
  window_view_create( MAIN_WINDOW );
  view_set_angles( structure_view,
      gtk_spin_button_get_value( rotate_structure ),
      gtk_spin_button_get_value( incline_structure ) );
}

/*-----------------------------------------------------------------------*/
