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

/* freqplots_config_hooks: change-edge hooks of the frequency-plots panel
 * selections, with the hover classification each one carries. */

#include "../config_hooks.h"
#include "../shared.h"

/*------------------------------------------------------------------------*/

/* freqplots_recount_ngraph - derive the active-plot count from the live
 * rc_config select fields.
 *
 * calc_data.ngraph is a derived value; each select hook is called at most
 * once per real change and unconditionally at window-creation time via
 * config_widget_run_hooks(), so recomputing from the fields (rather than
 * incrementing/decrementing per call) keeps every hook idempotent.
 */
static void
freqplots_recount_ngraph(void)
{
  calc_data.ngraph = freqplots_count_selected();
}

/* hook_freqplots_redraw - request a plot frame when the plots hold results
 *
 * Shared body for every freqplots hook whose whole effect is a redraw.
 */
void
hook_freqplots_redraw(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

const config_refresh_t hook_freqplots_redraw_refresh =
  { .fn = hook_freqplots_redraw, .cls = REFRESH_HOVER_SAFE };

/* hook_freqplots_panel_select - refresh derived plot state after a select
 * toggle.
 *
 * Shared body for the eight freqplots panel select toggles.
 */
void
hook_freqplots_panel_select(void)
{
  freqplots_recount_ngraph();

  hook_freqplots_redraw();
}

/* Admitting or dropping a panel re-lays out every remaining plot, so the
 * selection reaches the window on its commit alone */
const config_refresh_t hook_freqplots_panel_select_refresh =
  { .fn = hook_freqplots_panel_select, .cls = REFRESH_COMMIT_ONLY };

void
hook_freqplots_net_gain(void)
{
  /* Net gain gates the gain and viewer popups' port pull-downs, so re-gate
   * every open port-aware combo when the setting flips. */
  freqplots_refresh_port_combos();

  hook_freqplots_redraw();
}

const config_refresh_t hook_freqplots_net_gain_refresh =
  { .fn = hook_freqplots_net_gain, .cls = REFRESH_HOVER_SAFE };
