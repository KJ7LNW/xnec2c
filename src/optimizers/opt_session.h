/*
 *  Optimizer session orchestration.
 *
 *  Connects the Simple optimizer to the fitness function and NEC2
 *  evaluator.  Manages the optimizer pthread lifecycle.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_SESSION_H
#define OPT_SESSION_H    1

#include "../common.h"
#include "opt_fitness.h"
#include "opt_simple.h"

/** Maximum frequency steps supported by the optimizer session */
#define OPT_MAX_FREQ_STEPS 2048

/**
 * Algorithm-specific parameters for opt_start.
 *
 * Union layout matches simple_config_t.opts so the caller populates
 * upstream config structs directly — no field-by-field duplication.
 * Call simplex_config_init() or pso_config_init() on the appropriate
 * member, then override fields as needed.
 */
typedef union
{
	struct
	{
		simplex_config_t cfg;  /**< Simplex backend config */
		double *ssize;         /**< Multi-pass simplex size array */
		int num_ssize;         /**< Length of ssize array (0 = single pass) */
	} simplex;

	pso_config_t pso_cfg;      /**< PSO backend config */
} opt_algo_params_t;

/**
 * Optimizer session state, owned by the orchestration layer.
 */
typedef struct
{
	simple_t        *simple;                     /**< Simple optimizer handle */
	simple_config_t  simple_cfg;                 /**< Simple optimizer config */
	fitness_config_t fitness_cfg;                /**< Fitness function config */
	measurement_t    meas[OPT_MAX_FREQ_STEPS];   /**< Per-step measurements */
	double           freq[OPT_MAX_FREQ_STEPS];   /**< Per-step frequencies */
	int              num_steps;                  /**< Actual step count */
	pthread_t        thread;                     /**< Worker thread handle */
	gboolean         running;                    /**< Thread active flag */
	double           best_fitness;               /**< Best fitness found */
	simple_log_state_t last_log;                 /**< Most recent log state */
	gboolean         has_log;                    /**< TRUE after first log */
} opt_session_t;

/**
 * opt_start - launch optimizer in background thread
 * @vars: simple_var_t array (deep-copied by simple_new)
 * @num_vars: length of vars array
 * @fitness_cfg: fitness configuration (copied)
 * @algo: algorithm selector (OPT_SIMPLEX or OPT_PSO)
 * @algo_params: algorithm-specific parameters (simplex or PSO config)
 * @max_iter: maximum iterations per pass
 * @stagnant_count: stagnation iteration limit (0 = off)
 * @stagnant_tol: stagnation tolerance
 *
 * Returns 0 on success, -1 if already running.
 */
int opt_start(simple_var_t *vars, int num_vars,
	const fitness_config_t *fitness_cfg,
	enum optimizer_algo algo, const opt_algo_params_t *algo_params,
	int max_iter, int stagnant_count, double stagnant_tol);

/**
 * opt_cancel - request early termination of running optimizer
 */
void opt_cancel(void);

/**
 * opt_is_running - check if optimizer thread is active
 */
gboolean opt_is_running(void);

/**
 * opt_get_best_fitness - return best fitness from last or current run
 */
double opt_get_best_fitness(void);

/**
 * opt_get_log_state - return most recent log state snapshot
 *
 * Returns NULL if no log callback has fired yet.
 * Pointer valid until session is freed.
 */
const simple_log_state_t *opt_get_log_state(void);

/**
 * opt_get_result - return result variables from completed optimization
 * @num_vars: output, number of vars in returned array
 *
 * Returns NULL if no result available.  Owned by session, do not free.
 */
const simple_var_t *opt_get_result(int *num_vars);

#endif
