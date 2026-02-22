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

/* Column indices for goal row widget array.
 * Order matches the computation pipeline left-to-right:
 * Measurement -> Transform(Target, Exp) -> Reduce -> Weight */
enum goal_row_col
{
	GR_ENABLED,
	GR_METRIC,
	GR_VALUE,
	GR_TRANSFORM,
	GR_TARGET,
	GR_EXP,
	GR_REDUCE,
	GR_WEIGHT,
	GR_MHZ_MIN,
	GR_MHZ_MAX,
	GR_SCORE,
	GR_REMOVE,

	GR_NUM_COLS
};

/* Per-objective row widgets, indexed by goal_row_col */
typedef struct
{
	GtkWidget *w[GR_NUM_COLS];
} opt_goal_row_t;

/* Dynamic goal row list */
static GList *goal_row_list = NULL;

/* Panel state — all widgets owned by builder, not freed here */
static GtkWidget *opt_expander = NULL;
static GtkWidget *goals_container = NULL;
static GtkWidget *goals_grid = NULL;
static GtkWidget *algo_combo = NULL;

/* Simplex-specific */
static GtkWidget *simplex_label = NULL;
static GtkWidget *simplex_box = NULL;
static GtkWidget *ssize_entry = NULL;
static GtkWidget *simplex_min_size_entry = NULL;
static GtkWidget *simplex_temp_entry = NULL;

/* PSO-specific */
static GtkWidget *pso_label = NULL;
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

/* Formula display */
static GtkWidget *formula_display = NULL;
static GtkWidget *formula_help_button = NULL;

/* Cached formula markup (expression only, without score suffix) */
static GString *formula_base_markup = NULL;

/* Forward declarations */
static void on_opt_start_clicked(GtkButton *button, gpointer user_data);
static void on_opt_cancel_clicked(GtkButton *button, gpointer user_data);
static void on_algo_changed(GtkComboBox *combo, gpointer user_data);
static void on_opt_formula_help_clicked(GtkButton *button, gpointer user_data);
static void on_goal_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void on_add_metric_clicked(GtkButton *button, gpointer user_data);
static void on_remove_row_clicked(GtkButton *button, gpointer user_data);
static void on_metric_changed(GtkComboBox *combo, gpointer user_data);
static void on_goal_param_changed(GtkWidget *widget, gpointer user_data);
static gboolean check_opt_complete(gpointer user_data);
static void set_formula_with_score(double total_score);
static gboolean read_objective_from_row(opt_goal_row_t *gr,
	fitness_objective_t *obj);
static double update_goal_rows(const measurement_t *meas,
	const double *freq, int steps);
static void opt_ui_update_formula(void);

/* Pre-allocated buffers for timer-driven best-measurement display */
static measurement_t *timer_meas = NULL;
static double *timer_freq = NULL;

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
 * metric_combo_index_to_meas - map combo box index to MEASUREMENT_INDEXES
 *
 * The dropdown skips MEAS_MHZ (index 0), so combo index 0 = MEAS_ZREAL (1).
 */
static int metric_combo_index_to_meas(int combo_idx)
{
	return combo_idx + 1;
}

/**
 * meas_to_metric_combo_index - map MEASUREMENT_INDEXES to combo box index
 */
static int meas_to_metric_combo_index(int meas_index)
{
	return meas_index - 1;
}

/*------------------------------------------------------------------------*/

/**
 * rebuild_grid_rows - remove all rows from grid and re-add from list
 *
 * Row 0 is always the header.  Goal rows start at row 1.
 * The add-metric button is placed after the last row.
 */
static void rebuild_grid_rows(void)
{
	GList *iter;
	int row;
	int col;

	if (goals_grid == NULL)
	{
		return;
	}

	/* Remove all goal row widgets from grid (header stays at row 0) */
	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;

		for (col = 0; col < GR_NUM_COLS; col++)
		{
			g_object_ref(gr->w[col]);
			gtk_container_remove(GTK_CONTAINER(goals_grid), gr->w[col]);
		}
	}

	/* Re-attach in order */
	row = 1;
	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;

		for (col = 0; col < GR_NUM_COLS; col++)
		{
			gtk_grid_attach(GTK_GRID(goals_grid),
				gr->w[col], col, row, 1, 1);
			g_object_unref(gr->w[col]);
		}

		row++;
	}

	gtk_widget_show_all(goals_grid);
}

/*------------------------------------------------------------------------*/

/**
 * create_goal_row - allocate and populate a goal row for a measurement
 * @meas_index: MEASUREMENT_INDEXES value
 * @enabled: whether the checkbox starts active
 *
 * Creates all widgets, connects signals, sets defaults from
 * meas_fitness_defaults[].
 */
static opt_goal_row_t *create_goal_row(int meas_index, int enabled)
{
	opt_goal_row_t *gr = NULL;
	const meas_fitness_default_t *def;
	gchar buf[32];
	int i;

	mem_alloc((void **)&gr, sizeof(opt_goal_row_t), __LOCATION__);
	def = &meas_fitness_defaults[meas_index];

	/* Enabled checkbox */
	gr->w[GR_ENABLED] = gtk_check_button_new();
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(gr->w[GR_ENABLED]), enabled);
	g_signal_connect(gr->w[GR_ENABLED], "toggled",
		G_CALLBACK(on_goal_toggled), NULL);

	/* Measurement dropdown (skips MEAS_MHZ) */
	gr->w[GR_METRIC] = gtk_combo_box_text_new();
	for (i = 1; i < MEAS_COUNT; i++)
	{
		gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(gr->w[GR_METRIC]),
			meas_display_names[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(gr->w[GR_METRIC]),
		meas_to_metric_combo_index(meas_index));
	gtk_widget_set_tooltip_text(gr->w[GR_METRIC],
		meas_descriptions[meas_index]);
	g_signal_connect(gr->w[GR_METRIC], "changed",
		G_CALLBACK(on_metric_changed), gr);

	/* Value display (read-only, shows current NEC2 value) */
	gr->w[GR_VALUE] = gtk_label_new("\xe2\x80\x94");
	gtk_label_set_xalign(GTK_LABEL(gr->w[GR_VALUE]), 1.0);
	gtk_label_set_width_chars(GTK_LABEL(gr->w[GR_VALUE]), OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_VALUE],
		"Current NEC2 value at the frequency spinner position");

	/* Transform dropdown */
	gr->w[GR_TRANSFORM] = gtk_combo_box_text_new();
	for (i = 0; i < FIT_DIR_COUNT; i++)
	{
		gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(gr->w[GR_TRANSFORM]),
			fitness_direction_names[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(gr->w[GR_TRANSFORM]),
		def->direction);
	gtk_widget_set_tooltip_text(gr->w[GR_TRANSFORM],
		fitness_direction_tooltips[def->direction]);
	g_signal_connect(gr->w[GR_TRANSFORM], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* Target */
	snprintf(buf, sizeof(buf), "%.4g", def->default_target);
	gr->w[GR_TARGET] = make_entry(buf, OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_TARGET],
		"Goal value for the measurement");
	g_signal_connect(gr->w[GR_TARGET], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* Exponent */
	snprintf(buf, sizeof(buf), "%.4g", def->default_exponent);
	gr->w[GR_EXP] = make_entry(buf, OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_EXP],
		"Exponent applied to the transform result");
	g_signal_connect(gr->w[GR_EXP], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* Reduce combo */
	gr->w[GR_REDUCE] = gtk_combo_box_text_new();
	for (i = 0; i < FIT_REDUCE_COUNT; i++)
	{
		gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(gr->w[GR_REDUCE]),
			fitness_reduce_names[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(gr->w[GR_REDUCE]),
		def->default_reduce);
	gtk_widget_set_tooltip_text(gr->w[GR_REDUCE],
		fitness_reduce_tooltips[def->default_reduce]);
	g_signal_connect(gr->w[GR_REDUCE], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* Weight */
	snprintf(buf, sizeof(buf), "%.4g", def->default_weight);
	gr->w[GR_WEIGHT] = make_entry(buf, OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_WEIGHT],
		"Multiplier applied after reduction");
	g_signal_connect(gr->w[GR_WEIGHT], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* MHz min */
	gr->w[GR_MHZ_MIN] = make_entry("", OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_MHZ_MIN],
		"Lower frequency bound (empty = all frequencies)");
	g_signal_connect(gr->w[GR_MHZ_MIN], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* MHz max */
	gr->w[GR_MHZ_MAX] = make_entry("", OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_MHZ_MAX],
		"Upper frequency bound (empty = all frequencies)");
	g_signal_connect(gr->w[GR_MHZ_MAX], "changed",
		G_CALLBACK(on_goal_param_changed), gr);

	/* Score display (read-only, shows weight * reduce(transform(...))) */
	gr->w[GR_SCORE] = gtk_label_new("\xe2\x80\x94");
	gtk_label_set_xalign(GTK_LABEL(gr->w[GR_SCORE]), 1.0);
	gtk_label_set_width_chars(GTK_LABEL(gr->w[GR_SCORE]), OPT_UI_ENTRY_WIDTH);
	gtk_widget_set_tooltip_text(gr->w[GR_SCORE],
		"Computed: weight \xc3\x97 reduce(transform(value, target, exp))");

	/* Remove button */
	gr->w[GR_REMOVE] = gtk_button_new_with_label("\xe2\x88\x92");
	gtk_widget_set_tooltip_text(gr->w[GR_REMOVE], "Remove this objective");
	g_signal_connect(gr->w[GR_REMOVE], "clicked",
		G_CALLBACK(on_remove_row_clicked), gr);

	return gr;
}

/*------------------------------------------------------------------------*/

/**
 * destroy_goal_row - free a goal row and its widgets
 */
static void destroy_goal_row(opt_goal_row_t *gr)
{
	int col;

	for (col = 0; col < GR_NUM_COLS; col++)
	{
		gtk_widget_destroy(gr->w[col]);
	}

	free_ptr((void **)&gr);
}

/*------------------------------------------------------------------------*/

/**
 * add_goal_row - create a row and append to the dynamic list
 * @meas_index: MEASUREMENT_INDEXES value
 * @enabled: whether the checkbox starts active
 */
static void add_goal_row(int meas_index, int enabled)
{
	opt_goal_row_t *gr;

	gr = create_goal_row(meas_index, enabled);
	goal_row_list = g_list_append(goal_row_list, gr);
	rebuild_grid_rows();
	opt_ui_update_formula();
	opt_ui_update_values();
}

/*------------------------------------------------------------------------*/

/**
 * build_goals_grid - create fitness goals grid header and default rows
 */
static void build_goals_grid(void)
{
	GtkWidget *label;
	GtkWidget *add_button;

	goals_grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(goals_grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(goals_grid), 4);

	/* Header row — arrows show the computation pipeline */
	{
		const gchar *headers[GR_NUM_COLS] = {
			[GR_ENABLED]   = "\u2713",
			[GR_METRIC]    = "Measurement",
			[GR_VALUE]     = "Value",
			[GR_TRANSFORM] = "Transform \xe2\x86\x92",
			[GR_TARGET]    = "Target",
			[GR_EXP]       = "Exp \xe2\x86\x92",
			[GR_REDUCE]    = "Reduce \xe2\x86\x92",
			[GR_WEIGHT]    = "Weight",
			[GR_MHZ_MIN]   = "MHz lo",
			[GR_MHZ_MAX]   = "MHz hi \xe2\x86\x92",
			[GR_SCORE]     = "Score",
			[GR_REMOVE]    = "",
		};
		int col;

		for (col = 0; col < GR_NUM_COLS; col++)
		{
			gchar *markup;

			label = gtk_label_new(NULL);
			markup = g_markup_printf_escaped(
				"<b><u>%s</u></b>", headers[col]);
			gtk_label_set_markup(GTK_LABEL(label), markup);
			g_free(markup);
			gtk_grid_attach(GTK_GRID(goals_grid), label, col, 0, 1, 1);
		}
	}

	/* Default rows: VSWR, gain_max */
	add_goal_row(MEAS_VSWR, 1);
	add_goal_row(MEAS_GAIN_MAX, 1);

	/* Add-metric button below the grid */
	add_button = gtk_button_new_with_label("+ Add Metric");
	g_signal_connect(add_button, "clicked",
		G_CALLBACK(on_add_metric_clicked), NULL);

	gtk_box_pack_start(GTK_BOX(goals_container), goals_grid, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(goals_container), add_button, FALSE, FALSE, 4);
	gtk_widget_show_all(goals_container);
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

	simplex_label         = lookup_widget(builder, "opt_simplex_label");
	simplex_box           = lookup_widget(builder, "opt_simplex_box");
	ssize_entry           = lookup_widget(builder, "opt_simplex_sizes_entry");
	simplex_min_size_entry = lookup_widget(builder, "opt_simplex_min_size_entry");
	simplex_temp_entry    = lookup_widget(builder, "opt_simplex_temp_entry");

	pso_label             = lookup_widget(builder, "opt_pso_label");
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

	formula_display       = lookup_widget(builder, "opt_formula_display");
	formula_help_button   = lookup_widget(builder, "opt_formula_help_button");

	/* Build fitness goals grid programmatically into container */
	build_goals_grid();

	/* Connect signals */
	g_signal_connect(algo_combo, "changed",
		G_CALLBACK(on_algo_changed), NULL);
	g_signal_connect(start_button, "clicked",
		G_CALLBACK(on_opt_start_clicked), NULL);
	g_signal_connect(cancel_button, "clicked",
		G_CALLBACK(on_opt_cancel_clicked), NULL);
	g_signal_connect(formula_help_button, "clicked",
		G_CALLBACK(on_opt_formula_help_clicked), NULL);

	/* Pre-allocate buffers for timer-driven best-measurement display */
	mem_alloc((void **)&timer_meas,
		OPT_MAX_FREQ_STEPS * sizeof(measurement_t), __LOCATION__);
	mem_alloc((void **)&timer_freq,
		OPT_MAX_FREQ_STEPS * sizeof(double), __LOCATION__);

	/* Initialize formula display */
	opt_ui_update_formula();
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_cleanup - release references to optimizer panel widgets
 */
void opt_ui_cleanup(void)
{
	GList *iter;

	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		destroy_goal_row((opt_goal_row_t *)iter->data);
	}
	g_list_free(goal_row_list);
	goal_row_list = NULL;

	opt_expander = NULL;
	goals_container = NULL;
	goals_grid = NULL;
	algo_combo = NULL;
	ssize_entry = NULL;
	simplex_label = NULL;
	simplex_box = NULL;
	simplex_min_size_entry = NULL;
	simplex_temp_entry = NULL;
	pso_label = NULL;
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
	formula_display = NULL;
	formula_help_button = NULL;

	if (formula_base_markup != NULL)
	{
		g_string_free(formula_base_markup, TRUE);
		formula_base_markup = NULL;
	}

	free_ptr((void **)&timer_meas);
	free_ptr((void **)&timer_freq);
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_get_fitness_config - read current UI state into fitness config
 */
void opt_ui_get_fitness_config(fitness_config_t *cfg)
{
	GList *iter;

	memset(cfg, 0, sizeof(*cfg));
	cfg->obj = NULL;
	cfg->num_obj = 0;
	cfg->capacity = 0;

	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;
		fitness_objective_t tmp;
		fitness_objective_t *obj;

		if (!read_objective_from_row(gr, &tmp))
		{
			continue;
		}

		obj = fitness_config_add(cfg, tmp.meas_index);
		*obj = tmp;
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
static void on_algo_changed(GtkComboBox *combo, gpointer user_data)
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
 * on_goal_toggled - handle fitness goal checkbox toggle
 */
static void on_goal_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	(void)togglebutton;
	(void)user_data;

	opt_ui_update_formula();
	opt_ui_update_values();
}

/*------------------------------------------------------------------------*/

/**
 * on_goal_param_changed - handle any goal parameter widget change
 *
 * Triggers formula display refresh when weight, exponent, target,
 * direction, reduce, or MHz range entries are modified.
 */
static void on_goal_param_changed(GtkWidget *widget, gpointer user_data)
{
	opt_goal_row_t *gr = (opt_goal_row_t *)user_data;
	int dir_idx;
	int red_idx;

	(void)widget;

	/* Update direction and reduce tooltips from current selection */
	if (gr != NULL)
	{
		dir_idx = gtk_combo_box_get_active(
			GTK_COMBO_BOX(gr->w[GR_TRANSFORM]));
		if (dir_idx >= 0 && dir_idx < FIT_DIR_COUNT)
		{
			gtk_widget_set_tooltip_text(gr->w[GR_TRANSFORM],
				fitness_direction_tooltips[dir_idx]);
		}

		red_idx = gtk_combo_box_get_active(
			GTK_COMBO_BOX(gr->w[GR_REDUCE]));
		if (red_idx >= 0 && red_idx < FIT_REDUCE_COUNT)
		{
			gtk_widget_set_tooltip_text(gr->w[GR_REDUCE],
				fitness_reduce_tooltips[red_idx]);
		}
	}

	opt_ui_update_formula();
	opt_ui_update_values();
}

/*------------------------------------------------------------------------*/

/**
 * on_metric_changed - handle metric dropdown selection change
 *
 * Updates tooltip and auto-fills defaults from meas_fitness_defaults[].
 */
static void on_metric_changed(GtkComboBox *combo, gpointer user_data)
{
	opt_goal_row_t *gr = (opt_goal_row_t *)user_data;
	int combo_idx;
	int meas_index;
	const meas_fitness_default_t *def;
	gchar buf[32];

	combo_idx = gtk_combo_box_get_active(combo);
	if (combo_idx < 0)
	{
		return;
	}

	meas_index = metric_combo_index_to_meas(combo_idx);
	def = &meas_fitness_defaults[meas_index];

	/* Update tooltip to show measurement description */
	gtk_widget_set_tooltip_text(GTK_WIDGET(combo),
		meas_descriptions[meas_index]);

	/* Auto-fill defaults */
	gtk_combo_box_set_active(GTK_COMBO_BOX(gr->w[GR_TRANSFORM]),
		def->direction);
	gtk_widget_set_tooltip_text(gr->w[GR_TRANSFORM],
		fitness_direction_tooltips[def->direction]);

	snprintf(buf, sizeof(buf), "%.4g", def->default_weight);
	gtk_entry_set_text(GTK_ENTRY(gr->w[GR_WEIGHT]), buf);

	snprintf(buf, sizeof(buf), "%.4g", def->default_exponent);
	gtk_entry_set_text(GTK_ENTRY(gr->w[GR_EXP]), buf);

	snprintf(buf, sizeof(buf), "%.4g", def->default_target);
	gtk_entry_set_text(GTK_ENTRY(gr->w[GR_TARGET]), buf);

	gtk_combo_box_set_active(GTK_COMBO_BOX(gr->w[GR_REDUCE]),
		def->default_reduce);
	gtk_widget_set_tooltip_text(gr->w[GR_REDUCE],
		fitness_reduce_tooltips[def->default_reduce]);
}

/*------------------------------------------------------------------------*/

/**
 * on_add_metric_clicked - add a new metric row with VSWR defaults
 */
static void on_add_metric_clicked(GtkButton *button, gpointer user_data)
{
	(void)button;
	(void)user_data;

	add_goal_row(MEAS_VSWR, 0);
}

/*------------------------------------------------------------------------*/

/**
 * on_remove_row_clicked - remove the row associated with this button
 */
static void on_remove_row_clicked(GtkButton *button, gpointer user_data)
{
	opt_goal_row_t *gr = (opt_goal_row_t *)user_data;
	GList *link;

	(void)button;

	link = g_list_find(goal_row_list, gr);
	if (link == NULL)
	{
		return;
	}

	goal_row_list = g_list_delete_link(goal_row_list, link);
	destroy_goal_row(gr);
	rebuild_grid_rows();
	opt_ui_update_formula();
	opt_ui_update_values();
}

/*------------------------------------------------------------------------*/

/**
 * set_formula_with_score - set formula label to base markup with score suffix
 * @total_score: total fitness score, or NAN if unavailable
 *
 * Combines the cached formula_base_markup with a score suffix.
 * When total_score is NAN, shows formula without score.
 */
static void set_formula_with_score(double total_score)
{
	GString *full;

	if (formula_display == NULL || formula_base_markup == NULL)
	{
		return;
	}

	full = g_string_new(formula_base_markup->str);

	if (!isnan(total_score))
	{
		gchar score_str[32];

		snprintf(score_str, sizeof(score_str), "%.6g", total_score);
		g_string_append_printf(full,
			" = <b>%s</b>", score_str);
	}

	g_string_append(full,
		"\n<small><i>Optimizer minimizes this value</i></small>");

	gtk_label_set_markup(GTK_LABEL(formula_display), full->str);
	g_string_free(full, TRUE);
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_update_formula - update formula display with current enabled goals
 *
 * Builds the formula expression markup and caches it in
 * formula_base_markup.  Calls set_formula_with_score(NAN) to display
 * the formula without a computed score; the score is appended later
 * by opt_ui_update_values() when NEC2 data is available.
 */
static void opt_ui_update_formula(void)
{
	int term_count;
	int m;
	fitness_config_t cfg;

	if (formula_display == NULL)
	{
		return;
	}

	opt_ui_get_fitness_config(&cfg);

	/* Initialize or reset cached base markup */
	if (formula_base_markup == NULL)
	{
		formula_base_markup = g_string_new(NULL);
	}

	g_string_assign(formula_base_markup,
		"<b>Current Configuration:</b> F = ");
	term_count = 0;

	for (m = 0; m < cfg.num_obj; m++)
	{
		const fitness_objective_t *obj;
		const char *reduce_name;
		const char *metric_name;
		gchar weight_str[32];
		gchar exp_str[32];
		gchar target_str[32];

		obj = &cfg.obj[m];

		if (!obj->enabled)
		{
			continue;
		}

		if (term_count > 0)
		{
			g_string_append(formula_base_markup, " + ");
		}

		metric_name = meas_display_names[obj->meas_index];
		reduce_name = fitness_reduce_names[obj->reduce];

		snprintf(weight_str, sizeof(weight_str), "%.4g", obj->weight);
		snprintf(exp_str, sizeof(exp_str), "%.4g", obj->exponent);
		snprintf(target_str, sizeof(target_str), "%.4g", obj->target);

		/* Build term based on direction */
		if (obj->direction == FIT_DIR_MINIMIZE)
		{
			g_string_append_printf(formula_base_markup,
				"<b>%s</b>\xc2\xb7%s((%s/<b>%s</b>)"
				"<sup><b>%s</b></sup>)",
				weight_str, reduce_name,
				metric_name, target_str, exp_str);
		}
		else if (obj->direction == FIT_DIR_MAXIMIZE)
		{
			g_string_append_printf(formula_base_markup,
				"<b>%s</b>\xc2\xb7%s((<b>%s</b>/%s)"
				"<sup><b>%s</b></sup>)",
				weight_str, reduce_name,
				target_str, metric_name, exp_str);
		}
		else
		{
			g_string_append_printf(formula_base_markup,
				"<b>%s</b>\xc2\xb7%s(|%s\xe2\x88\x92<b>%s</b>|"
				"<sup><b>%s</b></sup>)",
				weight_str, reduce_name,
				metric_name, target_str, exp_str);
		}

		term_count++;
	}

	if (term_count == 0)
	{
		g_string_assign(formula_base_markup, "<i>No goals enabled</i>");
		gtk_label_set_markup(GTK_LABEL(formula_display),
			formula_base_markup->str);
	}
	else
	{
		/* Display formula without score; score added by
		 * opt_ui_update_values() when data is available */
		set_formula_with_score(NAN);
	}

	fitness_config_free(&cfg);
}

/*------------------------------------------------------------------------*/

/**
 * read_objective_from_row - populate fitness objective from goal row widgets
 * @gr: goal row to read
 * @obj: output objective (fully populated on success)
 *
 * Reads metric, direction, weight, exponent, target, reduce, and MHz
 * range from the row's widgets.  Falls back to defaults for invalid
 * combo box selections.  Returns TRUE if the metric combo is valid.
 */
static gboolean read_objective_from_row(opt_goal_row_t *gr,
	fitness_objective_t *obj)
{
	int combo_idx;
	int meas_index;
	const meas_fitness_default_t *def;

	combo_idx = gtk_combo_box_get_active(
		GTK_COMBO_BOX(gr->w[GR_METRIC]));
	if (combo_idx < 0)
	{
		return FALSE;
	}

	meas_index = metric_combo_index_to_meas(combo_idx);
	def = &meas_fitness_defaults[meas_index];

	obj->meas_index = meas_index;
	obj->enabled = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(gr->w[GR_ENABLED]));
	obj->direction = gtk_combo_box_get_active(
		GTK_COMBO_BOX(gr->w[GR_TRANSFORM]));
	obj->weight = get_entry_double(gr->w[GR_WEIGHT]);
	obj->exponent = get_entry_double(gr->w[GR_EXP]);
	obj->target = get_entry_double(gr->w[GR_TARGET]);
	obj->reduce = gtk_combo_box_get_active(
		GTK_COMBO_BOX(gr->w[GR_REDUCE]));
	obj->mhz_min = get_entry_double(gr->w[GR_MHZ_MIN]);
	obj->mhz_max = get_entry_double(gr->w[GR_MHZ_MAX]);

	/* Fall back to defaults for invalid combo selections */
	if (obj->direction < 0 || obj->direction >= FIT_DIR_COUNT)
	{
		obj->direction = def->direction;
	}

	if (obj->reduce < 0 || obj->reduce >= FIT_REDUCE_COUNT)
	{
		obj->reduce = def->default_reduce;
	}

	return TRUE;
}

/*------------------------------------------------------------------------*/

/**
 * update_goal_rows - update Value/Score labels from measurement arrays
 * @meas: measurement array, one per frequency step
 * @freq: frequency array in MHz
 * @steps: number of frequency steps
 *
 * Finds the frequency closest to calc_data.fmhz_save for the Value
 * column, computes per-objective scores via fitness_compute_objective,
 * and updates each goal row's labels.  Returns accumulated total
 * score for enabled objectives, or NAN if frequency data unavailable.
 */
static double update_goal_rows(const measurement_t *meas,
	const double *freq, int steps)
{
	GList *iter;
	int idx;
	int best_idx;
	double best_diff;
	double total_score;
	gchar buf[32];

	if (goal_row_list == NULL || steps <= 0
		|| calc_data.fmhz_save <= 0.0)
	{
		return NAN;
	}

	/* Find frequency index closest to fmhz_save */
	best_idx = 0;
	best_diff = fabs(freq[0] - calc_data.fmhz_save);

	for (idx = 1; idx < steps; idx++)
	{
		double diff = fabs(freq[idx] - calc_data.fmhz_save);

		if (diff < best_diff)
		{
			best_diff = diff;
			best_idx = idx;
		}
	}

	/* Update each row's Value and Score labels, accumulate total */
	total_score = 0.0;
	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;
		fitness_objective_t obj;
		double raw_value;
		double score;

		if (!read_objective_from_row(gr, &obj))
		{
			continue;
		}

		raw_value = meas[best_idx].a[obj.meas_index];

		/* Update Value label */
		if (raw_value == -1.0)
		{
			gtk_label_set_text(GTK_LABEL(gr->w[GR_VALUE]),
				"\xe2\x80\x94");
		}
		else
		{
			snprintf(buf, sizeof(buf), "%.4g", raw_value);
			gtk_label_set_text(GTK_LABEL(gr->w[GR_VALUE]), buf);
		}

		score = fitness_compute_objective(&obj, meas, steps, freq);

		if (obj.enabled)
		{
			snprintf(buf, sizeof(buf), "%.4g", score);
			total_score += score;
		}
		else
		{
			snprintf(buf, sizeof(buf), "(%.4g)", score);
		}
		gtk_label_set_text(GTK_LABEL(gr->w[GR_SCORE]), buf);
	}

	return total_score;
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_update_values - refresh Value and Score labels from NEC2 data
 *
 * When the optimizer is running, attempts to refresh from the
 * best-so-far measurement snapshot (non-blocking trylock).
 * If the lock is contended or no snapshot exists yet, leaves
 * labels untouched to avoid flashing dashes.
 *
 * When idle, computes per-row values from NEC2 data via meas_calc()
 * and shows the total fitness score in the formula display.
 */
void opt_ui_update_values(void)
{
	GList *iter;
	measurement_t *meas_all = NULL;
	double *freq_all = NULL;
	int idx;
	int steps;
	double total_score;

	if (goal_row_list == NULL)
	{
		return;
	}

	/* During optimization: try to refresh from best snapshot.
	 * If trylock fails, leave labels untouched to avoid flashing
	 * dashes when UI interactions trigger this function. */
	if (opt_is_running())
	{
		if (timer_meas != NULL)
		{
			int best_steps;

			if (opt_get_best_measurements(timer_meas, timer_freq,
				&best_steps))
			{
				total_score = update_goal_rows(timer_meas,
					timer_freq, best_steps);
				if (!isnan(total_score))
				{
					set_formula_with_score(total_score);
				}
				else
				{
					set_formula_with_score(
						opt_get_best_fitness());
				}
			}
		}
		return;
	}

	if (calc_data.steps_total <= 0)
	{
		return;
	}

	if (calc_data.fmhz_save <= 0.0)
	{
		return;
	}

	steps = calc_data.steps_total;

	/* Build measurement array for all steps */
	mem_alloc((void **)&meas_all, steps * sizeof(measurement_t), __LOCATION__);
	mem_alloc((void **)&freq_all, steps * sizeof(double), __LOCATION__);

	for (idx = 0; idx < steps; idx++)
	{
		meas_calc(&meas_all[idx], idx);
		freq_all[idx] = save.freq[idx];
	}

	total_score = update_goal_rows(meas_all, freq_all, steps);

	if (!isnan(total_score))
	{
		set_formula_with_score(total_score);
	}

	free_ptr((void **)&meas_all);
	free_ptr((void **)&freq_all);
}

/*------------------------------------------------------------------------*/

/**
 * on_opt_formula_help_clicked - show formula help dialog
 */
static void on_opt_formula_help_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog;
	GtkWidget *content_area;
	GtkWidget *scrolled;
	const gchar *help_text;

	(void)button;
	(void)user_data;

	help_text =
		"<span size='large' weight='bold'>Transform Directions</span>\n\n"
		"<b>minimize:</b> Lower values are better\n"
		"    Penalty = (value/target)\xe1\xb5\x89\xcb\xa3\xe1\xb5\x96\n"
		"    <i>Use for VSWR, noise, or metrics where smaller is optimal</i>\n\n"
		"<b>maximize:</b> Higher values are better\n"
		"    Penalty = (target/value)\xe1\xb5\x89\xcb\xa3\xe1\xb5\x96\n"
		"    <i>Use for gain, efficiency, F/B ratio</i>\n\n"
		"<b>deviate:</b> Target a specific value\n"
		"    Penalty = |value \xe2\x88\x92 target|\xe1\xb5\x89\xcb\xa3\xe1\xb5\x96\n"
		"    <i>Use for angle, impedance, or frequency alignment</i>\n\n\n"
		"<span size='large' weight='bold'>Reduction Functions</span>\n\n"
		"<b>avg:</b> Average penalty across band\n"
		"    <i>Balances all frequencies equally</i>\n\n"
		"<b>max:</b> Returns highest penalty (worst frequency point)\n"
		"    <i>Optimizer improves the worst frequency first\n"
		"    \"No point on the band can exceed X\"</i>\n\n"
		"<b>min:</b> Returns lowest penalty (best frequency point)\n"
		"    <i>Optimizer improves the best frequency, ignores others\n"
		"    Rarely useful (creates narrow-band solution)</i>\n\n"
		"<b>diff:</b> Returns penalty range (variation)\n"
		"    <i>Makes metric consistent across band\n"
		"    Use with gain_max for gain flatness</i>\n\n"
		"<b>sum:</b> Total penalty sum across all frequencies\n"
		"    <i>Emphasizes overall error magnitude</i>\n\n"
		"<b>mag:</b> Root mean square magnitude\n"
		"    sqrt(sum(penalty\xc2\xb2))\n"
		"    <i>Emphasizes large deviations more than average</i>\n\n\n"
		"<span size='large' weight='bold'>Gain Direction Metrics</span>\n\n"
		"The gain_dev_* metrics measure angular deviation (in degrees)\n"
		"between peak gain direction and a coordinate axis.\n"
		"Use direction=minimize with target near 0 to steer the beam.\n\n"
		"    gain_dev_px: deviation from +X axis\n"
		"    gain_dev_nx: deviation from -X axis\n"
		"    gain_dev_py: deviation from +Y axis\n"
		"    gain_dev_ny: deviation from -Y axis\n"
		"    gain_dev_pz: deviation from +Z axis (zenith)\n"
		"    gain_dev_nz: deviation from -Z axis (nadir)\n";

	dialog = gtk_dialog_new_with_buttons(
		"Fitness Formula Help",
		GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(formula_help_button))),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		"_Close", GTK_RESPONSE_CLOSE,
		NULL);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(scrolled), 6);

	/* Use GtkLabel with markup for proper formatting */
	{
		GtkWidget *label;
		GtkWidget *viewport;

		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label), help_text);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD);
		gtk_label_set_xalign(GTK_LABEL(label), 0.0);
		gtk_label_set_yalign(GTK_LABEL(label), 0.0);
		gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_widget_set_margin_start(label, 12);
		gtk_widget_set_margin_end(label, 12);
		gtk_widget_set_margin_top(label, 12);
		gtk_widget_set_margin_bottom(label, 12);

		viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(viewport), label);
		gtk_container_add(GTK_CONTAINER(scrolled), viewport);
	}

	gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
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
