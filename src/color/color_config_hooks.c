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

/* color_config_hooks: change-edge hooks of the color-projection, tone-family
 * and theme selections, with the hover classification each one carries. */

#include "../config_hooks.h"
#include "../anim/anim_class.h"
#include "../anim/anim_dialog.h"
#include "../shared.h"
#include "../callbacks.h"
#include "../config/widget/config_widget_ops.h"
#include "../rdpattern_ui.h"
#include "../structure_ui.h"
#include "color_palette.h"
#include "color_tone.h"

/* Snap window around a slider mark as a fraction of the slider span */
#define FAM_MARK_SNAP_FRACTION 0.025

/*------------------------------------------------------------------------*/

/** on_color_fam_format_value() - Render a slider value as the natural parameter
 * @_scale: emitting family slider, unused
 * @value: slider-domain value
 * @user_data: the slider's color_tone_row_t
 *
 * Returns a heap string in the family's natural-parameter format; GTK
 * frees it after display.
 */
  static gchar *
on_color_fam_format_value(GtkScale *_scale, gdouble value,
    gpointer user_data)
{
  const color_tone_row_t *row = user_data;

  return g_strdup_printf(row->value_fmt, row->param_map(value));
}

/** on_color_fam_change_value() - Snap a family slider onto its marks
 * @range: emitting family slider
 * @_scroll: scroll type, unused
 * @value: proposed slider-domain value
 * @user_data: the slider's color_tone_row_t
 *
 * Snaps within FAM_MARK_SNAP_FRACTION of the slider span onto the row's
 * mark; the set_value call emits the final value-changed.
 */
  static gboolean
on_color_fam_change_value(GtkRange *range, GtkScrollType _scroll,
    gdouble value, gpointer user_data)
{
  const color_tone_row_t *row = user_data;
  GtkAdjustment *adj = gtk_range_get_adjustment(range);
  double span = gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj);
  gboolean snapped = FALSE;
  int i;

  for( i = 0; !snapped && !isnan(row->marks[i]); i++ )
    if( !dl_fgt(fabs(value - row->marks[i]),
          span * FAM_MARK_SNAP_FRACTION - DL_EPS) )
    {
      gtk_range_set_value(range, row->marks[i]);
      snapped = TRUE;
    }

  return snapped;
}

/** color_tone_marks_attach() - Mark and wire the family sliders at creation
 *
 * Adds each family's snap marks labeled with the natural parameter and
 * connects the shared snap and value-format handlers with the row as
 * user data.
 */
  void
color_tone_marks_attach(void)
{
  int fam, i;
  char label[32];

  for( fam = 0; fam < COLOR_TONE_NUM; fam++ )
  {
    const color_tone_row_t *row = &color_tones[fam];
    GtkScale *scale;

    if( row->scale_id == NULL || row->marks == NULL )
      continue;

    scale = GTK_SCALE(Builder_Get_Object(animate_dialog_builder,
          row->scale_id));

    for( i = 0; !isnan(row->marks[i]); i++ )
    {
      snprintf(label, sizeof(label), "%g", row->param_map(row->marks[i]));
      gtk_scale_add_mark(scale, row->marks[i], GTK_POS_BOTTOM, label);
    }

    g_signal_connect(scale, "change-value",
        G_CALLBACK(on_color_fam_change_value), (gpointer)row);
    g_signal_connect(scale, "format-value",
        G_CALLBACK(on_color_fam_format_value), (gpointer)row);
  }
}

/*------------------------------------------------------------------------*/

/* Gate the wire overlay checkboxes by projection class and wire presence.
 * Comet needs a phase-varying projection; both overlays need wire-current
 * segments.  A disabled control carries a tooltip naming the reason.
 * Single source called from hook_color_vis (projection edge) and from
 * config_widget_run_hooks on dialog open. */
static void
anim_overlay_sensitivity(void)
{
  const char *no_wire = _("This overlay derives from wire-current segments;"
      " this model has none.");
  gboolean animated, has_wires;

  if( animate_dialog_builder == NULL )
    return;

  animated  = chroma_proj_animated(chroma_proj_selected());
  has_wires = anim_class_available(ANIM_CLASS_STRUCTURE_SEGMENT);

  config_widget_set_sensitive( &rc_config.overlay_comet,
      animated && has_wires );
  config_widget_set_sensitive( &rc_config.overlay_nodes, has_wires );

  if( !has_wires )
  {
    config_widget_set_tooltip( &rc_config.overlay_comet, no_wire );
    config_widget_set_tooltip( &rc_config.overlay_nodes, no_wire );
  }
  else
  {
    config_widget_set_tooltip( &rc_config.overlay_comet, animated
        ? _("Highlight the moving wave crest as a bright comet head.")
        : _("Comet rides the animated wave crest; this projection is a static"
            " read with no moving phase.") );
    config_widget_set_tooltip( &rc_config.overlay_nodes,
        _("Mark current nodes (cyan) and antinodes (red) along the wires.") );
  }
}

/** hook_color_vis() - Rebake baked colors and redraw after a
 * color projection or scale change, including the legend strips.
 *
 * The projection and tone family also select the structure color-code label
 * format, so the rebake and redraw run through the step-derived refresh path
 * that owns those readouts.  The frequency step is unchanged here, so the
 * frequency widgets keep the value the operator entered.
 */
void
hook_color_vis(void)
{
  freq_step_refresh_ui( TRUE );

  /* The legend strips live outside the structure and pattern drawing areas */
  if( main_window_builder != NULL )
    xnec2_widget_queue_draw( Builder_Get_Object(main_window_builder,
        "main_colorcode_drawingarea"), TRUE );

  if( rdpattern_window_builder != NULL )
    xnec2_widget_queue_draw( Builder_Get_Object(rdpattern_window_builder,
        "rdpattern_colorcode_drawingarea"), TRUE );

  /* Track the animate dialog's projection and segment scale formulas to their
   * selected rows and refresh the dialog's copy of the legend strip */
  if( animate_dialog_builder != NULL )
  {
    chroma_proj_t sel = chroma_proj_selected();

    gtk_label_set_markup( GTK_LABEL(Builder_Get_Object(animate_dialog_builder,
            "anim_proj_formula")),
        chroma_proj_rows[sel].formula );

    gtk_label_set_markup( GTK_LABEL(Builder_Get_Object(animate_dialog_builder,
            "anim_seg_scale_formula")),
        seg_scale_enc_rows[seg_scale_enc_selected()].formula );

    xnec2_widget_queue_draw( Builder_Get_Object(animate_dialog_builder,
        "anim_colorcode_drawingarea"), TRUE );
  }

  anim_overlay_sensitivity();
  anim_panel_sensitivity();
}

const config_refresh_t hook_color_vis_refresh =
  { .fn = hook_color_vis, .cls = REFRESH_HOVER_SAFE };

/*------------------------------------------------------------------------*/

/** hook_color_family() - Swap the active family's slider row and formula
 *
 * Shows only the active family's brightness-slider row in the animate
 * dialog, renders its closed-form transfer in the formula label, then
 * rebakes colors via hook_color_vis().
 */
void
hook_color_family(void)
{
  color_tone_t active = color_tone_active();
  int fam;

  if( animate_dialog_builder != NULL )
  {
    for( fam = 0; fam < COLOR_TONE_NUM; fam++ )
      gtk_widget_set_visible(
          GTK_WIDGET(Builder_Get_Object(animate_dialog_builder,
              color_tones[fam].row_id)),
          fam == (int)active );

    gtk_label_set_markup( GTK_LABEL(Builder_Get_Object(animate_dialog_builder,
            "anim_scale_formula")),
        color_tones[active].formula );
  }

  hook_color_vis();
}

const config_refresh_t hook_color_family_refresh =
  { .fn = hook_color_family, .cls = REFRESH_HOVER_SAFE };

/*------------------------------------------------------------------------*/

/** hook_theme_change() - Refresh everything derived from the active theme
 *
 * Single unified refresh for every theme-derived consumer: rebuild the
 * palette LUTs from the theme's gradient roles, rebake and redraw the
 * color-projected surfaces and legend strips, then repaint the frequency
 * plots through the redraw orchestration path.  Every theme commit,
 * preview, and revert edge calls this one function; a new theme-derived
 * consumer adds its refresh here.
 */
void
hook_theme_change(void)
{
  palette_registry_init();
  hook_color_vis();

  /* The Inverted item offers itself only where the rendered base theme
   * carries an inverted variant */
  freqplots_theme_invert_sync();
}

const config_refresh_t hook_theme_change_refresh =
  { .fn = hook_theme_change, .cls = REFRESH_HOVER_SAFE };
