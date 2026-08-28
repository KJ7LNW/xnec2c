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

/** config_widget_label_sync - name the active row on a collapsed pull-down
 * @b:       the binding whose value selects the row
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b optionally naming a value label
 *
 * Each row name lives once in its menu; the collapsed button borrows the
 * name of the row the stored value resolves to.  The mnemonic marker a row
 * carries is parsed rather than displayed.
 */
static void
config_widget_label_sync(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;
  GtkWidget *label;
  GtkWidget *row = NULL;
  int val;

  /* This group presents its selection on the rows alone */
  if( g->value_label_id == NULL )
    return;

  label = config_widget_lookup(builder, g->value_label_id);

  /* The lookup reports an unresolved id */
  if( label == NULL )
    return;

  val = field_read_int(b->field, b->size);

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
  else
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
 * write raises reaches no live handler.  Resolving a row is also where it
 * receives its binding, so hover and commit coverage follow the tree with no
 * per-selector call site.
 */
static void
config_widget_sync_group(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;

  config_widget_group_block(builder, g);

  for( e = g->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    config_widget_sync_element(b->field, b->size, *e, w);
    config_widget_row_attach(w, b->scope, *e);
  }

  config_widget_group_unblock(builder, g);

  config_widget_label_sync(b, builder, g);
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
