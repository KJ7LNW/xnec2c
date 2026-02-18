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

/**
 * Fitness metrics corresponding to measurement_t fields.
 * Order determines UI display order.
 */
enum fitness_metric
{
	FIT_VSWR,
	FIT_GAIN_MAX,
	FIT_GAIN_NET,
	FIT_GAIN_VIEWER,
	FIT_FB_RATIO,
	FIT_S11,
	FIT_GAIN_THETA,
	FIT_GAIN_PHI,
	FIT_GAIN_FLAT,

	FIT_METRIC_COUNT
};

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
	FIT_DIR_DEVIATE
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
 * Compile-time metadata for each fitness metric.
 * Populated in opt_fitness.c as a const array.
 */
typedef struct
{
	const char              *name;
	enum fitness_metric      metric;
	enum fitness_direction   direction;
	enum fitness_reduce      default_reduce;
	int                      meas_index;
	double                   default_target;
	double                   default_weight;
	double                   default_exponent;
} fitness_metric_info_t;

/** Compile-time metric info table, indexed by enum fitness_metric */
extern const fitness_metric_info_t fitness_metric_info[FIT_METRIC_COUNT];

/** Reduction name strings, indexed by enum fitness_reduce */
extern const char *fitness_reduce_names[FIT_REDUCE_COUNT];

/**
 * Per-objective user configuration.
 * One entry per metric in the fitness config.
 */
typedef struct
{
	int                  enabled;
	double               weight;
	double               exponent;
	double               target;
	enum fitness_reduce  reduce;
	double               mhz_min;   /**< NAN = use all FR card freqs */
	double               mhz_max;   /**< NAN = use all FR card freqs */
} fitness_objective_t;

/**
 * Complete fitness configuration.
 * Array of objectives indexed by enum fitness_metric.
 */
typedef struct
{
	fitness_objective_t obj[FIT_METRIC_COUNT];
} fitness_config_t;

/**
 * fitness_config_init - populate config with defaults from metric_info table
 * @cfg: config to initialize
 *
 * Sets each objective's weight, exponent, target, and reduce from the
 * compile-time defaults.  VSWR and GAIN_MAX are enabled; all others disabled.
 */
void fitness_config_init(fitness_config_t *cfg);

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
