/*
 *  Synchronous NEC2 frequency loop evaluation for optimizer.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <string.h>

#include "opt_nec2_eval.h"
#include "../shared.h"
#include "../sy_expr.h"
#include "../console.h"
#include "../utils.h"

/* Synchronization for blocking the optimizer thread until freq loop completes */
static GMutex eval_mutex;
static GCond eval_cond;
static gboolean eval_initialized = FALSE;

/* Context passed to the GTK callback for override-and-reload */
typedef struct
{
	const simple_var_t *vars;
	int num_vars;
} eval_apply_ctx_t;

/*------------------------------------------------------------------------*/

/**
 * nec2_eval_init - initialize evaluation synchronization primitives
 */
void nec2_eval_init(void)
{
	if (eval_initialized)
	{
		return;
	}

	g_mutex_init(&eval_mutex);
	g_cond_init(&eval_cond);
	eval_initialized = TRUE;
}

/*------------------------------------------------------------------------*/

/**
 * nec2_eval_cleanup - release synchronization primitives
 */
void nec2_eval_cleanup(void)
{
	if (!eval_initialized)
	{
		return;
	}

	g_cond_clear(&eval_cond);
	g_mutex_clear(&eval_mutex);
	eval_initialized = FALSE;
}

/*------------------------------------------------------------------------*/

/**
 * nec2_eval_signal - signal that the frequency loop has completed
 */
void nec2_eval_signal(void)
{
	if (!eval_initialized)
	{
		return;
	}

	g_mutex_lock(&eval_mutex);
	g_cond_signal(&eval_cond);
	g_mutex_unlock(&eval_mutex);
}

/*------------------------------------------------------------------------*/

/**
 * apply_vars_as_overrides - set SY symbol overrides from optimizer variables
 * @vars: simple_var_t array
 * @num_vars: length
 *
 * Each var's name maps to an SY symbol.  The first element of var->values
 * is applied as the override value.
 */
static void apply_vars_as_overrides(const simple_var_t *vars, int num_vars)
{
	int i;
	int j;

	for (i = 0; i < num_vars; i++)
	{
		if (vars[i].name == NULL || vars[i].values == NULL)
		{
			continue;
		}

		/* For single-valued variables, set element 0 */
		if (vars[i].values->size == 1)
		{
			sy_set_override(vars[i].name,
				gsl_vector_get(vars[i].values, 0), TRUE);
		}
		else
		{
			/* Multi-element: set indexed overrides (name_0, name_1, ...) */
			char indexed_name[128];

			for (j = 0; (size_t)j < vars[i].values->size; j++)
			{
				snprintf(indexed_name, sizeof(indexed_name),
					"%s_%d", vars[i].name, j);
				sy_set_override(indexed_name,
					gsl_vector_get(vars[i].values, j), TRUE);
			}
		}
	}
}

/*------------------------------------------------------------------------*/

/**
 * gtk_drain_pending - GTK main-thread callback: flush pending draw events
 *
 * Called via g_idle_add_once_sync after the frequency loop completes so
 * that all queued redraws are processed before the optimizer schedules
 * the next evaluation, reducing graph flicker.
 */
static void gtk_drain_pending(gpointer user_data)
{
	(void)user_data;

	while (gtk_events_pending())
	{
		gtk_main_iteration_do(FALSE);
	}
}

/*------------------------------------------------------------------------*/

/**
 * eval_apply_and_reload - GTK callback: set overrides, save .sy, reload
 * @user_data: pointer to eval_apply_ctx_t
 *
 * Runs on the GTK main thread via g_idle_add_once_sync.  Sets optimizer
 * variable overrides in the symbol table, persists all overrides to the
 * .sy file, and triggers Open_Input_File to re-read and run the freq loop.
 */
static void eval_apply_and_reload(gpointer user_data)
{
	eval_apply_ctx_t *ctx = (eval_apply_ctx_t *)user_data;
	static gboolean reload_flag = FALSE;
	char sy_filename[FILENAME_LEN];

	apply_vars_as_overrides(ctx->vars, ctx->num_vars);

	/* Persist overrides to .sy so sy_init + sy_load_overrides restores them */
	if (build_companion_path(rc_config.input_file, ".sy",
		sy_filename, sizeof(sy_filename)))
	{
		sy_save_overrides(sy_filename);
	}

	Open_Input_File((gpointer)&reload_flag);
}

/*------------------------------------------------------------------------*/

/**
 * nec2_eval_run - synchronously evaluate antenna with given variables
 */
int nec2_eval_run(const simple_var_t *vars, int num_vars,
	measurement_t *meas_out, int max_steps)
{
	eval_apply_ctx_t ctx;
	int steps;
	int i;
	int count;

	if (!eval_initialized)
	{
		pr_err("nec2_eval_run: not initialized\n");
		return -1;
	}

	ctx.vars = vars;
	ctx.num_vars = num_vars;

	/* Lock eval mutex before triggering freq loop so we do not miss the signal */
	g_mutex_lock(&eval_mutex);

	/* Apply overrides, save .sy, reload via GTK main loop */
	g_idle_add_once_sync(eval_apply_and_reload, &ctx);

	/* Wait for frequency loop completion signal */
	while (isFlagClear(FREQ_LOOP_DONE))
	{
		/* Timed wait: 100ms to allow periodic flag checks */
		gint64 end_time = g_get_monotonic_time() + 100 * G_TIME_SPAN_MILLISECOND;
		g_cond_wait_until(&eval_cond, &eval_mutex, end_time);
	}

	g_mutex_unlock(&eval_mutex);

	/* Flush pending GTK draw events before the next evaluation to reduce
	 * graph flicker caused by back-to-back frequency loop triggers */
	g_idle_add_once_sync(gtk_drain_pending, NULL);

	/* Collect measurements under freq_data_lock */
	g_mutex_lock(&freq_data_lock);

	steps = calc_data.steps_total;
	count = (steps < max_steps) ? steps : max_steps;

	for (i = 0; i < count; i++)
	{
		meas_calc(&meas_out[i], i);
	}

	g_mutex_unlock(&freq_data_lock);

	return count;
}

/*------------------------------------------------------------------------*/

/**
 * nec2_eval_get_freq - get frequency array after evaluation
 */
int nec2_eval_get_freq(double *freq_out, int max_steps)
{
	int steps;
	int count;

	g_mutex_lock(&freq_data_lock);

	steps = calc_data.steps_total;
	count = (steps < max_steps) ? steps : max_steps;

	memcpy(freq_out, save.freq, count * sizeof(double));

	g_mutex_unlock(&freq_data_lock);

	return count;
}
