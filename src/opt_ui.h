/*
 *  Optimizer configuration UI panel.
 *
 *  Looks up widgets from the sy_overrides GtkBuilder and populates
 *  the fitness goals grid programmatically from meas_fitness_defaults[].
 *  Provides Start/Cancel buttons and status display.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_UI_H
#define OPT_UI_H 1

#include "common.h"
#include "optimizers/opt_fitness.h"

/**
 * opt_ui_init - initialize optimizer panel from Glade builder
 * @builder: GtkBuilder containing opt_* widget IDs
 *
 * Looks up all optimizer widgets from the builder, builds the
 * fitness goals grid programmatically, and connects signals.
 */
void opt_ui_init(GtkBuilder *builder);

/**
 * opt_ui_cleanup - release references to optimizer panel widgets
 *
 * NULLs all static widget pointers.  Widgets are owned by the
 * builder and destroyed with the window.
 */
void opt_ui_cleanup(void);

/**
 * opt_ui_get_fitness_config - read current UI state into fitness config
 * @cfg: output config to populate
 */
void opt_ui_get_fitness_config(fitness_config_t *cfg);

/**
 * opt_ui_update_status - refresh status label from optimizer log state
 *
 * Reads opt_get_log_state() and formats into the status label.
 * Safe to call when no optimizer is running (displays idle state).
 */
void opt_ui_update_status(void);

/**
 * opt_ui_update_values - refresh Value, Score, and formula total from NEC2 data
 *
 * When the optimizer is running, attempts to refresh from the
 * best-so-far measurement snapshot (non-blocking trylock via
 * opt_get_best_measurements).  If the lock is contended or no
 * snapshot exists yet, leaves labels untouched to avoid flashing
 * dashes.
 *
 * When idle, finds the frequency index matching calc_data.fmhz_save,
 * computes measurements via meas_calc(), updates each goal row's
 * Value/Score labels, and shows the total fitness score in the
 * formula display as "F = ... = Score".
 *
 * Called on the GTK main thread.  Safe to call when no goals exist
 * or when frequency data is unavailable.
 */
void opt_ui_update_values(void);

#endif
