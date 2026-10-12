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

/* Plot per-frequency average power gain from the shared measurement row. */

#include "../freqplots_internal.h"
#include "../../shared.h"

/* Reuse the average-gain trace across rendered frames. */
static double *avg_gain = NULL;

/**
 * fp_agt_enabled() - report whether the average-gain panel is selected
 *
 * Return: nonzero when the panel is selected and a radiation pattern exists.
 */
  int
fp_agt_enabled(void)
{
  return rc_config.freqplots_agt_togglebutton && isFlagSet(ENABLE_RDPAT);
}

/**
 * fp_agt_free() - release the average-gain trace
 */
  void
fp_agt_free( void )
{
  mem_array_free( &avg_gain );

} /* fp_agt_free() */

/**
 * fp_agt_render() - render the average-gain panel
 * @ctx: frame context carrying the per-frequency measurement rows
 *
 * The trace is omitted when any step reports the average unavailable, since
 * a NAN sample has no plot position.
 *
 * Return: TRUE after depositing the panel.
 */
  gboolean
fp_agt_render(fp_plot_ctx_t *ctx)
{
  char *titles[3];

  mem_array_realloc( &avg_gain, ctx->num_fsteps );

  fp_meas_column_t col = { avg_gain, MEAS_AGT };
  fp_fill_meas_columns( ctx, &col, 1 );

  /* Blank panel keeps the layout without a trace */
  double *trace = fp_trace_unavailable( avg_gain, ctx->num_fsteps, -1e30 ) ?
    NULL : avg_gain;

  titles[0] = _("Average gain");
  titles[1] = _("Average power gain over the sampled solid angle");
  titles[2] = "        ";

  fp_plot_panel( ctx, trace, NULL, titles, FP_PANEL_AGT );

  return TRUE;
}
