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
 * structure_ui: Structure-window UI helpers and initialization.
 *
 * Houses viewer-gain display, frequency-step current buffers,
 * structure redraw queuing, and Init_Struct_Drawing which wires
 * prerender to the 2D projection buffer.
 */
#include "structure_ui.h"
#include "shared.h"
#include "config_hooks.h"
#include "callbacks.h"
#include "cairo/cairo_draw.h"
#include "prerender/prerender_aggregate.h"
#include "prerender/prerender_color.h"
#include "view/view_core.h"

/*-----------------------------------------------------------------------*/

/**
 * Draw_Structure_UI() - Update structure-window UI readouts
 *
 * Writes the viewer-gain entry, frequency-step label, and color-code
 * max label for the main window.  Called under freq_data_lock by
 * render() after dispatching to the backend.
 */
  void
Draw_Structure_UI(void)
{
  Show_Viewer_Gain(
      main_window_builder,
      "main_gain_entry",
      structure_view );

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
      switch(chroma_proj_palette_kind(chroma_proj_rows[color_proj_active()].hue_enc) )
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
      isFlagSet(FREQ_LOOP_RUNNING) )
  {
    char txt[16];
    if( isFlagSet(ENABLE_RDPAT) && (calc_data.freq_step >= 0) )
    {
      snprintf( txt, sizeof(txt)-1, "%.2f", Viewer_Gain(v, calc_data.freq_step) );
      gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(builder, widget)), txt );
    }
  }

} /* Show_Viewer_Gain() */

/*-----------------------------------------------------------------------*/

/**
 * free_crnt_step() - Release one fstep's crnt sub-buffers
 * @elem: pointer to one crnt_t element
 */
static void
free_crnt_step(void *elem)
{
  crnt_t *c = elem;
  mem_array_free(&c->air);
  mem_array_free(&c->aii);
  mem_array_free(&c->bir);
  mem_array_free(&c->bii);
  mem_array_free(&c->cir);
  mem_array_free(&c->cii);
  mem_array_free(&c->cur);
}

/*-----------------------------------------------------------------------*/

/**
 * Alloc_Crnt_Fstep_Buffers() - Allocate per-frequency-step crnt storage
 *
 * @nfrq: Number of frequency steps (steps_total + 1)
 *
 * Allocates crnt_fstep[] array so each frequency step holds the
 * current/charge data its solve writes.
 */
  void
Alloc_Crnt_Fstep_Buffers( int nfrq )
{
  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their sub-buffers for reuse by the inner alloc loop. */
  mem_array_resize(&crnt_fstep, nfrq, free_crnt_step);

  for( int i = 0; i < nfrq; i++ )
  {
    mem_array_realloc(&crnt_fstep[i].air, data.npm);
    mem_array_realloc(&crnt_fstep[i].aii, data.npm);
    mem_array_realloc(&crnt_fstep[i].bir, data.npm);
    mem_array_realloc(&crnt_fstep[i].bii, data.npm);
    mem_array_realloc(&crnt_fstep[i].cir, data.npm);
    mem_array_realloc(&crnt_fstep[i].cii, data.npm);
    mem_array_realloc(&crnt_fstep[i].cur, data.np3m);

  }

  alloc_struct_colors(nfrq);

} /* Alloc_Crnt_Fstep_Buffers() */

/*-----------------------------------------------------------------------*/

/*  Queue_Structure_Redraw()
 *
 *  Queues a redraw of the structure drawingarea and, when a
 *  "viewer gain" plot is active, the frequency-plot area.
 *
 *  Called by view_t observers when rotation, pan or zoom change.
 *  No projection-cache state is maintained here: view_R(), the
 *  pan_offset and the zoom are read directly at draw time.
 */
  void
Queue_Structure_Redraw(void)
{
  /* Trigger a redraw of structure drawingarea */
  if( structure_drawingarea )
    xnec2_widget_queue_draw( structure_drawingarea, TRUE );

  /* Trigger a redraw of plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) &&
      (rc_config.freqplots_gviewer_togglebutton || freqplots_popup_open(FP_PANEL_VIEWER)) &&
      isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    freqplots_redraw_all(TRUE);
  }

} /* Queue_Structure_Redraw() */

/*-----------------------------------------------------------------------*/

/** structure_view_changed_cb() - view_t change callback for structure view
 * @v:           view that changed
 * @_user_data:  unused
 *
 * Invoked by view_notify_change() whenever rotation, pan, zoom, or extent
 * changes.  Refreshes the WR/WI spin display and queues a structure redraw.
 * Propagation to the rdpattern view under common-projection sharing is
 * handled internally by view_t via the rotation_follower link established
 * by view_share_master().  Bound as changed_cb at view_new() in main.c.
 */
  void
structure_view_changed_cb(view_t *v, gpointer _user_data)
{
  (void)_user_data;

  view_update_spin_display( v );
  Queue_Structure_Redraw();

} /* structure_view_changed_cb() */

/*-----------------------------------------------------------------------*/

/** Animate_Phase() - Unified animation tick callback
 * @_udata: unused
 *
 * Returns G_SOURCE_REMOVE immediately when ANIMATE is cleared.
 * Otherwise advances flow_phase by one flow_phase_step, dispatches to all
 * active animation consumers, then queues redraws.  Animate_Phase owns
 * all queue decisions.
 */
  gboolean
Animate_Phase(gpointer _udata)
{
  if( isFlagClear(ANIMATE) )
  {
    anim_tag = 0;
    return( G_SOURCE_REMOVE );
  }

  flow_phase += (float)flow_phase_step;
  if( flow_phase >= (float)M_2PI )
    flow_phase -= (float)M_2PI;

  apply_animation_phase();

  return( G_SOURCE_CONTINUE );

} /* Animate_Phase() */

/*-----------------------------------------------------------------------*/

/** apply_animation_phase() - Render structure and pattern at the current phase
 *
 * Shared by the timer tick and the manual phase slider.  Reads flow_phase
 * without modifying it: queues the structure drawing area and the
 * radiation-pattern drawing area for near-field or patch-arrow overlay.  The
 * draw-time resolver derives the near-field frame at flow_phase.
 */
  void
apply_animation_phase(void)
{
  xnec2_widget_queue_draw( structure_drawingarea, TRUE );

  /* Queue rdpattern for near-field visualization or structure overlay */
  if(rdpat_ehfield_active() || overlay_struct_active() )
    xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );

  /* Update the phase readout from flow_phase without moving the slider, whose
   * position stays static while the timer animation runs.  The readout is
   * decoupled from the slider thumb.  Slider reads degrees; flow_phase stores
   * radians. */
  if( animate_dialog != NULL )
  {
    GtkLabel *readout = GTK_LABEL( Builder_Get_Object(
          animate_dialog_builder, "animate_phase_value") );
    gchar *text = g_strdup_printf( "φ %.0f°", (gdouble)flow_phase * TODEG );
    gtk_label_set_text( readout, text );
    g_free( text );
  }

} /* apply_animation_phase() */

/*-----------------------------------------------------------------------*/

/* Manual phase-slider interaction since the last reset; together with the
 * ANIMATE timer flag this forms the playback-live state. */
static gboolean animation_scrubbed = FALSE;

/** reset_animation_phase() - Zero the shared animation phase
 *
 * Called when animation stops to return arrows to reference direction
 * and end playback-liveness.
 */
  void
reset_animation_phase(void)
{
  flow_phase = 0.0f;
  animation_scrubbed = FALSE;
}

/*-----------------------------------------------------------------------*/

/** animation_set_scrubbed() - Mark manual phase scrubbing as playback
 *
 * Called by the phase-slider handlers; cleared by reset_animation_phase().
 * The rising edge swaps in the animated projection, including the legend;
 * later scrubs redraw through the phase change alone.
 */
  void
animation_set_scrubbed(void)
{
  if( animation_scrubbed )
    return;

  animation_scrubbed = TRUE;
  hook_color_vis();
}

/*-----------------------------------------------------------------------*/

/** animation_is_active() - Report whether the animated selection applies
 *
 * Playback-live boundary: the animate dialog is open and the phase is
 * advancing (ANIMATE timer running or the slider scrubbed).  Opening the
 * dialog alone leaves the static amplitude baseline in effect, so no
 * color change precedes the user starting playback.
 */
  gboolean
animation_is_active(void)
{
  return (animate_dialog != NULL)
      && (isFlagSet(ANIMATE) || animation_scrubbed);
}

/*-----------------------------------------------------------------------*/

/** color_proj_active() - Resolve the color projection now in effect
 *
 * A live menu-hover preview renders at once; otherwise the animated
 * projection applies while animation playback is live, else the static
 * amplitude baseline.
 */
  chroma_proj_t
color_proj_active(void)
{
  return (chroma_proj_preview_active() || animation_is_active())
      ? chroma_proj_selected()
      : CHROMA_PROJ_AMPLITUDE;
}

/*-----------------------------------------------------------------------*/

/*  Init_Struct_Drawing()
 *
 *  Initializes drawing parameters after geometry input
 */
  void
Init_Struct_Drawing( void )
{
  mem_array_realloc(&structure_segs, (data.n + 4 * data.m));
  Prerender_Aggregate();

  /* Viewport only exists in the UI process; children skip this harmlessly.
   * Guard: skip if no geometry loaded (segments or patches required). */
  if( (data.n > 0 || data.m > 0) && structure_view != NULL )
    view_set_viewport( structure_view, structure_width, structure_height );
}

/*-----------------------------------------------------------------------*/
