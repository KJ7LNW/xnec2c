/*
 * Particle Swarm Optimization Tests
 *
 * Validates PSO against functions with known minima:
 *   1. Parabola (x+3)^2 - 5  →  min at x=-3, fit=-5
 *   2. Sphere sum(x_i^2)     →  min at origin, fit=0
 *   3. Rosenbrock 2D         →  min at (1,1), fit=0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "particleswarm.h"

static int test_failures = 0;
static int test_count = 0;

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

/* (x+3)^2 - 5, minimum at x=-3, fitness=-5 */
static double fit_parabola(const gsl_vector *pos, void *ctx)
{
	(void)ctx;
	double x = gsl_vector_get(pos, 0);
	return (x + 3.0) * (x + 3.0) - 5.0;
}

/* sum(x_i^2), minimum at origin, fitness=0 */
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

/* Rosenbrock: sum( 100*(x_{i+1} - x_i^2)^2 + (1 - x_i)^2 ) */
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

static void test_parabola(void)
{
	printf("Test: 1D parabola (x+3)^2 - 5\n");

	pso_config_t cfg;
	pso_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.dimensions = 1;
	cfg.pos_min = -10.0;
	cfg.pos_max = 10.0;
	cfg.iterations = 500;

	pso_t *pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  FAIL: pso_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = pso_optimize(pso);
	const gsl_vector *best_pos = pso_get_best_pos(pso);
	double x = gsl_vector_get(best_pos, 0);

	assert_near("fit value", best_fit, -5.0, 0.01);
	assert_near("x position", x, -3.0, 0.01);

	pso_free(pso);
}

static void test_sphere_3d(void)
{
	printf("Test: 3D sphere function\n");

	pso_config_t cfg;
	pso_config_init(&cfg);
	cfg.fit_func = fit_sphere;
	cfg.dimensions = 3;
	cfg.pos_min = -50.0;
	cfg.pos_max = 50.0;
	cfg.iterations = 1000;

	pso_t *pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  FAIL: pso_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = pso_optimize(pso);
	const gsl_vector *best_pos = pso_get_best_pos(pso);

	assert_near("fit value", best_fit, 0.0, 0.1);
	for (int i = 0; i < 3; i++)
	{
		char name[32];
		snprintf(name, sizeof(name), "x[%d] position", i);
		assert_near(name, gsl_vector_get(best_pos, i), 0.0, 0.5);
	}

	pso_free(pso);
}

static void test_rosenbrock_2d(void)
{
	printf("Test: 2D Rosenbrock function\n");

	pso_config_t cfg;
	pso_config_init(&cfg);
	cfg.fit_func = fit_rosenbrock;
	cfg.dimensions = 2;
	cfg.pos_min = -5.0;
	cfg.pos_max = 10.0;
	cfg.iterations = 2000;
	cfg.num_particles = 40;

	pso_t *pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  FAIL: pso_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = pso_optimize(pso);
	const gsl_vector *best_pos = pso_get_best_pos(pso);

	assert_near("fit value", best_fit, 0.0, 1.0);
	assert_near("x[0] position", gsl_vector_get(best_pos, 0), 1.0, 0.5);
	assert_near("x[1] position", gsl_vector_get(best_pos, 1), 1.0, 0.5);

	pso_free(pso);
}

static void test_initial_guess(void)
{
	printf("Test: parabola with initialGuess near solution\n");

	gsl_vector *guess = gsl_vector_alloc(1);
	gsl_vector_set(guess, 0, -2.5);

	pso_config_t cfg;
	pso_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.initial_guess = guess;
	cfg.search_size = 0.3;
	cfg.pos_min = -10.0;
	cfg.pos_max = 10.0;
	cfg.iterations = 200;

	pso_t *pso = pso_new(&cfg);
	gsl_vector_free(guess);

	if (!pso)
	{
		printf("  FAIL: pso_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = pso_optimize(pso);
	const gsl_vector *best_pos = pso_get_best_pos(pso);
	double x = gsl_vector_get(best_pos, 0);

	assert_near("fit value", best_fit, -5.0, 0.01);
	assert_near("x position", x, -3.0, 0.05);

	pso_free(pso);
}

static void test_exit_fit(void)
{
	printf("Test: exitFit early termination\n");

	pso_config_t cfg;
	pso_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.dimensions = 1;
	cfg.pos_min = -10.0;
	cfg.pos_max = 10.0;
	cfg.iterations = 5000;
	cfg.exit_fit = -4.9;

	pso_t *pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  FAIL: pso_new returned NULL\n");
		test_failures++;
		return;
	}

	pso_optimize(pso);
	int iters = pso_get_iteration_count(pso);

	test_count++;
	if (iters < 5000)
	{
		printf("  PASS: early exit at iteration %d (< 5000)\n", iters);
	}
	else
	{
		printf("  FAIL: did not exit early (ran all %d iterations)\n", iters);
		test_failures++;
	}

	assert_near("fit <= exitFit", pso_get_best_fit(pso), -5.0, 0.2);

	pso_free(pso);
}

static void test_config_validation(void)
{
	printf("Test: config validation\n");

	pso_config_t cfg;
	pso_config_init(&cfg);

	/* No fitFunc */
	test_count++;
	pso_t *pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  PASS: NULL fitFunc rejected\n");
	}
	else
	{
		printf("  FAIL: NULL fitFunc accepted\n");
		test_failures++;
		pso_free(pso);
	}

	/* No dimensions and no initialGuess */
	cfg.fit_func = fit_parabola;
	cfg.dimensions = 0;
	test_count++;
	pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  PASS: zero dimensions without guess rejected\n");
	}
	else
	{
		printf("  FAIL: zero dimensions without guess accepted\n");
		test_failures++;
		pso_free(pso);
	}

	/* posMax <= posMin */
	cfg.dimensions = 1;
	cfg.pos_min = 10.0;
	cfg.pos_max = -10.0;
	test_count++;
	pso = pso_new(&cfg);
	if (!pso)
	{
		printf("  PASS: posMax <= posMin rejected\n");
	}
	else
	{
		printf("  FAIL: posMax <= posMin accepted\n");
		test_failures++;
		pso_free(pso);
	}
}

int main(void)
{
	printf("=== PSO Test Suite ===\n\n");

	test_parabola();
	printf("\n");
	test_sphere_3d();
	printf("\n");
	test_rosenbrock_2d();
	printf("\n");
	test_initial_guess();
	printf("\n");
	test_exit_fit();
	printf("\n");
	test_config_validation();

	printf("\n=== Results: %d tests, %d failures ===\n",
		test_count, test_failures);

	return test_failures > 0 ? 1 : 0;
}
