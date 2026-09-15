/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

/*
 * freqplots_theme_menu: builds the rendering-settings Color Theme pull-down
 * from the core theme registry (src/themes/theme.c).  The rows live on the
 * render settings window, so their construction is settings UI; the theme
 * model and registry are core and stay under themes/.  The "Inverted" check
 * item is an orthogonal axis above one radio group of base theme names.
 */

#include "../interface.h"
#include "../shared.h"
#include "../i18n.h"
#include "../themes/theme.h"
#include "../config/config_widget.h"

/* freqplots_invert_item_sync()
 *
 * Match the Inverted item's sensitivity to whether base carries an inverted
 * variant; a theme without one (legacy, smith-derived) leaves the item
 * insensitive and shows an explanatory tooltip, set only in that state. */
  static void
freqplots_invert_item_sync( GtkWidget *invert, const char *base )
{
  gboolean has = theme_has_inverted( base );

  gtk_widget_set_sensitive( invert, has );
  gtk_widget_set_tooltip_text( invert,
      has ? NULL : _("This theme has no inverted variant") );
}

/* freqplots_theme_invert_sync()
 *
 * Match the live Inverted item to the base theme the field now holds.  The
 * theme refresh calls this on every commit, hover and revert, so the item
 * follows the value rather than the click that produced it.  Idle while the
 * render settings window is unbuilt and carries no menu. */
  void
freqplots_theme_invert_sync( void )
{
  GtkWidget *menu;
  GtkWidget *invert;

  if( render_settings_builder == NULL ) return;

  menu = Builder_Get_Object( render_settings_builder,
      "render_theme_menu" );
  if( menu == NULL ) return;

  invert = g_object_get_data( G_OBJECT(menu), THEME_DATA_INVERT_ITEM );
  if( invert == NULL ) return;

  freqplots_invert_item_sync( invert, rc_config.freqplots_theme );
}

/* freqplots_theme_radio_append()
 *
 * Build one base-theme radio item into the shared group, seed its active
 * state from the persisted selection, and bind it to the theme field so the
 * config engine carries this row's hover and its commit. */
  static void
freqplots_theme_radio_append( GtkWidget *menu, GSList **group,
    const theme_menu_entry_t *e )
{
  GtkWidget *item = gtk_radio_menu_item_new_with_label( *group, e->display );

  /* Zeroed over the whole field width so a shorter theme name cannot retain
   * suffix bytes from the row staged before it. */
  char candidate[sizeof(rc_config.freqplots_theme)] = { 0 };

  *group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item) );

  Strlcpy( candidate, e->base_name, sizeof(candidate) );

  gtk_widget_show( item );
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );

  /* Appended first, so the row reaches the shell that ends its hover */
  config_widget_bind_row( item, rc_config.freqplots_theme, candidate );
}

/* freqplots_theme_menu_build()
 *
 * Generates the Color Theme pull-down from the theme registry into the
 * glade-supplied menu.  The "Inverted" check item is an orthogonal
 * axis above one radio group of base theme names; legacy heads the list as the
 * default, user custom themes follow it, and a separator divides that top
 * group from the built-in themes in registry order.
 * The Inverted item rides the menu shell, so the theme refresh reaches it
 * without a handler of its own.  Each row shows itself, leaving the shell
 * unmapped until the button pops it up. */
  void
freqplots_theme_menu_build( GtkBuilder *builder )
{
  GtkWidget       *menu = Builder_Get_Object( builder, "render_theme_menu" );
  GtkWidget       *invert;
  GtkWidget       *separator;
  const GPtrArray *entries = theme_menu_entries();
  GSList          *group = NULL;
  guint            i;

  if( menu == NULL || entries == NULL )
    return;

  invert = gtk_check_menu_item_new_with_mnemonic( _("_Invert") );
  gtk_widget_show( invert );
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), invert );

  /* Appended first, so the row reaches the shell that ends its hover */
  config_widget_bind_row( invert, &rc_config.freqplots_theme_invert, NULL );

  freqplots_invert_item_sync( invert, rc_config.freqplots_theme );
  g_object_set_data( G_OBJECT(menu), THEME_DATA_INVERT_ITEM, invert );

  /* Legacy heads the radio list; user custom themes follow it so they sit at
   * the top, adjacent to the default and easy to find, set off by a separator
   * from the built-in themes. */
  for( i = 0; i < entries->len; i++ )
  {
    theme_menu_entry_t *e = g_ptr_array_index( entries, i );

    if( g_strcmp0( e->base_name, "legacy" ) != 0 )
      continue;

    freqplots_theme_radio_append( menu, &group, e );
    break;
  }

  for( i = 0; i < entries->len; i++ )
  {
    theme_menu_entry_t *e = g_ptr_array_index( entries, i );

    if( !e->user_origin || g_strcmp0( e->base_name, "legacy" ) == 0 )
      continue;

    freqplots_theme_radio_append( menu, &group, e );
  }

  separator = gtk_separator_menu_item_new();
  gtk_widget_show( separator );
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), separator );

  for( i = 0; i < entries->len; i++ )
  {
    theme_menu_entry_t *e = g_ptr_array_index( entries, i );

    if( g_strcmp0( e->base_name, "legacy" ) == 0 || e->user_origin )
      continue;

    freqplots_theme_radio_append( menu, &group, e );
  }
}
