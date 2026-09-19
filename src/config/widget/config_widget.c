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
 * config_widget: row-rooted config-widget binding registry.
 *
 * Owns the field-to-tree registry, widget resolution, and the commit path
 * that carries a user edit from a bound widget into its field and on to
 * the field's hooks and peers.
 */

#include <string.h>
#include "config_widget_element.h"
#include "config_widget_priv.h"
#include "config_widget_row.h"
#include "../config_preview.h"
#include "../../mem/mem.h"

/* Each binding is owned on its own and the registry holds it by pointer, so
 * the capabilities a widget retains by reference keep their address while
 * the array relocates; the array header carries the fill count. */
static config_widget_binding_t **binding_registry = NULL;

/*------------------------------------------------------------------------*/

/** config_widget_find - locate a field's registry entry
 * @field: address of a registered field
 *
 * Return: the binding, or NULL when @field was never registered.
 */
config_widget_binding_t *
config_widget_find(const void *field)
{
  int i;
  int count = mem_array_count(binding_registry);

  for( i = 0; i < count; i++ )
    if( binding_registry[i]->scope.dest.storage == field )
      return binding_registry[i];

  return NULL;
}

/*------------------------------------------------------------------------*/

/** config_widget_lookup - resolve a widget id on a live builder
 * @builder:   dereferenced, non-NULL builder
 * @widget_id: glade id
 *
 * A missing widget on a live builder is a build-time wiring error.
 * Return: the widget, or NULL when missing (BUG already raised).
 */
GtkWidget *
config_widget_lookup(GtkBuilder *builder, const char *widget_id)
{
  GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(builder, widget_id));

  if( w == NULL )
    BUG("config_widget_lookup: missing widget '%s'\n", widget_id);

  return w;
}

/*------------------------------------------------------------------------*/

void
config_widget_register(void *field, size_t size, const config_widget_tree_t *tree)
{
  config_widget_binding_t *binding = NULL;
  int count = mem_array_count(binding_registry);

  /* A readout projects through the same arm as a commit, so its bytes span
   * the destination exactly */
  if( (tree->readout != NULL) && (tree->readout->size != size) )
    BUG("a readout source spans %zu bytes against a field of %zu\n",
        tree->readout->size, size);

  mem_new(&binding);
  binding->tree               = tree;
  binding->scope.dest.storage = field;
  binding->scope.dest.size    = size;
  binding->scope.dest.context = field;
  binding->scope.commit       = config_widget_commit_field;

  /* A tree naming no refresh paints no frame, so its rows carry no staging
   * render and reach the interface on their commit alone */
  if( tree->post_apply == NULL )
    binding->scope.cls = REFRESH_COMMIT_ONLY;
  else
  {
    binding->scope.dest.refresh = config_widget_post_apply;
    binding->scope.cls          = tree->post_apply->cls;
  }

  mem_array_resize(&binding_registry, count + 1, NULL);
  binding_registry[count] = binding;
}

/*------------------------------------------------------------------------*/

/**
 * config_widget_cleanup() - Release the configuration binding registry
 */
void
config_widget_cleanup(void)
{
  int i;
  int count = mem_array_count(binding_registry);

  for( i = 0; i < count; i++ )
    mem_free(&binding_registry[i]);

  mem_array_free(&binding_registry);
}

/*------------------------------------------------------------------------*/

int
config_widget_binding_count(void)
{
  return mem_array_count(binding_registry);
}

config_widget_binding_t *
config_widget_binding_at(int index)
{
  return binding_registry[index];
}

/*------------------------------------------------------------------------*/

const config_widget_scope_t *
config_widget_field_scope(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);

  if( b == NULL )
  {
    BUG("a row binding named a field outside the configuration registry\n");
    return NULL;
  }

  return &b->scope;
}

/*------------------------------------------------------------------------*/

void
config_widget_post_apply(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);

  if( b == NULL )
  {
    BUG("a refresh named a field outside the configuration registry\n");
    return;
  }

  if( b->tree->post_apply == NULL )
    BUG("a refresh named a field whose binding declares none\n");
  else
    b->tree->post_apply->fn();
}

/*------------------------------------------------------------------------*/

void
config_widget_commit_field(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);

  if( b == NULL )
  {
    BUG("a commit named a field outside the configuration registry\n");
    return;
  }

  if( b->tree->post_apply != NULL )
    b->tree->post_apply->fn();

  if( b->tree->on_change != NULL )
    b->tree->on_change();

  config_widget_sync_field(field);
}

/*------------------------------------------------------------------------*/

void
config_widget_field_changed(void *field)
{
  /* The caller's value is the committed one now, so a hover staging into
   * this field has nothing left to restore */
  config_preview_discard(field);

  config_widget_commit_field(field);
}

/*------------------------------------------------------------------------*/

/** config_widget_row_outcome - carry one committed row to its side effects
 * @s:       the capabilities the row dispatched through
 * @changed: whether the write moved the stored bytes
 * @staged:  whether the commit first closed a hover session
 */
static void
config_widget_row_outcome(const config_widget_scope_t *s, gboolean changed,
    gboolean staged)
{
  /* Widget and field already agreed and no hover needs restoring */
  if( !changed && !staged )
    return;

  if( changed )
    s->commit(s->dest.context);
  else
    /* Closing the hover left the committed value on screen unpainted */
    s->dest.refresh(s->dest.context);
}

/*------------------------------------------------------------------------*/

void
on_config_widget_changed(GtkWidget *widget, gpointer user_data)
{
  const config_widget_row_t *row = config_widget_row_get(widget);
  const config_widget_scope_t *s;

  (void)user_data;

  /* A widget carrying no binding shares this handler with nothing to commit */
  if( row == NULL )
    return;

  s = row->scope;

  unsigned char committed[s->dest.size];
  unsigned char proposed[s->dest.size];
  gboolean staged;

  if( !config_widget_element_commit_value(row->elt, widget, proposed,
        s->dest.size) )
    return;

  /* Restore the committed bytes first, so the write below reads as the
   * transition the user made rather than a no-op against the value the
   * hover was already showing */
  staged = config_preview_prepare_commit(s->dest.storage);

  memcpy(committed, s->dest.storage, s->dest.size);
  memcpy(s->dest.storage, proposed, s->dest.size);

  config_widget_row_outcome(s,
      memcmp(committed, proposed, s->dest.size) != 0, staged);
}
