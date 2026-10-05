#include "ui_style.h"

#include <gtk/gtk.h>

#include "console.h"
#include "themes/theme.h"

/* Runtime color definitions the application stylesheet refers to */
static GtkCssProvider *ui_style_color_provider;

/**
 * ui_style_init() - install the application stylesheet on the default screen
 *
 * The stylesheet rides the resource bundle and enters at application
 * priority, so it layers over the theme while a user stylesheet still wins.
 * Every window built afterwards resolves against it, which is why this runs
 * before the first window exists.  The screen keeps the provider for the life
 * of the display, so the local reference goes back at once.  The color
 * provider beside it stays referenced here, since every theme change reloads
 * its definitions.
 */
void ui_style_init(void)
{
	GtkCssProvider *provider = gtk_css_provider_new();

	gtk_css_provider_load_from_resource(provider, "/xnec2c.css");
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
		GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(provider);

	ui_style_color_provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
		GTK_STYLE_PROVIDER(ui_style_color_provider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/**
 * ui_style_sync_canvas_background() - define the stylesheet's canvas background
 *
 * Replaces the @canvas_background definition that the canvas border mixes
 * with the window color by the active theme's background, so the border
 * restyles with the theme.
 */
void ui_style_sync_canvas_background(void)
{
	rgb_f_t background = theme_active()->colors[THEME_ROLE_BACKGROUND];
	GdkRGBA rgba = { .red = (gdouble)background.r,
		.green = (gdouble)background.g, .blue = (gdouble)background.b,
		.alpha = 1.0 };
	char *color = gdk_rgba_to_string(&rgba);
	char *css = g_strdup_printf("@define-color canvas_background %s;", color);
	GError *error = NULL;

	if( !gtk_css_provider_load_from_data(ui_style_color_provider, css, -1,
			&error) )
	{
		BUG("canvas background definition '%s' failed to parse: %s\n",
			css, error->message);
		g_error_free(error);
	}

	g_free(css);
	g_free(color);
}
