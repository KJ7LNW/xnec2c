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

/* Build the radiation-pattern noise environment menu from the model
 * registries.  Bind runtime rows to config fields, apply the interpolation
 * fallback after commit, and derive sensitivity during refresh. */

#include "rdpattern_noise_menu.h"
#include "shared.h"
#include "measurements.h"
#include "i18n.h"
#include "config/widget/config_widget.h"
#include "config/widget/config_widget_row.h"

/* Names what each selectable method does with the model's frequency table.
 * A named method carries a menu row; the intrinsic methods name none. */
static const char *const noise_interp_purpose[ANT_TEMP_METHOD_COUNT] = {
  [ANT_TEMP_SNAP]     = N_("Read the noise temperature at the nearest "
                           "tabulated frequency."),
  [ANT_TEMP_INTERP]   = N_("Interpolate the noise temperature between "
                           "tabulated frequencies in log-log space, clamped "
                           "at the table edges."),
  [ANT_TEMP_FORMULA]  = NULL,
  [ANT_TEMP_GALACTIC] = NULL,
};

/**
 * noise_menu_shell() - return the flat noise environment menu
 *
 * Return: the menu, or NULL while the window is closed.
 */
static GtkWidget *
noise_menu_shell(void)
{
  if (rdpattern_window_builder == NULL)
    return NULL;

  return Builder_Get_Object(
      rdpattern_window_builder, "rdpattern_noise_env_menu_menu");
}

/**
 * noise_interp_update_sensitivity() - enable/disable interp menu items
 *
 * Computes the union of valid_interp bitmasks from the currently
 * selected sky and earth models.  Sets each interpolation menu item
 * sensitive if its method bit is present in the union, and describes
 * each item, naming the reason on the items it leaves insensitive.
 */
static void
noise_interp_update_sensitivity(void)
{
  GtkWidget *menu = noise_menu_shell();
  if (menu == NULL)
    return;

  const config_widget_scope_t *interp =
      config_widget_field_scope(&rc_config.ant_temp_interp);
  uint8_t allowed = sky_models[rc_config.ant_temp_sky].valid_interp
      | earth_models[rc_config.ant_temp_earth].valid_interp;
  GList *children = gtk_container_get_children(GTK_CONTAINER(menu));

  for (GList *l = children; l != NULL; l = l->next)
  {
    const config_widget_row_t *row = config_widget_row_get(GTK_WIDGET(l->data));

    /* One shell carries the rows of all three noise fields, and a heading
     * or a separator carries none */
    if ((row == NULL) || (row->scope != interp))
      continue;

    int idx = *(const int *)row->elt->value_bytes;
    gboolean usable = (allowed & ANT_TEMP_METHOD_BIT(idx)) != 0;
    const char *purpose = noise_interp_purpose[idx];

    gtk_widget_set_sensitive(GTK_WIDGET(l->data), usable);

    if (purpose == NULL)
    {
      BUG("an interpolation row named a method carrying no description\n");
      continue;
    }

    char *tooltip = g_strdup_printf("%s\n%s", _(purpose), usable ?
        _("Available while a selected sky or earth model reads a frequency "
          "table.") :
        _("The selected sky and earth models resolve by an intrinsic "
          "method; select a table-based model to enable."));

    gtk_widget_set_tooltip_text(GTK_WIDGET(l->data), tooltip);
    g_free(tooltip);
  }
  g_list_free(children);
}

/**
 * hook_noise_env() - render the selected noise environment
 *
 * Derive sensitivity and redraw from the three noise fields so a hover
 * revert restores both outputs.
 */
static void
hook_noise_env(void)
{
  noise_interp_update_sensitivity();
  freq_step_refresh_ui(TRUE);
}

const config_refresh_t hook_noise_env_refresh =
  { .fn = hook_noise_env, .cls = REFRESH_HOVER_SAFE };

/**
 * noise_interp_auto_switch() - validate interp against current models
 * @fallback: method to switch to if current interp is invalid
 *
 * Computes the union of valid interp methods from the selected sky
 * and earth models.  If the current interp is outside that set,
 * switches to the fallback and re-presents the interpolation group.
 */
static void
noise_interp_auto_switch(ant_temp_method_t fallback)
{
  uint8_t allowed = sky_models[rc_config.ant_temp_sky].valid_interp
      | earth_models[rc_config.ant_temp_earth].valid_interp;

  /* Leave the interpolation field unchanged when the models admit it */
  if ((allowed & ANT_TEMP_METHOD_BIT(rc_config.ant_temp_interp)) != 0)
    return;

  /* Clamp intrinsic methods to the menu-visible interpolation choices */
  rc_config.ant_temp_interp =
      ((fallback == ANT_TEMP_SNAP) || (fallback == ANT_TEMP_INTERP))
      ? fallback : ANT_TEMP_INTERP;

  /* Repaint the fallback after the earlier refresh painted the old method */
  config_widget_field_changed(&rc_config.ant_temp_interp);
}

/**
 * hook_noise_sky_commit() - reassign interp on a sky model commit
 *
 * A sky model narrows the interpolation methods its rows admit, so the
 * interpolation field moves with it.
 */
void
hook_noise_sky_commit(void)
{
  noise_interp_auto_switch(sky_models[rc_config.ant_temp_sky].method);
}

/**
 * hook_noise_earth_commit() - reassign interp on an earth model commit
 *
 * An earth model narrows the interpolation methods its rows admit, so the
 * interpolation field moves with it.
 */
void
hook_noise_earth_commit(void)
{
  noise_interp_auto_switch(earth_models[rc_config.ant_temp_earth].method);
}

/**
 * noise_menu_append_separator() - add a separator to the noise menu
 * @menu: target GtkMenuShell
 */
static void
noise_menu_append_separator(GtkWidget *menu)
{
  GtkWidget *sep = gtk_separator_menu_item_new();
  gtk_widget_show(sep);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
}

/**
 * noise_menu_append_heading() - add a disabled label as section heading
 * @menu:  target GtkMenuShell
 * @label: heading text
 */
static void
noise_menu_append_heading(GtkWidget *menu, const char *label)
{
  GtkWidget *item = gtk_menu_item_new_with_label(label);
  gtk_widget_set_sensitive(item, FALSE);
  gtk_widget_show(item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

/**
 * noise_menu_radio_append() - build one bound radio row into a group
 * @menu:  target GtkMenuShell
 * @group: radio group the row joins, extended to include it
 * @label: row text
 * @field: registered field the row writes
 * @idx:   value selecting this row
 *
 * Append the row before binding so it reaches the shell that ends its hover.
 * The stored selection reaches the finished shell through its field, so no
 * row carries an active state of its own here.
 */
static void
noise_menu_radio_append(GtkWidget *menu, GSList **group, const char *label,
    int *field, int idx)
{
  GtkWidget *item = gtk_radio_menu_item_new_with_label(*group, label);

  *group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

  gtk_widget_show(item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  config_widget_bind_row(item, field, &idx);
}

/**
 * noise_model_menus_populate() - build radio items in noise model menu
 *
 * Populates the flat noise model menu from the model registries,
 * separated by section headings and GtkSeparatorMenuItems.
 * Sets active items from rc_config.  Called once when the
 * rdpattern window opens.
 *
 * Derive the initial interpolation sensitivity here because the hook pass
 * runs before this shell holds a row.
 */
void
noise_model_menus_populate(void)
{
  GtkWidget *menu = noise_menu_shell();

  GSList *sky_group = NULL;
  GSList *earth_group = NULL;
  GSList *interp_group = NULL;

  if (menu == NULL)
    return;

  /* Add sky models */
  noise_menu_append_heading(menu, _("Sky Model"));
  for (int i = 0; i < ANT_TEMP_SKY_COUNT; i++)
    noise_menu_radio_append(menu, &sky_group, sky_models[i].name,
        &rc_config.ant_temp_sky, i);

  /* Add earth models */
  noise_menu_append_separator(menu);
  noise_menu_append_heading(menu, _("Earth Model"));
  for (int i = 0; i < ANT_TEMP_EARTH_COUNT; i++)
    noise_menu_radio_append(menu, &earth_group, earth_models[i].name,
        &rc_config.ant_temp_earth, i);

  /* Add user-selectable interpolation methods for table models; the purpose
   * table names them, keeping the intrinsic methods with their models */
  noise_menu_append_separator(menu);
  noise_menu_append_heading(menu, _("Interpolation"));
  for (int i = 0; i < ANT_TEMP_METHOD_COUNT; i++)
  {
    if (noise_interp_purpose[i] == NULL)
      continue;

    noise_menu_radio_append(menu, &interp_group, ant_temp_method_names[i],
        &rc_config.ant_temp_interp, i);
  }

  /* The shell holds every row now, so each field reaches the one row
   * carrying its stored selection */
  config_widget_sync_field(&rc_config.ant_temp_sky);
  config_widget_sync_field(&rc_config.ant_temp_earth);
  config_widget_sync_field(&rc_config.ant_temp_interp);

  noise_interp_update_sensitivity();
}
