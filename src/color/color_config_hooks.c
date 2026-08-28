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
#include "../shared.h"
#include "../callbacks.h"
#include "../rdpattern_ui.h"
#include "../structure_ui.h"
#include "color_palette.h"

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
  GtkWidget *comet, *nodes;

  if( animate_dialog_builder == NULL )
    return;

  animated  = chroma_proj_animated(chroma_proj_selected());
  has_wires = (data.n > 0);

  comet = GTK_WIDGET(Builder_Get_Object(animate_dialog_builder, "anim_overlay_comet"));
  nodes = GTK_WIDGET(Builder_Get_Object(animate_dialog_builder, "anim_overlay_nodes"));

  gtk_widget_set_sensitive( comet, animated && has_wires );
  gtk_widget_set_sensitive( nodes, has_wires );

  if( !has_wires )
  {
    gtk_widget_set_tooltip_text( comet, no_wire );
    gtk_widget_set_tooltip_text( nodes, no_wire );
  }
  else
  {
    gtk_widget_set_tooltip_text( comet, animated
        ? _("Highlight the moving wave crest as a bright comet head.")
        : _("Comet rides the animated wave crest; this projection is a static"
            " read with no moving phase.") );
    gtk_widget_set_tooltip_text( nodes,
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
