/*
 *  Particle Swarm Optimization using GSL.
 *  Ported from PDL::Opt::ParticleSwarm (Perl/PDL).
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

#ifndef PARTICLESWARM_H
#define PARTICLESWARM_H 1

#include <gsl/gsl_vector.h>

/** Log verbosity flags, combinable via bitwise OR */
enum pso_log_flags
{
	PSO_LOG_BETTER = 1,
	PSO_LOG_STALL  = 2,
	PSO_LOG_ITER   = 4
};

/** Fitness function: returns scalar fitness for a position vector */
typedef double (*pso_fit_func_t)(const gsl_vector *pos, void *ctx);

/** Log callback: called after each iteration with current best */
typedef void (*pso_log_func_t)(const gsl_vector *pos, double fit, void *ctx);

/**
 * Configuration for PSO optimizer.
 * Call pso_config_init() first, then override fields as needed.
 */
typedef struct
{
	int dimensions;            /**< Hypersurface dimensions (0 = from initial_guess) */
	int num_particles;         /**< Swarm size (0 = dimensions * 10) */
	int num_neighbors;         /**< Neighborhood size (0 = sqrt(num_particles)) */
	int iterations;            /**< Max iterations (0 = 1000) */

	double pos_min;            /**< Lower bound, uniform across dimensions */
	double pos_max;            /**< Upper bound, uniform across dimensions */

	gsl_vector *initial_guess; /**< Starting position, NULL for random */
	double search_size;        /**< Search range scale around guess, <0 = disabled */

	double stall_speed;        /**< Velocity below which particle is stalled */
	double stall_search_scale; /**< Growth factor for stall reinit search range */

	double me_weight;          /**< Personal best influence coefficient */
	double them_weight;        /**< Neighborhood best influence coefficient */
	double inertia;            /**< Previous velocity carry-over coefficient */

	double exit_fit;           /**< Stop if fitness <= this value (NAN = disabled) */
	int exit_plateau;          /**< Enable plateau detection (nonzero = on) */
	int exit_plateau_dp;       /**< Decimal places for plateau comparison (0 = 10) */
	int exit_plateau_window;   /**< Rolling window size for plateau (0 = iterations/10) */
	int exit_plateau_burnin;   /**< Warmup iterations before plateau check (0 = iterations/2) */

	double rand_start_velocity; /**< Random initial velocity scale (0 = start at rest) */
	int verbose;               /**< Bitfield of enum pso_log_flags */

	pso_fit_func_t fit_func;   /**< Required: fitness evaluation function */
	void *fit_func_ctx;        /**< Opaque context passed to fit_func */

	pso_log_func_t log_func;   /**< Optional: called each iteration with best state */
	void *log_func_ctx;        /**< Opaque context passed to log_func */
} pso_config_t;

/** Opaque PSO optimizer handle */
typedef struct pso_s pso_t;

/** Fill config with default values. Call before setting custom fields. */
void pso_config_init(pso_config_t *config);

/** Create optimizer from config. Returns NULL on invalid config. */
pso_t *pso_new(const pso_config_t *config);

/** Initialize or reinitialize the particle swarm. */
void pso_init(pso_t *pso);

/** Run optimization loop. Returns best fitness found. */
double pso_optimize(pso_t *pso);

/** Return best position found so far (owned by pso_t, do not free). */
const gsl_vector *pso_get_best_pos(const pso_t *pso);

/** Return best fitness value found so far. */
double pso_get_best_fit(const pso_t *pso);

/** Return total iterations performed across all optimize() calls. */
int pso_get_iteration_count(const pso_t *pso);

/** Free optimizer and all associated memory. */
void pso_free(pso_t *pso);

#endif
