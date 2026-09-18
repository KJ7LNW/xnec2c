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

/*------------------------------------------------------------------------*/

/** config_widget_project_element - write one source's bytes into one widget
 * @t:       the widget and the element describing how it expresses a value
 * @operand: the bytes to project and their extent
 */
static void
config_widget_project_element(const config_widget_target_t *t,
    const void *operand)
{
  const config_widget_source_t *src = operand;

  config_widget_sync_element(src->storage, src->size, t->element, t->widget);
}

/*------------------------------------------------------------------------*/

void
config_widget_group_project(const config_widget_group_view_t *gv,
    const void *operand)
{
  GtkBuilder *builder = *gv->declaration->builder;
  GtkWidget *menu = config_widget_menu_shell(builder, gv->declaration);

  config_widget_group_block(builder, gv->declaration);
  config_widget_menu_block(menu);

  config_widget_walk_members(gv, config_widget_project_element, operand);

  config_widget_menu_unblock(menu);
  config_widget_group_unblock(builder, gv->declaration);

  config_widget_label_sync(gv->binding, builder, gv->declaration, menu);
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_field(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);

  if( b == NULL )
  {
    BUG("config_widget_sync_field: field is not registered\n");
    return;
  }

  config_widget_source_t src = config_widget_scope_source(b);

  config_widget_walk_groups(b, config_widget_group_project, &src);
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
    config_widget_source_t src = config_widget_scope_source(b);
    const config_widget_group_t *const *g;

    for( g = b->tree->groups; *g != NULL; g++ )
    {
      config_widget_group_view_t gv = { .binding = b, .declaration = *g };

      if( (*g)->builder != builder )
        continue;

      config_widget_group_project(&gv, &src);
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
    config_widget_sync_field(config_widget_binding_at(i)->scope.dest.storage);
}
