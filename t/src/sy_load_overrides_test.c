/*
 * Test sy_load_overrides() functionality
 * Verifies loading symbol overrides from .sy files
 */

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include "../../src/sy_expr.h"

/* Path to test fixture - relative to t/ directory where tests run */
#define TEST_SY_FILE "fixtures/test_overrides.sy"
#define NONEXISTENT_FILE "fixtures/nonexistent.sy"

static void test_load_overrides_basic(void)
{
  gdouble min, max;

  sy_init();

  /* Load overrides from fixture file */
  g_assert(sy_load_overrides(TEST_SY_FILE));

  /* Verify HEIGHT was loaded with override values */
  g_assert(sy_get_bounds("HEIGHT", &min, &max));
  g_assert_cmpfloat(min, ==, 0.5);
  g_assert_cmpfloat(max, ==, 10.0);

  /* Verify WIDTH was loaded */
  g_assert(sy_get_bounds("WIDTH", &min, &max));
  g_assert_cmpfloat(min, ==, 1.0);
  g_assert_cmpfloat(max, ==, 5.0);

  /* Verify FREQ was loaded with partial values */
  g_assert(sy_get_bounds("FREQ", &min, &max));
  g_assert_cmpfloat(min, ==, 100.0);
  g_assert_cmpfloat(max, ==, 500.0);

  sy_cleanup();
}

static void test_load_overrides_nonexistent_file(void)
{
  sy_init();

  /* Nonexistent file returns FALSE silently */
  g_assert(!sy_load_overrides(NONEXISTENT_FILE));

  sy_cleanup();
}

static void test_load_overrides_null_filename(void)
{
  sy_init();

  /* NULL filename returns FALSE */
  g_assert(!sy_load_overrides(NULL));

  sy_cleanup();
}

static void test_load_overrides_preserves_on_define(void)
{
  gdouble min, max, result;

  sy_init();

  /* Load overrides first */
  g_assert(sy_load_overrides(TEST_SY_FILE));

  /* Define same symbol - min/max from .sy file preserved */
  g_assert(sy_define("HEIGHT", "5.0"));

  /* Bounds remain from .sy file */
  g_assert(sy_get_bounds("HEIGHT", &min, &max));
  g_assert_cmpfloat(min, ==, 0.5);
  g_assert_cmpfloat(max, ==, 10.0);

  /* Value evaluation uses override since override_active=1 */
  g_assert(sy_evaluate("HEIGHT", &result));
  g_assert_cmpfloat(result, ==, 2.5);

  sy_cleanup();
}

static void test_load_overrides_inactive_uses_defined_value(void)
{
  gdouble result;

  sy_init();

  /* Load overrides - WIDTH has override_active=0 */
  g_assert(sy_load_overrides(TEST_SY_FILE));

  /* Define WIDTH with a value */
  g_assert(sy_define("WIDTH", "7.0"));

  /* Value evaluation uses defined value since override_active=0 */
  g_assert(sy_evaluate("WIDTH", &result));
  g_assert_cmpfloat(result, ==, 7.0);

  sy_cleanup();
}

static void test_define_sets_defaults(void)
{
  gdouble min, max, result;

  sy_init();

  /* Define symbol without .sy file - defaults applied */
  g_assert(sy_define("NEWVAR", "20.0"));

  /* Bounds: min = value * 0.5, max = value * 2.0 */
  g_assert(sy_get_bounds("NEWVAR", &min, &max));
  g_assert_cmpfloat(min, ==, 10.0);
  g_assert_cmpfloat(max, ==, 40.0);

  /* Evaluation returns defined value (override_active defaults FALSE) */
  g_assert(sy_evaluate("NEWVAR", &result));
  g_assert_cmpfloat(result, ==, 20.0);

  sy_cleanup();
}

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/sy_load_overrides/basic", test_load_overrides_basic);
  g_test_add_func("/sy_load_overrides/nonexistent_file", test_load_overrides_nonexistent_file);
  g_test_add_func("/sy_load_overrides/null_filename", test_load_overrides_null_filename);
  g_test_add_func("/sy_load_overrides/preserves_on_define", test_load_overrides_preserves_on_define);
  g_test_add_func("/sy_load_overrides/inactive_uses_defined_value", test_load_overrides_inactive_uses_defined_value);
  g_test_add_func("/sy_load_overrides/define_sets_defaults", test_define_sets_defaults);

  return g_test_run();
}
