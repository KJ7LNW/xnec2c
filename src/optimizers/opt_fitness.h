/*
 *  Fitness function for antenna optimization.
 *
 *  Computes a weighted scalar from measurement_t arrays produced by
 *  meas_calc().  Each objective selects a measurement field, applies a
 *  direction-aware transform (ratio or deviation from target), reduces
 *  across frequency steps, and multiplies by weight.  The sum of all
 *  objectives is minimized by the optimizer backend.
 *
 *  Ported from NEC2::xnec2c::optimize (Perl/PDL).
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_FITNESS_H
#define OPT_FITNESS_H 1

#include "../measurements.h"

/** Guard value for denominators approaching zero */
#define FITNESS_EPSILON 1e-12

/** Initial allocation capacity for dynamic objective list */
#define FITNESS_OBJ_INIT_CAP 4

/**
 * Internal direction mode, determined by the metric.
 *
 * MINIMIZE: score = pow(value / target, exp)
 *   Score is 1.0 at target.  Values exceeding target are penalized (>1.0),
 *   values better than target are rewarded (<1.0).
 *
 * MAXIMIZE: score = pow(|target| / max(|value|, epsilon), exp)
 *   Score is 1.0 at target.  Values below target are penalized (>1.0),
 *   values above target are rewarded (<1.0).
 *
 * DEVIATE: score = pow(|value - target|, exp)
 *   Score is 0.0 at target.  Any deviation is penalized symmetrically.
 */
enum fitness_direction
{
	FIT_DIR_MINIMIZE,
	FIT_DIR_MAXIMIZE,
	FIT_DIR_DEVIATE,

	FIT_DIR_COUNT
};

/**
 * Reduction method across frequency steps within a goal's MHz range.
 */
enum fitness_reduce
{
	FIT_REDUCE_SUM,
	FIT_REDUCE_AVG,
	FIT_REDUCE_MIN,
	FIT_REDUCE_MAX,
	FIT_REDUCE_MAG,
	FIT_REDUCE_DIFF,

	FIT_REDUCE_COUNT
};

/**
 * Per-measurement default fitness parameters.
 * Indexed by MEASUREMENT_INDEXES.  Populated in opt_fitness.c.
 */
typedef struct
{
	enum fitness_direction   direction;
	enum fitness_reduce      default_reduce;
	double                   default_target;
	double                   default_weight;
	double                   default_exponent;
} meas_fitness_default_t;

/** Per-measurement fitness defaults, indexed by MEASUREMENT_INDEXES */
extern const meas_fitness_default_t meas_fitness_defaults[MEAS_COUNT];

/** Reduction name strings, indexed by enum fitness_reduce */
extern const char *fitness_reduce_names[FIT_REDUCE_COUNT];

/** Direction name strings, indexed by enum fitness_direction */
extern const char *fitness_direction_names[FIT_DIR_COUNT];

/** Direction tooltip strings, indexed by enum fitness_direction */
extern const char *fitness_direction_tooltips[FIT_DIR_COUNT];

/** Reduction tooltip strings, indexed by enum fitness_reduce */
extern const char *fitness_reduce_tooltips[FIT_REDUCE_COUNT];

/**
 * Per-objective user configuration.
 * Each objective selects a measurement via meas_index.
 */
typedef struct
{
	int                  meas_index; /**< MEASUREMENT_INDEXES value */
	int                  enabled;
	double               weight;
	double               exponent;
	double               target;
	enum fitness_direction direction;
	enum fitness_reduce  reduce;
	double               mhz_min;   /**< NAN = use all FR card freqs */
	double               mhz_max;   /**< NAN = use all FR card freqs */
} fitness_objective_t;

/**
 * Dynamic fitness configuration.
 * Variable-length array of objectives.
 */
typedef struct
{
	fitness_objective_t *obj;
	int                  num_obj;
	int                  capacity;
} fitness_config_t;

/**
 * fitness_config_init - populate config with default objectives
 * @cfg: config to initialize (zeroed, then VSWR + gain_max added)
 */
void fitness_config_init(fitness_config_t *cfg);

/**
 * fitness_config_add - append an objective using measurement defaults
 * @cfg: config to modify
 * @meas_index: MEASUREMENT_INDEXES value for the new objective
 *
 * Returns pointer to the new objective for further customization.
 */
fitness_objective_t *fitness_config_add(fitness_config_t *cfg, int meas_index);

/**
 * fitness_config_remove - remove objective at given index
 * @cfg: config to modify
 * @idx: index into obj array to remove
 */
void fitness_config_remove(fitness_config_t *cfg, int idx);

/**
 * fitness_config_copy - deep copy src into dst
 * @dst: destination (will be initialized)
 * @src: source config
 */
void fitness_config_copy(fitness_config_t *dst, const fitness_config_t *src);

/**
 * fitness_config_free - release dynamic memory in config
 * @cfg: config to free (zeroed after free)
 */
void fitness_config_free(fitness_config_t *cfg);

/**
 * fitness_compute_objective - evaluate one objective's weighted contribution
 * @obj: single objective to evaluate
 * @meas: array of measurement_t, one per frequency step
 * @num_steps: length of meas array
 * @freq_mhz: array of frequency values in MHz, one per step
 *
 * Returns weight * reduce(transform(...)) for this objective,
 * or 0.0 if disabled or no valid steps.
 */
double fitness_compute_objective(const fitness_objective_t *obj,
	const measurement_t *meas, int num_steps, const double *freq_mhz);

/**
 * fitness_compute - evaluate fitness across frequency steps
 * @cfg: fitness configuration
 * @meas: array of measurement_t, one per frequency step
 * @num_steps: length of meas array
 * @freq_mhz: array of frequency values in MHz, one per step
 *
 * Returns a scalar to be minimized.  Lower values indicate better
 * antenna performance according to the configured objectives.
 */
double fitness_compute(const fitness_config_t *cfg,
	const measurement_t *meas, int num_steps, const double *freq_mhz);

/**
 * fitness_transform - apply direction formula to a single value
 * @direction: MINIMIZE, MAXIMIZE, or DEVIATE
 * @value: raw measurement value
 * @target: goal target value
 * @exponent: power to raise the result
 *
 * Exposed for unit testing.
 */
double fitness_transform(enum fitness_direction direction,
	double value, double target, double exponent);

#endif
