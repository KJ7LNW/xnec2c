/*
 *  Named-variable optimizer - internal definitions.
 *
 *  Shared between opt_simple.c, opt_simple_var.c, opt_simple_engine.c.
 *  Not part of the public API.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_SIMPLE_INTERNAL_H
#define OPT_SIMPLE_INTERNAL_H 1

#include "opt_simple.h"

/** Result cache: dynamic array of (key, value) pairs */
typedef struct
{
	char   **keys;
	double  *values;
	int      count;
	int      capacity;
} simple_cache_t;

/** Full session state, opaque to public API callers */
struct simple_s
{
	/* Deep-copied config */
	enum optimizer_algo algorithm;
	simple_var_t *vars;
	int num_vars;

	int    max_iter;
	double exit_fit;
	int    srand_seed;
	int    nocache;
	int    stagnant_minima_count;
	double stagnant_minima_tolerance;

	simple_fit_func_t fit_func;
	void *fit_func_ctx;
	simple_log_func_t log_func;
	void *log_func_ctx;

	/* Algorithm-specific options, using backend config types directly.
	 * Pre-initialized at construction; per-pass fields set in builders. */
	union
	{
		struct
		{
			simplex_config_t cfg; /**< Pre-initialized, pointer fields NULL */
			double *ssize;        /**< Deep-copied multi-pass ssize array */
			int     num_ssize;
		} simplex;

		pso_config_t pso_cfg;     /**< Pre-initialized with defaults + user overrides */
	} algo_opts;

	/* Index map: gsl_vector dim -> (var_idx, elem_idx) */
	int *map_var;
	int *map_elem;
	int  total_dims;

	/** Sorted var name indices for deterministic packing order */
	int *sorted_var_indices;

	/* Work vars: pre-allocated, reused in fitness trampoline */
	simple_var_t *work_vars;

	/* Cross-pass best tracking */
	double best_minima;
	gsl_vector *best_vec;
	simple_var_t *best_vars;
	int best_pass;

	/* Stagnation state */
	double prev_minima;
	int    prev_minima_count;

	/* Counters */
	int optimization_pass;
	int log_count;
	int iter_count;

	/* Cache */
	simple_cache_t cache;
	int cache_hits;
	int cache_misses;

	/* Cancellation */
	volatile int cancel;

	/* Timing */
	double prev_time;

	/* Latest ssize from backend log trampoline */
	double current_ssize;

	/* Result vars (populated after optimize, round_result applied) */
	simple_var_t *result_vars;
	int num_result_vars;
};

/* ---- opt_simple_var.c ---- */

/**
 * simple_validate_var - validate a single simple_var_t
 * @v: var to validate
 * @idx: index for error messages
 *
 * Returns 0 on success, -1 on error (message to stderr).
 */
int simple_validate_var(const simple_var_t *v, int idx);

/**
 * simple_var_deep_copy - deep-copy a simple_var_t
 * @dst: destination (caller-allocated struct)
 * @src: source
 *
 * Allocates gsl_vectors and strdup's name.
 */
void simple_var_deep_copy(simple_var_t *dst, const simple_var_t *src);

/**
 * simple_var_free_contents - free internals of a simple_var_t
 * @v: var to free (struct itself not freed)
 */
void simple_var_free_contents(simple_var_t *v);

/**
 * simple_build_index_map - build the dim->(var,elem) index map
 * @s: session handle
 *
 * Iterates vars in sorted-name order. Populates map_var, map_elem,
 * total_dims, sorted_var_indices. Frees previous map if any.
 */
void simple_build_index_map(simple_t *s);

/**
 * simple_pack_vars - pack enabled var values into gsl_vector
 * @s: session handle
 *
 * Returns newly allocated gsl_vector of length total_dims.
 * Values divided by perturb_scale.
 */
gsl_vector *simple_pack_vars(const simple_t *s);

/**
 * simple_unpack_vec - unpack gsl_vector into work_vars
 * @s: session handle
 * @vec: optimizer position vector
 *
 * Writes enabled elements (scaled, rounded, clamped) into work_vars.
 * Disabled elements copied from s->vars unchanged.
 */
void simple_unpack_vec(simple_t *s, const gsl_vector *vec);

/**
 * simple_apply_round_result - apply round_result to a var array
 * @vars: var array to round in place
 * @num_vars: length
 */
void simple_apply_round_result(simple_var_t *vars, int num_vars);

/**
 * simple_nearest - round val to nearest increment of target
 * @target: rounding increment
 * @val: value to round
 */
double simple_nearest(double target, double val);

/**
 * simple_clamp - clamp val to [lo, hi]
 * @val: value
 * @lo: lower bound
 * @hi: upper bound
 */
double simple_clamp(double val, double lo, double hi);

/**
 * simple_compute_packed_bounds - per-dimension bounds in packed coordinate space
 * @s: session handle
 * @out_min: output gsl_vector* of length total_dims (caller frees)
 * @out_max: output gsl_vector* of length total_dims (caller frees)
 *
 * Each dimension's min/max is divided by the corresponding perturb_scale
 * to match the packed value space used by the optimizer backend.
 * Dimensions with no bounds set to -INFINITY / INFINITY.
 */
void simple_compute_packed_bounds(const simple_t *s,
	gsl_vector **out_min, gsl_vector **out_max);

/* ---- opt_simple_engine.c ---- */

/**
 * simple_cache_clear - free all cached entries
 * @cache: cache to clear
 */
void simple_cache_clear(simple_cache_t *cache);

/**
 * simple_cache_lookup - look up cached fitness for packed position
 * @s: session handle
 * @vec: packed position vector from optimizer backend
 * @found: output, set to 1 if found
 *
 * Builds key from packed vector with full precision. Returns cached value
 * if found, avoiding the need to unpack on cache hits.
 */
double simple_cache_lookup(simple_t *s, const gsl_vector *vec, int *found);

/**
 * simple_cache_store - store fitness result keyed on packed position
 * @s: session handle
 * @vec: packed position vector used as cache key
 * @value: fitness value to cache
 */
void simple_cache_store(simple_t *s, const gsl_vector *vec, double value);

/**
 * simple_fitness_trampoline - fitness callback installed in backend
 * @pos: position vector from backend
 * @ctx: simple_t* pointer
 *
 * Unpacks, clamps, rounds, checks cancel, calls user's fit_func.
 */
double simple_fitness_trampoline(const gsl_vector *pos, void *ctx);

/**
 * simple_simplex_log_trampoline - log callback for simplex backend
 * @simplex: current simplex matrix
 * @vals: fitness at each vertex
 * @ssize: current simplex size
 * @ctx: simple_t* pointer
 */
void simple_simplex_log_trampoline(const gsl_matrix *simplex,
	const gsl_vector *vals, double ssize, void *ctx);

/**
 * simple_pso_log_trampoline - log callback for PSO backend
 * @pos: current best position
 * @fit: current best fitness
 * @ctx: simple_t* pointer
 */
void simple_pso_log_trampoline(const gsl_vector *pos, double fit, void *ctx);

#endif
