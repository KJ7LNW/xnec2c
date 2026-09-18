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

/*
 * config_widget_label: the name a collapsed pull-down button reads.
 *
 * Resolves the row a stored value selects, among the rows a group declares
 * or the rows a selector built, and lends that row's name to the button.
 */

#include "config_widget_priv.h"

/*------------------------------------------------------------------------*/

/** config_widget_label_row_declared - the declared row a group's value selects
 * @b:       the binding whose value selects the row
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b naming its rows in the binding tree
 *
 * Return: the row expressing the stored value, or NULL when the group
 * declares none.
 */
static GtkWidget *
config_widget_label_row_declared(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;
  GtkWidget *row = NULL;
  int val = field_read_int(b->scope.dest.storage, b->scope.dest.size);

  for( e = g->elements; (*e != NULL) && (row == NULL); e++ )
  {
    const int *v = (*e)->values;

    /* One value ahead of the terminator is a selection this row alone
     * expresses; a longer list reaches several values through one widget */
    if( (v == NULL) || (v[0] != val) ||
        (v[1] != CONFIG_WIDGET_VALUES_END) )
      continue;

    row = config_widget_lookup(builder, (*e)->widget_id);
  }

  if( row == NULL )
    BUG("no menu row expresses the stored selection value %d\n", val);

  return row;
}

/** config_widget_label_row_built - the runtime row a group's value selects
 * @b:    the binding whose value selects the row
 * @menu: the group's runtime row container
 *
 * Return: the row whose selection equals the stored value, or NULL when the
 * stored value names no row the selector built.
 */
static GtkWidget *
config_widget_label_row_built(const config_widget_binding_t *b, GtkWidget *menu)
{
  GList *rows = gtk_container_get_children(GTK_CONTAINER(menu));
  GList *r;
  GtkWidget *row = NULL;

  for( r = rows; (r != NULL) && (row == NULL); r = r->next )
  {
    GtkWidget *w = GTK_WIDGET(r->data);
    const config_widget_element_t *elt = config_widget_menu_element(b, w);

    if( (elt == NULL) || !config_widget_element_holds(b->scope.dest.storage,
          b->scope.dest.size, elt) )
      continue;

    row = w;
  }

  g_list_free(rows);

  return row;
}

/** config_widget_label_sync - name the active row on a collapsed pull-down
 * @b:       the binding whose value selects the row
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b optionally naming a value label
 * @menu:    the group's resolved runtime row container, or NULL for a group
 *           declaring its rows
 *
 * Each row name lives once in its menu; the collapsed button borrows the
 * name of the row the stored value resolves to.  The mnemonic marker a row
 * carries is parsed rather than displayed.  A group whose rows a selector
 * built reads them from its shell, where a selection wider than an int
 * resolves at its own width.
 */
void
config_widget_label_sync(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g, GtkWidget *menu)
{
  GtkWidget *label;
  GtkWidget *row;

  /* This group presents its selection on the rows alone */
  if( g->value_label_id == NULL )
    return;

  label = config_widget_lookup(builder, g->value_label_id);

  /* The lookup reports an unresolved id */
  if( label == NULL )
    return;

  if( menu == NULL )
    row = config_widget_label_row_declared(b, builder, g);
  else
    row = config_widget_label_row_built(b, menu);

  /* A stored value no row expresses leaves the button reading the name it
   * already holds */
  if( row != NULL )
    gtk_label_set_text_with_mnemonic( GTK_LABEL(label),
        gtk_menu_item_get_label( GTK_MENU_ITEM(row) ) );
}
