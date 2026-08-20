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
 * config_widget: row-rooted config-widget binding engine.
 *
 * Application-agnostic registry, apply/sync arms, and peer-broadcast logic
 * for the config_widget_tree_t hierarchy declared in config_widget.h.
 */

#include "config_widget.h"
#include "../mem/mem.h"

/* Initial capacity for the amortized-growth binding registry. */
#define CONFIG_WIDGET_REGISTRY_INITIAL_CAP 64

/*------------------------------------------------------------------------*/

/* Size-aware field accessors.  Fields may be typed as enums whose storage
 * width is implementation-defined; these helpers read/write through the
 * field's actual width via memcpy, avoiding aliasing and width mismatches
 * from direct pointer casts. */

static inline int
field_read_int(const void *field, size_t size)
{
  int val = 0;
  memcpy(&val, field, size);
  return val;
}

static inline void
field_write_int(void *field, size_t size, int val)
{
  memcpy(field, &val, size);
}

static inline float
field_read_float(const void *field)
{
  float val;
  memcpy(&val, field, sizeof(float));
  return val;
}

static inline void
field_write_float(void *field, float val)
{
  memcpy(field, &val, sizeof(float));
}

static inline double
field_read_double(const void *field)
{
  double val;
  memcpy(&val, field, sizeof(double));
  return val;
}

static inline void
field_write_double(void *field, double val)
{
  memcpy(field, &val, sizeof(double));
}

/*------------------------------------------------------------------------*/

/* Engine-private registry entry.  field is the identity key for peer
 * broadcast; tree is the row's complete widget binding. */
typedef struct {
  void *field;
  size_t size;
  const config_widget_tree_t *tree;
} config_widget_binding_t;

static config_widget_binding_t *binding_registry = NULL;
static int binding_count = 0;

/*------------------------------------------------------------------------*/

/** config_widget_find - locate a field's registry entry
 * @field: address of a registered field
 *
 * Return: the binding, or NULL when @field was never registered.
 */
static config_widget_binding_t *
config_widget_find(void *field)
{
  int i;

  for( i = 0; i < binding_count; i++ )
    if( binding_registry[i].field == field )
      return &binding_registry[i];

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
static GtkWidget *
config_widget_lookup(GtkBuilder *builder, const char *widget_id)
{
  GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(builder, widget_id));

  if( w == NULL )
    BUG("config_widget_lookup: missing widget '%s'\n", widget_id);

  return w;
}

/*------------------------------------------------------------------------*/

/** config_widget_values_at - resolve a selection index to its field value
 * @values: NULL-terminated (CONFIG_WIDGET_VALUES_END) selection list
 * @index:  widget's active selection index
 *
 * Return: the field value at @index; BUGs and returns 0 when the
 * terminator is reached before @index.
 */
static int
config_widget_values_at(const int *values, int index)
{
  int i;

  for( i = 0; i <= index; i++ )
    if( values[i] == CONFIG_WIDGET_VALUES_END )
    {
      BUG("config_widget_values_at: index %d exceeds values list\n", index);
      return 0;
    }

  return values[index];
}

/*------------------------------------------------------------------------*/

/** config_widget_apply_element - read one widget's state into the field
 * @field: field address
 * @size:  field width
 * @elt:   element describing the widget and its selection values
 * @w:     the resolved widget
 *
 * Arm order mirrors GTK's class hierarchy: radio menu item and radio
 * button are checked before their plain check-menu-item/toggle-button
 * base classes.  Menu radios and combos require .values; a valued radio
 * writes only when active, a combo only when a row is selected.  A valued
 * toggle writes values[0] when pressed and values[1] when released, so an
 * exclusive toggle group reads a shared off value with none pressed; a
 * value-less radio or toggle mirrors a 0/1 field in both directions.
 */
static void
config_widget_apply_element(void *field, size_t size,
    const config_widget_element_t *elt, GtkWidget *w)
{
  if( GTK_IS_RADIO_MENU_ITEM(w) )
  {
    if( elt->values == NULL )
      BUG("config_widget_apply_element: radio menu item '%s' has no values\n",
          elt->widget_id);
    else if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)) )
      field_write_int(field, size, elt->values[0]);
  }
  else if( GTK_IS_CHECK_MENU_ITEM(w) )
  {
    field_write_int(field, size,
        gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)) ? 1 : 0);
  }
  else if( GTK_IS_RADIO_BUTTON(w) )
  {
    if( elt->values == NULL )
      /* Boolean group view: deactivation writes 0, so leaving the radio
       * for a sibling clears this view's field */
      field_write_int(field, size,
          gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) ? 1 : 0);
    else if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) )
      field_write_int(field, size, elt->values[0]);
  }
  else if( GTK_IS_COMBO_BOX(w) )
  {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    if( elt->values == NULL )
      BUG("config_widget_apply_element: combo '%s' has no values\n", elt->widget_id);
    else if( idx >= 0 ) /* -1 = no selection; leave the field unchanged */
      field_write_int(field, size, config_widget_values_at(elt->values, idx));
  }
  else if( GTK_IS_SPIN_BUTTON(w) )
  {
    if( size == sizeof(double) )
      field_write_double(field, gtk_spin_button_get_value(GTK_SPIN_BUTTON(w)));
    else
      field_write_int(field, size,
          (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(w)));

    /* commit and reformat the entry text on spin commit */
    gtk_spin_button_update(GTK_SPIN_BUTTON(w));
  }
  else if( GTK_IS_RANGE(w) )
  {
    if( size == sizeof(double) )
      field_write_double(field, gtk_range_get_value(GTK_RANGE(w)));
    else
      field_write_float(field, (float)gtk_range_get_value(GTK_RANGE(w)));
  }
  else if( GTK_IS_TOGGLE_BUTTON(w) )
  {
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    if( elt->values == NULL )
      field_write_int(field, size, active ? 1 : 0);
    else
      /* Exclusive two-value toggle: values[0] is written when pressed and
       * values[1], the group's shared released value, when the user clears
       * the active member, so the group reads the released value with none
       * pressed */
      field_write_int(field, size, active ? elt->values[0] : elt->values[1]);
  }
  else
    BUG("config_widget_apply_element: unknown widget type for '%s'\n", elt->widget_id);
}

/*------------------------------------------------------------------------*/

/** config_widget_sync_element - write the field's value into one widget
 * @field: field address
 * @size:  field width
 * @elt:   element describing the widget and its selection values
 * @w:     the resolved widget
 *
 * Combo sync scans .values for the field's current value; a miss clears
 * the selection (index -1) rather than leaving a stale row active.
 */
static void
config_widget_sync_element(const void *field, size_t size,
    const config_widget_element_t *elt, GtkWidget *w)
{
  if( GTK_IS_RADIO_MENU_ITEM(w) )
  {
    if( elt->values == NULL )
      BUG("config_widget_sync_element: radio menu item '%s' has no values\n",
          elt->widget_id);
    else if( field_read_int(field, size) == elt->values[0] )
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), TRUE);
  }
  else if( GTK_IS_CHECK_MENU_ITEM(w) )
  {
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),
        field_read_int(field, size));
  }
  else if( GTK_IS_RADIO_BUTTON(w) )
  {
    if( elt->values == NULL )
      /* Boolean group view mirrors the 0/1 field; a zero leaves the group
       * with no active member */
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
          field_read_int(field, size));
    else if( field_read_int(field, size) == elt->values[0] )
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), TRUE);
  }
  else if( GTK_IS_COMBO_BOX(w) )
  {
    if( elt->values == NULL )
      BUG("config_widget_sync_element: combo '%s' has no values\n", elt->widget_id);
    else
    {
      int val = field_read_int(field, size);
      int idx = -1;
      int i;

      for( i = 0; elt->values[i] != CONFIG_WIDGET_VALUES_END; i++ )
        if( elt->values[i] == val )
        {
          idx = i;
          break;
        }

      gtk_combo_box_set_active(GTK_COMBO_BOX(w), idx);
    }
  }
  else if( GTK_IS_SPIN_BUTTON(w) )
  {
    if( size == sizeof(double) )
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), field_read_double(field));
    else
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
          (gdouble)field_read_int(field, size));
  }
  else if( GTK_IS_RANGE(w) )
  {
    if( size == sizeof(double) )
      gtk_range_set_value(GTK_RANGE(w), field_read_double(field));
    else
      gtk_range_set_value(GTK_RANGE(w), (gdouble)field_read_float(field));
  }
  else if( GTK_IS_TOGGLE_BUTTON(w) )
  {
    if( elt->values == NULL )
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
          field_read_int(field, size));
    else
      /* Pressed only while the field holds this toggle's on value; all
       * members read the shared released value alike */
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
          field_read_int(field, size) == elt->values[0]);
  }
  else
    BUG("config_widget_sync_element: unknown widget type for '%s'\n", elt->widget_id);
}

/*------------------------------------------------------------------------*/

void
config_widget_register(void *field, size_t size, const config_widget_tree_t *tree)
{
  mem_array_reserve(&binding_registry, binding_count + 1,
      CONFIG_WIDGET_REGISTRY_INITIAL_CAP);

  binding_registry[binding_count].field = field;
  binding_registry[binding_count].size  = size;
  binding_registry[binding_count].tree  = tree;
  binding_count++;
}

/*------------------------------------------------------------------------*/

/**
 * config_widget_cleanup() - Release the configuration binding registry
 */
void
config_widget_cleanup(void)
{
  mem_array_free(&binding_registry);
  binding_count = 0;
}

/*------------------------------------------------------------------------*/

/** config_widget_sync_group - write a binding's field into one live group
 * @b:       the binding whose field value is broadcast
 * @builder: the group's resolved, non-NULL builder
 * @g:       a group of @b bound to @builder
 *
 * Each write is wrapped in SIGNAL_BLOCK/SIGNAL_UNBLOCK so the programmatic
 * update never re-enters on_config_widget_changed.
 */
static void
config_widget_sync_group(const config_widget_binding_t *b,
    GtkBuilder *builder, const config_widget_group_t *g)
{
  const config_widget_element_t *const *e;

  for( e = g->elements; *e != NULL; e++ )
  {
    GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);

    if( w == NULL )
      continue;

    SIGNAL_BLOCK(w, on_config_widget_changed);
    config_widget_sync_element(b->field, b->size, *e, w);
    SIGNAL_UNBLOCK(w, on_config_widget_changed);
  }
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
config_widget_field_changed(void *field)
{
  config_widget_binding_t *b = config_widget_find(field);

  if( b == NULL )
  {
    BUG("config_widget_field_changed: field is not registered\n");
    return;
  }

  if( b->tree->post_apply != NULL )
    b->tree->post_apply();

  if( b->tree->on_change != NULL )
    b->tree->on_change();

  config_widget_sync_field(field);
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_builder(GtkBuilder **builder)
{
  int i;

  if( builder == NULL || *builder == NULL )
    return;

  for( i = 0; i < binding_count; i++ )
  {
    config_widget_binding_t *b = &binding_registry[i];
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

  for( i = 0; i < binding_count; i++ )
  {
    config_widget_binding_t *b = &binding_registry[i];
    const config_widget_group_t *const *g;

    if( b->tree->post_apply == NULL )
      continue;

    for( g = b->tree->groups; *g != NULL; g++ )
      if( (*g)->builder == builder )
      {
        b->tree->post_apply();
        break; /* run once even if the tree has other groups on this builder */
      }
  }
}

/*------------------------------------------------------------------------*/

void
config_widget_sync_all(void)
{
  int i;

  for( i = 0; i < binding_count; i++ )
    config_widget_sync_field(binding_registry[i].field);
}

/*------------------------------------------------------------------------*/

void
on_config_widget_changed(GtkWidget *widget, gpointer user_data)
{
  int i;

  (void)user_data;

  for( i = 0; i < binding_count; i++ )
  {
    config_widget_binding_t *b = &binding_registry[i];
    const config_widget_group_t *const *g;

    for( g = b->tree->groups; *g != NULL; g++ )
    {
      const config_widget_element_t *const *e;
      GtkBuilder *builder;

      if( (*g)->builder == NULL || (builder = *(*g)->builder) == NULL )
        continue;

      for( e = (*g)->elements; *e != NULL; e++ )
      {
        GtkWidget *w = config_widget_lookup(builder, (*e)->widget_id);
        char old_val[sizeof(double)];

        if( w != widget )
          continue;

        memcpy(old_val, b->field, b->size);
        config_widget_apply_element(b->field, b->size, *e, w);

        if( memcmp(old_val, b->field, b->size) != 0 )
          config_widget_field_changed(b->field);

        return;
      }
    }
  }
}
