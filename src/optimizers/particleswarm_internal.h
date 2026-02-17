/*
 *  Particle Swarm Optimization - internal definitions.
 *
 *  Copyright (C) 2023- eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef PARTICLESWARM_INTERNAL_H
#define PARTICLESWARM_INTERNAL_H 1

#include "particleswarm.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>

/** Per-particle state arrays, all matrices are [dimensions x num_particles] */
typedef struct
{
	gsl_matrix *best_pos;
	gsl_matrix *curr_pos;
	gsl_matrix *next_pos;
	gsl_matrix *velocity;

	gsl_vector *best_fit;  /**< [num_particles] */
	gsl_vector *curr_fit;
	gsl_vector *next_fit;
	gsl_vector *stalls;    /**< Stall count per particle */
} pso_particles_t;

/** Full optimizer state, opaque to public API callers */
struct pso_s
{
	pso_config_t config;

	pso_particles_t *prtcls;
	gsl_vector *pos_min;         /**< Per-dimension lower bound [dimensions] */
	gsl_vector *pos_max;         /**< Per-dimension upper bound [dimensions] */

	double best_best;            /**< Global best fitness, INFINITY when unset */
	gsl_vector *best_best_pos;   /**< Position of global best [dimensions] */

	double bests_mean;           /**< Running mean for plateau detection */
	double *best_best_by_iter;   /**< Circular buffer [exit_plateau_window] */

	int iter_count;
	gsl_rng *rng;
	int initialized;
};

/** Initialize or reinitialize particles where mask[i] == 1.0.
 *  Sets bestPos, currPos, velocity, evaluates currFit and bestFit. */
void pso_init_particles(pso_t *pso, const gsl_vector *mask);

/** Compute nextPos = currPos + velocity, clip to bounds, evaluate fitness */
void pso_calc_next_pos(pso_t *pso);

/**
 * Advance particles: copy next to current, update personal and global bests.
 * Returns 1 if exitFit was reached (value in *exit_fit_val), 0 otherwise.
 */
int pso_move_particles(pso_t *pso, int iter, double *exit_fit_val);

/** Update velocities from personal/neighbor bests, handle stalls */
void pso_update_velocities(pso_t *pso, int iter);

/** Evaluate fitness for one position, converting NaN to INFINITY */
double pso_calc_fit(const pso_t *pso, const gsl_vector *pos);

/** Return index of best-fit neighbor for particle me */
int pso_get_best_neighbour(const pso_t *pso, int me);

/** Return uniform random value in [min, max] */
double pso_rand_in_range(pso_t *pso, double min, double max);

#endif
