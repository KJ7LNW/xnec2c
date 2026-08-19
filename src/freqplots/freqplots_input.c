/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

/*
 * freqplots_input: Plots-window lifecycle and pointer dispatch.
 *
 * Owns the window teardown path, the boundary decode turning a GDK frame into
 * the action it performs, and the router carrying that action to the leaf
 * owning it.  The leaves themselves live in freqplots_action.c.
 */

#include "freqplots_internal.h"
#include "freqplots_locus.h"
#include "../shared.h"
#include "../gdk_scroll.h"

#include <string.h>

/* Plots_Window_Killed()
 *
 * Cleans up after the plots window is closed
 */
  void
Plots_Window_Killed( void )
{
  freqplots_view_t *v = freqplots_main_view();

  // Close every popup before tearing down the primary view so no popup
  // references freed primary state.
  freqplots_destroy_all_popups();

  if( isFlagSet(PLOT_ENABLED) )
  {
    ClearFlag( PLOT_ENABLED );
    g_object_unref( freqplots_window_builder );
    freqplots_window_builder = NULL;

    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object(main_window_builder, "main_freqplots")), FALSE );
  }
  freqplots_window = NULL;
  canvas_clear( CANVAS_FREQPLOTS );
  kill_window = NULL;

  // Release heap tables, then zero the whole view so the resize caches
  // (prev_*) restart and width_available rescales on the next open.
  mem_array_free(&v->fr_plots);
  fp_locus_free(v);
  if (v->text_layout != NULL)
    g_object_unref(v->text_layout);
  memset(v, 0, sizeof(*v));

} /* Plots_Window_Killed() */

/*-----------------------------------------------------------------------*/

/* fp_pointer_from_event()
 *
 * Resolves one GDK frame to the action it performs on the plots surface.
 * Touches no view and reads rc_config alone, so a frame decodes identically
 * for every window showing the plots.  Every field arrives through a GDK
 * accessor, so a frame carrying none leaves the initialized value in place.
 */
  fp_pointer_t
fp_pointer_from_event( GdkEvent *e )
{
  fp_pointer_t p = {
    .act  = FP_ACT_NONE,
    .step = 1.0,
  };
  int button = 0;

  gdk_event_get_coords( e, &p.x, &p.y );

  /* A wheel notch and a trackpad frame both name a panel resize, and the
   * frame carries its own magnitude, so the scroll decode ends here. */
  if( e->type == GDK_SCROLL )
  {
    scroll_step_t s = scroll_step_from_deltas( e );

    p.step = s.step;

    if( s.active && s.direction == GDK_SCROLL_UP )
      p.act = FP_ACT_PANEL_GROW;
    else if( s.active && s.direction == GDK_SCROLL_DOWN )
      p.act = FP_ACT_PANEL_SHRINK;
    else
      p.act = FP_ACT_NONE;

    return( p );
  }

  if( e->type == GDK_BUTTON_PRESS )
  {
    guint pressed = 0;

    gdk_event_get_button( e, &pressed );
    button = (int)pressed;
  }
  else if( e->type == GDK_MOTION_NOTIFY )
  {
    /* Holding a button down drags the marker, so a held mask names the same
     * action its press named. */
    GdkModifierType state = 0;

    gdk_event_get_state( e, &state );

    if( (state & GDK_BUTTON1_MASK) != 0 )
      button = 1;
    else if( (state & GDK_BUTTON2_MASK) != 0 )
      button = 2;
    else if( (state & GDK_BUTTON3_MASK) != 0 )
      button = 3;
    else
      button = 0;
  }
  else
    button = 0;

  /* Swapping trades which button lerps and which snaps.  It states a
   * frequency preference, so it is applied before the acts are named and
   * leaves the panel acts untouched. */
  if( rc_config.freqplots_swap_click )
  {
    if( button == 1 )
      button = 3;
    else if( button == 3 )
      button = 1;
  }

  if( button == 1 )
    p.act = FP_ACT_FREQ_LERP;
  else if( button == 2 )
    p.act = FP_ACT_FREQ_CLEAR;
  else if( button == 3 )
    p.act = FP_ACT_FREQ_SNAP;
  else
    p.act = FP_ACT_NONE;

  return( p );

} /* fp_pointer_from_event() */

/*-----------------------------------------------------------------------*/

/* fp_act_apply()
 *
 * Dispatches one decoded gesture to the domain owning it, parks the gesture
 * when this view carries no layout table to resolve it against, and retires
 * the parked slot once a gesture is serviced.  The slot belongs to this
 * router alone, so the leaves report an outcome and never write it.  Returns
 * TRUE when the surface owes a frame.
 */
  gboolean
fp_act_apply( freqplots_view_t *v, const fp_pointer_t *p )
{
  fp_leaf_t r = FP_LEAF_IDLE;

  /* An optimizer sweep redraws on its own cadence, and servicing pointer
   * frames on top of it makes the plots choppy.  A parked gesture waits
   * through the sweep rather than being spent against it. */
  if( isFlagSet(SY_OPTIMIZER_ACTIVE) )
    return( FALSE );

  /* A view carrying no layout table resolves no pixel, so every frame parks
   * here, one naming no act included: the newest frame states the pointer's
   * present intent and supersedes what the single slot held. */
  if( v->fr_plots == NULL )
  {
    v->pending = *p;
    return( FALSE );
  }

  if( p->act == FP_ACT_NONE )
    return( FALSE );

  switch( p->act )
  {
    case FP_ACT_FREQ_LERP:
    case FP_ACT_FREQ_SNAP:
    case FP_ACT_FREQ_CLEAR:
      r = fp_freq_place( v, p );
      break;

    case FP_ACT_PANEL_GROW:
    case FP_ACT_PANEL_SHRINK:
      r = fp_panel_resize( v, p );
      break;

    case FP_ACT_NONE:
    case FP_ACT_COUNT:
      BUG("pointer action reached dispatch with no leaf to serve it");
      break;
  }

  switch( r )
  {
    case FP_LEAF_DEFER:
      v->pending = *p;
      break;

    case FP_LEAF_DIRTY:
      // Servicing the gesture retires whatever the slot was holding.
      v->pending.act = FP_ACT_NONE;
      break;

    case FP_LEAF_IDLE:
      // A bound refused the gesture, so a standing park outlives it.
      break;
  }

  return( r == FP_LEAF_DIRTY );

} /* fp_act_apply() */

/*-----------------------------------------------------------------------*/

/* freqplots_pointer_input()
 *
 * Entry for the plots drawing-area pointer signals: decode the frame, apply
 * it, and issue the surface's frame when the applied gesture changed state.
 */
  void
freqplots_pointer_input( freqplots_view_t *v, GdkEvent *e )
{
  fp_pointer_t p = fp_pointer_from_event( e );

  if( fp_act_apply( v, &p ) )
    freqplots_redraw_all( TRUE );

} /* freqplots_pointer_input() */

/*-----------------------------------------------------------------------*/

/* freqplots_pointer_replay()
 *
 * Applies the gesture parked while this view lacked geometry.  The gesture
 * travels by value because the router writes the slot it came from, and one
 * still finding no geometry parks afresh rather than compounding.  The draw
 * path calls this and draws on return, which is why the dirty report is not
 * acted on here.
 */
  void
freqplots_pointer_replay( freqplots_view_t *v )
{
  fp_pointer_t p = v->pending;

  if( p.act == FP_ACT_NONE )
    return;

  fp_act_apply( v, &p );

} /* freqplots_pointer_replay() */

/*-----------------------------------------------------------------------*/

/* Resolve a pixel to the graph type beneath it for double-click popout.
 * Returns the panel type, or FP_PANEL_ALL when no graph is hit.  The Smith
 * chart registers no plot rows, so it is matched through its locus rect. */
  fp_panel_t
freqplots_panel_at( freqplots_view_t *v, double px, double py )
{
  fr_plot_t *p = fr_plot_at( v, px, py );

  if( p != NULL )
    return p->panel_type;

  return fp_locus_panel_at( v, px, py );
}
