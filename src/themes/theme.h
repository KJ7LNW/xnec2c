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

#ifndef THEME_H
#define THEME_H  1

#include "../common.h"

/* g_object_data keys binding a theme menu item to its base name and the
 * Inverted item it governs; set by the menu builder, read by the handlers. */
#define THEME_DATA_BASE         "theme_base"
#define THEME_DATA_INVERT_ITEM  "theme_invert_item"

/* Bind every themable visual role to one immutable rendering color theme.
 * Define roles by visual function and resolve each through the indexed colors
 * array.  Supply backend leaf functions with plain colors from their frame
 * owner.  Keep line width on the separate global fp_width_t axis below.  Treat
 * name and display as identity metadata, index colors by theme_role_e, and bind
 * each slot to its INI key through theme.c's theme_role_key[]. */
typedef enum
{
  THEME_ROLE_BACKGROUND,
  THEME_ROLE_GRID,
  THEME_ROLE_GRID_PRIMARY,
  THEME_ROLE_GRID_SECONDARY,
  THEME_ROLE_GRID_EMPHASIS,
  THEME_ROLE_GRID_PERIMETER,
  THEME_ROLE_GRID_SCALE,
  THEME_ROLE_SERIES_PRIMARY,
  THEME_ROLE_SERIES_SECONDARY,
  THEME_ROLE_AXIS,
  THEME_ROLE_LABEL_PRIMARY,
  THEME_ROLE_LABEL_SECONDARY,
  THEME_ROLE_LABEL_AXIS,
  THEME_ROLE_TEXT_PRIMARY,   /* legible readout text over the view background */
  THEME_ROLE_TEXT_MUTED,     /* de-emphasized readout text */
  THEME_ROLE_VIEW_AXIS,
  THEME_ROLE_VIEW_AXIS_LABEL,
  THEME_ROLE_MARKER_EXTREME,
  THEME_ROLE_CURSOR,
  THEME_ROLE_GRAD_0,   /* magnitude low / cold pole / phase seam (legacy magenta) */
  THEME_ROLE_GRAD_1,   /* legacy blue */
  THEME_ROLE_GRAD_2,   /* legacy cyan */
  THEME_ROLE_GRAD_3,   /* legacy green */
  THEME_ROLE_GRAD_4,   /* legacy yellow */
  THEME_ROLE_GRAD_5,   /* magnitude high / hot pole (legacy red) */
  THEME_ROLE_GRAD_MID, /* diverging neutral center (legacy white) */
  THEME_ROLE_N
} theme_role_e;

typedef struct
{
  const char *name;
  const char *display;
  rgb_f_t     colors[THEME_ROLE_N];
} theme_t;

/* Per-purpose line-width roles indexing fp_width_t.widths; theme.c's
 * fp_width_key[] binds each slot to its width INI key. */
typedef enum
{
  FP_W_GRID,
  FP_W_GRID_EMPHASIS,
  FP_W_AXIS,
  FP_W_TRACE,
  FP_W_CURSOR,
  FP_W_N
} fp_width_e;

/* Per-purpose line width, the global axis shared by every theme.  The
 * per-panel fp_density_t scale multiplies these at draw time. */
typedef struct
{
  float widths[FP_W_N];
} fp_width_t;

/* One menu axis record: a base theme name with its origin.  The -inverted
 * variant is a separate orthogonal toggle, so it never appears here; user
 * origin marks the menu separator boundary. */
typedef struct
{
  const char *base_name;
  const char *display;
  gboolean    user_origin;
} theme_menu_entry_t;

/* Load the built-in theme INI from GResource then overlay the user INI from
 * the config directory, unioning user entries over built-ins.  Missing role
 * keys inherit the legacy theme's value so every resolved theme_t is
 * complete.  Idempotent: a second call rebuilds the registry. */
void theme_registry_init(void);

/* Resolve the active theme from rc_config.freqplots_theme and
 * rc_config.freqplots_theme_invert.  Composes the lookup key
 * (<name>, or <name>-inverted when the invert flag is set) and returns the
 * matching theme; falls back to legacy (warning once) for an absent base or
 * an absent -inverted variant. */
const theme_t *theme_active(void);

/* Transient menu-hover preview.  theme_active resolves the preview base over
 * rc_config until theme_preview_clear (a committed selection or a menu
 * collapse).  A preview never writes the committed rc_config selection. */
void theme_preview_set(const char *base);
void theme_preview_clear(void);
gboolean theme_preview_active(void);

/* Whether a -inverted registry entry exists for a base theme name. */
gboolean theme_has_inverted(const char *base_name);

/* Ordered menu axis: base theme names (no -inverted), deduped, in load
 * order with origin tags.  The menu builder partitions user before built-in
 * and draws the separator at the boundary. */
const GPtrArray *theme_menu_entries(void);

/* Load the per-purpose width defaults from GResource then overlay the user
 * width INI; present user keys override, missing keys keep the resource
 * defaults.  Idempotent. */
void fp_width_init(void);

/* The resolved global width configuration. */
const fp_width_t *fp_width_active(void);

#endif /* THEME_H */
