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
 * config_widget_element: per-widget read and write arm.
 *
 * Resolves one config_widget_element_t against one live GTK widget in
 * either direction: widget state into the field, and field value into the
 * widget.  Widget class selection lives here alone.
 */

#include "config_widget_priv.h"

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

/** config_widget_element_selects - whether an element names a selection value
 * @elt: element describing the row
 *
 * Return: TRUE when the row expresses one value, in either width.
 */
static gboolean
config_widget_element_selects(const config_widget_element_t *elt)
{
  return (elt->values != NULL) || (elt->value_bytes != NULL);
}

/*------------------------------------------------------------------------*/

gboolean
config_widget_element_select_value(const config_widget_element_t *elt,
    void *out, size_t size)
{
  if( !config_widget_element_selects(elt) )
    return FALSE;

  if( elt->value_bytes != NULL )
    memcpy(out, elt->value_bytes, size);
  else
    field_write_int(out, size, elt->values[0]);

  return TRUE;
}

/*------------------------------------------------------------------------*/

/** config_widget_element_commit_value - what committing a widget writes
 * @elt:  element describing the widget and its selection values
 * @w:    the resolved widget
 * @out:  buffer of @size bytes
 * @size: the selection's field width
 *
 * Arm order mirrors GTK's class hierarchy: radio menu item, then check menu
 * item, then plain menu item; radio button before its toggle-button base
 * class.  Menu radios, plain rows and combos name a selection; a valued
 * radio contributes only when active, a combo only when a row is selected.
 * A valued toggle writes values[0] when pressed and values[1] when released,
 * so an exclusive toggle group reads a shared off value with none pressed; a
 * value-less radio or toggle mirrors a 0/1 field in both directions.
 *
 * Return: FALSE when the widget contributes no value.
 */
gboolean
config_widget_element_commit_value(const config_widget_element_t *elt,
    GtkWidget *w, void *out, size_t size)
{
  gboolean writes = TRUE;

  if( GTK_IS_RADIO_MENU_ITEM(w) )
  {
    if( !config_widget_element_selects(elt) )
    {
      BUG("radio menu row '%s' names no selection value\n", elt->widget_id);
      writes = FALSE;
    }
    else if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)) )
      writes = config_widget_element_select_value(elt, out, size);
    else
      /* The active peer of this group carries the selection */
      writes = FALSE;
  }
  else if( GTK_IS_CHECK_MENU_ITEM(w) )
  {
    field_write_int(out, size,
        gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)) ? 1 : 0);
  }
  else if( GTK_IS_MENU_ITEM(w) )
  {
    /* A plain row commits the one selection it names when activated */
    if( config_widget_element_selects(elt) )
      writes = config_widget_element_select_value(elt, out, size);
    else
    {
      BUG("menu row '%s' names no selection value\n", elt->widget_id);
      writes = FALSE;
    }
  }
  else if( GTK_IS_RADIO_BUTTON(w) )
  {
    if( !config_widget_element_selects(elt) )
      /* Boolean group view: deactivation writes 0, so leaving the radio
       * for a sibling clears this view's field */
      field_write_int(out, size,
          gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) ? 1 : 0);
    else if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) )
      writes = config_widget_element_select_value(elt, out, size);
    else
      /* The active peer of this group carries the selection */
      writes = FALSE;
  }
  else if( GTK_IS_COMBO_BOX(w) )
  {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

    if( elt->values == NULL )
    {
      BUG("combo '%s' names no selection values\n", elt->widget_id);
      writes = FALSE;
    }
    else if( idx >= 0 )
      field_write_int(out, size, config_widget_values_at(elt->values, idx));
    else
      /* -1 = no selection; the field keeps its committed value */
      writes = FALSE;
  }
  else if( GTK_IS_SPIN_BUTTON(w) )
  {
    if( size == sizeof(double) )
      field_write_double(out, gtk_spin_button_get_value(GTK_SPIN_BUTTON(w)));
    else
      field_write_int(out, size,
          (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(w)));

    /* commit and reformat the entry text on spin commit */
    gtk_spin_button_update(GTK_SPIN_BUTTON(w));
  }
  else if( GTK_IS_RANGE(w) )
  {
    if( size == sizeof(double) )
      field_write_double(out, gtk_range_get_value(GTK_RANGE(w)));
    else
      field_write_float(out, (float)gtk_range_get_value(GTK_RANGE(w)));
  }
  else if( GTK_IS_TOGGLE_BUTTON(w) )
  {
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    if( elt->values == NULL )
      field_write_int(out, size, active ? 1 : 0);
    else
      /* Exclusive two-value toggle: values[0] is written when pressed and
       * values[1], the group's shared released value, when the user clears
       * the active member, so the group reads the released value with none
       * pressed */
      field_write_int(out, size, active ? elt->values[0] : elt->values[1]);
  }
  else
  {
    BUG("no commit value for the widget class of '%s'\n", elt->widget_id);
    writes = FALSE;
  }

  return writes;
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
void
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
