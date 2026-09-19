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
 * config_widget_row: attachment and lookup of one widget's binding.
 *
 * Carries a scope and an element on the widget that expresses them, so the
 * commit path resolves a widget's selection directly instead of searching
 * the registry for it.  A selector building its rows at runtime names the
 * candidate a valued row proposes; a boolean row derives its outcome from
 * widget state.  The element describing either form is built and owned here.
 */

#include <string.h>
#include "config_widget_row.h"
#include "config_widget_element.h"
#include "config_widget_priv.h"
#include "../config_preview_row.h"
#include "../../mem/mem.h"

/* Object-data key carrying a widget's binding row. */
#define CONFIG_WIDGET_DATA_ROW  "config-widget-row"

/* A runtime-built row: its record, the element describing it, and the
 * candidate bytes that element names, in one allocation, so the selection a
 * built row expresses dies with the widget expressing it.  row leads the
 * structure, so one release covers every kind of row. */
typedef struct {
  config_widget_row_t     row;
  config_widget_element_t elt;
  unsigned char           bytes[];
} config_widget_row_built_t;

/*------------------------------------------------------------------------*/

/** config_widget_row_free - release the binding of a departing widget
 * @data: the record GTK drops with its widget
 */
static void
config_widget_row_free(gpointer data)
{
  mem_free(&data);
}

/*------------------------------------------------------------------------*/

const config_widget_row_t *
config_widget_row_get(GtkWidget *w)
{
  return g_object_get_data(G_OBJECT(w), CONFIG_WIDGET_DATA_ROW);
}

/*------------------------------------------------------------------------*/

/** config_widget_scope_usable - whether a scope can carry rows
 * @scope: the capabilities a row would dispatch through
 *
 * Return: TRUE when the scope names bytes, a width and a commit.
 */
static gboolean
config_widget_scope_usable(const config_widget_scope_t *scope)
{
  return (scope != NULL) && (scope->dest.storage != NULL) &&
         (scope->dest.size != 0) && (scope->commit != NULL);
}

/** config_widget_row_present - carry a filled record onto its widget
 * @w:   the widget the record describes
 * @rec: the record, already naming its scope and element
 *
 * The hover edges read the record, so they are wired once it is readable.
 */
static void
config_widget_row_present(GtkWidget *w, config_widget_row_t *rec)
{
  g_object_set_data_full(G_OBJECT(w), CONFIG_WIDGET_DATA_ROW, rec,
      config_widget_row_free);

  /* A class carrying no row-hover edge runs nothing under the pointer */
  if( !config_preview_class_hoverable(w) )
    return;

  switch( rec->scope->cls )
  {
    case REFRESH_HOVER_SAFE:
      /* Require a scope render to paint each hover candidate */
      if( rec->scope->dest.refresh == NULL )
        BUG("hover preview reached row '%s' whose scope names no refresh\n",
            rec->elt->widget_id);
      else
        config_preview_row_attach(w);
      break;

    case REFRESH_COMMIT_ONLY:
      break;

    case REFRESH_CLASS_UNSET:
    case REFRESH_CLASS_COUNT:
      BUG("hover preview reached row '%s' whose refresh names no hover"
          " classification\n", rec->elt->widget_id);
      break;
  }
}

/*------------------------------------------------------------------------*/

void
config_widget_row_attach(GtkWidget *w, const config_widget_scope_t *scope,
    const config_widget_element_t *elt)
{
  config_widget_row_t *rec = NULL;

  if( (w == NULL) || (elt == NULL) || !config_widget_scope_usable(scope) )
  {
    BUG("a widget binding named no widget, element, or usable selection\n");
    return;
  }

  /* Re-resolving a group revisits rows it already bound */
  if( config_widget_row_get(w) != NULL )
    return;

  mem_new(&rec);
  rec->scope = scope;
  rec->elt   = elt;

  config_widget_row_present(w, rec);
}

/*------------------------------------------------------------------------*/

/** config_widget_row_connect - fire the commit path on a runtime row's edge
 * @w: a row no builder file connects
 *
 * A builder connects the commit handler of the rows it declares; a row built
 * at runtime carries no builder, so its own class names the edge.
 */
static void
config_widget_row_connect(GtkWidget *w)
{
  if( GTK_IS_MENU_ITEM(w) )
    g_signal_connect(w, "activate", G_CALLBACK(on_config_widget_changed), NULL);
  else if( GTK_IS_TOGGLE_BUTTON(w) )
    g_signal_connect(w, "toggled", G_CALLBACK(on_config_widget_changed), NULL);
  else
    BUG("a runtime row named a widget class carrying no commit edge\n");
}

/*------------------------------------------------------------------------*/

void
config_widget_bind_scoped(GtkWidget *w, const config_widget_scope_t *scope,
    const void *candidate)
{
  config_widget_row_built_t *built = NULL;

  if( (w == NULL) || !config_widget_scope_usable(scope) )
  {
    BUG("a runtime row named no widget or usable selection\n");
    return;
  }

  /* Binding one row twice would stage and commit it twice on one edge */
  if( config_widget_row_get(w) != NULL )
    return;

  if( candidate == NULL )
  {
    /* A row naming no bytes leaves its outcome to the widget's own state,
     * which only the classes answering a value-less element express */
    const config_widget_element_t state_derived_element = { 0 };
    unsigned char candidate_probe[scope->dest.size];

    if( !config_widget_element_candidate(&state_derived_element, w,
          candidate_probe, scope->dest.size) )
    {
      BUG("a runtime row named no candidate and no state-derived outcome\n");
      return;
    }

    mem_alloc(&built, sizeof(*built));
  }
  else
  {
    mem_alloc(&built, sizeof(*built) + scope->dest.size);
    memcpy(built->bytes, candidate, scope->dest.size);
    built->elt.value_bytes = built->bytes;
  }

  built->row.scope = scope;
  built->row.elt   = &built->elt;

  config_widget_row_connect(w);
  config_widget_row_present(w, &built->row);
}

/*------------------------------------------------------------------------*/

void
config_widget_bind_row(GtkWidget *w, void *field, const void *candidate)
{
  config_widget_bind_scoped(w, config_widget_field_scope(field), candidate);
}
