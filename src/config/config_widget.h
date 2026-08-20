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

#ifndef CONFIG_WIDGET_H
#define CONFIG_WIDGET_H 1

#include <limits.h>
#include <gtk/gtk.h>
#include "../console.h"

/*
 * config_widget: row-rooted config-widget binding engine.
 *
 * Application-agnostic projection of a field's value onto one or more GTK
 * widgets across one or more window builders.  Hierarchy: field -> tree ->
 * groups (one per builder) -> elements (one per widget) -> values (radio
 * and combo selection lists).  Cross-widget identity is the field address;
 * peers re-sync live on any write under SIGNAL_BLOCK/SIGNAL_UNBLOCK.
 */

/*------------------------------------------------------------------------*/

/* 0 is a live selection value, so INT_MIN (never a real value list member)
 * terminates a selection-value list instead of NULL. */
#define CONFIG_WIDGET_VALUES_END INT_MIN

/* One bound widget: its glade id and, for radios/combos/valued toggles, the
 * field values its selection positions express.  A valued toggle carries two
 * entries: [0] written when pressed, [1] the released state.  values == NULL
 * means pass-through (plain toggle, check menu item, spin, range). */
typedef struct {
  const char *widget_id;
  const int  *values;
} config_widget_element_t;

/* One builder's set of widgets bound to a field.  builder is dereferenced
 * at use; *builder == NULL means the window is not built yet and the
 * group is dormant. */
typedef struct {
  GtkBuilder **builder;
  const config_widget_element_t *const *elements;
} config_widget_group_t;

/* One field's complete binding: two optional hooks and every builder group
 * projecting the field.  post_apply is an idempotent refresh fired on every
 * change and on every bulk sync/create pass; on_change is a transition-edge
 * effect (non-idempotent or heavyweight) fired only on a real value change. */
typedef struct {
  void (*post_apply)(void);
  void (*on_change)(void);
  const config_widget_group_t *const *groups;
} config_widget_tree_t;

/*------------------------------------------------------------------------*/

/* Constructor macros: structure-only (type spelling, &, braces); all field
 * data is supplied by the caller as designated assignments, and every
 * NULL-terminator is written explicitly by the caller. */

#define CONFIG_WIDGET_TREE(...)   &(const config_widget_tree_t){ __VA_ARGS__ }
#define CONFIG_WIDGET_GROUPS(...) (const config_widget_group_t *const[]){ __VA_ARGS__ }
#define CONFIG_WIDGET_GROUP(...)  &(const config_widget_group_t){ __VA_ARGS__ }
#define CONFIG_WIDGETS(...)       (const config_widget_element_t *const[]){ __VA_ARGS__ }
#define CONFIG_WIDGET(...)        &(const config_widget_element_t){ __VA_ARGS__ }
#define CONFIG_WIDGET_VALUES(...) (const int[]){ __VA_ARGS__, CONFIG_WIDGET_VALUES_END }

/* Degenerate tree: one builder, one widget, pass-through value. */
#define CONFIG_WIDGET_SINGLE(bldr, id, hook) \
  CONFIG_WIDGET_TREE( .post_apply = (hook), \
    .groups = CONFIG_WIDGET_GROUPS( CONFIG_WIDGET_GROUP( .builder = (bldr), \
      .elements = CONFIG_WIDGETS( CONFIG_WIDGET( .widget_id = (id) ), NULL ) ), NULL ) )

/*------------------------------------------------------------------------*/

/* Compile-time field-width checks, application-agnostic (take a full
 * lvalue rather than an owning-struct field name). */

#define CONFIG_FIELD_INT_ASSERT(lval) \
  _Static_assert(sizeof(lval) == sizeof(int), #lval " is not int-width")

#define CONFIG_FIELD_FLT_ASSERT(lval) \
  _Static_assert(_Generic((lval), float: 1, default: 0), #lval " is not float")

#define CONFIG_FIELD_DBL_ASSERT(lval) \
  _Static_assert(_Generic((lval), double: 1, default: 0), #lval " is not double")

/*------------------------------------------------------------------------*/

/* Programmatic-write signal suppression.  ISO C forbids casting a function
 * pointer to gpointer (void *) directly; a union reinterpret is defined
 * behavior under C99/C11 and avoids that cast.
 * g_signal_handlers_block_by_func is bypassed in favor of the underlying
 * g_signal_handlers_block_matched to keep the cast site visible and
 * explicit. */
#define SIGNAL_BLOCK(widget, func) \
  do { \
    union { GCallback cb; gpointer p; } _u = { .cb = (GCallback)(func) }; \
    g_signal_handlers_block_matched((widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, _u.p, NULL); \
  } while(0)

#define SIGNAL_UNBLOCK(widget, func) \
  do { \
    union { GCallback cb; gpointer p; } _u = { .cb = (GCallback)(func) }; \
    g_signal_handlers_unblock_matched((widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, _u.p, NULL); \
  } while(0)

/*------------------------------------------------------------------------*/

/** config_widget_register - bind a field to its widget tree
 * @field: address of the field the tree projects
 * @size:  field width in bytes (4 = int/float, 8 = double)
 * @tree:  the field's complete widget binding
 */
void config_widget_register(void *field, size_t size,
                            const config_widget_tree_t *tree);
void config_widget_cleanup(void);

/** config_widget_sync_field - write a field's value into every live peer widget
 * @field: address of a registered field
 *
 * Each write is wrapped in SIGNAL_BLOCK/SIGNAL_UNBLOCK; no hook is invoked.
 */
void config_widget_sync_field(void *field);

/** config_widget_field_changed - run a field's change-edge hook, then sync peers
 * @field: address of a registered field
 *
 * For programmatic callers that mutate a field directly and must trigger
 * the same side effects and peer sync as a widget-driven change.
 */
void config_widget_field_changed(void *field);

/** config_widget_sync_builder - write every field's value into one builder's widgets
 * @builder: address of the builder pointer (eg &main_window_builder)
 *
 * Called at window create/show time; a NULL *builder is a no-op.
 */
void config_widget_sync_builder(GtkBuilder **builder);

/** config_widget_run_hooks - invoke the post_apply hook of every tree bound to a builder
 * @builder: address of the builder pointer (eg &main_window_builder)
 *
 * Each tree's hook runs at most once, even when the tree has multiple
 * groups on the same builder.
 */
void config_widget_run_hooks(GtkBuilder **builder);

/** config_widget_sync_all - write every registered field's value into its widgets */
void config_widget_sync_all(void);

/** on_config_widget_changed - unified glade signal handler for bound widgets
 * @widget:    the widget that fired the signal
 * @user_data: unused
 *
 * Locates the (field, element) whose resolved widget equals @widget across
 * every registered tree, applies the widget's state into the field, and on
 * change runs the field's hook followed by a peer sync.
 */
void on_config_widget_changed(GtkWidget *widget, gpointer user_data);

#endif /* CONFIG_WIDGET_H */
