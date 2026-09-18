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
 * config_widget_walk: the widgets one binding reaches.
 *
 * Resolves the rows a group declares and the shell a selector fills at
 * runtime, tells the rows of one field from the rows sharing that shell,
 * and silences and restores the commit handler across either set.
 */

#include "config_widget_priv.h"
#include "config_widget_row.h"

/*------------------------------------------------------------------------*/

/** config_widget_group_block - silence the commit handler across a group
 * @builder: the group's resolved, non-NULL builder
 * @g:       the group about to be written programmatically
 *
 * Writing one radio deactivates a peer, and GTK signals that peer rather
 * than the widget written, so the whole group falls silent for the write.
 * Paired with config_widget_group_unblock().
 */
void
config_widget_group_block(GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;

  for( e = g->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    SIGNAL_BLOCK(w, on_config_widget_changed);
  }
}

/** config_widget_group_unblock - restore the commit handler across a group
 * @builder: the group's resolved, non-NULL builder
 * @g:       the group whose programmatic write has completed
 *
 * Paired with config_widget_group_block().
 */
void
config_widget_group_unblock(GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;

  for( e = g->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    SIGNAL_UNBLOCK(w, on_config_widget_changed);
  }
}

/** config_widget_menu_shell - the runtime row container a group names
 * @builder: the group's resolved, non-NULL builder
 * @g:       the group, which names a menu only when a selector fills it
 *
 * Return: the menu holding rows built at runtime, or NULL when the group
 * declares every row it presents.
 */
GtkWidget *
config_widget_menu_shell(GtkBuilder *builder, const config_widget_group_t *g)
{
  if( g->value_menu_id == NULL )
    return NULL;

  return config_widget_lookup(builder, g->value_menu_id);
}

/** config_widget_menu_element - the element a menu child carries for one binding
 * @b:     the binding whose rows are wanted
 * @child: one child of a group's runtime menu
 *
 * One shell carries the rows of every field presented on it, so a child
 * dispatching through other capabilities belongs to another field, and a
 * separator carries no row at all.
 *
 * Return: the child's element, or NULL when the child is not a row of @b.
 */
const config_widget_element_t *
config_widget_menu_element(const config_widget_binding_t *b, GtkWidget *child)
{
  const config_widget_row_t *rec = config_widget_row_get(child);

  if( (rec == NULL) || (rec->scope != &b->scope) )
    return NULL;

  return rec->elt;
}

/** config_widget_menu_block - silence the commit handler across a runtime menu
 * @menu: the group's runtime row container, or NULL for a declared group
 *
 * Every row of the shell falls silent, whichever field it presents, so a
 * write reaches no handler through a peer it deactivates.  Paired with
 * config_widget_menu_unblock().
 */
void
config_widget_menu_block(GtkWidget *menu)
{
  GList *rows;
  GList *r;

  if( menu == NULL ) return;

  rows = gtk_container_get_children(GTK_CONTAINER(menu));

  for( r = rows; r != NULL; r = r->next )
    SIGNAL_BLOCK(GTK_WIDGET(r->data), on_config_widget_changed);

  g_list_free(rows);
}

/** config_widget_menu_unblock - restore the commit handler across a runtime menu
 * @menu: the group's runtime row container, or NULL for a declared group
 *
 * Paired with config_widget_menu_block().
 */
void
config_widget_menu_unblock(GtkWidget *menu)
{
  GList *rows;
  GList *r;

  if( menu == NULL ) return;

  rows = gtk_container_get_children(GTK_CONTAINER(menu));

  for( r = rows; r != NULL; r = r->next )
    SIGNAL_UNBLOCK(GTK_WIDGET(r->data), on_config_widget_changed);

  g_list_free(rows);
}

/*------------------------------------------------------------------------*/

/** config_widget_walk_groups - visit every live group one binding presents
 * @b:       any registered binding
 * @visit:   what the operation does at each group
 * @operand: carried through to @visit unread
 *
 * Groups arrive in declaration order.  A group whose window is not built
 * holds no widget and is passed over, so a datum reaching one window and a
 * datum reaching several are the same walk.
 */
void
config_widget_walk_groups(const config_widget_binding_t *b,
    config_widget_group_fn visit, const void *operand)
{
  const config_widget_group_t *const *g;

  for( g = b->tree->groups; *g != NULL; g++ )
  {
    if( (*g)->builder == NULL || *(*g)->builder == NULL )
      continue; /* dormant: window not built yet */

    config_widget_group_view_t gv = { .binding = b, .declaration = *g };

    visit(&gv, operand);
  }
}

/** config_widget_walk_declared - visit the rows a group's tree names
 * @gv:      a binding and one of its groups, whose builder is live
 * @visit:   what the operation does at each widget
 * @operand: carried through to @visit unread
 */
static void
config_widget_walk_declared(const config_widget_group_view_t *gv,
    config_widget_visit_fn visit, const void *operand)
{
  GtkBuilder *builder = *gv->declaration->builder;
  const config_widget_element_t *const *e;

  for( e = gv->declaration->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    /* Resolving a declared row is also where it receives its binding, so
     * hover and commit coverage follow the tree with no per-selector call
     * site */
    config_widget_row_attach(w, &gv->binding->scope, *e);

    config_widget_target_t t =
      { .widget = w, .element = *e, .scope = &gv->binding->scope };

    visit(&t, operand);
  }
}

/** config_widget_walk_runtime - visit the rows a selector built on a group's shell
 * @gv:      a binding and one of its groups, whose builder is live
 * @visit:   what the operation does at each widget
 * @operand: carried through to @visit unread
 */
static void
config_widget_walk_runtime(const config_widget_group_view_t *gv,
    config_widget_visit_fn visit, const void *operand)
{
  GtkWidget *menu;
  GList *rows;
  GList *r;

  menu = config_widget_menu_shell(*gv->declaration->builder, gv->declaration);

  if( menu == NULL ) return;

  rows = gtk_container_get_children(GTK_CONTAINER(menu));

  for( r = rows; r != NULL; r = r->next )
  {
    GtkWidget *w = GTK_WIDGET(r->data);
    const config_widget_element_t *elt =
      config_widget_menu_element(gv->binding, w);

    /* A child dispatching through other capabilities belongs to another
     * field, and a separator carries no row at all */
    if( elt == NULL )
      continue;

    config_widget_target_t t =
      { .widget = w, .element = elt, .scope = &gv->binding->scope };

    visit(&t, operand);
  }

  g_list_free(rows);
}

/** config_widget_walk_members - visit every widget one group presents a field on
 * @gv:      a binding and one of its groups, whose builder is live
 * @visit:   what the operation does at each widget
 * @operand: carried through to @visit unread
 *
 * Declared rows first, then the rows a selector built on the group's shell,
 * which is the order a write needs so a collapsed label reads a settled
 * group.
 */
void
config_widget_walk_members(const config_widget_group_view_t *gv,
    config_widget_visit_fn visit, const void *operand)
{
  config_widget_walk_declared(gv, visit, operand);
  config_widget_walk_runtime(gv, visit, operand);
}
