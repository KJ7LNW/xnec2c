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

#include "config_hooks.h"
#include "shared.h"
#include "callbacks.h"
#include "rdpattern_ui.h"
#include "structure_ui.h"
#include "color/color_palette.h"

/* flow_direction_mode_t enumerators for the animatable-mode test */
#include "opengl/opengl_structure.h"

/*------------------------------------------------------------------------*/

void
hook_polarization(void)
{
  Set_Polarization(calc_data.pol_type);
}

/*------------------------------------------------------------------------*/

void
hook_common_projection(void)
{
  opengl_common_projection_sync();
}

/*------------------------------------------------------------------------*/

void
hook_common_pan(void)
{
  Common_Pan_Sync();
}

/*------------------------------------------------------------------------*/

void
hook_flow_direction(void)
{
  gboolean animatable;

  Queue_Structure_Rebuild( TRUE );
  Queue_Radiation_Redraw(TRUE);

  /* Wire color animates in every flow mode; patch arrows animate only in
   * the phase-variant modes, so grey the Animate menu item only for a
   * patch-only model in a phase-invariant mode. */
  animatable = (data.n > 0) ||
    (rc_config.current_flow_visualization_mode == FLOW_DIR_REFERENCE_PHASE ||
     rc_config.current_flow_visualization_mode == FLOW_DIR_LIC ||
     rc_config.current_flow_visualization_mode == FLOW_DIR_WIREFRAME);

  gtk_widget_set_sensitive(
      Builder_Get_Object(main_window_builder, "main_structure_animate"),
      animatable);
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

  /* Track the animate dialog's projection selector label and formula to
   * the selected row and refresh the dialog's copy of the legend strip */
  if( animate_dialog_builder != NULL )
  {
    chroma_proj_t sel = chroma_proj_selected();

    gtk_button_set_label( GTK_BUTTON(Builder_Get_Object(animate_dialog_builder,
            "anim_color_proj_button")),
        gtk_menu_item_get_label( GTK_MENU_ITEM(Builder_Get_Object(
              animate_dialog_builder, chroma_proj_rows[sel].sel_id)) ) );

    gtk_label_set_markup( GTK_LABEL(Builder_Get_Object(animate_dialog_builder,
            "anim_proj_formula")),
        chroma_proj_rows[sel].formula );

    xnec2_widget_queue_draw( Builder_Get_Object(animate_dialog_builder,
        "anim_colorcode_drawingarea"), TRUE );
  }

  anim_overlay_sensitivity();
}

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

    gtk_button_set_label( GTK_BUTTON(Builder_Get_Object(animate_dialog_builder,
            "anim_color_family_button")),
        gtk_menu_item_get_label( GTK_MENU_ITEM(Builder_Get_Object(
              animate_dialog_builder, color_tones[active].sel_id)) ) );
  }

  hook_color_vis();
}

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
}

/*------------------------------------------------------------------------*/

/* Ortho toolbar image entries: builder pointer-to-pointer and image id.
 * The toggle buttons themselves are config_widget elements bound to
 * opengl_orthographic; only the decorative icon swap is hook-owned. */
static const struct
{
  GtkBuilder **builder;
  const gchar  *img_id;
} ortho_toolbars[] = {
  { &main_window_builder,      "main_ortho_image"      },
  { &rdpattern_window_builder, "rdpattern_ortho_image" },
};

void
hook_orthographic(void)
{
  const gchar *icon = rc_config.opengl_orthographic
      ? "/ortho_cube.svg" : "/persp_cube.svg";
  int i;

  for( i = 0; i < (int)(sizeof(ortho_toolbars) / sizeof(ortho_toolbars[0])); i++ )
  {
    GtkWidget *img;

    if( *ortho_toolbars[i].builder == NULL )
      continue;

    img = GTK_WIDGET(gtk_builder_get_object(*ortho_toolbars[i].builder,
        ortho_toolbars[i].img_id));
    if( img != NULL )
      gtk_image_set_from_resource(GTK_IMAGE(img), icon);
  }

  Queue_Structure_Redraw( TRUE );
  Queue_Radiation_Redraw(TRUE);
}

/*------------------------------------------------------------------------*/

void
hook_frequency(void)
{
  /* No frequency data loaded yet (eg config_widget_run_hooks() called from
   * Restore_GUI_State() before any NEC2 file is read); mirrors the guard
   * in freq_step_update_ui(). */
  if( save.freq == NULL )
    return;

  if(freq_sweep_active())
    return;

  if( freq_sweep_armed() )
    return;

  if( rc_config.freq_apply )
    user_set_frequency(calc_data.fmhz_save);
  else
    freq_display_update(calc_data.fmhz_save);
}

/*------------------------------------------------------------------------*/

void
hook_rdpat_e_field(void)
{
  Set_Window_Labels();
  if( rdpat_ehfield_active() )
    Queue_Radiation_Redraw(TRUE);
}

void
hook_rdpat_h_field(void)
{
  Set_Window_Labels();
  if( rdpat_ehfield_active() )
    Queue_Radiation_Redraw(TRUE);
}

void
hook_rdpat_poynting(void)
{
  Set_Window_Labels();
  if( rdpat_ehfield_active() )
    Queue_Radiation_Redraw(TRUE);
}

void
hook_rdpat_overlay(void)
{
  Queue_Radiation_Redraw(TRUE);
}

void
hook_rdpat_gradient_key(void)
{
  Queue_Radiation_Redraw(TRUE);
}

void
hook_rdpat_draw_style(void)
{
  Queue_Radiation_Redraw(TRUE);
}

/*------------------------------------------------------------------------*/

/* freqplots_recount_ngraph - derive the active-plot count from the live
 * rc_config select fields.
 *
 * calc_data.ngraph is a derived value; each select hook is called at most
 * once per real change and unconditionally at window-creation time via
 * config_widget_run_hooks(), so recomputing from the fields (rather than
 * incrementing/decrementing per call) keeps every hook idempotent.
 */
static void
freqplots_recount_ngraph(void)
{
  calc_data.ngraph = freqplots_count_selected();
}

/* freqplots_select_changed - refresh derived plot state after a select toggle
 *
 * Shared body for the eight freqplots select-toggle hooks.
 */
static void
freqplots_select_changed(void)
{
  freqplots_recount_ngraph();

  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void hook_freqplots_gmax(void)
{
  freqplots_select_changed();
}

void hook_freqplots_gdir(void)
{
  freqplots_select_changed();
}

void hook_freqplots_gviewer(void)
{
  freqplots_select_changed();
}

void hook_freqplots_vswr(void)
{
  freqplots_select_changed();
}

void hook_freqplots_zrlzim(void)
{
  freqplots_select_changed();
}

void hook_freqplots_zmgzph(void)
{
  freqplots_select_changed();
}

void hook_freqplots_smith(void)
{
  freqplots_select_changed();
}

void hook_freqplots_ant_temp(void)
{
  freqplots_select_changed();
}

void
hook_freqplots_net_gain(void)
{
  /* Net gain gates the gain and viewer popups' port pull-downs, so re-gate
   * every open port-aware combo when the setting flips. */
  freqplots_refresh_port_combos();

  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void
hook_freqplots_min_max(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void
hook_freqplots_clamp_vswr(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void
hook_freqplots_show_ant_temp(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void
hook_freqplots_round_x_axis(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

void
hook_freqplots_swap_click(void)
{
  /* Affects only the interpretation of future clicks; no redraw. */
}

void
hook_freqplots_s11(void)
{
  if( isFlagSet(PLOT_ENABLED) && freq_sweep_has_results())
    freqplots_redraw_all(TRUE);
}

/*------------------------------------------------------------------------*/

/* Apply-frequency checkbutton tree: session-only, no persistence row.
 * File-scope storage so the binding registry holds a pointer with static
 * lifetime; an inline compound literal would die when config_hooks_init()
 * returns. */
static const config_widget_tree_t *const freq_apply_tree =
  CONFIG_WIDGET_TREE( .groups = CONFIG_WIDGET_GROUPS(
    CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
      .elements = CONFIG_WIDGETS(
        CONFIG_WIDGET( .widget_id = "main_freq_checkbutton" ), NULL ) ),
    CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
      .elements = CONFIG_WIDGETS(
        CONFIG_WIDGET( .widget_id = "rdpattern_freq_checkbutton" ), NULL ) ),
    NULL ) );

void
config_hooks_init(void)
{
  config_widget_register( &rc_config.freq_apply, sizeof(rc_config.freq_apply),
    freq_apply_tree );
}
