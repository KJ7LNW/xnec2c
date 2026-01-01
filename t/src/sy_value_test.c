/*
 * Test sy_value_t structure initialization and bounds calculation
 */

#include <glib.h>
#include <math.h>
#include "../../src/sy_expr.h"

static void test_sy_value_bounds_initialized(void)
{
  gdouble min, max;

  sy_init();
  g_assert(sy_define("TEST", 10.0));
  g_assert(sy_get_bounds("TEST", &min, &max));
  g_assert_cmpfloat(min, ==, 5.0);
  g_assert_cmpfloat(max, ==, 20.0);
  sy_cleanup();
}

static void test_sy_value_zero_bounds(void)
{
  gdouble min, max;

  sy_init();
  g_assert(sy_define("ZERO", 0.0));
  g_assert(sy_get_bounds("ZERO", &min, &max));
  g_assert_cmpfloat(min, ==, 0.0);
  g_assert_cmpfloat(max, ==, 0.0);
  sy_cleanup();
}

static void test_sy_value_negative_bounds(void)
{
  gdouble min, max;

  sy_init();
  g_assert(sy_define("NEG", -10.0));
  g_assert(sy_get_bounds("NEG", &min, &max));
  g_assert_cmpfloat(min, ==, -5.0);
  g_assert_cmpfloat(max, ==, -20.0);
  sy_cleanup();
}

static void test_sy_value_small_value_bounds(void)
{
  gdouble min, max;

  sy_init();
  g_assert(sy_define("SMALL", 1e-10));
  g_assert(sy_get_bounds("SMALL", &min, &max));
  g_assert_cmpfloat(min, ==, 5e-11);
  g_assert_cmpfloat(max, ==, 2e-10);
  sy_cleanup();
}

static void test_sy_get_bounds_undefined_symbol(void)
{
  gdouble min, max;

  sy_init();
  g_assert(!sy_get_bounds("UNDEFINED", &min, &max));
  sy_cleanup();
}

static void test_sy_get_bounds_null_parameters(void)
{
  gdouble min, max;

  sy_init();
  g_assert(sy_define("TEST", 10.0));
  g_assert(!sy_get_bounds(NULL, &min, &max));
  g_assert(!sy_get_bounds("TEST", NULL, &max));
  g_assert(!sy_get_bounds("TEST", &min, NULL));
  sy_cleanup();
}

static void test_sy_value_evaluation_unchanged(void)
{
  gdouble result;

  sy_init();
  g_assert(sy_define("X", 15.0));
  g_assert(sy_evaluate("X", &result));
  g_assert_cmpfloat(result, ==, 15.0);
  g_assert(sy_evaluate("X*2", &result));
  g_assert_cmpfloat(result, ==, 30.0);
  sy_cleanup();
}

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/sy_value/bounds_initialized", test_sy_value_bounds_initialized);
  g_test_add_func("/sy_value/zero_bounds", test_sy_value_zero_bounds);
  g_test_add_func("/sy_value/negative_bounds", test_sy_value_negative_bounds);
  g_test_add_func("/sy_value/small_value_bounds", test_sy_value_small_value_bounds);
  g_test_add_func("/sy_value/undefined_symbol", test_sy_get_bounds_undefined_symbol);
  g_test_add_func("/sy_value/null_parameters", test_sy_get_bounds_null_parameters);
  g_test_add_func("/sy_value/evaluation_unchanged", test_sy_value_evaluation_unchanged);

  return g_test_run();
}
