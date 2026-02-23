/*
 *  Nelder-Mead Simplex Optimization - internal definitions.
 *
 *  Copyright (C) 2023- eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef SIMPLEX_INTERNAL_H
#define SIMPLEX_INTERNAL_H 1

#include "simplex.h"

#include <gsl/gsl_rng.h>

/** Full optimizer state, opaque to public API callers */
struct simplex_s
{
	simplex_config_t config;

	gsl_matrix *simp;        /**< Simplex vertices [dims x (dims+1)] */
	gsl_vector *vals;        /**< Fitness at each vertex [(dims+1)] */

	gsl_vector *centroid;    /**< Scratch: centroid of non-worst vertices [dims] */
	gsl_vector *trial;       /**< Scratch: reflected or contracted point [dims] */
	gsl_vector *trial2;      /**< Scratch: expanded point [dims] */

	gsl_vector *best_pos;    /**< Best position found [dims], persistent for getter */
	double best_fit;         /**< Best fitness found, INFINITY when unset */

	double ssize;            /**< Current simplex size */
	int iter_count;
	gsl_rng *rng;            /**< Only allocated when temperature > 0 */
	int initialized;
};

/**
 * simplex_construct_initial - build tetrahedral simplex from guess + initSize
 * @s: optimizer handle
 *
 * Constructs (dims+1) vertices, clamps to bounds, evaluates fitness.
 */
void simplex_construct_initial(simplex_t *s);

/**
 * simplex_find_extremes - locate best, worst, and second-worst vertices
 * @s: optimizer handle
 * @vals: fitness values to search (may be noise-perturbed copy)
 * @best_idx: output, index of minimum value
 * @worst_idx: output, index of maximum value
 * @worst2_idx: output, index of second-maximum value
 */
void simplex_find_extremes(const simplex_t *s, const gsl_vector *vals,
	int *best_idx, int *worst_idx, int *worst2_idx);

/**
 * simplex_calc_centroid - mean of all vertices except worst
 * @s: optimizer handle
 * @worst_idx: index of vertex to exclude
 *
 * Result stored in s->centroid.
 */
void simplex_calc_centroid(simplex_t *s, int worst_idx);

/**
 * simplex_reflect - reflect worst vertex through centroid
 * @s: optimizer handle
 * @worst_idx: index of vertex to reflect
 *
 * Computes trial = 2*centroid - worst, clamps, evaluates fitness.
 * Result stored in s->trial.
 */
double simplex_reflect(simplex_t *s, int worst_idx);

/**
 * simplex_expand - expand reflected point away from worst
 * @s: optimizer handle
 * @worst_idx: index of worst vertex
 *
 * Computes trial2 = trial + centroid - worst, clamps, evaluates fitness.
 * Result stored in s->trial2.
 */
double simplex_expand(simplex_t *s, int worst_idx);

/**
 * simplex_contract - contract worst vertex toward centroid
 * @s: optimizer handle
 * @worst_idx: index of worst vertex
 *
 * Computes trial = 0.5*centroid + 0.5*worst, clamps, evaluates fitness.
 * Result stored in s->trial.
 */
double simplex_contract(simplex_t *s, int worst_idx);

/**
 * simplex_shrink - shrink all vertices toward best
 * @s: optimizer handle
 * @best_idx: index of best vertex (kept fixed)
 *
 * Moves each non-best vertex halfway toward best, clamps, re-evaluates.
 */
void simplex_shrink(simplex_t *s, int best_idx);

/**
 * simplex_calc_size - compute simplex size metric
 * @s: optimizer handle
 *
 * Returns max euclidean distance from vertex 0 to all other vertices.
 */
double simplex_calc_size(const simplex_t *s);

/**
 * simplex_calc_fit - evaluate fitness, converting NaN to INFINITY
 * @s: optimizer handle
 * @pos: position vector [dims]
 */
double simplex_calc_fit(const simplex_t *s, const gsl_vector *pos);

/**
 * simplex_clamp_to_bounds - clip position to [posMin, posMax]
 * @s: optimizer handle
 * @point: vector to clamp in-place [dims]
 */
void simplex_clamp_to_bounds(const simplex_t *s, gsl_vector *point);

#endif
