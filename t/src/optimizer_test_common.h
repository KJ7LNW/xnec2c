/*
 * Shared test utilities for optimizer test suites.
 *
 * Provides fitness functions with known minima and assertion helpers.
 * Include from pso_test.c and simplex_test.c.
 */

#ifndef OPTIMIZER_TEST_COMMON_H
#define OPTIMIZER_TEST_COMMON_H 1

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <gsl/gsl_vector.h>

static int test_failures = 0;
static int test_count = 0;

/**
 * assert_near - check that actual is within tol of expected
 * @name: test name for output
 * @actual: measured value
 * @expected: target value
 * @tol: maximum allowed difference
 */
static int assert_near(const char *name, double actual, double expected, double tol)
{
	test_count++;
	if (fabs(actual - expected) < tol)
	{
		printf("  PASS: %s (got %.6f, want %.6f)\n", name, actual, expected);
		return 1;
	}

	printf("  FAIL: %s (got %.6f, want %.6f, tol %.6f)\n",
		name, actual, expected, tol);
	test_failures++;
	return 0;
}

/** (x+3)^2 - 5, minimum at x=-3, fitness=-5 */
static double fit_parabola(const gsl_vector *pos, void *ctx)
{
	(void)ctx;
	double x = gsl_vector_get(pos, 0);
	return (x + 3.0) * (x + 3.0) - 5.0;
}

/** sum(x_i^2), minimum at origin, fitness=0 */
static double fit_sphere(const gsl_vector *pos, void *ctx)
{
	(void)ctx;
	double sum = 0.0;
	for (size_t i = 0; i < pos->size; i++)
	{
		double x = gsl_vector_get(pos, i);
		sum += x * x;
	}
	return sum;
}

/**
 * test_alloc_uniform_bounds - allocate uniform bound vectors
 * @out_min: output lower bound vector (caller frees)
 * @out_max: output upper bound vector (caller frees)
 * @dims: number of dimensions
 * @lo: lower bound value for all dimensions
 * @hi: upper bound value for all dimensions
 */
static void test_alloc_uniform_bounds(gsl_vector **out_min, gsl_vector **out_max,
	int dims, double lo, double hi)
{
	*out_min = gsl_vector_alloc(dims);
	*out_max = gsl_vector_alloc(dims);
	gsl_vector_set_all(*out_min, lo);
	gsl_vector_set_all(*out_max, hi);
}

/**
 * test_free_bounds - free bound vectors and NULL the pointers
 * @bmin: pointer to lower bound vector pointer
 * @bmax: pointer to upper bound vector pointer
 */
static void test_free_bounds(gsl_vector **bmin, gsl_vector **bmax)
{
	gsl_vector_free(*bmin);
	gsl_vector_free(*bmax);
	*bmin = NULL;
	*bmax = NULL;
}

/** Rosenbrock: sum( 100*(x_{i+1} - x_i^2)^2 + (1 - x_i)^2 ) */
static double fit_rosenbrock(const gsl_vector *pos, void *ctx)
{
	(void)ctx;
	double sum = 0.0;
	for (size_t i = 0; i < pos->size - 1; i++)
	{
		double xi = gsl_vector_get(pos, i);
		double xi1 = gsl_vector_get(pos, i + 1);
		double a = xi1 - xi * xi;
		double b = 1.0 - xi;
		sum += 100.0 * a * a + b * b;
	}
	return sum;
}

#endif
