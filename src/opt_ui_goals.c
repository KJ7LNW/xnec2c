/*
 *  Optimizer UI — fitness goal row management and row signal handlers.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "opt_ui_internal.h"

/* Forward declarations for signal callbacks referenced in create_goal_row
 * and build_goals_grid before their definitions. */
static void on_goal_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void on_goal_param_changed(GtkWidget *widget, gpointer user_data);
static void on_metric_changed(GtkComboBox *combo, gpointer user_data);
static void on_remove_row_clicked(GtkButton *button, gpointer user_data);
static void on_add_metric_clicked(GtkButton *button, gpointer user_data);

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
void destroy_goal_row(opt_goal_row_t *gr)
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
void build_goals_grid(void)
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
