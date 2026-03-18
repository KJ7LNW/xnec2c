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

#include "expr_edit.h"
#include "sy_expr.h"
#include "shared.h"
#include <string.h>

/*------------------------------------------------------------------------*/

/* expr_edit_field_protected()
 *
 * Returns TRUE when expr contains active expression text.
 * Empty string "" indicates no expression — field is plain numeric.
 */
gboolean
expr_edit_field_protected( const gchar *expr )
{
  return (expr != NULL) && (expr[0] != '\0');
}

/*------------------------------------------------------------------------*/

/* expr_spin_input_cb()
 *
 * GTK "input" signal handler for expression-aware spinbuttons.
 * Called when GTK converts entry text to a numeric value (e.g. during
 * gtk_spin_button_update).  Evaluates expression text via sy_evaluate
 * so GTK receives the correct numeric value instead of a failed strtod
 * parse.  Returns FALSE for plain numbers to let GTK handle normally.
 */
static gint
expr_spin_input_cb( GtkSpinButton *spin,
    gdouble *new_val, gpointer data )
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin));

  if( sy_is_expression(text) )
  {
    /* Suppress error dialogs during display-time evaluation;
     * failures return GTK_INPUT_ERROR and GTK rejects the edit */
    sy_errors_begin();
    if( !sy_evaluate(text, new_val) )
    {
      sy_errors_discard();
      return GTK_INPUT_ERROR;
    }
    sy_errors_discard();

    /* Update stored expression copy to match current entry text,
     * so the output handler displays the latest user-typed expression */
    g_object_set_data_full(G_OBJECT(spin), "expr-text",
        g_strdup(text), g_free);

    return TRUE;
  }

  /* Plain number: clear stored expression so the output handler does
   * not restore a stale expression over the newly typed number */
  g_object_set_data(G_OBJECT(spin), "expr-text", NULL);

  return FALSE;
}

/*------------------------------------------------------------------------*/

/* expr_spin_output_cb()
 *
 * GTK "output" signal handler for expression-aware spinbuttons.
 * Called when GTK converts a numeric value to display text.  When an
 * expression is active, sets the entry text to the expression string
 * and returns TRUE to suppress GTK's default numeric formatting.
 */
static gboolean
expr_spin_output_cb( GtkSpinButton *spin, gpointer data )
{
  const gchar *expr =
    (const gchar *)g_object_get_data(G_OBJECT(spin), "expr-text");

  if( (expr != NULL) && (expr[0] != '\0') )
  {
    gtk_entry_set_text(GTK_ENTRY(spin), expr);
    return TRUE;
  }

  return FALSE;
}

/*------------------------------------------------------------------------*/

/* expr_edit_write_field()
 *
 * Populates a spin button for display.  Connects GTK input/output signal
 * handlers once per widget to prevent gtk_spin_button_update from
 * destroying expression text via strtod.  When an expression is active,
 * switches to non-numeric mode and shows the expression text.  When
 * plain numeric, restores numeric mode and sets the value normally.
 */
void
expr_edit_write_field( GtkSpinButton *spin,
    gdouble value, const gchar *expr )
{
  /* Connect input/output signal handlers once per widget */
  if( !g_object_get_data(G_OBJECT(spin), "expr-connected") )
  {
    g_signal_connect(spin, "input",
        G_CALLBACK(expr_spin_input_cb), NULL);
    g_signal_connect(spin, "output",
        G_CALLBACK(expr_spin_output_cb), NULL);
    g_object_set_data(G_OBJECT(spin), "expr-connected",
        GINT_TO_POINTER(TRUE));
  }

  if( expr_edit_field_protected(expr) )
  {
    /* Store a copy of expression text for the output handler */
    g_object_set_data_full(G_OBJECT(spin), "expr-text",
        g_strdup(expr), g_free);
    gtk_spin_button_set_numeric(spin, FALSE);
    gtk_entry_set_text(GTK_ENTRY(spin), expr);
  }
  else
  {
    /* Clear stored expression; output handler will return FALSE
     * and let GTK format the numeric value normally */
    g_object_set_data(G_OBJECT(spin), "expr-text", NULL);
    gtk_spin_button_set_numeric(spin, FALSE);
    gtk_spin_button_set_value(spin, value);
  }
}

/*------------------------------------------------------------------------*/

/* expr_edit_save_field()
 *
 * Reads the spin button text on apply.  Detects whether the user typed
 * a plain number or an expression, evaluates accordingly, and reports
 * both the numeric value and the expression text (or "" for plain).
 */
gboolean
expr_edit_save_field( GtkSpinButton *spin,
    gdouble *value, gchar *expr_out, gsize sz )
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin));

  if( sy_is_expression(text) )
  {
    if( !sy_evaluate(text, value) )
    {
      /* Preserve the failed expression text so the user can see and fix it.
       * Set numeric to zero as a safe default; caller handles error display
       * via sy_errors_begin/sy_errors_end accumulation. */
      g_strlcpy(expr_out, text, sz);
      *value = 0.0;
      return FALSE;
    }
    g_strlcpy(expr_out, text, sz);
  }
  else
  {
    *value = Strtod((char *)text, NULL);
    expr_out[0] = '\0';
  }

  return TRUE;
}

/*------------------------------------------------------------------------*/

/* expr_edit_read_exprs()
 *
 * Loads shadow expression column text into a caller-allocated array.
 * Empty string in a column means the corresponding field is plain numeric.
 */
void
expr_edit_read_exprs( GtkListStore *store, GtkTreeIter *iter,
    gint first_ecol, gchar exprs[][EXPR_FIELD_LEN], gint count )
{
  gint col, idx;
  gchar *text = NULL;

  for( idx = 0; idx < count; idx++ )
  {
    col = first_ecol + idx;
    gtk_tree_model_get(GTK_TREE_MODEL(store), iter, col, &text, -1);

    if( text != NULL )
    {
      g_strlcpy(exprs[idx], text, EXPR_FIELD_LEN);
      g_free(text);
      text = NULL;
    }
    else
    {
      exprs[idx][0] = '\0';
    }
  }
}

/*------------------------------------------------------------------------*/

/* expr_edit_write_exprs()
 *
 * Persists the expression text array back to shadow columns.
 * Writes "" to clear a column when the field reverted to plain numeric.
 */
void
expr_edit_write_exprs( GtkListStore *store, GtkTreeIter *iter,
    gint first_ecol, gchar exprs[][EXPR_FIELD_LEN], gint count )
{
  gint col, idx;

  for( idx = 0; idx < count; idx++ )
  {
    col = first_ecol + idx;
    gtk_list_store_set(store, iter, col, exprs[idx], -1);
  }
}

/*------------------------------------------------------------------------*/

/* expr_edit_protect_widgets()
 *
 * Grays out a group of widgets when the controlling field carries an
 * expression.  Sets tooltip to EXPR_TOOLTIP when protected, clears it
 * when plain numeric.
 */
void
expr_edit_protect_widgets( GtkBuilder *builder,
    const char *names[], int count, const char *expr )
{
  gboolean sens = !expr_edit_field_protected(expr);
  int i;

  for( i = 0; i < count; i++ )
  {
    GtkWidget *w = Builder_Get_Object(builder, (gchar *)names[i]);
    gtk_widget_set_sensitive(w, sens);
    gtk_widget_set_tooltip_text(w, sens ? NULL : EXPR_TOOLTIP);
  }
}

/*------------------------------------------------------------------------*/

