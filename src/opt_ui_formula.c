/*
 *  Optimizer UI — formula display, help dialog, and value/score updates.
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
#include "shared.h"

/* Format strings for each FIT_DIR_* value.
 * Arguments in order: weight_str, reduce_name, val1, val2, exp_str
 * val1/val2 swap for FIT_DIR_MAXIMIZE (target/metric instead of metric/target). */
static const char *formula_dir_fmts[FIT_DIR_COUNT] = {
	[FIT_DIR_MINIMIZE] =
		"<b>%s</b>\xc2\xb7%s((%s/<b>%s</b>)<sup><b>%s</b></sup>)",
	[FIT_DIR_MAXIMIZE] =
		"<b>%s</b>\xc2\xb7%s((<b>%s</b>/%s)<sup><b>%s</b></sup>)",
	[FIT_DIR_DEVIATE]  =
		"<b>%s</b>\xc2\xb7%s(|%s\xe2\x88\x92<b>%s</b>|<sup><b>%s</b></sup>)",
};

/*------------------------------------------------------------------------*/

/**
 * set_formula_with_score - set formula label to base markup with score suffix
 * @total_score: total fitness score, or NAN if unavailable
 *
 * Combines the cached formula_base_markup with a score suffix.
 * When total_score is NAN, shows formula without score.
 */
void set_formula_with_score(double total_score)
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
void opt_ui_update_formula(void)
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
		const char *val1;
		const char *val2;
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

		/* maximize swaps metric and target in the fraction */
		val1 = (obj->direction == FIT_DIR_MAXIMIZE) ? target_str : metric_name;
		val2 = (obj->direction == FIT_DIR_MAXIMIZE) ? metric_name : target_str;

		g_string_append_printf(formula_base_markup,
			formula_dir_fmts[obj->direction],
			weight_str, reduce_name, val1, val2, exp_str);

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
gboolean read_objective_from_row(opt_goal_row_t *gr,
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
double update_goal_rows(const measurement_t *meas,
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
void on_opt_formula_help_clicked(GtkButton *button, gpointer user_data)
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
