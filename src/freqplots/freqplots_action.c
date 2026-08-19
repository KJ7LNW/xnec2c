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
 * freqplots_action: Leaf actions a decoded pointer gesture reaches.
 *
 * Each leaf owns one domain and shares no branch with the other: the
 * frequency marker resolves through the locus registry, and FR-card panel
 * widths resolve through the plot layout table.  Both receive coordinates
 * already resolved, read no GDK event, and report an outcome the router acts
 * on; neither writes the view's deferred-gesture slot.
 */

#include "freqplots_internal.h"
#include "freqplots_locus.h"
#include "../shared.h"

#include <math.h>

/* Per-notch pixel travel when resizing adjacent FR-card panels.  A wheel notch
 * moves one grain; a trackpad frame scales the grain by its delta magnitude. */
#define FP_RESIZE_GRAIN  20

/* Narrowest an FR-card panel may become before a resize is refused. */
#define FP_RESIZE_MIN_WIDTH  100

/* fp_freq_place()
 *
 * Moves or clears the selected-frequency marker.  Every frequency act
 * resolves its pixel through the shared locus registry first, which serves
 * each plot family uniformly, so a frame landing on no locus parks whichever
 * act it carries.  A lerping act interpolates between bracketing samples; a
 * snapping act takes the nearest computed sample.
 */
  fp_leaf_t
fp_freq_place( freqplots_view_t *v, const fp_pointer_t *p )
{
  double fmhz = 0.0;

  if( !fp_locus_freq_at_pixel( v, p->x, p->y,
        p->act == FP_ACT_FREQ_SNAP, &fmhz ) )
    return( FP_LEAF_DEFER );

  if( p->act == FP_ACT_FREQ_CLEAR )
  {
    // Drop the marker by clearing the frequency the plots draw it at.
    calc_data.fmhz_save = 0.0;
    return( FP_LEAF_DIRTY );
  }

  /* A lerped marker tracks the pointer through a drag, so its frame is issued
   * ahead of the frequency change rather than waiting on it. */
  if( p->act == FP_ACT_FREQ_LERP )
    freqplots_redraw_all( TRUE );

  /* Round the resolved frequency to the nearest 1 Hz. */
  uint64_t ifmhz = ( fmhz * 1e6 + 0.5 );

  user_set_frequency( ifmhz / 1e6 );

  return( FP_LEAF_DIRTY );

} /* fp_freq_place() */

/*-----------------------------------------------------------------------*/

/* fp_panel_resize()
 *
 * Trades width between the hovered FR-card panel and one neighbour, then
 * propagates the pair across every position.  This is panel geometry rather
 * than frequency resolution, so it reads the fr_plot layout table instead of
 * the locus registry.
 */
  fp_leaf_t
fp_panel_resize( freqplots_view_t *v, const fp_pointer_t *p )
{
  fr_plot_t *fr_plot = fr_plot_at( v, p->x, p->y );
  fr_plot_t *fr_adj;
  int px_adjust;

  if( fr_plot == NULL || !FR_PLOT_T_IS_VALID(fr_plot) )
    return( FP_LEAF_DEFER );

  /* The last card has no successor, so it trades with its predecessor. */
  if( fr_plot->fr == calc_data.FR_cards - 1 )
    fr_adj = get_fr_plot( v, fr_plot->posn, fr_plot->fr - 1 );
  else
    fr_adj = get_fr_plot( v, fr_plot->posn, fr_plot->fr + 1 );

  /* A lone card has no partner to trade width with. */
  if( fr_adj == NULL )
    return( FP_LEAF_IDLE );

  /* Continuous pixel travel: a wheel notch (step near 1.0) moves one grain;
   * a trackpad frame scales the grain by its delta magnitude, floored at one
   * grain so an active frame always resizes. */
  px_adjust = (int)round( fmax(1.0, p->step) * FP_RESIZE_GRAIN );

  if( p->act == FP_ACT_PANEL_GROW )
  {
    // Growing draws width from the neighbour, so the neighbour bounds it.
    if( fr_adj->plot_rect.width < FP_RESIZE_MIN_WIDTH )
      return( FP_LEAF_IDLE );

    fr_adj->plot_rect.width  -= px_adjust;
    fr_plot->plot_rect.width += px_adjust;
  }
  else
  {
    // Shrinking gives width away, so the hovered panel bounds it.
    if( fr_plot->plot_rect.width < FP_RESIZE_MIN_WIDTH )
      return( FP_LEAF_IDLE );

    fr_adj->plot_rect.width  += px_adjust;
    fr_plot->plot_rect.width -= px_adjust;
  }

  // Sync widths for all positions based on fr_plot:
  fr_plot_sync_widths( v, fr_plot );

  return( FP_LEAF_DIRTY );

} /* fp_panel_resize() */
