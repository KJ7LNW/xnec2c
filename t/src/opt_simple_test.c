/*
 * Named-variable optimizer ("Simple") test suite.
 *
 * Validates the simple layer against functions with known minima
 * using both simplex and PSO backends:
 *
 *   1.  Parabola via simplex         -> min at x=-3, fit=-5
 *   2.  Parabola via PSO             -> same target
 *   3.  Multi-var with disabled elem -> verify disabled passthrough
 *   4.  Bounds clamping              -> solution clamped to [-1, 1]
 *   5.  perturb_scale                -> convergence with scaled vars
 *   6.  round_result                 -> final values rounded
 *   7.  round_each                   -> per-iteration rounding
 *   8.  Multi-pass ssize             -> multiple passes converge
 *   9.  Stagnant minima              -> early termination
 *  10.  Cache                        -> cache_hits > 0
 *  11.  Config validation            -> reject bad configs
 *  12.  set_vars / set_ssize         -> mutate and re-optimize
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opt_simple.h"

static int test_failures = 0;
static int test_count = 0;

/**
 * assert_near - check that actual is within tol of expected
 * @name: test description
 * @actual: measured value
 * @expected: target value
 * @tol: maximum allowed difference
 */
static int assert_near(const char *name, double actual, double expected,
	double tol)
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

/**
 * assert_true - check boolean condition
 * @name: test description
 * @cond: condition to verify
 */
static int assert_true(const char *name, int cond)
{
	test_count++;
	if (cond)
	{
		printf("  PASS: %s\n", name);
		return 1;
	}

	printf("  FAIL: %s\n", name);
	test_failures++;
	return 0;
}

/* ---- Fitness functions ---- */

/** (x+3)^2 - 5, minimum at x=-3, fitness=-5 */
static double fit_parabola(const simple_var_t *vars, int n, void *ctx)
{
	(void)n;
	(void)ctx;
	double x = gsl_vector_get(vars[0].values, 0);
	return (x + 3.0) * (x + 3.0) - 5.0;
}

/** sum(x_i^2), minimum at origin, fitness=0 */
static double fit_sphere(const simple_var_t *vars, int n, void *ctx)
{
	(void)ctx;
	double sum = 0.0;
	for (int v = 0; v < n; v++)
	{
		int elems = (int)vars[v].values->size;
		for (int e = 0; e < elems; e++)
		{
			double x = gsl_vector_get(vars[v].values, e);
			sum += x * x;
		}
	}
	return sum;
}

/* ---- Log callback for stagnation test ---- */

static int stagnation_log_count = 0;

static void stagnation_log(const simple_var_t *vars, int n,
	const simple_log_state_t *state, void *ctx)
{
	(void)vars;
	(void)n;
	(void)ctx;
	(void)state;
	stagnation_log_count++;
}

/* Captures cache_hits from log state for test_cache */
static int last_cache_hits = 0;

static void cache_log(const simple_var_t *vars, int n,
	const simple_log_state_t *state, void *ctx)
{
	(void)vars;
	(void)n;
	(void)ctx;
	last_cache_hits = state->cache_hits;
}

/* ---- Tests ---- */

static void test_parabola_simplex(void)
{
	printf("Test 1: parabola via simplex\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 3.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	double best = simple_optimize(s);
	assert_near("fit value", best, -5.0, 0.01);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_true("num_vars == 1", nv == 1);
	assert_near("x position", gsl_vector_get(result[0].values, 0), -3.0, 0.01);

	simple_free(s);
}

static void test_parabola_pso(void)
{
	printf("Test 2: parabola via PSO\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 1.0);

	gsl_vector *xmin = gsl_vector_alloc(1);
	gsl_vector_set(xmin, 0, -10.0);
	gsl_vector *xmax = gsl_vector_alloc(1);
	gsl_vector_set(xmax, 0, 10.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .min = xmin, .max = xmax }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_PSO);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(xmin);
	gsl_vector_free(xmax);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	double best = simple_optimize(s);
	assert_near("fit value", best, -5.0, 0.1);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_near("x position", gsl_vector_get(result[0].values, 0), -3.0, 0.1);

	simple_free(s);
}

static void test_multi_var_disabled(void)
{
	printf("Test 3: multi-var with disabled element\n");

	/* Var "a": 3 elements, a[2] disabled at value 99.0 */
	gsl_vector *av = gsl_vector_alloc(3);
	gsl_vector_set(av, 0, 5.0);
	gsl_vector_set(av, 1, -5.0);
	gsl_vector_set(av, 2, 99.0);

	gsl_vector *ae = gsl_vector_alloc(3);
	gsl_vector_set(ae, 0, 1.0);
	gsl_vector_set(ae, 1, 1.0);
	gsl_vector_set(ae, 2, 0.0);

	/* Var "b": 1 element, enabled */
	gsl_vector *bv = gsl_vector_alloc(1);
	gsl_vector_set(bv, 0, 3.0);

	simple_var_t vars[] =
	{
		{ .name = "a", .values = av, .enabled = ae },
		{ .name = "b", .values = bv }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 2;
	cfg.max_iter = 2000;
	cfg.fit_func = fit_sphere;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 2.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(av);
	gsl_vector_free(ae);
	gsl_vector_free(bv);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	simple_optimize(s);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_true("num_vars == 2", nv == 2);

	/* Find var "a" in result (order preserved) */
	const simple_var_t *ra = &result[0];
	const simple_var_t *rb = &result[1];

	/* a[0] and a[1] should be near 0 (minimized) */
	assert_near("a[0] near zero", gsl_vector_get(ra->values, 0), 0.0, 0.5);
	assert_near("a[1] near zero", gsl_vector_get(ra->values, 1), 0.0, 0.5);

	/* a[2] must remain 99.0 (disabled, passthrough) */
	assert_near("a[2] unchanged", gsl_vector_get(ra->values, 2), 99.0, 0.001);

	/* b[0] should be near 0 (minimized) */
	assert_near("b[0] near zero", gsl_vector_get(rb->values, 0), 0.0, 0.5);

	simple_free(s);
}

static void test_bounds_clamping(void)
{
	printf("Test 4: bounds clamping\n");

	/* Parabola min at x=-3, but clamp to [-1, 1] */
	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 0.5);

	gsl_vector *xmin = gsl_vector_alloc(1);
	gsl_vector_set(xmin, 0, -1.0);
	gsl_vector *xmax = gsl_vector_alloc(1);
	gsl_vector_set(xmax, 0, 1.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .min = xmin, .max = xmax }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 1.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(xmin);
	gsl_vector_free(xmax);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	simple_optimize(s);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	double x = gsl_vector_get(result[0].values, 0);

	/* Best within bounds is x=-1.0, fitness=(−1+3)²−5=−1 */
	assert_near("x clamped to -1", x, -1.0, 0.05);

	simple_free(s);
}

static void test_perturb_scale(void)
{
	printf("Test 5: perturb_scale\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	gsl_vector *ps = gsl_vector_alloc(1);
	gsl_vector_set(ps, 0, 5.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .perturb_scale = ps }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 1.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(ps);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	double best = simple_optimize(s);
	assert_near("fit value", best, -5.0, 0.01);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_near("x position", gsl_vector_get(result[0].values, 0), -3.0, 0.01);

	simple_free(s);
}

static void test_round_result(void)
{
	printf("Test 6: round_result\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	/* Round to nearest 0.5 */
	gsl_vector *rr = gsl_vector_alloc(1);
	gsl_vector_set(rr, 0, 0.5);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .round_result = rr }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 3.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(rr);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	simple_optimize(s);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	double x = gsl_vector_get(result[0].values, 0);

	/* x=-3.0 rounded to nearest 0.5 = -3.0 */
	assert_near("x rounded to -3.0", x, -3.0, 0.001);

	/* Verify it is actually a multiple of 0.5 */
	double remainder = fmod(fabs(x), 0.5);
	assert_true("x is multiple of 0.5",
		remainder < 0.001 || fabs(remainder - 0.5) < 0.001);

	simple_free(s);
}

static void test_round_each(void)
{
	printf("Test 7: round_each\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	/* Round to nearest 1.0 on each iteration */
	gsl_vector *re = gsl_vector_alloc(1);
	gsl_vector_set(re, 0, 1.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .round_each = re }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 500;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 3.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(re);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	double best = simple_optimize(s);

	/* With round_each=1.0, best is at x=-3 (integer), fit=-5 */
	assert_near("fit value", best, -5.0, 0.01);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_near("x at integer", gsl_vector_get(result[0].values, 0), -3.0, 0.01);

	simple_free(s);
}

static void test_multipass_ssize(void)
{
	printf("Test 8: multi-pass ssize\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv }
	};

	double ssize_arr[] = { 5.0, 2.0, 0.5 };

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 200;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.ssize = ssize_arr;
	cfg.opts.simplex.num_ssize = 3;

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	double best = simple_optimize(s);
	assert_near("fit value after 3 passes", best, -5.0, 0.01);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);
	assert_near("x position", gsl_vector_get(result[0].values, 0), -3.0, 0.01);

	simple_free(s);
}

static void test_stagnant_minima(void)
{
	printf("Test 9: stagnant minima detection\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, -2.99);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv }
	};

	stagnation_log_count = 0;

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 5000;
	cfg.fit_func = fit_parabola;
	cfg.log_func = stagnation_log;
	cfg.stagnant_minima_count = 5;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 0.001 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	simple_optimize(s);

	/* With tiny ssize near the solution and stagnation limit of 5,
	 * should terminate well before 5000 iterations */
	assert_true("log_count < 5000 (stagnation triggered)",
		stagnation_log_count < 5000);
	assert_near("fit value", simple_get_best_fit(s), -5.0, 0.01);

	simple_free(s);
}

static void test_cache(void)
{
	printf("Test 10: cache hits\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	/* round_each forces repeated values -> cache hits */
	gsl_vector *re = gsl_vector_alloc(1);
	gsl_vector_set(re, 0, 1.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv, .round_each = re }
	};

	simple_config_t cfg;
	last_cache_hits = 0;

	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 200;
	cfg.fit_func = fit_parabola;
	cfg.log_func = cache_log;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 3.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);
	gsl_vector_free(re);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	simple_optimize(s);

	/* With round_each rounding to integers, collisions are expected */
	assert_true("cache_hits > 0", last_cache_hits > 0);

	simple_free(s);
}

static void test_config_validation(void)
{
	printf("Test 11: config validation\n");

	simple_config_t cfg;
	simple_t *s;

	/* No fit_func */
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.num_vars = 1;
	test_count++;
	s = simple_new(&cfg);
	if (!s)
	{
		printf("  PASS: NULL fit_func rejected\n");
	}
	else
	{
		printf("  FAIL: NULL fit_func accepted\n");
		test_failures++;
		simple_free(s);
	}

	/* No vars */
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.fit_func = fit_parabola;
	test_count++;
	s = simple_new(&cfg);
	if (!s)
	{
		printf("  PASS: NULL vars rejected\n");
	}
	else
	{
		printf("  FAIL: NULL vars accepted\n");
		test_failures++;
		simple_free(s);
	}

	/* Var with NULL name */
	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 1.0);
	simple_var_t bad_vars[] = { { .name = NULL, .values = xv } };
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.fit_func = fit_parabola;
	cfg.vars = bad_vars;
	cfg.num_vars = 1;
	test_count++;
	s = simple_new(&cfg);
	if (!s)
	{
		printf("  PASS: NULL var name rejected\n");
	}
	else
	{
		printf("  FAIL: NULL var name accepted\n");
		test_failures++;
		simple_free(s);
	}

	/* perturb_scale with zero */
	gsl_vector *ps = gsl_vector_alloc(1);
	gsl_vector_set(ps, 0, 0.0);
	simple_var_t ps_vars[] = { { .name = "x", .values = xv, .perturb_scale = ps } };
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.fit_func = fit_parabola;
	cfg.vars = ps_vars;
	cfg.num_vars = 1;
	test_count++;
	s = simple_new(&cfg);
	if (!s)
	{
		printf("  PASS: zero perturb_scale rejected\n");
	}
	else
	{
		printf("  FAIL: zero perturb_scale accepted\n");
		test_failures++;
		simple_free(s);
	}

	gsl_vector_free(ps);
	gsl_vector_free(xv);
}

static void test_set_vars_and_ssize(void)
{
	printf("Test 12: set_vars and set_ssize\n");

	gsl_vector *xv = gsl_vector_alloc(1);
	gsl_vector_set(xv, 0, 10.0);

	simple_var_t vars[] =
	{
		{ .name = "x", .values = xv }
	};

	simple_config_t cfg;
	simple_config_init(&cfg, OPT_SIMPLEX);
	cfg.vars = vars;
	cfg.num_vars = 1;
	cfg.max_iter = 300;
	cfg.fit_func = fit_parabola;
	cfg.opts.simplex.num_ssize = 1;
	cfg.opts.simplex.ssize = (double[]){ 3.0 };

	simple_t *s = simple_new(&cfg);
	gsl_vector_free(xv);

	assert_true("simple_new succeeded", s != NULL);
	if (!s)
	{
		return;
	}

	/* First optimization */
	simple_optimize(s);
	double first_fit = simple_get_best_fit(s);

	/* Refine with smaller ssize */
	simple_set_ssize(s, 0.1);

	int nv;
	const simple_var_t *result = simple_get_result(s, &nv);

	/* Feed result back as new vars */
	simple_set_vars(s, result, nv);
	simple_optimize(s);

	double second_fit = simple_get_best_fit(s);
	assert_near("second fit", second_fit, -5.0, 0.001);
	assert_true("refined fit <= first fit", second_fit <= first_fit + 0.001);

	simple_free(s);
}

int main(void)
{
	printf("=== Simple Optimizer Test Suite ===\n\n");

	test_parabola_simplex();
	printf("\n");
	test_parabola_pso();
	printf("\n");
	test_multi_var_disabled();
	printf("\n");
	test_bounds_clamping();
	printf("\n");
	test_perturb_scale();
	printf("\n");
	test_round_result();
	printf("\n");
	test_round_each();
	printf("\n");
	test_multipass_ssize();
	printf("\n");
	test_stagnant_minima();
	printf("\n");
	test_cache();
	printf("\n");
	test_config_validation();
	printf("\n");
	test_set_vars_and_ssize();

	printf("\n=== Results: %d tests, %d failures ===\n",
		test_count, test_failures);

	return test_failures > 0 ? 1 : 0;
}
