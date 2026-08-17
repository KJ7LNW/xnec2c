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
#include <string.h>

#include "opt_fitness.h"
#include "../common.h"
#include "../console.h"

/*------------------------------------------------------------------------*/

/* Per-measurement fitness defaults, indexed by MEASUREMENT_INDEXES.
 * Provides sensible direction, reduction, target, weight, and exponent
 * for each measurement when first added to the objective list. */
const meas_fitness_default_t meas_fitness_defaults[MEAS_COUNT] =
{
	[MEAS_MHZ] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 0.0,
		.default_exponent = 1.0,
	},

	[MEAS_ZREAL] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 50.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_ZIMAG] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_ZMAG] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 50.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_ZPHASE] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_VSWR] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 5.0,
		.default_exponent = 2.0,
	},

	[MEAS_S11] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = -15.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_S11_REAL] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = -15.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_S11_IMAG] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_S11_ANG] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_MAX] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 12.0,
		.default_weight   = 10.0,
		.default_exponent = 0.5,
	},

	[MEAS_GAIN_NET] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 6.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_GAIN_THETA] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 90.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_PHI] =
	{
		.direction        = FIT_DIR_DEVIATE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 0.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_VIEWER] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 6.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_GAIN_VIEWER_NET] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 6.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_FB_RATIO] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 20.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_GAIN_DEV_PX] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_DEV_NX] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_DEV_PY] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_DEV_NY] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_DEV_PZ] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_GAIN_DEV_NZ] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1.0,
		.default_weight   = 10.0,
		.default_exponent = 2.0,
	},

	[MEAS_ANT_TEMP] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1000.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_ANT_TEMP_TOT] =
	{
		.direction        = FIT_DIR_MINIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 1000.0,
		.default_weight   = 1.0,
		.default_exponent = 1.0,
	},

	[MEAS_GT] =
	{
		.direction        = FIT_DIR_MAXIMIZE,
		.default_reduce   = FIT_REDUCE_AVG,
		.default_target   = 5.0,
		.default_weight   = 1.0,
		.default_exponent = 2.0,
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

/* Direction name strings for UI combo boxes */
const char *fitness_direction_names[FIT_DIR_COUNT] =
{
	[FIT_DIR_MINIMIZE] = "min score",
	[FIT_DIR_MAXIMIZE] = "max score",
	[FIT_DIR_DEVIATE]  = "± target",
};

/* Direction tooltip strings for UI combo boxes */
const char *fitness_direction_tooltips[FIT_DIR_COUNT] =
{
	[FIT_DIR_MINIMIZE] =
		"Score = (max(v − t, 0) / √(t² + 1))<sup>exp</sup>"
		" + τ / (1 + max(t − v, 0) / √(t² + 1))\n"
		"Penalizes values above target; score approaches zero below target",
	[FIT_DIR_MAXIMIZE] =
		"Score = (max(t − v, 0) / √(t² + 1))<sup>exp</sup>"
		" + τ / (1 + max(v − t, 0) / √(t² + 1))\n"
		"Penalizes values below target; score approaches zero above target",
	[FIT_DIR_DEVIATE]  =
		"Score = |value − target|<sup>exp</sup>\n"
		"Penalizes deviation from target; score=0 when value = target",
};

/* Reduction tooltip strings for UI combo boxes */
const char *fitness_reduce_tooltips[FIT_REDUCE_COUNT] =
{
	[FIT_REDUCE_SUM]  = "Adds the score from every NEC2 frequency step",
	[FIT_REDUCE_AVG]  = "Averages the score across all NEC2 frequency steps",
	[FIT_REDUCE_MIN]  = "Selects the smallest score among all frequency steps",
	[FIT_REDUCE_MAX]  = "Selects the largest score among all frequency steps",
	[FIT_REDUCE_MAG]  = "Root of summed squared scores: sqrt(sum(score<sup>2</sup>))",
	[FIT_REDUCE_DIFF] = "Spread between largest and smallest scores across frequency steps",
};

/*------------------------------------------------------------------------*/

/**
 * fitness_config_init - populate config with default objectives
 */
void fitness_config_init(fitness_config_t *cfg)
{
	fitness_objective_t *obj;

	memset(cfg, 0, sizeof(*cfg));
	mem_array_alloc(&cfg->obj, FITNESS_OBJ_INIT_CAP);
	cfg->num_obj = 0;

	/* Default goals: VSWR, Max Gain */
	obj = fitness_config_add(cfg, MEAS_VSWR);
	obj->enabled = 1;

	obj = fitness_config_add(cfg, MEAS_GAIN_MAX);
	obj->enabled = 1;
}

/*------------------------------------------------------------------------*/

/**
 * fitness_config_add - append an objective using measurement defaults
 */
fitness_objective_t *fitness_config_add(fitness_config_t *cfg, int meas_index)
{
	fitness_objective_t *obj;
	const meas_fitness_default_t *def;

	if (meas_index < 0 || meas_index >= MEAS_COUNT)
	{
		pr_err("fitness_config_add: invalid meas_index %d\n", meas_index);
		abort();
	}

	mem_array_reserve(&cfg->obj, cfg->num_obj + 1, FITNESS_OBJ_INIT_CAP);

	def = &meas_fitness_defaults[meas_index];
	obj = &cfg->obj[cfg->num_obj];

	obj->meas_index = meas_index;
	obj->enabled    = 0;
	obj->weight     = def->default_weight;
	obj->exponent   = def->default_exponent;
	obj->target     = def->default_target;
	obj->direction  = def->direction;
	obj->reduce     = def->default_reduce;
	obj->mhz_min    = NAN;
	obj->mhz_max    = NAN;

	cfg->num_obj++;

	return obj;
}

/*------------------------------------------------------------------------*/

/**
 * fitness_config_remove - remove objective at given index
 */
void fitness_config_remove(fitness_config_t *cfg, int idx)
{
	if (idx < 0 || idx >= cfg->num_obj)
	{
		return;
	}

	/* Shift remaining objectives down */
	memmove(&cfg->obj[idx], &cfg->obj[idx + 1],
		(cfg->num_obj - idx - 1) * sizeof(fitness_objective_t));

	cfg->num_obj--;
}

/*------------------------------------------------------------------------*/

/**
 * fitness_config_copy - deep copy src into dst
 */
void fitness_config_copy(fitness_config_t *dst, const fitness_config_t *src)
{
	dst->num_obj = src->num_obj;

	dst->obj = NULL;
	if (src->num_obj > 0)
	{
		mem_array_alloc(&dst->obj, src->num_obj);
		mem_array_cpy(dst->obj, src->obj, src->num_obj);
	}
}

/*------------------------------------------------------------------------*/

/**
 * fitness_config_free - release dynamic memory in config
 */
void fitness_config_free(fitness_config_t *cfg)
{
	mem_array_free(&cfg->obj);
	memset(cfg, 0, sizeof(*cfg));
}

/*------------------------------------------------------------------------*/

/**
 * fitness_transform - apply direction formula to a single value
 */
double fitness_transform(enum fitness_direction direction,
	double value, double target, double exponent)
{
	double norm;
	double penalty;

	switch (direction)
	{
		case FIT_DIR_MINIMIZE:
		{
			/* Normalize by √(t²+1): behaves as |target| for large
			 * targets, transitions to absolute error (denominator=1)
			 * near target=0 where relative error is undefined. */
			double shortfall = value - target;
			double tau = pow(FITNESS_TENSION, exponent);

			norm = hypot(target, 1.0);
			penalty = fmax(shortfall, 0.0);

			/* Tension term: monotonically decreasing residual past
			 * target so the optimizer keeps improving met objectives.
			 * Bounded by tau, negligible vs. real penalties. */
			return pow(penalty / norm, exponent)
				+ tau / (1.0 + fmax(-shortfall, 0.0) / norm);
		}

		case FIT_DIR_MAXIMIZE:
		{
			/* Mirror of MINIMIZE: penalizes value below target.
			 * No sign-aware branch needed; subtraction on the number
			 * line is inherently sign-agnostic. */
			double shortfall = target - value;
			double tau = pow(FITNESS_TENSION, exponent);

			norm = hypot(target, 1.0);
			penalty = fmax(shortfall, 0.0);

			return pow(penalty / norm, exponent)
				+ tau / (1.0 + fmax(-shortfall, 0.0) / norm);
		}

		case FIT_DIR_DEVIATE:
			/* |v − t|^exp — no normalization or tension */
			penalty = fabs(value - target);
			return pow(penalty, exponent);

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
 * fitness_compute_objective - evaluate one objective's weighted contribution
 */
double fitness_compute_objective(const fitness_objective_t *obj,
	const measurement_t *meas, int num_steps, const double *freq_mhz)
{
	double transformed[num_steps];
	int i;
	int step_count;
	double raw;
	double reduced;
	double mhz_lo;
	double mhz_hi;

	if (!obj->enabled)
	{
		return 0.0;
	}

	if (obj->weight == 0.0)
	{
		return 0.0;
	}

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

		raw = meas[i].a[obj->meas_index];

		/* Skip invalid measurements (meas_calc sets fields to -1) */
		if (raw == -1.0)
		{
			continue;
		}

		transformed[step_count] = fitness_transform(
			obj->direction, raw, obj->target, obj->exponent);

		/* Clamp infinities and NaNs to a large penalty */
		if (!isfinite(transformed[step_count]))
		{
			transformed[step_count] = 1e6;
		}

		step_count++;
	}

	if (step_count == 0)
	{
		return 0.0;
	}

	reduced = reduce_values(transformed, step_count, obj->reduce);
	return obj->weight * reduced;
}

/*------------------------------------------------------------------------*/

/**
 * fitness_compute - evaluate fitness across frequency steps
 */
double fitness_compute(const fitness_config_t *cfg,
	const measurement_t *meas, int num_steps, const double *freq_mhz)
{
	double total;
	int m;

	total = 0.0;

	for (m = 0; m < cfg->num_obj; m++)
	{
		total += fitness_compute_objective(&cfg->obj[m],
			meas, num_steps, freq_mhz);
	}

	return total;
}
