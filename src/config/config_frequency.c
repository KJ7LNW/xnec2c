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
 * config_frequency: the frequency selection's configuration domain.
 *
 * Carries a committed frequency to the solver or to the display alone, and
 * owns the session-only apply-frequency field the main and radiation
 * windows both present.
 */

#include "config_frequency.h"
#include "widget/config_widget_ops.h"
#include "../shared.h"

/* The committed frequency and the frequency the display reads are both
 * double-width, so one projection arm carries either of them. */
CONFIG_FIELD_DBL_ASSERT(calc_data.fmhz_save);
CONFIG_FIELD_DBL_ASSERT(calc_data.freq_mhz);

/*------------------------------------------------------------------------*/

void
hook_frequency(void)
{
  /* No frequency data loaded yet (eg config_widget_run_hooks() called from
   * Restore_GUI_State() before any NEC2 file is read); mirrors the guard
   * in freq_step_update_ui(). */
  if( save.freq == NULL )
    return;

  if(freq_sweep_active())
    return;

  if( freq_sweep_armed() )
    return;

  if( rc_config.freq_apply )
    user_set_frequency(calc_data.fmhz_save);
  else
    freq_display_update(calc_data.fmhz_save);
}

/*------------------------------------------------------------------------*/

/** config_frequency_capture_main - hold the main window's frequency control
 * @widget: the spin button the binding resolved
 */
void
config_frequency_capture_main(GtkWidget *widget)
{
  mainwin_frequency = GTK_SPIN_BUTTON(widget);
}

/** config_frequency_capture_pattern - hold the radiation window's frequency control
 * @widget: the spin button the binding resolved
 */
void
config_frequency_capture_pattern(GtkWidget *widget)
{
  rdpattern_frequency = GTK_SPIN_BUTTON(widget);
}

/** config_frequency_pattern_ready - whether the radiation window takes frequency
 *
 * Drawing is what makes the radiation window's frequency control a live
 * participant; a window already built but not yet drawing holds a control
 * the frequency path leaves alone.
 *
 * Return: TRUE while the radiation window draws and holds its control.
 */
static gboolean
config_frequency_pattern_ready(void)
{
  return isFlagSet(DRAW_ENABLED) && (rdpattern_frequency != NULL);
}

const config_widget_participation_t config_frequency_readout_rule =
  { .operation = &config_widget_readout_operation,
    .allowed   = config_frequency_pattern_ready };

const config_widget_participation_t config_frequency_sensitive_rule =
  { .operation = &config_widget_sensitive_operation,
    .allowed   = config_frequency_pattern_ready };

/*------------------------------------------------------------------------*/


/* Apply-frequency checkbutton tree: session-only, no persistence row.
 * File-scope storage so the binding registry holds a pointer with static
 * lifetime; an inline compound literal would die when
 * config_frequency_init() returns. */
static const config_widget_tree_t *const freq_apply_tree =
  CONFIG_WIDGET_TREE( .groups = CONFIG_WIDGET_GROUPS(
    CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
      .elements = CONFIG_WIDGETS(
        CONFIG_WIDGET( .widget_id = "main_freq_checkbutton" ), NULL ) ),
    CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
      .elements = CONFIG_WIDGETS(
        CONFIG_WIDGET( .widget_id = "rdpattern_freq_checkbutton" ), NULL ) ),
    NULL ) );

void
config_frequency_init(void)
{
  config_widget_register( &rc_config.freq_apply, sizeof(rc_config.freq_apply),
    freq_apply_tree );
}
