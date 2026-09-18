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

#ifndef CONFIG_WIDGET_PRIV_H
#define CONFIG_WIDGET_PRIV_H 1

#include <string.h>
#include "config_widget.h"

/*
 * config_widget engine internals shared by the registry, the projection
 * arm, and the per-widget read/write arm.  Nothing here is part of the
 * binding API declared in config_widget.h.
 */

/*------------------------------------------------------------------------*/

/* Engine-private registry entry.  tree is the row's complete widget
 * binding; the embedded scope carries the field address peer broadcast keys
 * on, that field's width, and the capability record every row of the field
 * dispatches through.  The registry owns each entry by pointer, so the
 * scope address a widget retains survives the array relocating as it
 * grows. */
typedef struct {
  const config_widget_tree_t *tree;
  config_widget_scope_t scope;
} config_widget_binding_t;

/** config_widget_binding_count - how many bindings the registry holds
 *
 * Return: the bound every binding walk stops below.
 */
int config_widget_binding_count(void);

/** config_widget_binding_at - the binding at one walk position
 * @index: a position below config_widget_binding_count()
 *
 * Return: the binding, which the registry continues to own.
 */
config_widget_binding_t *config_widget_binding_at(int index);

/*------------------------------------------------------------------------*/

/* Size-aware field accessors.  Fields may be typed as enums whose storage
 * width is implementation-defined; these helpers read/write through the
 * field's actual width via memcpy, avoiding aliasing and width mismatches
 * from direct pointer casts. */

/** field_read_int - read an int-compatible field at its actual width */
static inline int
field_read_int(const void *field, size_t size)
{
  int val = 0;
  memcpy(&val, field, size);
  return val;
}

/** field_write_int - write an int-compatible field at its actual width */
static inline void
field_write_int(void *field, size_t size, int val)
{
  memcpy(field, &val, size);
}

/** field_read_float - read a float field without aliasing */
static inline float
field_read_float(const void *field)
{
  float val;
  memcpy(&val, field, sizeof(float));
  return val;
}

/** field_write_float - write a float field without aliasing */
static inline void
field_write_float(void *field, float val)
{
  memcpy(field, &val, sizeof(float));
}

/** field_read_double - read a double field without aliasing */
static inline double
field_read_double(const void *field)
{
  double val;
  memcpy(&val, field, sizeof(double));
  return val;
}

/** field_write_double - write a double field without aliasing */
static inline void
field_write_double(void *field, double val)
{
  memcpy(field, &val, sizeof(double));
}

/*------------------------------------------------------------------------*/

/** config_widget_find - locate a field's registry entry
 * @field: address of a registered field
 *
 * Return: the binding, or NULL when @field was never registered.
 */
config_widget_binding_t *config_widget_find(void *field);

/** config_widget_lookup - resolve a widget id on a live builder
 * @builder:   dereferenced, non-NULL builder
 * @widget_id: glade id
 *
 * Return: the widget, or NULL when missing (BUG already raised).
 */
GtkWidget *config_widget_lookup(GtkBuilder *builder, const char *widget_id);

/** config_widget_group_block - silence the commit handler across a group */
void config_widget_group_block(GtkBuilder *builder,
    const config_widget_group_t *g);

/** config_widget_group_unblock - restore the commit handler across a group */
void config_widget_group_unblock(GtkBuilder *builder,
    const config_widget_group_t *g);

/** config_widget_menu_shell - the runtime row container a group names
 * @builder: the group's resolved, non-NULL builder
 * @g:       the group, which names a menu only when a selector fills it
 *
 * Return: the menu holding rows built at runtime, or NULL when the group
 * declares every row it presents.
 */
GtkWidget *config_widget_menu_shell(GtkBuilder *builder,
    const config_widget_group_t *g);

/** config_widget_menu_element - the element a menu child carries for one binding
 * @b:     the binding whose rows are wanted
 * @child: one child of a group's runtime menu
 *
 * Return: the child's element, or NULL when the child is not a row of @b.
 */
const config_widget_element_t *config_widget_menu_element(
    const config_widget_binding_t *b, GtkWidget *child);

/** config_widget_menu_block - silence the commit handler across a runtime menu */
void config_widget_menu_block(GtkWidget *menu);

/** config_widget_menu_unblock - restore the commit handler across a runtime menu */
void config_widget_menu_unblock(GtkWidget *menu);

/** config_widget_label_sync - name the active row on a collapsed pull-down
 * @b:       the binding whose value selects the row
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b optionally naming a value label
 * @menu:    the group's resolved runtime row container, or NULL for a group
 *           declaring its rows
 */
void config_widget_label_sync(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g, GtkWidget *menu);

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

#endif /* CONFIG_WIDGET_PRIV_H */
