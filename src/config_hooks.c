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
#include "anim/anim_dialog.h"

/*------------------------------------------------------------------------*/

void
hook_polarization(void)
{
  polarization_refresh();

  /* The far-zone reference radios turn the linear pair of gain selections,
   * so their availability follows this selection */
  anim_panel_sensitivity();
}

const config_refresh_t hook_polarization_refresh =
  { .fn = hook_polarization, .cls = REFRESH_HOVER_SAFE };

/*------------------------------------------------------------------------*/

void
hook_common_projection(void)
{
  opengl_common_projection_sync();
}

/* Sharing or unsharing the view master leaves the views where it put them,
 * so restoring the field does not restore the former camera */
const config_refresh_t hook_common_projection_refresh =
  { .fn = hook_common_projection, .cls = REFRESH_COMMIT_ONLY };

/*------------------------------------------------------------------------*/

void
hook_common_pan(void)
{
  Common_Pan_Sync();
}

/* The pan offset copied into the pattern view survives a revert of the
 * field, which would relocate that camera permanently */
const config_refresh_t hook_common_pan_refresh =
  { .fn = hook_common_pan, .cls = REFRESH_COMMIT_ONLY };

/*------------------------------------------------------------------------*/

void
hook_flow_direction(void)
{
  Queue_Structure_Rebuild( TRUE );
  Queue_Radiation_Redraw(TRUE);

  /* The selected mark decides whether the patch class carries phase, so the
   * Animate entries re-read their class truth on this edge. */
  anim_panel_sensitivity();
}

const config_refresh_t hook_flow_direction_refresh =
  { .fn = hook_flow_direction, .cls = REFRESH_HOVER_SAFE };

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

/* Swapping the projection re-frames the structure and the pattern together,
 * so the selection reaches both views on its commit alone */
const config_refresh_t hook_orthographic_refresh =
  { .fn = hook_orthographic, .cls = REFRESH_COMMIT_ONLY };

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
hook_rdpat_ehfield(void)
{
  Set_Window_Labels();
  if( rdpat_ehfield_active() )
    Queue_Radiation_Redraw(TRUE);

  /* The channel selections decide whether the near-field class presents
   * content, so the animation controls re-read that truth here. */
  anim_panel_sensitivity();
}

const config_refresh_t hook_rdpat_ehfield_refresh =
  { .fn = hook_rdpat_ehfield, .cls = REFRESH_HOVER_SAFE };

void
hook_rdpat_redraw(void)
{
  Queue_Radiation_Redraw(TRUE);
}

const config_refresh_t hook_rdpat_redraw_refresh =
  { .fn = hook_rdpat_redraw, .cls = REFRESH_HOVER_SAFE };

/* The far-zone overlay selection admits the vectors into the pattern frame
 * and governs whether the far-field panel accepts input, so its edge both
 * redraws the pattern and re-greys the panel. */
void
hook_ff_overlay(void)
{
  hook_rdpat_redraw();
  anim_panel_sensitivity();
}

const config_refresh_t hook_ff_overlay_refresh =
  { .fn = hook_ff_overlay, .cls = REFRESH_HOVER_SAFE };

/* Antenna elevation tilts the structure against the sky and earth noise
 * models and turns the pattern in the viewer, so every consumer derived
 * from the active step re-reads it. */
void
hook_ant_temp(void)
{
  freq_step_refresh_ui(TRUE);
}

const config_refresh_t hook_ant_temp_refresh =
  { .fn = hook_ant_temp, .cls = REFRESH_HOVER_SAFE };

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
