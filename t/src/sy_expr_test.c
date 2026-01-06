/*
 * SY Expression Evaluator Tests
 * Tests symbolic expression parsing, evaluation, and NEC2 file loading
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glib.h>

#define SY_EXPR_TEST_BUILD 1
#include "../src/sy_expr.h"

static int test_failures = 0;

/* Test helper for floating point comparison */
static gboolean
assert_double_eq(const char *test_name, gdouble actual, gdouble expected, gdouble epsilon)
{
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

/* Test basic arithmetic */
static void
test_basic_arithmetic(void)
{
  gdouble result;

  printf("Testing basic arithmetic...\n");

  sy_init();

  g_assert(sy_evaluate("2+3", &result));
  assert_double_eq("2+3", result, 5.0, 1e-9);

  g_assert(sy_evaluate("10-4", &result));
  assert_double_eq("10-4", result, 6.0, 1e-9);

  g_assert(sy_evaluate("3*4", &result));
  assert_double_eq("3*4", result, 12.0, 1e-9);

  g_assert(sy_evaluate("15/3", &result));
  assert_double_eq("15/3", result, 5.0, 1e-9);

  sy_cleanup();
}

/* Test operator precedence */
static void
test_precedence(void)
{
  gdouble result;

  printf("Testing operator precedence...\n");

  sy_init();

  g_assert(sy_evaluate("2+3*4", &result));
  assert_double_eq("2+3*4", result, 14.0, 1e-9);

  g_assert(sy_evaluate("2*3+4", &result));
  assert_double_eq("2*3+4", result, 10.0, 1e-9);

  g_assert(sy_evaluate("(2+3)*4", &result));
  assert_double_eq("(2+3)*4", result, 20.0, 1e-9);

  sy_cleanup();
}

/* Test exponentiation */
static void
test_exponentiation(void)
{
  gdouble result;

  printf("Testing exponentiation...\n");

  sy_init();

  g_assert(sy_evaluate("2^3", &result));
  assert_double_eq("2^3", result, 8.0, 1e-9);

  g_assert(sy_evaluate("2^3^2", &result));
  assert_double_eq("2^3^2 (right-assoc)", result, 512.0, 1e-9);

  g_assert(sy_evaluate("-3^2", &result));
  assert_double_eq("-3^2", result, -9.0, 1e-9);

  sy_cleanup();
}

/* Test trigonometric functions */
static void
test_trig_functions(void)
{
  gdouble result;

  printf("Testing trigonometric functions...\n");

  sy_init();

  g_assert(sy_evaluate("sin(90)", &result));
  assert_double_eq("sin(90)", result, 1.0, 1e-9);

  g_assert(sy_evaluate("cos(0)", &result));
  assert_double_eq("cos(0)", result, 1.0, 1e-9);

  g_assert(sy_evaluate("tan(45)", &result));
  assert_double_eq("tan(45)", result, 1.0, 1e-6);

  sy_cleanup();
}

/* Test mathematical functions */
static void
test_math_functions(void)
{
  gdouble result;

  printf("Testing mathematical functions...\n");

  sy_init();

  g_assert(sy_evaluate("sqr(16)", &result));
  assert_double_eq("sqr(16)", result, 4.0, 1e-9);

  g_assert(sy_evaluate("abs(-5)", &result));
  assert_double_eq("abs(-5)", result, 5.0, 1e-9);

  g_assert(sy_evaluate("max(3,7)", &result));
  assert_double_eq("max(3,7)", result, 7.0, 1e-9);

  g_assert(sy_evaluate("min(3,7)", &result));
  assert_double_eq("min(3,7)", result, 3.0, 1e-9);

  sy_cleanup();
}

/* Test constants */
static void
test_constants(void)
{
  gdouble result;

  printf("Testing predefined constants...\n");

  sy_init();

  g_assert(sy_evaluate("PI", &result));
  assert_double_eq("PI", result, 3.14159265358979323846, 1e-9);

  g_assert(sy_evaluate("2*PI", &result));
  assert_double_eq("2*PI", result, 2.0 * 3.14159265358979323846, 1e-9);

  g_assert(sy_evaluate("MM", &result));
  assert_double_eq("MM", result, 1.0E-3, 1e-12);

  sy_cleanup();
}

/* Test symbol definition and reference */
static void
test_symbols(void)
{
  gdouble result;

  printf("Testing symbol definition and reference...\n");

  sy_init();

  g_assert(sy_define("H", "0.5"));
  g_assert(sy_evaluate("H", &result));
  assert_double_eq("H", result, 0.5, 1e-9);

  g_assert(sy_evaluate("H*2", &result));
  assert_double_eq("H*2", result, 1.0, 1e-9);

  g_assert(sy_define("fp", "0.0135"));
  g_assert(sy_evaluate("H+fp", &result));
  assert_double_eq("H+fp", result, 0.5135, 1e-9);

  sy_cleanup();
}

/* Test expression detection */
static void
test_is_expression(void)
{
  printf("Testing expression detection...\n");

  g_assert(sy_is_expression("H+fp") == TRUE);
  g_assert(sy_is_expression("2*PI") == TRUE);
  g_assert(sy_is_expression("sin(45)") == TRUE);
  g_assert(sy_is_expression("2^3") == TRUE);

  g_assert(sy_is_expression("3.14") == FALSE);
  g_assert(sy_is_expression("1.5E-3") == FALSE);
  g_assert(sy_is_expression("0.001") == FALSE);

  printf("  PASS: All expression detection tests\n");
}

int
main(int argc, char *argv[])
{
  printf("=== SY Expression Evaluator Tests ===\n\n");

  test_basic_arithmetic();
  test_precedence();
  test_exponentiation();
  test_trig_functions();
  test_math_functions();
  test_constants();
  test_symbols();
  test_is_expression();

  printf("\n=== Test Summary ===\n");
  if( test_failures == 0 )
  {
    printf("All tests PASSED\n");
    return 0;
  }
  else
  {
    printf("%d test(s) FAILED\n", test_failures);
    return 1;
  }
}
