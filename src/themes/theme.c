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
 * theme: Frequency-plots color-theme registry and per-purpose width config.
 *
 * Built-in themes ship in the GResource themes.ini; the user may override or
 * extend them from ~/.xnec2c/themes.ini.  One designated role table drives
 * INI parsing and missing-key inheritance, so adding a role touches only the
 * theme_t struct and the table.  Line width is the separate global fp_width_t
 * axis loaded from freqplots_widths.ini by the same GKeyFile mechanism.
 */

#include "theme.h"
#include "../shared.h"
#include "../rc_config.h"

/* GResource paths (prefix "/" in resources/xnec2c.gresource.xml). */
#define THEME_RESOURCE  "/themes.ini"
#define WIDTH_RESOURCE  "/freqplots_widths.ini"

/* Single group in the width INI. */
#define WIDTH_GROUP     "widths"

/* The hand-authored theme that every other theme inherits absent keys from. */
#define LEGACY_THEME    "legacy"

/* Suffix marking a theme's inverted variant group; one definition shared by
 * the menu-axis filter and the inverted-key composition. */
#define THEME_INVERTED_SUFFIX  "-inverted"

/* Role -> INI key string, indexed by theme_role_e.  Designated initializers
 * bind each slot to its enum so the parse loop, the missing-key inheritance,
 * and the themes-json2ini.pl emission all derive from this one table; a
 * reordered or missing role is a compile-time gap, not a silent mismatch. */
static const char *const theme_role_key[THEME_ROLE_N] = {
  [THEME_ROLE_BACKGROUND]       = "background",
  [THEME_ROLE_GRID]             = "grid",
  [THEME_ROLE_GRID_PRIMARY]     = "grid_primary",
  [THEME_ROLE_GRID_SECONDARY]   = "grid_secondary",
  [THEME_ROLE_GRID_EMPHASIS]    = "grid_emphasis",
  [THEME_ROLE_GRID_PERIMETER]   = "grid_perimeter",
  [THEME_ROLE_GRID_SCALE]       = "grid_scale",
  [THEME_ROLE_SERIES_PRIMARY]   = "series_primary",
  [THEME_ROLE_SERIES_SECONDARY] = "series_secondary",
  [THEME_ROLE_AXIS]             = "axis",
  [THEME_ROLE_LABEL_PRIMARY]    = "label_primary",
  [THEME_ROLE_LABEL_SECONDARY]  = "label_secondary",
  [THEME_ROLE_LABEL_AXIS]       = "label_axis",
  [THEME_ROLE_TEXT_PRIMARY]     = "text_primary",
  [THEME_ROLE_TEXT_MUTED]       = "text_muted",
  [THEME_ROLE_VIEW_AXIS]        = "view_axis",
  [THEME_ROLE_VIEW_AXIS_LABEL]  = "view_axis_label",
  [THEME_ROLE_MARKER_EXTREME]   = "marker_extreme",
  [THEME_ROLE_CURSOR]           = "cursor",
  [THEME_ROLE_GRAD_0]           = "gradient_0",
  [THEME_ROLE_GRAD_1]           = "gradient_1",
  [THEME_ROLE_GRAD_2]           = "gradient_2",
  [THEME_ROLE_GRAD_3]           = "gradient_3",
  [THEME_ROLE_GRAD_4]           = "gradient_4",
  [THEME_ROLE_GRAD_5]           = "gradient_5",
  [THEME_ROLE_GRAD_MID]         = "gradient_mid",
};

/* Width purpose -> INI key string, indexed by fp_width_e. */
static const char *const fp_width_key[FP_W_N] = {
  [FP_W_GRID]          = "w_grid",
  [FP_W_GRID_EMPHASIS] = "w_grid_emphasis",
  [FP_W_AXIS]          = "w_axis",
  [FP_W_TRACE]         = "w_trace",
  [FP_W_CURSOR]        = "w_cursor",
};

/* Registry: full name -> theme_t*; menu axis: deduped base names in load
 * order; menu_by_base dedups base names across the two source files. */
static GHashTable *theme_by_name;
static GPtrArray  *menu_entries;
static GHashTable *menu_by_base;

/* Role-inheritance source, filled from the built-in [legacy] group. */
static theme_t legacy_defaults;

/* The resolved global width configuration. */
static fp_width_t fp_widths;

/* Transient menu-hover preview override; theme_active resolves this base in
 * place of the committed rc_config selection while a preview is active. */
static char     theme_preview[64];
static gboolean theme_preview_on;

/*-----------------------------------------------------------------------*/

/* Parse an uppercase or lowercase "#RRGGBB" triple into normalised floats.
 * Returns FALSE for any value that is not a six-digit hex color. */
  static gboolean
parse_hex( const char *s, rgb_f_t *out )
{
  guint r, g, b;

  if( s == NULL )
    return FALSE;

  if( sscanf(s, "#%2x%2x%2x", &r, &g, &b) != 3 )
    return FALSE;

  out->r = (float)r / 255.0f;
  out->g = (float)g / 255.0f;
  out->b = (float)b / 255.0f;
  return TRUE;
}

/* Fill every color role of out from group, inheriting def for any absent or
 * malformed key.  A malformed value warns once per occurrence and falls back,
 * never aborts and never silently blanks. */
  static void
theme_fill_roles( GKeyFile *kf, const char *group, const theme_t *def,
    theme_t *out )
{
  theme_role_e r;

  for( r = 0; r < THEME_ROLE_N; r++ )
  {
    if( g_key_file_has_key(kf, group, theme_role_key[r], NULL) )
    {
      char *raw = g_key_file_get_string(kf, group, theme_role_key[r], NULL);

      if( !parse_hex(raw, &out->colors[r]) )
      {
        pr_warn("theme '%s' role '%s' invalid color, using legacy default\n",
            group, theme_role_key[r]);
        out->colors[r] = def->colors[r];
      }

      g_free(raw);
    }
    else
      out->colors[r] = def->colors[r];
  }
}

/* Record a base theme name on the menu axis, deduping across source files.
 * The -inverted variant is a separate toggle and never appears here.  A user
 * redefinition of a built-in base marks the entry user-origin so it sorts
 * into the user partition. */
  static void
menu_record( const char *group, const char *display, gboolean user_origin )
{
  theme_menu_entry_t *e;

  if( g_str_has_suffix(group, THEME_INVERTED_SUFFIX) )
    return;

  e = g_hash_table_lookup(menu_by_base, group);
  if( e != NULL )
  {
    if( user_origin )
      e->user_origin = TRUE;
    return;
  }

  e = g_new0(theme_menu_entry_t, 1);
  e->base_name   = g_strdup(group);
  e->display     = g_strdup(display);
  e->user_origin = user_origin;

  g_ptr_array_add(menu_entries, e);
  g_hash_table_insert(menu_by_base, g_strdup(group), e);
}

/* Build one registry entry from a group, inserting it into the registry
 * (overriding a same-named built-in) and onto the menu axis. */
  static void
theme_add( GKeyFile *kf, const char *group, gboolean user_origin )
{
  theme_t *t       = g_new0(theme_t, 1);
  char    *display = g_key_file_get_string(kf, group, "display", NULL);

  t->name    = g_strdup(group);
  t->display = ( display != NULL ) ? display : g_strdup(group);
  theme_fill_roles(kf, group, &legacy_defaults, t);

  g_hash_table_insert(theme_by_name, g_strdup(group), t);
  menu_record(group, t->display, user_origin);
}

/* Parse every group of one loaded key file into the registry. */
  static void
theme_load_keyfile( GKeyFile *kf, gboolean user_origin )
{
  gchar **groups = g_key_file_get_groups(kf, NULL);
  gsize   i;

  for( i = 0; groups[i] != NULL; i++ )
    theme_add(kf, groups[i], user_origin);

  g_strfreev(groups);
}

/* Load a GResource INI into a fresh GKeyFile; NULL on failure. */
  static GKeyFile *
keyfile_from_resource( const char *res_path )
{
  GError      *err = NULL;
  GBytes      *bytes;
  const char  *data;
  gsize        size;
  GKeyFile    *kf;

  bytes = g_resources_lookup_data(res_path, 0, &err);
  if( bytes == NULL )
  {
    pr_err("theme: cannot load resource %s: %s\n",
        res_path, err ? err->message : "unknown error");
    g_clear_error(&err);
    return NULL;
  }

  data = g_bytes_get_data(bytes, &size);
  kf = g_key_file_new();
  if( !g_key_file_load_from_data(kf, data, size, G_KEY_FILE_NONE, &err) )
  {
    pr_err("theme: cannot parse resource %s: %s\n",
        res_path, err ? err->message : "unknown error");
    g_clear_error(&err);
    g_key_file_free(kf);
    g_bytes_unref(bytes);
    return NULL;
  }

  g_bytes_unref(bytes);
  return kf;
}

/* Load a user INI into a fresh GKeyFile; NULL and silent when the file is
 * absent (normal), NULL and a single warning when present but unparseable. */
  static GKeyFile *
keyfile_from_user( const char *path )
{
  GError   *err = NULL;
  GKeyFile *kf;

  if( access(path, R_OK) != 0 )
    return NULL;

  kf = g_key_file_new();
  if( !g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, &err) )
  {
    pr_warn("theme: cannot parse %s, using built-in defaults\n", path);
    g_clear_error(&err);
    g_key_file_free(kf);
    return NULL;
  }

  return kf;
}

/* Build the path to a per-user config file under ~/.xnec2c/. */
  static char *
user_conf_path( const char *basename )
{
  char home[FILENAME_LEN];

  get_conf_dir(home, sizeof(home));
  return g_strdup_printf("%s/.xnec2c/%s", home, basename);
}

  static void
theme_free( gpointer data )
{
  theme_t *t = data;

  g_free((char *)t->name);
  g_free((char *)t->display);
  g_free(t);
}

  static void
menu_entry_free( gpointer data )
{
  theme_menu_entry_t *e = data;

  g_free((char *)e->base_name);
  g_free((char *)e->display);
  g_free(e);
}

/*-----------------------------------------------------------------------*/

  void
theme_registry_init( void )
{
  GKeyFile *builtin;
  GKeyFile *user;
  char     *user_path;

  if( theme_by_name != NULL )
    g_hash_table_destroy(theme_by_name);
  if( menu_by_base != NULL )
    g_hash_table_destroy(menu_by_base);
  if( menu_entries != NULL )
    g_ptr_array_free(menu_entries, TRUE);

  theme_by_name = g_hash_table_new_full(g_str_hash, g_str_equal,
      g_free, theme_free);
  menu_by_base  = g_hash_table_new_full(g_str_hash, g_str_equal,
      g_free, NULL);
  menu_entries  = g_ptr_array_new_with_free_func(menu_entry_free);

  builtin = keyfile_from_resource(THEME_RESOURCE);
  if( builtin == NULL )
    return;

  /* Establish the inheritance defaults before parsing any group so missing
   * keys resolve at parse time and every stored theme_t is complete. */
  memset(&legacy_defaults, 0, sizeof(legacy_defaults));
  theme_fill_roles(builtin, LEGACY_THEME, &legacy_defaults, &legacy_defaults);

  theme_load_keyfile(builtin, FALSE);
  g_key_file_free(builtin);

  user_path = user_conf_path("themes.ini");
  user = keyfile_from_user(user_path);
  if( user != NULL )
  {
    theme_load_keyfile(user, TRUE);
    g_key_file_free(user);
  }
  g_free(user_path);
}

/* Warn once per (category, name): a theme lacking an inverted variant or a
 * missing base warns the first time it resolves, then stays silent, so a
 * repeated redraw does not flood the log while each distinct offender is
 * still reported. */
  static gboolean
theme_warn_once( const char *category, const char *name )
{
  static GHashTable *seen;
  char               key[160];

  if( seen == NULL )
    seen = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

  snprintf(key, sizeof(key), "%s:%s", category, name);
  if( g_hash_table_contains(seen, key) )
    return FALSE;

  g_hash_table_add(seen, g_strdup(key));
  return TRUE;
}

  const theme_t *
theme_active( void )
{
  const char     *base;
  const theme_t  *t;

  if( theme_preview_on )
    base = theme_preview;
  else
  {
    base = rc_config.freqplots_theme;
    if( base[0] == '\0' )
      base = LEGACY_THEME;
  }

  if( rc_config.freqplots_theme_invert )
  {
    char key[80];

    snprintf(key, sizeof(key), "%s" THEME_INVERTED_SUFFIX, base);
    t = g_hash_table_lookup(theme_by_name, key);
    if( t != NULL )
      return t;

    if( theme_warn_once("invert", base) )
      pr_warn("theme '%s' has no inverted variant, using base\n", base);
  }

  t = g_hash_table_lookup(theme_by_name, base);
  if( t != NULL )
    return t;

  if( theme_warn_once("missing", base) )
    pr_warn("theme '%s' not found, using legacy\n", base);

  return g_hash_table_lookup(theme_by_name, LEGACY_THEME);
}

/* Set the transient hover-preview theme; theme_active resolves it over the
 * committed selection until theme_preview_clear.  A NULL base clears it. */
  void
theme_preview_set( const char *base )
{
  if( base == NULL )
  {
    theme_preview_on = FALSE;
    return;
  }

  g_strlcpy(theme_preview, base, sizeof(theme_preview));
  theme_preview_on = TRUE;
}

/* Drop any active hover preview so theme_active resolves rc_config again. */
  void
theme_preview_clear( void )
{
  theme_preview_on = FALSE;
}

/* Whether a hover preview is currently overriding the committed selection. */
  gboolean
theme_preview_active( void )
{
  return theme_preview_on;
}

  gboolean
theme_has_inverted( const char *base_name )
{
  char key[80];

  if( theme_by_name == NULL )
    return FALSE;

  snprintf(key, sizeof(key), "%s" THEME_INVERTED_SUFFIX, base_name);
  return g_hash_table_lookup(theme_by_name, key) != NULL;
}

  const GPtrArray *
theme_menu_entries( void )
{
  return menu_entries;
}

/*-----------------------------------------------------------------------*/

/* Overlay every width purpose present in kf onto fp_widths; absent keys keep
 * their current value, a malformed value warns once and keeps it. */
  static void
fp_width_overlay( GKeyFile *kf )
{
  fp_width_e p;

  for( p = 0; p < FP_W_N; p++ )
  {
    GError *err = NULL;
    double  v;

    if( !g_key_file_has_key(kf, WIDTH_GROUP, fp_width_key[p], NULL) )
      continue;

    v = g_key_file_get_double(kf, WIDTH_GROUP, fp_width_key[p], &err);
    if( err != NULL )
    {
      pr_warn("width '%s' invalid, keeping current value\n", fp_width_key[p]);
      g_clear_error(&err);
      continue;
    }

    fp_widths.widths[p] = (float)v;
  }
}

  void
fp_width_init( void )
{
  GKeyFile *builtin;
  GKeyFile *user;
  char     *user_path;

  builtin = keyfile_from_resource(WIDTH_RESOURCE);
  if( builtin != NULL )
  {
    fp_width_overlay(builtin);
    g_key_file_free(builtin);
  }

  user_path = user_conf_path("freqplots_widths.ini");
  user = keyfile_from_user(user_path);
  if( user != NULL )
  {
    fp_width_overlay(user);
    g_key_file_free(user);
  }
  g_free(user_path);
}

  const fp_width_t *
fp_width_active( void )
{
  return &fp_widths;
}
