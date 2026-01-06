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

#ifndef SY_EXPR_H
#define SY_EXPR_H    1

#include <glib.h>

/* Get bounds for a symbol
 * name: symbol name
 * min: pointer to store minimum value
 * max: pointer to store maximum value
 * Returns: TRUE on success, FALSE if symbol not found or parameters NULL
 */
gboolean sy_get_bounds(const gchar *name, gdouble *min, gdouble *max);

/* Initialize symbol table and expression evaluator */
gboolean sy_init(void);

/* Cleanup symbol table and free resources */
void sy_cleanup(void);

/* Define or update a symbol with given value or expression
 * name: symbol name (will be normalized to uppercase for case-insensitive lookup)
 * value_or_expr: numeric string or expression to evaluate and store
 * Returns: TRUE on success, FALSE on error
 */
gboolean sy_define(const gchar *name, const gchar *value_or_expr);

/* Evaluate an expression and return result
 * expr: expression string (e.g., "H+fp*2", "sin(45)", "2*PI")
 * result: pointer to store evaluated result
 * Returns: TRUE on success, FALSE on error (undefined symbol, syntax error, etc.)
 */
gboolean sy_evaluate(const gchar *expr, gdouble *result);

/* Check if a field contains an expression rather than a plain number
 * field: string to check
 * Returns: TRUE if field contains expression syntax, FALSE if plain number
 */
gboolean sy_is_expression(const gchar *field);

/* Load symbol overrides from .sy file
 * filename: path to .sy file (e.g., "model.sy")
 * Returns: TRUE if file loaded, FALSE if file missing or empty
 * Silently returns FALSE if file does not exist
 * Uses pr_err for malformed lines, continues processing
 */
gboolean sy_load_overrides(const gchar *filename);

#endif
