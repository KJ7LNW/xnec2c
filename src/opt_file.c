/*
 *  Optimizer configuration file I/O.
 *
 *  Persists optimizer panel state (algorithm, convergence parameters,
 *  simplex/PSO settings, fitness goals) to a GKeyFile-format .opt
 *  file alongside the .nec model file.
 *
 *  Per-symbol optimizer state (opt checkbox, min/max bounds) is
 *  handled by the .sy file; this module handles only the optimizer
 *  panel configuration.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "opt_file.h"
#include "opt_ui_internal.h"
#include "rc_config.h"
#include "utils.h"
#include "shared.h"

/* Suppresses saves while opt_file_load is populating widgets */
static gboolean loading = FALSE;

/* GKeyFile group and key names */
#define GRP_OPTIMIZER    "optimizer"
#define GRP_SIMPLEX      "simplex"
#define GRP_PSO          "pso"
#define GRP_GOAL_PREFIX  "goal "

#define KEY_ALGORITHM    "algorithm"
#define KEY_MAX_ITER     "max_iter"
#define KEY_STAG_COUNT   "stagnant_count"
#define KEY_STAG_TOL     "stagnant_tol"

#define KEY_SIZES        "sizes"
#define KEY_MIN_SIZE     "min_size"
#define KEY_TEMPERATURE  "temperature"

#define KEY_PARTICLES    "particles"
#define KEY_NEIGHBORS    "neighbors"
#define KEY_INERTIA      "inertia"
#define KEY_COGNITIVE    "cognitive"
#define KEY_SOCIAL       "social"
#define KEY_SEARCH_SIZE  "search_size"

#define KEY_ENABLED      "enabled"
#define KEY_METRIC       "metric"
#define KEY_DIRECTION    "direction"
#define KEY_TARGET       "target"
#define KEY_EXPONENT     "exponent"
#define KEY_REDUCE       "reduce"
#define KEY_WEIGHT       "weight"
#define KEY_MHZ_MIN      "mhz_min"
#define KEY_MHZ_MAX      "mhz_max"

#define ALGO_SIMPLEX     "simplex"
#define ALGO_PSO         "pso"

/*------------------------------------------------------------------------*/

/**
 * lookup_name_index - find string in name array, return index
 * @names: NULL-terminated or count-bounded string array
 * @count: number of entries
 * @name: string to find
 *
 * Returns index on match, -1 if not found.
 */
static int lookup_name_index(const char **names, int count, const char *name)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (g_strcmp0(names[i], name) == 0)
		{
			return i;
		}
	}

	return -1;
}

/*------------------------------------------------------------------------*/

/**
 * on_entry_focus_out_save - save .opt when an entry loses focus
 */
static gboolean on_entry_focus_out_save(GtkWidget *widget,
	GdkEvent *event, gpointer user_data)
{
	(void)widget;
	(void)event;
	(void)user_data;

	if (!loading)
	{
		opt_file_save();
	}
	return FALSE;
}

/*------------------------------------------------------------------------*/

/**
 * on_combo_changed_save - save .opt when a combo selection changes
 */
static void on_combo_changed_save(GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	(void)user_data;

	if (!loading)
	{
		opt_file_save();
	}
}

/*------------------------------------------------------------------------*/

/**
 * on_check_toggled_save - save .opt when a checkbox is toggled
 */
static void on_check_toggled_save(GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	(void)user_data;

	if (!loading)
	{
		opt_file_save();
	}
}

/*------------------------------------------------------------------------*/

/**
 * opt_file_connect_entry - connect focus-out-event to trigger save
 */
void opt_file_connect_entry(GtkWidget *entry)
{
	g_signal_connect(entry, "focus-out-event",
		G_CALLBACK(on_entry_focus_out_save), NULL);
}

/*------------------------------------------------------------------------*/

/**
 * opt_file_connect_combo - connect changed signal to trigger save
 */
void opt_file_connect_combo(GtkWidget *combo)
{
	g_signal_connect(combo, "changed",
		G_CALLBACK(on_combo_changed_save), NULL);
}

/*------------------------------------------------------------------------*/

/**
 * opt_file_connect_check - connect toggled signal to trigger save
 */
void opt_file_connect_check(GtkWidget *check)
{
	g_signal_connect(check, "toggled",
		G_CALLBACK(on_check_toggled_save), NULL);
}

/*------------------------------------------------------------------------*/

/**
 * save_int_entry - write integer from entry, NAN-safe
 * @kf: key file
 * @grp: group name
 * @key: key name
 * @entry: GtkEntry widget to read
 *
 * Reads the entry as double, writes 0 if NAN.
 */
static void save_int_entry(GKeyFile *kf, const gchar *grp,
	const gchar *key, GtkWidget *entry)
{
	double val;

	val = get_entry_double(entry);
	g_key_file_set_integer(kf, grp, key, isnan(val) ? 0 : (gint)val);
}

/*------------------------------------------------------------------------*/

/**
 * save_optimizer_group - write [optimizer] section
 */
static void save_optimizer_group(GKeyFile *kf)
{
	gint algo_idx;
	const gchar *algo_name;

	algo_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(algo_combo));
	algo_name = (algo_idx == 0) ? ALGO_SIMPLEX : ALGO_PSO;

	g_key_file_set_string(kf, GRP_OPTIMIZER, KEY_ALGORITHM, algo_name);
	save_int_entry(kf, GRP_OPTIMIZER, KEY_MAX_ITER, max_iter_entry);
	save_int_entry(kf, GRP_OPTIMIZER, KEY_STAG_COUNT, stagnant_count_entry);
	g_key_file_set_double(kf, GRP_OPTIMIZER, KEY_STAG_TOL,
		get_entry_double(stagnant_tol_entry));
}

/*------------------------------------------------------------------------*/

/**
 * save_simplex_group - write [simplex] section
 */
static void save_simplex_group(GKeyFile *kf)
{
	double ssize_arr[32];
	int num_ssize;

	num_ssize = parse_ssize_list(ssize_entry, ssize_arr, 32);
	if (num_ssize > 0)
	{
		g_key_file_set_double_list(kf, GRP_SIMPLEX, KEY_SIZES,
			ssize_arr, (gsize)num_ssize);
	}

	g_key_file_set_double(kf, GRP_SIMPLEX, KEY_MIN_SIZE,
		get_entry_double(simplex_min_size_entry));
	g_key_file_set_double(kf, GRP_SIMPLEX, KEY_TEMPERATURE,
		get_entry_double(simplex_temp_entry));
}

/*------------------------------------------------------------------------*/

/**
 * save_pso_group - write [pso] section
 */
static void save_pso_group(GKeyFile *kf)
{
	save_int_entry(kf, GRP_PSO, KEY_PARTICLES, pso_particles_entry);
	save_int_entry(kf, GRP_PSO, KEY_NEIGHBORS, pso_neighbors_entry);
	g_key_file_set_double(kf, GRP_PSO, KEY_INERTIA,
		get_entry_double(pso_inertia_entry));
	g_key_file_set_double(kf, GRP_PSO, KEY_COGNITIVE,
		get_entry_double(pso_me_weight_entry));
	g_key_file_set_double(kf, GRP_PSO, KEY_SOCIAL,
		get_entry_double(pso_them_weight_entry));
	g_key_file_set_double(kf, GRP_PSO, KEY_SEARCH_SIZE,
		get_entry_double(pso_search_size_entry));
}

/*------------------------------------------------------------------------*/

/**
 * save_goal_groups - write [goal N] sections from goal_row_list
 */
static void save_goal_groups(GKeyFile *kf)
{
	GList *iter;
	int idx;

	idx = 0;
	for (iter = goal_row_list; iter != NULL; iter = iter->next)
	{
		opt_goal_row_t *gr = (opt_goal_row_t *)iter->data;
		fitness_objective_t obj;
		gchar group[32];
		double mhz_val;

		if (!read_objective_from_row(gr, &obj))
		{
			continue;
		}

		snprintf(group, sizeof(group), "%s%d", GRP_GOAL_PREFIX, idx);

		g_key_file_set_boolean(kf, group, KEY_ENABLED, obj.enabled);
		g_key_file_set_string(kf, group, KEY_METRIC,
			meas_names[obj.meas_index]);
		g_key_file_set_string(kf, group, KEY_DIRECTION,
			fitness_direction_names[obj.direction]);
		g_key_file_set_double(kf, group, KEY_TARGET, obj.target);
		g_key_file_set_double(kf, group, KEY_EXPONENT, obj.exponent);
		g_key_file_set_string(kf, group, KEY_REDUCE,
			fitness_reduce_names[obj.reduce]);
		g_key_file_set_double(kf, group, KEY_WEIGHT, obj.weight);

		/* Write MHz bounds only when set */
		mhz_val = get_entry_double(gr->w[GR_MHZ_MIN]);
		if (!isnan(mhz_val))
		{
			g_key_file_set_double(kf, group, KEY_MHZ_MIN, mhz_val);
		}

		mhz_val = get_entry_double(gr->w[GR_MHZ_MAX]);
		if (!isnan(mhz_val))
		{
			g_key_file_set_double(kf, group, KEY_MHZ_MAX, mhz_val);
		}

		idx++;
	}
}

/*------------------------------------------------------------------------*/

/**
 * opt_file_save - write optimizer configuration to .opt file
 */
gboolean opt_file_save(void)
{
	gchar path[FILENAME_LEN];
	GKeyFile *kf = NULL;
	GError *err = NULL;
	gboolean ok;

	if (!build_companion_path(rc_config.input_file, ".opt",
		path, sizeof(path)))
	{
		return FALSE;
	}

	/* Widgets not initialized yet */
	if (algo_combo == NULL)
	{
		return FALSE;
	}

	kf = g_key_file_new();

	save_optimizer_group(kf);
	save_simplex_group(kf);
	save_pso_group(kf);
	save_goal_groups(kf);

	ok = g_key_file_save_to_file(kf, path, &err);
	if (!ok)
	{
		pr_err("opt_file_save: %s\n", err->message);
		g_clear_error(&err);
	}

	g_key_file_free(kf);
	return ok;
}

/*------------------------------------------------------------------------*/

/**
 * load_int_entry - read integer key into entry widget if present
 * @kf: key file
 * @grp: group name
 * @key: key name
 * @entry: GtkEntry widget to populate
 */
static void load_int_entry(GKeyFile *kf, const gchar *grp,
	const gchar *key, GtkWidget *entry)
{
	gchar buf[32];

	if (!g_key_file_has_key(kf, grp, key, NULL))
	{
		return;
	}

	snprintf(buf, sizeof(buf), "%d",
		g_key_file_get_integer(kf, grp, key, NULL));
	gtk_entry_set_text(GTK_ENTRY(entry), buf);
}

/*------------------------------------------------------------------------*/

/**
 * load_double_entry - read double key into entry widget if present
 * @kf: key file
 * @grp: group name
 * @key: key name
 * @entry: GtkEntry widget to populate
 */
static void load_double_entry(GKeyFile *kf, const gchar *grp,
	const gchar *key, GtkWidget *entry)
{
	gchar buf[32];

	if (!g_key_file_has_key(kf, grp, key, NULL))
	{
		return;
	}

	snprintf(buf, sizeof(buf), "%.4g",
		g_key_file_get_double(kf, grp, key, NULL));
	gtk_entry_set_text(GTK_ENTRY(entry), buf);
}

/*------------------------------------------------------------------------*/

/**
 * load_optimizer_group - read [optimizer] section into UI widgets
 */
static void load_optimizer_group(GKeyFile *kf)
{
	gchar *algo = NULL;

	algo = g_key_file_get_string(kf, GRP_OPTIMIZER, KEY_ALGORITHM, NULL);
	if (algo != NULL)
	{
		if (g_strcmp0(algo, ALGO_PSO) == 0)
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(algo_combo), 1);
		}
		else
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(algo_combo), 0);
		}
		g_free(algo);
	}

	load_int_entry(kf, GRP_OPTIMIZER, KEY_MAX_ITER, max_iter_entry);
	load_int_entry(kf, GRP_OPTIMIZER, KEY_STAG_COUNT, stagnant_count_entry);
	load_double_entry(kf, GRP_OPTIMIZER, KEY_STAG_TOL, stagnant_tol_entry);
}

/*------------------------------------------------------------------------*/

/**
 * load_simplex_group - read [simplex] section into UI widgets
 */
static void load_simplex_group(GKeyFile *kf)
{
	if (!g_key_file_has_group(kf, GRP_SIMPLEX))
	{
		return;
	}

	/* Sizes list requires special handling (double list → comma string) */
	if (g_key_file_has_key(kf, GRP_SIMPLEX, KEY_SIZES, NULL))
	{
		gsize len = 0;
		gdouble *vals;

		vals = g_key_file_get_double_list(kf, GRP_SIMPLEX,
			KEY_SIZES, &len, NULL);
		if (vals != NULL)
		{
			GString *str;
			gsize i;

			str = g_string_new(NULL);
			for (i = 0; i < len; i++)
			{
				if (i > 0)
				{
					g_string_append(str, ", ");
				}
				g_string_append_printf(str, "%.4g", vals[i]);
			}

			gtk_entry_set_text(GTK_ENTRY(ssize_entry), str->str);
			g_string_free(str, TRUE);
			g_free(vals);
		}
	}

	load_double_entry(kf, GRP_SIMPLEX, KEY_MIN_SIZE, simplex_min_size_entry);
	load_double_entry(kf, GRP_SIMPLEX, KEY_TEMPERATURE, simplex_temp_entry);
}

/*------------------------------------------------------------------------*/

/**
 * load_pso_group - read [pso] section into UI widgets
 */
static void load_pso_group(GKeyFile *kf)
{
	if (!g_key_file_has_group(kf, GRP_PSO))
	{
		return;
	}

	load_int_entry(kf, GRP_PSO, KEY_PARTICLES, pso_particles_entry);
	load_int_entry(kf, GRP_PSO, KEY_NEIGHBORS, pso_neighbors_entry);
	load_double_entry(kf, GRP_PSO, KEY_INERTIA, pso_inertia_entry);
	load_double_entry(kf, GRP_PSO, KEY_COGNITIVE, pso_me_weight_entry);
	load_double_entry(kf, GRP_PSO, KEY_SOCIAL, pso_them_weight_entry);
	load_double_entry(kf, GRP_PSO, KEY_SEARCH_SIZE,
		pso_search_size_entry);
}

/*------------------------------------------------------------------------*/

/**
 * load_goal_groups - read [goal N] sections and rebuild goal rows
 */
static void load_goal_groups(GKeyFile *kf)
{
	gchar **groups = NULL;
	gsize num_groups;
	gsize i;
	int loaded;

	groups = g_key_file_get_groups(kf, &num_groups);
	if (groups == NULL)
	{
		return;
	}

	loaded = 0;
	for (i = 0; i < num_groups; i++)
	{
		fitness_objective_t obj;
		gchar *str = NULL;
		int idx;

		if (!g_str_has_prefix(groups[i], GRP_GOAL_PREFIX))
		{
			continue;
		}

		/* Metric name → meas_index */
		str = g_key_file_get_string(kf, groups[i], KEY_METRIC, NULL);
		if (str == NULL)
		{
			continue;
		}

		idx = meas_name_idx(str, (int)strlen(str));
		g_free(str);
		if (idx < 0 || idx >= MEAS_COUNT)
		{
			pr_err("opt_file_load: unknown metric in [%s]\n",
				groups[i]);
			continue;
		}

		obj.meas_index = idx;
		obj.enabled = g_key_file_get_boolean(kf, groups[i],
			KEY_ENABLED, NULL);

		/* Direction name → enum */
		str = g_key_file_get_string(kf, groups[i],
			KEY_DIRECTION, NULL);
		obj.direction = FIT_DIR_MINIMIZE;
		if (str != NULL)
		{
			idx = lookup_name_index(fitness_direction_names,
				FIT_DIR_COUNT, str);
			if (idx >= 0)
			{
				obj.direction = idx;
			}
			g_free(str);
		}

		obj.target = g_key_file_get_double(kf, groups[i],
			KEY_TARGET, NULL);
		obj.exponent = g_key_file_get_double(kf, groups[i],
			KEY_EXPONENT, NULL);

		/* Reduce name → enum */
		str = g_key_file_get_string(kf, groups[i],
			KEY_REDUCE, NULL);
		obj.reduce = FIT_REDUCE_SUM;
		if (str != NULL)
		{
			idx = lookup_name_index(fitness_reduce_names,
				FIT_REDUCE_COUNT, str);
			if (idx >= 0)
			{
				obj.reduce = idx;
			}
			g_free(str);
		}

		obj.weight = g_key_file_get_double(kf, groups[i],
			KEY_WEIGHT, NULL);

		/* MHz bounds: absent key → NAN */
		if (g_key_file_has_key(kf, groups[i], KEY_MHZ_MIN, NULL))
		{
			obj.mhz_min = g_key_file_get_double(kf, groups[i],
				KEY_MHZ_MIN, NULL);
		}
		else
		{
			obj.mhz_min = NAN;
		}

		if (g_key_file_has_key(kf, groups[i], KEY_MHZ_MAX, NULL))
		{
			obj.mhz_max = g_key_file_get_double(kf, groups[i],
				KEY_MHZ_MAX, NULL);
		}
		else
		{
			obj.mhz_max = NAN;
		}

		/* Clear defaults on first loaded goal */
		if (loaded == 0)
		{
			clear_goal_rows();
		}

		add_goal_from_obj(&obj);
		loaded++;
	}

	g_strfreev(groups);

	if (loaded > 0)
	{
		pr_info("Loaded %d optimizer goals\n", loaded);
	}
}

/*------------------------------------------------------------------------*/

/**
 * opt_file_load - load optimizer configuration from .opt file
 */
gboolean opt_file_load(void)
{
	gchar path[FILENAME_LEN];
	GKeyFile *kf = NULL;
	GError *err = NULL;
	gboolean ok;

	if (!build_companion_path(rc_config.input_file, ".opt",
		path, sizeof(path)))
	{
		return FALSE;
	}

	/* Widgets not initialized yet */
	if (algo_combo == NULL)
	{
		return FALSE;
	}

	kf = g_key_file_new();
	ok = g_key_file_load_from_file(kf, path, G_KEY_FILE_KEEP_COMMENTS,
		&err);
	if (!ok)
	{
		/* File not found is normal for new models */
		if (!g_error_matches(err, G_FILE_ERROR, G_FILE_ERROR_NOENT))
		{
			pr_err("opt_file_load: %s\n", err->message);
		}
		g_clear_error(&err);
		g_key_file_free(kf);
		return FALSE;
	}

	/* Suppress auto-save signals while populating widgets */
	loading = TRUE;

	load_optimizer_group(kf);
	load_simplex_group(kf);
	load_pso_group(kf);
	load_goal_groups(kf);

	loading = FALSE;

	/* Refresh formula display after loading goals */
	opt_ui_update_formula();
	opt_ui_update_values();

	g_key_file_free(kf);
	return TRUE;
}
