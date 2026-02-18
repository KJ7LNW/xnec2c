/*
 *  Fitness function for antenna optimization.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "opt_fitness.h"
#include "../console.h"

/*------------------------------------------------------------------------*/

/* Compile-time metric info table.
 * Order matches enum fitness_metric for direct indexing. */
const fitness_metric_info_t fitness_metric_info[FIT_METRIC_COUNT] =
{
	[FIT_VSWR] =
	{
		.name             = "VSWR",
		.metric           = FIT_VSWR,
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_VSWR,
		.default_target   = 1.5,
		.default_weight   = 5.0,
		.default_exponent = 2.0,
	},

	[FIT_GAIN_MAX] =
	{
		.name             = "Max Gain",
		.metric           = FIT_GAIN_MAX,
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_GAIN_MAX,
		.default_target   = 8.0,
		.default_weight   = 10.0,
		.default_exponent = 0.5,
	},

	[FIT_GAIN_NET] =
	{
		.name             = "Net Gain",
		.metric           = FIT_GAIN_NET,
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_GAIN_NET,
		.default_target   = 6.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[FIT_GAIN_VIEWER] =
	{
		.name             = "Viewer Gain",
		.metric           = FIT_GAIN_VIEWER,
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_GAIN_VIEWER,
		.default_target   = 6.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[FIT_FB_RATIO] =
	{
		.name             = "F/B Ratio",
		.metric           = FIT_FB_RATIO,
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_FB_RATIO,
		.default_target   = 20.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[FIT_S11] =
	{
		.name             = "S11",
		.metric           = FIT_S11,
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_S11,
		.default_target   = -15.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[FIT_GAIN_THETA] =
	{
		.name             = "Gain Theta",
		.metric           = FIT_GAIN_THETA,
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_GAIN_THETA,
		.default_target   = 90.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[FIT_GAIN_PHI] =
	{
		.name             = "Gain Phi",
		.metric           = FIT_GAIN_PHI,
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.meas_index       = MEAS_GAIN_PHI,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[FIT_GAIN_FLAT] =
	{
		.name             = "Gain Flatness",
		.metric           = FIT_GAIN_FLAT,
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_DIFF,
		.meas_index       = MEAS_GAIN_MAX,
		.default_target   = 3.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},
};

/*------------------------------------------------------------------------*/

/* Reduction name strings for UI combo boxes */
const char *fitness_reduce_names[FIT_REDUCE_COUNT] =
{
	[FIT_REDUCE_SUM]  = "sum",
	[FIT_REDUCE_AVG]  = "avg",
	[FIT_REDUCE_MIN]  = "min",
	[FIT_REDUCE_MAX]  = "max",
	[FIT_REDUCE_MAG]  = "mag",
	[FIT_REDUCE_DIFF] = "diff",
};

/*------------------------------------------------------------------------*/

/**
 * fitness_config_init - populate config with defaults from metric_info table
 */
void fitness_config_init(fitness_config_t *cfg)
{
	int i;

	for (i = 0; i < FIT_METRIC_COUNT; i++)
	{
		cfg->obj[i].enabled  = 0;
		cfg->obj[i].weight   = fitness_metric_info[i].default_weight;
		cfg->obj[i].exponent = fitness_metric_info[i].default_exponent;
		cfg->obj[i].target   = fitness_metric_info[i].default_target;
		cfg->obj[i].reduce   = fitness_metric_info[i].default_reduce;
		cfg->obj[i].mhz_min  = NAN;
		cfg->obj[i].mhz_max  = NAN;
	}

	/* Enable default goals: VSWR, Max Gain, F/B Ratio */
	cfg->obj[FIT_VSWR].enabled = 1;
	cfg->obj[FIT_GAIN_MAX].enabled = 1;
	cfg->obj[FIT_FB_RATIO].enabled = 1;
}

/*------------------------------------------------------------------------*/

/**
 * fitness_transform - apply direction formula to a single value
 */
double fitness_transform(enum fitness_direction direction,
	double value, double target, double exponent)
{
	double ratio;
	double deviation;

	switch (direction)
	{
		case FIT_DIR_MINIMIZE:
			/* pow(value / target, exp) */
			if (fabs(target) < FITNESS_EPSILON)
			{
				return pow(fabs(value) + 1.0, exponent);
			}
			ratio = value / target;
			return pow(ratio, exponent);

		case FIT_DIR_MAXIMIZE:
			/* pow(|target| / max(|value|, epsilon), exp) */
			ratio = fabs(target) / fmax(fabs(value), FITNESS_EPSILON);
			return pow(ratio, exponent);

		case FIT_DIR_DEVIATE:
			/* pow(|value - target|, exp) */
			deviation = fabs(value - target);
			return pow(deviation, exponent);

		default:
			break;
	}

	pr_err("fitness_transform: unknown direction %d\n", direction);
	abort();
}

/*------------------------------------------------------------------------*/

/**
 * reduce_values - apply reduction to an array of transformed values
 * @values: array of per-step transformed scores
 * @n: number of values
 * @reduce: reduction method
 *
 * Returns the reduced scalar.
 */
static double reduce_values(const double *values, int n,
	enum fitness_reduce reduce)
{
	double sum;
	double mag_sum;
	double vmin;
	double vmax;
	int i;

	if (n <= 0)
	{
		return 0.0;
	}

	sum = 0.0;
	mag_sum = 0.0;
	vmin = values[0];
	vmax = values[0];

	for (i = 0; i < n; i++)
	{
		sum += values[i];
		mag_sum += values[i] * values[i];

		if (values[i] < vmin)
		{
			vmin = values[i];
		}

		if (values[i] > vmax)
		{
			vmax = values[i];
		}
	}

	switch (reduce)
	{
		case FIT_REDUCE_SUM:
			return sum;

		case FIT_REDUCE_AVG:
			return sum / n;

		case FIT_REDUCE_MIN:
			return vmin;

		case FIT_REDUCE_MAX:
			return vmax;

		case FIT_REDUCE_MAG:
			return sqrt(mag_sum);

		case FIT_REDUCE_DIFF:
			return vmax - vmin;

		default:
			break;
	}

	pr_err("reduce_values: unknown reduction method %d\n", reduce);
	abort();
}

/*------------------------------------------------------------------------*/

/**
 * fitness_compute - evaluate fitness across frequency steps
 */
double fitness_compute(const fitness_config_t *cfg,
	const measurement_t *meas, int num_steps, const double *freq_mhz)
{
	double total;
	double transformed[num_steps];
	int i;
	int m;
	int step_count;
	double raw;
	double reduced;
	double mhz_lo;
	double mhz_hi;
	const fitness_objective_t *obj;
	const fitness_metric_info_t *info;

	total = 0.0;

	for (m = 0; m < FIT_METRIC_COUNT; m++)
	{
		obj = &cfg->obj[m];

		if (!obj->enabled)
		{
			continue;
		}

		if (obj->weight == 0.0)
		{
			continue;
		}

		info = &fitness_metric_info[m];

		/* Determine MHz range for this objective */
		mhz_lo = isnan(obj->mhz_min) ? -INFINITY : obj->mhz_min;
		mhz_hi = isnan(obj->mhz_max) ? INFINITY : obj->mhz_max;

		/* Collect and transform values within freq range */
		step_count = 0;
		for (i = 0; i < num_steps; i++)
		{
			if (freq_mhz[i] < mhz_lo || freq_mhz[i] > mhz_hi)
			{
				continue;
			}

			raw = meas[i].a[info->meas_index];

			/* Skip invalid measurements (meas_calc initializes all fields to -1) */
			if (raw == -1.0)
			{
				continue;
			}

			transformed[step_count] = fitness_transform(
				info->direction, raw, obj->target, obj->exponent);

			/* Clamp infinities and NaNs to a large penalty */
			if (!isfinite(transformed[step_count]))
			{
				transformed[step_count] = 1e6;
			}

			step_count++;
		}

		if (step_count == 0)
		{
			continue;
		}

		reduced = reduce_values(transformed, step_count, obj->reduce);
		total += obj->weight * reduced;
	}

	return total;
}
