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
 * config_widget_element_sync: per-widget write arm.
 *
 * Projects one field value onto one live GTK widget.  The opposite
 * direction, what a widget expresses back to its field, lives in
 * config_widget_element.c.
 */

#include "config_widget_priv.h"

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
