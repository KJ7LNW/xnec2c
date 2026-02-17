/*
 *  Named-variable optimizer - variable management.
 *
 *  Deep copy, index map construction, pack/unpack between
 *  named vars and gsl_vector, rounding, clamping.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#include "opt_simple_internal.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_blas.h>

/* ---- Validation ---- */

/**
 * simple_validate_var - validate a single simple_var_t
 * @v: var to validate
 * @idx: index for error messages
 *
 * Checks name, values, enabled mask, min/max bounds, perturb_scale,
 * round_each, round_result for internal consistency.
 * Returns 0 on success, -1 on error (message to stderr).
 */
int simple_validate_var(const simple_var_t *v, int idx)
{
	if (!v->name || v->name[0] == '\0')
	{
		fprintf(stderr, "simple_new: var[%d] name is NULL or empty\n", idx);
		return -1;
	}

	if (!v->values || v->values->size == 0)
	{
		fprintf(stderr, "simple_new: var '%s' values is NULL or empty\n", v->name);
		return -1;
	}

	size_t n = v->values->size;

	/* Validate enabled: all elements 0.0 or 1.0 */
	if (v->enabled)
	{
		if (v->enabled->size != n)
		{
			fprintf(stderr, "simple_new: var '%s' enabled->size (%zu) != values->size (%zu)\n",
				v->name, v->enabled->size, n);
			return -1;
		}

		for (size_t i = 0; i < n; i++)
		{
			double e = gsl_vector_get(v->enabled, i);
			if (e != 0.0 && e != 1.0)
			{
				fprintf(stderr, "simple_new: var '%s' enabled[%zu] = %g (must be 0.0 or 1.0)\n",
					v->name, i, e);
				return -1;
			}
		}
	}

	/* Validate min/max: both or neither, matching size, min <= max, initial in bounds */
	if ((v->min != NULL) != (v->max != NULL))
	{
		fprintf(stderr, "simple_new: var '%s' min and max must both be set or both NULL\n",
			v->name);
		return -1;
	}

	if (v->min && v->max)
	{
		if (v->min->size != n || v->max->size != n)
		{
			fprintf(stderr, "simple_new: var '%s' min/max size mismatch with values\n",
				v->name);
			return -1;
		}

		for (size_t i = 0; i < n; i++)
		{
			double lo = gsl_vector_get(v->min, i);
			double hi = gsl_vector_get(v->max, i);
			if (lo > hi)
			{
				fprintf(stderr, "simple_new: var '%s' min[%zu]=%g > max[%zu]=%g\n",
					v->name, i, lo, i, hi);
				return -1;
			}

			/* Check initial values within bounds (enabled elements only) */
			int enabled = (!v->enabled || gsl_vector_get(v->enabled, i) != 0.0);
			if (enabled)
			{
				double val = gsl_vector_get(v->values, i);
				if (val < lo || val > hi)
				{
					fprintf(stderr,
						"simple_new: var '%s'[%zu] initial value %g outside [%g, %g]\n",
						v->name, i, val, lo, hi);
					return -1;
				}
			}
		}
	}

	/* Validate perturb_scale: matching size, no zeros */
	if (v->perturb_scale)
	{
		if (v->perturb_scale->size != n)
		{
			fprintf(stderr, "simple_new: var '%s' perturb_scale size mismatch\n", v->name);
			return -1;
		}

		for (size_t i = 0; i < n; i++)
		{
			if (gsl_vector_get(v->perturb_scale, i) == 0.0)
			{
				fprintf(stderr, "simple_new: var '%s' perturb_scale[%zu] is zero\n",
					v->name, i);
				return -1;
			}
		}
	}

	/* Validate round_each: matching size, all > 0 */
	if (v->round_each)
	{
		if (v->round_each->size != n)
		{
			fprintf(stderr, "simple_new: var '%s' round_each size mismatch\n", v->name);
			return -1;
		}

		if (gsl_vector_min(v->round_each) <= 0.0)
		{
			fprintf(stderr, "simple_new: var '%s' round_each has element <= 0\n",
				v->name);
			return -1;
		}
	}

	/* Validate round_result: matching size, all > 0 */
	if (v->round_result)
	{
		if (v->round_result->size != n)
		{
			fprintf(stderr, "simple_new: var '%s' round_result size mismatch\n", v->name);
			return -1;
		}

		if (gsl_vector_min(v->round_result) <= 0.0)
		{
			fprintf(stderr, "simple_new: var '%s' round_result has element <= 0\n",
				v->name);
			return -1;
		}
	}

	return 0;
}

/* ---- Deep copy ---- */

/**
 * _gsl_vector_dup - allocate and copy a gsl_vector
 * @src: vector to copy (NULL returns NULL)
 */
static gsl_vector *_gsl_vector_dup(const gsl_vector *src)
{
	if (!src)
	{
		return NULL;
	}

	gsl_vector *dst = gsl_vector_alloc(src->size);
	gsl_vector_memcpy(dst, src);

	return dst;
}

/** Field offsets for all gsl_vector* members of simple_var_t */
static const size_t _vec_field_offsets[] =
{
	offsetof(simple_var_t, values),
	offsetof(simple_var_t, enabled),
	offsetof(simple_var_t, min),
	offsetof(simple_var_t, max),
	offsetof(simple_var_t, perturb_scale),
	offsetof(simple_var_t, round_each),
	offsetof(simple_var_t, round_result)
};

#define NUM_VEC_FIELDS (sizeof(_vec_field_offsets) / sizeof(_vec_field_offsets[0]))

/**
 * simple_var_deep_copy - deep-copy all fields of a simple_var_t
 * @dst: destination struct (caller-allocated, zeroed)
 * @src: source struct
 */
void simple_var_deep_copy(simple_var_t *dst, const simple_var_t *src)
{
	dst->name = strdup(src->name);

	for (size_t i = 0; i < NUM_VEC_FIELDS; i++)
	{
		size_t offset = _vec_field_offsets[i];
		gsl_vector **src_field = (gsl_vector **)((char *)src + offset);
		gsl_vector **dst_field = (gsl_vector **)((char *)dst + offset);
		*dst_field = _gsl_vector_dup(*src_field);
	}
}

/**
 * simple_var_free_contents - free all heap memory within a var
 * @v: var whose contents to free (struct itself not freed)
 */
void simple_var_free_contents(simple_var_t *v)
{
	if (!v)
	{
		return;
	}

	free((char *)v->name);
	v->name = NULL;

	for (size_t i = 0; i < NUM_VEC_FIELDS; i++)
	{
		size_t offset = _vec_field_offsets[i];
		gsl_vector **field = (gsl_vector **)((char *)v + offset);

		if (*field)
		{
			gsl_vector_free(*field);
			*field = NULL;
		}
	}
}

/**
 * _name_cmp - qsort comparator for sorted_var_indices by var name
 * @a: pointer to int index
 * @b: pointer to int index
 *
 * Uses _cmp_vars as static context. Set before calling qsort.
 * Not thread-safe: serial-only constraint.
 */
static const simple_var_t *_cmp_vars;

static int _name_cmp(const void *a, const void *b)
{
	int ia = *(const int *)a;
	int ib = *(const int *)b;

	return strcmp(_cmp_vars[ia].name, _cmp_vars[ib].name);
}

/**
 * simple_build_index_map - construct dim->(var,elem) mapping
 * @s: session handle
 *
 * Iterates vars in name-sorted order. For each enabled element,
 * appends (var_index, elem_index) to the map. Sets total_dims.
 */
void simple_build_index_map(simple_t *s)
{
	/* Free previous map */
	free(s->map_var);
	free(s->map_elem);
	free(s->sorted_var_indices);
	s->map_var = NULL;
	s->map_elem = NULL;
	s->sorted_var_indices = NULL;
	s->total_dims = 0;

	/* Build sorted index array */
	s->sorted_var_indices = calloc(s->num_vars, sizeof(int));
	for (int i = 0; i < s->num_vars; i++)
	{
		s->sorted_var_indices[i] = i;
	}

	_cmp_vars = s->vars;
	qsort(s->sorted_var_indices, s->num_vars, sizeof(int), _name_cmp);

	/* Count total enabled dimensions */
	int total = 0;
	for (int si = 0; si < s->num_vars; si++)
	{
		int vi = s->sorted_var_indices[si];
		const simple_var_t *v = &s->vars[vi];

		if (!v->enabled)
		{
			total += (int)v->values->size;
		}
		else
		{
			total += (int)gsl_blas_dasum(v->enabled);
		}
	}

	s->total_dims = total;
	if (total == 0)
	{
		return;
	}

	/* Allocate and populate maps */
	s->map_var  = calloc(total, sizeof(int));
	s->map_elem = calloc(total, sizeof(int));

	int dim = 0;
	for (int si = 0; si < s->num_vars; si++)
	{
		int vi = s->sorted_var_indices[si];
		const simple_var_t *v = &s->vars[vi];
		int n = (int)v->values->size;

		for (int e = 0; e < n; e++)
		{
			if (!v->enabled || gsl_vector_get(v->enabled, e) != 0.0)
			{
				s->map_var[dim]  = vi;
				s->map_elem[dim] = e;
				dim++;
			}
		}
	}
}

/**
 * simple_pack_vars - build initial guess gsl_vector from vars
 * @s: session handle
 *
 * Returns newly allocated vector. Each dimension is
 * value / perturb_scale for that element.
 */
gsl_vector *simple_pack_vars(const simple_t *s)
{
	gsl_vector *vec = gsl_vector_alloc(s->total_dims);

	for (int d = 0; d < s->total_dims; d++)
	{
		int vi = s->map_var[d];
		int ei = s->map_elem[d];
		const simple_var_t *v = &s->vars[vi];

		double val = gsl_vector_get(v->values, ei);

		/* Scale by perturb_scale for internal optimizer space */
		double scale = 1.0;
		if (v->perturb_scale)
		{
			scale = gsl_vector_get(v->perturb_scale, ei);
		}
		gsl_vector_set(vec, d, val / scale);
	}

	return vec;
}

/**
 * simple_unpack_vec - write optimizer vector into work_vars
 * @s: session handle
 * @vec: position vector from backend
 *
 * For enabled elements: extract from vec, apply perturb_scale,
 * round_each, and clamp to [min, max]. For disabled elements:
 * copy from s->vars unchanged.
 */
void simple_unpack_vec(simple_t *s, const gsl_vector *vec)
{
	/* Copy all values from config vars (handles disabled elements) */
	for (int i = 0; i < s->num_vars; i++)
	{
		gsl_vector_memcpy(s->work_vars[i].values, s->vars[i].values);
	}

	/* Overwrite enabled elements from optimizer vector */
	for (int d = 0; d < s->total_dims; d++)
	{
		int vi = s->map_var[d];
		int ei = s->map_elem[d];
		const simple_var_t *v = &s->vars[vi];

		double val = gsl_vector_get(vec, d);

		/* Reverse the perturb_scale division from packing */
		double scale = 1.0;
		if (v->perturb_scale)
		{
			scale = gsl_vector_get(v->perturb_scale, ei);
		}
		val *= scale;

		/* Per-iteration rounding */
		if (v->round_each)
		{
			double r = gsl_vector_get(v->round_each, ei);
			val = simple_nearest(r, val);
		}

		/* Clamp to bounds */
		if (v->min && v->max)
		{
			double lo = gsl_vector_get(v->min, ei);
			double hi = gsl_vector_get(v->max, ei);
			val = simple_clamp(val, lo, hi);
		}

		gsl_vector_set(s->work_vars[vi].values, ei, val);
	}
}

/**
 * simple_apply_round_result - round final values per round_result
 * @vars: variable array to modify in place
 * @num_vars: length
 */
void simple_apply_round_result(simple_var_t *vars, int num_vars)
{
	for (int i = 0; i < num_vars; i++)
	{
		simple_var_t *v = &vars[i];
		if (!v->round_result)
		{
			continue;
		}

		int n = (int)v->values->size;
		for (int e = 0; e < n; e++)
		{
			double r = gsl_vector_get(v->round_result, e);
			double val = gsl_vector_get(v->values, e);
			gsl_vector_set(v->values, e, simple_nearest(r, val));
		}
	}
}

/**
 * simple_nearest - round val to nearest multiple of target
 * @target: rounding granularity (must be > 0)
 * @val: value to round
 *
 * Discretizes continuous optimizer output to physical constraints
 * (e.g., wire lengths quantized to manufacturing tolerances).
 * Equivalent to Perl Math::Round::nearest().
 */
double simple_nearest(double target, double val)
{
	/* Normalize to integer multiples then scale back */
	return floor(val / target + 0.5) * target;
}

/**
 * simple_clamp - clamp val to [lo, hi]
 * @val: input value
 * @lo: lower bound
 * @hi: upper bound
 *
 * Enforces per-element min/max constraints after the optimizer
 * proposes values outside the feasible region.
 */
double simple_clamp(double val, double lo, double hi)
{
	if (val < lo)
	{
		return lo;
	}

	if (val > hi)
	{
		return hi;
	}

	return val;
}

/**
 * simple_compute_global_bounds - find extreme bounds across all vars
 * @s: session handle
 * @out_min: receives min of all per-element mins
 * @out_max: receives max of all per-element maxes
 *
 * If any var lacks bounds, sets output to -INFINITY / INFINITY.
 */
void simple_compute_global_bounds(const simple_t *s,
	double *out_min, double *out_max)
{
	double gmin = INFINITY;
	double gmax = -INFINITY;
	int any_unbounded = 0;

	for (int d = 0; d < s->total_dims; d++)
	{
		int vi = s->map_var[d];
		int ei = s->map_elem[d];
		const simple_var_t *v = &s->vars[vi];

		if (!v->min || !v->max)
		{
			any_unbounded = 1;
			break;
		}

		double lo = gsl_vector_get(v->min, ei);
		double hi = gsl_vector_get(v->max, ei);
		if (lo < gmin)
		{
			gmin = lo;
		}
		if (hi > gmax)
		{
			gmax = hi;
		}
	}

	if (any_unbounded || s->total_dims == 0)
	{
		*out_min = -INFINITY;
		*out_max = INFINITY;
	}
	else
	{
		*out_min = gmin;
		*out_max = gmax;
	}
}
