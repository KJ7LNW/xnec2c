/*
 *  Nelder-Mead Simplex Optimization using GSL.
 *  Ported from PDL::Opt::Simplex (Perl/PDL).
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

#ifndef SIMPLEX_H
#define SIMPLEX_H 1

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

/** Log verbosity flags, combinable via bitwise OR */
enum simplex_log_flags
{
	SIMPLEX_LOG_BETTER = 1,
	SIMPLEX_LOG_ITER   = 2
};

/** Fitness function: returns scalar fitness for a position vector */
typedef double (*simplex_fit_func_t)(const gsl_vector *pos, void *ctx);

/** Log callback: called after each iteration with full simplex state */
typedef void (*simplex_log_func_t)(const gsl_matrix *simplex,
	const gsl_vector *vals, double ssize, void *ctx);

/**
 * Configuration for Nelder-Mead simplex optimizer.
 * Call simplex_config_init() first, then override fields as needed.
 */
typedef struct
{
	int dimensions;              /**< Hypersurface dimensions (0 = from initial_guess) */

	gsl_vector *initial_guess;   /**< Starting point, NULL if initial_simplex provided */
	gsl_matrix *initial_simplex; /**< Pre-built [dims x (dims+1)], overrides guess+size */
	double init_size;            /**< Step size for simplex construction */

	double min_size;             /**< Convergence: stop when simplex size < this */
	int max_iter;                /**< Maximum iterations (0 = 1000) */

	double pos_min;              /**< Lower bound, uniform across dimensions */
	double pos_max;              /**< Upper bound, uniform across dimensions */

	double temperature;          /**< Simulated annealing noise scale (0 = off) */
	double exit_fit;             /**< Stop if fitness <= this value (NAN = disabled) */
	int verbose;                 /**< Bitfield of enum simplex_log_flags */

	simplex_fit_func_t fit_func; /**< Required: fitness evaluation function */
	void *fit_func_ctx;          /**< Opaque context passed to fit_func */

	simplex_log_func_t log_func; /**< Optional: called each iteration with state */
	void *log_func_ctx;          /**< Opaque context passed to log_func */
} simplex_config_t;

/** Opaque simplex optimizer handle */
typedef struct simplex_s simplex_t;

/** Fill config with default values. Call before setting custom fields. */
void simplex_config_init(simplex_config_t *config);

/** Create optimizer from config. Returns NULL on invalid config. */
simplex_t *simplex_new(const simplex_config_t *config);

/** Initialize or reinitialize the simplex. */
void simplex_init(simplex_t *s);

/** Run optimization loop. Returns best fitness found. */
double simplex_optimize(simplex_t *s);

/** Return best position found so far (owned by simplex_t, do not free). */
const gsl_vector *simplex_get_best_pos(const simplex_t *s);

/** Return best fitness value found so far. */
double simplex_get_best_fit(const simplex_t *s);

/** Return current simplex size (convergence metric). */
double simplex_get_simplex_size(const simplex_t *s);

/** Return total iterations performed across all optimize() calls. */
int simplex_get_iteration_count(const simplex_t *s);

/** Free optimizer and all associated memory. */
void simplex_free(simplex_t *s);

#endif
