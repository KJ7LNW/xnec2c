/*
 *  Nelder-Mead Simplex Optimization - geometric engine operations.
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

#include <gsl/gsl_blas.h>
#include <math.h>

/**
 * simplex_clamp_to_bounds - clip each dimension to [posMin, posMax]
 * @s: optimizer handle
 * @point: vector to clamp in-place
 *
 * No-op for dimensions where bounds are infinite.
 */
void simplex_clamp_to_bounds(const simplex_t *s, gsl_vector *point)
{
	int dims = s->config.dimensions;

	for (int d = 0; d < dims; d++)
	{
		double lo = gsl_vector_get(s->config.pos_min, d);
		double hi = gsl_vector_get(s->config.pos_max, d);
		double v = gsl_vector_get(point, d);

		if (v < lo)
		{
			gsl_vector_set(point, d, lo);
		}
		else if (v > hi)
		{
			gsl_vector_set(point, d, hi);
		}
	}
}

/**
 * simplex_calc_fit - evaluate fitness, converting NaN to INFINITY
 * @s: optimizer handle
 * @pos: position vector
 */
double simplex_calc_fit(const simplex_t *s, const gsl_vector *pos)
{
	double f = s->config.fit_func(pos, s->config.fit_func_ctx);

	if (isnan(f))
	{
		return INFINITY;
	}

	return f;
}

/**
 * simplex_construct_initial - build tetrahedral simplex from guess + initSize
 * @s: optimizer handle
 *
 * Implements the PDL tetrahedron construction: at step i (0-based),
 * vertices 0..i shift by -initSize*i/(i+1) on axis i, and vertex
 * i+1 shifts by +initSize*(1-i/(i+1)) on axis i.
 * All vertices are clamped and fitness evaluated.
 */
void simplex_construct_initial(simplex_t *s)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;

	/* Start all vertices at initialGuess */
	for (int j = 0; j < nv; j++)
	{
		gsl_vector_view col = gsl_matrix_column(s->simp, j);
		gsl_vector_memcpy(&col.vector, s->config.initial_guess);
	}

	/* Perturb to form tetrahedron */
	double sz = s->config.init_size;
	for (int i = 0; i < dims; i++)
	{
		double pj = (double)i / (double)(i + 1);

		/* Vertices 0..i: shift negative on axis i */
		for (int j = 0; j <= i; j++)
		{
			double v = gsl_matrix_get(s->simp, i, j);
			gsl_matrix_set(s->simp, i, j, v - sz * pj);
		}

		/* Vertex i+1: shift positive on axis i */
		double v = gsl_matrix_get(s->simp, i, i + 1);
		gsl_matrix_set(s->simp, i, i + 1, v + sz * (1.0 - pj));
	}

	/* Clamp all vertices and evaluate fitness */
	for (int j = 0; j < nv; j++)
	{
		gsl_vector_view col = gsl_matrix_column(s->simp, j);
		simplex_clamp_to_bounds(s, &col.vector);
		gsl_vector_set(s->vals, j, simplex_calc_fit(s, &col.vector));
	}
}

/**
 * simplex_find_extremes - locate best, worst, and second-worst vertices
 * @s: optimizer handle
 * @vals: fitness values to search (may be noise-perturbed)
 * @best_idx: output, index of minimum
 * @worst_idx: output, index of maximum
 * @worst2_idx: output, index of second-maximum
 */
void simplex_find_extremes(const simplex_t *s, const gsl_vector *vals,
	int *best_idx, int *worst_idx, int *worst2_idx)
{
	int nv = s->config.dimensions + 1;
	double best_val = INFINITY;
	double worst_val = -INFINITY;
	double worst2_val = -INFINITY;

	*best_idx = 0;
	*worst_idx = 0;
	*worst2_idx = 0;

	for (int j = 0; j < nv; j++)
	{
		double v = gsl_vector_get(vals, j);

		if (v < best_val)
		{
			best_val = v;
			*best_idx = j;
		}

		if (v > worst_val)
		{
			worst2_val = worst_val;
			*worst2_idx = *worst_idx;
			worst_val = v;
			*worst_idx = j;
		}
		else if (v > worst2_val)
		{
			worst2_val = v;
			*worst2_idx = j;
		}
	}
}

/**
 * simplex_calc_centroid - mean of all vertices except worst
 * @s: optimizer handle
 * @worst_idx: vertex index to exclude
 *
 * Result stored in s->centroid.
 */
void simplex_calc_centroid(simplex_t *s, int worst_idx)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;

	gsl_vector_set_zero(s->centroid);

	for (int j = 0; j < nv; j++)
	{
		if (j == worst_idx)
		{
			continue;
		}

		gsl_vector_const_view col = gsl_matrix_const_column(s->simp, j);
		gsl_vector_add(s->centroid, &col.vector);
	}

	/* Divide by dims (number of vertices minus one) */
	gsl_vector_scale(s->centroid, 1.0 / dims);
}

/**
 * simplex_reflect - reflect worst vertex through centroid
 * @s: optimizer handle
 * @worst_idx: vertex to reflect
 *
 * trial = 2*centroid - worst. Clamped before evaluation.
 * Returns fitness of trial point.
 */
double simplex_reflect(simplex_t *s, int worst_idx)
{
	gsl_vector_const_view worst = gsl_matrix_const_column(s->simp, worst_idx);

	/* trial = 2*centroid - worst */
	gsl_vector_memcpy(s->trial, s->centroid);
	gsl_blas_dscal(2.0, s->trial);
	gsl_blas_daxpy(-1.0, &worst.vector, s->trial);

	simplex_clamp_to_bounds(s, s->trial);
	return simplex_calc_fit(s, s->trial);
}

/**
 * simplex_expand - expand reflected point away from worst
 * @s: optimizer handle
 * @worst_idx: worst vertex index
 *
 * trial2 = trial + centroid - worst. Clamped before evaluation.
 * Returns fitness of expanded point.
 */
double simplex_expand(simplex_t *s, int worst_idx)
{
	gsl_vector_const_view worst = gsl_matrix_const_column(s->simp, worst_idx);

	/* trial2 = trial + centroid - worst */
	gsl_vector_memcpy(s->trial2, s->trial);
	gsl_blas_daxpy(1.0, s->centroid, s->trial2);
	gsl_blas_daxpy(-1.0, &worst.vector, s->trial2);

	simplex_clamp_to_bounds(s, s->trial2);
	return simplex_calc_fit(s, s->trial2);
}

/**
 * simplex_contract - contract worst vertex toward centroid
 * @s: optimizer handle
 * @worst_idx: worst vertex index
 *
 * trial = 0.5*centroid + 0.5*worst. Clamped before evaluation.
 * Returns fitness of contracted point.
 */
double simplex_contract(simplex_t *s, int worst_idx)
{
	gsl_vector_const_view worst = gsl_matrix_const_column(s->simp, worst_idx);

	/* trial = 0.5*centroid + 0.5*worst */
	gsl_vector_memcpy(s->trial, s->centroid);
	gsl_blas_daxpy(1.0, &worst.vector, s->trial);
	gsl_blas_dscal(0.5, s->trial);

	simplex_clamp_to_bounds(s, s->trial);
	return simplex_calc_fit(s, s->trial);
}

/**
 * simplex_shrink - shrink all vertices toward best
 * @s: optimizer handle
 * @best_idx: vertex to keep fixed
 *
 * Each non-best vertex moves halfway toward best, is clamped,
 * and has its fitness re-evaluated.
 */
void simplex_shrink(simplex_t *s, int best_idx)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;
	gsl_vector_const_view best = gsl_matrix_const_column(s->simp, best_idx);

	for (int j = 0; j < nv; j++)
	{
		if (j == best_idx)
		{
			continue;
		}

		gsl_vector_view col = gsl_matrix_column(s->simp, j);
		for (int d = 0; d < dims; d++)
		{
			double b = gsl_vector_get(&best.vector, d);
			double v = gsl_vector_get(&col.vector, d);
			gsl_vector_set(&col.vector, d, 0.5 * b + 0.5 * v);
		}

		simplex_clamp_to_bounds(s, &col.vector);
		gsl_vector_set(s->vals, j, simplex_calc_fit(s, &col.vector));
	}
}

/**
 * simplex_calc_size - max euclidean distance from vertex 0 to others
 * @s: optimizer handle
 *
 * This is the convergence metric: when it drops below minSize,
 * the simplex has collapsed to a point.
 */
double simplex_calc_size(const simplex_t *s)
{
	int dims = s->config.dimensions;
	int nv = dims + 1;
	double max_dist = 0.0;
	gsl_vector_const_view v0 = gsl_matrix_const_column(s->simp, 0);

	for (int j = 1; j < nv; j++)
	{
		double dist_sq = 0.0;
		gsl_vector_const_view vj = gsl_matrix_const_column(s->simp, j);

		for (int d = 0; d < dims; d++)
		{
			double diff = gsl_vector_get(&vj.vector, d)
				- gsl_vector_get(&v0.vector, d);
			dist_sq += diff * diff;
		}

		double dist = sqrt(dist_sq);
		if (dist > max_dist)
		{
			max_dist = dist;
		}
	}

	return max_dist;
}
