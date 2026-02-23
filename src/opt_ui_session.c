/*
 *  Optimizer UI — session start/cancel/status and algorithm selection.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "opt_ui_internal.h"
#include "optimizers/opt_session.h"
#include "optimizers/simplex.h"
#include "optimizers/particleswarm.h"
#include "sy_overrides.h"
#include "shared.h"

/**
 * check_opt_complete - periodic check if optimizer has finished
 */
static gboolean check_opt_complete(gpointer user_data)
{
	(void)user_data;

	/* Update status display while running */
	opt_ui_update_status();

	if (opt_is_running())
	{
		return G_SOURCE_CONTINUE;
	}

	/* Optimization finished: update UI */
	gtk_widget_set_sensitive(start_button, TRUE);
	gtk_widget_set_sensitive(cancel_button, FALSE);
	sy_overrides_set_apply_enabled(TRUE);

	/* Apply results back to override entries */
	{
		int num_result_vars;
		const simple_var_t *result_vars;

		result_vars = opt_get_result(&num_result_vars);
		if (result_vars != NULL)
		{
			sy_overrides_set_opt_results(result_vars, num_result_vars);
		}
	}

	/* Restore green line (fmhz_save) display state that was skipped
	 * during optimization to avoid deadlock in Frequency_Loop. */
	restore_fmhz_save_display();

	/* Final status: preserve full running metrics, append completion */
	opt_ui_update_status();

	if (status_label != NULL)
	{
		const gchar *current;
		gchar buf[512];

		current = gtk_label_get_text(GTK_LABEL(status_label));
		snprintf(buf, sizeof(buf), "%s  [Complete]", current);
		gtk_label_set_text(GTK_LABEL(status_label), buf);
	}

	pr_notice("opt_ui: optimization finished, best fitness: %.6g\n",
		opt_get_best_fitness());

	return G_SOURCE_REMOVE;
}

/*------------------------------------------------------------------------*/

/**
 * on_opt_start_clicked - Start Optimization button handler
 */
void on_opt_start_clicked(GtkButton *button, gpointer user_data)
{
	fitness_config_t fit_cfg;
	simple_var_t *vars;
	int num_vars;
	enum optimizer_algo algo;
	opt_algo_params_t algo_params;
	double ssize_arr[32];
	int max_iter;
	int stagnant_count;
	double stagnant_tol;
	int ret;

	(void)button;
	(void)user_data;

	if (opt_is_running())
	{
		return;
	}

	/* Collect Opt-flagged variables */
	num_vars = sy_overrides_get_opt_vars(&vars);
	if (num_vars == 0)
	{
		Notice(GTK_BUTTONS_OK, "Optimization",
			"No variables are flagged for optimization.\n"
			"Check the Opt column for variables to optimize.");
		return;
	}

	/* Read fitness config from UI */
	opt_ui_get_fitness_config(&fit_cfg);

	/* Verify at least one fitness goal is enabled */
	{
		int have_goal = 0;
		int g;

		for (g = 0; g < fit_cfg.num_obj; g++)
		{
			if (fit_cfg.obj[g].enabled)
			{
				have_goal = 1;
				break;
			}
		}

		if (!have_goal)
		{
			sy_overrides_free_opt_vars(vars, num_vars);
			fitness_config_free(&fit_cfg);
			Notice(GTK_BUTTONS_OK, "Optimization",
				"No fitness goals are enabled.\n"
				"Check at least one metric in the Fitness Goals grid.");
			return;
		}
	}

	/* Persist all current overrides to .sy before starting */
	sy_overrides_save_state();

	/* Read algorithm */
	algo = (gtk_combo_box_get_active(GTK_COMBO_BOX(algo_combo)) == 0)
		? OPT_SIMPLEX : OPT_PSO;

	/* Populate upstream config structs directly */
	memset(&algo_params, 0, sizeof(algo_params));

	if (algo == OPT_SIMPLEX)
	{
		simplex_config_init(&algo_params.simplex.cfg);

		algo_params.simplex.cfg.min_size =
			get_entry_double(simplex_min_size_entry);
		algo_params.simplex.cfg.temperature =
			get_entry_double(simplex_temp_entry);

		algo_params.simplex.num_ssize =
			parse_ssize_list(ssize_entry, ssize_arr, 32);
		algo_params.simplex.ssize = ssize_arr;
	}
	else
	{
		double num_particles;
		double num_neighbors;

		pso_config_init(&algo_params.pso_cfg);

		num_particles = get_entry_double(pso_particles_entry);
		num_neighbors = get_entry_double(pso_neighbors_entry);

		algo_params.pso_cfg.num_particles =
			isnan(num_particles) ? 0 : (int)num_particles;
		algo_params.pso_cfg.num_neighbors =
			isnan(num_neighbors) ? 0 : (int)num_neighbors;
		algo_params.pso_cfg.inertia =
			get_entry_double(pso_inertia_entry);
		algo_params.pso_cfg.me_weight =
			get_entry_double(pso_me_weight_entry);
		algo_params.pso_cfg.them_weight =
			get_entry_double(pso_them_weight_entry);
		algo_params.pso_cfg.search_size =
			get_entry_double(pso_search_size_entry);
	}

	/* Read convergence params */
	{
		double max_iter_val;
		double stagnant_count_val;

		max_iter_val = get_entry_double(max_iter_entry);
		stagnant_count_val = get_entry_double(stagnant_count_entry);

		max_iter = isnan(max_iter_val) ? 0 : (int)max_iter_val;
		stagnant_count = isnan(stagnant_count_val) ? 0 : (int)stagnant_count_val;
		stagnant_tol = get_entry_double(stagnant_tol_entry);
	}

	if (max_iter <= 0)
	{
		max_iter = 100;
	}

	/* Launch optimizer */
	ret = opt_start(vars, num_vars, &fit_cfg, algo, &algo_params,
		max_iter, stagnant_count, stagnant_tol);

	/* vars deep-copied by simple_new inside opt_start; free original */
	sy_overrides_free_opt_vars(vars, num_vars);
	fitness_config_free(&fit_cfg);

	if (ret == 0)
	{
		gtk_widget_set_sensitive(start_button, FALSE);
		gtk_widget_set_sensitive(cancel_button, TRUE);
		sy_overrides_set_apply_enabled(FALSE);

		if (status_label != NULL)
		{
			gtk_label_set_text(GTK_LABEL(status_label), "Starting...");
		}

		/* Poll for progress and completion */
		g_timeout_add(500, check_opt_complete, NULL);
	}
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_update_status - refresh status label from optimizer log state
 *
 * Also updates the formula display with the current best fitness
 * while the optimizer is running, avoiding the heavier
 * opt_ui_update_values() path that could interfere with the
 * frequency loop thread's synchronous GTK dispatches.
 */
void opt_ui_update_status(void)
{
	const simple_log_state_t *log;
	gchar buf[256];

	if (status_label == NULL)
	{
		return;
	}

	log = opt_get_log_state();
	if (log == NULL)
	{
		return;
	}

	snprintf(buf, sizeof(buf),
		"Pass %d/%d  Iter %d  Evals %d  Fitness %.6g  Best %.6g  "
		"Stagnant %d  Cache %d/%d",
		log->optimization_pass, log->num_passes,
		log->log_count, log->iter_count,
		log->minima, log->best_minima,
		log->prev_minima_count,
		log->cache_hits, log->cache_misses);

	gtk_label_set_text(GTK_LABEL(status_label), buf);

	/* Update per-row Value/Score from best measurement snapshot.
	 * Uses pre-allocated buffers and trylock to avoid blocking
	 * the GTK main thread or allocating on each timer tick. */
	if (opt_is_running() && timer_meas != NULL)
	{
		int best_steps;
		double total;

		if (opt_get_best_measurements(timer_meas, timer_freq,
			&best_steps))
		{
			total = update_goal_rows(timer_meas, timer_freq,
				best_steps);
			if (!isnan(total))
			{
				set_formula_with_score(total);
			}
			else
			{
				set_formula_with_score(log->best_minima);
			}
		}
		else
		{
			set_formula_with_score(log->best_minima);
		}
	}
}

/*------------------------------------------------------------------------*/

/**
 * on_algo_changed - toggle simplex/PSO parameter visibility
 */
void on_algo_changed(GtkComboBox *combo, gpointer user_data)
{
	gint active;

	(void)user_data;

	active = gtk_combo_box_get_active(combo);

	/* Combo box index maps to optimizer_algo enum: 0=OPT_SIMPLEX, 1=OPT_PSO */
	if (active == 0)
	{
		gtk_widget_show(simplex_label);
		gtk_widget_show_all(simplex_box);
		gtk_widget_hide(pso_label);
		gtk_widget_hide(pso_box);
	}
	else
	{
		gtk_widget_hide(simplex_label);
		gtk_widget_hide(simplex_box);
		gtk_widget_show(pso_label);
		gtk_widget_show(pso_box);
	}
}

/*------------------------------------------------------------------------*/

/**
 * on_opt_cancel_clicked - Cancel Optimization button handler
 */
void on_opt_cancel_clicked(GtkButton *button, gpointer user_data)
{
	(void)button;
	(void)user_data;

	opt_cancel();
}
