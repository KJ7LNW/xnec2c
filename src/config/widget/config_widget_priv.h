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

/* One binding presented on one live builder.  The builder comes from the
 * declaration at use, so a view stays good across a window that closes and
 * opens again. */
typedef struct {
  const config_widget_binding_t *binding;
  const config_widget_group_t   *declaration;
} config_widget_group_view_t;

/* One widget an operation reaches, normalized across a row the tree
 * declares and a row a selector built, so a single presentation and a
 * shell full of them read alike. */
typedef struct {
  GtkWidget                     *widget;
  const config_widget_element_t *element;
  const config_widget_scope_t   *scope;
} config_widget_target_t;

/* What an operation does at one normalized target. */
typedef void (*config_widget_visit_fn)(const config_widget_target_t *t,
    const void *operand);

/* What an operation does at one live group.  Every operation body reads
 * this one signature, so nothing chooses an implementation by inspecting
 * its operand. */
typedef void (*config_widget_group_fn)(const config_widget_group_view_t *gv,
    const void *operand);

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

/** config_widget_find - locate a field's registry entry
 * @field: address of a registered field
 *
 * Return: the binding, or NULL when @field was never registered.
 */
config_widget_binding_t *config_widget_find(const void *field);

/** config_widget_scope_source - the bytes a binding's commit writes
 * @b: any registered binding
 *
 * Return: the destination read as a projection source, so a committed sync
 * and a passive readout differ only by which source they name.
 */
static inline config_widget_source_t
config_widget_scope_source(const config_widget_binding_t *b)
{
  return (config_widget_source_t){ .storage = b->scope.dest.storage,
                                   .size    = b->scope.dest.size };
}

/** config_widget_walk_groups - visit every live group one binding presents
 * @b:       any registered binding
 * @visit:   what the operation does at each group
 * @operand: carried through to @visit unread
 */
void config_widget_walk_groups(const config_widget_binding_t *b,
    config_widget_group_fn visit, const void *operand);

/** config_widget_walk_members - visit every widget one group presents a field on
 * @gv:      a binding and one of its groups, whose builder is live
 * @visit:   what the operation does at each widget
 * @operand: carried through to @visit unread
 *
 * Declared rows resolve from the builder and receive their binding as they
 * resolve; rows a selector built carry their own element and are told from
 * the rows of other fields sharing their shell.  Both reach @visit as the
 * same target, so one presentation and a shell full of them read alike.
 */
void config_widget_walk_members(const config_widget_group_view_t *gv,
    config_widget_visit_fn visit, const void *operand);

/** config_widget_group_project - write one source's bytes into one live group
 * @gv:      a binding and one of its groups, whose builder is live
 * @operand: the config_widget_source_t naming the bytes and their extent
 *
 * The group is silenced across the whole write, so a radio deactivation the
 * write raises reaches no live handler.  The collapsed label then names the
 * committed selection.  A committed sync and a passive readout reach this
 * one body and differ only by the source they name.
 */
void config_widget_group_project(const config_widget_group_view_t *gv,
    const void *operand);

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

#endif /* CONFIG_WIDGET_PRIV_H */
