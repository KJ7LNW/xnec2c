/*
 *  Named-variable optimizer - cache and trampoline callbacks.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Cache ---- */

/**
 * _cache_build_key - build cache key string from packed position vector
 * @vec: optimizer position vector in packed coordinate space
 *
 * Format: "v0,v1,v2,..." with full IEEE 754 precision (%.17g).
 * Keyed on the packed vector directly to avoid perturb_scale round-trip
 * drift and to enable cache lookup before unpacking.
 * Returns malloc'd string; caller frees.
 */
static char *_cache_build_key(const gsl_vector *vec)
{
	/* Each %.17g value is at most 25 chars; add comma delimiter */
	int dims = (int)vec->size;
	int bufsize = dims * 26 + 1;
	char *key = malloc(bufsize);
	int pos = 0;

	for (int d = 0; d < dims; d++)
	{
		if (d > 0)
		{
			pos += snprintf(key + pos, bufsize - pos, ",");
		}
		pos += snprintf(key + pos, bufsize - pos, "%.17g",
			gsl_vector_get(vec, d));
	}

	return key;
}

/**
 * simple_cache_clear - free all entries in cache
 * @cache: cache to clear
 */
void simple_cache_clear(simple_cache_t *cache)
{
	for (int i = 0; i < cache->count; i++)
	{
		free(cache->keys[i]);
	}

	free(cache->keys);
	free(cache->values);
	cache->keys = NULL;
	cache->values = NULL;
	cache->count = 0;
	cache->capacity = 0;
}

/**
 * simple_cache_lookup - search cache for packed position vector
 * @s: session handle
 * @vec: packed position vector from optimizer backend
 * @found: output, set to 1 if cache hit
 *
 * Returns cached fitness on hit, NAN on miss.
 */
double simple_cache_lookup(simple_t *s, const gsl_vector *vec, int *found)
{
	*found = 0;

	if (s->nocache)
	{
		s->cache_misses++;
		return NAN;
	}

	char *key = _cache_build_key(vec);

	for (int i = 0; i < s->cache.count; i++)
	{
		if (strcmp(s->cache.keys[i], key) == 0)
		{
			free(key);
			s->cache_hits++;
			*found = 1;
			return s->cache.values[i];
		}
	}

	free(key);
	s->cache_misses++;
	return NAN;
}

/**
 * simple_cache_store - store fitness result in cache
 * @s: session handle
 * @vec: packed position vector used as cache key
 * @value: fitness to store
 */
void simple_cache_store(simple_t *s, const gsl_vector *vec, double value)
{
	if (s->nocache)
	{
		return;
	}

	/* Grow arrays if needed */
	if (s->cache.count >= s->cache.capacity)
	{
		int new_cap = (s->cache.capacity == 0) ? 64 : s->cache.capacity * 2;
		s->cache.keys   = realloc(s->cache.keys,   new_cap * sizeof(char *));
		s->cache.values = realloc(s->cache.values, new_cap * sizeof(double));
		s->cache.capacity = new_cap;
	}

	char *key = _cache_build_key(vec);
	s->cache.keys[s->cache.count]   = key;
	s->cache.values[s->cache.count] = value;
	s->cache.count++;
}

/* ---- Timing ---- */

/**
 * _get_time - return monotonic time in seconds
 */
static double _get_time(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ---- Fitness trampoline ---- */

/**
 * simple_fitness_trampoline - adapter between backend and user fitness
 * @pos: position vector from optimizer backend (packed coordinate space)
 * @ctx: simple_t* pointer
 *
 * 1. Check cancel -> return INFINITY
 * 2. Increment iter_count (cache hits counted as iterations)
 * 3. Check cache on packed vector -> return cached if hit
 * 4. Unpack pos into work_vars (scale, round, clamp) — only on cache miss
 * 5. Call user's fit_func
 * 6. Store in cache (keyed on packed vector)
 * 7. Track best
 * 8. Convert NaN to INFINITY
 */
double simple_fitness_trampoline(const gsl_vector *pos, void *ctx)
{
	simple_t *s = ctx;

	/* Early exit on cancellation */
	if (s->cancel)
	{
		return INFINITY;
	}

	s->iter_count++;

	/* Cache lookup on packed vector (before unpack to avoid drift) */
	int found;
	double result = simple_cache_lookup(s, pos, &found);
	if (found)
	{
		return result;
	}

	/* Unpack only on cache miss */
	simple_unpack_vec(s, pos);

	/* Call user fitness function */
	result = s->fit_func(s->work_vars, s->num_vars, s->fit_func_ctx);

	/* NaN safety: treat as worst possible */
	if (isnan(result))
	{
		result = INFINITY;
	}

	simple_cache_store(s, pos, result);

	/* Track cross-pass best */
	if (result < s->best_minima)
	{
		s->best_minima = result;
		s->best_pass = s->optimization_pass;

		/* Deep-copy current position vector */
		if (!s->best_vec)
		{
			s->best_vec = gsl_vector_alloc(pos->size);
		}
		gsl_vector_memcpy(s->best_vec, pos);

		/* Deep-copy work_vars as best_vars */
		if (s->best_vars)
		{
			for (int i = 0; i < s->num_vars; i++)
			{
				simple_var_free_contents(&s->best_vars[i]);
			}
			free(s->best_vars);
		}

		s->best_vars = calloc(s->num_vars, sizeof(simple_var_t));
		for (int i = 0; i < s->num_vars; i++)
		{
			simple_var_deep_copy(&s->best_vars[i], &s->work_vars[i]);
		}
	}

	return result;
}

/* ---- Log trampolines ---- */

/**
 * _log_common - shared log logic for both backends
 * @s: session handle
 * @ssize: simplex size (INFINITY for PSO)
 *
 * Updates stagnation tracking, timing, calls user log callback.
 */
static void _log_common(simple_t *s, double ssize)
{
	s->current_ssize = ssize;

	/* Timing */
	double now = _get_time();
	double elapsed = 0.0;
	if (s->prev_time > 0.0)
	{
		elapsed = now - s->prev_time;
	}
	s->prev_time = now;

	s->log_count++;

	double minima = s->best_minima;

	/* Stagnation detection: best_minima only decreases, so check
	 * whether the improvement since last log is below tolerance. */
	if (s->stagnant_minima_count > 0 && isfinite(s->prev_minima)
		&& fabs(s->prev_minima - minima) < s->stagnant_minima_tolerance)
	{
		s->prev_minima_count++;
		if (!s->cancel && s->prev_minima_count > s->stagnant_minima_count)
		{
			s->cancel = 1;
		}
	}
	else if (!s->cancel)
	{
		s->prev_minima = minima;
		s->prev_minima_count = 0;
	}

	/* Call user log callback */
	if (!s->log_func)
	{
		return;
	}

	simple_log_state_t state;
	memset(&state, 0, sizeof(state));

	state.ssize              = ssize;
	state.minima             = minima;
	state.elapsed            = elapsed;
	state.srand_seed         = s->srand_seed;
	state.optimization_pass  = s->optimization_pass;
	state.num_passes         = (s->algorithm == OPT_SIMPLEX)
		? s->algo_opts.simplex.num_ssize : 1;
	state.best_pass          = s->best_pass;
	state.best_minima        = s->best_minima;
	state.best_vars          = s->best_vars;
	state.num_best_vars      = s->num_vars;
	state.log_count          = s->log_count;
	state.iter_count         = s->iter_count;
	state.cancel             = s->cancel;
	state.prev_minima_count  = s->prev_minima_count;
	state.cache_hits         = s->cache_hits;
	state.cache_misses       = s->cache_misses;

	/* Pass work_vars (current iteration values) to user */
	s->log_func(s->work_vars, s->num_vars, &state, s->log_func_ctx);
}

/**
 * simple_simplex_log_trampoline - log adapter for simplex backend
 * @simplex: simplex matrix (unused by simple layer)
 * @vals: vertex fitness values (unused by simple layer)
 * @ssize: current simplex size
 * @ctx: simple_t* pointer
 */
void simple_simplex_log_trampoline(const gsl_matrix *simplex,
	const gsl_vector *vals, double ssize, void *ctx)
{
	(void)simplex;
	(void)vals;
	_log_common(ctx, ssize);
}

/**
 * simple_pso_log_trampoline - log adapter for PSO backend
 * @pos: best position (unused by simple layer)
 * @fit: best fitness (unused by simple layer)
 * @ctx: simple_t* pointer
 */
void simple_pso_log_trampoline(const gsl_vector *pos, double fit, void *ctx)
{
	(void)pos;
	(void)fit;

	/* PSO has no simplex-size equivalent */
	_log_common(ctx, INFINITY);
}
