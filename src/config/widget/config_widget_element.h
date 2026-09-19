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

#ifndef CONFIG_WIDGET_ELEMENT_H
#define CONFIG_WIDGET_ELEMENT_H 1

#include "config_widget.h"

/*
 * What one live GTK widget and its element say to each other: the value a
 * click is about to write, the value a click did write, whether a stored
 * value is the one a row names, and the projection of a value back onto
 * the widget.  Every caller holding a widget and its element reads this
 * contract, inside the engine and out.
 */

/** config_widget_element_candidate - predict what clicking a widget writes
 * @elt:  element describing the widget and its selection values
 * @w:    widget before its click edge
 * @out:  buffer of @size bytes
 * @size: the selection's field width
 *
 * Return: FALSE when the widget exposes no discrete click outcome, leaving
 * @out untouched.
 */
gboolean config_widget_element_candidate(const config_widget_element_t *elt,
    GtkWidget *w, void *out, size_t size);

/** config_widget_element_selects - whether an element names a selection value
 * @elt: element describing the row
 *
 * Return: TRUE when the row expresses one value, in either width; FALSE when
 * the row expresses its own widget state instead.
 */
gboolean config_widget_element_selects(const config_widget_element_t *elt);

/** config_widget_element_holds - whether a field holds a row's selection
 * @field: field address
 * @size:  the selection's field width
 * @elt:   element naming a selection in either width
 *
 * Return: TRUE when the stored value equals the value @elt names; FALSE for
 * a row naming no selection, which expresses its own state instead.
 */
gboolean config_widget_element_holds(const void *field, size_t size,
    const config_widget_element_t *elt);

/** config_widget_element_commit_value - what committing a widget writes
 * @elt:  element describing the widget
 * @w:    the resolved widget
 * @out:  buffer of @size bytes
 * @size: the selection's field width
 *
 * Return: FALSE when the widget contributes no value, which is the inactive
 * valued radio whose active peer carries the selection, and an unselected
 * combo.
 */
gboolean config_widget_element_commit_value(const config_widget_element_t *elt,
    GtkWidget *w, void *out, size_t size);

/** config_widget_sync_element - write the field's value into one widget */
void config_widget_sync_element(const void *field, size_t size,
    const config_widget_element_t *elt, GtkWidget *w);

#endif /* CONFIG_WIDGET_ELEMENT_H */
