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

#include "../shared.h"
#include "../callbacks.h"
#include "../config/widget/config_widget_ops.h"
#include "render_settings.h"
#include "render_settings_internal.h"
#include "render_settings_common.h"

#include "../structure_ui.h"
#include "../rdpattern_ui.h"

#ifdef HAVE_OPENGL
#include "../opengl/opengl_state.h"
#endif


/*------------------------------------------------------------------------*/

/** general_tab_apply_gl_sensitivity - Disable renderer toggle when GL unavailable
 *
 * Called by render_settings_sync_from_config() after the builder's widgets
 * are populated, to apply hardware-availability constraints to the OpenGL
 * renderer checkbox.
 */
static void
general_tab_apply_gl_sensitivity(void)
{
#ifdef HAVE_OPENGL
  gboolean usable = !opengl_gl_context_failed();
  const char *reason = usable ? NULL :
      "OpenGL is not available on this display.\n"
      "Cairo rendering is active.";
#else
  gboolean usable = FALSE;
  const char *reason =
      "Built without OpenGL support.\n"
      "Cairo rendering is active.";
#endif

  config_widget_set_sensitive(&rc_config.use_opengl_renderer, usable);
  config_widget_set_tooltip(&rc_config.use_opengl_renderer, reason);
}

/*------------------------------------------------------------------------*/

/** render_settings_sync_from_config - Populate all widgets from rc_config
 *
 * Called after window creation and when external code changes rc_config
 * (e.g., menu item handlers).  Ortho toolbar buttons on other windows sync
 * through the field-address peer broadcast, not from here.
 */
void
render_settings_sync_from_config(void)
{
  if( render_settings_builder == NULL )
    return;

  config_widget_sync_builder(&render_settings_builder);
  general_tab_apply_gl_sensitivity();
}

/*------------------------------------------------------------------------*/

/** render_settings_init - Create builder, load glade files, wire signals
 *
 * Loads render_settings.glade (window + General tab), then appends
 * OpenGL and Cairo tab pages from their respective sub-glade files.
 * Idempotent; returns TRUE if the window was already created.
 */
gboolean
render_settings_init(void)
{
  if( render_settings_window != NULL )
    return TRUE;

  if( !render_settings_load_glade() )
    return FALSE;

  /* Theme rows come from the startup-initialized registry, so the pull-down
   * fills before the first sync resolves its collapsed label */
  freqplots_theme_menu_build( render_settings_builder );

  /* Append tab pages: General (from render_settings.glade),
   * OpenGL (from opengl_settings.glade), Cairo (from cairo_settings.glade) */
  {
    GtkWidget *notebook = GTK_WIDGET(gtk_builder_get_object(
          render_settings_builder, "render_settings_notebook"));
    GtkWidget *general_content = GTK_WIDGET(gtk_builder_get_object(
          render_settings_builder, "general_tab_content"));
#ifdef HAVE_OPENGL
    GtkWidget *gl_content = GTK_WIDGET(gtk_builder_get_object(
          render_settings_builder, "opengl_tab_content"));
#endif
    GtkWidget *cairo_content = GTK_WIDGET(gtk_builder_get_object(
          render_settings_builder, "cairo_tab_content"));

    if( notebook != NULL && general_content != NULL )
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
          general_content, gtk_label_new("General"));

#ifdef HAVE_OPENGL
    if( notebook != NULL && gl_content != NULL )
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
          gl_content, gtk_label_new("OpenGL"));
#endif

    if( notebook != NULL && cairo_content != NULL )
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
          cairo_content, gtk_label_new("Cairo"));

    /* Flip tab side for RTL locales */
    if( notebook != NULL &&
        gtk_widget_get_direction(notebook) == GTK_TEXT_DIR_RTL )
      gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_RIGHT);
  }

  render_settings_sync_from_config();

  return TRUE;
}

/*------------------------------------------------------------------------*/

/** render_settings_show - Show the settings window, init if needed */
void
render_settings_show(void)
{
  if( !render_settings_init() )
    return;

  render_settings_sync_from_config();
  gtk_widget_show_all(render_settings_window);
  gtk_window_present(GTK_WINDOW(render_settings_window));
}

/*------------------------------------------------------------------------*/

/** render_settings_hide - Hide the settings window */
void
render_settings_hide(void)
{
  if( render_settings_window != NULL )
    gtk_widget_hide(render_settings_window);
}

/*------------------------------------------------------------------------*/

/* Signal handler: menu item activate */
void
on_render_settings_show_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  (void)menuitem;
  (void)user_data;

  render_settings_show();
}

/*------------------------------------------------------------------------*/

/* Signal handler: Close button */
void
on_render_settings_close_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  render_settings_hide();
}

/*------------------------------------------------------------------------*/

/* Signal handler: window close (X button) */
gboolean
on_render_settings_window_delete_event(
    GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void)widget;
  (void)event;
  (void)user_data;

  render_settings_hide();
  return TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal handler: window destroy */
void
on_render_settings_window_destroy(GtkWidget *widget, gpointer user_data)
{
  (void)widget;
  (void)user_data;

  if( render_settings_builder != NULL )
  {
    g_object_unref(render_settings_builder);
    render_settings_builder = NULL;
  }

  render_settings_window = NULL;
}
