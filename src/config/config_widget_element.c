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
 * config_widget_element: per-widget input arm.
 *
 * Resolves what clicking one live GTK widget writes into its field, both as
 * a pre-click candidate and as a post-click commit.  Projection of a field
 * back onto its widget lives in config_widget_element_sync.c.
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
gboolean
config_widget_element_selects(const config_widget_element_t *elt)
{
  return (elt->values != NULL) || (elt->value_bytes != NULL);
}

/*------------------------------------------------------------------------*/

/** config_widget_element_named_value - copy the value a row names
 * @elt:  element describing the row
 * @out:  buffer of @size bytes
 * @size: the selection's field width
 */
static void
config_widget_element_named_value(const config_widget_element_t *elt,
    void *out, size_t size)
{
  if( elt->value_bytes != NULL )
    memcpy(out, elt->value_bytes, size);
  else
    field_write_int(out, size, elt->values[0]);
}

/*------------------------------------------------------------------------*/

/** config_widget_element_holds - whether a field holds a row's selection
 * @field: field address
 * @size:  the selection's field width
 * @elt:   element describing the row
 *
 * Compares the field against the value the row names, at the field's full
 * width, so a name-valued row is matched over the whole field rather than
 * through an int window and a match equals the commit that row would write.
 *
 * Return: TRUE when the stored value equals the value @elt names.
 */
gboolean
config_widget_element_holds(const void *field, size_t size,
    const config_widget_element_t *elt)
{
  /* A row naming no selection expresses its own state, not a value */
  if( !config_widget_element_selects(elt) )
    return FALSE;

  unsigned char named[size];

  config_widget_element_named_value(elt, named, size);

  return memcmp(field, named, size) == 0;
}

/*------------------------------------------------------------------------*/

/** config_widget_element_candidate - predict what clicking a widget writes
 * @elt:  element describing the widget and its selection values
 * @w:    widget before its click edge
 * @out:  buffer of @size bytes
 * @size: the selection's field width
 *
 * A row naming a value proposes it; a toggling row proposes the value its
 * click produces, so a candidate equals the commit that follows it.
 *
 * Return: FALSE when the widget exposes no discrete click outcome.
 */
gboolean
config_widget_element_candidate(const config_widget_element_t *elt,
    GtkWidget *w, void *out, size_t size)
{
  gboolean writes = TRUE;

  if( GTK_IS_RADIO_MENU_ITEM(w) )
  {
    if( config_widget_element_selects(elt) )
      config_widget_element_named_value(elt, out, size);
    else
      writes = FALSE;
  }
  else if( GTK_IS_CHECK_MENU_ITEM(w) )
  {
    field_write_int(out, size,
        gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)) ? 0 : 1);
  }
  else if( GTK_IS_MENU_ITEM(w) )
  {
    if( config_widget_element_selects(elt) )
      config_widget_element_named_value(elt, out, size);
    else
      writes = FALSE;
  }
  else if( GTK_IS_RADIO_BUTTON(w) )
  {
    if( config_widget_element_selects(elt) )
      config_widget_element_named_value(elt, out, size);
    else
      field_write_int(out, size,
          gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) ? 0 : 1);
  }
  else if( GTK_IS_TOGGLE_BUTTON(w) )
  {
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    if( elt->values == NULL )
      field_write_int(out, size, active ? 0 : 1);
    else
      field_write_int(out, size, active ? elt->values[1] : elt->values[0]);
  }
  else
    writes = FALSE;

  return writes;
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
      config_widget_element_named_value(elt, out, size);
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
      config_widget_element_named_value(elt, out, size);
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
      config_widget_element_named_value(elt, out, size);
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
