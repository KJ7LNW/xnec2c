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

#include "opt_ui_internal.h"
#include "opt_file.h"
#include "optimizers/opt_session.h"

/* Dynamic goal row list */
GList *goal_row_list = NULL;

/* Panel state — all widgets owned by builder, not freed here */
GtkWidget *opt_expander           = NULL;
GtkWidget *goals_container        = NULL;
GtkWidget *goals_grid             = NULL;
GtkWidget *algo_combo             = NULL;

/* Simplex-specific */
GtkWidget *simplex_label          = NULL;
GtkWidget *simplex_box            = NULL;
GtkWidget *ssize_entry            = NULL;
GtkWidget *simplex_min_size_entry = NULL;
GtkWidget *simplex_temp_entry     = NULL;

/* PSO-specific */
GtkWidget *pso_label              = NULL;
GtkWidget *pso_box                = NULL;
GtkWidget *pso_particles_entry    = NULL;
GtkWidget *pso_neighbors_entry    = NULL;
GtkWidget *pso_inertia_entry      = NULL;
GtkWidget *pso_me_weight_entry    = NULL;
GtkWidget *pso_them_weight_entry  = NULL;
GtkWidget *pso_search_size_entry  = NULL;

/* Common convergence */
GtkWidget *stagnant_count_entry   = NULL;
GtkWidget *stagnant_tol_entry     = NULL;
GtkWidget *max_iter_entry         = NULL;

/* Buttons and status */
GtkWidget *start_button           = NULL;
GtkWidget *cancel_button          = NULL;
GtkWidget *status_label           = NULL;

/* Formula display */
GtkWidget *formula_display        = NULL;
GtkWidget *formula_help_button    = NULL;

/* Cached formula markup (expression only, without score suffix) */
GString *formula_base_markup = NULL;

/* Pre-allocated buffers for timer-driven best-measurement display */
measurement_t *timer_meas = NULL;
double *timer_freq = NULL;

/* Widget lookup table shared by init and cleanup */
static const struct
{
	GtkWidget **ptr;
	const char *id;
} widget_map[] = {
	{ &opt_expander,           "opt_expander"               },
	{ &goals_container,        "opt_goals_container"        },
	{ &algo_combo,             "opt_algo_combo"             },
	{ &simplex_label,          "opt_simplex_label"          },
	{ &simplex_box,            "opt_simplex_box"            },
	{ &ssize_entry,            "opt_simplex_sizes_entry"    },
	{ &simplex_min_size_entry, "opt_simplex_min_size_entry" },
	{ &simplex_temp_entry,     "opt_simplex_temp_entry"     },
	{ &pso_label,              "opt_pso_label"              },
	{ &pso_box,                "opt_pso_box"                },
	{ &pso_particles_entry,    "opt_pso_particles_entry"    },
	{ &pso_neighbors_entry,    "opt_pso_neighbors_entry"    },
	{ &pso_inertia_entry,      "opt_pso_inertia_entry"      },
	{ &pso_me_weight_entry,    "opt_pso_me_weight_entry"    },
	{ &pso_them_weight_entry,  "opt_pso_them_weight_entry"  },
	{ &pso_search_size_entry,  "opt_pso_search_size_entry"  },
	{ &stagnant_count_entry,   "opt_stagnant_count_entry"   },
	{ &stagnant_tol_entry,     "opt_stagnant_tol_entry"     },
	{ &max_iter_entry,         "opt_max_iter_entry"         },
	{ &start_button,           "opt_start_button"           },
	{ &cancel_button,          "opt_cancel_button"          },
	{ &status_label,           "opt_status_label"           },
	{ &formula_display,        "opt_formula_display"        },
	{ &formula_help_button,    "opt_formula_help_button"    },
};

#define WIDGET_MAP_COUNT (sizeof(widget_map) / sizeof(widget_map[0]))

/*------------------------------------------------------------------------*/

/**
 * get_entry_double - parse double from entry, NAN if empty
 */
double get_entry_double(GtkWidget *entry)
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
GtkWidget *make_entry(const gchar *text, gint width)
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

/**
 * metric_combo_index_to_meas - map combo box index to MEASUREMENT_INDEXES
 *
 * The dropdown skips MEAS_MHZ (index 0), so combo index 0 = MEAS_ZREAL (1).
 */
int metric_combo_index_to_meas(int combo_idx)
{
	return combo_idx + 1;
}

/**
 * meas_to_metric_combo_index - map MEASUREMENT_INDEXES to combo box index
 */
int meas_to_metric_combo_index(int meas_index)
{
	return meas_index - 1;
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
	size_t i;

	if (opt_expander != NULL)
	{
		return;
	}

	/* Look up all widgets from the builder via table */
	for (i = 0; i < WIDGET_MAP_COUNT; i++)
	{
		*widget_map[i].ptr = lookup_widget(builder, widget_map[i].id);
	}

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

	/* Auto-save .opt on widget changes */
	{
		static const struct
		{
			GtkWidget **ptr;
			void (*connect)(GtkWidget *);
		} save_map[] = {
			{ &algo_combo,             opt_file_connect_combo },
			{ &max_iter_entry,         opt_file_connect_entry },
			{ &stagnant_count_entry,   opt_file_connect_entry },
			{ &stagnant_tol_entry,     opt_file_connect_entry },
			{ &ssize_entry,            opt_file_connect_entry },
			{ &simplex_min_size_entry, opt_file_connect_entry },
			{ &simplex_temp_entry,     opt_file_connect_entry },
			{ &pso_particles_entry,    opt_file_connect_entry },
			{ &pso_neighbors_entry,    opt_file_connect_entry },
			{ &pso_inertia_entry,      opt_file_connect_entry },
			{ &pso_me_weight_entry,    opt_file_connect_entry },
			{ &pso_them_weight_entry,  opt_file_connect_entry },
			{ &pso_search_size_entry,  opt_file_connect_entry },
		};
		size_t s;

		for (s = 0; s < sizeof(save_map) / sizeof(save_map[0]); s++)
		{
			save_map[s].connect(*save_map[s].ptr);
		}
	}

	/* Pre-allocate buffers for timer-driven best-measurement display */
	mem_alloc((void **)&timer_meas,
		OPT_MAX_FREQ_STEPS * sizeof(measurement_t), __LOCATION__);
	mem_alloc((void **)&timer_freq,
		OPT_MAX_FREQ_STEPS * sizeof(double), __LOCATION__);

	/* Load saved configuration, then initialize formula display */
	opt_file_load();
	opt_ui_update_formula();
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_cleanup - release references to optimizer panel widgets
 */
void opt_ui_cleanup(void)
{
	GList *iter;
	size_t i;

	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		destroy_goal_row((opt_goal_row_t *)iter->data);
	}
	g_list_free(goal_row_list);
	goal_row_list = NULL;

	/* Null all builder-owned widget pointers via table */
	for (i = 0; i < WIDGET_MAP_COUNT; i++)
	{
		*widget_map[i].ptr = NULL;
	}

	/* goals_grid is created programmatically, not in the builder table */
	goals_grid = NULL;

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
int parse_ssize_list(GtkWidget *entry, double *out, int max)
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
