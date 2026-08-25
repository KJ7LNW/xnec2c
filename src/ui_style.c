#include "ui_style.h"

#include <gtk/gtk.h>

/**
 * ui_style_init() - install the application stylesheet on the default screen
 *
 * The stylesheet rides the resource bundle and enters at application
 * priority, so it layers over the theme while a user stylesheet still wins.
 * Every window built afterwards resolves against it, which is why this runs
 * before the first window exists.  The screen keeps the provider for the life
 * of the display, so the local reference goes back at once.
 */
void ui_style_init(void)
{
	GtkCssProvider *provider = gtk_css_provider_new();

	gtk_css_provider_load_from_resource(provider, "/xnec2c.css");
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
		GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(provider);
}
