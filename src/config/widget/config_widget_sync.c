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
 * config_widget_sync: projection of a field's value onto its widgets.
 *
 * Broadcasts a field to every live peer widget across the builders its tree
 * names, and runs the builder-scoped bulk sync and hook passes.
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
static void
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
static void
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
static GtkWidget *
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
static const config_widget_element_t *
config_widget_menu_element(const config_widget_binding_t *b, GtkWidget *child)
{
  const config_widget_row_t *rec = config_widget_row_get(child);

  if( (rec == NULL) || (rec->scope != b->scope) )
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
static void
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
static void
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

/** config_widget_sync_elements - write a binding's field into its declared rows
 * @b:       the binding whose field value is broadcast
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b bound to @builder
 *
 * Resolving a row is also where it receives its binding, so hover and commit
 * coverage follow the tree with no per-selector call site.
 */
static void
config_widget_sync_elements(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;

  for( e = g->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    config_widget_sync_element(b->field, b->size, *e, w);
    config_widget_row_attach(w, b->scope, *e);
  }
}

/** config_widget_sync_menu - write a binding's field into its runtime rows
 * @b:    the binding whose field value is broadcast
 * @menu: the group's runtime row container, or NULL for a declared group
 *
 * Each row carries the element the selector built for it and its binding
 * with it, so the projection reads that description rather than a declared
 * one.
 */
static void
config_widget_sync_menu(const config_widget_binding_t *b, GtkWidget *menu)
{
  GList *rows;
  GList *r;

  if( menu == NULL ) return;

  rows = gtk_container_get_children(GTK_CONTAINER(menu));

  for( r = rows; r != NULL; r = r->next )
  {
    GtkWidget *w = GTK_WIDGET(r->data);
    const config_widget_element_t *elt = config_widget_menu_element(b, w);

    if( elt == NULL )
      continue;

    config_widget_sync_element(b->field, b->size, elt, w);
  }

  g_list_free(rows);
}

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
  int val = field_read_int(b->field, b->size);

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

    if( (elt == NULL) || !config_widget_element_holds(b->field, b->size, elt) )
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
static void
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

/*------------------------------------------------------------------------*/

/** config_widget_sync_group - write a binding's field into one live group
 * @b:       the binding whose field value is broadcast
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b bound to @builder
 *
 * The group is silenced across the whole write, so a radio deactivation the
 * write raises reaches no live handler.  Declared rows and rows a selector
 * built project alike, and the collapsed label then names the committed
 * selection.
 */
static void
config_widget_sync_group(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  GtkWidget *menu = config_widget_menu_shell(builder, g);

  config_widget_group_block(builder, g);
  config_widget_menu_block(menu);

  config_widget_sync_elements(b, builder, g);
  config_widget_sync_menu(b, menu);

  config_widget_menu_unblock(menu);
  config_widget_group_unblock(builder, g);

  config_widget_label_sync(b, builder, g, menu);
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_field(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);
  const config_widget_group_t *const *g;

  if( b == NULL )
  {
    BUG("config_widget_sync_field: field is not registered\n");
    return;
  }

  for( g = b->tree->groups; *g != NULL; g++ )
  {
    GtkBuilder *builder;

    if( (*g)->builder == NULL || (builder = *(*g)->builder) == NULL )
      continue; /* dormant: window not built yet */

    config_widget_sync_group(b, builder, *g);
  }
}

/*------------------------------------------------------------------------*/

/** config_widget_first_id - a bound id naming a field in a diagnostic
 * @b: the binding whose groups are searched
 *
 * Return: the first element id any group declares, or a placeholder when
 * every group reaches its rows through a runtime-filled menu shell.
 */
static const char *
config_widget_first_id(const config_widget_binding_t *b)
{
  const config_widget_group_t *const *g;
  const char *id = NULL;

  for( g = b->tree->groups; *g != NULL && id == NULL; g++ )
  {
    if( (*g)->elements[0] != NULL )
      id = (*g)->elements[0]->widget_id;
  }

  return id != NULL ? id : "(no declared element)";
}

/*------------------------------------------------------------------------*/

GtkWidget *
config_widget_field_widget(void *field, GtkBuilder **builder)
{
  config_widget_binding_t *b = config_widget_find(field);
  const config_widget_group_t *const *g;
  const config_widget_group_t *found = NULL;
  GtkWidget *w = NULL;
  const char *id;

  if( b == NULL )
  {
    BUG("config_widget_field_widget: field is not registered\n");
    return NULL;
  }

  id = config_widget_first_id(b);

  for( g = b->tree->groups; *g != NULL && found == NULL; g++ )
  {
    if( (*g)->builder == builder )
      found = *g;
  }

  /* A dormant group is a caller error here rather than the no-op a bulk
   * sync takes: a window that is not built holds no widget to return. */
  if( found == NULL )
    BUG("config_widget_field_widget: %s names no group on this builder\n", id);
  else if( *builder == NULL )
    BUG("config_widget_field_widget: %s names a window not built\n", id);
  else if( found->elements[0] == NULL || found->elements[1] != NULL )
    BUG("config_widget_field_widget: %s does not name one widget\n", id);
  else
    w = config_widget_lookup(*builder, found->elements[0]->widget_id);

  return w;
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_builder(GtkBuilder **builder)
{
  int i;

  if( builder == NULL || *builder == NULL )
    return;

  for( i = 0; i < config_widget_binding_count(); i++ )
  {
    config_widget_binding_t *b = config_widget_binding_at(i);
    const config_widget_group_t *const *g;

    for( g = b->tree->groups; *g != NULL; g++ )
    {
      if( (*g)->builder != builder )
        continue;

      config_widget_sync_group(b, *builder, *g);
    }
  }
}

/*------------------------------------------------------------------------*/

void
config_widget_run_hooks(GtkBuilder **builder)
{
  int i;

  if( builder == NULL || *builder == NULL )
    return;

  for( i = 0; i < config_widget_binding_count(); i++ )
  {
    config_widget_binding_t *b = config_widget_binding_at(i);
    const config_widget_group_t *const *g;

    if( b->tree->post_apply == NULL )
      continue;

    for( g = b->tree->groups; *g != NULL; g++ )
      if( (*g)->builder == builder )
      {
        b->tree->post_apply->fn();
        break; /* run once even if the tree has other groups on this builder */
      }
  }
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_all(void)
{
  int i;

  for( i = 0; i < config_widget_binding_count(); i++ )
    config_widget_sync_field(config_widget_binding_at(i)->field);
}
