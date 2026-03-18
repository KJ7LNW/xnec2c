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

#ifndef EXPR_EDIT_H
#define EXPR_EDIT_H  1

#include "common.h"
#include "editors.h"

/*
 * Maximum length of a single expression field text, including NUL terminator.
 * Matches the field_buf size used in parse_field_with_expression().
 */
#define EXPR_FIELD_LEN  256

/* Tooltip text shown on widgets disabled by an SY expression */
#define EXPR_TOOLTIP _("Controlled by SY expression")

/*
 * expr_edit_field_protected - test whether a field carries an expression
 * @expr: shadow expression string for the field
 *
 * Returns TRUE when the field contains an active expression that must not
 * be overwritten by radio-button or checkbutton write-back.
 */
gboolean expr_edit_field_protected( const gchar *expr );

/*
 * expr_edit_write_field - populate a spin button for display
 * @spin:   spin button widget
 * @value:  evaluated numeric value
 * @expr:   shadow expression string ("" when field is plain numeric)
 *
 * When expr is non-empty, sets the spin button to non-numeric mode and
 * displays the expression text.  Spin button arrows are inert in this mode.
 * When expr is empty, restores numeric mode and sets the numeric value.
 */
void expr_edit_write_field( GtkSpinButton *spin,
    gdouble value, const gchar *expr );

/*
 * expr_edit_save_field - read back a spin button and detect expressions
 * @spin:     spin button widget
 * @value:    output: evaluated numeric value
 * @expr_out: output: expression text, or "" if plain numeric
 * @sz:       size of expr_out buffer
 *
 * Reads the spin button text.  If it is an expression, evaluates it and
 * copies the text to expr_out.  If it is a plain number, parses it and
 * clears expr_out.  Returns TRUE on success, FALSE if evaluation fails.
 */
gboolean expr_edit_save_field( GtkSpinButton *spin,
    gdouble *value, gchar *expr_out, gsize sz );

/*
 * expr_edit_read_exprs - load shadow expression columns for a treeview row
 * @store:      list store containing the row
 * @iter:       iterator for the row
 * @first_ecol: first shadow expression column index (e.g. GEOM_COL_EI1)
 * @exprs:      caller-allocated array of EXPR_FIELD_LEN-byte buffers
 * @count:      number of shadow columns to read
 */
void expr_edit_read_exprs( GtkListStore *store, GtkTreeIter *iter,
    gint first_ecol, gchar exprs[][EXPR_FIELD_LEN], gint count );

/*
 * expr_edit_write_exprs - persist shadow expression columns for a treeview row
 * @store:      list store containing the row
 * @iter:       iterator for the row
 * @first_ecol: first shadow expression column index
 * @exprs:      array of EXPR_FIELD_LEN-byte expression buffers
 * @count:      number of shadow columns to write
 */
void expr_edit_write_exprs( GtkListStore *store, GtkTreeIter *iter,
    gint first_ecol, gchar exprs[][EXPR_FIELD_LEN], gint count );

/*
 * expr_edit_protect_widgets - gray out widgets controlled by an expression
 * @builder: GtkBuilder containing the widgets
 * @names:   array of widget name strings
 * @count:   number of entries in names[]
 * @expr:    shadow expression string for the controlling field
 *
 * Sets each named widget insensitive and adds a tooltip when the field
 * carries an expression.  Restores sensitivity and clears tooltip when
 * the field is plain numeric.
 */
void expr_edit_protect_widgets( GtkBuilder *builder,
    const char *names[], int count, const char *expr );

#endif /* EXPR_EDIT_H */
