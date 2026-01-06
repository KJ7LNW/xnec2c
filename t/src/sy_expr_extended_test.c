/*
 * Test suite for SY expression evaluator
 * Tests symbolic variable definitions and expression evaluation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../src/sy_expr.h"

#define EPSILON 1e-6

static int tests_run = 0;
static int tests_passed = 0;

static void
test_result(const char *test_name, int passed)
{
  tests_run++;
  if( passed )
  {
    tests_passed++;
    printf("PASS: %s\n", test_name);
  }
  else
    printf("FAIL: %s\n", test_name);
}

static void
test_init_cleanup(void)
{
  int result;

  result = sy_init();
  test_result("sy_init returns TRUE", result == 1);

  sy_cleanup();
  test_result("sy_cleanup executes without crash", 1);
}

static void
test_basic_symbols(void)
{
  double value;
  int result;

  sy_init();

  result = sy_define("H", "0.5");
  test_result("Define symbol H=0.5", result == 1);

  result = sy_evaluate("H", &value);
  test_result("Evaluate H", result == 1 && fabs(value - 0.5) < EPSILON);

  result = sy_define("fp", "0.0135");
  test_result("Define symbol fp=0.0135", result == 1);

  result = sy_evaluate("fp", &value);
  test_result("Evaluate fp", result == 1 && fabs(value - 0.0135) < EPSILON);

  sy_cleanup();
}

static void
test_arithmetic_operators(void)
{
  double value;
  int result;

  sy_init();
  sy_define("A", "10.0");
  sy_define("B", "3.0");

  result = sy_evaluate("A+B", &value);
  test_result("Addition A+B=13", result == 1 && fabs(value - 13.0) < EPSILON);

  result = sy_evaluate("A-B", &value);
  test_result("Subtraction A-B=7", result == 1 && fabs(value - 7.0) < EPSILON);

  result = sy_evaluate("A*B", &value);
  test_result("Multiplication A*B=30", result == 1 && fabs(value - 30.0) < EPSILON);

  result = sy_evaluate("A/B", &value);
  test_result("Division A/B=3.333...", result == 1 && fabs(value - 10.0/3.0) < EPSILON);

  result = sy_evaluate("B^2", &value);
  test_result("Exponentiation B^2=9", result == 1 && fabs(value - 9.0) < EPSILON);

  sy_cleanup();
}

static void
test_operator_precedence(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("2+3*4", &value);
  test_result("Precedence 2+3*4=14", result == 1 && fabs(value - 14.0) < EPSILON);

  result = sy_evaluate("2^3^2", &value);
  test_result("Right-assoc 2^3^2=512", result == 1 && fabs(value - 512.0) < EPSILON);

  result = sy_evaluate("(2+3)*4", &value);
  test_result("Parentheses (2+3)*4=20", result == 1 && fabs(value - 20.0) < EPSILON);

  sy_cleanup();
}

static void
test_trig_functions(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("sin(30)", &value);
  test_result("sin(30 deg)=0.5", result == 1 && fabs(value - 0.5) < EPSILON);

  result = sy_evaluate("cos(60)", &value);
  test_result("cos(60 deg)=0.5", result == 1 && fabs(value - 0.5) < EPSILON);

  result = sy_evaluate("tan(45)", &value);
  test_result("tan(45 deg)=1", result == 1 && fabs(value - 1.0) < 1e-6);

  sy_cleanup();
}

static void
test_math_functions(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("sqr(16)", &value);
  test_result("sqr(16)=4", result == 1 && fabs(value - 4.0) < EPSILON);

  result = sy_evaluate("abs(-5.5)", &value);
  test_result("abs(-5.5)=5.5", result == 1 && fabs(value - 5.5) < EPSILON);

  result = sy_evaluate("max(10,20)", &value);
  test_result("max(10,20)=20", result == 1 && fabs(value - 20.0) < EPSILON);

  result = sy_evaluate("min(10,20)", &value);
  test_result("min(10,20)=10", result == 1 && fabs(value - 10.0) < EPSILON);

  result = sy_evaluate("atn(1)", &value);
  test_result("atn(1)=45 deg", result == 1 && fabs(value - 45.0) < 1e-6);

  result = sy_evaluate("exp(1)", &value);
  test_result("exp(1)=e", result == 1 && fabs(value - 2.71828182845905) < 1e-10);

  result = sy_evaluate("log(2.71828182845905)", &value);
  test_result("log(e)=1", result == 1 && fabs(value - 1.0) < 1e-6);

  result = sy_evaluate("log10(100)", &value);
  test_result("log10(100)=2", result == 1 && fabs(value - 2.0) < EPSILON);

  result = sy_evaluate("sgn(5)", &value);
  test_result("sgn(5)=1", result == 1 && fabs(value - 1.0) < EPSILON);

  result = sy_evaluate("sgn(-5)", &value);
  test_result("sgn(-5)=-1", result == 1 && fabs(value + 1.0) < EPSILON);

  result = sy_evaluate("sgn(0)", &value);
  test_result("sgn(0)=0", result == 1 && fabs(value) < EPSILON);

  result = sy_evaluate("int(3.7)", &value);
  test_result("int(3.7)=4", result == 1 && fabs(value - 4.0) < EPSILON);

  result = sy_evaluate("int(3.2)", &value);
  test_result("int(3.2)=3", result == 1 && fabs(value - 3.0) < EPSILON);

  result = sy_evaluate("fix(3.7)", &value);
  test_result("fix(3.7)=3", result == 1 && fabs(value - 3.0) < EPSILON);

  result = sy_evaluate("fix(-3.7)", &value);
  test_result("fix(-3.7)=-3", result == 1 && fabs(value + 3.0) < EPSILON);

  result = sy_evaluate("mod(10,3)", &value);
  test_result("mod(10,3)=1", result == 1 && fabs(value - 1.0) < EPSILON);

  result = sy_evaluate("mod(10.5,3)", &value);
  test_result("mod(10.5,3)=1.5", result == 1 && fabs(value - 1.5) < EPSILON);

  sy_cleanup();
}

static void
test_constants(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("PI", &value);
  test_result("Constant PI", result == 1 && fabs(value - 3.14159265358979) < 1e-10);

  result = sy_evaluate("2*PI", &value);
  test_result("Expression 2*PI", result == 1 && fabs(value - 6.28318530717959) < 1e-10);

  result = sy_evaluate("MM", &value);
  test_result("Constant MM=1e-3", result == 1 && fabs(value - 1.0E-3) < EPSILON);

  result = sy_evaluate("CM", &value);
  test_result("Constant CM=1e-2", result == 1 && fabs(value - 1.0E-2) < EPSILON);

  result = sy_evaluate("IN", &value);
  test_result("Constant IN=2.54e-2", result == 1 && fabs(value - 2.54E-2) < EPSILON);

  result = sy_evaluate("FT", &value);
  test_result("Constant FT=0.3048", result == 1 && fabs(value - 0.3048) < EPSILON);

  result = sy_evaluate("PF", &value);
  test_result("Constant PF=1e-12", result == 1 && fabs(value - 1.0E-12) < EPSILON);

  result = sy_evaluate("NF", &value);
  test_result("Constant NF=1e-9", result == 1 && fabs(value - 1.0E-9) < EPSILON);

  result = sy_evaluate("UF", &value);
  test_result("Constant UF=1e-6", result == 1 && fabs(value - 1.0E-6) < EPSILON);

  result = sy_evaluate("NH", &value);
  test_result("Constant NH=1e-9", result == 1 && fabs(value - 1.0E-9) < EPSILON);

  result = sy_evaluate("UH", &value);
  test_result("Constant UH=1e-6", result == 1 && fabs(value - 1.0E-6) < EPSILON);

  result = sy_evaluate("AWG_0", &value);
  test_result("Constant AWG_0", result == 1 && fabs(value - 0.00412623) < 1e-8);

  result = sy_evaluate("AWG_5", &value);
  test_result("Constant AWG_5", result == 1 && fabs(value - 0.00231013) < 1e-8);

  result = sy_evaluate("AWG_10", &value);
  test_result("Constant AWG_10", result == 1 && fabs(value - 0.00129413) < 1e-8);

  result = sy_evaluate("AWG_15", &value);
  test_result("Constant AWG_15", result == 1 && fabs(value - 0.00072517) < 1e-8);

  result = sy_evaluate("AWG_20", &value);
  test_result("Constant AWG_20", result == 1 && fabs(value - 0.00040640) < 1e-8);

  sy_cleanup();
}

static void
test_complex_expression(void)
{
  double value;
  int result;

  sy_init();
  sy_define("H", "0.5");
  sy_define("fp", "0.0135");

  result = sy_evaluate("H+fp", &value);
  test_result("Complex: H+fp", result == 1 && fabs(value - 0.5135) < EPSILON);

  result = sy_evaluate("H+fp*2", &value);
  test_result("Complex: H+fp*2", result == 1 && fabs(value - 0.527) < EPSILON);

  result = sy_evaluate("(H+fp)*2", &value);
  test_result("Complex: (H+fp)*2", result == 1 && fabs(value - 1.027) < EPSILON);

  sy_cleanup();
}

static void
test_is_expression(void)
{
  int result;

  result = sy_is_expression("3.14");
  test_result("is_expression('3.14')=FALSE", result == 0);

  result = sy_is_expression("1.5E-3");
  test_result("is_expression('1.5E-3')=FALSE", result == 0);

  result = sy_is_expression("H+fp");
  test_result("is_expression('H+fp')=TRUE", result == 1);

  result = sy_is_expression("2*PI");
  test_result("is_expression('2*PI')=TRUE", result == 1);

  result = sy_is_expression("sin(45)");
  test_result("is_expression('sin(45)')=TRUE", result == 1);
}

static void
test_error_handling(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("UNDEFINED_SYMBOL", &value);
  test_result("Undefined symbol returns FALSE", result == 0);

  result = sy_evaluate("10/0", &value);
  test_result("Division by zero returns FALSE", result == 0);

  result = sy_evaluate("sqr(-1)", &value);
  test_result("Domain error sqr(-1) returns NaN", result == 1 && isnan(value));

  result = sy_evaluate("log(-1)", &value);
  test_result("Domain error log(-1) returns NaN", result == 1 && isnan(value));

  result = sy_evaluate("(2+3", &value);
  test_result("Mismatched parentheses returns FALSE", result == 0);

  result = sy_evaluate("2++3", &value);
  test_result("Unary plus 2++3=2+3=5", result == 1 && fabs(value - 5.0) < EPSILON);

  sy_cleanup();
}

static void
test_edge_cases(void)
{
  double value;
  int result;

  sy_init();
  sy_define("X", "5.0");

  result = sy_evaluate("-X", &value);
  test_result("Unary minus -X=-5", result == 1 && fabs(value + 5.0) < EPSILON);

  result = sy_evaluate("-3+2", &value);
  test_result("Unary minus in expr -3+2=-1", result == 1 && fabs(value + 1.0) < EPSILON);

  result = sy_evaluate("2^-1", &value);
  test_result("Negative exponent 2^-1=0.5", result == 1 && fabs(value - 0.5) < EPSILON);

  sy_define("x", "10.0");
  result = sy_evaluate("X", &value);
  test_result("Case insensitivity x vs X", result == 1 && fabs(value - 10.0) < EPSILON);

  result = sy_evaluate("  2  +  3  ", &value);
  test_result("Whitespace handling", result == 1 && fabs(value - 5.0) < EPSILON);

  result = sy_evaluate("sin(atn(1))", &value);
  test_result("Nested functions sin(atn(1))", result == 1 && fabs(value - 0.70710678118) < 1e-6);

  result = sy_evaluate("max(min(10,20),5)", &value);
  test_result("Nested max(min(10,20),5)=10", result == 1 && fabs(value - 10.0) < EPSILON);

  sy_cleanup();
}

static void
test_scientific_notation(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("1.5E-3", &value);
  test_result("Scientific 1.5E-3", result == 1 && fabs(value - 1.5E-3) < EPSILON);

  result = sy_evaluate("1.5e-3", &value);
  test_result("Scientific 1.5e-3", result == 1 && fabs(value - 1.5e-3) < EPSILON);

  result = sy_evaluate("2.54E+2", &value);
  test_result("Scientific 2.54E+2", result == 1 && fabs(value - 254.0) < EPSILON);

  result = sy_evaluate("1E6", &value);
  test_result("Scientific 1E6", result == 1 && fabs(value - 1.0E6) < EPSILON);

  result = sy_evaluate("3.14E0", &value);
  test_result("Scientific 3.14E0", result == 1 && fabs(value - 3.14) < EPSILON);

  sy_cleanup();
}

static void
test_associativity(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("10-5-2", &value);
  test_result("Left-assoc subtraction 10-5-2=3", result == 1 && fabs(value - 3.0) < EPSILON);

  result = sy_evaluate("20/4/2", &value);
  test_result("Left-assoc division 20/4/2=2.5", result == 1 && fabs(value - 2.5) < EPSILON);

  result = sy_evaluate("2^2^3", &value);
  test_result("Right-assoc exponent 2^2^3=256", result == 1 && fabs(value - 256.0) < EPSILON);

  result = sy_evaluate("100/10*2", &value);
  test_result("Left-assoc mixed 100/10*2=20", result == 1 && fabs(value - 20.0) < EPSILON);

  sy_cleanup();
}

static void
test_multi_symbol_expressions(void)
{
  double value;
  int result;

  sy_init();
  sy_define("A", "10.0");
  sy_define("B", "5.0");
  sy_define("C", "2.0");

  result = sy_evaluate("A+B+C", &value);
  test_result("Multi-symbol A+B+C=17", result == 1 && fabs(value - 17.0) < EPSILON);

  result = sy_evaluate("A*B/C", &value);
  test_result("Multi-symbol A*B/C=25", result == 1 && fabs(value - 25.0) < EPSILON);

  result = sy_evaluate("(A+B)*C", &value);
  test_result("Multi-symbol (A+B)*C=30", result == 1 && fabs(value - 30.0) < EPSILON);

  result = sy_evaluate("A^C+B", &value);
  test_result("Multi-symbol A^C+B=105", result == 1 && fabs(value - 105.0) < EPSILON);

  sy_cleanup();
}

static void
test_function_arity_validation(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("sin()", &value);
  test_result("Function arity: sin() missing arg returns FALSE", result == 0);

  result = sy_evaluate("max(5)", &value);
  test_result("Function arity: max(5) missing arg returns FALSE", result == 0);

  result = sy_evaluate("mod(10)", &value);
  test_result("Function arity: mod(10) missing arg returns FALSE", result == 0);

  result = sy_evaluate("min()", &value);
  test_result("Function arity: min() missing args returns FALSE", result == 0);

  sy_cleanup();
}

static void
test_all_awg_constants(void)
{
  double value;
  int result;

  sy_init();

  for( int i = 0; i <= 20; i++ )
  {
    char awg_name[16];
    char test_name[64];
    snprintf(awg_name, sizeof(awg_name), "AWG_%d", i);
    snprintf(test_name, sizeof(test_name), "AWG_%d exists and positive", i);

    result = sy_evaluate(awg_name, &value);
    test_result(test_name, result == 1 && value > 0 && value < 0.01);
  }

  sy_cleanup();
}

static void
test_symbol_redefinition(void)
{
  double value;
  int result;

  sy_init();

  sy_define("X", "10.0");
  result = sy_evaluate("X", &value);
  test_result("Initial X=10", result == 1 && fabs(value - 10.0) < EPSILON);

  sy_define("X", "20.0");
  result = sy_evaluate("X", &value);
  test_result("Redefined X=20", result == 1 && fabs(value - 20.0) < EPSILON);

  sy_cleanup();
}

static void
test_empty_and_whitespace(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("", &value);
  test_result("Empty expression returns FALSE", result == 0);

  result = sy_evaluate("   ", &value);
  test_result("Whitespace-only expression returns FALSE", result == 0);

  result = sy_evaluate("\t\n", &value);
  test_result("Tabs and newlines returns FALSE", result == 0);

  sy_cleanup();
}

static void
test_extreme_values(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("1E308", &value);
  test_result("Very large number 1E308", result == 1 && value > 1E307);

  result = sy_evaluate("1E-308", &value);
  test_result("Very small number 1E-308", result == 1 && value < 1E-307 && value > 0);

  result = sy_evaluate("2^10", &value);
  test_result("Power result 2^10=1024", result == 1 && fabs(value - 1024.0) < EPSILON);

  result = sy_evaluate("10^-6", &value);
  test_result("Negative power 10^-6", result == 1 && fabs(value - 1.0E-6) < 1E-9);

  sy_cleanup();
}

static void
test_invalid_function_names(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("sine(30)", &value);
  test_result("Invalid function 'sine' returns FALSE", result == 0);

  result = sy_evaluate("cosine(60)", &value);
  test_result("Invalid function 'cosine' returns FALSE", result == 0);

  result = sy_evaluate("sqrt(4)", &value);
  test_result("Invalid function 'sqrt' (should be 'sqr') returns FALSE", result == 0);

  sy_cleanup();
}

static void
test_complex_nested_expressions(void)
{
  double value;
  int result;

  sy_init();
  sy_define("H", "0.5");
  sy_define("fp", "0.0135");
  sy_define("sll", "0.068");

  result = sy_evaluate("H+fp+sll*3", &value);
  test_result("Complex nested: H+fp+sll*3", result == 1 && fabs(value - 0.7175) < 1e-6);

  result = sy_evaluate("(H+fp)*(sll+0.1)", &value);
  test_result("Complex nested: (H+fp)*(sll+0.1)", result == 1 && fabs(value - 0.0862680) < 1e-6);

  result = sy_evaluate("sin(atn(H/fp))", &value);
  test_result("Nested trig with symbols", result == 1 && fabs(value - 0.99963570) < 1e-6);

  sy_cleanup();
}

static void
test_operator_combinations(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("5*-3", &value);
  test_result("Multiplication with unary: 5*-3=-15", result == 1 && fabs(value + 15.0) < 1e-6);

  result = sy_evaluate("10/-2", &value);
  test_result("Division with unary: 10/-2=-5", result == 1 && fabs(value + 5.0) < 1e-6);

  result = sy_evaluate("2^-3", &value);
  test_result("Power with unary: 2^-3=0.125", result == 1 && fabs(value - 0.125) < 1e-6);

  result = sy_evaluate("-2^2", &value);
  test_result("Unary before power: -2^2=-4", result == 1 && fabs(value + 4.0) < 1e-6);

  sy_cleanup();
}

static void
test_exhaustive_precedence_ordering(void)
{
  double value;
  int result;

  sy_init();

  result = sy_evaluate("1+2*3^4", &value);
  test_result("Precedence chain 1+2*3^4=1+2*81=163", result == 1 && fabs(value - 163.0) < EPSILON);

  result = sy_evaluate("2*3^2+1", &value);
  test_result("Precedence chain 2*3^2+1=2*9+1=19", result == 1 && fabs(value - 19.0) < EPSILON);

  result = sy_evaluate("10-2*3", &value);
  test_result("Mult before sub: 10-2*3=10-6=4", result == 1 && fabs(value - 4.0) < EPSILON);

  result = sy_evaluate("10/2+3", &value);
  test_result("Div before add: 10/2+3=5+3=8", result == 1 && fabs(value - 8.0) < EPSILON);

  result = sy_evaluate("2+3^2*4", &value);
  test_result("Power before mult: 2+3^2*4=2+9*4=38", result == 1 && fabs(value - 38.0) < EPSILON);

  result = sy_evaluate("2*3+4*5", &value);
  test_result("Equal precedence left: 2*3+4*5=6+20=26", result == 1 && fabs(value - 26.0) < EPSILON);

  result = sy_evaluate("3*2^3", &value);
  test_result("Power before mult: 3*2^3=3*8=24", result == 1 && fabs(value - 24.0) < EPSILON);

  result = sy_evaluate("2^3*3", &value);
  test_result("Power before mult: 2^3*3=8*3=24", result == 1 && fabs(value - 24.0) < EPSILON);

  result = sy_evaluate("4^2/2", &value);
  test_result("Power before div: 4^2/2=16/2=8", result == 1 && fabs(value - 8.0) < EPSILON);

  result = sy_evaluate("16/2^2", &value);
  test_result("Power before div: 16/2^2=16/4=4", result == 1 && fabs(value - 4.0) < EPSILON);

  result = sy_evaluate("10-5+3", &value);
  test_result("Left-assoc add/sub: 10-5+3=5+3=8", result == 1 && fabs(value - 8.0) < EPSILON);

  result = sy_evaluate("20/4*2", &value);
  test_result("Left-assoc mult/div: 20/4*2=5*2=10", result == 1 && fabs(value - 10.0) < EPSILON);

  result = sy_evaluate("20*4/2", &value);
  test_result("Left-assoc mult/div: 20*4/2=80/2=40", result == 1 && fabs(value - 40.0) < EPSILON);

  result = sy_evaluate("3^2^2", &value);
  test_result("Right-assoc power: 3^2^2=3^4=81", result == 1 && fabs(value - 81.0) < EPSILON);

  result = sy_evaluate("-3^2", &value);
  test_result("Unary before power: -3^2=-9", result == 1 && fabs(value + 9.0) < EPSILON);

  result = sy_evaluate("(-3)^2", &value);
  test_result("Paren override: (-3)^2=9", result == 1 && fabs(value - 9.0) < EPSILON);

  result = sy_evaluate("2+-3", &value);
  test_result("Add then unary: 2+-3=2+(-3)=-1", result == 1 && fabs(value + 1.0) < EPSILON);

  result = sy_evaluate("2*+3", &value);
  test_result("Mult then unary plus: 2*+3=2*3=6", result == 1 && fabs(value - 6.0) < EPSILON);

  result = sy_evaluate("2^+3", &value);
  test_result("Power then unary plus: 2^+3=2^3=8", result == 1 && fabs(value - 8.0) < EPSILON);

  result = sy_evaluate("-(2+3)", &value);
  test_result("Unary on parentheses: -(2+3)=-5", result == 1 && fabs(value + 5.0) < EPSILON);

  result = sy_evaluate("-(2*3)", &value);
  test_result("Unary on parentheses: -(2*3)=-6", result == 1 && fabs(value + 6.0) < EPSILON);

  result = sy_evaluate("-(2^3)", &value);
  test_result("Unary on parentheses: -(2^3)=-8", result == 1 && fabs(value + 8.0) < EPSILON);

  result = sy_evaluate("2^(3+1)", &value);
  test_result("Parentheses in exponent: 2^(3+1)=16", result == 1 && fabs(value - 16.0) < EPSILON);

  result = sy_evaluate("(2+3)^2", &value);
  test_result("Parentheses before power: (2+3)^2=25", result == 1 && fabs(value - 25.0) < EPSILON);

  result = sy_evaluate("2^3+2^2", &value);
  test_result("Multiple powers: 2^3+2^2=8+4=12", result == 1 && fabs(value - 12.0) < EPSILON);

  result = sy_evaluate("2+3*4-5", &value);
  test_result("Mixed precedence: 2+3*4-5=2+12-5=9", result == 1 && fabs(value - 9.0) < EPSILON);

  result = sy_evaluate("2*3+4/2", &value);
  test_result("Mixed mult/div/add: 2*3+4/2=6+2=8", result == 1 && fabs(value - 8.0) < EPSILON);

  result = sy_evaluate("10/2-3", &value);
  test_result("Div then sub: 10/2-3=5-3=2", result == 1 && fabs(value - 2.0) < EPSILON);

  result = sy_evaluate("10-2/2", &value);
  test_result("Div before sub: 10-2/2=10-1=9", result == 1 && fabs(value - 9.0) < EPSILON);

  result = sy_evaluate("-2*3", &value);
  test_result("Unary then mult: -2*3=-6", result == 1 && fabs(value + 6.0) < EPSILON);

  result = sy_evaluate("-2-3", &value);
  test_result("Unary then sub: -2-3=-5", result == 1 && fabs(value + 5.0) < EPSILON);

  result = sy_evaluate("-2+3", &value);
  test_result("Unary then add: -2+3=1", result == 1 && fabs(value - 1.0) < EPSILON);

  result = sy_evaluate("--2", &value);
  test_result("Double unary: --2=2", result == 1 && fabs(value - 2.0) < EPSILON);

  result = sy_evaluate("---2", &value);
  test_result("Triple unary: ---2=-2", result == 1 && fabs(value + 2.0) < EPSILON);

  sy_cleanup();
}

int
main(void)
{
  printf("Running SY expression evaluator tests...\n\n");

  test_init_cleanup();
  test_basic_symbols();
  test_arithmetic_operators();
  test_operator_precedence();
  test_trig_functions();
  test_math_functions();
  test_constants();
  test_complex_expression();
  test_is_expression();
  test_error_handling();
  test_edge_cases();
  test_scientific_notation();
  test_associativity();
  test_multi_symbol_expressions();
  test_function_arity_validation();
  test_all_awg_constants();
  test_symbol_redefinition();
  test_exhaustive_precedence_ordering();
  test_empty_and_whitespace();
  test_extreme_values();
  test_invalid_function_names();
  test_complex_nested_expressions();
  test_operator_combinations();

  printf("\n========================================\n");
  printf("Tests run: %d\n", tests_run);
  printf("Tests passed: %d\n", tests_passed);
  printf("Tests failed: %d\n", tests_run - tests_passed);
  printf("========================================\n");

  if( tests_passed == tests_run )
  {
    printf("ALL TESTS PASSED\n");
    return 0;
  }
  else
  {
    printf("SOME TESTS FAILED\n");
    return 1;
  }
}
