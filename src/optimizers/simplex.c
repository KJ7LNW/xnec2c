/*
 *  Nelder-Mead Simplex Optimization - public API and main loop.
 *
 *  Copyright (C) 2023- eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "simplex_internal.h"
#include "optimizer_bounds.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double _nelder_mead(simplex_t *s);

/**
 * simplex_config_init - fill config with safe defaults
 * @config: pointer to config struct to initialize
 *
 * Sets all fields to sensible defaults.  Caller must still set
 * fitFunc and either initialGuess or initialSimplex before calling
 * simplex_new().
 */
void simplex_config_init(simplex_config_t *config)
{
	memset(config, 0, sizeof(*config));
	config->init_size = 1.0;
	config->min_size = 1e-6;
	config->pos_min = NULL;
	config->pos_max = NULL;
	config->exit_fit = NAN;
}

/**
 * simplex_new - create a new simplex optimizer
 * @config: configuration (copied internally)
 *
 * Validates configuration, applies computed defaults, and allocates
 * the optimizer.  Returns NULL with a message to stderr on error.
 */
simplex_t *simplex_new(const simplex_config_t *config)
{
	if (!config->fit_func)
	{
		fprintf(stderr, "simplex_new: fit_func is required\n");
		return NULL;
	}

	if (!config->initial_guess && !config->initial_simplex)
	{
		fprintf(stderr, "simplex_new: initial_guess or initial_simplex required\n");
		return NULL;
	}

	/* Derive dimensions early for bounds validation */
	int dims = config->dimensions;
	if (dims <= 0 && config->initial_guess)
	{
		dims = (int)config->initial_guess->size;
	}
	else if (dims <= 0 && config->initial_simplex)
	{
		dims = (int)config->initial_simplex->size1;
	}

	if (dims <= 0)
	{
		fprintf(stderr, "simplex_new: cannot determine dimensions\n");
		return NULL;
	}

	/* Bounds: both or neither */
	if ((config->pos_min != NULL) != (config->pos_max != NULL))
	{
		fprintf(stderr, "simplex_new: pos_min and pos_max must both be set or both NULL\n");
		return NULL;
	}

	if (config->pos_min && config->pos_max)
	{
		if (optimizer_validate_bounds(config->pos_min, config->pos_max,
			dims, 1, "simplex_new") != 0)
		{
			return NULL;
		}
	}

	/* Validate initial_simplex shape */
	if (config->initial_simplex)
	{
		if ((int)config->initial_simplex->size1 != dims
			|| (int)config->initial_simplex->size2 != dims + 1)
		{
			fprintf(stderr,
				"simplex_new: initial_simplex must be [%d x %d], got [%zu x %zu]\n",
				dims, dims + 1,
				config->initial_simplex->size1, config->initial_simplex->size2);
			return NULL;
		}
	}

	simplex_t *s = calloc(1, sizeof(simplex_t));
	if (!s)
	{
		return NULL;
	}

	s->config = *config;

	/* Prevent simplex_free from freeing caller's pointers before deep-copy */
	s->config.initial_guess = NULL;
	s->config.initial_simplex = NULL;
	s->config.pos_min = NULL;
	s->config.pos_max = NULL;

	s->config.dimensions = dims;

	if (s->config.max_iter <= 0)
	{
		s->config.max_iter = 1000;
	}

	/* Deep-copy initial_guess */
	if (config->initial_guess)
	{
		s->config.initial_guess = gsl_vector_alloc(config->initial_guess->size);
		gsl_vector_memcpy(s->config.initial_guess, config->initial_guess);
	}

	/* Deep-copy initial_simplex */
	if (config->initial_simplex)
	{
		s->config.initial_simplex = gsl_matrix_alloc(
			config->initial_simplex->size1,
			config->initial_simplex->size2);
		gsl_matrix_memcpy(s->config.initial_simplex, config->initial_simplex);
	}

	/* Deep-copy bounds, or default to unbounded */
	if (config->pos_min && config->pos_max)
	{
		optimizer_deep_copy_bounds(&s->config.pos_min, &s->config.pos_max,
			config->pos_min, config->pos_max);
	}
	else
	{
		s->config.pos_min = gsl_vector_alloc(dims);
		gsl_vector_set_all(s->config.pos_min, -INFINITY);
		s->config.pos_max = gsl_vector_alloc(dims);
		gsl_vector_set_all(s->config.pos_max, INFINITY);
	}

	/* Scratch vectors */
	s->centroid = gsl_vector_alloc(dims);
	s->trial = gsl_vector_alloc(dims);
	s->trial2 = gsl_vector_alloc(dims);

	/* Persistent best tracking */
	s->best_pos = gsl_vector_alloc(dims);
	s->best_fit = INFINITY;

	/* Simplex matrix and fitness vector */
	int nv = dims + 1;
	s->simp = gsl_matrix_alloc(dims, nv);
	s->vals = gsl_vector_alloc(nv);

	/* RNG only needed for simulated annealing */
	if (config->temperature > 0.0)
	{
		s->rng = gsl_rng_alloc(gsl_rng_mt19937);
		gsl_rng_set(s->rng, (unsigned long)time(NULL));
	}

	return s;
}

/**
 * simplex_init - initialize or reinitialize the simplex
 * @s: optimizer handle
 *
 * Builds the initial simplex from config (either from initialGuess
 * + initSize or from a pre-built initialSimplex), evaluates fitness,
 * and computes the initial simplex size.
 */
void simplex_init(simplex_t *s)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;

	s->iter_count = 0;
	s->ssize = 0.0;

	if (s->config.initial_simplex)
	{
		/* Copy pre-built simplex, clamp, evaluate */
		gsl_matrix_memcpy(s->simp, s->config.initial_simplex);
		for (int j = 0; j < nv; j++)
		{
			gsl_vector_view col = gsl_matrix_column(s->simp, j);
			simplex_clamp_to_bounds(s, &col.vector);
			gsl_vector_set(s->vals, j, simplex_calc_fit(s, &col.vector));
		}
	}
	else
	{
		simplex_construct_initial(s);
	}

	s->ssize = simplex_calc_size(s);

	/* Seed best_pos/best_fit from initial simplex */
	s->best_fit = INFINITY;
	for (int j = 0; j < nv; j++)
	{
		double f = gsl_vector_get(s->vals, j);
		if (f < s->best_fit)
		{
			s->best_fit = f;
			gsl_vector_const_view col = gsl_matrix_const_column(s->simp, j);
			gsl_vector_memcpy(s->best_pos, &col.vector);
		}
	}

	s->initialized = 1;
}

/**
 * _nelder_mead - main Nelder-Mead optimization loop
 * @s: optimizer handle
 *
 * Implements the five-case Nelder-Mead algorithm: reflection+expansion,
 * reflection (better than best), reflection (better than 2nd worst),
 * contraction, and multiple contraction (shrink).
 */
static double _nelder_mead(simplex_t *s)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;
	int max_iter = s->config.max_iter;
	double temperature = s->config.temperature;

	/* Perturbed values for SA comparison; points to s->vals when t==0 */
	gsl_vector *cmp_vals = NULL;
	if (temperature > 0.0)
	{
		cmp_vals = gsl_vector_alloc(nv);
	}

	/* Log initial state */
	if (s->config.log_func)
	{
		s->config.log_func(s->simp, s->vals, s->ssize, s->config.log_func_ctx);
	}

	for (int iter = 0; iter < max_iter && s->ssize > s->config.min_size; iter++)
	{
		s->iter_count++;

		/* Build comparison values (with SA noise if enabled) */
		const gsl_vector *vals_for_cmp;
		if (temperature > 0.0)
		{
			for (int j = 0; j < nv; j++)
			{
				double noise = -log(gsl_rng_uniform_pos(s->rng) + 1e-5);
				double v = gsl_vector_get(s->vals, j) + temperature * noise;
				gsl_vector_set(cmp_vals, j, v);
			}
			vals_for_cmp = cmp_vals;
		}
		else
		{
			vals_for_cmp = s->vals;
		}

		int best_idx, worst_idx, worst2_idx;
		simplex_find_extremes(s, vals_for_cmp, &best_idx, &worst_idx, &worst2_idx);

		double best_val = gsl_vector_get(vals_for_cmp, best_idx);
		double worst_val = gsl_vector_get(vals_for_cmp, worst_idx);
		double worst2_val = gsl_vector_get(vals_for_cmp, worst2_idx);

		simplex_calc_centroid(s, worst_idx);

		/* Reflect worst through centroid */
		double val = simplex_reflect(s, worst_idx);
		if (temperature > 0.0)
		{
			val = val - temperature * (-log(gsl_rng_uniform_pos(s->rng) + 1e-5));
		}

		if (val < best_val)
		{
			/* Try expansion */
			double val2 = simplex_expand(s, worst_idx);

			gsl_vector_view worst_col = gsl_matrix_column(s->simp, worst_idx);
			if (val2 < val)
			{
				/* Expansion wins */
				gsl_vector_memcpy(&worst_col.vector, s->trial2);
				gsl_vector_set(s->vals, worst_idx, val2);
			}
			else
			{
				/* Reflection wins */
				gsl_vector_memcpy(&worst_col.vector, s->trial);
				gsl_vector_set(s->vals, worst_idx, val);
			}
		}
		else if (val < worst2_val)
		{
			/* Reflection accepted */
			gsl_vector_view worst_col = gsl_matrix_column(s->simp, worst_idx);
			gsl_vector_memcpy(&worst_col.vector, s->trial);
			gsl_vector_set(s->vals, worst_idx, val);
		}
		else
		{
			/* Try contraction */
			double val2 = simplex_contract(s, worst_idx);
			if (val2 < worst_val)
			{
				/* Contraction accepted */
				gsl_vector_view worst_col = gsl_matrix_column(s->simp, worst_idx);
				gsl_vector_memcpy(&worst_col.vector, s->trial);
				gsl_vector_set(s->vals, worst_idx, val2);
			}
			else
			{
				/* Contraction failed: shrink toward best */
				simplex_shrink(s, best_idx);
			}
		}

		s->ssize = simplex_calc_size(s);

		/* Update persistent best tracking */
		{
			int b_idx, w_idx, w2_idx;
			simplex_find_extremes(s, s->vals, &b_idx, &w_idx, &w2_idx);
			double cur_best = gsl_vector_get(s->vals, b_idx);

			if (cur_best < s->best_fit)
			{
				s->best_fit = cur_best;
				gsl_vector_const_view col = gsl_matrix_const_column(s->simp, b_idx);
				gsl_vector_memcpy(s->best_pos, &col.vector);

				if (s->config.verbose & SIMPLEX_LOG_BETTER)
				{
					printf("#%05d: best=%.6f ssize=%.2e\n",
						s->iter_count, cur_best, s->ssize);
				}
			}

			if (s->config.verbose & SIMPLEX_LOG_ITER)
			{
				printf("#%05d: best=%.6f worst=%.6f ssize=%.2e\n",
					s->iter_count, cur_best,
					gsl_vector_get(s->vals, w_idx), s->ssize);
			}
		}

		if (s->config.log_func)
		{
			s->config.log_func(s->simp, s->vals, s->ssize, s->config.log_func_ctx);
		}

		/* Early termination on fitness threshold */
		if (!isnan(s->config.exit_fit) && s->best_fit <= s->config.exit_fit)
		{
			break;
		}
	}

	if (cmp_vals)
	{
		gsl_vector_free(cmp_vals);
	}

	return simplex_get_best_fit(s);
}

/**
 * simplex_optimize - run the optimization
 * @s: optimizer handle
 *
 * Calls simplex_init() if not already initialized, then runs
 * the Nelder-Mead loop.
 */
double simplex_optimize(simplex_t *s)
{
	if (!s->initialized)
	{
		simplex_init(s);
	}

	return _nelder_mead(s);
}

/**
 * simplex_get_best_pos - return best position found
 * @s: optimizer handle
 *
 * Returns persistent bestPos vector (owned by simplex_t, do not free).
 */
const gsl_vector *simplex_get_best_pos(const simplex_t *s)
{
	return s->best_pos;
}

/**
 * simplex_get_best_fit - return best fitness value found
 * @s: optimizer handle
 */
double simplex_get_best_fit(const simplex_t *s)
{
	return s->best_fit;
}

/**
 * simplex_get_simplex_size - return current simplex size
 * @s: optimizer handle
 */
double simplex_get_simplex_size(const simplex_t *s)
{
	return s->ssize;
}

/**
 * simplex_get_iteration_count - return total iterations performed
 * @s: optimizer handle
 */
int simplex_get_iteration_count(const simplex_t *s)
{
	return s->iter_count;
}

/**
 * simplex_free - release all resources
 * @s: optimizer handle (may be NULL)
 */
void simplex_free(simplex_t *s)
{
	if (!s)
	{
		return;
	}

	gsl_matrix_free(s->simp);
	gsl_vector_free(s->vals);
	gsl_vector_free(s->centroid);
	gsl_vector_free(s->trial);
	gsl_vector_free(s->trial2);
	gsl_vector_free(s->best_pos);

	if (s->config.pos_min)
	{
		gsl_vector_free(s->config.pos_min);
	}

	if (s->config.pos_max)
	{
		gsl_vector_free(s->config.pos_max);
	}

	if (s->config.initial_guess)
	{
		gsl_vector_free(s->config.initial_guess);
	}

	if (s->config.initial_simplex)
	{
		gsl_matrix_free(s->config.initial_simplex);
	}

	if (s->rng)
	{
		gsl_rng_free(s->rng);
	}

	free(s);
}
