/*
 *  Particle Swarm Optimization - public API and main loop.
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

#include "particleswarm_internal.h"
#include "optimizer_bounds.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static pso_particles_t *_alloc_particles(int dims, int np);
static void _free_particles(pso_particles_t *p);
static double _swarm(pso_t *pso, int iterations);

/**
 * pso_config_init - fill config with safe defaults
 * @config: pointer to config struct to initialize
 *
 * Sets all fields to sensible defaults.  Caller must still set
 * fitFunc and either dimensions or initialGuess before calling pso_new().
 */
void pso_config_init(pso_config_t *config)
{
	memset(config, 0, sizeof(*config));
	config->pos_min = NULL;
	config->pos_max = NULL;
	config->search_size = 0.25;
	config->stall_speed = 1e-9;
	config->stall_search_scale = 1.0;

	/* Clerc & Kennedy constriction coefficients */
	config->me_weight = 1.4962;
	config->them_weight = 1.4962;
	config->inertia = 0.7298;
	config->exit_fit = NAN;
}

/**
 * pso_new - create a new PSO optimizer
 * @config: configuration (copied internally)
 *
 * Validates configuration, applies computed defaults, and allocates
 * the optimizer.  Returns NULL with a message to stderr on error.
 */
pso_t *pso_new(const pso_config_t *config)
{
	if (!config->fit_func)
	{
		fprintf(stderr, "pso_new: fitFunc is required\n");
		return NULL;
	}

	if (config->dimensions <= 0 && !config->initial_guess)
	{
		fprintf(stderr, "pso_new: dimensions or initialGuess required\n");
		return NULL;
	}

	/* Determine dimensions early for bounds validation */
	int d = config->dimensions;
	if (d <= 0 && config->initial_guess)
	{
		d = (int)config->initial_guess->size;
	}

	/* PSO requires explicit per-dimension bounds */
	if (!config->pos_min || !config->pos_max)
	{
		fprintf(stderr, "pso_new: pos_min and pos_max are required\n");
		return NULL;
	}

	if (optimizer_validate_bounds(config->pos_min, config->pos_max,
		d, 0, "pso_new") != 0)
	{
		return NULL;
	}

	pso_t *pso = calloc(1, sizeof(pso_t));
	if (!pso)
	{
		return NULL;
	}

	pso->config = *config;

	/* Prevent pso_free from freeing caller's pointers before deep-copy */
	pso->config.initial_guess = NULL;
	pso->config.pos_min = NULL;
	pso->config.pos_max = NULL;

	/* Use pre-validated dimensions */
	pso->config.dimensions = d;

	/* Deep-copy initialGuess so caller can free theirs */
	if (config->initial_guess)
	{
		pso->config.initial_guess = gsl_vector_alloc(config->initial_guess->size);
		gsl_vector_memcpy(pso->config.initial_guess, config->initial_guess);
	}

	/* Computed defaults for swarm topology */
	if (pso->config.num_particles <= 0)
	{
		pso->config.num_particles = d * 10;
	}
	if (pso->config.num_neighbors <= 0)
	{
		pso->config.num_neighbors = (int)sqrt((double)pso->config.num_particles);
	}
	if (pso->config.num_neighbors >= pso->config.num_particles)
	{
		pso->config.num_neighbors = pso->config.num_particles - 1;
	}
	if (pso->config.iterations <= 0)
	{
		pso->config.iterations = 1000;
	}

	/* Plateau detection defaults */
	if (pso->config.exit_plateau_dp <= 0)
	{
		pso->config.exit_plateau_dp = 10;
	}
	if (pso->config.exit_plateau_window <= 0)
	{
		pso->config.exit_plateau_window = (int)(pso->config.iterations * 0.1);
	}
	if (pso->config.exit_plateau_burnin <= 0)
	{
		pso->config.exit_plateau_burnin = (int)(pso->config.iterations * 0.5);
	}

	/* Deep-copy per-dimension bounds (validated non-NULL above) */
	optimizer_deep_copy_bounds(&pso->config.pos_min, &pso->config.pos_max,
		config->pos_min, config->pos_max);

	pso->best_best_pos = gsl_vector_alloc(d);
	pso->rng = gsl_rng_alloc(gsl_rng_mt19937);
	gsl_rng_set(pso->rng, (unsigned long)time(NULL));

	/* Plateau circular buffer */
	if (pso->config.exit_plateau)
	{
		pso->best_best_by_iter = calloc(pso->config.exit_plateau_window, sizeof(double));
	}

	pso->best_best = INFINITY;

	return pso;
}

/**
 * pso_init - initialize or reinitialize the particle swarm
 * @pso: optimizer handle
 *
 * Allocates particle arrays and sets initial positions, velocities,
 * and fitness values.  Called automatically by pso_optimize() if needed.
 */
void pso_init(pso_t *pso)
{
	int d = pso->config.dimensions;
	int np = pso->config.num_particles;

	_free_particles(pso->prtcls);
	pso->prtcls = _alloc_particles(d, np);

	pso->best_best = INFINITY;
	pso->bests_mean = 0.0;
	pso->iter_count = 0;

	if (pso->best_best_by_iter)
	{
		memset(pso->best_best_by_iter, 0, pso->config.exit_plateau_window * sizeof(double));
	}

	/* Initialize all particles */
	gsl_vector *mask = gsl_vector_alloc(np);
	gsl_vector_set_all(mask, 1.0);
	pso_init_particles(pso, mask);
	pso_calc_next_pos(pso);
	gsl_vector_free(mask);

	/* Seed bestBest from particle with lowest bestFit so getters
	 * never encounter an unset state after init completes. */
	for (int i = 0; i < np; i++)
	{
		double f = gsl_vector_get(pso->prtcls->best_fit, i);
		if (f < pso->best_best)
		{
			pso->best_best = f;
			gsl_vector_const_view col = gsl_matrix_const_column(pso->prtcls->best_pos, i);
			gsl_vector_memcpy(pso->best_best_pos, &col.vector);
		}
	}

	pso->initialized = 1;
}

/**
 * _swarm - main optimization loop
 * @pso: optimizer handle
 * @iterations: number of iterations to run
 *
 * Runs the PSO update cycle: move particles, update velocities,
 * check plateau exit condition.  Returns best fitness found.
 */
static double _swarm(pso_t *pso, int iterations)
{
	for (int iter = 1; iter <= iterations; iter++)
	{
		/* Check external cancellation flag */
		if (pso->config.cancel_flag != NULL && *pso->config.cancel_flag)
		{
			break;
		}

		pso->iter_count++;

		double exit_val;
		if (pso_move_particles(pso, iter, &exit_val))
		{
			break;
		}

		pso_update_velocities(pso, iter);

		if (pso->config.log_func)
		{
			pso->config.log_func(
				pso_get_best_pos(pso),
				pso_get_best_fit(pso),
				pso->config.log_func_ctx);
		}

		/* Plateau detection */
		if (!pso->config.exit_plateau || isinf(pso->best_best))
		{
			continue;
		}

		int burnin = pso->config.exit_plateau_burnin;
		int window = pso->config.exit_plateau_window;

		if (iter >= burnin - window)
		{
			int i = iter % window;
			pso->bests_mean -= pso->best_best_by_iter[i];
			pso->best_best_by_iter[i] = pso->best_best / window;
			pso->bests_mean += pso->best_best_by_iter[i];
		}

		if (iter <= burnin)
		{
			continue;
		}

		/* Compare at specified decimal precision */
		char fmt[16];
		snprintf(fmt, sizeof(fmt), "%%.%df", pso->config.exit_plateau_dp);

		char mean_str[64], curr_str[64];
		snprintf(mean_str, sizeof(mean_str), fmt, pso->bests_mean);
		snprintf(curr_str, sizeof(curr_str), fmt, pso->best_best);

		if (strcmp(mean_str, curr_str) == 0)
		{
			break;
		}
	}

	return pso_get_best_fit(pso);
}

/**
 * pso_optimize - run the optimization
 * @pso: optimizer handle
 *
 * Calls pso_init() if not already initialized, then runs the swarm.
 * May be called repeatedly to continue from where the last run stopped.
 */
double pso_optimize(pso_t *pso)
{
	if (!pso->initialized)
	{
		pso_init(pso);
	}

	return _swarm(pso, pso->config.iterations);
}

/** pso_get_best_pos - return best position found */
const gsl_vector *pso_get_best_pos(const pso_t *pso)
{
	return pso->best_best_pos;
}

/** pso_get_best_fit - return best fitness value found */
double pso_get_best_fit(const pso_t *pso)
{
	return pso->best_best;
}

/** pso_get_iteration_count - return total iterations performed */
int pso_get_iteration_count(const pso_t *pso)
{
	return pso->iter_count;
}

/**
 * pso_free - release all resources
 * @pso: optimizer handle (may be NULL)
 */
void pso_free(pso_t *pso)
{
	if (!pso)
	{
		return;
	}

	_free_particles(pso->prtcls);
	gsl_vector_free(pso->best_best_pos);

	if (pso->config.pos_min)
	{
		gsl_vector_free(pso->config.pos_min);
	}

	if (pso->config.pos_max)
	{
		gsl_vector_free(pso->config.pos_max);
	}

	if (pso->config.initial_guess)
	{
		gsl_vector_free(pso->config.initial_guess);
	}

	if (pso->rng)
	{
		gsl_rng_free(pso->rng);
	}

	free(pso->best_best_by_iter);
	free(pso);
}

/** Allocate particle arrays for dims dimensions and np particles */
static pso_particles_t *_alloc_particles(int dims, int np)
{
	pso_particles_t *p = calloc(1, sizeof(pso_particles_t));
	if (!p)
	{
		return NULL;
	}

	p->best_pos  = gsl_matrix_calloc(dims, np);
	p->curr_pos  = gsl_matrix_calloc(dims, np);
	p->next_pos  = gsl_matrix_calloc(dims, np);
	p->velocity = gsl_matrix_calloc(dims, np);

	p->best_fit = gsl_vector_alloc(np);
	p->curr_fit = gsl_vector_alloc(np);
	p->next_fit = gsl_vector_alloc(np);
	p->stalls  = gsl_vector_alloc(np);

	/* Initialize fitness to infinity, stalls to -1 */
	gsl_vector_set_all(p->best_fit, INFINITY);
	gsl_vector_set_all(p->curr_fit, INFINITY);
	gsl_vector_set_all(p->next_fit, INFINITY);
	gsl_vector_set_all(p->stalls, -1.0);

	return p;
}

/** Free particle arrays */
static void _free_particles(pso_particles_t *p)
{
	if (!p)
	{
		return;
	}

	gsl_matrix_free(p->best_pos);
	gsl_matrix_free(p->curr_pos);
	gsl_matrix_free(p->next_pos);
	gsl_matrix_free(p->velocity);

	gsl_vector_free(p->best_fit);
	gsl_vector_free(p->curr_fit);
	gsl_vector_free(p->next_fit);
	gsl_vector_free(p->stalls);

	free(p);
}
