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

/* Character width for numeric input fields */
#define SY_NUMERIC_WIDTH_CHARS 8

/* Row widget structure for each symbol */
typedef struct
{
  gchar *name;
  GtkWidget *name_label;
  GtkWidget *value_label;
  GtkWidget *override_entry;
  GtkWidget *override_check;
  GtkWidget *min_entry;
  GtkWidget *slider;
  GtkWidget *max_entry;
  GtkWidget *expr_label;
  GtkAdjustment *adjustment;
  gdouble original_value;
  gdouble min_value;
  gdouble max_value;
  gboolean is_calculated;
  gint grid_row;
} sy_row_t;

/* Window state */
static GPtrArray *rows = NULL;
static GtkWidget *content_grid = NULL;
static GtkWidget *show_defaults_toggle = NULL;
static GtkWidget *input_header = NULL;
static GtkWidget *calc_header = NULL;
static GtkCssProvider *css_provider = NULL;
static gboolean dirty = FALSE;

/* Forward declarations */
static void populate_row_widgets(sy_row_t *row, GtkGrid *grid, gint row_index,
    const gchar *expression, gdouble override_value, gboolean override_active);
static void sy_row_free(sy_row_t *row);
static void clear_rows(void);
static void update_strikethrough(sy_row_t *row);
static gboolean is_default_row(sy_row_t *row);
static void set_row_visible(sy_row_t *row, gboolean visible);
static void apply_visibility_filter(void);
static gint compare_rows_by_calculated(gconstpointer a, gconstpointer b);
static GtkWidget *create_section_header(const gchar *text);

/*------------------------------------------------------------------------*/

/* Temporary structure for collecting data during sy_foreach */
typedef struct
{
  gchar *name;
  gchar *expression;
  gdouble value;
  gdouble min_value;
  gdouble max_value;
  gdouble override_value;
  gboolean override_active;
  gboolean is_calculated;
} sy_collect_t;

/* Callback for collecting rows from symbol table */
static void
populate_row_callback(const gchar *name, gdouble value,
    gboolean is_calculated, const gchar *expression,
    gdouble min_value, gdouble max_value,
    gdouble override_value, gboolean override_active,
    gpointer user_data)
{
  GPtrArray *collect_array = (GPtrArray *)user_data;
  sy_collect_t *data;

  data = g_new0(sy_collect_t, 1);
  data->name = g_strdup(name);
  data->expression = g_strdup(expression != NULL ? expression : "");
  data->value = value;
  data->min_value = min_value;
  data->max_value = max_value;
  data->override_value = override_value;
  data->override_active = override_active;
  data->is_calculated = is_calculated;

  g_ptr_array_add(collect_array, data);
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
  min_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->min_entry)), NULL);
  max_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->max_entry)), NULL);

  /* Auto-adjust bounds if value exceeds current limits */
  if( val < min_val )
  {
    /* Recalculate min as half of current value */
    min_val = val * 0.5;
    snprintf(buf, sizeof(buf), "%.3f", min_val);
    gtk_entry_set_text(GTK_ENTRY(row->min_entry), buf);
  }
  else if( val > max_val )
  {
    /* Recalculate max as twice current value */
    max_val = val * 2.0;
    snprintf(buf, sizeof(buf), "%.3f", max_val);
    gtk_entry_set_text(GTK_ENTRY(row->max_entry), buf);
  }
  else
  {
    /* Value within bounds: no adjustment needed */
  }

  /* Update override entry to match slider */
  snprintf(buf, sizeof(buf), "%.3f", val);
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
  else
  {
    /* Invalid or empty input: slider unchanged */
  }

  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: min entry changed */
static void
on_min_entry_changed(GtkEditable *editable, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  const gchar *text;
  gdouble min_val;
  gchar *endptr;

  (void)editable;

  text = gtk_entry_get_text(GTK_ENTRY(row->min_entry));
  min_val = g_strtod(text, &endptr);

  if( endptr != text && *endptr == '\0' )
  {
    gtk_adjustment_set_lower(row->adjustment, min_val);
  }
  else
  {
    /* Invalid input: adjustment unchanged */
  }

  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Signal: max entry changed */
static void
on_max_entry_changed(GtkEditable *editable, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  const gchar *text;
  gdouble max_val;
  gchar *endptr;

  (void)editable;

  text = gtk_entry_get_text(GTK_ENTRY(row->max_entry));
  max_val = g_strtod(text, &endptr);

  if( endptr != text && *endptr == '\0' )
  {
    gtk_adjustment_set_upper(row->adjustment, max_val);
  }
  else
  {
    /* Invalid input: adjustment unchanged */
  }

  dirty = TRUE;
}

/*------------------------------------------------------------------------*/

/* Check if row has default values (epsilon for floating point comparison) */
static gboolean
is_default_row(sy_row_t *row)
{
  gboolean active;
  gdouble min_val;
  gdouble max_val;
  gdouble expected_min;
  gdouble expected_max;
  const gdouble epsilon = 1e-9;

  if( row == NULL )
    return FALSE;

  active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->override_check));
  if( active )
    return FALSE;

  min_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->min_entry)), NULL);
  max_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->max_entry)), NULL);
  expected_min = row->original_value * 0.5;
  expected_max = row->original_value * 2.0;

  return fabs(min_val - expected_min) < epsilon && fabs(max_val - expected_max) < epsilon;
}

/*------------------------------------------------------------------------*/

/* Set visibility for all widgets in a row */
static void
set_row_visible(sy_row_t *row, gboolean visible)
{
  if( row == NULL )
    return;

  gtk_widget_set_visible(row->name_label, visible);
  gtk_widget_set_visible(row->value_label, visible);
  gtk_widget_set_visible(row->override_entry, visible);
  gtk_widget_set_visible(row->override_check, visible);
  gtk_widget_set_visible(row->min_entry, visible);
  gtk_widget_set_visible(row->slider, visible);
  gtk_widget_set_visible(row->max_entry, visible);

  if( row->expr_label != NULL )
    gtk_widget_set_visible(row->expr_label, visible);
}

/*------------------------------------------------------------------------*/

/* Apply visibility filter based on show_defaults toggle */
static void
apply_visibility_filter(void)
{
  guint i;
  sy_row_t *row;
  gboolean show_defaults;

  if( rows == NULL || show_defaults_toggle == NULL )
    return;

  show_defaults = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_defaults_toggle));

  for( i = 0; i < rows->len; i++ )
  {
    row = g_ptr_array_index(rows, i);

    if( show_defaults )
      set_row_visible(row, TRUE);
    else
      set_row_visible(row, !is_default_row(row));
  }
}

/*------------------------------------------------------------------------*/

/* Compare function for sorting collected data: inputs first, calculated second */
static gint
compare_rows_by_calculated(gconstpointer a, gconstpointer b)
{
  const sy_collect_t *data_a = *(const sy_collect_t **)a;
  const sy_collect_t *data_b = *(const sy_collect_t **)b;

  if( data_a->is_calculated == data_b->is_calculated )
    return 0;

  if( data_a->is_calculated )
    return 1;
  else
    return -1;
}

/*------------------------------------------------------------------------*/

/* Create a section header label */
static GtkWidget *
create_section_header(const gchar *text)
{
  GtkWidget *label;
  gchar *markup;

  label = gtk_label_new(NULL);
  markup = g_markup_printf_escaped("<b>%s</b>", text);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_widget_set_margin_top(label, 6);
  gtk_widget_set_margin_bottom(label, 2);

  return label;
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

/* Populate widgets for an existing row structure and attach to grid */
static void
populate_row_widgets(sy_row_t *row, GtkGrid *grid, gint row_index,
    const gchar *expression, gdouble override_value, gboolean override_active)
{
  gchar buf[64];
  gdouble slider_value;

  row->grid_row = row_index;

  /* Name label (column 0, fixed width) */
  row->name_label = gtk_label_new(row->name);
  gtk_widget_set_size_request(row->name_label, 90, -1);
  gtk_label_set_xalign(GTK_LABEL(row->name_label), 0.0);
  gtk_grid_attach(grid, row->name_label, 0, row_index, 1, 1);

  /* Value label (column 1, fixed width, 3 decimal places) */
  snprintf(buf, sizeof(buf), "%.3f", row->original_value);
  row->value_label = gtk_label_new(buf);
  gtk_label_set_width_chars(GTK_LABEL(row->value_label), SY_NUMERIC_WIDTH_CHARS);
  gtk_label_set_max_width_chars(GTK_LABEL(row->value_label), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->value_label, FALSE);
  gtk_label_set_xalign(GTK_LABEL(row->value_label), 1.0);
  gtk_grid_attach(grid, row->value_label, 1, row_index, 1, 1);

  /* Override entry (column 2, fixed width, 3 decimal places, right-justified) */
  row->override_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->override_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->override_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->override_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->override_entry), 1.0);
  if( !isnan(override_value) )
  {
    snprintf(buf, sizeof(buf), "%.3f", override_value);
    gtk_entry_set_text(GTK_ENTRY(row->override_entry), buf);
  }
  else
  {
    /* No override value: leave entry empty */
  }
  gtk_grid_attach(grid, row->override_entry, 2, row_index, 1, 1);

  /* Override checkbox (column 3, fixed width) */
  row->override_check = gtk_check_button_new();
  gtk_widget_set_size_request(row->override_check, 30, -1);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->override_check), override_active);
  gtk_grid_attach(grid, row->override_check, 3, row_index, 1, 1);

  /* Min entry (column 4, fixed width, 3 decimal places, right-justified) */
  row->min_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->min_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->min_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->min_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->min_entry), 1.0);
  snprintf(buf, sizeof(buf), "%.3f", row->min_value);
  gtk_entry_set_text(GTK_ENTRY(row->min_entry), buf);
  gtk_grid_attach(grid, row->min_entry, 4, row_index, 1, 1);

  /* Slider adjustment and widget (column 5, fixed width, no expand) */
  slider_value = isnan(override_value) ? row->original_value : override_value;
  row->adjustment = gtk_adjustment_new(slider_value, row->min_value, row->max_value,
      (row->max_value - row->min_value) / 100.0,
      (row->max_value - row->min_value) / 10.0, 0);
  row->slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, row->adjustment);
  gtk_scale_set_draw_value(GTK_SCALE(row->slider), FALSE);
  gtk_widget_set_size_request(row->slider, 200, -1);
  gtk_widget_set_hexpand(row->slider, FALSE);
  gtk_grid_attach(grid, row->slider, 5, row_index, 1, 1);

  /* Max entry (column 6, fixed width, 3 decimal places, right-justified) */
  row->max_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->max_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->max_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->max_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->max_entry), 1.0);
  snprintf(buf, sizeof(buf), "%.3f", row->max_value);
  gtk_entry_set_text(GTK_ENTRY(row->max_entry), buf);
  gtk_grid_attach(grid, row->max_entry, 6, row_index, 1, 1);

  /* Expression label (column 7, expanding) */
  if( row->is_calculated && expression != NULL && expression[0] != '\0' )
  {
    snprintf(buf, sizeof(buf), "= %s", expression);
    row->expr_label = gtk_label_new(buf);
    gtk_label_set_xalign(GTK_LABEL(row->expr_label), 0.0);
    gtk_widget_set_hexpand(row->expr_label, TRUE);
    gtk_grid_attach(grid, row->expr_label, 7, row_index, 1, 1);
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
  g_signal_connect(row->min_entry, "changed",
      G_CALLBACK(on_min_entry_changed), row);
  g_signal_connect(row->max_entry, "changed",
      G_CALLBACK(on_max_entry_changed), row);

  /* Apply initial strikethrough if override is active */
  update_strikethrough(row);

  gtk_widget_show_all(row->name_label);
  gtk_widget_show_all(row->value_label);
  gtk_widget_show_all(row->override_entry);
  gtk_widget_show_all(row->override_check);
  gtk_widget_show_all(row->min_entry);
  gtk_widget_show_all(row->slider);
  gtk_widget_show_all(row->max_entry);

  if( row->expr_label != NULL )
  {
    gtk_widget_show_all(row->expr_label);
  }
  else
  {
    /* No expression label for input symbols */
  }
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

    gtk_widget_destroy(row->name_label);
    gtk_widget_destroy(row->value_label);
    gtk_widget_destroy(row->override_entry);
    gtk_widget_destroy(row->override_check);
    gtk_widget_destroy(row->min_entry);
    gtk_widget_destroy(row->slider);
    gtk_widget_destroy(row->max_entry);

    if( row->expr_label != NULL )
    {
      gtk_widget_destroy(row->expr_label);
    }
    else
    {
      /* No expression label to destroy for input symbols */
    }

    sy_row_free(row);
  }

  g_ptr_array_set_size(rows, 0);

  if( input_header != NULL )
  {
    gtk_widget_destroy(input_header);
    input_header = NULL;
  }

  if( calc_header != NULL )
  {
    gtk_widget_destroy(calc_header);
    calc_header = NULL;
  }
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
  content_grid = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_content"));
  show_defaults_toggle = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_show_defaults"));

  if( sy_overrides_window == NULL || content_grid == NULL || show_defaults_toggle == NULL )
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
  GPtrArray *collect_array;
  guint i;
  sy_collect_t *data;
  sy_row_t *row;
  gint grid_row;
  gboolean in_input_section;

  if( sy_overrides_window == NULL )
    return;

  clear_rows();

  collect_array = g_ptr_array_new();
  sy_foreach(populate_row_callback, collect_array);

  if( collect_array->len == 0 )
  {
    g_ptr_array_free(collect_array, TRUE);
    dirty = FALSE;
    return;
  }

  g_ptr_array_sort(collect_array, compare_rows_by_calculated);

  grid_row = 0;
  in_input_section = FALSE;

  for( i = 0; i < collect_array->len; i++ )
  {
    data = g_ptr_array_index(collect_array, i);

    if( !data->is_calculated && input_header == NULL )
    {
      input_header = create_section_header("Input Symbols");
      gtk_grid_attach(GTK_GRID(content_grid), input_header, 0, grid_row, 8, 1);
      gtk_widget_show_all(input_header);
      grid_row++;
      in_input_section = TRUE;
    }
    else if( data->is_calculated && calc_header == NULL )
    {
      if( in_input_section )
      {
        grid_row++;
      }
      else
      {
        /* No input section preceded: no spacer needed */
      }

      calc_header = create_section_header("Calculated Symbols");
      gtk_grid_attach(GTK_GRID(content_grid), calc_header, 0, grid_row, 8, 1);
      gtk_widget_show_all(calc_header);
      grid_row++;
      in_input_section = FALSE;
    }
    else
    {
      /* Cases: input row within input section, or calculated row after calculated header */
    }

    row = g_new0(sy_row_t, 1);
    row->name = g_strdup(data->name);
    row->original_value = data->value;
    row->min_value = data->min_value;
    row->max_value = data->max_value;
    row->is_calculated = data->is_calculated;

    populate_row_widgets(row, GTK_GRID(content_grid), grid_row,
        data->expression, data->override_value, data->override_active);

    g_ptr_array_add(rows, row);
    grid_row++;
  }

  for( i = 0; i < collect_array->len; i++ )
  {
    data = g_ptr_array_index(collect_array, i);
    g_free(data->name);
    g_free(data->expression);
    g_free(data);
  }

  g_ptr_array_free(collect_array, TRUE);

  apply_visibility_filter();
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
  content_grid = NULL;
  show_defaults_toggle = NULL;
  input_header = NULL;
  calc_header = NULL;
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

/* Signal handler: Show defaults toggle */
void
on_sy_show_defaults_toggled(GtkToggleButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;

  apply_visibility_filter();
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
    min_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->min_entry)), NULL);
    max_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->max_entry)), NULL);

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
