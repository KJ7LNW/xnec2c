/*
 *  Named-variable optimizer - public API.
 *
 *  Config init, session creation/destruction, multi-pass optimize loop,
 *  result extraction, and mutator functions.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#include "opt_simple_internal.h"
#include "../console.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Allocate work vars ---- */

/**
 * _alloc_work_vars - allocate pre-sized work_vars array
 * @s: session handle
 *
 * Each work_var has same name and values->size as the config var,
 * but values are a fresh allocation. Other fields point to config
 * var fields (shared, not copied).
 */
static void _alloc_work_vars(simple_t *s)
{
	s->work_vars = calloc(s->num_vars, sizeof(simple_var_t));

	for (int i = 0; i < s->num_vars; i++)
	{
		s->work_vars[i].name   = s->vars[i].name;
		s->work_vars[i].values = gsl_vector_alloc(s->vars[i].values->size);

		/* Share attribute pointers — work_vars are transient per-call */
		s->work_vars[i].enabled       = s->vars[i].enabled;
		s->work_vars[i].min           = s->vars[i].min;
		s->work_vars[i].max           = s->vars[i].max;
		s->work_vars[i].perturb_scale = s->vars[i].perturb_scale;
		s->work_vars[i].round_each    = s->vars[i].round_each;
		s->work_vars[i].round_result  = s->vars[i].round_result;
	}
}

/**
 * _free_work_vars - free work_vars array
 * @s: session handle
 *
 * Frees only the values vector (other fields are shared references).
 */
static void _free_work_vars(simple_t *s)
{
	if (!s->work_vars)
	{
		return;
	}

	for (int i = 0; i < s->num_vars; i++)
	{
		if (s->work_vars[i].values)
		{
			gsl_vector_free(s->work_vars[i].values);
		}
	}

	free(s->work_vars);
	s->work_vars = NULL;
}

/* ---- Single-pass backend construction ---- */

/**
 * _build_simplex_optimizer - create simplex backend for one pass
 * @s: session handle
 * @ssize: simplex size for this pass
 * @initial: initial guess vector (owned by caller)
 *
 * Builds simplex_config_t, installs trampolines, returns optimizer_t.
 */
static optimizer_t *_build_simplex_optimizer(simple_t *s, double ssize,
	gsl_vector *initial)
{
	gsl_vector *bmin, *bmax;
	simple_compute_packed_bounds(s, &bmin, &bmax);

	/* Copy pre-initialized config, set per-pass fields */
	simplex_config_t cfg = s->algo_opts.simplex.cfg;
	cfg.dimensions     = s->total_dims;
	cfg.initial_guess  = initial;
	cfg.init_size      = ssize;
	cfg.max_iter       = s->max_iter;
	cfg.pos_min        = bmin;
	cfg.pos_max        = bmax;
	cfg.exit_fit       = s->exit_fit;
	cfg.fit_func       = simple_fitness_trampoline;
	cfg.fit_func_ctx   = s;
	cfg.log_func       = simple_simplex_log_trampoline;
	cfg.log_func_ctx   = s;

	optimizer_t *opt = optimizer_new_simplex(&cfg);

	/* Backend deep-copies bounds; free our temporaries */
	gsl_vector_free(bmin);
	gsl_vector_free(bmax);

	return opt;
}

/**
 * _build_pso_optimizer - create PSO backend for one pass
 * @s: session handle
 * @initial: initial guess vector (owned by caller)
 *
 * Builds pso_config_t, installs trampolines, returns optimizer_t.
 */
static optimizer_t *_build_pso_optimizer(simple_t *s, gsl_vector *initial)
{
	gsl_vector *bmin, *bmax;
	simple_compute_packed_bounds(s, &bmin, &bmax);

	/* Copy pre-initialized config, set per-pass fields */
	pso_config_t cfg   = s->algo_opts.pso_cfg;
	cfg.dimensions     = s->total_dims;
	cfg.initial_guess  = initial;
	cfg.iterations     = s->max_iter;
	cfg.pos_min        = bmin;
	cfg.pos_max        = bmax;
	cfg.exit_fit       = s->exit_fit;
	cfg.exit_plateau   = 0;
	cfg.fit_func       = simple_fitness_trampoline;
	cfg.fit_func_ctx   = s;
	cfg.log_func       = simple_pso_log_trampoline;
	cfg.log_func_ctx   = s;
	cfg.cancel_flag    = &s->cancel;

	optimizer_t *opt = optimizer_new_pso(&cfg);

	/* Backend deep-copies bounds; free our temporaries */
	gsl_vector_free(bmin);
	gsl_vector_free(bmax);

	return opt;
}

/**
 * _optimize_single_pass - run one optimization pass
 * @s: session handle
 *
 * Builds backend, runs optimizer, frees backend.
 */
static void _optimize_single_pass(simple_t *s)
{
	gsl_vector *initial = simple_pack_vars(s);

	s->cancel = 0;
	s->prev_minima = INFINITY;
	s->prev_minima_count = 0;

	/* Seed best on first pass by evaluating initial point */
	if (s->optimization_pass == 1)
	{
		double seed_fit = simple_fitness_trampoline(initial, s);
		(void)seed_fit;
	}

	/* Build backend */
	optimizer_t *opt = NULL;

	switch (s->algorithm)
	{
		case OPT_SIMPLEX:
		{
			double ssize = 1.0;
			int nsz = s->algo_opts.simplex.num_ssize;
			if (nsz > 0)
			{
				int pass_idx = s->optimization_pass - 1;
				if (pass_idx < nsz)
				{
					ssize = s->algo_opts.simplex.ssize[pass_idx];
				}
				else
				{
					ssize = s->algo_opts.simplex.ssize[nsz - 1];
				}
			}
			opt = _build_simplex_optimizer(s, ssize, initial);
			break;
		}

		case OPT_PSO:
			opt = _build_pso_optimizer(s, initial);
			break;

		default:
			fprintf(stderr, "simple: unknown algorithm %d\n", s->algorithm);
			gsl_vector_free(initial);
			return;
	}

	gsl_vector_free(initial);

	if (!opt)
	{
		fprintf(stderr, "simple: backend creation failed\n");
		return;
	}

	/* Run optimization */
	optimizer_optimize(opt);

	optimizer_free(opt);
}

/**
 * _update_vars_from_best - copy best_vars values into s->vars
 * @s: session handle
 *
 * Used between multi-pass iterations to seed next pass.
 */
static void _update_vars_from_best(simple_t *s)
{
	if (!s->best_vars)
	{
		return;
	}

	for (int i = 0; i < s->num_vars; i++)
	{
		gsl_vector_memcpy(s->vars[i].values, s->best_vars[i].values);
	}
}

/**
 * _populate_result - build result_vars from best_vars with round_result
 * @s: session handle
 */
static void _populate_result(simple_t *s)
{
	/* Free previous result */
	if (s->result_vars)
	{
		for (int i = 0; i < s->num_result_vars; i++)
		{
			simple_var_free_contents(&s->result_vars[i]);
		}
		free(s->result_vars);
	}

	/* Deep-copy best_vars (or vars if no optimization occurred) */
	const simple_var_t *source = s->best_vars ? s->best_vars : s->vars;

	s->result_vars = calloc(s->num_vars, sizeof(simple_var_t));
	s->num_result_vars = s->num_vars;

	for (int i = 0; i < s->num_vars; i++)
	{
		simple_var_deep_copy(&s->result_vars[i], &source[i]);
	}

	simple_apply_round_result(s->result_vars, s->num_result_vars);
}

/* ---- Public API ---- */

/**
 * simple_config_init - fill config with safe defaults
 * @cfg: config struct to initialize
 * @algo: algorithm selector
 */
void simple_config_init(simple_config_t *cfg, enum optimizer_algo algo)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->algorithm = algo;
	cfg->exit_fit = NAN;

	/* Initialize backend config with defaults */
	switch (algo)
	{
		case OPT_SIMPLEX:
			simplex_config_init(&cfg->opts.simplex.cfg);
			break;

		case OPT_PSO:
			pso_config_init(&cfg->opts.pso_cfg);
			break;

		default:
			pr_err("simple_config_init: unknown algorithm %d\n", algo);
			break;
	}
}

/**
 * simple_new - create optimization session from config
 * @cfg: fully populated config
 *
 * Validates all vars, deep-copies config, builds index map,
 * allocates work structures.  Returns NULL on error.
 */
simple_t *simple_new(const simple_config_t *cfg)
{
	if (!cfg->fit_func)
	{
		fprintf(stderr, "simple_new: fit_func is required\n");
		return NULL;
	}

	if (!cfg->vars || cfg->num_vars <= 0)
	{
		fprintf(stderr, "simple_new: vars is NULL or num_vars <= 0\n");
		return NULL;
	}

	/* Validate each var */
	for (int i = 0; i < cfg->num_vars; i++)
	{
		if (simple_validate_var(&cfg->vars[i], i) != 0)
		{
			return NULL;
		}
	}

	/* Validate simplex ssize values */
	if (cfg->algorithm == OPT_SIMPLEX && cfg->opts.simplex.num_ssize > 0)
	{
		for (int i = 0; i < cfg->opts.simplex.num_ssize; i++)
		{
			if (cfg->opts.simplex.ssize[i] <= 0.0)
			{
				fprintf(stderr, "simple_new: ssize[%d] = %g must be > 0\n",
					i, cfg->opts.simplex.ssize[i]);
				return NULL;
			}
		}
	}


	simple_t *s = calloc(1, sizeof(*s));
	if (!s)
	{
		return NULL;
	}

	/* Copy scalars */
	s->algorithm = cfg->algorithm;
	s->num_vars  = cfg->num_vars;
	s->max_iter  = cfg->max_iter > 0 ? cfg->max_iter : 1000;
	s->exit_fit  = cfg->exit_fit;
	s->nocache   = cfg->nocache;
	s->stagnant_minima_count     = cfg->stagnant_minima_count;
	s->stagnant_minima_tolerance = cfg->stagnant_minima_tolerance;

	s->fit_func     = cfg->fit_func;
	s->fit_func_ctx = cfg->fit_func_ctx;
	s->log_func     = cfg->log_func;
	s->log_func_ctx = cfg->log_func_ctx;

	/* Seed random number generator */
	if (cfg->srand_seed != 0)
	{
		s->srand_seed = cfg->srand_seed;
	}
	else
	{
		s->srand_seed = (int)(time(NULL) % 1000000000);
	}
	srand(s->srand_seed);

	/* Deep-copy vars */
	s->vars = calloc(s->num_vars, sizeof(simple_var_t));
	for (int i = 0; i < s->num_vars; i++)
	{
		simple_var_deep_copy(&s->vars[i], &cfg->vars[i]);
	}

	/* Algorithm-specific config */
	switch (s->algorithm)
	{
		case OPT_SIMPLEX:
		{
			/* Deep-copy backend config */
			s->algo_opts.simplex.cfg = cfg->opts.simplex.cfg;

			/* Deep-copy ssize array */
			if (cfg->opts.simplex.num_ssize > 0 && cfg->opts.simplex.ssize)
			{
				s->algo_opts.simplex.num_ssize = cfg->opts.simplex.num_ssize;
				s->algo_opts.simplex.ssize = malloc(
					s->algo_opts.simplex.num_ssize * sizeof(double));
				memcpy(s->algo_opts.simplex.ssize, cfg->opts.simplex.ssize,
					s->algo_opts.simplex.num_ssize * sizeof(double));
			}
			else
			{
				/* Default: single pass with ssize=1.0 */
				s->algo_opts.simplex.num_ssize = 1;
				s->algo_opts.simplex.ssize = malloc(sizeof(double));
				s->algo_opts.simplex.ssize[0] = 1.0;
			}

			/* Stagnation tolerance defaults to simplex convergence threshold */
			if (s->stagnant_minima_tolerance == 0.0)
			{
				s->stagnant_minima_tolerance = s->algo_opts.simplex.cfg.min_size;
			}
			break;
		}

		case OPT_PSO:
		{
			/* Deep-copy backend config */
			s->algo_opts.pso_cfg = cfg->opts.pso_cfg;

			/* Stagnation tolerance defaults to a small value for PSO */
			if (s->stagnant_minima_tolerance == 0.0)
			{
				s->stagnant_minima_tolerance = 1e-6;
			}
			break;
		}

		default:
			fprintf(stderr, "simple_new: unknown algorithm %d\n", s->algorithm);
			simple_free(s);
			return NULL;
	}

	/* Build index map and work vars */
	simple_build_index_map(s);

	if (s->total_dims == 0)
	{
		fprintf(stderr, "simple_new: no enabled dimensions across all vars\n");
		simple_free(s);
		return NULL;
	}

	_alloc_work_vars(s);

	/* Initialize best tracking */
	s->best_minima = INFINITY;
	s->best_pass   = 0;
	s->prev_minima = INFINITY;

	return s;
}

/**
 * simple_optimize - run optimization with multi-pass if configured
 * @s: session handle
 *
 * Resets counters, iterates ssize passes for simplex (single pass
 * for PSO), tracks best across passes, populates result.
 */
double simple_optimize(simple_t *s)
{
	/* Reset state */
	s->optimization_pass = 1;
	s->log_count  = 0;
	s->iter_count = 0;
	s->best_minima = INFINITY;
	s->best_pass = 0;
	s->cache_hits = 0;
	s->cache_misses = 0;
	s->prev_time = 0.0;
	s->cancel = 0;
	s->user_cancel = 0;

	/* Free previous best_vars */
	if (s->best_vars)
	{
		for (int i = 0; i < s->num_vars; i++)
		{
			simple_var_free_contents(&s->best_vars[i]);
		}
		free(s->best_vars);
		s->best_vars = NULL;
	}

	if (s->best_vec)
	{
		gsl_vector_free(s->best_vec);
		s->best_vec = NULL;
	}

	simple_cache_clear(&s->cache);

	/* Multi-pass loop */
	int num_passes = (s->algorithm == OPT_SIMPLEX)
		? s->algo_opts.simplex.num_ssize : 1;

	for (int pass = 0; pass < num_passes; pass++)
	{
		s->optimization_pass = pass + 1;
		s->log_count = 0;

		_optimize_single_pass(s);

		if (s->user_cancel)
		{
			break;
		}

		/* Feed best result into next pass */
		if (pass < num_passes - 1)
		{
			_update_vars_from_best(s);
		}
	}

	_populate_result(s);

	return s->best_minima;
}

/**
 * simple_get_result - return result vars with round_result applied
 * @s: session handle
 * @num_vars: output, set to number of vars
 *
 * Owned by simple_t; do not free.
 */
const simple_var_t *simple_get_result(const simple_t *s, int *num_vars)
{
	if (num_vars)
	{
		*num_vars = s->num_result_vars;
	}

	return s->result_vars;
}

/**
 * simple_get_best_fit - return best fitness across all passes
 * @s: session handle
 */
double simple_get_best_fit(const simple_t *s)
{
	return s->best_minima;
}

/**
 * simple_set_vars - replace vars and rebuild internals
 * @s: session handle
 * @vars: new variable array (deep-copied)
 * @num_vars: length
 */
void simple_set_vars(simple_t *s, const simple_var_t *vars, int num_vars)
{
	/* Free old vars */
	for (int i = 0; i < s->num_vars; i++)
	{
		simple_var_free_contents(&s->vars[i]);
	}
	free(s->vars);
	_free_work_vars(s);

	/* Deep-copy new vars */
	s->num_vars = num_vars;
	s->vars = calloc(num_vars, sizeof(simple_var_t));
	for (int i = 0; i < num_vars; i++)
	{
		simple_var_deep_copy(&s->vars[i], &vars[i]);
	}

	/* Rebuild index map and work vars */
	simple_build_index_map(s);
	_alloc_work_vars(s);
}

/**
 * simple_set_ssize - replace ssize with single value
 * @s: session handle
 * @ssize: new simplex size
 */
void simple_set_ssize(simple_t *s, double ssize)
{
	free(s->algo_opts.simplex.ssize);
	s->algo_opts.simplex.num_ssize = 1;
	s->algo_opts.simplex.ssize = malloc(sizeof(double));
	s->algo_opts.simplex.ssize[0] = ssize;
}

/**
 * simple_scale_ssize - multiply all ssize entries by scale
 * @s: session handle
 * @scale: multiplier
 */
void simple_scale_ssize(simple_t *s, double scale)
{
	for (int i = 0; i < s->algo_opts.simplex.num_ssize; i++)
	{
		s->algo_opts.simplex.ssize[i] *= scale;
	}
}

/**
 * simple_cancel - request early termination
 * @s: session handle
 */
void simple_cancel(simple_t *s)
{
	s->user_cancel = 1;
	s->cancel = 1;
}

/**
 * simple_free - release session and all resources
 * @s: session handle (NULL safe)
 */
void simple_free(simple_t *s)
{
	if (!s)
	{
		return;
	}

	/* Config vars */
	if (s->vars)
	{
		for (int i = 0; i < s->num_vars; i++)
		{
			simple_var_free_contents(&s->vars[i]);
		}
		free(s->vars);
	}

	/* Work vars */
	_free_work_vars(s);

	/* Best vars */
	if (s->best_vars)
	{
		for (int i = 0; i < s->num_vars; i++)
		{
			simple_var_free_contents(&s->best_vars[i]);
		}
		free(s->best_vars);
	}

	if (s->best_vec)
	{
		gsl_vector_free(s->best_vec);
	}

	/* Result vars */
	if (s->result_vars)
	{
		for (int i = 0; i < s->num_result_vars; i++)
		{
			simple_var_free_contents(&s->result_vars[i]);
		}
		free(s->result_vars);
	}

	/* Ssize array (simplex only; PSO union member overlaps, no action needed) */
	if (s->algorithm == OPT_SIMPLEX)
	{
		free(s->algo_opts.simplex.ssize);
	}

	/* Index map */
	free(s->map_var);
	free(s->map_elem);
	free(s->sorted_var_indices);

	/* Cache */
	simple_cache_clear(&s->cache);

	free(s);
}
