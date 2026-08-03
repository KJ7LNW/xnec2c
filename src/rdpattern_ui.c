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
#include "measurements.h"
#include "rc_config.h"
#include "render/render_dispatch.h"
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

/* Update_Rdpattern_UI()
 *
 * Updates radiation pattern window UI elements
 */
  void
Update_Rdpattern_UI(void)
{
  int fstep, pol;
  gchar txt[16];

  fstep = calc_data.freq_step;
  if( fstep < 0 || !rad_pattern )
    return;

  pol = calc_data.pol_type;

  if( fstep > calc_data.steps_total )
    return;

  if( !rad_pattern[fstep].max_gain || !rad_pattern[fstep].min_gain )
    return;

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

/* Set_Polarization()
 *
 * Sets the polarization type of gain to be plotted
 */

  void
Set_Polarization( int pol )
{
  calc_data.pol_type = pol;
  Set_Window_Labels();

  /* Show gain in direction of viewer */
  g_rec_mutex_lock(&freq_data_lock);
  Show_Viewer_Gain( main_window_builder, "main_gain_entry", structure_view );
  g_rec_mutex_unlock(&freq_data_lock);

  freq_step_update_ui(calc_data.freq_step, TRUE);

} /* Set_Polarization() */

/*-----------------------------------------------------------------------*/

/* Set_Gain_Style()
 *
 * Sets the radiation pattern Gain scaling style
 */
  void
Set_Gain_Style( int gs )
{
  static char *scale_widget_names[NUM_SCALES] = {
	  "rdpattern_linear_power",
	  "rdpattern_linear_voltage",
	  "rdpattern_arrl_style",
	  "rdpattern_logarithmic",
	  "rdpattern_noise_temp",
	  "rdpattern_noise_temp_log",
	  };

  GtkWidget *widget;

  // This should never happen:
  if (gs >= NUM_SCALES)
	  return;

  rc_config.gain_style = gs;

  widget = Builder_Get_Object( rdpattern_window_builder, scale_widget_names[rc_config.gain_style] );
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );

  /* Update units label and row label for noise vs gain modes */
  widget = Builder_Get_Object( rdpattern_window_builder, "rdpattern_gain_units_label" );
  if (widget)
    gtk_label_set_text( GTK_LABEL(widget),
        IS_NOISE_MODE(gs) ? "K/sr" : "dB" );

  widget = Builder_Get_Object( rdpattern_window_builder, "rdpattern_gain_row_label" );
  if (widget)
  {
    if (IS_NOISE_MODE(gs))
    {
      gtk_label_set_markup( GTK_LABEL(widget),
          "T<sub>b</sub>(θ,φ)" );
    }
    else
    {
      gtk_label_set_text( GTK_LABEL(widget), "Gain" );
    }
  }

  /* Desensitize noise-specific controls when not in noise mode */
  gboolean noise = IS_NOISE_MODE(gs);

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

  /* Check for model compatibility warnings when entering noise mode */
  if (noise)
  {
    g_rec_mutex_lock(&freq_data_lock);
    Check_Noise_Warnings(calc_data.freq_step);
    g_rec_mutex_unlock(&freq_data_lock);
  }

  Set_Window_Labels();

  freq_step_update_ui(calc_data.freq_step, TRUE);

} /* Set_Gain_Style() */

/*-----------------------------------------------------------------------*/

/*  Queue_Radiation_Redraw()
 *
 *  Queues a redraw of the radiation drawingarea (and the plot area
 *  when viewer-gain or antenna-temperature plots are visible).
 */
  void
Queue_Radiation_Redraw(void)
{
  /* Trigger a redraw of radiation drawingarea */
  if( isFlagSet(DRAW_ENABLED) )
  {
    xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
  }

  /* Trigger a redraw of plots drawingarea if doing "viewer" gain
   * or antenna temperature (noise env / elevation changes affect Ta) */
  if( isFlagSet(PLOT_ENABLED) &&
      (rc_config.freqplots_gviewer_togglebutton || rc_config.freqplots_ant_temp_togglebutton ||
       freqplots_popup_open(FP_PANEL_VIEWER) ||
       freqplots_popup_open(FP_PANEL_ANT_TEMP)) &&
      isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    freqplots_redraw_all(TRUE);
  }

} /* Queue_Radiation_Redraw() */

/*-----------------------------------------------------------------------*/

/** rdpattern_view_changed_cb() - view_t change callback for rdpattern view
 * @v:           view that changed
 * @_user_data:  unused
 *
 * Invoked by view_notify_change() whenever rotation, pan, zoom, or extent
 * changes.  Refreshes the WR/WI spin display and queues a radiation pattern
 * redraw.  Bound as changed_cb at view_new() in callbacks.c; when sharing is
 * active the master (structure_view) reaches this callback via its
 * rotation_follower link inside view_notify_change().
 */
  void
rdpattern_view_changed_cb(view_t *v, gpointer _user_data)
{
  (void)_user_data;

  view_update_spin_display( v );
  Queue_Radiation_Redraw();

} /* rdpattern_view_changed_cb() */

/*-----------------------------------------------------------------------*/

/**
 * rdpattern_overlay_shift_scroll() - Adjust overlay structure scale from scroll input
 * @dir:      GDK_SCROLL_UP or GDK_SCROLL_DOWN
 * @vp_w:     viewport width in pixels
 * @vp_h:     viewport height in pixels
 * @zoom_pct: current overlay scale as percentage (rdpattern_overlay_scale_adj * 100.0)
 *
 * Reads the cached dispatch result to gate on overlay_active and far-field mode.
 * Mutates rc_config.rdpattern_overlay_scale_adj and queues a rdpattern redraw.
 * Returns TRUE when the event was consumed.
 */
  gboolean
rdpattern_overlay_shift_scroll(GdkScrollDirection dir,
    int vp_w, int vp_h, double zoom_pct)
{
  const render_check_result_t *rc = render_check_rdpat();
  double scale;

  if( !rc->overlay_active || rc->mode != RENDER_MODE_FARFIELD )
    return FALSE;

  scale = compute_zoom_scale(vp_w, vp_h, zoom_pct);

  if( dir == GDK_SCROLL_UP )
    rc_config.rdpattern_overlay_scale_adj *= (1.0 + 0.1 * scale);
  else if( dir == GDK_SCROLL_DOWN )
    rc_config.rdpattern_overlay_scale_adj /= (1.0 + 0.1 * scale);
  else
    return FALSE;

  xnec2_widget_queue_draw(rdpattern_drawingarea, TRUE);

  return TRUE;
}

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
 * Allocates near_field_fstep[] array so each frequency step can store
 * its computed near E/H field data for later restoration.
 */
  void
Alloc_Nearfield_Fstep_Buffers( int nfrq )
{
  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their point buffers for reuse by the inner alloc loop. */
  mem_array_resize(&near_field_fstep, nfrq, free_near_step);

  size_t npts = (size_t)fpat.nrx * fpat.nry * fpat.nrz;

  for( int i = 0; i < nfrq; i++ )
  {
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
  if( near_field_fstep == NULL )
    return;

  int nfrq = mem_array_count(near_field_fstep);
  for( int i = 0; i < nfrq; i++ )
    free_near_step(&near_field_fstep[i]);

  mem_array_free(&near_field_fstep);

} /* Free_Nearfield_Fstep_Buffers() */

/*-----------------------------------------------------------------------*/

/**
 * Save_Nearfield_Data() - Save current near field data for a frequency step
 *
 * @fstep: Frequency step index
 *
 * Copies the global near_field struct arrays into near_field_fstep[fstep].
 */
  void
Save_Nearfield_Data( int fstep )
{
  if( isFlagClear(ENABLE_NEAREH)
      || near_field_fstep == NULL
      || fstep < 0 || fstep > calc_data.steps_total
      || near_field_fstep[fstep].points == NULL)
    return;

  size_t nbytes = (size_t)fpat.nrx * fpat.nry * fpat.nrz * sizeof(near_field_point_t);
  memcpy(near_field_fstep[fstep].points, near_field.points, nbytes);

  /* Spatial extent; the magnitude maxima derive at draw in the resolver */
  near_field_fstep[fstep].r_max = near_field.r_max;

} /* Save_Nearfield_Data() */

/*-----------------------------------------------------------------------*/

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
  if( animate_dialog != NULL )
  {
    Gtk_Widget_Destroy( &animate_dialog );
    ClearFlag( ANIMATE );
    if( anim_tag ) g_source_remove( anim_tag );
    anim_tag = 0;
  }

  if( isFlagSet(DRAW_ENABLED) )
  {
    ClearFlag( DRAW_FLAGS );
    rdpattern_drawingarea = NULL;
    g_object_unref( rdpattern_window_builder );
    rdpattern_window_builder = NULL;
    Free_Draw_Buffers();

    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object( main_window_builder, "main_rdpattern")), FALSE );
  }
  rdpattern_window = NULL;
  kill_window = NULL;

} /* Rdpattern_Window_Killed() */

/*-----------------------------------------------------------------------*/

/* Set_Window_Labels()
 *
 * Sets radiation pattern window labels
 * according to what is being drawn.
 */
  void
Set_Window_Labels( void )
{
  char *pol_type[NUM_POL] =
  {
    _("Total Gain"),
    _("Horizontal Polarization"),
    _("Vertical Polarization"),
    _("RH Circular Polarization"),
    _("LH Circular Polarization")
  };

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
    /* Set window labels */
    Strlcpy( txt, _("Radiation Patterns"), s );
    if(rdpat_gain_active())
    {
      Strlcpy( txt, _("Radiation Pattern: - "), s );
      Strlcat( txt, pol_type[calc_data.pol_type], s );
      Strlcat( txt, " - ", s );
      Strlcat( txt, scale[rc_config.gain_style], s );

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

    gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_label")), txt );

  } /* if( isFlagSet(DRAW_ENABLED) ) */

  if( isFlagSet(PLOT_ENABLED) )
  {
    Strlcpy( txt, _("Frequency Data Plots - "), s );
    Strlcat( txt, pol_type[calc_data.pol_type], s );
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

/* Alloc_Nearfield_Buffers
 *
 * Allocates memory to the radiation pattern buffers
 */
  void
Alloc_Nearfield_Buffers( int n1, int n2, int n3 )
{
  if( isFlagClear(ALLOC_NEAREH_BUFF) ) return;
  ClearFlag( ALLOC_NEAREH_BUFF );
  mem_array_realloc(&near_field.points, (n1 * n2 * n3));

} /* Alloc_Nearfield_Buffers() */

/*-----------------------------------------------------------------------*/

/* near_field_data_free()
 *
 * Releases the near E/H field point array.
 */
  void
near_field_data_free( void )
{
  mem_array_free( &near_field.points );

} /* near_field_data_free() */

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
		double tht_mg = rad_pattern[fstep].max_gain_tht[pol] * M_PI / 180.0;
		double phi_mg = rad_pattern[fstep].max_gain_phi[pol] * M_PI / 180.0;
		double elev_rad = elev_deg * M_PI / 180.0;

		double tht_rad = theta_deg * M_PI / 180.0;
		double phi_rad = phi_deg * M_PI / 180.0;

		double xr, yr, zr;
		ant_temp_rotate_point(tht_rad, phi_rad,
			tht_mg, phi_mg, -elev_rad,
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

/* Free_Draw_Buffers()
 *
 * Frees the buffers used for drawing.
 * Vertex data now lives in ff_pre[].vertices (prerender_state). */
  void
Free_Draw_Buffers( void )
{
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

