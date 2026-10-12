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
 * gain_graph: Max-gain plot type.
 *
 * Plots raw max gain against frequency, paired with F/B ratio, F/R ratio, or
 * net gain, and optionally the max-gain direction.  All per-frequency values
 * come from the shared meas_calc() column source.
 */

#include "../freqplots_internal.h"
#include "../../shared.h"

/* Gain-panel second trace per selector outcome: its measurement, the bound
 * below which a step reads unavailable, and its axis and panel titles. */
typedef struct
{
  int         meas_idx;
  double      invalid_min;
  const char *axis_title;
  const char *panel_title;
} fp_gain_trace_row_t;

static const fp_gain_trace_row_t fp_gain_trace_rows[] = {
  [FP_GAIN_TRACE_FB]  = { MEAS_FB_RATIO,    0.0,   N_("F/B Ratio dB"),
                          N_("Max Gain & F/B Ratio vs Frequency") },
  [FP_GAIN_TRACE_FR]  = { MEAS_FR_RATIO_AZ, -1e30, N_("F/R Ratio dB"),
                          N_("Max Gain & F/R Ratio vs Frequency") },
  [FP_GAIN_TRACE_NET] = { MEAS_GAIN_NET,    -1e30, N_("Net Gain dBi"),
                          N_("Max Gain & Net Gain vs Frequency") },
};

_Static_assert(G_N_ELEMENTS(fp_gain_trace_rows) == FP_GAIN_TRACE_COUNT,
    "fp_gain_trace_rows must hold one row per fp_gain_trace_t");

  int
fp_gain_enabled(void)
{
  return rc_config.freqplots_gmax_togglebutton && isFlagSet(ENABLE_RDPAT);
}

/**
 * fp_gain_second_trace() - Resolve the Gain panel's second trace
 *
 * Net gain takes precedence when selected and a feedpoint defines mismatch
 * loss; otherwise the F/R selector chooses F/R over F/B.
 *
 * Return: the selected second trace.
 */
  fp_gain_trace_t
fp_gain_second_trace(void)
{
  fp_gain_trace_t trace;

  if( rc_config.freqplots_net_gain && fpat_has_feedpoint() )
    trace = FP_GAIN_TRACE_NET;
  else if( rc_config.freqplots_fr_ratio )
    trace = FP_GAIN_TRACE_FR;
  else
    trace = FP_GAIN_TRACE_FB;

  return trace;
}

/* Max-gain plot trace buffers, reused across fp_gain_render() calls. */
static double *gmax = NULL, *second_trace = NULL;
static double *gdir_tht = NULL, *gdir_phi = NULL;

/* fp_gain_free()
 *
 * Releases the max-gain plot trace buffers.
 */
  void
fp_gain_free( void )
{
  mem_array_free( &gmax );
  mem_array_free( &second_trace );
  mem_array_free( &gdir_tht );
  mem_array_free( &gdir_phi );

} /* fp_gain_free() */

  gboolean
fp_gain_render(fp_plot_ctx_t *ctx)
{
  char *titles[3];
  gboolean second_unavailable;
  double *right;
  int idx;

  const fp_gain_trace_row_t *trace = &fp_gain_trace_rows[fp_gain_second_trace()];

  /* Allocate max gain, directions, and the second trace */
  mem_array_realloc(&gmax, ctx->num_fsteps);
  mem_array_realloc(&gdir_tht, ctx->num_fsteps);
  mem_array_realloc(&gdir_phi, ctx->num_fsteps);
  mem_array_realloc(&second_trace, ctx->num_fsteps);

  /* Gather max gain, direction, and the second trace in one measurement pass */
  const fp_meas_column_t cols[] = {
    { gmax,     MEAS_GAIN_MAX   },
    { gdir_tht, MEAS_GAIN_THETA },
    { gdir_phi, MEAS_GAIN_PHI   },
    { second_trace, trace->meas_idx },
  };
  fp_fill_meas_columns( ctx, cols, (int)G_N_ELEMENTS(cols) );

  /* The second trace is unavailable when any step reports it unavailable */
  second_unavailable = FALSE;
  for( idx = 0; idx < ctx->num_fsteps && !second_unavailable; idx++ )
    second_unavailable = fp_value_unavailable( second_trace[idx], trace->invalid_min );

  /* Plot gain and the second trace when every step carries it */
  titles[0] = _("Raw Gain dBi");
  titles[1] = _(trace->panel_title);
  if( second_unavailable )
  {
    /* Blank right-axis title keeps the panel layout without a trace */
    right = NULL;
    titles[2] = "        ";
  }
  else
  {
    right = second_trace;
    titles[2] = _(trace->axis_title);
  }
  fp_plot_panel(ctx, gmax, right, titles, FP_PANEL_GAIN);

  /* Deposit max gain direction unconditionally; the dispatch gate emits it
   * only when selected (primary) or pinned (popup). */
  titles[0] = _("Rad Angle - deg");
  titles[1] = _("Max Gain Direction vs Frequency");
  titles[2] = _("Phi - deg");
  fp_plot_panel(ctx, gdir_tht, gdir_phi, titles, FP_PANEL_GAIN_DIR);

  return TRUE;
}
