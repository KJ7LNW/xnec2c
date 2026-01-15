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

#include "sy_overrides.h"
#include "sy_expr.h"
#include "shared.h"
#include "rc_config.h"
#include "interface.h"
#include "callbacks.h"

/* Row widget structure for each symbol */
typedef struct
{
  gchar *name;
  GtkWidget *row_box;
  GtkWidget *name_label;
  GtkWidget *value_label;
  GtkWidget *override_entry;
  GtkWidget *override_check;
  GtkWidget *min_spin;
  GtkWidget *slider;
  GtkWidget *max_spin;
  GtkWidget *expr_label;
  GtkAdjustment *adjustment;
  gdouble original_value;
  gboolean is_calculated;
} sy_row_t;

/* Window state */
static GPtrArray *rows = NULL;
static GtkWidget *content_box = NULL;
static GtkCssProvider *css_provider = NULL;
static gboolean dirty = FALSE;

/* Forward declarations */
static sy_row_t *sy_row_create(const gchar *name, gdouble value,
    gboolean is_calculated, const gchar *expression,
    gdouble min_value, gdouble max_value,
    gdouble override_value, gboolean override_active);
static void sy_row_free(sy_row_t *row);
static void clear_rows(void);
static void update_strikethrough(sy_row_t *row);

/*------------------------------------------------------------------------*/

/* Callback for populating rows from symbol table */
static void
populate_row_callback(const gchar *name, gdouble value,
    gboolean is_calculated, const gchar *expression,
    gdouble min_value, gdouble max_value,
    gdouble override_value, gboolean override_active,
    gpointer user_data)
{
  sy_row_t *row;

  (void)user_data;

  row = sy_row_create(name, value, is_calculated, expression,
      min_value, max_value, override_value, override_active);

  if( row != NULL )
  {
    g_ptr_array_add(rows, row);
    gtk_box_pack_start(GTK_BOX(content_box), row->row_box, FALSE, FALSE, 0);
  }
}

/*------------------------------------------------------------------------*/

/* Signal: override checkbox toggled */
static void
on_override_check_toggled(GtkToggleButton *button, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;

  (void)button;

  update_strikethrough(row);
  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: slider value changed */
static void
on_slider_value_changed(GtkRange *range, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  gdouble val;
  gdouble min_val;
  gdouble max_val;
  gchar buf[32];

  (void)range;

  val = gtk_adjustment_get_value(row->adjustment);
  min_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(row->min_spin));
  max_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(row->max_spin));

  /* Auto-adjust bounds if value exceeds current limits */
  if( val < min_val )
  {
    /* Recalculate min as half of current value */
    min_val = val * 0.5;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(row->min_spin), min_val);
  }
  else if( val > max_val )
  {
    /* Recalculate max as twice current value */
    max_val = val * 2.0;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(row->max_spin), max_val);
  }

  /* Update override entry to match slider */
  snprintf(buf, sizeof(buf), "%g", val);
  gtk_entry_set_text(GTK_ENTRY(row->override_entry), buf);
  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: override entry changed */
static void
on_override_entry_changed(GtkEditable *editable, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  const gchar *text;
  gdouble val;
  gchar *endptr;

  (void)editable;

  text = gtk_entry_get_text(GTK_ENTRY(row->override_entry));
  val = g_strtod(text, &endptr);

  if( endptr != text && *endptr == '\0' )
  {
    /* Valid number: update slider position */
    g_signal_handlers_block_by_func(row->slider, on_slider_value_changed, row);
    gtk_adjustment_set_value(row->adjustment, val);
    g_signal_handlers_unblock_by_func(row->slider, on_slider_value_changed, row);
  }

  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: min spinbutton changed */
static void
on_min_spin_changed(GtkSpinButton *button, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  gdouble min_val;

  min_val = gtk_spin_button_get_value(button);
  gtk_adjustment_set_lower(row->adjustment, min_val);
  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: max spinbutton changed */
static void
on_max_spin_changed(GtkSpinButton *button, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  gdouble max_val;

  max_val = gtk_spin_button_get_value(button);
  gtk_adjustment_set_upper(row->adjustment, max_val);
  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Update strikethrough style on value label */
static void
update_strikethrough(sy_row_t *row)
{
  GtkStyleContext *context;
  gboolean active;

  context = gtk_widget_get_style_context(row->value_label);
  active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->override_check));

  if( active )
    gtk_style_context_add_class(context, "strikethrough");
  else
    gtk_style_context_remove_class(context, "strikethrough");
}

/*------------------------------------------------------------------------*/

/* Create a row widget for a symbol */
static sy_row_t *
sy_row_create(const gchar *name, gdouble value,
    gboolean is_calculated, const gchar *expression,
    gdouble min_value, gdouble max_value,
    gdouble override_value, gboolean override_active)
{
  sy_row_t *row;
  gchar buf[64];
  gdouble slider_value;

  row = g_new0(sy_row_t, 1);
  row->name = g_strdup(name);
  row->original_value = value;
  row->is_calculated = is_calculated;

  /* Create horizontal box for row */
  row->row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_widget_set_margin_start(row->row_box, 4);
  gtk_widget_set_margin_end(row->row_box, 4);

  /* Name label (fixed width) */
  row->name_label = gtk_label_new(name);
  gtk_widget_set_size_request(row->name_label, 80, -1);
  gtk_label_set_xalign(GTK_LABEL(row->name_label), 0.0);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->name_label, FALSE, FALSE, 0);

  /* Value label (read-only, fixed width) */
  snprintf(buf, sizeof(buf), "%g", value);
  row->value_label = gtk_label_new(buf);
  gtk_widget_set_size_request(row->value_label, 70, -1);
  gtk_label_set_xalign(GTK_LABEL(row->value_label), 1.0);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->value_label, FALSE, FALSE, 0);

  /* Override entry */
  row->override_entry = gtk_entry_new();
  gtk_widget_set_size_request(row->override_entry, 80, -1);
  if( !isnan(override_value) )
  {
    snprintf(buf, sizeof(buf), "%g", override_value);
    gtk_entry_set_text(GTK_ENTRY(row->override_entry), buf);
  }
  gtk_box_pack_start(GTK_BOX(row->row_box), row->override_entry, FALSE, FALSE, 0);

  /* Override checkbox */
  row->override_check = gtk_check_button_new();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->override_check), override_active);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->override_check, FALSE, FALSE, 0);

  /* Min spinbutton */
  row->min_spin = gtk_spin_button_new_with_range(-1e9, 1e9, 0.1);
  gtk_widget_set_size_request(row->min_spin, 80, -1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(row->min_spin), min_value);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->min_spin, FALSE, FALSE, 0);

  /* Slider adjustment and widget */
  slider_value = isnan(override_value) ? value : override_value;
  row->adjustment = gtk_adjustment_new(slider_value, min_value, max_value,
      (max_value - min_value) / 100.0,
      (max_value - min_value) / 10.0, 0);
  row->slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, row->adjustment);
  gtk_scale_set_draw_value(GTK_SCALE(row->slider), FALSE);
  gtk_widget_set_size_request(row->slider, 150, -1);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->slider, TRUE, TRUE, 0);

  /* Max spinbutton */
  row->max_spin = gtk_spin_button_new_with_range(-1e9, 1e9, 0.1);
  gtk_widget_set_size_request(row->max_spin, 80, -1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(row->max_spin), max_value);
  gtk_box_pack_start(GTK_BOX(row->row_box), row->max_spin, FALSE, FALSE, 0);

  /* Expression label (only if calculated) */
  if( is_calculated && expression != NULL && expression[0] != '\0' )
  {
    snprintf(buf, sizeof(buf), "= %s", expression);
    row->expr_label = gtk_label_new(buf);
    gtk_label_set_xalign(GTK_LABEL(row->expr_label), 0.0);
    gtk_box_pack_start(GTK_BOX(row->row_box), row->expr_label, FALSE, FALSE, 0);
  }
  else
  {
    row->expr_label = NULL;
  }

  /* Connect signals */
  g_signal_connect(row->override_check, "toggled",
      G_CALLBACK(on_override_check_toggled), row);
  g_signal_connect(row->slider, "value-changed",
      G_CALLBACK(on_slider_value_changed), row);
  g_signal_connect(row->override_entry, "changed",
      G_CALLBACK(on_override_entry_changed), row);
  g_signal_connect(row->min_spin, "value-changed",
      G_CALLBACK(on_min_spin_changed), row);
  g_signal_connect(row->max_spin, "value-changed",
      G_CALLBACK(on_max_spin_changed), row);

  /* Apply initial strikethrough if override is active */
  update_strikethrough(row);

  gtk_widget_show_all(row->row_box);
  return row;
}

/*------------------------------------------------------------------------*/

/* Free a row widget */
static void
sy_row_free(sy_row_t *row)
{
  if( row == NULL )
    return;

  g_free(row->name);
  g_free(row);
}

/*------------------------------------------------------------------------*/

/* Clear all rows from display */
static void
clear_rows(void)
{
  guint i;
  sy_row_t *row;

  if( rows == NULL )
    return;

  for( i = 0; i < rows->len; i++ )
  {
    row = g_ptr_array_index(rows, i);
    gtk_widget_destroy(row->row_box);
    sy_row_free(row);
  }

  g_ptr_array_set_size(rows, 0);
}

/*------------------------------------------------------------------------*/

gboolean
sy_overrides_init(void)
{
  GError *gerror = NULL;
  const gchar *css_data = ".strikethrough { text-decoration: line-through; }";

  if( sy_overrides_window != NULL )
    return TRUE;

  /* Create builder and load glade file */
  sy_overrides_builder = gtk_builder_new();
  if( !gtk_builder_add_from_resource(sy_overrides_builder,
        "/sy_overrides.glade", &gerror) )
  {
    pr_err("sy_overrides_init: failed to load glade: %s\n", gerror->message);
    g_error_free(gerror);
    return FALSE;
  }

  gtk_builder_connect_signals(sy_overrides_builder, NULL);

  sy_overrides_window = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_window"));
  content_box = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_content"));

  if( sy_overrides_window == NULL || content_box == NULL )
  {
    pr_err("sy_overrides_init: failed to get widgets from glade\n");
    return FALSE;
  }

  /* Initialize rows array */
  rows = g_ptr_array_new();

  /* Setup CSS provider for strikethrough */
  css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(),
      GTK_STYLE_PROVIDER(css_provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  return TRUE;
}

/*------------------------------------------------------------------------*/

void
sy_overrides_show(void)
{
  if( sy_overrides_window == NULL )
  {
    if( !sy_overrides_init() )
      return;
  }

  /* Restore window geometry from config */
  Set_Window_Geometry(sy_overrides_window,
      rc_config.sy_overrides_x, rc_config.sy_overrides_y,
      rc_config.sy_overrides_width, rc_config.sy_overrides_height);

  sy_overrides_refresh();
  gtk_widget_show(sy_overrides_window);
  gtk_window_present(GTK_WINDOW(sy_overrides_window));
}

/*------------------------------------------------------------------------*/

void
sy_overrides_hide(void)
{
  if( sy_overrides_window != NULL )
    gtk_widget_hide(sy_overrides_window);
}

/*------------------------------------------------------------------------*/

void
sy_overrides_refresh(void)
{
  if( sy_overrides_window == NULL )
    return;

  clear_rows();
  sy_foreach(populate_row_callback, NULL);
  dirty = FALSE;
}

/*------------------------------------------------------------------------*/

void
sy_overrides_cleanup(void)
{
  clear_rows();

  if( rows != NULL )
  {
    g_ptr_array_free(rows, TRUE);
    rows = NULL;
  }

  if( css_provider != NULL )
  {
    g_object_unref(css_provider);
    css_provider = NULL;
  }

  if( sy_overrides_builder != NULL )
  {
    g_object_unref(sy_overrides_builder);
    sy_overrides_builder = NULL;
  }

  sy_overrides_window = NULL;
  content_box = NULL;
}

/*------------------------------------------------------------------------*/

/* Signal handler: menu item activate */
void
on_show_sy_overrides_activate(GtkMenuItem *menuitem, gpointer user_data)
{
  (void)user_data;

  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    sy_overrides_show();
  else
    sy_overrides_hide();
}

/*------------------------------------------------------------------------*/

/* Signal handler: window delete event */
gboolean
on_sy_overrides_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GtkWidget *menu_item;

  (void)widget;
  (void)event;
  (void)user_data;

  /* Uncheck menu item */
  menu_item = Builder_Get_Object(main_window_builder, "show_sy_overrides");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), FALSE);

  sy_overrides_cleanup();
  return TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal handler: Apply button clicked */
void
on_sy_overrides_apply_clicked(GtkButton *button, gpointer user_data)
{
  guint i;
  sy_row_t *row;
  const gchar *text;
  gdouble override_val;
  gdouble min_val;
  gdouble max_val;
  gboolean active;
  gchar sy_filename[FILENAME_LEN];
  gchar *dot;

  (void)button;
  (void)user_data;

  /* Update symbol table from UI */
  for( i = 0; i < rows->len; i++ )
  {
    row = g_ptr_array_index(rows, i);

    text = gtk_entry_get_text(GTK_ENTRY(row->override_entry));
    override_val = g_strtod(text, NULL);
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->override_check));
    min_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(row->min_spin));
    max_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(row->max_spin));

    sy_set_override(row->name, override_val, active);
    sy_set_bounds(row->name, min_val, max_val);
  }

  /* Save to .sy file (derive from input file) */
  if( strlen(rc_config.input_file) > 0 )
  {
    Strlcpy(sy_filename, rc_config.input_file, sizeof(sy_filename));
    dot = strrchr(sy_filename, '.');
    if( dot != NULL )
      *dot = '\0';

    Strlcat(sy_filename, ".sy", sizeof(sy_filename));
    sy_save_overrides(sy_filename);
  }

  dirty = FALSE;
}

/*------------------------------------------------------------------------*/

/* Signal handler: Cancel button clicked */
void
on_sy_overrides_cancel_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  sy_overrides_refresh();
}

/*------------------------------------------------------------------------*/

/* Signal handler: Close button clicked */
void
on_sy_overrides_close_clicked(GtkButton *button, gpointer user_data)
{
  GtkWidget *menu_item;

  (void)button;
  (void)user_data;

  /* Uncheck menu item */
  menu_item = Builder_Get_Object(main_window_builder, "show_sy_overrides");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), FALSE);

  sy_overrides_hide();
}
