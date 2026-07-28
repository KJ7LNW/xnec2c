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
 * agt_graph: Average Gain Test plot type.
 *
 * Plots the AGT linear ratio (~1.0 free space / ~2.0 over perfect ground
 * for a lossless model) alongside AGT efficiency %, across every swept
 * frequency point. Values come from the shared meas_calc() column source
 * and report the -1 sentinel (via MEAS_AGT/MEAS_AGT_EFFICIENCY) whenever
 * the RP card wasn't set up with averaging (A=1 or A=2) for that step.
 */

#include "../freqplots_internal.h"
#include "../../shared.h"

  int
fp_agt_enabled(void)
{
  /* Gate on the RP card's OWN A field (A=1 or A=2), as the user actually
   * wrote it -- not fpat.iavp, which xnec2c auto-enables by default for
   * its own kernel-accuracy diagnostic regardless of what the RP card
   * says (see input.c). AGT values/CSV columns stay populated either
   * way since that diagnostic is useful info on its own; this only
   * controls whether the dedicated AGT tab is considered "on". */
  return rc_config.freqplots_agt_togglebutton
      && isFlagSet(ENABLE_RDPAT)
      && fpat.iavp_requested;
}

/* AGT plot trace buffers, reused across fp_agt_render() calls. */
static double *agt = NULL, *agt_eff = NULL;

/* fp_agt_free()
 *
 * Releases the AGT plot trace buffers.
 */
  void
fp_agt_free( void )
{
  mem_array_free( &agt );
  mem_array_free( &agt_eff );

} /* fp_agt_free() */

  gboolean
fp_agt_render(fp_plot_ctx_t *ctx)
{
  char *titles[3];

  mem_array_realloc(&agt, ctx->num_fsteps);
  mem_array_realloc(&agt_eff, ctx->num_fsteps);

  fp_meas_column_t cols[2];
  int ncols = 0;
  cols[ncols++] = (fp_meas_column_t){ agt,     MEAS_AGT            };
  cols[ncols++] = (fp_meas_column_t){ agt_eff, MEAS_AGT_EFFICIENCY };
  fp_fill_meas_columns( ctx, cols, ncols );

  titles[0] = _("Average Gain");
  titles[1] = _("Average Gain Test vs Frequency");
  titles[2] = _("AGT Efficiency %");
  fp_plot_panel(ctx, agt, agt_eff, titles, FP_PANEL_AGT);

  return TRUE;
}
