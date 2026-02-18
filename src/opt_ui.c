/*
 *  Optimizer configuration UI panel.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "opt_ui.h"
#include "optimizers/opt_session.h"
#include "sy_overrides.h"
#include "shared.h"

/* Per-metric row widgets */
typedef struct
{
	GtkWidget *enabled_check;
	GtkWidget *weight_entry;
	GtkWidget *exp_entry;
	GtkWidget *target_entry;
	GtkWidget *reduce_combo;
	GtkWidget *mhz_min_entry;
	GtkWidget *mhz_max_entry;
} opt_goal_row_t;

/* Panel state — all widgets owned by builder, not freed here */
static GtkWidget *opt_expander = NULL;
static GtkWidget *goals_container = NULL;
static opt_goal_row_t goal_rows[FIT_METRIC_COUNT];
static GtkWidget *algo_combo = NULL;

/* Simplex-specific */
static GtkWidget *simplex_box = NULL;
static GtkWidget *ssize_entry = NULL;
static GtkWidget *simplex_min_size_entry = NULL;
static GtkWidget *simplex_temp_entry = NULL;

/* PSO-specific */
static GtkWidget *pso_box = NULL;
static GtkWidget *pso_particles_entry = NULL;
static GtkWidget *pso_neighbors_entry = NULL;
static GtkWidget *pso_inertia_entry = NULL;
static GtkWidget *pso_me_weight_entry = NULL;
static GtkWidget *pso_them_weight_entry = NULL;
static GtkWidget *pso_search_size_entry = NULL;

/* Common convergence */
static GtkWidget *stagnant_count_entry = NULL;
static GtkWidget *stagnant_tol_entry = NULL;
static GtkWidget *max_iter_entry = NULL;

/* Buttons and status */
static GtkWidget *start_button = NULL;
static GtkWidget *cancel_button = NULL;
static GtkWidget *status_label = NULL;

/* Forward declarations */
static void on_opt_start_clicked(GtkButton *button, gpointer user_data);
static void on_opt_cancel_clicked(GtkButton *button, gpointer user_data);
static void on_algo_changed(GtkComboBox *combo, gpointer user_data);
static gboolean check_opt_complete(gpointer user_data);

/*------------------------------------------------------------------------*/

/**
 * get_entry_double - parse double from entry, NAN if empty
 */
static double get_entry_double(GtkWidget *entry)
{
	const gchar *text;
	gchar *endptr;
	double val;

	text = gtk_entry_get_text(GTK_ENTRY(entry));

	if (text[0] == '\0')
	{
		return NAN;
	}

	val = g_strtod(text, &endptr);

	if (endptr == text)
	{
		return NAN;
	}

	return val;
}

/*------------------------------------------------------------------------*/

/**
 * make_entry - create a right-aligned entry with initial text
 */
static GtkWidget *make_entry(const gchar *text, gint width)
{
	GtkWidget *entry;

	entry = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entry), width);
	gtk_entry_set_max_width_chars(GTK_ENTRY(entry), width);
	gtk_entry_set_alignment(GTK_ENTRY(entry), 1.0);
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	return entry;
}

/*------------------------------------------------------------------------*/

#define OPT_UI_ENTRY_WIDTH 8

/**
 * build_goals_grid - create fitness goals grid and pack into container
 *
 * Programmatically builds one row per fitness_metric_info[] entry.
 */
static void build_goals_grid(void)
{
	GtkWidget *grid;
	GtkWidget *label;
	gchar buf[32];
	int m;
	int row;
	fitness_config_t defaults;

	fitness_config_init(&defaults);

	grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 4);

	/* Header row */
	{
		const gchar *headers[] = {
			"\u2713", "Metric", "Weight", "Exp",
			"Target", "Reduce", "MHz min", "MHz max"
		};
		int col;

		for (col = 0; col < 8; col++)
		{
			gchar *markup;

			label = gtk_label_new(NULL);
			markup = g_markup_printf_escaped(
				"<b><u>%s</u></b>", headers[col]);
			gtk_label_set_markup(GTK_LABEL(label), markup);
			g_free(markup);
			gtk_grid_attach(GTK_GRID(grid), label, col, 0, 1, 1);
		}
	}

	/* One row per metric */
	for (m = 0; m < FIT_METRIC_COUNT; m++)
	{
		const fitness_metric_info_t *info = &fitness_metric_info[m];
		opt_goal_row_t *gr = &goal_rows[m];
		int col;

		row = m + 1;

		/* Enabled checkbox */
		col = 0;
		gr->enabled_check = gtk_check_button_new();
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(gr->enabled_check),
			defaults.obj[m].enabled);
		gtk_grid_attach(GTK_GRID(grid), gr->enabled_check,
			col, row, 1, 1);

		/* Metric name */
		col = 1;
		label = gtk_label_new(info->name);
		gtk_label_set_xalign(GTK_LABEL(label), 0.0);
		gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);

		/* Weight */
		col = 2;
		snprintf(buf, sizeof(buf), "%.4g", info->default_weight);
		gr->weight_entry = make_entry(buf, OPT_UI_ENTRY_WIDTH);
		gtk_grid_attach(GTK_GRID(grid), gr->weight_entry,
			col, row, 1, 1);

		/* Exponent */
		col = 3;
		snprintf(buf, sizeof(buf), "%.4g", info->default_exponent);
		gr->exp_entry = make_entry(buf, OPT_UI_ENTRY_WIDTH);
		gtk_grid_attach(GTK_GRID(grid), gr->exp_entry,
			col, row, 1, 1);

		/* Target */
		col = 4;
		snprintf(buf, sizeof(buf), "%.4g", info->default_target);
		gr->target_entry = make_entry(buf, OPT_UI_ENTRY_WIDTH);
		gtk_grid_attach(GTK_GRID(grid), gr->target_entry,
			col, row, 1, 1);

		/* Reduce combo */
		col = 5;
		gr->reduce_combo = gtk_combo_box_text_new();
		{
			int r;

			for (r = 0; r < FIT_REDUCE_COUNT; r++)
			{
				gtk_combo_box_text_append_text(
					GTK_COMBO_BOX_TEXT(gr->reduce_combo),
					fitness_reduce_names[r]);
			}
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(gr->reduce_combo),
			info->default_reduce);
		gtk_grid_attach(GTK_GRID(grid), gr->reduce_combo,
			col, row, 1, 1);

		/* MHz min */
		col = 6;
		gr->mhz_min_entry = make_entry("", OPT_UI_ENTRY_WIDTH);
		gtk_grid_attach(GTK_GRID(grid), gr->mhz_min_entry,
			col, row, 1, 1);

		/* MHz max */
		col = 7;
		gr->mhz_max_entry = make_entry("", OPT_UI_ENTRY_WIDTH);
		gtk_grid_attach(GTK_GRID(grid), gr->mhz_max_entry,
			col, row, 1, 1);
	}

	gtk_box_pack_start(GTK_BOX(goals_container), grid, FALSE, FALSE, 0);
	gtk_widget_show_all(grid);
}

/*------------------------------------------------------------------------*/

/**
 * lookup_widget - get widget from builder by ID
 */
static GtkWidget *lookup_widget(GtkBuilder *builder, const gchar *id)
{
	return GTK_WIDGET(gtk_builder_get_object(builder, id));
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_init - initialize optimizer panel from Glade builder
 */
void opt_ui_init(GtkBuilder *builder)
{
	if (opt_expander != NULL)
	{
		return;
	}

	/* Look up all widgets from the builder */
	opt_expander          = lookup_widget(builder, "opt_expander");
	goals_container       = lookup_widget(builder, "opt_goals_container");
	algo_combo            = lookup_widget(builder, "opt_algo_combo");

	simplex_box           = lookup_widget(builder, "opt_simplex_box");
	ssize_entry           = lookup_widget(builder, "opt_simplex_sizes_entry");
	simplex_min_size_entry = lookup_widget(builder, "opt_simplex_min_size_entry");
	simplex_temp_entry    = lookup_widget(builder, "opt_simplex_temp_entry");

	pso_box               = lookup_widget(builder, "opt_pso_box");
	pso_particles_entry   = lookup_widget(builder, "opt_pso_particles_entry");
	pso_neighbors_entry   = lookup_widget(builder, "opt_pso_neighbors_entry");
	pso_inertia_entry     = lookup_widget(builder, "opt_pso_inertia_entry");
	pso_me_weight_entry   = lookup_widget(builder, "opt_pso_me_weight_entry");
	pso_them_weight_entry = lookup_widget(builder, "opt_pso_them_weight_entry");
	pso_search_size_entry = lookup_widget(builder, "opt_pso_search_size_entry");

	stagnant_count_entry  = lookup_widget(builder, "opt_stagnant_count_entry");
	stagnant_tol_entry    = lookup_widget(builder, "opt_stagnant_tol_entry");
	max_iter_entry        = lookup_widget(builder, "opt_max_iter_entry");

	start_button          = lookup_widget(builder, "opt_start_button");
	cancel_button         = lookup_widget(builder, "opt_cancel_button");
	status_label          = lookup_widget(builder, "opt_status_label");

	/* Build fitness goals grid programmatically into container */
	build_goals_grid();

	/* Connect signals */
	g_signal_connect(algo_combo, "changed",
		G_CALLBACK(on_algo_changed), NULL);
	g_signal_connect(start_button, "clicked",
		G_CALLBACK(on_opt_start_clicked), NULL);
	g_signal_connect(cancel_button, "clicked",
		G_CALLBACK(on_opt_cancel_clicked), NULL);
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_cleanup - release references to optimizer panel widgets
 */
void opt_ui_cleanup(void)
{
	opt_expander = NULL;
	goals_container = NULL;
	algo_combo = NULL;
	ssize_entry = NULL;
	simplex_box = NULL;
	simplex_min_size_entry = NULL;
	simplex_temp_entry = NULL;
	pso_box = NULL;
	pso_particles_entry = NULL;
	pso_neighbors_entry = NULL;
	pso_inertia_entry = NULL;
	pso_me_weight_entry = NULL;
	pso_them_weight_entry = NULL;
	pso_search_size_entry = NULL;
	stagnant_count_entry = NULL;
	stagnant_tol_entry = NULL;
	max_iter_entry = NULL;
	start_button = NULL;
	cancel_button = NULL;
	status_label = NULL;

	memset(goal_rows, 0, sizeof(goal_rows));
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_get_fitness_config - read current UI state into fitness config
 */
void opt_ui_get_fitness_config(fitness_config_t *cfg)
{
	int m;

	fitness_config_init(cfg);

	for (m = 0; m < FIT_METRIC_COUNT; m++)
	{
		opt_goal_row_t *gr = &goal_rows[m];

		cfg->obj[m].enabled = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(gr->enabled_check));
		cfg->obj[m].weight = get_entry_double(gr->weight_entry);
		cfg->obj[m].exponent = get_entry_double(gr->exp_entry);
		cfg->obj[m].target = get_entry_double(gr->target_entry);
		cfg->obj[m].reduce = gtk_combo_box_get_active(
			GTK_COMBO_BOX(gr->reduce_combo));
		cfg->obj[m].mhz_min = get_entry_double(gr->mhz_min_entry);
		cfg->obj[m].mhz_max = get_entry_double(gr->mhz_max_entry);
	}
}

/*------------------------------------------------------------------------*/

/**
 * parse_ssize_list - parse comma-separated doubles from entry
 * @entry: text entry widget
 * @out: output array (caller allocated)
 * @max: capacity
 *
 * Returns number of values parsed.
 */
static int parse_ssize_list(GtkWidget *entry, double *out, int max)
{
	const gchar *text;
	gchar **tokens;
	int count;
	int i;

	text = gtk_entry_get_text(GTK_ENTRY(entry));
	tokens = g_strsplit(text, ",", max);

	count = 0;
	for (i = 0; tokens[i] != NULL && count < max; i++)
	{
		gchar *stripped;
		gchar *endptr;
		double val;

		stripped = g_strstrip(tokens[i]);
		if (stripped[0] == '\0')
		{
			continue;
		}

		val = g_strtod(stripped, &endptr);
		if (endptr != stripped)
		{
			out[count++] = val;
		}
	}

	g_strfreev(tokens);
	return count;
}

/*------------------------------------------------------------------------*/

/**
 * on_opt_start_clicked - Start Optimization button handler
 */
static void on_opt_start_clicked(GtkButton *button, gpointer user_data)
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

		for (g = 0; g < FIT_METRIC_COUNT; g++)
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

	if (ret == 0)
	{
		gtk_widget_set_sensitive(start_button, FALSE);
		gtk_widget_set_sensitive(cancel_button, TRUE);

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
 * opt_ui_update_status - refresh status label from optimizer log state
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
}

/*------------------------------------------------------------------------*/

/**
 * on_algo_changed - toggle simplex/PSO parameter visibility
 */
static void on_algo_changed(GtkComboBox *combo, gpointer user_data)
{
	gint active;

	(void)user_data;

	active = gtk_combo_box_get_active(combo);

	/* Combo box index maps to optimizer_algo enum: 0=OPT_SIMPLEX, 1=OPT_PSO */
	if (active == 0)
	{
		gtk_widget_show_all(simplex_box);
		gtk_widget_hide(pso_box);
	}
	else
	{
		gtk_widget_hide(simplex_box);
		gtk_widget_show(pso_box);
	}
}

/*------------------------------------------------------------------------*/

/**
 * on_opt_cancel_clicked - Cancel Optimization button handler
 */
static void on_opt_cancel_clicked(GtkButton *button, gpointer user_data)
{
	(void)button;
	(void)user_data;

	opt_cancel();
}
