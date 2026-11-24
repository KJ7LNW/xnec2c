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

/* Bind each score band to its measurements, frequencies, and entry count.
 * Pass only that count to the fitness reduction so an extra selected-frequency
 * slot computed beyond the sweep never reaches a score. */
typedef struct
{
	const measurement_t *measurement;
	const double        *frequency_mhz;
	int                  count;
} opt_goal_band_t;

/* Display format strings for each FIT_DIR_* value.
 * All directions use 5 args: weight_str, reduce_name, val1, val2, exp_str.
 * MINIMIZE/MAXIMIZE use 𝑛 for √(t²+1); full formula in per-row tooltip.
 * Trailing " =" aligns with the Score column. */
static const char *formula_dir_fmts[FIT_DIR_COUNT] = {
	[FIT_DIR_MINIMIZE] =
		"<b>%s</b>·%s(((%s−<b>%s</b>)<sup>+</sup>"
		"/𝑛)<sup><b>%s</b></sup>) =",
	[FIT_DIR_MAXIMIZE] =
		"<b>%s</b>·%s(((<b>%s</b>−%s)<sup>+</sup>"
		"/𝑛)<sup><b>%s</b></sup>) =",
	[FIT_DIR_DEVIATE]  =
		"<b>%s</b>·%s(|%s−<b>%s</b>|<sup><b>%s</b></sup>) =",
};

/* Per-direction tooltip format strings with full formula and tension term.
 * Arg counts vary per direction; see tooltip construction in
 * update_goal_scores() for the per-direction arg lists. */
static const char *formula_tooltip_fmts[FIT_DIR_COUNT] = {
	[FIT_DIR_MINIMIZE] =
		N_("<b>minimize</b>: penalizes <i>%s</i> above <b>%s</b>\n\n"
		"score = (max(%s − <b>%s</b>, 0) / 𝑛)<sup><b>%s</b></sup>\n"
		"            + τ / (1 + max(<b>%s</b> − %s, 0) / 𝑛)\n\n"
		"𝑛 = √(<b>%s</b>² + 1)  (normalization)\n"
		"τ = (10<sup>−4</sup>)<sup><b>%s</b></sup>"
		"  (tension: residual gradient past target)"),
	[FIT_DIR_MAXIMIZE] =
		N_("<b>maximize</b>: penalizes <i>%s</i> below <b>%s</b>\n\n"
		"score = (max(<b>%s</b> − %s, 0) / 𝑛)<sup><b>%s</b></sup>\n"
		"            + τ / (1 + max(%s − <b>%s</b>, 0) / 𝑛)\n\n"
		"𝑛 = √(<b>%s</b>² + 1)  (normalization)\n"
		"τ = (10<sup>−4</sup>)<sup><b>%s</b></sup>"
		"  (tension: residual gradient past target)"),
	[FIT_DIR_DEVIATE] =
		N_("<b>deviate</b>: penalizes <i>%s</i> away from <b>%s</b>\n\n"
		"score = |%s − <b>%s</b>|<sup><b>%s</b></sup>\n\n"
		"No normalization or tension term."),
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
	if (formula_base_markup == NULL)
	{
		return;
	}

	/* Update totals row in the goals grid */
	if (totals_formula_label != NULL)
	{
		gtk_label_set_markup(GTK_LABEL(totals_formula_label),
			formula_base_markup);
	}

	if (totals_score_label != NULL)
	{
		if (!isnan(total_score))
		{
			gchar score_str[32];
			gchar *markup;

			snprintf(score_str, sizeof(score_str),
				"%.6g", total_score);
			markup = g_strdup_printf("<b>%s</b>", score_str);
			gtk_label_set_markup(
				GTK_LABEL(totals_score_label), markup);
			g_free(markup);
		}
		else
		{
			gtk_label_set_text(
				GTK_LABEL(totals_score_label), "—");
		}
	}

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

	if (totals_formula_label == NULL)
	{
		return;
	}

	opt_ui_get_fitness_config(&cfg);

	/* Count enabled objectives */
	term_count = 0;
	for (m = 0; m < cfg.num_obj; m++)
	{
		if (cfg.obj[m].enabled)
		{
			term_count++;
		}
	}

	if (term_count == 0)
	{
		formula_base_markup = _("<i>No goals enabled</i>");
		gtk_label_set_markup(GTK_LABEL(totals_formula_label),
			formula_base_markup);
	}
	else
	{
		/* Summary: individual terms are in per-row Formula column */
		formula_base_markup =
			_("<b>F = Σ(scores) =</b>\n"
			"<small><i>Optimizer minimizes F</i></small>");
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
 * nearest_frequency_index - locate the entry closest to a selected frequency
 * @frequency_mhz: frequency axis in MHz holding at least @count entries
 * @count: number of entries on the axis, greater than zero
 * @selected_mhz: selected frequency in MHz
 *
 * Retains the earlier entry when two are equally distant, so a selection
 * landing midway between two entries resolves the same way on every call.
 * Returns an index in 0..@count-1.
 */
static int nearest_frequency_index(const double *frequency_mhz, int count,
	double selected_mhz)
{
	int idx;
	int nearest;
	double nearest_diff;

	nearest = 0;
	nearest_diff = fabs(frequency_mhz[0] - selected_mhz);

	for (idx = 1; idx < count; idx++)
	{
		double diff = fabs(frequency_mhz[idx] - selected_mhz);
		gboolean is_closer = diff < nearest_diff;

		nearest = is_closer ? idx : nearest;
		nearest_diff = is_closer ? diff : nearest_diff;
	}

	return nearest;
}

/*------------------------------------------------------------------------*/

/**
 * update_goal_values - write every goal row's Value from one measurement
 * @selected: measurement taken at the selected frequency
 *
 * Writes the Value column alone, so changing the selected frequency refreshes
 * the displayed measurement while the band scores stand.
 */
static void update_goal_values(const measurement_t *selected)
{
	GList *iter;
	gchar buf[32];

	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;
		fitness_objective_t obj;
		double raw_value;

		if (!read_objective_from_row(gr, &obj))
		{
			continue;
		}

		raw_value = selected->a[obj.meas_index];

		if (raw_value == -1.0)
		{
			gtk_label_set_text(GTK_LABEL(gr->w[GR_VALUE]),
				"—");
		}
		else
		{
			snprintf(buf, sizeof(buf), "%.4g", raw_value);
			gtk_label_set_text(GTK_LABEL(gr->w[GR_VALUE]), buf);
		}
	}
}

/*------------------------------------------------------------------------*/

/**
 * update_goal_scores - write every goal row's Score, formula, and total
 * @band: band of ordinary sweep entries the reduction runs over
 *
 * Reduces each objective over @band->count entries, the same bound the
 * optimizer reduces over, and writes the Score column together with the
 * per-row formula fragment and its tooltip.  Returns the accumulated score
 * of the enabled objectives.
 */
static double update_goal_scores(const opt_goal_band_t *band)
{
	GList *iter;
	double total_score;
	gchar buf[32];

	/* Update each row's Score label, accumulate total */
	total_score = 0.0;
	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;
		fitness_objective_t obj;
		double score;

		if (!read_objective_from_row(gr, &obj))
		{
			continue;
		}

		score = fitness_compute_objective(&obj, band->measurement,
			band->count, band->frequency_mhz);

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

		/* Per-row formula fragment with Pango markup */
		{
			const char *reduce_name;
			const char *val1;
			const char *val2;
			gchar weight_str[32];
			gchar exp_str[32];
			gchar target_str[32];
			gchar *metric_compact;
			gchar *fragment;
			const gchar *src;
			gchar *dst;

			/* Strip spaces from metric name for compact notation */
			metric_compact = g_strdup(
				_(meas_display_names[obj.meas_index]));
			dst = metric_compact;
			for (src = metric_compact; *src != '\0'; src++)
			{
				if (*src != ' ')
				{
					*dst++ = *src;
				}
			}
			*dst = '\0';

			reduce_name = fitness_reduce_names[obj.reduce];

			snprintf(weight_str, sizeof(weight_str),
				"%.4g", obj.weight);
			snprintf(exp_str, sizeof(exp_str),
				"%.4g", obj.exponent);
			snprintf(target_str, sizeof(target_str),
				"%.4g", obj.target);

			val1 = (obj.direction == FIT_DIR_MAXIMIZE)
				? target_str : metric_compact;
			val2 = (obj.direction == FIT_DIR_MAXIMIZE)
				? metric_compact : target_str;

			fragment = g_strdup_printf(
				formula_dir_fmts[obj.direction],
				weight_str, reduce_name,
				val1, val2, exp_str);

			gtk_label_set_markup(
				GTK_LABEL(gr->w[GR_FORMULA]), fragment);
			g_free(fragment);

			/* Verbose tooltip with full formula and tension */
			if (obj.direction == FIT_DIR_DEVIATE)
			{
				fragment = g_strdup_printf(
					_(formula_tooltip_fmts[obj.direction]),
					metric_compact, target_str,
					metric_compact, target_str, exp_str);
			}
			else
			{
				/* MINIMIZE: metric, target in penalty;
				 *   target, metric in tension overshoot.
				 * MAXIMIZE: target, metric in penalty;
				 *   metric, target in tension overshoot. */
				const char *p1;
				const char *p2;
				const char *t1;
				const char *t2;

				if (obj.direction == FIT_DIR_MINIMIZE)
				{
					p1 = metric_compact;
					p2 = target_str;
					t1 = target_str;
					t2 = metric_compact;
				}
				else
				{
					p1 = target_str;
					p2 = metric_compact;
					t1 = metric_compact;
					t2 = target_str;
				}

				fragment = g_strdup_printf(
					_(formula_tooltip_fmts[obj.direction]),
					metric_compact, target_str,
					p1, p2, exp_str,
					t1, t2,
					target_str, exp_str);
			}
			gtk_widget_set_tooltip_markup(
				gr->w[GR_FORMULA], fragment);
			g_free(fragment);
			g_free(metric_compact);
		}
	}

	return total_score;
}

/*------------------------------------------------------------------------*/

/**
 * idle_selected_step - resolve selected Value against available idle data
 *
 * Returns the active step when it contains the selected frequency, including
 * the separately computed extra slot.  Otherwise returns the nearest ordinary
 * sweep step so an unapplied spinner edit still has a stable Value readout.
 */
static int idle_selected_step(void)
{
	int active_step;
	int selected_step;

	active_step = calc_data.freq_step;
	if (active_step >= 0 && active_step <= calc_data.steps_total
		&& save.fstep[active_step]
		&& FREQ_EQ(save.freq[active_step], calc_data.fmhz_save))
	{
		selected_step = active_step;
	}
	else
	{
		/* Resolve an unapplied spinner edit against ordinary sweep data */
		selected_step = nearest_frequency_index(save.freq,
			calc_data.steps_total, calc_data.fmhz_save);
	}

	return selected_step;
}

/*------------------------------------------------------------------------*/

/**
 * update_idle_selected_values - write Value from the selected sweep step
 *
 * Computes the measurement of the step the selection resolves to, so an
 * off-grid selection displays its own extra slot once that slot holds data.
 */
static void update_idle_selected_values(void)
{
	measurement_t selected;

	if (calc_data.steps_total <= 0)
	{
		return;
	}

	meas_calc(&selected, idle_selected_step(), calc_data.ex_port);
	update_goal_values(&selected);
}

/*------------------------------------------------------------------------*/

/**
 * running_best_band - view the optimizer's best-so-far snapshot as a band
 * @band: receives the snapshot measurements, their frequencies, and count
 *
 * Copies the snapshot into the preallocated timer buffers through a
 * non-blocking trylock, so the GTK main thread never waits on the optimizer
 * thread.  Returns FALSE and leaves @band untouched when no snapshot exists
 * yet or another thread holds its lock.
 */
static gboolean running_best_band(opt_goal_band_t *band)
{
	int best_steps;

	if (timer_meas == NULL || timer_freq == NULL
		|| !opt_get_best_measurements(timer_meas, timer_freq,
			&best_steps))
	{
		return FALSE;
	}

	*band = (opt_goal_band_t){
		.measurement = timer_meas,
		.frequency_mhz = timer_freq,
		.count = best_steps,
	};

	return TRUE;
}

/*------------------------------------------------------------------------*/

/**
 * update_band_selected_values - write Value from the nearest band entry
 * @band: band to read
 *
 * Serves the snapshot paths, which carry ordinary sweep entries and hold no
 * extra slot for an off-grid selected frequency.
 */
static void update_band_selected_values(const opt_goal_band_t *band)
{
	int selected;

	selected = nearest_frequency_index(band->frequency_mhz, band->count,
		calc_data.fmhz_save);
	update_goal_values(&band->measurement[selected]);
}

/*------------------------------------------------------------------------*/

/**
 * update_running_selected_values - write Value from the best snapshot
 *
 * Leaves Value labels standing when the non-blocking snapshot copy misses.
 */
static void update_running_selected_values(void)
{
	opt_goal_band_t band;

	if (!running_best_band(&band))
	{
		return;
	}

	update_band_selected_values(&band);
}

/*------------------------------------------------------------------------*/

/**
 * update_running_values - refresh every readout from the best snapshot
 *
 * Leaves row labels standing when the non-blocking snapshot copy misses and
 * refreshes the formula total from the authoritative running fitness.
 */
static void update_running_values(void)
{
	opt_goal_band_t band;

	if (!running_best_band(&band))
	{
		set_formula_with_score(opt_get_best_fitness());
		return;
	}

	update_band_selected_values(&band);
	set_formula_with_score(update_goal_scores(&band));
}

/*------------------------------------------------------------------------*/

/**
 * update_idle_values - refresh every readout from computed idle data
 *
 * Reduces each Score over the sweep steps alone, so the extra slot an
 * off-grid selection computes reaches Value without entering a score.
 */
static void update_idle_values(void)
{
	measurement_t *meas_all = NULL;
	measurement_t selected_extra;
	const measurement_t *selected;
	opt_goal_band_t band;
	int idx;
	int selected_step;
	int steps;

	steps = calc_data.steps_total;
	if (steps <= 0)
	{
		return;
	}

	mem_array_alloc(&meas_all, steps);

	for (idx = 0; idx < steps; idx++)
	{
		meas_calc(&meas_all[idx], idx, calc_data.ex_port);
	}

	selected_step = idle_selected_step();
	if (selected_step < steps)
	{
		selected = &meas_all[selected_step];
	}
	else
	{
		meas_calc(&selected_extra, selected_step, calc_data.ex_port);
		selected = &selected_extra;
	}

	update_goal_values(selected);
	band = (opt_goal_band_t){
		.measurement = meas_all,
		.frequency_mhz = save.freq,
		.count = steps,
	};
	set_formula_with_score(update_goal_scores(&band));

	mem_array_free(&meas_all);
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_update_selected_values - refresh Value from the selected frequency
 *
 * Writes the Value column alone, leaving every Score and the formula total
 * as they stand.
 */
void opt_ui_update_selected_values(void)
{
	if (goal_row_list == NULL || calc_data.fmhz_save <= 0.0)
	{
		return;
	}

	g_rec_mutex_lock(&freq_data_lock);

	if (opt_is_running())
	{
		update_running_selected_values();
	}
	else
	{
		update_idle_selected_values();
	}

	g_rec_mutex_unlock(&freq_data_lock);
}

/*------------------------------------------------------------------------*/

/**
 * opt_ui_update_values - refresh Value, Score, and formula from NEC2 data
 *
 * Prepares one measurement for the selected Value and a separate ordinary
 * sweep band for every Score, so the extra slot an off-grid selection
 * computes displays as a Value without entering the optimizer band.
 */
void opt_ui_update_values(void)
{
	if (goal_row_list == NULL || calc_data.fmhz_save <= 0.0)
	{
		return;
	}

	g_rec_mutex_lock(&freq_data_lock);

	if (opt_is_running())
	{
		update_running_values();
	}
	else
	{
		update_idle_values();
	}

	g_rec_mutex_unlock(&freq_data_lock);
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
		_("<span size='large' weight='bold'>Transform Directions</span>\n\n"
		"<b>minimize:</b> Lower values are better\n"
		"    Penalty = ((value − target)<sup>+</sup> / √(target²+1))<sup>exp</sup>\n"
		"    <i>Score approaches zero below target; penalizes excess above target</i>\n\n"
		"<b>maximize:</b> Higher values are better\n"
		"    Penalty = ((target − value)<sup>+</sup> / √(target²+1))<sup>exp</sup>\n"
		"    <i>Score approaches zero above target; works for positive and negative targets</i>\n\n"
		"<b>deviate:</b> Target a specific value\n"
		"    Penalty = |value − target|<sup>exp</sup>\n"
		"    <i>Penalizes deviation from target; score=0 when value = target</i>\n\n\n"
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
		"    sqrt(sum(penalty<sup>2</sup>))\n"
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
		"    gain_dev_nz: deviation from -Z axis (nadir)\n");

	dialog = gtk_dialog_new_with_buttons(
		_("Fitness Formula Help"),
		GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(formula_help_button))),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		_("_Close"), GTK_RESPONSE_CLOSE,
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
