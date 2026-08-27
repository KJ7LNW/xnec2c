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

#ifndef CONFIG_WIDGET_ROW_H
#define CONFIG_WIDGET_ROW_H 1

#include "config_widget.h"

/*
 * config_widget_row: the binding one widget carries.
 *
 * A row names the selection it belongs to and the element describing how
 * this particular widget expresses a value.  It mirrors nothing its scope
 * owns, so destination, refresh, commit edge and preview policy each keep a
 * single owner.  Static rows resolved from a builder and runtime rows built
 * by a selector carry the same record, so the commit path and the hover
 * edges read one description for either kind.
 */
typedef struct {
  const config_widget_scope_t   *scope;
  const config_widget_element_t *elt;
} config_widget_row_t;

/*------------------------------------------------------------------------*/

/** config_widget_row_attach - bind a widget through a caller-owned element
 * @w:     the widget the element describes
 * @scope: the capabilities the widget dispatches through
 * @elt:   an element of static storage duration
 *
 * For rows a builder resolves, whose element is declared in the field's
 * binding tree.  Wires the hover edges of a previewing row.  Repeated
 * attachment on one widget is idempotent, so re-resolving a group rebinds
 * nothing.
 */
void config_widget_row_attach(GtkWidget *w, const config_widget_scope_t *scope,
                              const config_widget_element_t *elt);

/** config_widget_row_get - read the binding a widget carries
 * @w: any widget
 *
 * Return: the row, or NULL when @w carries no binding.
 */
const config_widget_row_t *config_widget_row_get(GtkWidget *w);

#endif /* CONFIG_WIDGET_ROW_H */
