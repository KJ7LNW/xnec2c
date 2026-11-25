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

/*
 * structure_ui: Structure-window UI readouts.
 *
 * Houses viewer-gain display and the frequency-step and color-code
 * label readouts.
 */
#include "structure_ui.h"
#include "shared.h"
#include "prerender/prerender_color.h"

/*-----------------------------------------------------------------------*/

/**
 * structure_viewer_readout() - Refresh the structure viewer-gain readout
 *
 * Writes the gain in the current structure-view direction.
 */
  void
structure_viewer_readout(void)
{
  g_rec_mutex_lock(&freq_data_lock);

  Show_Viewer_Gain(
      main_window_builder,
      "main_gain_entry",
      structure_view );

  g_rec_mutex_unlock(&freq_data_lock);
}

/*-----------------------------------------------------------------------*/

/**
 * Draw_Structure_UI() - Update frequency-step structure readouts
 *
 * Writes the viewer-gain entry, frequency-step label, and color-code
 * labels for the active frequency step.
 */
  void
Draw_Structure_UI(void)
{
  g_rec_mutex_lock(&freq_data_lock);

  structure_viewer_readout();

  if( calc_data.freq_step >= 0 )
    Display_Fstep( structure_fstep_entry, calc_data.freq_step );

  /* Update structure-window color code labels for current/charge display */
  int fstep = calc_data.freq_step;
  if( CRNT_FSTEP_AVAILABLE(fstep) && struct_colors )
  {
    char maxlabel[16] = "", zerolabel[16] = "";
    gboolean do_update = FALSE;
    double maxval = 0.0;

    if(struct_view_currents())
    {
      maxval = (double)struct_colors[fstep].wire_crnt_cmax * (double)data.wlam;
      do_update = TRUE;
    }
    else if(struct_view_charges())
    {
      maxval = (double)struct_colors[fstep].wire_chrg_cmax
             * 1.0E-6 / (double)calc_data.freq_mhz;
      do_update = TRUE;
    }
    /* else geometry mode: labels retain previous values */

    if( do_update )
    {
      color_tone_t fam = color_tone_active();

      /* Label endpoints per the projection's hue palette */
      switch(chroma_proj_palette_kind(chroma_proj_rows[chroma_proj_active()].hue_enc) )
      {
        case PALETTE_DIVERGING:
          snprintf( maxlabel, sizeof(maxlabel) - 1, "%8.2E", maxval );
          snprintf( zerolabel, sizeof(zerolabel) - 1, "-%.1E", maxval );
          break;

        case PALETTE_CYCLIC:
          snprintf( maxlabel, sizeof(maxlabel) - 1, "%s", "360°" );
          snprintf( zerolabel, sizeof(zerolabel) - 1, "%s", "0°" );
          break;

        case PALETTE_RAMP:
          snprintf( maxlabel, sizeof(maxlabel) - 1, "%8.2E", maxval );
          if( fam == COLOR_TONE_DB )
          {
            /* The dB strip bottoms out at the range floor, not zero */
            tone_param_t tp;
            tone_param_init( &tp, fam );
            snprintf( zerolabel, sizeof(zerolabel) - 1, "%8.2E",
                maxval * tp.floor_ratio );
          }
          else
            snprintf( zerolabel, sizeof(zerolabel) - 1, "%s", "0" );
          break;

        case PALETTE_NUM:
          BUG("unreachable palette kind sentinel\n");
          break;
      }

      gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
              main_window_builder, "main_colorcode_maxlabel")), maxlabel );
      gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
              main_window_builder, "main_colorcode_zerolabel")), zerolabel );
    }
  }
  /* else no current data: labels retain previous values */

  g_rec_mutex_unlock(&freq_data_lock);
}

/*-----------------------------------------------------------------------*/

/* Show_Viewer_Gain()
 *
 * Shows gain in direction of viewer
 */
  void
Show_Viewer_Gain(
    GtkBuilder *builder,
    gchar *widget,
    view_t *v )
{
  if(struct_view_currents() ||
      struct_view_charges()  ||
      rdpat_gain_active()     ||
      freq_sweep_active())
  {
    char txt[16];
    if( isFlagSet(ENABLE_RDPAT) && RDPAT_FSTEP_AVAILABLE(calc_data.freq_step) )
    {
      snprintf( txt, sizeof(txt)-1, "%.2f", Viewer_Gain(v, calc_data.freq_step) );
      gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(builder, widget)), txt );
    }
  }

} /* Show_Viewer_Gain() */

/*-----------------------------------------------------------------------*/
