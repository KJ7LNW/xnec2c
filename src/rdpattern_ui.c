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

#include "rdpattern_ui.h"
#include "gdk_scroll.h"
#include "measurements.h"
#include "rc_config.h"
#include "render/render_dispatch.h"
#include "render/render_surface.h"
#include "shared.h"
#include "structure_ui.h"

#ifdef HAVE_OPENGL
#include "opengl/opengl_rdpattern.h"
#endif

/* Constants for display layout */



static const char *nearfield_animation_error_msg =
  N_("Animation requires near field data.\n\n"
     "E-field animation: Add NE card to NEC file\n"
     "H-field animation: Add NH card to NEC file\n"
     "Poynting vector: Add both NE and NH cards");

/*-----------------------------------------------------------------------*/

/* Scale_Gain()
 *
 * Scales radiation pattern gain according to selected style
 * ( ARRL style, logarithmic or linear voltage/power )
 */
double Scale_Gain( double gain, int fstep, int idx )
{
  double t_sky = 0.0, t_earth = 0.0;

  /* Resolve noise temperatures from per-fstep table when in noise mode.
   * Custom model slots store sentinel -1.0; substitute rc_config values. */
  if( IS_NOISE_MODE(rc_config.gain_style) )
    noise_temp_resolve(fstep, &t_sky, &t_earth);

  gain += Polarization_Factor( calc_data.pol_type, fstep, idx );
  return Scale_Gain_Resolved( gain, fstep, idx, t_sky, t_earth );

} /* Scale_Gain() */

/*-----------------------------------------------------------------------*/

/**
 * Scale_Gain_Resolved() - Map a gain value to a pattern radius
 * @gain:    raw gain value (dBi) before polarization factor
 * @fstep:   frequency step index
 * @idx:     pattern vertex index
 * @t_sky:   sky noise temperature (K); read only when gain_style is a noise mode
 * @t_earth: earth noise temperature (K); read only when gain_style is a noise mode
 *
 * Applies the active gain style (rc_config.gain_style) to convert a raw gain
 * value into a pattern radius.  Noise-mode callers supply pre-resolved t_sky
 * and t_earth from noise_temp[fstep]; non-noise callers pass 0.0 for both.
 */
  double
Scale_Gain_Resolved(double gain, int fstep, int idx,
    double t_sky, double t_earth)
{
  double scaled_rad = 0.0;

  int gs = rc_config.gain_style;

  /* Clamp unrecognized gain styles, future styles add cases below; callers
   * with older Scale_Gain get safe fallback */
  if( gs < 0 || gs >= NUM_SCALES )
  {
    static gboolean warned = FALSE;
    if( !warned )
    {
      warned = TRUE;
      pr_err("Scale_Gain_Resolved: gain_style %d out of range [0..%d], defaulting to GS_LINP\n",
          gs, NUM_SCALES - 1);
    }
    gs = rc_config_default_int(&rc_config.gain_style);
  }

  switch( gs )
  {
    case GS_LINP:
      scaled_rad = pow(10.0, (gain / 10.0));
      break;

    case GS_LINV:
      scaled_rad = pow(10.0, (gain / 20.0));
      break;

    case GS_ARRL:
      scaled_rad = exp(0.058267 * gain);
      break;

    case GS_LOG:
      scaled_rad = gain;
      if( scaled_rad < -40 )
        scaled_rad = 0.0;
      else
        scaled_rad = scaled_rad / 40.0 + 1.0;
      break;

    /* Noise temperature gain scales */
    case GS_NOISE:
    case GS_NOISE_LOG:
      {
        int ith = idx % fpat.nth;
        int iph = idx / fpat.nth;
        double tht = (fpat.thets + ith * fpat.dth) * M_PI / 180.0;
        double phi = (fpat.phis  + iph * fpat.dph) * M_PI / 180.0;

        int pol = calc_data.pol_type;
        double tht_mg  = rad_pattern[fstep].max_gain_tht[pol] * M_PI / 180.0;
        double phi_mg  = rad_pattern[fstep].max_gain_phi[pol] * M_PI / 180.0;
        double elev_rad = rc_config.ant_temp_elevation * M_PI / 180.0;

        double z_w = ant_temp_z_world(tht, phi, tht_mg, phi_mg, elev_rad);

        /* Blend t_sky/t_earth across one angular step to avoid a hard
         * boundary discontinuity that appears as spikes in log-scale mode. */
        double half_w   = 0.5 * fmax(fpat.dth, fpat.dph) * M_PI / 180.0;
        double alpha    = fmax(0.0, fmin(1.0, (z_w + half_w) / (2.0 * half_w)));
        double t_bright = alpha * t_sky + (1.0 - alpha) * t_earth;
        double g_lin    = pow(10.0, gain / 10.0);

        /* Noise temperature density (K/sr): resolution-independent.
         * fabs(sin(tht)): NEC2 allows tht > 180 deg as a coordinate
         * convenience; (360-tht, phi+180) is the same physical direction
         * with positive sin.  The solid angle element dOmega =
         * sin(tht)dthdphi non-negative */
        double cell_k = g_lin * t_bright * fabs(sin(tht)) / (4.0 * M_PI);

        if( rc_config.gain_style == GS_NOISE )
          scaled_rad = cell_k;
        else
        {
          /* Log-compressed for visualization; recoverable to K */
          scaled_rad = log10(1.0 + cell_k);
        }
      }
      break;

    default:
      scaled_rad = 0.0;
      break;

  } /* switch( gs ) */

  return scaled_rad;

} /* Scale_Gain_Resolved() */

/*-----------------------------------------------------------------------*/

/**
 * ant_temp_entry_set_kelvin() - render "%.0f K" into a noise-temperature entry
 * @entry:  target GtkEntry; no-op if NULL or currently focused
 * @kelvin: temperature in Kelvin
 *
 * Focus guard prevents clobbering a user-initiated edit in progress.
 */
  void
ant_temp_entry_set_kelvin(GtkWidget *entry, double kelvin)
{
  if (!entry || gtk_widget_has_focus(entry))
    return;

  char buf[24];
  snprintf(buf, sizeof(buf), "%.0f K", kelvin);
  gtk_entry_set_text(GTK_ENTRY(entry), buf);
}

/**
 * ant_temp_entry_set_unresolved() - show the em-dash placeholder in an entry
 * @entry: target GtkEntry; no-op if NULL or currently focused
 */
  void
ant_temp_entry_set_unresolved(GtkWidget *entry)
{
  if (!entry || gtk_widget_has_focus(entry))
    return;

  gtk_entry_set_text(GTK_ENTRY(entry), "— K");
}

/*-----------------------------------------------------------------------*/

/**
 * rdpattern_viewer_readout() - Refresh the viewer-direction radiation readout
 *
 * Writes gain or noise density for the current view and frequency step.
 * Called when either the view or active frequency step changes.
 */
static void
rdpattern_viewer_readout(void)
{
  int fstep;
  gchar txt[16];

  g_rec_mutex_lock(&freq_data_lock);

  fstep = calc_data.freq_step;

  /* The view outlives its window: under shared projection a structure
   * rotation reaches this callback after Rdpattern_Window_Killed()
   * released the builder these entries resolve through. */
  if( isFlagClear(DRAW_ENABLED) || !RDPAT_FSTEP_AVAILABLE(fstep) )
  {
    g_rec_mutex_unlock(&freq_data_lock);
    return;
  }

  /* Show gain (dBi) or noise density (K/sr) in direction of viewer */
  if (IS_NOISE_MODE(rc_config.gain_style))
  {
    double ksr = Viewer_Noise_Value(rdpattern_view, fstep);
    snprintf(txt, sizeof(txt) - 1, "%.2f", ksr);
    gtk_entry_set_text(GTK_ENTRY(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_viewer_gain")), txt);
  }
  else
  {
    Show_Viewer_Gain(
        rdpattern_window_builder,
        "rdpattern_viewer_gain",
        rdpattern_view);
  }

  g_rec_mutex_unlock(&freq_data_lock);
}

/*-----------------------------------------------------------------------*/

/**
 * Update_Rdpattern_UI() - Refresh frequency-step radiation readouts
 *
 * Updates color-code labels, viewer gain, frequency, and antenna-temperature
 * entries for the active frequency step.
 */
  void
Update_Rdpattern_UI(void)
{
  int fstep, pol;
  gchar txt[16];

  g_rec_mutex_lock(&freq_data_lock);

  fstep = calc_data.freq_step;
  if( isFlagClear(DRAW_ENABLED) || !RDPAT_FSTEP_AVAILABLE(fstep) )
  {
    g_rec_mutex_unlock(&freq_data_lock);
    return;
  }

  pol = calc_data.pol_type;

  /* Show max and min on color code bar; values are noise density (K/sr)
   * in noise mode, gain (dBi) otherwise */
  if (IS_NOISE_MODE(rc_config.gain_style))
  {
    /* Inverse_Scale_Gain recovers K/sr from log-compressed noise_scaled values */
    snprintf(txt, sizeof(txt)-1, "%.0f",
        Inverse_Scale_Gain(rad_pattern[fstep].noise_scaled_max));
    gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_colorcode_maxlabel")),
        txt);

    snprintf(txt, sizeof(txt)-1, "%.0f",
        Inverse_Scale_Gain(rad_pattern[fstep].noise_scaled_min));
    gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_colorcode_minlabel")),
        txt);
  }
  else
  {
    snprintf(txt, sizeof(txt)-1, "%.2f", rad_pattern[fstep].max_gain[pol]);
    gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_colorcode_maxlabel")),
        txt);

    double color_min_gain = rad_pattern[fstep].min_gain[pol];
    if (color_min_gain < COLOR_MIN_GAIN)
      color_min_gain = COLOR_MIN_GAIN;
    snprintf(txt, sizeof(txt)-1, "%.2f", color_min_gain);
    gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_colorcode_minlabel")),
        txt);
  }

  rdpattern_viewer_readout();

  /* Display frequency step */
  if( calc_data.freq_step >= 0 )
    Display_Fstep( rdpattern_fstep_entry, calc_data.freq_step );

  /* Update TA readout in toolbar */
  {
    measurement_t meas = { .a = {0} };
    meas_calc(&meas, fstep, calc_data.ex_port);
    GtkWidget *temp_entry = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_ant_temp_entry");
    if (temp_entry)
    {
      char buf[24];
      if (meas.ant_temp_tot >= 0.0)
        snprintf(buf, sizeof(buf), "%.0f K", meas.ant_temp_tot);
      else
        snprintf(buf, sizeof(buf), "— K");
      gtk_entry_set_text(GTK_ENTRY(temp_entry), buf);
    }

    /* Populate T_earth and T_sky readouts from current noise model */
    double t_sky_val, t_earth_val;
    int ok = ant_temp_resolve(save.freq[fstep],
        rc_config.ant_temp_sky, rc_config.ant_temp_earth,
        rc_config.ant_temp_interp,
        &t_sky_val, &t_earth_val) == 0;

    GtkWidget *earth_entry = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_t_earth_entry");
    GtkWidget *sky_entry = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_t_sky_entry");
    if (ok)
    {
      ant_temp_entry_set_kelvin(earth_entry, t_earth_val);
      ant_temp_entry_set_kelvin(sky_entry, t_sky_val);
    }
    else
    {
      ant_temp_entry_set_unresolved(earth_entry);
      ant_temp_entry_set_unresolved(sky_entry);
    }
  }

  g_rec_mutex_unlock(&freq_data_lock);

} /* Update_Rdpattern_UI() */


/*-----------------------------------------------------------------------*/
gboolean
Validate_Nearfield_Animation( void )
{
  if( ((draw_efield_active() || draw_poynting_active()) && !(fpat.nfeh & NEAR_EFIELD)) ||
      ((draw_hfield_active() || draw_poynting_active()) && !(fpat.nfeh & NEAR_HFIELD)) )
  {
    Notice( GTK_BUTTONS_OK, _("Near Field Animation"), "%s",
        _(nearfield_animation_error_msg) );
    return( FALSE );
  }

  int fstep = calc_data.freq_step;
  if( !NF_FSTEP_AVAILABLE(fstep) )
  {
    Notice( GTK_BUTTONS_OK, _("Near Field Animation"), "%s",
        _(nearfield_animation_error_msg) );
    return( FALSE );
  }

  if( near_field_fstep[fstep].points == NULL )
  {
    Notice( GTK_BUTTONS_OK, _("Near Field Animation"), "%s",
        _(nearfield_animation_error_msg) );
    return( FALSE );
  }

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* Polarization_Factor()
 *
 * Calculates polarization factor from axial
 * ratio and tilt of polarization ellipse
 */
  double
Polarization_Factor( int pol_type, int fstep, int idx )
{
  double axrt, axrt2, tilt2, polf = 1.0;

  switch( pol_type )
  {
    case POL_TOTAL:
      polf = 1.0;
      break;

    case POL_HORIZ:
      axrt2  = rad_pattern[fstep].axrt[idx];
      axrt2 *= axrt2;
      tilt2  = sin( rad_pattern[fstep].tilt[idx] );
      tilt2 *= tilt2;
      polf = (axrt2 + (1.0 - axrt2) * tilt2) / (1.0 + axrt2);
      break;

    case POL_VERT:
      axrt2  = rad_pattern[fstep].axrt[idx];
      axrt2 *= axrt2;
      tilt2  = cos( rad_pattern[fstep].tilt[idx] );
      tilt2 *= tilt2;
      polf = (axrt2 + (1.0 - axrt2) * tilt2) / (1.0 + axrt2);
      break;

    case POL_LHCP:
      axrt  = rad_pattern[fstep].axrt[idx];
      axrt2 = axrt * axrt;
      polf  = (1.0 + 2.0 * axrt + axrt2) / 2.0 / (1.0 + axrt2);
      break;

    case POL_RHCP:
      axrt  = rad_pattern[fstep].axrt[idx];
      axrt2 = axrt * axrt;
      polf  = (1.0 - 2.0 * axrt + axrt2) / 2.0 / (1.0 + axrt2);
  }

  if( polf < 1.0E-200 ) polf = 1.0E-200;
  polf = 10.0 * log10( polf );

  return( polf );
} /* Polarization_Factor() */

/*-----------------------------------------------------------------------*/

/* polarization_refresh()
 *
 * Refreshes everything derived from the polarization type of gain plotted
 */

  void
polarization_refresh( void )
{
  Set_Window_Labels();
  freq_step_refresh_ui(TRUE);

} /* polarization_refresh() */

/*-----------------------------------------------------------------------*/

/* Set_Gain_Style()
 *
 * Refreshes everything derived from the radiation pattern Gain scaling style
 */
  void
Set_Gain_Style( void )
{
  GtkWidget *widget;
  int gs = rc_config.gain_style;

  if( gs < GS_LINP || gs >= NUM_SCALES )
  {
    BUG("gain style %d is outside supported range [%d, %d)\n",
        gs, GS_LINP, NUM_SCALES);
    return;
  }

  /* Dormant: the pattern window holds every readout the refresh below writes */
  if( rdpattern_window_builder == NULL )
    return;

  gboolean noise = IS_NOISE_MODE(gs);

  /* Update units label and row label for noise vs gain modes */
  widget = Builder_Get_Object( rdpattern_window_builder, "rdpattern_gain_units_label" );
  if (widget)
    gtk_label_set_text( GTK_LABEL(widget), noise ? "K/sr" : "dB" );

  widget = Builder_Get_Object( rdpattern_window_builder, "rdpattern_gain_row_label" );
  if (widget)
  {
    if (noise)
    {
      gtk_label_set_markup( GTK_LABEL(widget),
          "T<sub>b</sub>(θ,φ)" );
    }
    else
    {
      gtk_label_set_text( GTK_LABEL(widget), "Gain" );
    }
  }

  /* Declare the viewer readout row as viewer-direction specific */
  static const char *const viewer_row_ids[] = {
    "rdpattern_gain_row_label",
    "rdpattern_viewer_gain",
    "rdpattern_gain_units_label",
    NULL
  };

  static const char *viewer_row_gain_tip =
    N_("Gain in the direction of the viewer, for the current view angles.");
  static const char *viewer_row_noise_tip =
    N_("Brightness temperature in the direction of the viewer, "
       "for the current view angles.");

  const char *viewer_row_tip = noise ? _(viewer_row_noise_tip) : _(viewer_row_gain_tip);
  for (int i = 0; viewer_row_ids[i] != NULL; i++)
  {
    widget = Builder_Get_Object(
        rdpattern_window_builder, (gchar *)viewer_row_ids[i] );
    if (widget)
      gtk_widget_set_tooltip_text( widget, viewer_row_tip );
  }

  /* Desensitize noise-specific controls when not in noise mode */

  static const struct {
    const char *widget_id;
    const char *tooltip;
    const char *disabled_tooltip;
  } noise_widgets[] = {
    { "rdpattern_elevation_spinbutton",
      N_("Observation elevation angle. Shifts sky/earth "
         "boundary for antenna temperature evaluation. "
         "0° = horizon."),
      N_("Observation elevation for antenna temperature.\n"
         "Select Gain Style → Noise Temperature to enable.") },
    { "rdpattern_elevation_label",
      N_("Observation elevation angle. Shifts sky/earth "
         "boundary for antenna temperature evaluation. "
         "0° = horizon."),
      N_("Observation elevation for antenna temperature.\n"
         "Select Gain Style → Noise Temperature to enable.") },
    { "rdpattern_noise_env_menu",
      N_("Independent sky, earth, and interpolation "
         "selectors for noise temperature models."),
      N_("Sky and earth noise temperature models.\n"
         "Select Gain Style → Noise Temperature to enable.") },
  };

  int n_noise_widgets = sizeof(noise_widgets) / sizeof(noise_widgets[0]);
  for (int i = 0; i < n_noise_widgets; i++)
  {
    widget = Builder_Get_Object(
        rdpattern_window_builder, (gchar *)noise_widgets[i].widget_id);
    if (widget)
    {
      gtk_widget_set_sensitive(widget, noise);
      gtk_widget_set_tooltip_text(widget,
          noise ? _(noise_widgets[i].tooltip) : _(noise_widgets[i].disabled_tooltip));
    }
  }

  Set_Window_Labels();

  freq_step_refresh_ui(TRUE);

} /* Set_Gain_Style() */

/*-----------------------------------------------------------------------*/

/** gain_style_check_warnings - report noise-mode model compatibility
 *
 * This is the committed transition edge alone, so crossing the noise rows on
 * hover stays silent.
 */
  void
gain_style_check_warnings( void )
{
  if( !IS_NOISE_MODE(rc_config.gain_style) )
    return;

  g_rec_mutex_lock(&freq_data_lock);
  Check_Noise_Warnings(calc_data.freq_step);
  g_rec_mutex_unlock(&freq_data_lock);

} /* gain_style_check_warnings() */

/*-----------------------------------------------------------------------*/

/*  Queue_Radiation_Redraw()
 *  @force:  bypass the intermediate-redraw suppression gate
 *
 *  Queues a redraw of the radiation drawingarea.  Callers that also change
 *  a frequency-plot input repaint the plots at their own edge.
 */
  void
Queue_Radiation_Redraw(gboolean force)
{
  if( isFlagSet(DRAW_ENABLED) )
    canvas_queue_redraw( CANVAS_RDPATTERN, force );

} /* Queue_Radiation_Redraw() */

/*-----------------------------------------------------------------------*/

/** rdpattern_view_changed_cb() - view_t change callback for rdpattern view
 * @v:           view that changed
 * @_user_data:  unused
 *
 * Invoked by view_notify_change() whenever rotation, pan, zoom, or extent
 * changes.  Refreshes the WR/WI spin display and viewer readout, then queues
 * a radiation pattern redraw and the view-dependent frequency plots.  The
 * frequency-step readouts are view-independent and stay untouched here.
 * Bound as changed_cb at view_new() in callbacks.c; when sharing is active
 * the master (structure_view) reaches this callback via its
 * rotation_follower link inside view_notify_change().
 */
  void
rdpattern_view_changed_cb(view_t *v, gpointer _user_data)
{
  /* The viewer-gain and antenna-temperature panels plot the value in the
   * view direction, so a view change moves their traces */
  static const fp_panel_t view_panels[] =
    { FP_PANEL_VIEWER, FP_PANEL_ANT_TEMP, FP_PANEL_COUNT };

  (void)_user_data;

  view_update_spin_display( v );
  rdpattern_viewer_readout();
  Queue_Radiation_Redraw( TRUE );
  freqplots_redraw_if_showing( view_panels );

} /* rdpattern_view_changed_cb() */

/*-----------------------------------------------------------------------*/

/**
 * rdpattern_overlay_shift_scroll() - Adjust overlay structure scale from scroll input
 * @dir:  GDK_SCROLL_UP or GDK_SCROLL_DOWN, the caller admitting no other
 *        direction
 * @view: view whose viewport dimensions scale the increment
 *
 * Reads the stored dispatch result to gate on overlay_active and far-field mode.
 * Mutates rc_config.rdpattern_overlay_scale_adj and queues a rdpattern redraw.
 * Returns TRUE when the event was consumed.
 */
  static gboolean
rdpattern_overlay_shift_scroll(GdkScrollDirection dir, const view_t *view)
{
  const render_check_result_t *rc = render_last_rdpattern_check();
  double scale;

  if( !rc->overlay_active || rc->mode != RENDER_MODE_FARFIELD )
    return FALSE;

  scale = compute_zoom_scale(view->width, view->height,
      rc_config.rdpattern_overlay_scale_adj * 100.0);

  /* The caller admits up and down alone, leaving down as the else */
  if( dir == GDK_SCROLL_UP )
    rc_config.rdpattern_overlay_scale_adj *= (1.0 + 0.1 * scale);
  else
    rc_config.rdpattern_overlay_scale_adj /= (1.0 + 0.1 * scale);

  Queue_Radiation_Redraw(TRUE);

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/**
 * rdpattern_shift_scroll() - Scale the overlay structure from a scroll event
 * @event: scroll event carrying the shift modifier
 * @surface: surface of the scrolled view, naming the viewport the scale reads
 */
  static gboolean
rdpattern_shift_scroll(GdkEventScroll *event, render_surface_t *surface)
{
  scroll_step_t s;

  if( surface == NULL )
    return( FALSE );

  s = scroll_step_from_deltas((GdkEvent *)event);

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  return( rdpattern_overlay_shift_scroll(s.direction, surface->view) );

} /* rdpattern_shift_scroll() */

/*-----------------------------------------------------------------------*/

/* Shift+scroll scales the structure overlay the far-field pattern carries */
surface_capability_t rdpattern_overlay_scale_cap = {
  .handler = rdpattern_shift_scroll,
  .notice  = "Shift+Scroll to Scale Structure",
  .subject = SURFACE_CAP_SUBJECT_OVERLAY_GEOMETRY
};

/*-----------------------------------------------------------------------*/

/* Modifier scroll capabilities the Cairo radiation-pattern surface offers;
 * cylinder geometry belongs to the OpenGL engine, so ctrl+scroll is declined */
const surface_input_ops_t rdpattern_cairo_input = {
  .by_modifier = { [SURFACE_MOD_SHIFT] = &rdpattern_overlay_scale_cap }
};

/*-----------------------------------------------------------------------*/

/**
 * free_near_step() - Release one fstep's near-field point buffer
 * @elem: pointer to one near_field_t element
 */
static void
free_near_step(void *elem)
{
  near_field_t *nf = elem;
  mem_array_free(&nf->points);
}

/*-----------------------------------------------------------------------*/

/**
 * Alloc_Nearfield_Fstep_Buffers() - Allocate per-frequency-step near field storage
 *
 * @nfrq: Number of frequency steps (steps_total + 1)
 *
 * Allocates near_field_fstep[] array so each frequency step holds the
 * near E/H field data nfpat() writes.
 */
  void
Alloc_Nearfield_Fstep_Buffers( int nfrq )
{
  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their point buffers for reuse by the inner alloc loop. */
  mem_array_resize(&near_field_fstep, nfrq, free_near_step);

  /* Absent NE/NH cards the outer array still exists and only the per-step
   * point buffers are skipped. */
  size_t npts = (size_t)fpat.nrx * fpat.nry * fpat.nrz;
  if( npts > 0 )
  {
    for( int i = 0; i < nfrq; i++ )
      mem_array_realloc(&near_field_fstep[i].points, npts);
  }

} /* Alloc_Nearfield_Fstep_Buffers() */

/*-----------------------------------------------------------------------*/

/**
 * Free_Nearfield_Fstep_Buffers() - Free per-frequency-step near field storage
 */
  void
Free_Nearfield_Fstep_Buffers( void )
{
  int nfrq = mem_array_count(near_field_fstep);

  for( int i = 0; i < nfrq; i++ )
    free_near_step(&near_field_fstep[i]);
  mem_array_free(&near_field_fstep);

} /* Free_Nearfield_Fstep_Buffers() */

/*-----------------------------------------------------------------------*/

/* Viewer_Gain()
 *
 * Calculate gain in direction of viewer
 * (e.g. Perpenticular to the Screen)
 */
  double
Viewer_Gain( view_t *v, int fstep )
{
  double phi, theta, gain;
  int nth, nph, idx;

  /* Read spherical direction of viewing axis directly from the
   * rotation matrix; theta is polar from +Z, phi is azimuth.
   * At the pole phi is NAN; use zero so the pattern lookup still
   * reads the polar cell. */
  view_get_theta_phi( v, &theta, &phi );
  if( isnan(phi) )
    phi = 0.0;

  /* Ground-plane occlusion on the resolved direction */
  if( (gnd.ksymp == 2) &&
      (theta > 90.01)  &&
      (gnd.ifar != 1) )
    return( -999.99 );

  nth = fpat_theta_cell( theta );
  nph = fpat_phi_cell( phi );
  idx = nth + nph * fpat.nth;
  gain = rad_pattern[fstep].gtot[idx] +
    Polarization_Factor(calc_data.pol_type, fstep, idx);
  if( gain < -999.99 ) gain = -999.99;

  return( gain );

} /* Viewer_Gain() */

/*-----------------------------------------------------------------------*/

/* Rdpattern_Window_Killed()
 *
 * Cleans up after the rad pattern window is closed
 */
  void
Rdpattern_Window_Killed( void )
{
  if( isFlagSet(DRAW_ENABLED) )
  {
    ClearFlag( DRAW_FLAGS );
    g_object_unref( rdpattern_window_builder );
    rdpattern_window_builder = NULL;

    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object( main_window_builder, "main_rdpattern")), FALSE );
  }

  /* Keep the DRAW_ENABLED gate closed while destroying the dialog because
   * its destroy handler refreshes the visualization after the caller frees
   * the radiation-pattern view. */
  if( animate_dialog != NULL )
    Gtk_Widget_Destroy( &animate_dialog );

  rdpattern_window = NULL;
  canvas_clear( CANVAS_RDPATTERN );
  kill_window = NULL;

} /* Rdpattern_Window_Killed() */

/*-----------------------------------------------------------------------*/

/**
 * pol_type_name() - Name the polarization selection gain is scaled by
 * @pol: POL_TYPE enumerator selecting the polarization component
 *
 * Window labels and messages naming the selection read the same names,
 * so both take them from this one table.
 */
  const char *
pol_type_name( int pol )
{
  static const char *const name[NUM_POL] =
  {
    [POL_TOTAL] = N_("Total Gain"),
    [POL_HORIZ] = N_("Horizontal Polarization"),
    [POL_VERT]  = N_("Vertical Polarization"),
    [POL_RHCP]  = N_("RH Circular Polarization"),
    [POL_LHCP]  = N_("LH Circular Polarization")
  };

  if( (pol < 0) || (pol >= NUM_POL) )
  {
    BUG("polarization selection out of range: %d\n", pol);
    pol = POL_TOTAL;
  }

  return( _(name[pol]) );

} /* pol_type_name() */

/*-----------------------------------------------------------------------*/

/**
 * set_pol_name_weight() - Weight the polarization name in a window label
 * @attrs: label attribute list receiving the weight
 * @pol:   POL_TYPE enumerator selecting normal or bold weight
 * @start: byte offset of the name in the label text
 * @end:   byte offset one past the name
 *
 * A non-total selection scales every gain the pattern and plots report, so
 * its name carries bold weight while the total selection holds normal weight.
 */
  static void
set_pol_name_weight( PangoAttrList *attrs, int pol, size_t start, size_t end )
{
  PangoWeight weight = (pol == POL_TOTAL) ?
      PANGO_WEIGHT_NORMAL : PANGO_WEIGHT_BOLD;
  PangoAttribute *attr = pango_attr_weight_new( weight );

  attr->start_index = (guint)start;
  attr->end_index   = (guint)end;
  pango_attr_list_insert( attrs, attr );

} /* set_pol_name_weight() */

/*-----------------------------------------------------------------------*/

/**
 * Set_Window_Labels() - Set labels for active pattern and frequency views
 *
 * The pattern label names its content, polarization, and scale; the frequency
 * label names the polarization shared by its plots.
 */
  void
Set_Window_Labels( void )
{
  char *scale[NUM_SCALES] =
  {
    _("Linear Power"),
    _("Linear Voltage"),
    _("ARRL Scale"),
    _("Logarithmic Scale"),
    _("Noise Temperature"),
    _("Noise Temp (log scale)"),
  };

  char txt[256];
  size_t s = sizeof( txt );

  if( isFlagSet(DRAW_ENABLED) )
  {
    GtkLabel *label = GTK_LABEL( Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_label") );
    PangoAttrList *pol_attrs = pango_attr_list_new();

    /* Set window labels */
    Strlcpy( txt, _("Radiation Patterns"), s );
    if(rdpat_gain_active())
    {
      size_t pol_start, pol_end;

      Strlcpy( txt, _("Radiation Pattern: - "), s );
      pol_start = strlen( txt );
      Strlcat( txt, pol_type_name(calc_data.pol_type), s );
      pol_end = strlen( txt );
      Strlcat( txt, " - ", s );
      Strlcat( txt, scale[rc_config.gain_style], s );

      set_pol_name_weight( pol_attrs, calc_data.pol_type, pol_start, pol_end );

      /* Append noise model info when in noise display mode */
      if( IS_NOISE_MODE(rc_config.gain_style) )
      {
        Strlcat( txt, " - Sky:", s );
        Strlcat( txt, ant_temp_sky_name(rc_config.ant_temp_sky), s );
        Strlcat( txt, " - Earth:", s );
        Strlcat( txt, ant_temp_earth_name(rc_config.ant_temp_earth), s );
        Strlcat( txt, " - ", s );
        if( rc_config.ant_temp_interp >= 0 &&
            rc_config.ant_temp_interp < ANT_TEMP_METHOD_COUNT )
          Strlcat( txt, ant_temp_method_names[rc_config.ant_temp_interp], s );
      }
    }
    else if(rdpat_ehfield_active())
    {
      Strlcpy( txt, _("Near Fields:"), s );
      if( draw_efield_active() )
        Strlcat( txt, _(" - E Field"), s );
      if( draw_hfield_active() )
        Strlcat( txt, _(" - H Field"), s );
      if( draw_poynting_active() )
        Strlcat( txt, _(" - Poynting Vector"), s );
    }

    gtk_label_set_text( label, txt );
    gtk_label_set_attributes( label, pol_attrs );
    pango_attr_list_unref( pol_attrs );

  } /* if( isFlagSet(DRAW_ENABLED) ) */

  if( isFlagSet(PLOT_ENABLED) )
  {
    Strlcpy( txt, _("Frequency Data Plots - "), s );
    Strlcat( txt, pol_type_name(calc_data.pol_type), s );
    gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
            freqplots_window_builder, "freqplots_label")), txt );
  }

} /* Set_Window_Labels() */

/*-----------------------------------------------------------------------*/

/**
 * free_rad_pattern_step() - Release one fstep's radiation-pattern sub-buffers
 * @elem: pointer to one rad_pattern_t element
 */
static void
free_rad_pattern_step(void *elem)
{
  rad_pattern_t *rp = elem;
  mem_array_free(&rp->gtot);
  mem_array_free(&rp->max_gain);
  mem_array_free(&rp->min_gain);
  mem_array_free(&rp->max_gain_tht);
  mem_array_free(&rp->max_gain_phi);
  mem_array_free(&rp->max_gain_idx);
  mem_array_free(&rp->min_gain_idx);
  mem_array_free(&rp->axrt);
  mem_array_free(&rp->tilt);
  mem_array_free(&rp->sens);
  mem_array_free(&rp->phasor);
}

/*-----------------------------------------------------------------------*/

/* Alloc_Rdpattern_Buffers
 *
 * Allocates memory to the radiation pattern buffers
 */
  void
_Alloc_Rdpattern_Buffers( int nfrq, int nth, int nph )
{
  int idx;

  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their sub-buffers for reuse by the inner alloc loop. */
  mem_array_resize(&rad_pattern, nfrq, free_rad_pattern_step);

  /* Per-fstep angular sample count; gates the inner sub-buffer alloc so
   * the outer-array invariant holds even for a degenerate nth*nph. */
  int nrec = (nph * nth);
  if( nrec > 0 )
    for( idx = 0; idx < nfrq; idx++ )
    {
      mem_array_alloc(&rad_pattern[idx].gtot, nrec);
      mem_array_alloc(&rad_pattern[idx].axrt, nrec);
      mem_array_alloc(&rad_pattern[idx].tilt, nrec);
      mem_array_alloc(&rad_pattern[idx].max_gain, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].min_gain, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].max_gain_tht, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].max_gain_phi, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].max_gain_idx, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].min_gain_idx, NUM_POL);
      mem_array_alloc(&rad_pattern[idx].sens, nrec);
      mem_array_alloc(&rad_pattern[idx].phasor, nrec);
    }
  mem_array_realloc(&noise_temp, nfrq);
  mem_array_zero(noise_temp);

} /* Alloc_Rdpattern_Buffers() */

void Alloc_Rdpattern_Buffers( int nfrq, int nth, int nph )
{
	g_rec_mutex_lock(&freq_data_lock);
	_Alloc_Rdpattern_Buffers(nfrq, nth, nph);
	g_rec_mutex_unlock(&freq_data_lock);
}

/*-----------------------------------------------------------------------*/

/**
 * free_rdpattern_buffers() - Release per-frequency radiation-pattern storage
 */
  void
free_rdpattern_buffers(void)
{
  int nfrq = mem_array_count(rad_pattern);

  for( int i = 0; i < nfrq; i++ )
    free_rad_pattern_step(&rad_pattern[i]);
  mem_array_free(&rad_pattern);
  mem_array_free(&noise_temp);

} /* free_rdpattern_buffers() */

/*-----------------------------------------------------------------------*/

/* Inverse_Scale_Gain()
 *
 * Calculates the actual dB value from a scaled gain value
 * This is the inverse of Scale_Gain()
 */
double
Inverse_Scale_Gain(double scaled_val)
{
  double db_val = 0.0;

  switch(rc_config.gain_style)
  {
    case GS_LINP:
      if (scaled_val > 0.0)
        db_val = 10.0 * log10(scaled_val);
      else
        db_val = -999.99;
      break;

    case GS_LINV:
      if (scaled_val > 0.0)
        db_val = 20.0 * log10(scaled_val);
      else
        db_val = -999.99;
      break;

    case GS_ARRL:
      if (scaled_val > 0.0)
        db_val = log(scaled_val) / 0.058267;
      else
        db_val = -999.99;
      break;

    case GS_LOG:
      db_val = (scaled_val - 1.0) * 40.0;
      break;

    /* Gain-weighted brightness temperature in Kelvin */
    case GS_NOISE:
      db_val = scaled_val;
      break;

    /* Log-compressed: recover Kelvin from log10(1 + G_lin*T_bright) */
    case GS_NOISE_LOG:
      db_val = pow(10.0, scaled_val) - 1.0;
      break;
  }

  return db_val;
}

/**
 * Viewer_Noise_Value() - noise temperature density at the viewer direction
 * @proj_parameters: projection parameters defining viewer angle
 * @fstep:           frequency step index
 *
 * Returns the K/sr noise density for the pattern cell visually at the
 * viewer direction.  When ant_temp_elevation is non-zero, the 3D pattern
 * is rendered with a Rodrigues rotation; this function applies the inverse
 * rotation to find the correct unrotated pattern cell.
 *
 * Returns 0.0 when the direction is occluded by a ground plane or when
 * noise environment resolution fails.
 */
double
Viewer_Noise_Value(view_t *v, int fstep)
{
	double phi_deg, theta_deg;
	int nth, nph, idx, pol;

	if (!rad_pattern)
		return 0.0;

	pol = calc_data.pol_type;
	if (pol < 0 || pol >= NUM_POL)
		return 0.0;

	/* Viewer direction in spherical coordinates, read from the
	 * rotation matrix directly.  theta is polar from +Z; phi is
	 * azimuth from +X and is NAN at the pole (viewing axis along
	 * Z); default to zero for the pattern-grid lookup there. */
	view_get_theta_phi( v, &theta_deg, &phi_deg );
	if (isnan(phi_deg))
		phi_deg = 0.0;

	if (fpat.dth == 0.0)
		theta_deg = fpat.thets;

	/* When elevation is non-zero, the 3D pattern is visually rotated
	 * via ant_temp_rotate_point().  Apply the inverse rotation (negated
	 * angle) to map the viewer direction back to unrotated pattern
	 * coordinates so the readout matches the cell on screen. */
	double elev_deg = rc_config.ant_temp_elevation;
	if (elev_deg != 0.0)
	{
		double phi_mg = rad_pattern[fstep].max_gain_phi[pol] * M_PI / 180.0;
		double elev_rad = elev_deg * M_PI / 180.0;

		double tht_rad = theta_deg * M_PI / 180.0;
		double phi_rad = phi_deg * M_PI / 180.0;

		double xr, yr, zr;
		ant_temp_rotate_point(tht_rad, phi_rad,
			phi_mg, -elev_rad,
			&xr, &yr, &zr);

		if (zr > 1.0)
			zr = 1.0;
		else if (zr < -1.0)
			zr = -1.0;

		theta_deg = acos(zr) * 180.0 / M_PI;
		phi_deg = atan2(yr, xr) * 180.0 / M_PI;
		if (phi_deg < 0.0)
			phi_deg += 360.0;
	}

	/* Ground-plane occlusion on the resolved direction */
	if ((gnd.ksymp == 2) &&
		(theta_deg > 90.01) &&
		(gnd.ifar != 1))
		return 0.0;

	/* Snap to pattern grid */
	nth = fpat_theta_cell(theta_deg);
	nph = fpat_phi_cell(phi_deg);
	idx = nth + nph * fpat.nth;
	double gain = rad_pattern[fstep].gtot[idx]
		+ Polarization_Factor(pol, fstep, idx);

	return Inverse_Scale_Gain(Scale_Gain(gain, fstep, idx));
}

/*-----------------------------------------------------------------------*/

