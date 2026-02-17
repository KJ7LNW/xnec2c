/*
 *  Particle Swarm Optimization - internal engine operations.
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

#include <gsl/gsl_blas.h>
#include <math.h>
#include <stdio.h>

/**
 * pso_rand_in_range - uniform random in [min, max]
 * @pso: optimizer (owns the RNG)
 * @min: lower bound
 * @max: upper bound
 */
double pso_rand_in_range(pso_t *pso, double min, double max)
{
	return min + (max - min) * gsl_rng_uniform(pso->rng);
}

/**
 * pso_calc_fit - evaluate fitness, converting NaN to INFINITY
 * @pso: optimizer
 * @pos: position vector [dimensions]
 */
double pso_calc_fit(const pso_t *pso, const gsl_vector *pos)
{
	double f = pso->config.fit_func(pos, pso->config.fit_func_ctx);
	if (isnan(f))
	{
		return INFINITY;
	}
	return f;
}

/**
 * pso_get_best_neighbour - find neighbor with lowest bestFit
 * @pso: optimizer
 * @me: index of the particle whose neighborhood to search
 *
 * Examines numNeighbors particles starting at index (me+1) mod numParticles.
 */
int pso_get_best_neighbour(const pso_t *pso, int me)
{
	int np = pso->config.num_particles;
	int nn = pso->config.num_neighbors;
	double best = INFINITY;
	int best_idx = (me + 1) % np;

	for (int k = 0; k < nn; k++)
	{
		int idx = (k + me + 1) % np;
		double f = gsl_vector_get(pso->prtcls->best_fit, idx);
		if (f < best)
		{
			best = f;
			best_idx = idx;
		}
	}

	return best_idx;
}

/**
 * pso_calc_next_pos - compute next positions and evaluate fitness
 * @pso: optimizer
 *
 * For each particle: nextPos = currPos + velocity, then clip to bounds
 * and zero the velocity component for any dimension that hit a wall.
 */
void pso_calc_next_pos(pso_t *pso)
{
	int dims = pso->config.dimensions;
	int np = pso->config.num_particles;
	pso_particles_t *p = pso->prtcls;

	for (int i = 0; i < np; i++)
	{
		for (int d = 0; d < dims; d++)
		{
			double pos = gsl_matrix_get(p->curr_pos, d, i)
				+ gsl_matrix_get(p->velocity, d, i);
			double lo = gsl_vector_get(pso->pos_min, d);
			double hi = gsl_vector_get(pso->pos_max, d);

			/* Zero velocity on out-of-bounds, then clip */
			if (pos < lo || pos > hi)
			{
				gsl_matrix_set(p->velocity, d, i, 0.0);
				if (pos < lo)
				{
					pos = lo;
				}
				else
				{
					pos = hi;
				}
			}
			gsl_matrix_set(p->next_pos, d, i, pos);
		}

		gsl_vector_const_view col = gsl_matrix_const_column(p->next_pos, i);
		gsl_vector_set(p->next_fit, i, pso_calc_fit(pso, &col.vector));
	}
}

/**
 * pso_init_particles - initialize or reinitialize masked particles
 * @pso: optimizer
 * @mask: vector of length numParticles; 1.0 = reinitialize, 0.0 = skip
 *
 * Sets bestPos, currPos, velocity, and evaluates currFit and bestFit
 * for each active particle.  Increments stall counters.
 */
void pso_init_particles(pso_t *pso, const gsl_vector *mask)
{
	int dims = pso->config.dimensions;
	int np = pso->config.num_particles;
	pso_particles_t *p = pso->prtcls;

	for (int i = 0; i < np; i++)
	{
		if (gsl_vector_get(mask, i) < 0.5)
		{
			continue;
		}

		double stalls = gsl_vector_get(p->stalls, i) + 1.0;
		gsl_vector_set(p->stalls, i, stalls);

		/* Set bestPos, currPos, and velocity per dimension */
		double ss = pso->config.search_size
			* pow(pso->config.stall_search_scale, stalls);

		for (int d = 0; d < dims; d++)
		{
			double lo = gsl_vector_get(pso->pos_min, d);
			double hi = gsl_vector_get(pso->pos_max, d);

			/* bestPos: initialGuess or random */
			double bpos;
			if (pso->config.initial_guess)
			{
				bpos = gsl_vector_get(pso->config.initial_guess, d);
			}
			else
			{
				bpos = pso_rand_in_range(pso, lo, hi);
			}
			gsl_matrix_set(p->best_pos, d, i, bpos);

			/* currPos: searchSize-based or random */
			double cpos;
			if (pso->config.search_size >= 0.0)
			{
				double guess;
				if (pso->config.initial_guess && pso->iter_count == 0)
				{
					guess = gsl_vector_get(pso->config.initial_guess, d);
				}
				else
				{
					guess = bpos;
				}

				/* Scale search range by stallSearchScale^stalls */
				double r = pso_rand_in_range(pso, -1.0, 1.0);
				cpos = guess + ss * (hi - lo) * r;
			}
			else
			{
				cpos = pso_rand_in_range(pso, lo, hi);
			}
			gsl_matrix_set(p->curr_pos, d, i, cpos);

			/* velocity: random or zero */
			double vel = 0.0;
			if (pso->config.rand_start_velocity != 0.0)
			{
				double r = pso_rand_in_range(pso, -1.0, 1.0);
				vel = pso->config.rand_start_velocity * (hi - lo) / 100.0 * r;
			}
			gsl_matrix_set(p->velocity, d, i, vel);
		}

		/* Evaluate fitness */
		gsl_vector_const_view curr_col = gsl_matrix_const_column(p->curr_pos, i);
		gsl_vector_set(p->curr_fit, i, pso_calc_fit(pso, &curr_col.vector));

		gsl_vector_const_view best_col = gsl_matrix_const_column(p->best_pos, i);
		gsl_vector_set(p->best_fit, i, pso_calc_fit(pso, &best_col.vector));
	}

}

/**
 * pso_move_particles - advance all particles and update bests
 * @pso: optimizer
 * @iter: current iteration number (for logging)
 * @exit_fit_val: output, set if exitFit threshold was reached
 *
 * Copies nextPos/nextFit to currPos/currFit, then updates each
 * particle's personal best and the global best.
 * Returns 1 if exitFit was reached, 0 otherwise.
 */
int pso_move_particles(pso_t *pso, int iter, double *exit_fit_val)
{
	int np = pso->config.num_particles;
	pso_particles_t *p = pso->prtcls;

	if (pso->config.verbose & PSO_LOG_ITER)
	{
		printf("Iter %d\n", iter);
	}

	gsl_matrix_memcpy(p->curr_pos, p->next_pos);
	gsl_vector_memcpy(p->curr_fit, p->next_fit);

	for (int i = 0; i < np; i++)
	{
		double fit = gsl_vector_get(p->curr_fit, i);
		double bfit = gsl_vector_get(p->best_fit, i);

		if (fit < bfit)
		{
			gsl_vector_set(p->best_fit, i, fit);
			gsl_vector_view dst = gsl_matrix_column(p->best_pos, i);
			gsl_vector_const_view src = gsl_matrix_const_column(p->curr_pos, i);
			gsl_vector_memcpy(&dst.vector, &src.vector);

			if (pso->config.verbose & PSO_LOG_BETTER)
			{
				gsl_vector_const_view v = gsl_matrix_const_column(p->velocity, i);
				printf("#%05d: Particle %d best: %.4f (v: %.5f)\n",
					iter, i, fit, gsl_blas_dnrm2(&v.vector));
			}

			if (isinf(pso->best_best) || fit < pso->best_best)
			{
				pso->best_best = fit;
				gsl_vector_const_view pos = gsl_matrix_const_column(p->curr_pos, i);
				gsl_vector_memcpy(pso->best_best_pos, &pos.vector);
			}
		}

		if (!isnan(pso->config.exit_fit) && fit < pso->config.exit_fit)
		{
			*exit_fit_val = fit;
			return 1;
		}

		if (pso->config.verbose & PSO_LOG_ITER)
		{
			gsl_vector_const_view v = gsl_matrix_const_column(p->velocity, i);
			printf("Part %3d fit %15.2f (vmag=%8.6f)\n",
				i, fit, gsl_blas_dnrm2(&v.vector));
		}
	}

	return 0;
}

/**
 * pso_update_velocities - compute new velocities, handle stalls
 * @pso: optimizer
 * @iter: current iteration (for logging)
 *
 * For each particle, blends inertia, personal-best delta, and
 * neighbor-best delta into the new velocity.  Detects stalled
 * particles and reinitializes them via pso_init_particles().
 */
void pso_update_velocities(pso_t *pso, int iter)
{
	int dims = pso->config.dimensions;
	int np = pso->config.num_particles;
	pso_particles_t *p = pso->prtcls;

	for (int i = 0; i < np; i++)
	{
		int best_n = pso_get_best_neighbour(pso, i);

		for (int d = 0; d < dims; d++)
		{
			double me_r = pso_rand_in_range(pso,
				-pso->config.me_weight, pso->config.me_weight);
			double them_r = pso_rand_in_range(pso,
				-pso->config.them_weight, pso->config.them_weight);

			double curr = gsl_matrix_get(p->curr_pos, d, i);
			double me_delta = gsl_matrix_get(p->best_pos, d, i) - curr;
			double them_delta = gsl_matrix_get(p->best_pos, d, best_n) - curr;
			double vel = gsl_matrix_get(p->velocity, d, i);

			vel = vel * pso->config.inertia + me_r * me_delta + them_r * them_delta;
			gsl_matrix_set(p->velocity, d, i, vel);
		}
	}

	/* Detect stalls and reinitialize */
	gsl_vector *stalled = gsl_vector_calloc(np);
	int any_stalled = 0;

	for (int i = 0; i < np; i++)
	{
		gsl_vector_const_view vel = gsl_matrix_const_column(p->velocity, i);
		double vmag = gsl_blas_dnrm2(&vel.vector);
		if (vmag < pso->config.stall_speed)
		{
			gsl_vector_set(stalled, i, 1.0);
			any_stalled = 1;
		}
	}

	if (any_stalled)
	{
		if (pso->config.verbose & PSO_LOG_STALL)
		{
			for (int i = 0; i < np; i++)
			{
				if (gsl_vector_get(stalled, i) < 0.5)
				{
					continue;
				}
				gsl_vector_const_view vel = gsl_matrix_const_column(p->velocity, i);
				printf("#%05d: Particle %d stalled (count=%.0f v=%.2e)\n",
					iter, i, gsl_vector_get(p->stalls, i),
					gsl_blas_dnrm2(&vel.vector));
			}
		}
		pso_init_particles(pso, stalled);
	}

	gsl_vector_free(stalled);
	pso_calc_next_pos(pso);
}
