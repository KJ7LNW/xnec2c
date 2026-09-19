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

#ifndef CONFIG_PREVIEW_ROW_H
#define CONFIG_PREVIEW_ROW_H 1

#include <gtk/gtk.h>

/*
 * The hover edges one bound row carries.  A selection binding a row asks
 * this domain whether the row's class hovers at all, and hands it the row
 * to wire; the staging session behind those edges stays here.
 */

/** config_preview_class_hoverable - test for per-row hover edges
 * @w: widget whose class defines the available edges
 *
 * Return: TRUE for menu items and toggle buttons; FALSE for every class that
 * presents no discrete row under the pointer.
 */
gboolean config_preview_class_hoverable(GtkWidget *w);

/** config_preview_row_attach - wire the hover edges a bound row's class carries
 * @w: a widget already carrying its binding row, of a class the caller has
 *     confirmed hoverable
 *
 * A menu item stages on selection and its shell reverts when that shell
 * completes a selection or withdraws; a toggle button stages on pointer
 * entry and reverts on pointer exit.
 */
void config_preview_row_attach(GtkWidget *w);

#endif /* CONFIG_PREVIEW_ROW_H */
