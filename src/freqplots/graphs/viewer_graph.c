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
 * viewer_graph: Viewer-direction gain plot type.
 *
 * Plots gain in the structure-viewer direction against frequency.  Both the
 * viewer gain and the optional net-gain trace come from the shared per-frame
 * measurement pass, which evaluates Viewer_Gain() once for the active view.
 */

#include "../freqplots_internal.h"
#include "../../shared.h"

  int
fp_viewer_enabled(void)
{
  return rc_config.freqplots_gviewer_togglebutton && isFlagSet(ENABLE_RDPAT);
}

/* Viewer-direction gain trace buffers, reused across fp_viewer_render() calls. */
static double *vgain = NULL, *netgain = NULL;

/* fp_viewer_free()
 *
 * Releases the viewer-direction gain trace buffers.
 */
  void
fp_viewer_free( void )
{
  mem_array_free( &vgain );
  mem_array_free( &netgain );

} /* fp_viewer_free() */

  gboolean
fp_viewer_render(fp_plot_ctx_t *ctx)
{
  char *titles[3];

  /* Plotting frame titles */
  titles[0] = _("Raw Gain dBi");
  titles[1] = _("Gain in Viewer Direction vs Frequency");

  /* Viewer-direction gain comes from the shared per-frame measurement pass;
   * meas_calc already evaluated Viewer_Gain() for the active view. */
  mem_array_realloc(&vgain, ctx->num_fsteps);
  fp_meas_column_t vcol = { vgain, MEAS_GAIN_VIEWER };
  fp_fill_meas_columns( ctx, &vcol, 1 );

  /* Plot net gain if selected and a feedpoint defines mismatch loss */
  if( fp_net_gain_shown() )
  {
    mem_array_realloc(&netgain, ctx->num_fsteps);

    fp_meas_column_t col = { netgain, MEAS_GAIN_VIEWER_NET };
    fp_fill_meas_columns( ctx, &col, 1 );

    titles[2] = _("Net gain dBi");
    fp_plot_panel(ctx, vgain, netgain, titles, FP_PANEL_VIEWER);
  }
  else
  {
    titles[2] = "        ";
    fp_plot_panel(ctx, vgain, NULL, titles, FP_PANEL_VIEWER);
  }

  return TRUE;
}
