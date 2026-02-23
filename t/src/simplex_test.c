/*
 * Nelder-Mead Simplex Optimization Tests
 *
 * Validates simplex against functions with known minima:
 *   1. Parabola (x+3)^2 - 5  ->  min at x=-3, fit=-5
 *   2. Sphere sum(x_i^2)     ->  min at origin, fit=0
 *   3. Rosenbrock 2D         ->  min at (1,1), fit=0
 *   4. Bounded parabola      ->  clamped to [-5,5]
 *   5. exitFit termination   ->  early stop
 *   6. Config validation      ->  reject bad configs
 */

#include <stdlib.h>

#include "simplex.h"
#include "optimizer_test_common.h"

static void test_parabola(void)
{
	printf("Test: 1D parabola (x+3)^2 - 5\n");

	gsl_vector *guess = gsl_vector_alloc(1);
	gsl_vector_set(guess, 0, 10.0);

	simplex_config_t cfg;
	simplex_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.initial_guess = guess;
	cfg.init_size = 3.0;
	cfg.max_iter = 500;

	simplex_t *s = simplex_new(&cfg);
	gsl_vector_free(guess);

	if (!s)
	{
		printf("  FAIL: simplex_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = simplex_optimize(s);
	const gsl_vector *best_pos = simplex_get_best_pos(s);
	double x = gsl_vector_get(best_pos, 0);

	assert_near("fit value", best_fit, -5.0, 0.001);
	assert_near("x position", x, -3.0, 0.001);

	simplex_free(s);
}

static void test_sphere_3d(void)
{
	printf("Test: 3D sphere function\n");

	gsl_vector *guess = gsl_vector_alloc(3);
	gsl_vector_set(guess, 0, 5.0);
	gsl_vector_set(guess, 1, -5.0);
	gsl_vector_set(guess, 2, 3.0);

	simplex_config_t cfg;
	simplex_config_init(&cfg);
	cfg.fit_func = fit_sphere;
	cfg.initial_guess = guess;
	cfg.init_size = 2.0;
	cfg.max_iter = 2000;

	simplex_t *s = simplex_new(&cfg);
	gsl_vector_free(guess);

	if (!s)
	{
		printf("  FAIL: simplex_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = simplex_optimize(s);
	const gsl_vector *best_pos = simplex_get_best_pos(s);

	assert_near("fit value", best_fit, 0.0, 0.01);
	for (int i = 0; i < 3; i++)
	{
		char name[32];
		snprintf(name, sizeof(name), "x[%d] position", i);
		assert_near(name, gsl_vector_get(best_pos, i), 0.0, 0.1);
	}

	simplex_free(s);
}

static void test_rosenbrock_2d(void)
{
	printf("Test: 2D Rosenbrock function\n");

	gsl_vector *guess = gsl_vector_alloc(2);
	gsl_vector_set(guess, 0, -1.0);
	gsl_vector_set(guess, 1, 1.0);

	simplex_config_t cfg;
	simplex_config_init(&cfg);
	cfg.fit_func = fit_rosenbrock;
	cfg.initial_guess = guess;
	cfg.init_size = 1.0;
	cfg.max_iter = 5000;
	cfg.min_size = 1e-8;

	simplex_t *s = simplex_new(&cfg);
	gsl_vector_free(guess);

	if (!s)
	{
		printf("  FAIL: simplex_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = simplex_optimize(s);
	const gsl_vector *best_pos = simplex_get_best_pos(s);

	assert_near("fit value", best_fit, 0.0, 0.1);
	assert_near("x[0] position", gsl_vector_get(best_pos, 0), 1.0, 0.1);
	assert_near("x[1] position", gsl_vector_get(best_pos, 1), 1.0, 0.1);

	simplex_free(s);
}

static void test_bounded_parabola(void)
{
	printf("Test: bounded parabola with posMin/posMax\n");

	/* Guess at 4.0, bounds [-5, 5], minimum at x=-3 is within bounds */
	gsl_vector *guess = gsl_vector_alloc(1);
	gsl_vector_set(guess, 0, 4.0);

	simplex_config_t cfg;
	simplex_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.initial_guess = guess;
	cfg.init_size = 3.0;
	cfg.max_iter = 500;

	test_alloc_uniform_bounds(&cfg.pos_min, &cfg.pos_max, 1, -5.0, 5.0);

	simplex_t *s = simplex_new(&cfg);
	gsl_vector_free(guess);
	test_free_bounds(&cfg.pos_min, &cfg.pos_max);

	if (!s)
	{
		printf("  FAIL: simplex_new returned NULL\n");
		test_failures++;
		return;
	}

	double best_fit = simplex_optimize(s);
	const gsl_vector *best_pos = simplex_get_best_pos(s);
	double x = gsl_vector_get(best_pos, 0);

	assert_near("fit value", best_fit, -5.0, 0.01);
	assert_near("x position", x, -3.0, 0.01);

	/* Verify position is within bounds */
	test_count++;
	if (x >= -5.0 && x <= 5.0)
	{
		printf("  PASS: x=%.6f within bounds [-5, 5]\n", x);
	}
	else
	{
		printf("  FAIL: x=%.6f outside bounds [-5, 5]\n", x);
		test_failures++;
	}

	simplex_free(s);
}

static void test_exit_fit(void)
{
	printf("Test: exitFit early termination\n");

	gsl_vector *guess = gsl_vector_alloc(1);
	gsl_vector_set(guess, 0, 10.0);

	simplex_config_t cfg;
	simplex_config_init(&cfg);
	cfg.fit_func = fit_parabola;
	cfg.initial_guess = guess;
	cfg.init_size = 3.0;
	cfg.max_iter = 5000;
	cfg.exit_fit = -4.9;

	simplex_t *s = simplex_new(&cfg);
	gsl_vector_free(guess);

	if (!s)
	{
		printf("  FAIL: simplex_new returned NULL\n");
		test_failures++;
		return;
	}

	simplex_optimize(s);
	int iters = simplex_get_iteration_count(s);

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

	assert_near("fit <= exitFit", simplex_get_best_fit(s), -5.0, 0.2);

	simplex_free(s);
}

static void test_config_validation(void)
{
	printf("Test: config validation\n");

	simplex_config_t cfg;
	simplex_config_init(&cfg);

	/* No fitFunc */
	test_count++;
	simplex_t *s = simplex_new(&cfg);
	if (!s)
	{
		printf("  PASS: NULL fitFunc rejected\n");
	}
	else
	{
		printf("  FAIL: NULL fitFunc accepted\n");
		test_failures++;
		simplex_free(s);
	}

	/* No initialGuess and no initialSimplex */
	cfg.fit_func = fit_parabola;
	test_count++;
	s = simplex_new(&cfg);
	if (!s)
	{
		printf("  PASS: no guess/simplex rejected\n");
	}
	else
	{
		printf("  FAIL: no guess/simplex accepted\n");
		test_failures++;
		simplex_free(s);
	}

	/* posMax <= posMin */
	gsl_vector *guess = gsl_vector_alloc(1);
	gsl_vector_set(guess, 0, 0.0);
	cfg.initial_guess = guess;

	test_alloc_uniform_bounds(&cfg.pos_min, &cfg.pos_max, 1, 10.0, -10.0);
	test_count++;
	s = simplex_new(&cfg);
	test_free_bounds(&cfg.pos_min, &cfg.pos_max);
	if (!s)
	{
		printf("  PASS: posMax <= posMin rejected\n");
	}
	else
	{
		printf("  FAIL: posMax <= posMin accepted\n");
		test_failures++;
		simplex_free(s);
	}

	gsl_vector_free(guess);
}

int main(void)
{
	printf("=== Simplex Test Suite ===\n\n");

	test_parabola();
	printf("\n");
	test_sphere_3d();
	printf("\n");
	test_rosenbrock_2d();
	printf("\n");
	test_bounded_parabola();
	printf("\n");
	test_exit_fit();
	printf("\n");
	test_config_validation();

	printf("\n=== Results: %d tests, %d failures ===\n",
		test_count, test_failures);

	return test_failures > 0 ? 1 : 0;
}
