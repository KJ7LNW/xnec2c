/*
 *  Named-variable optimization session manager.
 *
 *  Provides the "Simple" interface: named variables with per-element
 *  attributes (bounds, perturb_scale, rounding, enabled mask) are
 *  packed into gsl_vector for the backend and unpacked for the user's
 *  fitness callback.  Manages multi-pass ssize, caching, stagnation
 *  detection, cancellation, and cross-pass best tracking.
 *
 *  Ported from PDL::Opt::Simplex::Simple (Perl/PDL).
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_SIMPLE_H
#define OPT_SIMPLE_H 1

#include <gsl/gsl_vector.h>

#include "optimizer.h"

/**
 * Per-variable descriptor.
 *
 * values->size is the single source of truth for element count.
 * All non-NULL vectors must have size == values->size.
 * Deep-copied at simple_new(); caller frees originals.
 */
typedef struct
{
	const char *name;          /**< Variable name (must be unique) */
	gsl_vector *values;        /**< Current values (required, ->size = element count) */
	gsl_vector *enabled;       /**< 0.0/1.0 per element (NULL = all enabled) */
	gsl_vector *min;           /**< Per-element lower bound (NULL = unbounded) */
	gsl_vector *max;           /**< Per-element upper bound (NULL = unbounded) */
	gsl_vector *perturb_scale; /**< Per-element scale factor (NULL = all 1.0) */
	gsl_vector *round_each;    /**< Round to nearest on each iteration (NULL = off) */
	gsl_vector *round_result;  /**< Round to nearest after optimization (NULL = off) */
} simple_var_t;

/**
 * State passed to the log callback each iteration.
 * All fields mirror the Perl Simple.pm log state.
 */
typedef struct
{
	double ssize;               /**< Current simplex size (INFINITY for PSO) */
	double minima;              /**< Current best minima */
	double elapsed;             /**< Seconds since last log call */
	int    srand_seed;          /**< Random seed for this run */
	int    optimization_pass;   /**< Current pass number (1-based) */
	int    num_passes;          /**< Total number of passes */
	int    best_pass;           /**< Pass that produced best result */
	double best_minima;         /**< Best minima across all passes */
	const simple_var_t *best_vars;  /**< Vars at best minima */
	int    num_best_vars;       /**< Length of best_vars array */
	int    log_count;           /**< Times log called in current pass */
	int    iter_count;          /**< Times fitness called total */
	int    cancel;              /**< Nonzero if cancellation pending */
	int    prev_minima_count;   /**< Consecutive stagnant iterations */
	int    cache_hits;          /**< Cache hit count */
	int    cache_misses;        /**< Cache miss count */
} simple_log_state_t;

/**
 * Fitness callback.
 *
 * Receives unpacked named vars with current values.
 * Must return a scalar to be minimized.
 */
typedef double (*simple_fit_func_t)(const simple_var_t *vars, int num_vars,
	void *ctx);

/**
 * Log callback.
 *
 * Called after each backend iteration with current vars and rich state.
 * Return value ignored.
 */
typedef void (*simple_log_func_t)(const simple_var_t *vars, int num_vars,
	const simple_log_state_t *state, void *ctx);

/**
 * Configuration for the Simple optimizer.
 * Call simple_config_init() first, then set fields.
 */
typedef struct
{
	enum optimizer_algo algorithm;

	simple_var_t *vars;         /**< Array of variable descriptors */
	int num_vars;               /**< Length of vars array */

	int    max_iter;            /**< Max iterations per pass (0 = 1000) */
	double exit_fit;            /**< Stop if fitness <= this (NAN = disabled) */
	int    srand_seed;          /**< Random seed (0 = auto-generate) */
	int    nocache;             /**< Nonzero to disable result caching */

	int    stagnant_minima_count;    /**< Cancel after this many stagnant iters (0 = off) */
	double stagnant_minima_tolerance; /**< Threshold for stagnation (0 = use tolerance) */

	simple_fit_func_t fit_func; /**< Required: fitness function */
	void *fit_func_ctx;         /**< Opaque context for fit_func */

	simple_log_func_t log_func; /**< Optional: log callback */
	void *log_func_ctx;         /**< Opaque context for log_func */

	/** Algorithm-specific backend configs.
	 * User calls simplex_config_init() or pso_config_init() on the
	 * appropriate field before setting values.  simple_new() deep-copies
	 * the config and overwrites computed fields (dimensions, bounds, etc). */
	union
	{
		struct
		{
			simplex_config_t cfg;  /**< Backend config (call simplex_config_init first) */
			double *ssize;         /**< Multi-pass ssize array (simple-layer only) */
			int    num_ssize;      /**< Length (0 = single pass, ssize=1.0) */
		} simplex;

		pso_config_t pso_cfg;  /**< Backend config (call pso_config_init first) */
	} opts;
} simple_config_t;

/** Opaque session handle */
typedef struct simple_s simple_t;

/**
 * simple_config_init - fill config with safe defaults
 * @cfg: config to initialize
 * @algo: algorithm selector
 */
void simple_config_init(simple_config_t *cfg, enum optimizer_algo algo);

/**
 * simple_new - create optimization session
 * @cfg: fully populated config (deep-copied internally)
 *
 * Validates config, builds index map, allocates work structures.
 * Returns NULL with message to stderr on invalid config.
 */
simple_t *simple_new(const simple_config_t *cfg);

/**
 * simple_optimize - run optimization (multi-pass if configured)
 * @s: session handle
 *
 * Returns best fitness found across all passes.
 */
double simple_optimize(simple_t *s);

/**
 * simple_get_result - return result vars with round_result applied
 * @s: session handle
 * @num_vars: output, number of vars in returned array
 *
 * Returned array is owned by simple_t; do not free.
 */
const simple_var_t *simple_get_result(const simple_t *s, int *num_vars);

/**
 * simple_get_best_fit - return best fitness across all passes
 * @s: session handle
 */
double simple_get_best_fit(const simple_t *s);

/**
 * simple_set_vars - replace variables and rebuild index map
 * @s: session handle
 * @vars: new variable array (deep-copied)
 * @num_vars: length
 */
void simple_set_vars(simple_t *s, const simple_var_t *vars, int num_vars);

/**
 * simple_set_ssize - replace ssize array with single value (simplex only)
 * @s: session handle
 * @ssize: new simplex size
 */
void simple_set_ssize(simple_t *s, double ssize);

/**
 * simple_scale_ssize - multiply all ssize entries by scale (simplex only)
 * @s: session handle
 * @scale: multiplier
 */
void simple_scale_ssize(simple_t *s, double scale);

/**
 * simple_cancel - request early termination
 * @s: session handle
 *
 * Sets cancel flag; fitness trampoline returns INFINITY on next call.
 */
void simple_cancel(simple_t *s);

/**
 * simple_free - release session and all resources
 * @s: session handle (NULL safe)
 */
void simple_free(simple_t *s);

#endif
