/*
 *  Optimizer session orchestration.
 *
 *  Connects the Simple optimizer to the fitness function and NEC2
 *  evaluator.  Manages the optimizer pthread lifecycle.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <pthread.h>

#include "opt_session.h"
#include "../shared.h"
#include "opt_nec2_eval.h"

/* Active optimizer session (one at a time) */
static opt_session_t *active_session = NULL;

/*------------------------------------------------------------------------*/

/**
 * opt_fitness_callback - fitness function called by simple optimizer
 * @vars: current variable values from optimizer
 * @num_vars: length of vars array
 * @ctx: opaque pointer to opt_session_t
 *
 * Runs a synchronous NEC2 evaluation and computes fitness.
 */
static double opt_fitness_callback(const simple_var_t *vars, int num_vars,
	void *ctx)
{
	opt_session_t *session = (opt_session_t *)ctx;
	int steps;
	double fitness;

	steps = nec2_eval_run(vars, num_vars,
		session->meas, OPT_MAX_FREQ_STEPS);

	if (steps <= 0)
	{
		return INFINITY;
	}

	session->num_steps = steps;
	nec2_eval_get_freq(session->freq, OPT_MAX_FREQ_STEPS);

	fitness = fitness_compute(&session->fitness_cfg,
		session->meas, steps, session->freq);

	/* Snapshot measurements when this evaluation improves on the
	 * best snapshot seen so far.  Uses best_snap_fitness rather
	 * than best_fitness which is only updated by the log callback
	 * once per iteration and would allow worse evals to overwrite. */
	if (fitness < session->best_snap_fitness)
	{
		g_mutex_lock(&session->best_lock);
		memcpy(session->best_meas, session->meas,
			steps * sizeof(measurement_t));
		memcpy(session->best_freq, session->freq,
			steps * sizeof(double));
		session->best_num_steps = steps;
		session->best_snap_fitness = fitness;
		session->has_best_meas = TRUE;
		g_mutex_unlock(&session->best_lock);
	}

	return fitness;
}

/*------------------------------------------------------------------------*/

/**
 * opt_log_callback - log function called by simple optimizer each iteration
 * @vars: current variable values
 * @num_vars: length of vars array
 * @state: iteration state from simple optimizer
 * @ctx: opaque pointer to opt_session_t
 */
static void opt_log_callback(const simple_var_t *vars, int num_vars,
	const simple_log_state_t *state, void *ctx)
{
	opt_session_t *session = (opt_session_t *)ctx;

	(void)vars;
	(void)num_vars;

	session->best_fitness = state->best_minima;
	session->last_log = *state;
	session->has_log = TRUE;

	pr_notice("opt: pass %d/%d iter %d fitness %.6g best %.6g ssize %.4g "
		"stagnant %d cache %d/%d\n",
		state->optimization_pass, state->num_passes,
		state->iter_count, state->minima, state->best_minima,
		state->ssize, state->prev_minima_count,
		state->cache_hits, state->cache_misses);
}

/*------------------------------------------------------------------------*/

/**
 * opt_thread_func - optimizer worker thread entry point
 * @arg: pointer to opt_session_t
 */
static void *opt_thread_func(void *arg)
{
	opt_session_t *session = (opt_session_t *)arg;

	SetFlag(SUPPRESS_INTERMEDIATE_REDRAWS);

	nec2_eval_init();

	session->best_fitness = simple_optimize(session->simple);

	pr_notice("opt: optimization complete, best fitness: %.6g\n",
		session->best_fitness);

	/* Final evaluation with best result to update xnec2c display */
	{
		int num_result_vars;
		const simple_var_t *result_vars;

		result_vars = simple_get_result(session->simple, &num_result_vars);
		if (result_vars != NULL)
		{
			nec2_eval_run(result_vars, num_result_vars,
				session->meas, OPT_MAX_FREQ_STEPS);
		}
	}

	nec2_eval_cleanup();
	ClearFlag(SUPPRESS_INTERMEDIATE_REDRAWS);
	session->running = FALSE;

	return NULL;
}

/*------------------------------------------------------------------------*/

/**
 * opt_start - launch optimizer in background thread
 */
int opt_start(simple_var_t *vars, int num_vars,
	const fitness_config_t *fitness_cfg,
	enum optimizer_algo algo, const opt_algo_params_t *algo_params,
	int max_iter, int stagnant_count, double stagnant_tol)
{
	opt_session_t *session = NULL;
	int ret;

	if (active_session != NULL && active_session->running)
	{
		pr_err("opt_start: optimizer already running\n");
		return -1;
	}

	/* Free previous session */
	if (active_session != NULL)
	{
		simple_free(active_session->simple);
		fitness_config_free(&active_session->fitness_cfg);
		if (active_session->simple_cfg.algorithm == OPT_SIMPLEX)
		{
			free_ptr((void **)&active_session->simple_cfg.opts.simplex.ssize);
		}
		g_mutex_clear(&active_session->best_lock);
		free_ptr((void **)&active_session);
		active_session = NULL;
	}

	mem_alloc((void **)&session, sizeof(opt_session_t), __LOCATION__);
	g_mutex_init(&session->best_lock);
	fitness_config_copy(&session->fitness_cfg, fitness_cfg);
	session->running = TRUE;
	session->best_fitness = INFINITY;
	session->best_snap_fitness = INFINITY;

	/* Configure simple optimizer */
	simple_config_init(&session->simple_cfg, algo);

	session->simple_cfg.vars = vars;
	session->simple_cfg.num_vars = num_vars;
	session->simple_cfg.max_iter = max_iter;
	session->simple_cfg.stagnant_minima_count = stagnant_count;
	session->simple_cfg.stagnant_minima_tolerance = stagnant_tol;
	session->simple_cfg.fit_func = opt_fitness_callback;
	session->simple_cfg.fit_func_ctx = session;
	session->simple_cfg.log_func = opt_log_callback;
	session->simple_cfg.log_func_ctx = session;

	/* Copy upstream config structs directly from caller */
	if (algo == OPT_SIMPLEX)
	{
		session->simple_cfg.opts.simplex.cfg = algo_params->simplex.cfg;

		/* Deep-copy ssize array; caller's stack buffer will not persist */
		if (algo_params->simplex.ssize != NULL
			&& algo_params->simplex.num_ssize > 0)
		{
			double *ssize_copy = NULL;

			mem_alloc((void **)&ssize_copy,
				algo_params->simplex.num_ssize * sizeof(double),
				__LOCATION__);
			memcpy(ssize_copy, algo_params->simplex.ssize,
				algo_params->simplex.num_ssize * sizeof(double));
			session->simple_cfg.opts.simplex.ssize = ssize_copy;
			session->simple_cfg.opts.simplex.num_ssize =
				algo_params->simplex.num_ssize;
		}
	}
	else
	{
		session->simple_cfg.opts.pso_cfg = algo_params->pso_cfg;
	}

	session->simple = simple_new(&session->simple_cfg);
	if (session->simple == NULL)
	{
		pr_err("opt_start: simple_new failed\n");
		goto cleanup;
	}

	active_session = session;

	ret = pthread_create(&session->thread, NULL, opt_thread_func, session);
	if (ret != 0)
	{
		pr_err("opt_start: pthread_create failed: %d\n", ret);
		simple_free(session->simple);
		active_session = NULL;
		goto cleanup;
	}

	pthread_detach(session->thread);

	return 0;

cleanup:
	session->running = FALSE;
	if (algo == OPT_SIMPLEX)
	{
		free_ptr((void **)&session->simple_cfg.opts.simplex.ssize);
	}
	free_ptr((void **)&session);
	return -1;
}

/*------------------------------------------------------------------------*/

/**
 * opt_cancel - request early termination of running optimizer
 */
void opt_cancel(void)
{
	if (active_session == NULL || !active_session->running)
	{
		return;
	}

	simple_cancel(active_session->simple);
}

/*------------------------------------------------------------------------*/

/**
 * opt_is_running - check if optimizer thread is active
 */
gboolean opt_is_running(void)
{
	if (active_session == NULL)
	{
		return FALSE;
	}

	return active_session->running;
}

/*------------------------------------------------------------------------*/

/**
 * opt_get_best_fitness - return best fitness from last or current run
 */
double opt_get_best_fitness(void)
{
	if (active_session == NULL)
	{
		return INFINITY;
	}

	return active_session->best_fitness;
}

/*------------------------------------------------------------------------*/

/**
 * opt_get_log_state - return most recent log state snapshot
 */
const simple_log_state_t *opt_get_log_state(void)
{
	if (active_session == NULL || !active_session->has_log)
	{
		return NULL;
	}

	return &active_session->last_log;
}

/*------------------------------------------------------------------------*/

/**
 * opt_get_best_measurements - copy best-so-far measurements via trylock
 * @meas_out: caller buffer, at least OPT_MAX_FREQ_STEPS entries
 * @freq_out: caller buffer, at least OPT_MAX_FREQ_STEPS entries
 * @num_steps_out: receives number of valid entries copied
 *
 * Non-blocking: uses g_mutex_trylock so the GTK main thread never
 * stalls waiting on the optimizer thread.
 */
gboolean opt_get_best_measurements(measurement_t *meas_out,
	double *freq_out, int *num_steps_out)
{
	opt_session_t *s = active_session;

	if (s == NULL)
	{
		return FALSE;
	}

	if (!g_mutex_trylock(&s->best_lock))
	{
		return FALSE;
	}

	if (!s->has_best_meas || s->best_num_steps <= 0)
	{
		g_mutex_unlock(&s->best_lock);
		return FALSE;
	}

	int n = s->best_num_steps;

	memcpy(meas_out, s->best_meas, n * sizeof(measurement_t));
	memcpy(freq_out, s->best_freq, n * sizeof(double));
	*num_steps_out = n;

	g_mutex_unlock(&s->best_lock);
	return TRUE;
}

/*------------------------------------------------------------------------*/

/**
 * opt_get_result - return result variables from completed optimization
 */
const simple_var_t *opt_get_result(int *num_vars)
{
	if (active_session == NULL || active_session->simple == NULL)
	{
		*num_vars = 0;
		return NULL;
	}

	return simple_get_result(active_session->simple, num_vars);
}

/*------------------------------------------------------------------------*/
