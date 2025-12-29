/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include "sy_expr.h"
#include "shared.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)

/* Token types for expression parsing */
typedef enum
{
  SY_TOKEN_NUMBER = 0,
  SY_TOKEN_SYMBOL,
  SY_TOKEN_OPERATOR,
  SY_TOKEN_FUNCTION,
  SY_TOKEN_LPAREN,
  SY_TOKEN_RPAREN,
  SY_TOKEN_COMMA,
  SY_TOKEN_END
} sy_token_type_t;

/* Token structure */
typedef struct
{
  sy_token_type_t type;
  union
  {
    gdouble number;
    gchar name[32];
    gchar op;
  } value;
  gint precedence;
} sy_token_t;

/* Function dispatch entry */
typedef struct
{
  const gchar *name;
  gint arity;
  gdouble (*func1)(gdouble);
  gdouble (*func2)(gdouble, gdouble);
} sy_function_t;

/* Predefined constant entry */
typedef struct
{
  const gchar *name;
  gdouble value;
} sy_constant_t;

/* Symbol table stores user-defined variables */
static GHashTable *symbol_table = NULL;

/* Mathematical function implementations */
static gdouble sy_func_sin(gdouble a) { return sin(a * DEG2RAD); }
static gdouble sy_func_cos(gdouble a) { return cos(a * DEG2RAD); }
static gdouble sy_func_tan(gdouble a) { return tan(a * DEG2RAD); }
static gdouble sy_func_atn(gdouble a) { return atan(a) * RAD2DEG; }
static gdouble sy_func_sqr(gdouble a) { return sqrt(a); }
static gdouble sy_func_exp(gdouble a) { return exp(a); }
static gdouble sy_func_log(gdouble a) { return log(a); }
static gdouble sy_func_log10(gdouble a) { return log10(a); }
static gdouble sy_func_abs(gdouble a) { return fabs(a); }
static gdouble sy_func_sgn(gdouble a) { return (a > 0.0) - (a < 0.0); }
static gdouble sy_func_int(gdouble a) { return round(a); }
static gdouble sy_func_fix(gdouble a) { return trunc(a); }
static gdouble sy_func_mod(gdouble a, gdouble b) { return fmod(a, b); }
static gdouble sy_func_max(gdouble a, gdouble b) { return fmax(a, b); }
static gdouble sy_func_min(gdouble a, gdouble b) { return fmin(a, b); }

/* Function dispatch table at file top after includes */
static const sy_function_t sy_functions[] =
{
  { "SIN",   1, sy_func_sin,   NULL },
  { "COS",   1, sy_func_cos,   NULL },
  { "TAN",   1, sy_func_tan,   NULL },
  { "ATN",   1, sy_func_atn,   NULL },
  { "SQR",   1, sy_func_sqr,   NULL },
  { "EXP",   1, sy_func_exp,   NULL },
  { "LOG",   1, sy_func_log,   NULL },
  { "LOG10", 1, sy_func_log10, NULL },
  { "ABS",   1, sy_func_abs,   NULL },
  { "SGN",   1, sy_func_sgn,   NULL },
  { "INT",   1, sy_func_int,   NULL },
  { "FIX",   1, sy_func_fix,   NULL },
  { "MOD",   2, NULL, sy_func_mod },
  { "MAX",   2, NULL, sy_func_max },
  { "MIN",   2, NULL, sy_func_min },
  { NULL,    0, NULL, NULL }
};

/* Predefined constants table at file top */
static const sy_constant_t sy_constants[] =
{
  { "PI",     3.14159265358979323846 },
  { "MM",     1.0E-3 },
  { "CM",     1.0E-2 },
  { "IN",     2.54E-2 },
  { "FT",     0.3048 },
  { "PF",     1.0E-12 },
  { "NF",     1.0E-9 },
  { "UF",     1.0E-6 },
  { "NH",     1.0E-9 },
  { "UH",     1.0E-6 },
  { "AWG_0",  0.00412623 },
  { "AWG_1",  0.00367411 },
  { "AWG_2",  0.00327152 },
  { "AWG_3",  0.00291338 },
  { "AWG_4",  0.00259461 },
  { "AWG_5",  0.00231013 },
  { "AWG_6",  0.00205740 },
  { "AWG_7",  0.00183261 },
  { "AWG_8",  0.00163195 },
  { "AWG_9",  0.00145288 },
  { "AWG_10", 0.00129413 },
  { "AWG_11", 0.00115189 },
  { "AWG_12", 0.00102616 },
  { "AWG_13", 0.00091440 },
  { "AWG_14", 0.00081407 },
  { "AWG_15", 0.00072517 },
  { "AWG_16", 0.00064516 },
  { "AWG_17", 0.00057531 },
  { "AWG_18", 0.00051308 },
  { "AWG_19", 0.00045720 },
  { "AWG_20", 0.00040640 },
  { NULL,     0.0 }
};

/* Operator table - single source of truth for operator properties
 * Defines precedence, associativity, and evaluation for all operators
 * Precedence hierarchy: add/sub (10), mult/div (20), unary (25), power (30)
 */
typedef struct
{
  gchar op;
  gint precedence;
  gboolean right_assoc;
  gdouble (*eval)(gdouble, gdouble);
} sy_operator_t;

static gdouble sy_op_add(gdouble a, gdouble b) { return a + b; }
static gdouble sy_op_sub(gdouble a, gdouble b) { return a - b; }
static gdouble sy_op_mul(gdouble a, gdouble b) { return a * b; }
static gdouble sy_op_div(gdouble a, gdouble b) { return a / b; }
static gdouble sy_op_pow(gdouble a, gdouble b) { return pow(a, b); }

static const sy_operator_t sy_operators[] =
{
  { '+', 10, FALSE, sy_op_add },
  { '-', 10, FALSE, sy_op_sub },
  { '*', 20, FALSE, sy_op_mul },
  { '/', 20, FALSE, sy_op_div },
  { '^', 30, TRUE,  sy_op_pow },
  { '\0', 0, FALSE, NULL }
};

/* Lookup operator in table */
static const sy_operator_t *
sy_lookup_operator(gchar op)
{
  gint i;

  for( i = 0; sy_operators[i].op != '\0'; i++ )
  {
    if( sy_operators[i].op == op )
      return &sy_operators[i];
  }

  return NULL;
}

/* Get operator precedence from table */
static gint
sy_get_precedence(gchar op)
{
  const sy_operator_t *entry;

  entry = sy_lookup_operator(op);
  if( entry != NULL )
    return entry->precedence;
  else
    return 0;
}

/* Check if operator is right-associative from table */
static gboolean
sy_is_right_assoc(gchar op)
{
  const sy_operator_t *entry;

  entry = sy_lookup_operator(op);
  if( entry != NULL )
    return entry->right_assoc;
  else
    return FALSE;
}

/* Character class helper functions for expression parsing and validation
 * These replace open-coded character checks with named, readable functions
 */

/* Operator characters in expressions */
static inline gboolean is_sy_operator(gchar c) {
  return strchr("+-*/^", c) != NULL;
}

/* Characters that start a number */
static inline gboolean is_sy_number_start(gchar c) {
  return g_ascii_isdigit(c) || c == '.';
}

/* Characters that start an identifier */
static inline gboolean is_sy_ident_start(gchar c) {
  return g_ascii_isalpha(c) || c == '_';
}

/* Characters within an identifier */
static inline gboolean is_sy_ident_char(gchar c) {
  return g_ascii_isalnum(c) || c == '_';
}

/* Scientific notation exponent marker */
static inline gboolean is_sy_exponent_marker(gchar c) {
  return c == 'E' || c == 'e';
}

/* Whitespace for field separation */
static inline gboolean is_sy_whitespace(gchar c) {
  return c == ' ' || c == '\t';
}

/* Expression-only operators (not valid in plain numbers) */
static inline gboolean is_sy_expr_only_char(gchar c) {
  return strchr("*/^()", c) != NULL;
}

/* Sign characters (context-dependent: unary or binary) */
static inline gboolean is_sy_sign(gchar c) {
  return c == '+' || c == '-';
}

/* Normalize name to uppercase for case-insensitive lookup
 * Rationale: 4nec2 specification requires case-insensitive symbol and function names
 * for compatibility with existing antenna modeling files
 */
static void
sy_normalize_name(const gchar *name, gchar *upper_name, gsize size)
{
  gchar *temp;

  temp = g_ascii_strup(name, -1);
  g_strlcpy(upper_name, temp, size);
  g_free(temp);
}

/* Lookup constant by name (case-insensitive) */
static gboolean
sy_lookup_constant(const gchar *name, gdouble *value)
{
  gchar upper_name[32];
  gint i;

  sy_normalize_name(name, upper_name, sizeof(upper_name));

  for( i = 0; sy_constants[i].name != NULL; i++ )
  {
    if( strcmp(upper_name, sy_constants[i].name) == 0 )
    {
      *value = sy_constants[i].value;
      return TRUE;
    }
  }

  return FALSE;
}

/* Lookup function by name (case-insensitive) */
static const sy_function_t *
sy_lookup_function(const gchar *name)
{
  gchar upper_name[32];
  gint j;

  sy_normalize_name(name, upper_name, sizeof(upper_name));

  for( j = 0; sy_functions[j].name != NULL; j++ )
  {
    if( strcmp(upper_name, sy_functions[j].name) == 0 )
      return &sy_functions[j];
  }

  return NULL;
}

/* Tokenize expression string into array of tokens
 * Recognizes: numbers, identifiers, operators, parentheses, commas
 * Numbers support scientific notation (e.g., 1.5E-3)
 * Uses else-if chain for mutual exclusivity and compiler optimization
 */
static GArray *
sy_tokenize(const gchar *expr)
{
  GArray *tokens;
  sy_token_t token;
  const gchar *p;
  gchar *endptr;
  gchar buf[32];
  gint buf_idx;

  tokens = g_array_new(FALSE, FALSE, sizeof(sy_token_t));
  p = expr;

  while( *p != '\0' )
  {
    if( is_sy_whitespace(*p) )
    {
      p++;
    }
    else if( is_sy_number_start(*p) )
    {
      token.type = SY_TOKEN_NUMBER;
      token.value.number = strtod(p, &endptr);
      token.precedence = 0;
      g_array_append_val(tokens, token);
      p = endptr;
    }
    else if( is_sy_ident_start(*p) )
    {
      buf_idx = 0;
      while( is_sy_ident_char(*p) && buf_idx < 31 )
      {
        buf[buf_idx++] = *p++;
      }
      buf[buf_idx] = '\0';

      if( sy_lookup_function(buf) != NULL )
      {
        token.type = SY_TOKEN_FUNCTION;
        g_strlcpy(token.value.name, buf, sizeof(token.value.name));
        token.precedence = 0;
        g_array_append_val(tokens, token);
      }
      else
      {
        token.type = SY_TOKEN_SYMBOL;
        g_strlcpy(token.value.name, buf, sizeof(token.value.name));
        token.precedence = 0;
        g_array_append_val(tokens, token);
      }
    }
    else if( is_sy_operator(*p) )
    {
      /* Operator handling with unary minus/plus transformation
       *
       * Unary minus transformed to (-1) * operand with context-dependent precedence
       * Context determines precedence:
       *   - At start/after '('/after ',': 25 (between mult and power) for -2^2 = -(2^2) = -4
       *   - After binary operator: 35 (above power) for 2^-3 = 2^((-1)*3) = 0.125
       * Unary context: at start, after operators, after '(', after ','
       */
      gboolean is_unary = FALSE;
      gboolean after_operator = FALSE;
      sy_token_t *prev;

      if( tokens->len == 0 )
      {
        is_unary = TRUE;
      }
      else
      {
        prev = &g_array_index(tokens, sy_token_t, tokens->len - 1);
        if( prev->type == SY_TOKEN_OPERATOR ||
            prev->type == SY_TOKEN_LPAREN ||
            prev->type == SY_TOKEN_COMMA )
        {
          is_unary = TRUE;
          if( prev->type == SY_TOKEN_OPERATOR )
            after_operator = TRUE;
        }
      }

      if( is_unary && is_sy_sign(*p) )
      {
        if( *p == '-' )
        {
          gint unary_prec;

          if( after_operator )
            unary_prec = 35;
          else
            unary_prec = 25;

          token.type = SY_TOKEN_NUMBER;
          token.value.number = -1.0;
          token.precedence = 0;
          g_array_append_val(tokens, token);

          token.type = SY_TOKEN_OPERATOR;
          token.value.op = '*';
          token.precedence = unary_prec;
          g_array_append_val(tokens, token);
        }

        p++;
      }
      else
      {
        token.type = SY_TOKEN_OPERATOR;
        token.value.op = *p;
        token.precedence = sy_get_precedence(*p);
        g_array_append_val(tokens, token);
        p++;
      }
    }
    else if( *p == '(' )
    {
      token.type = SY_TOKEN_LPAREN;
      token.precedence = 0;
      g_array_append_val(tokens, token);
      p++;
    }
    else if( *p == ')' )
    {
      token.type = SY_TOKEN_RPAREN;
      token.precedence = 0;
      g_array_append_val(tokens, token);
      p++;
    }
    else if( *p == ',' )
    {
      token.type = SY_TOKEN_COMMA;
      token.precedence = 0;
      g_array_append_val(tokens, token);
      p++;
    }
    else
    {
      Stop(_("Expression syntax error: unexpected character"), ERR_OK);
      g_array_free(tokens, TRUE);
      return NULL;
    }
  }

  token.type = SY_TOKEN_END;
  token.precedence = 0;
  g_array_append_val(tokens, token);

  return tokens;
}

/* Convert infix token array to RPN using Shunting-yard algorithm
 * Dijkstra's algorithm for parsing mathematical expressions
 * Handles operator precedence and right-associativity of exponentiation
 * Allocates copies of tokens for output queue since input array will be freed
 */
static GQueue *
sy_infix_to_rpn(GArray *tokens)
{
  GQueue *output;
  GQueue *operators;
  sy_token_t *token;
  sy_token_t *op_token;
  sy_token_t *token_copy;
  guint i;

  output = g_queue_new();
  operators = g_queue_new();

  for( i = 0; i < tokens->len; i++ )
  {
    token = &g_array_index(tokens, sy_token_t, i);

    if( token->type == SY_TOKEN_NUMBER || token->type == SY_TOKEN_SYMBOL )
    {
      token_copy = g_new(sy_token_t, 1);
      *token_copy = *token;
      g_queue_push_tail(output, token_copy);
    }
    else if( token->type == SY_TOKEN_FUNCTION )
    {
      token_copy = g_new(sy_token_t, 1);
      *token_copy = *token;
      g_queue_push_head(operators, token_copy);
    }
    else if( token->type == SY_TOKEN_COMMA )
    {
      while( !g_queue_is_empty(operators) )
      {
        op_token = (sy_token_t *)g_queue_peek_head(operators);
        if( op_token->type == SY_TOKEN_LPAREN )
          break;
        else
        {
          g_queue_pop_head(operators);
          g_queue_push_tail(output, op_token);
        }
      }
    }
    else if( token->type == SY_TOKEN_OPERATOR )
    {
      while( !g_queue_is_empty(operators) )
      {
        op_token = (sy_token_t *)g_queue_peek_head(operators);
        if( op_token->type != SY_TOKEN_OPERATOR )
          break;
        else
        {
          if( sy_is_right_assoc(token->value.op) )
          {
            if( op_token->precedence > token->precedence )
            {
              g_queue_pop_head(operators);
              g_queue_push_tail(output, op_token);
            }
            else
              break;
          }
          else
          {
            if( op_token->precedence >= token->precedence )
            {
              g_queue_pop_head(operators);
              g_queue_push_tail(output, op_token);
            }
            else
              break;
          }
        }
      }

      token_copy = g_new(sy_token_t, 1);
      *token_copy = *token;
      g_queue_push_head(operators, token_copy);
    }
    else if( token->type == SY_TOKEN_LPAREN )
    {
      token_copy = g_new(sy_token_t, 1);
      *token_copy = *token;
      g_queue_push_head(operators, token_copy);
    }
    else if( token->type == SY_TOKEN_RPAREN )
    {
      while( !g_queue_is_empty(operators) )
      {
        op_token = (sy_token_t *)g_queue_peek_head(operators);
        if( op_token->type == SY_TOKEN_LPAREN )
        {
          g_queue_pop_head(operators);
          g_free(op_token);
          break;
        }
        else
        {
          g_queue_pop_head(operators);
          g_queue_push_tail(output, op_token);
        }
      }

      if( !g_queue_is_empty(operators) )
      {
        op_token = (sy_token_t *)g_queue_peek_head(operators);
        if( op_token->type == SY_TOKEN_FUNCTION )
        {
          g_queue_pop_head(operators);
          g_queue_push_tail(output, op_token);
        }
      }
    }
    else if( token->type == SY_TOKEN_END )
    {
      break;
    }
    else
    {
      Stop(_("Expression parsing error: unexpected token"), ERR_OK);
      g_queue_free(output);
      g_queue_free(operators);
      return NULL;
    }
  }

  while( !g_queue_is_empty(operators) )
  {
    op_token = (sy_token_t *)g_queue_pop_head(operators);
    if( op_token->type == SY_TOKEN_LPAREN || op_token->type == SY_TOKEN_RPAREN )
    {
      Stop(_("Expression error: mismatched parentheses"), ERR_OK);
      g_free(op_token);
      g_queue_free_full(output, g_free);
      g_queue_free_full(operators, g_free);
      return NULL;
    }
    else
      g_queue_push_tail(output, op_token);
  }

  g_queue_free(operators);
  return output;
}

/* Evaluate RPN expression from queue
 * Uses value stack to process numbers, operators, and functions
 * Looks up symbols from symbol table and constants from constant table
 */
static gboolean
sy_evaluate_rpn(GQueue *rpn, gdouble *result)
{
  GQueue *stack;
  sy_token_t *token;
  gdouble *val1;
  gdouble *val2;
  gdouble *res;
  gdouble tmp;
  const sy_function_t *func;
  gchar upper_name[32];

  stack = g_queue_new();

  while( !g_queue_is_empty(rpn) )
  {
    token = (sy_token_t *)g_queue_pop_head(rpn);

    if( token->type == SY_TOKEN_NUMBER )
    {
      pr_debug("NUMBER: %.9f\n", token->value.number);
      res = g_new(gdouble, 1);
      *res = token->value.number;
      g_queue_push_head(stack, res);
      g_free(token);
      continue;
    }

    if( token->type == SY_TOKEN_SYMBOL )
    {
      sy_normalize_name(token->value.name, upper_name, sizeof(upper_name));

      if( sy_lookup_constant(upper_name, &tmp) )
      {
        pr_debug("SYMBOL '%s' from CONST = %.9f\n", upper_name, tmp);
        res = g_new(gdouble, 1);
        *res = tmp;
        g_queue_push_head(stack, res);
      }
      else
      {
        if( symbol_table == NULL )
        {
          Stop(_("Symbol table not initialized"), ERR_OK);
          g_free(token);
          g_queue_free_full(stack, g_free);
          return FALSE;
        }
        else
        {
          val1 = (gdouble *)g_hash_table_lookup(symbol_table, upper_name);
          if( val1 == NULL )
          {
            gchar err_msg[128];
            snprintf(err_msg, sizeof(err_msg), _("Undefined symbol: %s"), upper_name);
            Stop(err_msg, ERR_OK);
            g_free(token);
            g_queue_free_full(stack, g_free);
            return FALSE;
          }
          else
          {
            pr_debug("SYMBOL '%s' from TABLE = %.9f\n", upper_name, *val1);
            res = g_new(gdouble, 1);
            *res = *val1;
            g_queue_push_head(stack, res);
          }
        }
      }

      g_free(token);
      continue;
    }

    if( token->type == SY_TOKEN_OPERATOR )
    {
      const sy_operator_t *op_entry;

      if( g_queue_is_empty(stack) )
      {
        Stop(_("Expression error: missing operand"), ERR_OK);
        g_free(token);
        g_queue_free_full(stack, g_free);
        return FALSE;
      }

      val2 = (gdouble *)g_queue_pop_head(stack);

      if( g_queue_is_empty(stack) )
      {
        Stop(_("Expression error: missing operand"), ERR_OK);
        g_free(token);
        g_free(val2);
        g_queue_free_full(stack, g_free);
        return FALSE;
      }

      val1 = (gdouble *)g_queue_pop_head(stack);

      op_entry = sy_lookup_operator(token->value.op);
      if( op_entry == NULL )
      {
        Stop(_("Unknown operator"), ERR_OK);
        g_free(token);
        g_free(val1);
        g_free(val2);
        g_queue_free_full(stack, g_free);
        return FALSE;
      }

      res = g_new(gdouble, 1);

      if( token->value.op == '/' && *val2 == 0.0 )
      {
        Stop(_("Division by zero"), ERR_OK);
        g_free(token);
        g_free(val1);
        g_free(val2);
        g_free(res);
        g_queue_free_full(stack, g_free);
        return FALSE;
      }

      *res = op_entry->eval(*val1, *val2);
      pr_debug("OP '%c': %.9f %c %.9f = %.9f\n",
               token->value.op, *val1, token->value.op, *val2, *res);
      g_free(val1);
      g_free(val2);
      g_queue_push_head(stack, res);

      g_free(token);
      continue;
    }
    else if( token->type == SY_TOKEN_FUNCTION )
    {
      func = sy_lookup_function(token->value.name);
      if( func == NULL )
      {
        gchar err_msg[128];
        snprintf(err_msg, sizeof(err_msg), _("Unknown function: %s"), token->value.name);
        Stop(err_msg, ERR_OK);
        g_free(token);
        g_queue_free_full(stack, g_free);
        return FALSE;
      }
      else
      {
        if( func->arity == 1 )
        {
          if( g_queue_is_empty(stack) )
          {
            Stop(_("Function error: missing argument"), ERR_OK);
            g_free(token);
            g_queue_free_full(stack, g_free);
            return FALSE;
          }
          else
          {
            val1 = (gdouble *)g_queue_pop_head(stack);
            res = g_new(gdouble, 1);
            *res = func->func1(*val1);
            pr_debug("FUNC '%s(1)': %.9f = %.9f\n", token->value.name, *val1, *res);
            g_free(val1);
            g_queue_push_head(stack, res);
          }
        }
        else if( func->arity == 2 )
        {
          if( g_queue_is_empty(stack) )
          {
            Stop(_("Function error: missing argument"), ERR_OK);
            g_free(token);
            g_queue_free_full(stack, g_free);
            return FALSE;
          }
          else
            val2 = (gdouble *)g_queue_pop_head(stack);

          if( g_queue_is_empty(stack) )
          {
            Stop(_("Function error: missing argument"), ERR_OK);
            g_free(token);
            g_free(val2);
            g_queue_free_full(stack, g_free);
            return FALSE;
          }
          else
            val1 = (gdouble *)g_queue_pop_head(stack);

          res = g_new(gdouble, 1);
          *res = func->func2(*val1, *val2);
          pr_debug("FUNC '%s(2)': %.9f, %.9f = %.9f\n",
                   token->value.name, *val1, *val2, *res);
          g_free(val1);
          g_free(val2);
          g_queue_push_head(stack, res);
        }
        else
        {
          Stop(_("Function error: invalid arity"), ERR_OK);
          g_free(token);
          g_queue_free_full(stack, g_free);
          return FALSE;
        }
      }

      g_free(token);
      continue;
    }

    /* Totality enforcement: handle unexpected token types in RPN queue
     * Parentheses and commas should never appear in RPN output
     */
    Stop(_("Expression error: unexpected token type in RPN"), ERR_OK);
    g_free(token);
    g_queue_free_full(stack, g_free);
    return FALSE;
  }

  if( g_queue_get_length(stack) != 1 )
  {
    Stop(_("Expression error: invalid result"), ERR_OK);
    g_queue_free_full(stack, g_free);
    return FALSE;
  }
  else
  {
    val1 = (gdouble *)g_queue_pop_head(stack);
    *result = *val1;
    g_free(val1);
  }

  g_queue_free(stack);
  return TRUE;
}

/* Public interface implementations */

gboolean
sy_init(void)
{
  if( symbol_table != NULL )
    sy_cleanup();

  symbol_table = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    g_free,
    g_free);

  if( symbol_table == NULL )
    return FALSE;
  else
    return TRUE;
}

void
sy_cleanup(void)
{
  if( symbol_table != NULL )
  {
    g_hash_table_destroy(symbol_table);
    symbol_table = NULL;
  }
}

gboolean
sy_define(const gchar *name, gdouble value)
{
  gchar *upper_name;
  gchar temp_name[64];
  gdouble *val_ptr;

  if( symbol_table == NULL )
  {
    Stop(_("Symbol table not initialized"), ERR_OK);
    return FALSE;
  }
  else
  {
    sy_normalize_name(name, temp_name, sizeof(temp_name));
    upper_name = g_strdup(temp_name);

    val_ptr = g_new(gdouble, 1);
    *val_ptr = value;

    g_hash_table_insert(symbol_table, upper_name, val_ptr);
    return TRUE;
  }
}

gboolean
sy_evaluate(const gchar *expr, gdouble *result)
{
  GArray *tokens;
  GQueue *rpn;
  gboolean success;

  if( expr == NULL || result == NULL )
  {
    Stop(_("Invalid arguments to sy_evaluate"), ERR_OK);
    return FALSE;
  }
  else
  {
    tokens = sy_tokenize(expr);
    if( tokens == NULL )
      return FALSE;
    else
    {
      rpn = sy_infix_to_rpn(tokens);
      g_array_free(tokens, TRUE);

      if( rpn == NULL )
        return FALSE;
      else
      {
        /* RPN queue visualization for debugging */
        if( rpn != NULL )
        {
          GList *iter;
          sy_token_t *t;
          gint idx;

          pr_debug("RPN queue (%d tokens):\n", g_queue_get_length(rpn));
          iter = rpn->head;
          idx = 0;

          while( iter != NULL )
          {
            t = (sy_token_t *)iter->data;

            if( t->type == SY_TOKEN_NUMBER )
            {
              pr_debug("  [%d] NUMBER: %.9f\n", idx, t->value.number);
            }
            else if( t->type == SY_TOKEN_SYMBOL )
            {
              pr_debug("  [%d] SYMBOL: '%s'\n", idx, t->value.name);
            }
            else if( t->type == SY_TOKEN_OPERATOR )
            {
              pr_debug("  [%d] OP: '%c'\n", idx, t->value.op);
            }
            else if( t->type == SY_TOKEN_FUNCTION )
            {
              pr_debug("  [%d] FUNC: '%s'\n", idx, t->value.name);
            }
            else
            {
              pr_debug("  [%d] TOKEN: type=%d\n", idx, t->type);
            }

            iter = iter->next;
            idx++;
          }
        }

        success = sy_evaluate_rpn(rpn, result);
        g_queue_free_full(rpn, g_free);
        return success;
      }
    }
  }
}

gboolean
sy_is_expression(const gchar *field)
{
  const gchar *p;
  gboolean in_exponent;

  if( field == NULL )
    return FALSE;
  else
  {
    p = field;
    in_exponent = FALSE;

    while( *p != '\0' )
    {
      if( is_sy_whitespace(*p) )
      {
        p++;
        continue;
      }

      if( is_sy_exponent_marker(*p) )
      {
        in_exponent = TRUE;
        p++;
        continue;
      }

      if( in_exponent && is_sy_sign(*p) )
      {
        in_exponent = FALSE;
        p++;
        continue;
      }

      if( g_ascii_isalpha((gint)*p) || *p == '_' )
        return TRUE;
      else
      {
        if( is_sy_expr_only_char(*p) )
          return TRUE;
        else
        {
          p++;
          in_exponent = FALSE;
        }
      }
    }

    return FALSE;
  }
}
