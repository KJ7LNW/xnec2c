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
#define SY_NUMERIC_WIDTH_CHARS 12

/* Format string for value display (5 decimal places) */
#define SY_VALUE_FMT "%.5f"


/* Size group collection for column alignment */
typedef struct
{
  GtkSizeGroup *name;
  GtkSizeGroup *value;
  GtkSizeGroup *override;
  GtkSizeGroup *check;
  GtkSizeGroup *min;
  GtkSizeGroup *slider;
  GtkSizeGroup *max;
  GtkSizeGroup *expr;
} sy_size_groups_t;

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
static GtkWidget *input_expander = NULL;
static GtkWidget *input_col_headers[8] = {NULL};
static GtkWidget *calc_expander = NULL;
static GtkWidget *calc_col_headers[8] = {NULL};
static GtkCssProvider *css_provider = NULL;
static sy_size_groups_t sg = {0};
static gboolean dirty = FALSE;

/* Auto-apply state */
static guint debounce_timer_id = 0;
static gboolean auto_apply_enabled = FALSE;
static gboolean pending_apply = FALSE;
static guint last_applied_hash = 0;
static GtkWidget *busy_spinner = NULL;
static GtkWidget *auto_apply_check = NULL;
static GtkWidget *apply_button = NULL;

/* Forward declarations */
static void populate_row_widgets(sy_row_t *row, GtkGrid *grid, gint row_index,
    const gchar *expression, gdouble override_value, gboolean override_active, gboolean show_widgets);
static void sy_row_free(sy_row_t *row);
static void clear_rows(void);
static void update_strikethrough(sy_row_t *row);
static void set_row_visible(sy_row_t *row, gboolean visible);
static gint compare_rows_by_calculated(gconstpointer a, gconstpointer b);
static GtkWidget *create_section_header(const gchar *text);
static void create_column_headers(GtkWidget **header_widgets, GtkGrid *grid, gint row_index, gint num_columns);
static void on_expander_notify_expanded(GObject *object, GParamSpec *pspec, gpointer user_data);
static guint compute_override_hash(void);
static gboolean is_calculation_busy(void);
static void apply_overrides_to_symbols(void);
static void try_auto_apply(void);
static gboolean debounce_expired(gpointer user_data);
static gboolean check_pending_apply(gpointer user_data);
static void on_auto_apply_toggled(GtkToggleButton *button, gpointer user_data);
static void on_override_value_changed(GtkWidget *widget, gpointer user_data);
static gboolean on_override_entry_focus_out(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void set_row_sensitivity(sy_row_t *row, gboolean sensitive);
static void set_entry_value(GtkEntry *entry, gdouble val);

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

/* Set sensitivity of row widgets based on override state */
static void
set_row_sensitivity(sy_row_t *row, gboolean sensitive)
{
  gtk_widget_set_sensitive(row->override_entry, sensitive);
  gtk_widget_set_sensitive(row->slider, sensitive);
  gtk_widget_set_sensitive(row->min_entry, sensitive);
  gtk_widget_set_sensitive(row->max_entry, sensitive);
}

/*------------------------------------------------------------------------*/

/* Set entry text to formatted value */
static void
set_entry_value(GtkEntry *entry, gdouble val)
{
  gchar buf[32];

  snprintf(buf, sizeof(buf), SY_VALUE_FMT, val);
  gtk_entry_set_text(entry, buf);
}

/*------------------------------------------------------------------------*/

/* Signal: override checkbox toggled */
static void
on_override_check_toggled(GtkToggleButton *button, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  gboolean active;

  (void)button;

  active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->override_check));
  set_row_sensitivity(row, active);

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

  (void)range;

  val = gtk_adjustment_get_value(row->adjustment);
  min_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->min_entry)), NULL);
  max_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->max_entry)), NULL);

  /* Auto-adjust bounds if value exceeds current limits */
  if( val < min_val )
  {
    /* Recalculate min as half of current value */
    min_val = val * 0.5;
    set_entry_value(GTK_ENTRY(row->min_entry), min_val);
  }
  else if( val > max_val )
  {
    /* Recalculate max as twice current value */
    max_val = val * 2.0;
    set_entry_value(GTK_ENTRY(row->max_entry), max_val);
  }
  else
  {
    /* Value within bounds: no adjustment needed */
  }

  /* Update override entry to match slider */
  set_entry_value(GTK_ENTRY(row->override_entry), val);
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

/* Signal: override entry focus out */
static gboolean
on_override_entry_focus_out(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  sy_row_t *row = (sy_row_t *)user_data;
  const gchar *text;
  gdouble val;
  gdouble min_val;
  gdouble max_val;
  gchar *endptr;

  (void)widget;
  (void)event;

  text = gtk_entry_get_text(GTK_ENTRY(row->override_entry));
  val = g_strtod(text, &endptr);

  /* Invalid input: no adjustment */
  if( endptr == text || *endptr != '\0' )
  {
    return FALSE;
  }

  min_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->min_entry)), NULL);
  max_val = g_strtod(gtk_entry_get_text(GTK_ENTRY(row->max_entry)), NULL);

  if( val < min_val )
  {
    set_entry_value(GTK_ENTRY(row->min_entry), val);
  }
  else if( val > max_val )
  {
    set_entry_value(GTK_ENTRY(row->max_entry), val);
  }

  return FALSE;
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

/* Compute hash of current override state */
static guint
compute_override_hash(void)
{
  GString *state;
  guint i;
  sy_row_t *row;
  const gchar *text;
  gboolean active;
  guint hash;
  gchar buf[32];

  if( rows == NULL || rows->len == 0 )
    return 0;

  state = g_string_new(NULL);

  for( i = 0; i < rows->len; i++ )
  {
    row = g_ptr_array_index(rows, i);
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->override_check));

    g_string_append(state, row->name);
    g_string_append(state, ":");

    if( active )
    {
      text = gtk_entry_get_text(GTK_ENTRY(row->override_entry));
      g_string_append(state, text);
    }
    else
    {
      snprintf(buf, sizeof(buf), SY_VALUE_FMT, row->original_value);
      g_string_append(state, buf);
    }

    g_string_append(state, ";");
  }

  hash = g_str_hash(state->str);
  g_string_free(state, TRUE);

  return hash;
}

/*------------------------------------------------------------------------*/

/* Check if calculation is busy */
static gboolean
is_calculation_busy(void)
{
  gint i;

  if( isFlagSet(FREQ_LOOP_RUNNING | FREQ_LOOP_INIT | INPUT_PENDING) ||
      isFlagClear(FREQ_LOOP_DONE) )
  {
    return TRUE;
  }
  else
  {
    /* Check child processes for busy state */
  }

  for( i = 0; i < num_child_procs; i++ )
  {
    if( forked_proc_data != NULL &&
        forked_proc_data[i] != NULL &&
        forked_proc_data[i]->busy )
    {
      return TRUE;
    }
    else
    {
      /* Process not busy or does not exist */
    }
  }

  return FALSE;
}

/*------------------------------------------------------------------------*/

/* Apply overrides to symbol table and save */
static void
apply_overrides_to_symbols(void)
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

  if( strlen(rc_config.input_file) > 0 )
  {
    Strlcpy(sy_filename, rc_config.input_file, sizeof(sy_filename));
    dot = strrchr(sy_filename, '.');
    if( dot != NULL )
    {
      *dot = '\0';
    }
    else
    {
      /* No extension found */
    }

    Strlcat(sy_filename, ".sy", sizeof(sy_filename));
    sy_save_overrides(sy_filename);
  }
  else
  {
    /* No input file to derive .sy filename from */
  }
}

/*------------------------------------------------------------------------*/

/* Try to auto-apply changes */
static void
try_auto_apply(void)
{
  static gboolean new_flag = 0;
  guint current_hash;

  current_hash = compute_override_hash();

  if( current_hash == last_applied_hash )
    return;

  if( is_calculation_busy() )
  {
    pending_apply = TRUE;
    if( busy_spinner != NULL )
    {
      gtk_widget_show(busy_spinner);
      gtk_spinner_start(GTK_SPINNER(busy_spinner));
    }
    else
    {
      /* Spinner not available */
    }
    g_idle_add(check_pending_apply, NULL);
    return;
  }
  else
  {
    /* Not busy: proceed with apply */
  }

  apply_overrides_to_symbols();
  last_applied_hash = current_hash;
  dirty = FALSE;

  g_idle_add(Open_Input_File, (gpointer)&new_flag);
}

/*------------------------------------------------------------------------*/

/* Debounce timer expired */
static gboolean
debounce_expired(gpointer user_data)
{
  (void)user_data;

  debounce_timer_id = 0;
  try_auto_apply();

  return G_SOURCE_REMOVE;
}

/*------------------------------------------------------------------------*/

/* Check if pending apply can proceed */
static gboolean
check_pending_apply(gpointer user_data)
{
  (void)user_data;

  if( !pending_apply )
    return G_SOURCE_REMOVE;

  if( is_calculation_busy() )
    return G_SOURCE_CONTINUE;

  pending_apply = FALSE;

  if( busy_spinner != NULL )
  {
    gtk_spinner_stop(GTK_SPINNER(busy_spinner));
    gtk_widget_hide(busy_spinner);
  }
  else
  {
    /* Spinner not available */
  }

  try_auto_apply();

  return G_SOURCE_REMOVE;
}

/*------------------------------------------------------------------------*/

/* Signal handler: auto-apply checkbox toggled */
static void
on_auto_apply_toggled(GtkToggleButton *button, gpointer user_data)
{
  (void)user_data;

  auto_apply_enabled = gtk_toggle_button_get_active(button);

  if( auto_apply_enabled )
  {
    SetFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
    try_auto_apply();
  }
  else
  {
    if( debounce_timer_id != 0 )
    {
      g_source_remove(debounce_timer_id);
      debounce_timer_id = 0;
    }

    pending_apply = FALSE;

    if( busy_spinner != NULL )
    {
      gtk_spinner_stop(GTK_SPINNER(busy_spinner));
      gtk_widget_hide(busy_spinner);
    }

    // Optimizer menu active: keep flag set
    GtkWidget *optimizer_menu = Builder_Get_Object(main_window_builder, "optimizer_output");
    if( optimizer_menu == NULL || !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(optimizer_menu)) )
    {
      ClearFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
    }
  }

  if( apply_button != NULL )
  {
    gtk_widget_set_sensitive(apply_button, !auto_apply_enabled);
  }
  else
  {
    /* Apply button not available */
  }
}

/*------------------------------------------------------------------------*/

/* Signal handler: override value or checkbox changed */
static void
on_override_value_changed(GtkWidget *widget, gpointer user_data)
{
  (void)widget;
  (void)user_data;

  if( !auto_apply_enabled )
    return;

  if( debounce_timer_id != 0 )
  {
    g_source_remove(debounce_timer_id);
  }
  else
  {
    /* No existing timer to remove */
  }

  debounce_timer_id = g_timeout_add(300, debounce_expired, NULL);
}

/*------------------------------------------------------------------------*/

/* Apply visibility filter based on expander state */
static void
apply_visibility_filter(void)
{
  guint i;
  sy_row_t *row;
  gboolean input_expanded;
  gboolean calc_expanded;

  if( rows == NULL )
    return;

  input_expanded = input_expander != NULL && gtk_expander_get_expanded(GTK_EXPANDER(input_expander));
  calc_expanded = calc_expander != NULL && gtk_expander_get_expanded(GTK_EXPANDER(calc_expander));

  for( i = 0; i < rows->len; i++ )
  {
    row = g_ptr_array_index(rows, i);

    if( row->is_calculated )
      set_row_visible(row, calc_expanded);
    else
      set_row_visible(row, input_expanded);
  }

  for( i = 0; i < 8; i++ )
  {
    if( input_col_headers[i] != NULL )
      gtk_widget_set_visible(input_col_headers[i], input_expanded);
  }

  for( i = 0; i < 8; i++ )
  {
    if( calc_col_headers[i] != NULL )
      gtk_widget_set_visible(calc_col_headers[i], calc_expanded);
  }
}

/*------------------------------------------------------------------------*/

/* Signal: expander expanded state changed */
static void
on_expander_notify_expanded(GObject *object, GParamSpec *pspec, gpointer user_data)
{
  (void)object;
  (void)pspec;
  (void)user_data;

  apply_visibility_filter();
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

/* Create column header row and attach to grid */
static void
create_column_headers(GtkWidget **header_widgets, GtkGrid *grid, gint row_index, gint num_columns)
{
  GtkWidget *label;
  GtkStyleContext *context;
  const gchar *labels[] = {"Symbol", "Value", "Override", "✓", "Min", "", "Max", "Expression"};
  GtkSizeGroup *groups[] = {sg.name, sg.value, sg.override, sg.check, sg.min, sg.slider, sg.max, sg.expr};
  gfloat xaligns[] = {0.0, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.0};
  gchar *markup;
  gint i;

  for( i = 0; i < num_columns; i++ )
  {
    label = gtk_label_new(NULL);

    /* Apply markup only to non-empty labels; slider column label is empty */
    if( labels[i][0] != '\0' )
    {
      markup = g_markup_printf_escaped("<b><u>%s</u></b>", labels[i]);
      gtk_label_set_markup(GTK_LABEL(label), markup);
      g_free(markup);
    }

    gtk_label_set_xalign(GTK_LABEL(label), xaligns[i]);
    context = gtk_widget_get_style_context(label);
    gtk_style_context_add_class(context, "header");
    gtk_grid_attach(grid, label, i, row_index, 1, 1);
    gtk_size_group_add_widget(groups[i], label);
    gtk_widget_show_all(label);
    header_widgets[i] = label;
  }
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
    const gchar *expression, gdouble override_value, gboolean override_active, gboolean show_widgets)
{
  gchar buf[64];
  gdouble slider_value;

  row->grid_row = row_index;

  /* Name label (column 0, fixed width) */
  row->name_label = gtk_label_new(row->name);
  gtk_widget_set_size_request(row->name_label, 90, -1);
  gtk_label_set_xalign(GTK_LABEL(row->name_label), 0.0);
  gtk_grid_attach(grid, row->name_label, 0, row_index, 1, 1);
  gtk_size_group_add_widget(sg.name, row->name_label);

  /* Value label (column 1, fixed width, 5 decimal places) */
  snprintf(buf, sizeof(buf), SY_VALUE_FMT, row->original_value);
  row->value_label = gtk_label_new(buf);
  gtk_label_set_width_chars(GTK_LABEL(row->value_label), SY_NUMERIC_WIDTH_CHARS);
  gtk_label_set_max_width_chars(GTK_LABEL(row->value_label), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->value_label, FALSE);
  gtk_label_set_xalign(GTK_LABEL(row->value_label), 1.0);
  gtk_grid_attach(grid, row->value_label, 1, row_index, 1, 1);
  gtk_size_group_add_widget(sg.value, row->value_label);

  /* Override entry (column 2, fixed width, 5 decimal places, right-justified) */
  row->override_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->override_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->override_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->override_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->override_entry), 1.0);
  if( !isnan(override_value) )
  {
    set_entry_value(GTK_ENTRY(row->override_entry), override_value);
  }
  else
  {
    set_entry_value(GTK_ENTRY(row->override_entry), row->original_value);
  }
  gtk_grid_attach(grid, row->override_entry, 2, row_index, 1, 1);
  gtk_size_group_add_widget(sg.override, row->override_entry);

  /* Override checkbox (column 3, fixed width) */
  row->override_check = gtk_check_button_new();
  gtk_widget_set_size_request(row->override_check, 30, -1);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->override_check), override_active);
  gtk_widget_set_halign(row->override_check, GTK_ALIGN_CENTER);
  gtk_grid_attach(grid, row->override_check, 3, row_index, 1, 1);
  gtk_size_group_add_widget(sg.check, row->override_check);

  /* Determine effective slider value and adjust bounds if needed */
  slider_value = isnan(override_value) ? row->original_value : override_value;

  if( slider_value < row->min_value )
  {
    row->min_value = slider_value;
  }
  else if( slider_value > row->max_value )
  {
    row->max_value = slider_value;
  }

  /* Min entry (column 4, fixed width, 5 decimal places, right-justified) */
  row->min_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->min_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->min_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->min_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->min_entry), 1.0);
  set_entry_value(GTK_ENTRY(row->min_entry), row->min_value);
  gtk_grid_attach(grid, row->min_entry, 4, row_index, 1, 1);
  gtk_size_group_add_widget(sg.min, row->min_entry);

  /* Slider adjustment and widget (column 5, fixed width, no expand) */
  row->adjustment = gtk_adjustment_new(slider_value, row->min_value, row->max_value,
      (row->max_value - row->min_value) / 100.0,
      (row->max_value - row->min_value) / 10.0, 0);
  row->slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, row->adjustment);
  gtk_scale_set_draw_value(GTK_SCALE(row->slider), FALSE);
  gtk_widget_set_size_request(row->slider, 200, -1);
  gtk_widget_set_hexpand(row->slider, FALSE);
  gtk_grid_attach(grid, row->slider, 5, row_index, 1, 1);
  gtk_size_group_add_widget(sg.slider, row->slider);

  /* Max entry (column 6, fixed width, 5 decimal places, right-justified) */
  row->max_entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(row->max_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_entry_set_max_width_chars(GTK_ENTRY(row->max_entry), SY_NUMERIC_WIDTH_CHARS);
  gtk_widget_set_hexpand(row->max_entry, FALSE);
  gtk_entry_set_alignment(GTK_ENTRY(row->max_entry), 1.0);
  set_entry_value(GTK_ENTRY(row->max_entry), row->max_value);
  gtk_grid_attach(grid, row->max_entry, 6, row_index, 1, 1);
  gtk_size_group_add_widget(sg.max, row->max_entry);

  /* Set initial sensitivity based on override checkbox state */
  set_row_sensitivity(row, override_active);

  /* Expression label (column 7, expanding) */
  if( row->is_calculated && expression != NULL && expression[0] != '\0' )
  {
    snprintf(buf, sizeof(buf), "= %s", expression);
    row->expr_label = gtk_label_new(buf);
    gtk_label_set_xalign(GTK_LABEL(row->expr_label), 0.0);
    gtk_widget_set_hexpand(row->expr_label, TRUE);
    gtk_grid_attach(grid, row->expr_label, 7, row_index, 1, 1);
    gtk_size_group_add_widget(sg.expr, row->expr_label);
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
  g_signal_connect(row->override_entry, "focus-out-event",
      G_CALLBACK(on_override_entry_focus_out), row);
  g_signal_connect(row->min_entry, "changed",
      G_CALLBACK(on_min_entry_changed), row);
  g_signal_connect(row->max_entry, "changed",
      G_CALLBACK(on_max_entry_changed), row);

  /* Connect auto-apply signals */
  g_signal_connect(row->override_entry, "changed",
      G_CALLBACK(on_override_value_changed), NULL);
  g_signal_connect(row->override_check, "toggled",
      G_CALLBACK(on_override_value_changed), NULL);

  /* Apply initial strikethrough if override is active */
  update_strikethrough(row);

  if( show_widgets )
  {
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
  }
  else
  {
    /* Widgets hidden initially; shown when expander is expanded */
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

  if( input_expander != NULL )
  {
    gtk_widget_destroy(input_expander);
    input_expander = NULL;
  }

  for( i = 0; i < 8; i++ )
  {
    if( input_col_headers[i] != NULL )
    {
      gtk_widget_destroy(input_col_headers[i]);
      input_col_headers[i] = NULL;
    }
  }

  if( calc_expander != NULL )
  {
    gtk_widget_destroy(calc_expander);
    calc_expander = NULL;
  }

  for( i = 0; i < 8; i++ )
  {
    if( calc_col_headers[i] != NULL )
    {
      gtk_widget_destroy(calc_col_headers[i]);
      calc_col_headers[i] = NULL;
    }
  }
}

/*------------------------------------------------------------------------*/

gboolean
sy_overrides_init(void)
{
  GError *gerror = NULL;

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

  if( sy_overrides_window == NULL || content_grid == NULL )
  {
    pr_err("sy_overrides_init: failed to get widgets from glade\n");
    return FALSE;
  }

  /* Initialize rows array */
  rows = g_ptr_array_new();

  /* Load size groups from glade */
  sg.name = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_name"));
  sg.value = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_value"));
  sg.override = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_override"));
  sg.check = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_check"));
  sg.min = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_min"));
  sg.slider = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_slider"));
  sg.max = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_max"));
  sg.expr = GTK_SIZE_GROUP(gtk_builder_get_object(sy_overrides_builder, "column_sg_expr"));

  /* Setup CSS provider for strikethrough */
  css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(css_provider, "/sy_overrides.css");
  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(),
      GTK_STYLE_PROVIDER(css_provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  /* Get auto-apply widgets */
  auto_apply_check = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_auto_apply"));
  busy_spinner = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_busy_spinner"));
  apply_button = GTK_WIDGET(
      gtk_builder_get_object(sy_overrides_builder, "sy_overrides_apply"));

  if( auto_apply_check != NULL )
  {
    g_signal_connect(auto_apply_check, "toggled",
        G_CALLBACK(on_auto_apply_toggled), NULL);
  }
  else
  {
    /* Auto-apply checkbox not found in glade */
  }

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

  for( i = 0; i < collect_array->len; i++ )
  {
    data = g_ptr_array_index(collect_array, i);

    if( !data->is_calculated && input_expander == NULL )
    {
      input_expander = gtk_expander_new("Input Symbols");
      gtk_expander_set_expanded(GTK_EXPANDER(input_expander), TRUE);
      gtk_widget_set_margin_top(input_expander, 6);
      gtk_widget_set_margin_bottom(input_expander, 2);
      gtk_grid_attach(GTK_GRID(content_grid), input_expander, 0, grid_row, 8, 1);
      gtk_widget_show_all(input_expander);

      g_signal_connect(input_expander, "notify::expanded",
          G_CALLBACK(on_expander_notify_expanded), NULL);

      grid_row++;

      create_column_headers(input_col_headers, GTK_GRID(content_grid), grid_row, 7);
      grid_row++;
    }
    else if( data->is_calculated && calc_expander == NULL )
    {
      calc_expander = gtk_expander_new("Calculated Symbols");
      gtk_expander_set_expanded(GTK_EXPANDER(calc_expander), FALSE);
      gtk_widget_set_margin_top(calc_expander, 6);
      gtk_widget_set_margin_bottom(calc_expander, 2);
      gtk_grid_attach(GTK_GRID(content_grid), calc_expander, 0, grid_row, 8, 1);
      gtk_widget_show_all(calc_expander);

      g_signal_connect(calc_expander, "notify::expanded",
          G_CALLBACK(on_expander_notify_expanded), NULL);

      grid_row++;

      create_column_headers(calc_col_headers, GTK_GRID(content_grid), grid_row, 8);
      grid_row++;
    }
    else
    {
      /* Rows within existing section: no header creation needed */
    }

    row = g_new0(sy_row_t, 1);
    row->name = g_strdup(data->name);
    row->original_value = data->value;
    row->min_value = data->min_value;
    row->max_value = data->max_value;
    row->is_calculated = data->is_calculated;

    populate_row_widgets(row, GTK_GRID(content_grid), grid_row,
        data->expression, data->override_value, data->override_active,
        !data->is_calculated);

    grid_row++;

    g_ptr_array_add(rows, row);
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
  last_applied_hash = compute_override_hash();
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

  memset(&sg, 0, sizeof(sg));

  if( sy_overrides_builder != NULL )
  {
    g_object_unref(sy_overrides_builder);
    sy_overrides_builder = NULL;
  }

  sy_overrides_window = NULL;
  content_grid = NULL;
  input_expander = NULL;
  calc_expander = NULL;
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
  static gboolean new_flag = 0;

  (void)button;
  (void)user_data;

  apply_overrides_to_symbols();
  last_applied_hash = compute_override_hash();
  dirty = FALSE;

  g_idle_add(Open_Input_File, (gpointer)&new_flag);
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
