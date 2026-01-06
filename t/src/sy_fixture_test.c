/*
 * Integration tests for SY card implementation
 * Loads .nec fixture files and validates symbol evaluation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glib.h>

#define SY_EXPR_TEST_BUILD 1
#include "../src/sy_expr.h"

extern int Stop(int err, const char *format, ...);

static int test_failures = 0;
static int tests_run = 0;

/* Test helper for floating point comparison */
static gboolean
assert_double_eq(const char *test_name, gdouble actual, gdouble expected, gdouble epsilon)
{
  tests_run++;
  if( fabs(actual - expected) < epsilon )
  {
    printf("  PASS: %s (%.10f)\n", test_name, actual);
    return TRUE;
  }
  else
  {
    printf("  FAIL: %s - expected %.10f, got %.10f\n", test_name, expected, actual);
    test_failures++;
    return FALSE;
  }
}

/* Parse SY definitions from line (simple implementation for testing) */
static gboolean
parse_sy_line(const gchar *line)
{
  gchar **definitions;
  gint i;
  gboolean result = TRUE;

  /* Skip "SY " prefix */
  if( strncmp(line, "SY ", 3) != 0 )
    return FALSE;

  line += 3;

  /* Split by comma */
  definitions = g_strsplit(line, ",", -1);

  for( i = 0; definitions[i] != NULL; i++ )
  {
    gchar *def = g_strstrip(definitions[i]);
    gchar *eq = strchr(def, '=');

    if( eq == NULL )
    {
      result = FALSE;
      break;
    }

    *eq = '\0';
    gchar *name = g_strstrip(def);
    gchar *expr = g_strstrip(eq + 1);

    if( !sy_define(name, expr) )
    {
      fprintf(stderr, "Failed to define: %s = %s\n", name, expr);
      result = FALSE;
      break;
    }
  }

  g_strfreev(definitions);
  return result;
}

/* Test sy_comma_format.nec */
static void
test_comma_format_fixture(void)
{
  FILE *fp;
  gchar line[256];
  gdouble a_val, b_val, c_val;

  printf("\nTesting fixtures/sy_comma_format.nec...\n");

  fp = fopen("fixtures/sy_comma_format.nec", "r");
  if( fp == NULL )
  {
    printf("  SKIP: Could not open fixtures/sy_comma_format.nec\n");
    return;
  }

  sy_init();

  while( fgets(line, sizeof(line), fp) != NULL )
  {
    if( strncmp(line, "SY ", 3) == 0 )
    {
      g_assert(parse_sy_line(line));
    }
  }

  fclose(fp);

  /* Verify symbol values */
  g_assert(sy_evaluate("a", &a_val));
  assert_double_eq("a=1", a_val, 1.0, 1e-9);

  g_assert(sy_evaluate("b", &b_val));
  assert_double_eq("b=a*2=2", b_val, 2.0, 1e-9);

  g_assert(sy_evaluate("c", &c_val));
  assert_double_eq("c=b+1=3", c_val, 3.0, 1e-9);

  sy_cleanup();
}

/* Test sy_separate_cards.nec */
static void
test_separate_cards_fixture(void)
{
  FILE *fp;
  gchar line[256];
  gdouble a_val, b_val, c_val;

  printf("\nTesting fixtures/sy_separate_cards.nec...\n");

  fp = fopen("fixtures/sy_separate_cards.nec", "r");
  if( fp == NULL )
  {
    printf("  SKIP: Could not open fixtures/sy_separate_cards.nec\n");
    return;
  }

  sy_init();

  while( fgets(line, sizeof(line), fp) != NULL )
  {
    if( strncmp(line, "SY ", 3) == 0 )
    {
      g_assert(parse_sy_line(line));
    }
  }

  fclose(fp);

  /* Verify symbol values match comma format */
  g_assert(sy_evaluate("a", &a_val));
  assert_double_eq("a=1", a_val, 1.0, 1e-9);

  g_assert(sy_evaluate("b", &b_val));
  assert_double_eq("b=a*2=2", b_val, 2.0, 1e-9);

  g_assert(sy_evaluate("c", &c_val));
  assert_double_eq("c=b+1=3", c_val, 3.0, 1e-9);

  sy_cleanup();
}

/* Test sy_math_geom.nec */
static void
test_math_geom_fixture(void)
{
  FILE *fp;
  gchar line[256];
  gdouble freq_val, lambda_val, height_val, radius_val;

  printf("\nTesting fixtures/sy_math_geom.nec...\n");

  fp = fopen("fixtures/sy_math_geom.nec", "r");
  if( fp == NULL )
  {
    printf("  SKIP: Could not open fixtures/sy_math_geom.nec\n");
    return;
  }

  sy_init();

  while( fgets(line, sizeof(line), fp) != NULL )
  {
    if( strncmp(line, "SY ", 3) == 0 )
    {
      g_assert(parse_sy_line(line));
    }
  }

  fclose(fp);

  /* Verify calculated values */
  g_assert(sy_evaluate("freq", &freq_val));
  assert_double_eq("freq=1090", freq_val, 1090.0, 1e-9);

  g_assert(sy_evaluate("lambda", &lambda_val));
  assert_double_eq("lambda=300/freq", lambda_val, 300.0/1090.0, 1e-6);

  g_assert(sy_evaluate("height", &height_val));
  assert_double_eq("height=lambda/4", height_val, (300.0/1090.0)/4.0, 1e-6);

  g_assert(sy_evaluate("radius", &radius_val));
  assert_double_eq("radius=1.5*MM", radius_val, 1.5e-3, 1e-9);

  sy_cleanup();
}

/* Test sy_math_cmnd.nec */
static void
test_math_cmnd_fixture(void)
{
  FILE *fp;
  gchar line[256];
  gdouble f_start_val, f_step_val, n_steps_val;

  printf("\nTesting fixtures/sy_math_cmnd.nec...\n");

  fp = fopen("fixtures/sy_math_cmnd.nec", "r");
  if( fp == NULL )
  {
    printf("  SKIP: Could not open fixtures/sy_math_cmnd.nec\n");
    return;
  }

  sy_init();

  while( fgets(line, sizeof(line), fp) != NULL )
  {
    if( strncmp(line, "SY ", 3) == 0 )
    {
      g_assert(parse_sy_line(line));
    }
  }

  fclose(fp);

  /* Verify values */
  g_assert(sy_evaluate("f_start", &f_start_val));
  assert_double_eq("f_start=1000", f_start_val, 1000.0, 1e-9);

  g_assert(sy_evaluate("f_step", &f_step_val));
  assert_double_eq("f_step=10", f_step_val, 10.0, 1e-9);

  g_assert(sy_evaluate("n_steps", &n_steps_val));
  assert_double_eq("n_steps=10", n_steps_val, 10.0, 1e-9);

  sy_cleanup();
}

int
main(int argc, char *argv[])
{
  printf("=== SY Fixture Integration Tests ===\n");

  test_comma_format_fixture();
  test_separate_cards_fixture();
  test_math_geom_fixture();
  test_math_cmnd_fixture();

  printf("\n=== Test Summary ===\n");
  printf("Tests run: %d\n", tests_run);
  if( test_failures == 0 )
  {
    printf("All fixture tests PASSED\n");
    return 0;
  }
  else
  {
    printf("%d test(s) FAILED\n", test_failures);
    return 1;
  }
}
