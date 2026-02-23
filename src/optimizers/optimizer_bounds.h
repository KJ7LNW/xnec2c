/*
 *  Shared bounds validation and deep-copy utilities for optimizer backends.
 *
 *  Header-only (static inline) to avoid Makefile changes.
 *  Included by particleswarm.c and simplex.c.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPTIMIZER_BOUNDS_H
#define OPTIMIZER_BOUNDS_H 1

#include <gsl/gsl_vector.h>

#include <math.h>
#include <stdio.h>

/**
 * optimizer_validate_bounds - validate per-dimension bound vectors
 * @pos_min: lower bounds vector
 * @pos_max: upper bounds vector
 * @dims: expected dimension count
 * @allow_infinite: if nonzero, skip comparison for infinite bounds
 * @caller: function name for error messages
 *
 * Checks that both vectors have the expected size and that
 * pos_max[i] > pos_min[i] for each dimension.  When allow_infinite
 * is set, dimensions where either bound is non-finite are skipped.
 *
 * Returns 0 on success, -1 on validation failure (message to stderr).
 */
static inline int optimizer_validate_bounds(
	const gsl_vector *pos_min, const gsl_vector *pos_max,
	int dims, int allow_infinite, const char *caller)
{
	if ((int)pos_min->size != dims || (int)pos_max->size != dims)
	{
		fprintf(stderr, "%s: pos_min/pos_max size (%zu, %zu) != dimensions (%d)\n",
			caller, pos_min->size, pos_max->size, dims);
		return -1;
	}

	for (int i = 0; i < dims; i++)
	{
		double lo = gsl_vector_get(pos_min, i);
		double hi = gsl_vector_get(pos_max, i);

		if (allow_infinite && (!isfinite(lo) || !isfinite(hi)))
		{
			continue;
		}

		if (hi <= lo)
		{
			fprintf(stderr, "%s: pos_max[%d] (%g) <= pos_min[%d] (%g)\n",
				caller, i, hi, i, lo);
			return -1;
		}
	}

	return 0;
}

/**
 * optimizer_deep_copy_bounds - deep-copy bound vectors into destination pointers
 * @dst_min: output, receives newly allocated copy (caller frees)
 * @dst_max: output, receives newly allocated copy (caller frees)
 * @src_min: source lower bounds
 * @src_max: source upper bounds
 */
static inline void optimizer_deep_copy_bounds(
	gsl_vector **dst_min, gsl_vector **dst_max,
	const gsl_vector *src_min, const gsl_vector *src_max)
{
	*dst_min = gsl_vector_alloc(src_min->size);
	gsl_vector_memcpy(*dst_min, src_min);

	*dst_max = gsl_vector_alloc(src_max->size);
	gsl_vector_memcpy(*dst_max, src_max);
}

#endif
