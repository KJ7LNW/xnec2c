/*
 *  Unit tests for opt_fitness.c
 *
 *  Tests fitness_transform(), reduce_values (via fitness_compute),
 *  and full fitness_compute() with synthetic measurement_t arrays.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "opt_fitness.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_NEAR(actual, expected, tol, msg) \
	do { \
		tests_run++; \
		double _a = (actual), _e = (expected), _t = (tol); \
		if (fabs(_a - _e) <= _t) { \
			tests_passed++; \
		} \
		else { \
			tests_failed++; \
			fprintf(stderr, "FAIL %s:%d: %s\n" \
				"  expected: %.10g  actual: %.10g  diff: %.10g  tol: %.10g\n", \
				__FILE__, __LINE__, (msg), _e, _a, fabs(_a - _e), _t); \
		} \
	} while (0)

#define ASSERT_TRUE(cond, msg) \
	do { \
		tests_run++; \
		if ((cond)) { \
			tests_passed++; \
		} \
		else { \
			tests_failed++; \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
		} \
	} while (0)

/*------------------------------------------------------------------------*/

/**
 * build_meas - construct synthetic measurement_t array
 * @n: number of steps
 * @field_idx: MEAS_* index to populate
 * @values: array of n values for the field
 * @freq: array of n frequencies in MHz
 * @out: output measurement_t array (caller allocated)
 *
 * Initializes all fields to -1, then sets the specified field.
 */
static void build_meas(int n, int field_idx, const double *values,
	measurement_t *out)
{
	int i;
	int j;

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < MEAS_COUNT; j++)
		{
			out[i].a[j] = -1.0;
		}

		out[i].a[field_idx] = values[i];
	}
}

/*------------------------------------------------------------------------*/

static void test_transform_minimize(void)
{
	double result;

	/* At target: score = 1.0 */
	result = fitness_transform(FIT_DIR_MINIMIZE, 1.5, 1.5, 2.0);
	ASSERT_NEAR(result, 1.0, 1e-10, "MINIMIZE at target");

	/* Worse than target (VSWR 3.0, target 1.5): score = (3.0/1.5)^2 = 4.0 */
	result = fitness_transform(FIT_DIR_MINIMIZE, 3.0, 1.5, 2.0);
	ASSERT_NEAR(result, 4.0, 1e-10, "MINIMIZE worse than target");

	/* Better than target (VSWR 1.2, target 1.5): score = (1.2/1.5)^2 = 0.64 */
	result = fitness_transform(FIT_DIR_MINIMIZE, 1.2, 1.5, 2.0);
	ASSERT_NEAR(result, 0.64, 1e-10, "MINIMIZE better than target");

	/* Exponent 1.0: linear */
	result = fitness_transform(FIT_DIR_MINIMIZE, 3.0, 1.5, 1.0);
	ASSERT_NEAR(result, 2.0, 1e-10, "MINIMIZE linear exponent");
}

/*------------------------------------------------------------------------*/

static void test_transform_maximize(void)
{
	double result;

	/* At target: score = 1.0 */
	result = fitness_transform(FIT_DIR_MAXIMIZE, 8.0, 8.0, 0.5);
	ASSERT_NEAR(result, 1.0, 1e-10, "MAXIMIZE at target");

	/* Better than target (gain 10, target 8): score = (8/10)^0.5 */
	result = fitness_transform(FIT_DIR_MAXIMIZE, 10.0, 8.0, 0.5);
	ASSERT_NEAR(result, pow(0.8, 0.5), 1e-10, "MAXIMIZE better than target");

	/* Worse than target (gain 4, target 8): score = (8/4)^0.5 */
	result = fitness_transform(FIT_DIR_MAXIMIZE, 4.0, 8.0, 0.5);
	ASSERT_NEAR(result, pow(2.0, 0.5), 1e-10, "MAXIMIZE worse than target");

	/* S11: target -15, value -20 (better): |15|/|20| = 0.75 */
	result = fitness_transform(FIT_DIR_MAXIMIZE, -20.0, -15.0, 2.0);
	ASSERT_NEAR(result, pow(0.75, 2.0), 1e-10, "MAXIMIZE S11 better");

	/* S11: target -15, value -10 (worse): |15|/|10| = 1.5 */
	result = fitness_transform(FIT_DIR_MAXIMIZE, -10.0, -15.0, 2.0);
	ASSERT_NEAR(result, pow(1.5, 2.0), 1e-10, "MAXIMIZE S11 worse");

	/* Near-zero value: epsilon guard */
	result = fitness_transform(FIT_DIR_MAXIMIZE, 0.0, 8.0, 1.0);
	ASSERT_TRUE(result > 1e6, "MAXIMIZE near-zero value gives large penalty");
}

/*------------------------------------------------------------------------*/

static void test_transform_deviate(void)
{
	double result;

	/* At target: score = 0.0 */
	result = fitness_transform(FIT_DIR_DEVIATE, 90.0, 90.0, 2.0);
	ASSERT_NEAR(result, 0.0, 1e-10, "DEVIATE at target");

	/* 5 degrees off: |95-90|^2 = 25 */
	result = fitness_transform(FIT_DIR_DEVIATE, 95.0, 90.0, 2.0);
	ASSERT_NEAR(result, 25.0, 1e-10, "DEVIATE 5 deg positive");

	/* Symmetric: -5 degrees */
	result = fitness_transform(FIT_DIR_DEVIATE, 85.0, 90.0, 2.0);
	ASSERT_NEAR(result, 25.0, 1e-10, "DEVIATE 5 deg negative");

	/* Exponent 1.0: linear deviation */
	result = fitness_transform(FIT_DIR_DEVIATE, 95.0, 90.0, 1.0);
	ASSERT_NEAR(result, 5.0, 1e-10, "DEVIATE linear exponent");
}

/*------------------------------------------------------------------------*/

static void test_config_init(void)
{
	fitness_config_t cfg;

	fitness_config_init(&cfg);

	ASSERT_TRUE(cfg.obj[FIT_VSWR].enabled == 1, "VSWR enabled by default");
	ASSERT_TRUE(cfg.obj[FIT_GAIN_MAX].enabled == 1, "GAIN_MAX enabled by default");
	ASSERT_TRUE(cfg.obj[FIT_FB_RATIO].enabled == 1, "FB_RATIO enabled by default");
	ASSERT_TRUE(cfg.obj[FIT_GAIN_THETA].enabled == 0, "GAIN_THETA disabled by default");
	ASSERT_TRUE(cfg.obj[FIT_GAIN_PHI].enabled == 0, "GAIN_PHI disabled by default");
	ASSERT_TRUE(cfg.obj[FIT_S11].enabled == 0, "S11 disabled by default");

	ASSERT_NEAR(cfg.obj[FIT_VSWR].target, 1.5, 1e-10, "VSWR default target");
	ASSERT_NEAR(cfg.obj[FIT_VSWR].weight, 5.0, 1e-10, "VSWR default weight");
	ASSERT_NEAR(cfg.obj[FIT_VSWR].exponent, 2.0, 1e-10, "VSWR default exponent");

	ASSERT_NEAR(cfg.obj[FIT_GAIN_MAX].target, 8.0, 1e-10, "GAIN_MAX default target");
	ASSERT_NEAR(cfg.obj[FIT_GAIN_MAX].weight, 10.0, 1e-10, "GAIN_MAX default weight");

	ASSERT_TRUE(isnan(cfg.obj[FIT_VSWR].mhz_min), "mhz_min defaults to NAN");
	ASSERT_TRUE(isnan(cfg.obj[FIT_VSWR].mhz_max), "mhz_max defaults to NAN");
}

/*------------------------------------------------------------------------*/

static void test_compute_vswr_only(void)
{
	fitness_config_t cfg;
	measurement_t meas[4];
	double freq[4] = { 144.0, 145.0, 146.0, 147.0 };
	double vswr_vals[4] = { 1.5, 2.0, 1.8, 1.5 };
	double result;
	double expected;

	fitness_config_init(&cfg);

	/* Disable gain, keep only VSWR */
	cfg.obj[FIT_GAIN_MAX].enabled = 0;

	build_meas(4, MEAS_VSWR, vswr_vals, meas);

	result = fitness_compute(&cfg, meas, 4, freq);

	/* avg of pow(val/1.5, 2.0) for each step:
	 * step 0: (1.5/1.5)^2 = 1.0
	 * step 1: (2.0/1.5)^2 = 1.7778
	 * step 2: (1.8/1.5)^2 = 1.44
	 * step 3: (1.5/1.5)^2 = 1.0
	 * avg = (1.0 + 1.7778 + 1.44 + 1.0) / 4 = 1.3044
	 * weight 5.0 * 1.3044 = 6.5222
	 */
	expected = 5.0 * (1.0 + pow(2.0/1.5, 2.0) + pow(1.8/1.5, 2.0) + 1.0) / 4.0;
	ASSERT_NEAR(result, expected, 1e-6, "VSWR-only compute");
}

/*------------------------------------------------------------------------*/

static void test_compute_gain_only(void)
{
	fitness_config_t cfg;
	measurement_t meas[3];
	double freq[3] = { 144.0, 146.0, 148.0 };
	double gain_vals[3] = { 8.0, 10.0, 6.0 };
	double result;
	double expected;

	fitness_config_init(&cfg);

	/* Disable VSWR, keep only gain */
	cfg.obj[FIT_VSWR].enabled = 0;

	build_meas(3, MEAS_GAIN_MAX, gain_vals, meas);

	result = fitness_compute(&cfg, meas, 3, freq);

	/* avg of pow(|8.0|/max(|val|,eps), 0.5):
	 * step 0: (8/8)^0.5 = 1.0
	 * step 1: (8/10)^0.5 = 0.8944
	 * step 2: (8/6)^0.5 = 1.1547
	 * avg = (1.0 + 0.8944 + 1.1547) / 3
	 * weight 10.0 * avg
	 */
	expected = 10.0 * (pow(8.0/8.0, 0.5) + pow(8.0/10.0, 0.5) + pow(8.0/6.0, 0.5)) / 3.0;
	ASSERT_NEAR(result, expected, 1e-6, "GAIN-only compute");
}

/*------------------------------------------------------------------------*/

static void test_compute_combined(void)
{
	fitness_config_t cfg;
	measurement_t meas[2];
	double freq[2] = { 145.0, 146.0 };
	double vswr_vals[2] = { 1.5, 2.0 };
	double gain_vals[2] = { 8.0, 10.0 };
	double result;
	double vswr_contrib;
	double gain_contrib;
	int i;

	fitness_config_init(&cfg);

	/* Build measurements with both fields populated */
	for (i = 0; i < 2; i++)
	{
		int j;

		for (j = 0; j < MEAS_COUNT; j++)
		{
			meas[i].a[j] = -1.0;
		}

		meas[i].a[MEAS_VSWR] = vswr_vals[i];
		meas[i].a[MEAS_GAIN_MAX] = gain_vals[i];
	}

	result = fitness_compute(&cfg, meas, 2, freq);

	vswr_contrib = 5.0 * (pow(1.5/1.5, 2.0) + pow(2.0/1.5, 2.0)) / 2.0;
	gain_contrib = 10.0 * (pow(8.0/8.0, 0.5) + pow(8.0/10.0, 0.5)) / 2.0;

	ASSERT_NEAR(result, vswr_contrib + gain_contrib, 1e-6, "combined VSWR+gain");
}

/*------------------------------------------------------------------------*/

static void test_compute_mhz_range(void)
{
	fitness_config_t cfg;
	measurement_t meas[5];
	double freq[5] = { 140.0, 144.0, 146.0, 148.0, 152.0 };
	double vswr_vals[5] = { 5.0, 1.5, 1.8, 1.5, 5.0 };
	double result_all;
	double result_band;

	fitness_config_init(&cfg);
	cfg.obj[FIT_GAIN_MAX].enabled = 0;

	build_meas(5, MEAS_VSWR, vswr_vals, meas);

	/* All frequencies */
	result_all = fitness_compute(&cfg, meas, 5, freq);

	/* Restrict to 144-148 MHz */
	cfg.obj[FIT_VSWR].mhz_min = 144.0;
	cfg.obj[FIT_VSWR].mhz_max = 148.0;

	result_band = fitness_compute(&cfg, meas, 5, freq);

	/* Band result excludes the 5.0 VSWR outliers */
	ASSERT_TRUE(result_band < result_all, "MHz range excludes outliers");
}

/*------------------------------------------------------------------------*/

static void test_compute_invalid_measurements(void)
{
	fitness_config_t cfg;
	measurement_t meas[3];
	double freq[3] = { 144.0, 146.0, 148.0 };
	double result;

	fitness_config_init(&cfg);
	cfg.obj[FIT_GAIN_MAX].enabled = 0;

	/* All fields set to -1 (invalid sentinel) */
	int i;
	int j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < MEAS_COUNT; j++)
		{
			meas[i].a[j] = -1.0;
		}
	}

	result = fitness_compute(&cfg, meas, 3, freq);

	/* No valid steps: result is 0 (no contributions) */
	ASSERT_NEAR(result, 0.0, 1e-10, "all invalid measurements yield 0");
}

/*------------------------------------------------------------------------*/

static void test_reduce_diff(void)
{
	fitness_config_t cfg;
	measurement_t meas[4];
	double freq[4] = { 144.0, 145.0, 146.0, 147.0 };
	double gain_vals[4] = { 6.0, 8.0, 10.0, 7.0 };
	double result;

	fitness_config_init(&cfg);

	/* Enable only gain flatness with diff reduce */
	cfg.obj[FIT_VSWR].enabled = 0;
	cfg.obj[FIT_GAIN_MAX].enabled = 0;
	cfg.obj[FIT_GAIN_FLAT].enabled = 1;

	build_meas(4, MEAS_GAIN_MAX, gain_vals, meas);

	result = fitness_compute(&cfg, meas, 4, freq);

	/* diff reduce: max - min of transformed values
	 * FIT_GAIN_FLAT uses MINIMIZE direction, target=3.0, exp=1.0
	 * transformed: 6/3=2.0, 8/3=2.667, 10/3=3.333, 7/3=2.333
	 * diff = 3.333 - 2.0 = 1.333
	 * weight = 1.0 * 1.333
	 */
	double t0 = 6.0 / 3.0;
	double t1 = 8.0 / 3.0;
	double t2 = 10.0 / 3.0;
	double t3 = 7.0 / 3.0;
	double expected = 1.0 * (t2 - t0);

	ASSERT_NEAR(result, expected, 1e-6, "diff reduce for gain flatness");
}

/*------------------------------------------------------------------------*/

static void test_reduce_max(void)
{
	fitness_config_t cfg;
	measurement_t meas[3];
	double freq[3] = { 144.0, 146.0, 148.0 };
	double vswr_vals[3] = { 1.5, 3.0, 1.8 };
	double result;
	double expected;

	fitness_config_init(&cfg);
	cfg.obj[FIT_GAIN_MAX].enabled = 0;
	cfg.obj[FIT_VSWR].reduce = FIT_REDUCE_MAX;

	build_meas(3, MEAS_VSWR, vswr_vals, meas);

	result = fitness_compute(&cfg, meas, 3, freq);

	/* max reduce picks worst VSWR: (3.0/1.5)^2 = 4.0 */
	expected = 5.0 * pow(3.0 / 1.5, 2.0);

	ASSERT_NEAR(result, expected, 1e-6, "max reduce picks worst VSWR");
}

/*------------------------------------------------------------------------*/

static void test_metric_info_table(void)
{
	int i;

	for (i = 0; i < FIT_METRIC_COUNT; i++)
	{
		ASSERT_TRUE(fitness_metric_info[i].name != NULL,
			"metric_info name non-NULL");
		ASSERT_TRUE(fitness_metric_info[i].metric == i,
			"metric_info index matches enum");
		ASSERT_TRUE(fitness_metric_info[i].meas_index >= 0 &&
			fitness_metric_info[i].meas_index < MEAS_COUNT,
			"metric_info meas_index in range");
	}

	/* Verify reduce names table */
	for (i = 0; i < FIT_REDUCE_COUNT; i++)
	{
		ASSERT_TRUE(fitness_reduce_names[i] != NULL,
			"reduce name non-NULL");
	}
}

/*------------------------------------------------------------------------*/

static void test_zero_weight_skipped(void)
{
	fitness_config_t cfg;
	measurement_t meas[2];
	double freq[2] = { 145.0, 146.0 };
	double vswr_vals[2] = { 5.0, 5.0 };
	double result;

	fitness_config_init(&cfg);
	cfg.obj[FIT_GAIN_MAX].enabled = 0;
	cfg.obj[FIT_VSWR].weight = 0.0;

	build_meas(2, MEAS_VSWR, vswr_vals, meas);

	result = fitness_compute(&cfg, meas, 2, freq);

	ASSERT_NEAR(result, 0.0, 1e-10, "zero weight contributes nothing");
}

/*------------------------------------------------------------------------*/

int main(void)
{
	printf("opt_fitness_test: running tests\n");

	test_transform_minimize();
	test_transform_maximize();
	test_transform_deviate();
	test_config_init();
	test_compute_vswr_only();
	test_compute_gain_only();
	test_compute_combined();
	test_compute_mhz_range();
	test_compute_invalid_measurements();
	test_reduce_diff();
	test_reduce_max();
	test_metric_info_table();
	test_zero_weight_skipped();

	printf("\nopt_fitness_test: %d tests, %d passed, %d failed\n",
		tests_run, tests_passed, tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
