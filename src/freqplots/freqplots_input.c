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
 * freqplots_input: Plots-window lifecycle and pointer interaction.
 *
 * Owns the deferred click event used to set the display frequency, the
 * window teardown path, and the mouse handling that moves the frequency
 * marker and resizes FR-card panels.
 */

#include "freqplots_internal.h"
#include "freqplots_locus.h"
#include "../shared.h"
#include "../gdk_scroll.h"

#include <math.h>
#include <string.h>

/* Per-notch pixel travel when resizing adjacent FR-card panels.  A wheel notch
 * moves one grain; a trackpad frame scales the grain by its delta magnitude. */
#define FP_RESIZE_GRAIN  20

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
  mem_free(&v->prev_click_event);
  if (v->text_layout != NULL)
    g_object_unref(v->text_layout);
  memset(v, 0, sizeof(*v));

} /* Plots_Window_Killed() */

/*-----------------------------------------------------------------------*/

void save_click_event(freqplots_view_t *v, GdkEvent *e)
{
	if (v->prev_click_event == NULL)
		mem_new(&v->prev_click_event);

	memcpy(v->prev_click_event, e, sizeof(GdkEvent));
}

GdkEvent *freqplots_pending_click(freqplots_view_t *v)
{
	return v->prev_click_event;
}


/* Set_Frequecy_On_Click()
 *
 * Sets the current freq after click by user on plots drawingarea
 */
  void
Set_Frequency_On_Click( freqplots_view_t *v, GdkEvent *e)
{
  double fmhz = 0.0;
  int set_fmhz = 0, draw_freqplot = 0;
  int button;
  scroll_step_t scroll = {0};

  GdkEventButton *button_event = (GdkEventButton *)e;
  GdkEventMotion *motion_event = (GdkEventMotion *)e;

  // fr_plot: the plot where the mouse hovered:
  fr_plot_t *fr_plot = NULL;

  // fr_adj: the plot to adjust when using the mouse wheel:
  fr_plot_t *fr_adj = NULL;

  if (isFlagSet(SY_OPTIMIZER_ACTIVE))
	  return;

  if (v->fr_plots == NULL)
  {
	save_click_event(v, e);
	return;
  }

  // find the fr_plot structure that the mouse is within:
  fr_plot = fr_plot_at(v, button_event->x, button_event->y);

  // Decode the button, scroll direction, or button-drag state.
  button = 0;

  if (e->type == GDK_SCROLL)
	  scroll = scroll_step_from_deltas(e);

  if (e->type == GDK_BUTTON_PRESS)
	  button = button_event->button;
  else if (scroll.active &&
	  scroll.direction == GDK_SCROLL_UP)
	  button = 4;
  else if (scroll.active &&
	  scroll.direction == GDK_SCROLL_DOWN)
	  button = 5;
  else if (e->type == GDK_MOTION_NOTIFY)
  {
  // Support holding the button down to drag the green line:
  if (motion_event->state & GDK_BUTTON1_MASK) button = 1;
	  else if (motion_event->state & GDK_BUTTON2_MASK) button = 2;
	  else if (motion_event->state & GDK_BUTTON3_MASK) button = 3;
  }

  /* Swap primary and secondary click actions when configured so a left
   * click snaps to a calculated frequency and a right click selects an
   * arbitrary frequency. */
  if (rc_config.freqplots_swap_click)
  {
    if (button == 1) button = 3;
    else if (button == 3) button = 1;
  }

  // Mouse-wheel scroll over an FR-card panel resizes panel widths.  This is
  // panel geometry, not frequency resolution, so it stays on the fr_plot
  // layout table rather than the locus registry.
  if (button == 4 || button == 5)
  {
    if (fr_plot == NULL || !FR_PLOT_T_IS_VALID(fr_plot))
    {
      save_click_event(v, e);
      return;
    }

    // If its the last plot than shrink/grow the previous:
    if (fr_plot->fr == calc_data.FR_cards-1)
      fr_adj = get_fr_plot(v, fr_plot->posn, fr_plot->fr-1);

    // Otherwise shink/grow the next:
    else
      fr_adj = get_fr_plot(v, fr_plot->posn, fr_plot->fr+1);

    // Abort if there is only one plot:
    if (fr_adj == NULL)
      return;

    // Continuous pixel travel: a wheel notch (step near 1.0) moves one grain;
    // a trackpad frame scales the grain by its delta magnitude, floored at one
    // grain so an active frame always resizes.
    int px_adjust = (int)round(fmax(1.0, scroll.step) * FP_RESIZE_GRAIN);

    // scroll up
    if (button == 4)
    {
      if (fr_adj->plot_rect.width < 100)
        return;

      fr_adj->plot_rect.width -= px_adjust;
      fr_plot->plot_rect.width += px_adjust;
    }
    // scroll down
    else
    {
      if (fr_plot->plot_rect.width < 100)
        return;

      fr_adj->plot_rect.width += px_adjust;
      fr_plot->plot_rect.width -= px_adjust;
    }

    // Sync widths for all positions based on fr_plot:
    fr_plot_sync_widths(v, fr_plot);

    draw_freqplot = 1;
  }
  // Primary, secondary, and drag clicks resolve to a frequency through the
  // shared locus registry, which serves every plot family uniformly: a
  // primary click lerps between bracketing samples; a secondary click snaps
  // to the nearest sample, including the green-line extra slot once present.
  else if (button == 1 || button == 2 || button == 3)
  {
    if (!fp_locus_freq_at_pixel(v, button_event->x, button_event->y,
            button == 3, &fmhz))
    {
      // No panel under the click; defer until this view has geometry.
      save_click_event(v, e);
      return;
    }

    if (button == 2)
    {
      // Disable drawing of the freq line.
      calc_data.fmhz_save = 0.0;
      draw_freqplot = 1;
    }
    else
    {
      draw_freqplot = 1;
      set_fmhz = 1;
      if (button == 1)
        freqplots_redraw_all(TRUE);
    }
  }
  else
  {
    // No actionable button under the pointer, e.g. a bare hover.
    return;
  }

  if (set_fmhz)
  {
	  /* Round frequency to nearest 1 Hz */
	  uint64_t ifmhz = ( fmhz * 1e6 + 0.5 );
	  fmhz = ifmhz / 1e6;

	  user_set_frequency(fmhz);
  }

  // If prev_click_event is set then we are being called from Plot_Frequency_Data()
  // to update fmhz so don't redraw because Plot_Frequency_Data() is going to draw
  // after we return.
  if (draw_freqplot && v->prev_click_event == NULL)
    freqplots_redraw_all(TRUE);

  // Free the pending click since it was serviced.
  if (v->prev_click_event != NULL)
	  mem_free(&v->prev_click_event);

} /* Set_Freq_On_Click() */

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
