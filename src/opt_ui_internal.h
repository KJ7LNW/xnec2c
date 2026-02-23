/*
 *  Internal shared header for opt_ui split files.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_UI_INTERNAL_H
#define OPT_UI_INTERNAL_H 1

#include "opt_ui.h"

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

#define OPT_UI_ENTRY_WIDTH 8

/* Shared state — defined in opt_ui.c */
extern GList *goal_row_list;
extern GtkWidget *opt_expander;
extern GtkWidget *goals_container;
extern GtkWidget *goals_grid;
extern GtkWidget *algo_combo;
extern GtkWidget *simplex_label;
extern GtkWidget *simplex_box;
extern GtkWidget *ssize_entry;
extern GtkWidget *simplex_min_size_entry;
extern GtkWidget *simplex_temp_entry;
extern GtkWidget *pso_label;
extern GtkWidget *pso_box;
extern GtkWidget *pso_particles_entry;
extern GtkWidget *pso_neighbors_entry;
extern GtkWidget *pso_inertia_entry;
extern GtkWidget *pso_me_weight_entry;
extern GtkWidget *pso_them_weight_entry;
extern GtkWidget *pso_search_size_entry;
extern GtkWidget *stagnant_count_entry;
extern GtkWidget *stagnant_tol_entry;
extern GtkWidget *max_iter_entry;
extern GtkWidget *start_button;
extern GtkWidget *cancel_button;
extern GtkWidget *status_label;
extern GtkWidget *formula_display;
extern GtkWidget *formula_help_button;
extern GString *formula_base_markup;
extern measurement_t *timer_meas;
extern double *timer_freq;

/* Utilities — opt_ui.c */
double get_entry_double(GtkWidget *entry);
GtkWidget *make_entry(const gchar *text, gint width);
int metric_combo_index_to_meas(int combo_idx);
int meas_to_metric_combo_index(int meas_index);
int parse_ssize_list(GtkWidget *entry, double *out, int max);

/* Auto-save signal connections — opt_file.c */
void opt_file_connect_entry(GtkWidget *entry);
void opt_file_connect_combo(GtkWidget *combo);
void opt_file_connect_check(GtkWidget *check);

/* Goal row management — opt_ui_goals.c */
void build_goals_grid(void);
void destroy_goal_row(opt_goal_row_t *gr);
void clear_goal_rows(void);
void add_goal_from_obj(const fitness_objective_t *obj);

/* Formula and scoring — opt_ui_formula.c */
void opt_ui_update_formula(void);
void set_formula_with_score(double total_score);
gboolean read_objective_from_row(opt_goal_row_t *gr,
	fitness_objective_t *obj);
double update_goal_rows(const measurement_t *meas,
	const double *freq, int steps);
void on_opt_formula_help_clicked(GtkButton *button, gpointer user_data);

/* Session signal callbacks — opt_ui_session.c */
void on_opt_start_clicked(GtkButton *button, gpointer user_data);
void on_opt_cancel_clicked(GtkButton *button, gpointer user_data);
void on_algo_changed(GtkComboBox *combo, gpointer user_data);

#endif
