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

#include "callbacks.h"
#include "gdk_scroll.h"
#include "shared.h"
#include "opt_ui.h"
#include "optimizers/opt_session.h"
#include "measurements.h"
#include "themes/theme.h"
#include "rdpattern_ui.h"
#include "structure_ui.h"
#include "config_hooks.h"
#include "rc_config.h"
#include "cairo/cairo_frame.h"
#include "cairo/cairo_fit.h"
#include <pthread.h>

#include "opengl/opengl_state.h"
#include "settings/render_settings.h"
#include "settings/render_settings_common.h"
#ifdef HAVE_OPENGL
#include "opengl-engine/opengl_view.h"
#include "opengl-engine/opengl_view_fit.h"
#include "opengl/opengl_rdpattern.h"
#include "opengl/opengl_msaa.h"
#endif

#ifndef HAVE_OPENGL

/* Hide a widget by builder id; used to remove OpenGL-only toolbar
 * buttons (eg orthographic projection) in Cairo-only builds. */
void
hide_widget_by_id(GtkBuilder *builder, const char *widget_id)
{
  GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(builder, widget_id));
  if( w != NULL )
    gtk_widget_hide(w);
}

#endif /* !HAVE_OPENGL */

static void noise_model_menus_populate(void);
static void noise_interp_menu_set_active(int method);
static void noise_interp_update_sensitivity(void);
static void noise_interp_auto_switch(int fallback);

/* Action flag for NEC2 "card" editors */
static int editor_action = EDITOR_NEW;


/* Motion-event decimation: accept 1 in MOTION_EVENTS_COUNT events.
 * Distinct from VIEW_DRAG_DIVISOR; this controls callback frequency,
 * not drag angular sensitivity. */
#define MOTION_EVENTS_COUNT 8

/*-----------------------------------------------------------------------*/

static int saveas_width;
static int saveas_height;

/* One-shot flags for noise mode compatibility warnings */
static gboolean noise_ground_warned = FALSE;
static gboolean noise_offaxis_warned = FALSE;

/*-----------------------------------------------------------------------*/

/**
 * Check_Noise_Warnings() - emit one-shot Notice() dialogs when the
 * NEC model is incompatible with antenna temperature conventions.
 *
 * Called when entering a noise gain style or when restoring one at
 * program start.
 */
  void
Check_Noise_Warnings(int fstep)
{
  if (fstep < 0 || isFlagClear(ENABLE_RDPAT))
    return;

  /* Ground plane detected */
  if (!noise_ground_warned && gnd.ksymp == 2)
  {
    noise_ground_warned = TRUE;
    Notice(GTK_BUTTONS_OK,
        _("Antenna Temperature"),
        _("Ground plane detected in model. "
          "Ta results will overcount ground effects. "
          "Free-space models are standard for antenna "
          "temperature evaluation."));
  }

  /* Beam not in XY plane */
  int pol = calc_data.pol_type;
  double off_axis = fabs(
      rad_pattern[fstep].max_gain_tht[pol] - 90.0);
  if (!noise_offaxis_warned && fpat.dth > 0.0
      && off_axis > fpat.dth)
  {
    noise_offaxis_warned = TRUE;
    Notice(GTK_BUTTONS_OK,
        _("Antenna Temperature"),
        _("Max gain at θ=%.0f° (%.0f° from XY plane). "
          "Convention assumes beam in XY plane with "
          "observation elevation set by spinbutton."),
        rad_pattern[fstep].max_gain_tht[pol],
        off_axis);
  }
}

/*-----------------------------------------------------------------------*/


char *get_nec_filename_stem(char *dst, char *newext, size_t maxlen)
{
	int i, len;
	char *ext, *p = rc_config.input_file;

	len = strlen(rc_config.input_file);
	for (i = 0; i < len; i++)
	{
		if (rc_config.input_file[i] == '/' || rc_config.input_file[i] == '\\')
			p = &rc_config.input_file[i+1];
	}

	strncpy(dst, p, maxlen);
	ext = strstr(dst, ".nec");
	if (ext != NULL && ext[4] == '\0')
		*ext = '\0';

	if (newext != NULL)
		strncat(dst, newext, maxlen-strlen(dst));

	return dst;
}

  void
on_main_window_destroy(
    GObject     *object,
    gpointer    user_data)
{
  Gtk_Quit();
}


  gboolean
on_main_window_delete_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  kill_window = main_window;
  /* Boundary quit intent: MAIN_QUIT is read upstream by Nec2_Edit_Save and the
   * confirm-dialog re-prompt; xnec2c_request_quit sets it again unconditionally. */
  SetFlag( MAIN_QUIT );

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return( TRUE );

  /* Save GUI state for restoring windows */
  Get_GUI_State();
  Save_Config();

  /* Quit without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
    xnec2c_request_quit();
    return( TRUE );
  }

  Delete_Event( _("Are you sure you wish to quit xnec2c?") );
  return( TRUE );
}


/* Forward declaration: full definition sits near on_fit_view_clicked()
 * later in this file, shared by that mouse handler and the Home-key
 * handling in on_main_window_key_press_event()/
 * on_rdpattern_window_key_press_event() below. */
static void Fit_View( view_t *target, GCallback zoom_handler );

/* Pan_View_On_Arrow_Key()
 *
 * Shared arrow-key panning logic for the structure and radiation pattern
 * windows. By default each pans ONLY its own view (the window the key
 * was pressed in) -- users may want to frame each view differently. If
 * rc_config.common_pan is enabled (View menu: "Common Pan", off by
 * default, mirroring the existing "Common Projection" toggle), the same
 * delta is also applied to the OTHER window's view, keeping both in
 * sync. The two window key-press handlers share this one implementation
 * rather than duplicating the pan-delta logic. Hold Shift for a 4x
 * larger step. Returns TRUE if the key was an arrow key and panning was
 * applied, FALSE otherwise so the caller can fall through to its own
 * unhandled-key behavior.
 */
  static gboolean
Pan_View_On_Arrow_Key( view_t *v, GdkEventKey *event )
{
  const float PAN_STEP_PX      = 15.0f;
  const float PAN_STEP_PX_FAST = 60.0f;
  float step = (event->state & GDK_SHIFT_MASK) ? PAN_STEP_PX_FAST : PAN_STEP_PX;
  float dx = 0.0f, dy = 0.0f;

  switch( event->keyval )
  {
    case GDK_KEY_Left:  dx = -step; break;
    case GDK_KEY_Right: dx =  step; break;
    case GDK_KEY_Up:    dy = -step; break;
    case GDK_KEY_Down:  dy =  step; break;
    default: return( FALSE );
  }

  if( v != NULL )
    view_apply_pan_delta( v, dx, dy );

  if( rc_config.common_pan )
  {
    view_t *other = (v == structure_view) ? rdpattern_view : structure_view;
    if( other != NULL )
      view_apply_pan_delta( other, dx, dy );
  }

  return( TRUE );
}


  gboolean
on_main_window_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  if( event->state & GDK_CONTROL_MASK )
  {
    switch( event->keyval )
    {
      case GDK_KEY_r:
        gtk_widget_grab_focus( GTK_WIDGET(rotate_structure) );
        return( TRUE );

      case GDK_KEY_i:
        gtk_widget_grab_focus( GTK_WIDGET(incline_structure) );
        return( TRUE );

      case GDK_KEY_z:
        gtk_widget_grab_focus( GTK_WIDGET(structure_zoom) );
        return( TRUE );

      case GDK_KEY_f:
        gtk_widget_grab_focus( GTK_WIDGET(mainwin_frequency) );
        return( TRUE );
    }
  }

  if( event->keyval == GDK_KEY_Home )
  {
    Fit_View( structure_view, G_CALLBACK(on_main_zoom_spinbutton_value_changed) );
    return( TRUE );
  }

  if( Pan_View_On_Arrow_Key( structure_view, event ) )
    return( TRUE );

  return( FALSE );
}


  void
on_new_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
        _("A new NEC2 input file may not be created\n"
          "while the Frequency Loop is running") ) )
    return;

  SetFlag( OPEN_NEW_NEC2 );

  /* Reset on opening new file */
  calc_data.FR_cards    = 0;
  calc_data.steps_total = 0;
  if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    calc_data.fmhz_save = 0.0;
  }

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return;

  /* Open editor window if needed */
  if( nec2_edit_window == NULL )
  {
    Close_File( &input_fp );
    Open_Nec2_Editor( NEC2_EDITOR_NEW );
  }
  else Nec2_Input_File_Treeview( NEC2_EDITOR_NEW );

  rc_config.input_file[0] = '\0';
  selected_treeview = cmnt_treeview;
}


  void
on_open_input_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
        _("A new NEC2 input file may not be opened\n"
          "while the Frequency Loop is running")) )
    return;

  SetFlag( OPEN_INPUT );

  /* Reset on opening new file */
  if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    calc_data.fmhz_save = 0.0;
  }

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() )
  {
    SetFlag( NEC2_SAVE );
    return;
  }

  /* Open file chooser to select a NEC2 input file */
  file_chooser = Open_Filechooser(
      GTK_FILE_CHOOSER_ACTION_OPEN,
      "*.nec", NULL, NULL, rc_config.working_dir );
}


  void
on_main_save_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char saveas[FILENAME_LEN + 25];
  size_t s = sizeof( saveas );

  /* Count number of structure image files saved of geometry,
   * currents or charges, to avoid over-writing saved files */
  static int cgm = 0, ccr = 0, cch = 0;

  if( strlen(rc_config.input_file) == 0 ) return;

  /* Make the structure image save file name from input file
   * name. The count of each image type saved is incremented */
  if(struct_view_currents())
    snprintf( saveas, s, "%s-%s_%03d.%s",
        rc_config.input_file, "current", ++ccr, "png" );
  else if(struct_view_charges())
    snprintf( saveas, s, "%s-%s_%03d.%s",
        rc_config.input_file, "charge", ++cch, "png" );
  else
    snprintf( saveas, s, "%s-%s_%03d.%s",
        rc_config.input_file, "geometry", ++cgm, "png" );

  saveas_canvas = CANVAS_STRUCTURE;
  saveas_width  = structure_width;
  saveas_height = structure_height;

  /* Open file chooser to save structure image */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, saveas, rc_config.working_dir );
}


  void
on_main_save_as_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  saveas_canvas = CANVAS_STRUCTURE;
  saveas_width  = structure_width;
  saveas_height = structure_height;

  /* Open file chooser to save structure image */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, get_nec_filename_stem(newfn, ".png", PATH_MAX),
      rc_config.working_dir );
}

  void
on_struct_save_as_gnuplot_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save structure image */
  SetFlag( STRUCT_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.gplot", NULL, get_nec_filename_stem(newfn, "-structure.gplot", PATH_MAX),
      rc_config.working_dir );
}

  void
on_struct_save_currents_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_Currents_CSV;
  filechooser_callback->extension = ".csv";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.csv", NULL, get_nec_filename_stem(newfn, "-currents.csv", PATH_MAX),
      rc_config.working_dir );
}


  void
on_optimizer_output_toggled(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
#ifndef HAVE_INOTIFY
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;

    Notice(GTK_BUTTONS_OK, _("xnec2c geometry optimizer"),
        _("xnec2c was built without inotify support: the optimizer cannot be enabled."));
    ClearFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), FALSE);
    return;
#endif

  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    GtkWidget *w = Builder_Get_Object(main_window_builder, "main_freqplots");

    if (!gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(w)))
        gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(w), TRUE);
    else if(!freq_sweep_complete())
      Start_Frequency_Loop();

    // Do an initial write in case the optimizer is waiting for the .csv:
    if (freq_sweep_complete())
        Write_Optimizer_Data();

    if (!opt_have_files_to_save())
    {
        Notice(GTK_BUTTONS_OK, _("Xnec2c Optimizer"), _("No files are selected for writing. "
            "However, xnec2c will still reload and recalculate the input file on "
            "modification when triggered by inotify."
            "Select files for writing in \"File->Optimization Settings\"."));
    }

    // Start the inotify watcher that reloads the engine on file change
    optimizer_output_start();
  }
  else
  {
    GtkWidget *auto_checkbox = NULL;
    gboolean auto_active = FALSE;

    if( sy_overrides_builder != NULL )
    {
      auto_checkbox = Builder_Get_Object(sy_overrides_builder, "sy_overrides_auto_apply");
      if( auto_checkbox != NULL )
      {
        auto_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_checkbox));
      }
    }

    // Auto-apply active: keep the watcher running
    if( !auto_active )
    {
      optimizer_output_stop();
    }
  }
}


  void
on_quit_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  kill_window = main_window;
  /* Boundary quit intent: MAIN_QUIT is read upstream by Nec2_Edit_Save and the
   * confirm-dialog re-prompt; xnec2c_request_quit sets it again unconditionally. */
  SetFlag( MAIN_QUIT );

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return;

  /* Save GUI state for restoring windows */
  Get_GUI_State();
  Save_Config();

  /* Quit without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
    xnec2c_request_quit();
    return;
  }

  Delete_Event( _("Are you sure you wish to quit xnec2c?") );
}


  void
on_main_rdpattern_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  /* Open radiation pattern rendering window */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    GtkWidget *widget;

    if (rc_config.rdpattern_x < 0 || rc_config.rdpattern_y < 0)
    {
        Get_GUI_State();
        rc_config.rdpattern_x = rc_config.main_x;
        rc_config.rdpattern_y = rc_config.main_y + rc_config.main_height;
    }

    rdpattern_window = create_rdpattern_window( &rdpattern_window_builder );

    /* The transport buttons carry the sweep state, so a freshly built window
     * takes its face and its tooltips from the readout. */
    freq_sweep_controls_refresh();

    /* Spin widgets must be resolved before creating the GL widget:
     * opengl_rdpattern_surface_new() dereferences rdpattern_view,
     * which in turn borrows the spin-button pointers. */
    rotate_rdpattern  = GTK_SPIN_BUTTON( Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_rotate_spinbutton") );
    incline_rdpattern = GTK_SPIN_BUTTON(Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_incline_spinbutton") );
    rdpattern_frequency = GTK_SPIN_BUTTON(Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_freq_spinbutton") );
    rdpattern_zoom = GTK_SPIN_BUTTON(Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_zoom_spinbutton") );
    rdpattern_fstep_entry = GTK_ENTRY(Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_fstep_entry") ) ;

    /* Override the swept controllers on the rotation and incline spins so a
     * wheel notch moves five degrees while a trackpad frame still floors at
     * the one-degree quantum. */
    scroll_install_spin_notches( rotate_rdpattern,  SCROLL_ANGLE_INCREMENT );
    scroll_install_spin_notches( incline_rdpattern, SCROLL_ANGLE_INCREMENT );

    /* Create the rdpattern view before the GL widget; the GL widget
     * constructor installs observers on rdpattern_view and returns
     * NULL otherwise. */
    if( rdpattern_view == NULL )
    {
      rdpattern_view = view_new( VIEW_RDPATTERN,
          rotate_rdpattern, incline_rdpattern, rdpattern_zoom,
          rdpattern_view_changed_cb, NULL );
      view_set_spin_handlers( rdpattern_view,
          G_CALLBACK(on_rdpattern_rotate_spinbutton_value_changed),
          G_CALLBACK(on_rdpattern_incline_spinbutton_value_changed) );
      view_set_drag_mode( rdpattern_view,
          rc_config.view_drag_constrained
              ? VIEW_DRAG_CONSTRAINED : VIEW_DRAG_FREE );
    }

    canvas_add_surface( CANVAS_RDPATTERN,
        cairo_surface_adopt(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_drawingarea" ),
          rdpattern_view) );

#ifdef HAVE_OPENGL
    {
      GtkWidget *box = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_box");

      canvas_add_surface( CANVAS_RDPATTERN,
          opengl_rdpattern_surface_new(GTK_CONTAINER(box)) );

      opengl_set_renderer(rc_config.use_opengl_renderer);
    }
#else
    canvas_set_engine( CANVAS_RDPATTERN, &cairo_engine );

    hide_widget_by_id(rdpattern_window_builder, "rdpattern_ortho_button");
#endif
    canvas_sync_viewport( CANVAS_RDPATTERN );

    /* Restore radiation pattern window widget state from the bound config
     * fields, then run each tree's hook to refresh the derived draw flags. */
    config_widget_sync_builder( &rdpattern_window_builder );
    config_widget_run_hooks( &rdpattern_window_builder );

    /* Request geometry and show after all structural mutations and
     * widget state restorations so sizing is the last layout operation */
    Set_Window_Geometry( rdpattern_window,
        rc_config.rdpattern_x, rc_config.rdpattern_y,
        rc_config.rdpattern_width, rc_config.rdpattern_height );
    gtk_widget_show( rdpattern_window );
    Update_Window_Titles();

    Main_Rdpattern_Activate( TRUE );

    /* Restore gain style */
    Set_Gain_Style(rc_config.gain_style);

    /* Populate and restore noise model sub-menus */
    noise_model_menus_populate();

    /* Restore elevation spinbutton */
    widget = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_elevation_spinbutton");
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), rc_config.ant_temp_elevation);

#ifdef HAVE_OPENGL
    /* Establish view sharing after all initialization completes.
     * Single entry point so persistence-to-runtime mapping lives in
     * opengl_common_projection_sync(). */
    opengl_common_projection_sync();

    /* Sync ortho toolbar button now that rdpattern window builder is ready */
    render_settings_sync_from_config();
#endif

  } /* if( gtk_check_menu_item_get_active(...) ) */
  else if( isFlagSet(DRAW_ENABLED) )
    Gtk_Widget_Destroy( &rdpattern_window );
}


/* Right-click a plot-select button to open that graph's detached popup; the
 * panel travels as the connection's user data so one handler serves every
 * button.  A GtkButton toggles only on the primary button, so the button's
 * main-window selection state is left unchanged.  Other buttons fall through
 * to the default toggle handler. */
  static gboolean
freqplots_panel_button_press_cb( GtkWidget *widget, GdkEventButton *event,
    gpointer user_data )
{
  (void)widget;

  if( event->button != 3 )
    return FALSE;

  freqplots_open_panel( (fp_panel_t)GPOINTER_TO_INT(user_data) );
  return TRUE;
}

/* Wire each plot-select button's right-click to its popup opener; the panel
 * descriptor table names each panel's button widget. */
  static void
freqplots_connect_panel_buttons( GtkBuilder *builder )
{
  int p;

  for( p = 0; p < FP_PANEL_COUNT; p++ )
  {
    GtkWidget *btn = Builder_Get_Object( builder,
        freqplots_panel_select_id( p ) );

    g_signal_connect( btn, "button-press-event",
        G_CALLBACK(freqplots_panel_button_press_cb),
        GINT_TO_POINTER(p) );
  }
}


  void
on_main_freqplots_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  /* Open window for plotting frequency
   * related data (gain, vswr etc) */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    if( Main_Freqplots_Activate() )
    {
      if (rc_config.freqplots_x < 0 || rc_config.rdpattern_y < 0)
      {
          Get_GUI_State();
          rc_config.freqplots_x = rc_config.main_x + rc_config.main_width;
          rc_config.freqplots_y = rc_config.main_y;
      }

      freqplots_window = create_freqplots_window( &freqplots_window_builder );

      /* The transport buttons carry the sweep state, so a freshly built
       * window takes its face and its tooltips from the readout. */
      freq_sweep_controls_refresh();

      GtkWidget *fp_da = Builder_Get_Object(
          freqplots_window_builder, "freqplots_drawingarea" );
      freqplots_main_view()->window      = freqplots_window;
      freqplots_main_view()->canvas      = CANVAS_FREQPLOTS;
      canvas_add_surface( CANVAS_FREQPLOTS,
          cairo_surface_adopt(fp_da, NULL) );
      canvas_set_engine( CANVAS_FREQPLOTS, &cairo_engine );
      freqplots_main_view()->filter      = FP_PANEL_ALL;
      g_object_set_data( G_OBJECT(fp_da), "fp_view", freqplots_main_view() );
      freqplots_connect_panel_buttons( freqplots_window_builder );
      Set_Window_Labels();
      calc_data.ngraph = 0;

      /* Set the Zo spinbutton value */
      GtkWidget *spin = Builder_Get_Object(
          freqplots_window_builder, "freqplots_zo_spinbutton" );
      gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin), (gdouble)calc_data.zo );

      /* Populate the excitation-port selector for the current model. */
      freqplots_populate_port_combo();

      GtkAllocation alloc;
      gtk_widget_get_allocation( fp_da, &alloc );
      freqplots_main_view()->width  = alloc.width;
      freqplots_main_view()->height = alloc.height;

      /* Restore frequency plots window widget state from the bound config
       * fields, then run each tree's hook to set the derived plot-select
       * flags and recompute the active-plot count. */
      config_widget_sync_builder( &freqplots_window_builder );
      config_widget_run_hooks( &freqplots_window_builder );

      /* Gray feedpoint-dependent widgets for feedpoint-less excitations. */
      freqplots_gate_feedpoint_widgets();

      /* Request geometry and show after all widget state restorations
       * so sizing is the last layout operation */
      Set_Window_Geometry( freqplots_window,
          rc_config.freqplots_x, rc_config.freqplots_y,
          rc_config.freqplots_width, rc_config.freqplots_height );
      gtk_widget_show( freqplots_window );
      Update_Window_Titles();

      if( (rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS)) && !freq_sweep_has_results())
        Start_Frequency_Loop();

    } /* if( Main_Freqplots_Activate() */
    else gtk_check_menu_item_set_active(
        GTK_CHECK_MENU_ITEM(menuitem), FALSE );
  }
  else if( isFlagSet(PLOT_ENABLED) )
    Gtk_Widget_Destroy( &freqplots_window );
}


/* opengl_common_projection_sync()
 *
 * Public entry point used by main.c when the rdpattern window is
 * already open during file load.  Applies the persisted common-
 * projection preference by installing or removing the master-follower
 * link between the two view_t instances.
 */
  void
opengl_common_projection_sync(void)
{
  if( rdpattern_view == NULL || structure_view == NULL )
    return;

  if( rc_config.main_common_projection )
    view_share_master( rdpattern_view, structure_view );
  else
    view_unshare_master( rdpattern_view );
}

/* Common_Pan_Sync()
 *
 * Public entry point (mirrors opengl_common_projection_sync() above,
 * called the same way from main.c on new file load) applying the
 * persisted common-pan preference. Unlike projection, pan isn't a
 * live master/follower link -- Pan_View_On_Arrow_Key() already keeps
 * both views moving together going forward once enabled by mirroring
 * every arrow-key delta to both. What's still needed is the one-time
 * "snap to match" the moment the setting turns on (or a new file
 * loads with it already on), same as toggling Common Projection
 * immediately re-aligns the rdpattern view rather than waiting for
 * the next rotation. Only the rdpattern view is moved to match
 * structure_view; the reverse direction has no natural default since
 * either view could be "correct" -- structure_view is treated as the
 * reference the same way it's the master for projection sharing.
 */
  void
Common_Pan_Sync(void)
{
  if( !rc_config.common_pan )
    return;

  if( rdpattern_view == NULL || structure_view == NULL )
    return;

  rdpattern_view->pan_offset[0] = structure_view->pan_offset[0];
  rdpattern_view->pan_offset[1] = structure_view->pan_offset[1];
  view_notify_change( rdpattern_view );
}

/*-----------------------------------------------------------------------*/

/* view_presets - indexed by preset id: 0=X axis, 1=Y axis, 2=Z axis, 3=default */
static const struct { double wr; double wi; } view_presets[4] = {
  { VIEW_PRESET_X_WR, VIEW_PRESET_X_WI },
  { VIEW_PRESET_Y_WR, VIEW_PRESET_Y_WI },
  { VIEW_PRESET_Z_WR, VIEW_PRESET_Z_WI },
  { VIEW_DEFAULT_WR,  VIEW_DEFAULT_WI  },
};

/* preset_ids - widget IDs indexed by [win_idx][preset]; win_idx: 0=main, 1=rdpattern */
static const char *preset_ids[2][4] = {
  { "main_x_axis",      "main_y_axis",      "main_z_axis",      "main_default_view" },
  { "rdpattern_x_axis", "rdpattern_y_axis", "rdpattern_z_axis", "rdpattern_default_view" },
};

/**
 * window_type_from_widget - determine origin window from widget hierarchy
 * @widget: any widget belonging to the main or rdpattern window
 *
 * Compares the widget's toplevel against stored window globals.
 * Returns MAIN_WINDOW for the structure window, RDPATTERN_WINDOW otherwise.
 */
  static window_t
window_type_from_widget(GtkWidget *widget)
{
  GtkWidget *top = gtk_widget_get_toplevel(widget);
  return (top == rdpattern_window) ? RDPATTERN_WINDOW : MAIN_WINDOW;
}

/**
 * set_view_preset - apply preset viewing angle to the target window
 * @wr:          rotate angle in degrees
 * @wi:          incline angle in degrees
 * @window_type: MAIN_WINDOW for structure, RDPATTERN_WINDOW for radiation pattern
 *
 * view_set_angles rewrites the rotation matrix and fires observers
 * that update spin widgets and queue redraws on both renderers.
 * view_reset_pan clears the accumulated pan offset separately so the
 * preset lands centered.
 */
  static void
set_view_preset(double wr, double wi, window_t window_type)
{
  view_t *target =
      (window_type == MAIN_WINDOW) ? structure_view : rdpattern_view;

  if( target == NULL )
    return;

  view_set_angles( target, wr, wi );
  view_reset_pan( target );
}

/*-----------------------------------------------------------------------*/

  void
on_view_preset_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  window_t wt = window_type_from_widget(GTK_WIDGET(button));
  GtkBuilder *builder = (wt == MAIN_WINDOW)
    ? main_window_builder : rdpattern_window_builder;
  int win_idx = (wt == MAIN_WINDOW) ? 0 : 1;
  int preset = 3;
  int i;

  for( i = 0; i < 4; i++ )
  {
    if( GTK_WIDGET(button) == Builder_Get_Object(builder, preset_ids[win_idx][i]) )
    {
      preset = i;
      break;
    }
  }

  set_view_preset(view_presets[preset].wr, view_presets[preset].wi, wt);
}


  void
on_main_rotate_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  (void)spinbutton;
  (void)user_data;

  if( structure_view == NULL )
    return;

  /* view_set_angles() rebuilds R and fires view_update_spin_display()
   * via the observer; gtk_spin_button_update() is omitted because
   * its text-to-value resync re-emits value-changed and re-enters. */
  view_set_angles( structure_view,
      gtk_spin_button_get_value( rotate_structure ),
      gtk_spin_button_get_value( incline_structure ) );

} /* on_main_rotate_spinbutton_value_changed() */


  void
on_main_incline_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  (void)spinbutton;
  (void)user_data;

  if( structure_view == NULL )
    return;

  view_set_angles( structure_view,
      gtk_spin_button_get_value( rotate_structure ),
      gtk_spin_button_get_value( incline_structure ) );

} /* on_main_incline_spinbutton_value_changed() */


  gboolean
on_colorcode_drawingarea_draw(
    GtkWidget       *widget,
    cairo_t         *cr,
    gpointer         user_data)
{
  draw_colorcode_projected( cr );
  return( TRUE );
}


  void
on_new_freq_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  if(!freq_sweep_active())
  {
    GtkSpinButton *sb = (window_type_from_widget(GTK_WIDGET(button)) == MAIN_WINDOW)
      ? mainwin_frequency : rdpattern_frequency;
    user_set_frequency((double)gtk_spin_button_get_value(sb));
  }
}


  gboolean
on_structure_drawingarea_configure_event(
    GtkWidget       *widget,
    GdkEventConfigure *event,
    gpointer         user_data)
{
  structure_width  = event->width;
  structure_height = event->height;
  if( structure_view != NULL )
    view_set_viewport( structure_view,
        structure_width, structure_height );

  return( TRUE );
}


  gboolean
on_structure_drawingarea_motion_notify_event(
    GtkWidget       *widget,
    GdkEventMotion  *event,
    gpointer         user_data)
{
  static int cnt = 0;

  /* Use only 1 in MOTION_EVENTS_COUNT event */
  if( (cnt++ < MOTION_EVENTS_COUNT) ||
      isFlagSet(BLOCK_MOTION_EV) )
    return( FALSE );

  cnt = 0;

  /* Handle motion events */
  if( structure_view != NULL )
    Motion_Event( event, structure_view );

  return( TRUE );
}


/** on_structure_drawingarea_draw() - Render the structure view with active theme colors
 * @widget: structure drawing area, which owns the surface it presents
 * @cr: Cairo context for the current frame
 * @user_data: unused callback data
 */
  gboolean
on_structure_drawingarea_draw(
    GtkWidget       *widget,
    cairo_t         *cr,
    gpointer         user_data)
{
  (void)user_data;

  return render_cairo( cairo_surface_of_widget(widget), cr );
}


  void
on_filechooserdialog_response(
    GtkDialog       *dialog,
    gint             response_id,
    gpointer         user_data)
{
  Filechooser_Response(
      dialog, response_id, saveas_width, saveas_height );
  if( response_id != GTK_RESPONSE_OK )
    Gtk_Widget_Destroy( &file_chooser );
}


  void
on_filechooserdialog_destroy(
    GObject       *object,
    gpointer      user_data)
{
  file_chooser = NULL;
}


  gboolean
on_freqplots_window_delete_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  /* Capture window state before destroy */
  get_freqplots_window_state();

  /* Close freq plots window without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
    Gtk_Widget_Destroy( &freqplots_window );
    return( TRUE );
  }

  kill_window = freqplots_window;
  Delete_Event( _("Are you sure you wish to close this window?") );
  return( TRUE );
}


  void
on_freqplots_window_destroy(
    GObject       *object,
    gpointer       user_data)
{
  Plots_Window_Killed();
}


  void
on_freqplots_save_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char saveas[FILENAME_LEN + 24];
  size_t s = sizeof( saveas );
  static int cnt = 0;

  if( (strlen(rc_config.input_file) == 0) ||
      (freqplots_count_selected() < 1) )
    return;

  saveas_canvas = CANVAS_FREQPLOTS;
  saveas_width  = freqplots_main_view()->width;
  saveas_height = freqplots_main_view()->height;

  /* Make file name from input file name,
   * to save frequency plots drawing */
  snprintf( saveas, s, "%s-%s_%03d%s",
      rc_config.input_file, "plots", ++cnt, ".png" );

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, saveas, rc_config.working_dir );
}


  void
on_freqplots_save_as_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  saveas_canvas = CANVAS_FREQPLOTS;
  saveas_width  = freqplots_main_view()->width;
  saveas_height = freqplots_main_view()->height;

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, get_nec_filename_stem(newfn, ".png", PATH_MAX),
      rc_config.working_dir );
}


  void
on_freqplots_save_as_gnuplot_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */
  SetFlag( PLOTS_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.gplot", NULL, get_nec_filename_stem(newfn, ".gplot", PATH_MAX),
      rc_config.working_dir );
}

  void
on_freqplots_save_as_s1p_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_FreqPlots_S1P;
  filechooser_callback->extension = ".s1p";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.s1p", NULL, get_nec_filename_stem(newfn, ".s1p", PATH_MAX),
      rc_config.working_dir );
}

  void
on_freqplots_save_as_s2p_max_gain_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_FreqPlots_S2P_Max_Gain;
  filechooser_callback->extension = ".s2p";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.s2p", NULL, get_nec_filename_stem(newfn, "-maxgain.s2p", PATH_MAX),
      rc_config.working_dir );
}

  void
on_freqplots_save_as_s2p_viewer_gain_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_FreqPlots_S2P_Viewer_Gain;
  filechooser_callback->extension = ".s2p";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.s2p", NULL, get_nec_filename_stem(newfn, "-viewergain.s2p", PATH_MAX),
      rc_config.working_dir );
}

  void
on_freqplots_save_as_csv_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_FreqPlots_CSV;
  filechooser_callback->extension = ".csv";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.csv", NULL, get_nec_filename_stem(newfn, ".csv", PATH_MAX),
      rc_config.working_dir );
}

  void
on_freqplots_zo_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  /* Set the value of Z0 used for VSWR calculations */
  calc_data.zo = gtk_spin_button_get_value(spinbutton);
  if( isFlagSet(PLOT_ENABLED) )
  {
    freqplots_redraw_all(TRUE);
  }

  gtk_spin_button_update( spinbutton );
}


  gboolean
on_freqplots_drawingarea_draw(
    GtkWidget       *widget,
    cairo_t         *cr,
    gpointer         user_data)
{
  freqplots_view_t *view = g_object_get_data( G_OBJECT(widget), "fp_view" );

  if( view == NULL )
    return( FALSE );

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
    return( FALSE );

  /* Draw the frequency dependent data plots */
  Plot_Frequency_Data( view, cr );
  return( TRUE );
}


  gboolean
on_freqplots_drawingarea_configure_event(
    GtkWidget       *widget,
    GdkEventConfigure *event,
    gpointer         user_data)
{
  freqplots_view_t *view = g_object_get_data( G_OBJECT(widget), "fp_view" );

  if( view == NULL )
    return( FALSE );

  view->width  = event->width;
  view->height = event->height;

  return( TRUE );
}


  gboolean
on_freqplots_drawingarea_button_press_event(
    GtkWidget       *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  freqplots_view_t *view = g_object_get_data( G_OBJECT(widget), "fp_view" );

  if( view == NULL )
    return( FALSE );

  /* Double-click detaches the clicked graph into its own popup window. */
  if( event->type == GDK_2BUTTON_PRESS )
  {
    fp_panel_t panel = freqplots_panel_at( view, event->x, event->y );

    if( panel != FP_PANEL_ALL )
    {
      freqplots_open_panel( panel );
      return( TRUE );
    }
  }

  Set_Frequency_On_Click( view, (GdkEvent*)event );
  return( TRUE );
}

  gboolean
on_freqplots_drawingarea_scroll_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  freqplots_view_t *view = g_object_get_data( G_OBJECT(widget), "fp_view" );

  if( view == NULL )
    return( FALSE );

  Set_Frequency_On_Click( view, event );
  return TRUE;
}

  gboolean
on_freqplots_drawingarea_motion_notify_event(
    GtkWidget       *widget,
    GdkEventMotion  *event,
    gpointer         user_data)
{
  freqplots_view_t *view = g_object_get_data( G_OBJECT(widget), "fp_view" );

  if( view == NULL )
    return( FALSE );

  Set_Frequency_On_Click( view, (GdkEvent*)event );
  return( TRUE );
}

/* Popup teardown: release the heap view for this graph type.  GTK is already
 * destroying the window, so only the registry slot and view are freed. */
  void
on_freqplots_popup_destroy(
    GtkWidget       *widget,
    gpointer         user_data)
{
  freqplots_view_t *v = user_data;

  freqplots_close_panel( v->filter );
}

/* Ctrl+W closes a frequency-plots popup window; the window's destroy handler
 * then releases its view. */
  gboolean
on_freqplots_popup_key_press_event(
    GtkWidget       *widget,
    GdkEventKey     *event,
    gpointer         user_data)
{
  if( (event->keyval == GDK_KEY_w) &&
      (event->state & GDK_CONTROL_MASK) )
  {
    gtk_widget_destroy( widget );
    return( TRUE );
  }

  return( FALSE );
}

  void
on_rdpattern_window_destroy(
    GObject       *object,
    gpointer       user_data)
{
#ifdef HAVE_OPENGL
  /* Detach follower so the structure view's observer list no longer
   * references the rdpattern view that is about to be freed. */
  if( rdpattern_view != NULL && rdpattern_view->rotation_master != NULL )
    view_unshare_master( rdpattern_view );

  /* Release in reverse acquisition order: the GL view state borrows
   * rdpattern_view for resize, render and button handling, so the widget
   * holding that pointer dies before the view it reads. */
  opengl_rdpattern_cleanup();
#endif

  view_free( &rdpattern_view );

  Rdpattern_Window_Killed();
}


  gboolean
on_rdpattern_window_delete_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  /* Capture window state before destroy */
  get_rdpattern_window_state();

  /* Close rdpattern window without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
    Gtk_Widget_Destroy( &rdpattern_window );
    return( TRUE );
  }

  kill_window = rdpattern_window;
  Delete_Event( _("Are you sure you wish to close this window?") );
  return( TRUE );
}


  void
on_rdpattern_save_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char saveas[FILENAME_LEN + 24];
  size_t s = sizeof( saveas );
  static int cgn = 0, ceh = 0;

  if( strlen(rc_config.input_file) == 0 ) return;

  saveas_canvas = CANVAS_RDPATTERN;
  saveas_width  = rdpattern_view->width;
  saveas_height = rdpattern_view->height;

  /* Make the rad pattern save
   * file name from input name */
  if(rdpat_gain_active())
    snprintf( saveas, s, "%s-%s_%03d%s",
        rc_config.input_file, "gain", ++cgn, ".png" );
  else if(rdpat_ehfield_active())
    snprintf( saveas, s, "%s-%s_%03d%s",
        rc_config.input_file, "fields", ++ceh, ".png" );
  else return;

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, saveas, rc_config.working_dir );
}


  void
on_rdpattern_save_as_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  saveas_canvas = CANVAS_RDPATTERN;
  saveas_width  = rdpattern_view->width;
  saveas_height = rdpattern_view->height;

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.png", NULL, get_nec_filename_stem(newfn, ".png", PATH_MAX),
      rc_config.working_dir );
}


  void
on_rdpattern_save_as_gnuplot_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */
  SetFlag( RDPAT_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.gplot", NULL, get_nec_filename_stem(newfn, "-radpattern.gplot", PATH_MAX),
      rc_config.working_dir );
}

  void
on_rdpattern_save_as_csv_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  char newfn[PATH_MAX];
  /* Open file chooser to save frequency plots */

  mem_new(&filechooser_callback);
  filechooser_callback->callback = Save_RadPattern_CSV;
  filechooser_callback->extension = ".csv";
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.csv", NULL, get_nec_filename_stem(newfn, "-radpattern.csv", PATH_MAX),
      rc_config.working_dir );
}


  void
on_rdpattern_linear_power_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_LINP );
}


  void
on_rdpattern_linear_voltage_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_LINV );
}


  void
on_rdpattern_arrl_style_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_ARRL );
}


  void
on_rdpattern_logarithmic_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_LOG );
}


  void
on_rdpattern_noise_temp_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_NOISE );
}


  void
on_rdpattern_noise_temp_log_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    Set_Gain_Style( GS_NOISE_LOG );
}

/**
 * noise_interp_menu_set_active() - sync interp radio to given method
 * @method: ant_temp_method_t value to activate
 *
 * Searches the flat noise model menu for interp radio items tagged
 * with "interp-idx" and activates the matching one.
 */
static void
noise_interp_menu_set_active(int method)
{
  GtkWidget *menu = Builder_Get_Object(
      rdpattern_window_builder, "rdpattern_noise_env_menu_menu");
  if (!menu)
    return;

  GList *children = gtk_container_get_children(GTK_CONTAINER(menu));
  for (GList *l = children; l; l = l->next)
  {
    gpointer data = g_object_get_data(G_OBJECT(l->data), "interp-idx");
    if (!data)
      continue;

    /* Stored as index+1 so that index 0 is distinguishable from NULL */
    int idx = GPOINTER_TO_INT(data) - 1;
    if (idx == method)
    {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(l->data), TRUE);
      break;
    }
  }
  g_list_free(children);
}

/**
 * noise_interp_update_sensitivity() - enable/disable interp menu items
 *
 * Computes the union of valid_interp bitmasks from the currently
 * selected sky and earth models.  Sets each interpolation menu item
 * sensitive if its method bit is present in the union.
 */
static void
noise_interp_update_sensitivity(void)
{
  GtkWidget *menu = Builder_Get_Object(
      rdpattern_window_builder, "rdpattern_noise_env_menu_menu");
  if (!menu)
    return;

  uint8_t allowed = sky_models[rc_config.ant_temp_sky].valid_interp
      | earth_models[rc_config.ant_temp_earth].valid_interp;

  GList *children = gtk_container_get_children(GTK_CONTAINER(menu));
  for (GList *l = children; l; l = l->next)
  {
    gpointer data = g_object_get_data(G_OBJECT(l->data), "interp-idx");
    if (!data)
      continue;

    int idx = GPOINTER_TO_INT(data) - 1;
    gtk_widget_set_sensitive(GTK_WIDGET(l->data),
        (allowed & ANT_TEMP_METHOD_BIT(idx)) != 0);
  }
  g_list_free(children);
}

/**
 * noise_interp_auto_switch() - validate interp against current models
 * @fallback: method to switch to if current interp is invalid
 *
 * Computes the union of valid interp methods from the selected sky
 * and earth models.  If the current interp is outside that set,
 * switches to the fallback.  Updates menu sensitivity and active item.
 */
static void
noise_interp_auto_switch(int fallback)
{
  /* Clamp fallback to menu-visible methods; formula/galactic models
   * pass their intrinsic method which has no menu radio item */
  if (fallback != ANT_TEMP_SNAP && fallback != ANT_TEMP_INTERP)
    fallback = ANT_TEMP_INTERP;

  uint8_t allowed = sky_models[rc_config.ant_temp_sky].valid_interp
      | earth_models[rc_config.ant_temp_earth].valid_interp;
  if (!(allowed & ANT_TEMP_METHOD_BIT(rc_config.ant_temp_interp)))
    rc_config.ant_temp_interp = fallback;
  noise_interp_update_sensitivity();
  noise_interp_menu_set_active(rc_config.ant_temp_interp);
}

/* Noise model radio menu item callbacks — one per selector */

static void
on_noise_sky_activate(GtkCheckMenuItem *item, gpointer user_data)
{
  if (!gtk_check_menu_item_get_active(item))
    return;

  int idx = GPOINTER_TO_INT(user_data);
  rc_config.ant_temp_sky = idx;
  noise_interp_auto_switch(sky_models[idx].method);
  freq_step_refresh_ui(TRUE);
}

static void
on_noise_earth_activate(GtkCheckMenuItem *item, gpointer user_data)
{
  if (!gtk_check_menu_item_get_active(item))
    return;

  int idx = GPOINTER_TO_INT(user_data);
  rc_config.ant_temp_earth = idx;
  noise_interp_auto_switch(earth_models[idx].method);
  freq_step_refresh_ui(TRUE);
}

static void
on_noise_interp_activate(GtkCheckMenuItem *item, gpointer user_data)
{
  if (!gtk_check_menu_item_get_active(item))
    return;

  rc_config.ant_temp_interp = GPOINTER_TO_INT(user_data);
  freq_step_refresh_ui(TRUE);
}

/* Side context bound by the sky and earth focus-out handlers.
 * Fields point to the rc_config slots the handler edits so the commit
 * helper has no flag parameter. */
typedef struct
{
  double   ref;            /* resolved reference temperature (K) */
  int      ok;             /* ant_temp_resolve status (1 = ref valid) */
  int     *model_sel;      /* &rc_config.ant_temp_{sky,earth} */
  int      custom_enum;    /* ANT_TEMP_{SKY,EARTH}_CUSTOM */
  double  *custom_store;   /* &rc_config.ant_temp_custom_t_{sky,earth} */
} ant_temp_commit_ctx_t;

/**
 * ant_temp_resolve_current() - resolve sky/earth at current freq step
 * @t_sky:   output sky brightness temperature (K)
 * @t_earth: output earth/man-made noise temperature (K)
 *
 * Returns 1 when the resolve succeeded, 0 otherwise. Output parameters
 * are valid only when the return is 1.
 */
static int
ant_temp_resolve_current(double *t_sky, double *t_earth)
{
  return ant_temp_resolve(save.freq[calc_data.freq_step],
      rc_config.ant_temp_sky, rc_config.ant_temp_earth,
      rc_config.ant_temp_interp, t_sky, t_earth) == 0;
}

/**
 * ant_temp_entry_commit() - parse an entry and commit to Custom if changed
 * @widget: GtkEntry being edited
 * @c:      side context with resolved reference and rc_config slots
 *
 * Parses the entry text. On parse failure or out-of-range, restores the
 * displayed value from the side's resolved reference. On successful parse
 * with delta exceeding ANT_TEMP_K_EPSILON, stores the constant, selects
 * the Custom model for this side, snaps interp, and triggers recomputation.
 */
static void
ant_temp_entry_commit(GtkWidget *widget, ant_temp_commit_ctx_t c)
{
  const char *text = gtk_entry_get_text(GTK_ENTRY(widget));
  char *endptr = NULL;
  double val = strtod(text, &endptr);

  /* Parse failure or out-of-range: restore displayed value and bail */
  if (endptr == text || val <= ANT_TEMP_K_MIN)
  {
    if (c.ok)
      ant_temp_entry_set_kelvin(widget, c.ref);
    else
      ant_temp_entry_set_unresolved(widget);
    return;
  }

  /* Commit when resolution failed (no reference) or delta exceeds epsilon */
  if (!c.ok || fabs(val - c.ref) > ANT_TEMP_K_EPSILON)
  {
    *c.custom_store = val;
    *c.model_sel = c.custom_enum;
    noise_interp_auto_switch(ANT_TEMP_SNAP);
    freq_step_refresh_ui(TRUE);
  }
}

/**
 * on_rdpattern_t_earth_focus_out() - focus-out handler for earth temp entry
 * @_event:     unused; required by the focus-out-event GSignal signature
 * @_user_data: unused; required by the GCallback signature
 */
gboolean
on_rdpattern_t_earth_focus_out(GtkWidget *widget,
    GdkEvent *_event, gpointer _user_data)
{
  double t_sky, t_earth;
  int ok = ant_temp_resolve_current(&t_sky, &t_earth);
  ant_temp_entry_commit(widget, (ant_temp_commit_ctx_t){
      .ref          = t_earth,
      .ok           = ok,
      .model_sel    = &rc_config.ant_temp_earth,
      .custom_enum  = ANT_TEMP_EARTH_CUSTOM,
      .custom_store = &rc_config.ant_temp_custom_t_earth,
  });
  return FALSE;
}

/**
 * on_rdpattern_t_sky_focus_out() - focus-out handler for sky temp entry
 * @_event:     unused; required by the focus-out-event GSignal signature
 * @_user_data: unused; required by the GCallback signature
 */
gboolean
on_rdpattern_t_sky_focus_out(GtkWidget *widget,
    GdkEvent *_event, gpointer _user_data)
{
  double t_sky, t_earth;
  int ok = ant_temp_resolve_current(&t_sky, &t_earth);
  ant_temp_entry_commit(widget, (ant_temp_commit_ctx_t){
      .ref          = t_sky,
      .ok           = ok,
      .model_sel    = &rc_config.ant_temp_sky,
      .custom_enum  = ANT_TEMP_SKY_CUSTOM,
      .custom_store = &rc_config.ant_temp_custom_t_sky,
  });
  return FALSE;
}

/**
 * on_rdpattern_ant_temp_activate() - Enter key in earth/sky temp entry
 * @_user_data: unused; required by the GCallback signature
 *
 * Moves focus away so the focus-out handler applies the value.
 */
void
on_rdpattern_ant_temp_activate(GtkEntry *entry, gpointer _user_data)
{
  GtkWidget *top = gtk_widget_get_toplevel(GTK_WIDGET(entry));
  gtk_widget_child_focus(top, GTK_DIR_TAB_FORWARD);
}

/**
 * noise_menu_append_separator() - add a separator to the noise menu
 * @menu: target GtkMenuShell
 */
static void
noise_menu_append_separator(GtkWidget *menu)
{
  GtkWidget *sep = gtk_separator_menu_item_new();
  gtk_widget_show(sep);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
}

/**
 * noise_menu_append_heading() - add a disabled label as section heading
 * @menu:  target GtkMenuShell
 * @label: heading text
 */
static void
noise_menu_append_heading(GtkWidget *menu, const char *label)
{
  GtkWidget *item = gtk_menu_item_new_with_label(label);
  gtk_widget_set_sensitive(item, FALSE);
  gtk_widget_show(item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

/**
 * noise_model_menus_populate() - build radio items in noise model menu
 *
 * Populates the flat noise model menu from the model registries,
 * separated by section headings and GtkSeparatorMenuItems.
 * Sets active items from rc_config.  Called once when the
 * rdpattern window opens.
 */
static void
noise_model_menus_populate(void)
{
  GtkWidget *menu = Builder_Get_Object(
      rdpattern_window_builder, "rdpattern_noise_env_menu_menu");

  GSList *sky_group = NULL;
  GSList *earth_group = NULL;
  GSList *interp_group = NULL;

  /* Sky models */
  noise_menu_append_heading(menu, "Sky Model");
  for (int i = 0; i < ANT_TEMP_SKY_COUNT; i++)
  {
    GtkWidget *item = gtk_radio_menu_item_new_with_label(sky_group, sky_models[i].name);
    sky_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    if (i == rc_config.ant_temp_sky)
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    g_signal_connect(item, "toggled",
        G_CALLBACK(on_noise_sky_activate), GINT_TO_POINTER(i));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  }

  /* Earth models */
  noise_menu_append_separator(menu);
  noise_menu_append_heading(menu, "Earth Model");
  for (int i = 0; i < ANT_TEMP_EARTH_COUNT; i++)
  {
    GtkWidget *item = gtk_radio_menu_item_new_with_label(earth_group, earth_models[i].name);
    earth_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    if (i == rc_config.ant_temp_earth)
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    g_signal_connect(item, "toggled",
        G_CALLBACK(on_noise_earth_activate), GINT_TO_POINTER(i));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  }

  /* Interpolation method (user-selectable for table models only;
   * formula and galactic are intrinsic to their models) */
  noise_menu_append_separator(menu);
  noise_menu_append_heading(menu, "Interpolation");
  for (int i = 0; i < ANT_TEMP_METHOD_COUNT; i++)
  {
    if (i == ANT_TEMP_FORMULA || i == ANT_TEMP_GALACTIC)
      continue;

    GtkWidget *item = gtk_radio_menu_item_new_with_label(
        interp_group, ant_temp_method_names[i]);
    interp_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    /* Store index+1 so that index 0 is distinguishable from NULL */
    g_object_set_data(G_OBJECT(item), "interp-idx", GINT_TO_POINTER(i + 1));
    if (i == rc_config.ant_temp_interp)
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    g_signal_connect(item, "toggled",
        G_CALLBACK(on_noise_interp_activate), GINT_TO_POINTER(i));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  }

  noise_interp_update_sensitivity();
}


/* Elevation spin button callback */

  void
on_rdpattern_elevation_spinbutton_value_changed(
    GtkSpinButton *spinbutton, gpointer user_data)
{
  rc_config.ant_temp_elevation = gtk_spin_button_get_value(spinbutton);
  freq_step_refresh_ui(TRUE);
}


  void
opengl_set_renderer(gboolean enable)
{
#ifdef HAVE_OPENGL
  /* A display whose GL context creation failed cannot present the GL engine,
   * so the request resolves to Cairo before the setting and the canvases
   * are written from it. */
  if( enable && opengl_gl_context_failed() )
  {
    pr_warn("OpenGL is not available on this display; cannot enable renderer.\n");
    enable = FALSE;
  }

  rc_config.use_opengl_renderer = enable ? 1 : 0;

  /* Renderer toggle is the authoritative drag-neutral point.  Cairo
   * Motion_Event writes drag_button as a side effect of every processed
   * motion sample based on event->state; throttling via MOTION_EVENTS_COUNT
   * can leave drag_button non-NONE after a release.  The GL on_motion
   * handler trusts drag_button alone, so a stale Cairo drag_button would
   * make the first mouse move over the freshly-shown GL widget look like
   * an ongoing drag.  Clear both views unconditionally, and clear
   * BLOCK_MOTION_EV in case a Cairo motion handler was interrupted. */
  if( structure_view != NULL )
    view_end_drag( structure_view );
  if( rdpattern_view != NULL )
    view_end_drag( rdpattern_view );
  ClearFlag( BLOCK_MOTION_EV );

  const render_engine_t *engine = enable ? &gl_engine : &cairo_engine;

  /* Swap renderer if radiation pattern window is open */
  if( canvas_set_engine( CANVAS_RDPATTERN, engine ) )
  {
    /* Paint the freshly swapped widget during setup, before DRAW_ENABLED
     * gates Queue_Radiation_Redraw() */
    canvas_queue_redraw( CANVAS_RDPATTERN, TRUE );
  }

  /* Swap renderer for structure view in main window */
  if( canvas_set_engine( CANVAS_STRUCTURE, engine ) )
    Queue_Structure_Rebuild( TRUE );
#endif
}


  void
opengl_set_constrained_rotation(gboolean constrained)
{
  drag_mode_t mode;

  rc_config.view_drag_constrained = constrained ? 1 : 0;
  mode = constrained ? VIEW_DRAG_CONSTRAINED : VIEW_DRAG_FREE;

  if( structure_view != NULL )
    view_set_drag_mode( structure_view, mode );

  if( rdpattern_view != NULL )
    view_set_drag_mode( rdpattern_view, mode );

} /* opengl_set_constrained_rotation() */


/*-----------------------------------------------------------------------*/


  void
on_rdpattern_rotate_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  (void)spinbutton;
  (void)user_data;

  if( rdpattern_view == NULL )
    return;

  view_set_angles( rdpattern_view,
      gtk_spin_button_get_value( rotate_rdpattern ),
      gtk_spin_button_get_value( incline_rdpattern ) );

} /* on_rdpattern_rotate_spinbutton_value_changed() */


  void
on_rdpattern_incline_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  (void)spinbutton;
  (void)user_data;

  if( rdpattern_view == NULL )
    return;

  view_set_angles( rdpattern_view,
      gtk_spin_button_get_value( rotate_rdpattern ),
      gtk_spin_button_get_value( incline_rdpattern ) );

} /* on_rdpattern_incline_spinbutton_value_changed() */


  gboolean
on_rdpattern_colorcode_drawingarea_draw(
    GtkWidget       *widget,
    cairo_t         *cr,
    gpointer         user_data)
{
  Draw_Colorcode( cr );
  return( TRUE );
}




  gboolean
on_rdpattern_drawingarea_configure_event(
    GtkWidget       *widget,
    GdkEventConfigure *event,
    gpointer         user_data)
{
  if( rdpattern_view != NULL )
    view_set_viewport( rdpattern_view, event->width, event->height );

  return( TRUE );
}


/** on_rdpattern_drawingarea_draw() - Render the radiation-pattern view with active theme colors
 * @widget: radiation-pattern drawing area, which owns the surface it presents
 * @cr: Cairo context for the current frame
 * @user_data: unused callback data
 */
  gboolean
on_rdpattern_drawingarea_draw(
    GtkWidget       *widget,
    cairo_t         *cr,
    gpointer         user_data)
{
  (void)user_data;

  return render_cairo( cairo_surface_of_widget(widget), cr );
}


  gboolean
on_rdpattern_drawingarea_motion_notify_event(
    GtkWidget       *widget,
    GdkEventMotion  *event,
    gpointer         user_data)
{
  static int cnt = 0;

  /* Use only 1 in MOTION_EVENTS_COUNT event */
  if( (cnt++ < MOTION_EVENTS_COUNT) ||
      isFlagSet(BLOCK_MOTION_EV) )
    return( FALSE );

  cnt = 0;

  /* Handle motion events */
  if( rdpattern_view != NULL )
    Motion_Event( event, rdpattern_view );

  return( TRUE );
}


  void
on_quit_cancelbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Gtk_Widget_Destroy( &quit_dialog );
  ClearFlag( MAIN_QUIT );
  kill_window = NULL;
}


  void
on_quit_okbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Optimizer running: defer the quit through xnec2c_request_quit so the worker
   * settles on the live main loop; its evaluation pumps the loop re-entrantly,
   * so the Stop_Frequency_Loop branch below must not run. */
  if( opt_is_running() )
  {
    Gtk_Widget_Destroy( &quit_dialog );
    xnec2c_request_quit();
    return;
  }

  if(freq_sweep_active())
  {
    if( isFlagSet(MAIN_QUIT) )
    {
      Stop_Frequency_Loop();
      gtk_label_set_text( GTK_LABEL(
            Builder_Get_Object(quit_dialog_builder, "quit_label")),
          _("Are you sure you wish to quit xnec2c?") );
      ClearFlag( MAIN_QUIT );
      return;
    }

    /* Stop freq loop if only one of plots
     * or radiation pattern windows is open */
    if( (isFlagSet(DRAW_ENABLED) && isFlagClear(PLOT_ENABLED)) ||
        (isFlagClear(DRAW_ENABLED) && isFlagSet(PLOT_ENABLED)) )
      Stop_Frequency_Loop();

  } /* if( freq_sweep_active() ) */

  Gtk_Widget_Destroy( &quit_dialog );
  Gtk_Widget_Destroy( &kill_window );
}


  gboolean
on_rdpattern_window_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  if( event->state & GDK_CONTROL_MASK )
  {
    switch( event->keyval )
    {
      case GDK_KEY_r:
        gtk_widget_grab_focus( GTK_WIDGET(rotate_rdpattern) );
        return( TRUE );

      case GDK_KEY_i:
        gtk_widget_grab_focus( GTK_WIDGET(incline_rdpattern) );
        return( TRUE );

      case GDK_KEY_z:
        gtk_widget_grab_focus( GTK_WIDGET(rdpattern_zoom) );
        return( TRUE );

      case GDK_KEY_f:
        gtk_widget_grab_focus( GTK_WIDGET(rdpattern_frequency) );
        return( TRUE );
    }
  }

  if( event->keyval == GDK_KEY_Home )
  {
    Fit_View( rdpattern_view, G_CALLBACK(on_rdpattern_zoom_spinbutton_value_changed) );
    return( TRUE );
  }

  if( Pan_View_On_Arrow_Key( rdpattern_view, event ) )
    return( TRUE );

  return( FALSE );
}


/* Animation panel controls mirror visualization fields owned by the main and
 * rdpattern windows.  Each control is meaningful only while its owning window
 * is open; anim_panel_sensitivity greys those whose owner is closed.  The
 * control values themselves are config_widget elements synced from their
 * fields, so no value mirroring lives here. */
static const struct
{
  const char  *panel_id;
  GtkBuilder **owner_builder;
} anim_panel_owners[] =
{
  { "anim_display_frame", &main_window_builder },
  { "anim_efield",   &rdpattern_window_builder },
  { "anim_hfield",   &rdpattern_window_builder },
  { "anim_poynting", &rdpattern_window_builder },
};

/** anim_panel_sensitivity() - Grey animation panel controls by owner state
 *
 * Greys each panel control whose owning window is closed, greys the
 * flow-direction combo when the main window is closed or the model
 * carries no surface patches, and greys the structure frame when the
 * main window is closed.
 */
  static void
anim_panel_sensitivity(void)
{
  GtkWidget *widget;
  gboolean has_patches;
  size_t i;

  if( animate_dialog == NULL )
    return;

  for( i = 0; i < G_N_ELEMENTS(anim_panel_owners); i++ )
  {
    widget = Builder_Get_Object( animate_dialog_builder,
        anim_panel_owners[i].panel_id );
    gtk_widget_set_sensitive( widget,
        *anim_panel_owners[i].owner_builder != NULL );
  }

  /* Flow direction styles patch arrows only; wire color animation and the
   * color projection/scale combos stay usable for wire-only models. */
  widget = Builder_Get_Object( animate_dialog_builder, "anim_flow_dir" );
  has_patches = (data.m > 0);
  gtk_widget_set_sensitive( widget,
      (main_window_builder != NULL) && has_patches );
  gtk_widget_set_tooltip_text( widget,
      has_patches
      ? _("Select how the animated patch current flow is rendered.\n"
          "Mirrors the Visualization menu setting in the main window.")
      : _("Patch flow animation requires surface patches"
          " (SP/SM cards) in the model.") );
}

/* The near-field static-baseline menu items in the rdpattern window choose the
 * peak or instantaneous vector drawn while the animation window is closed.
 * While it is open the phase slider drives the vectors, so the selection has no
 * effect; grey the items and explain why.  Derives from animate_dialog, synced
 * at the dialog open and destroy edges. */
  static void
nf_static_menu_sync(void)
{
  static const char *const item_id[] = { "near_peak_value", "near_snapshot" };
  gboolean open = (animate_dialog != NULL);
  const char *reason = open
      ? _("The animation window drives the near-field vectors from the phase"
          " slider; close it to choose the static peak or instantaneous"
          " baseline.")
      : NULL;
  size_t i;

  if( rdpattern_window_builder == NULL )
    return;

  for( i = 0; i < G_N_ELEMENTS(item_id); i++ )
  {
    GtkWidget *w = GTK_WIDGET(
        Builder_Get_Object(rdpattern_window_builder, item_id[i]) );

    gtk_widget_set_sensitive( w, !open );
    gtk_widget_set_tooltip_text( w, reason );
  }
}

/* Wrap value into the half-open phase span [lower, lower+span). */
  static gdouble
phase_wrap(gdouble value, gdouble lower, gdouble span)
{
  value = fmod( value - lower, span );
  if( value < 0.0 )
    value += span;
  return( value + lower );
}

/** on_animate_phase_slider_change_value() - Keyboard wrap-aware phase scrub
 * @range: emitting GtkScale
 * @scroll: scroll action type (arrow key, page, etc.)
 * @value: proposed new value before GTK clamps to adjustment bounds
 * @user_data: unused
 *
 * Handles keyboard-driven phase changes (arrow keys, page up/down) with
 * fmod wrapping.  Mouse drag (GTK_SCROLL_JUMP) is handled by
 * on_phase_slider_motion_notify instead, because GTK3 clamps the proposed
 * value at the lower bound for leftward drag but not at the upper bound
 * for rightward drag.
 */
  gboolean
on_animate_phase_slider_change_value(GtkRange *range, GtkScrollType scroll,
                                     gdouble value, gpointer user_data)
{
  (void)user_data;

  /* Mouse drag handled by motion-notify for symmetric wrap */
  if( scroll == GTK_SCROLL_JUMP )
    return( TRUE );

  GtkAdjustment *adj = gtk_range_get_adjustment( range );
  gdouble lower = gtk_adjustment_get_lower( adj );
  gdouble upper = gtk_adjustment_get_upper( adj );
  gdouble span = upper - lower;

  if( span <= 0.0 )
    return( TRUE );

  value = phase_wrap( value, lower, span );
  gtk_range_set_value( range, value );

  flow_phase = (float)( value * TORAD );
  animation_set_scrubbed();
  apply_animation_phase();

  return( TRUE );
}

/** on_phase_slider_motion_notify() - Pixel-delta drag with symmetric wrap
 * @widget: the phase GtkScale
 * @event: motion event with raw pixel coordinates
 * @user_data: unused
 *
 * Tracks consecutive mouse positions during button-1 drag and converts
 * pixel deltas to degree deltas, applying fmod wrapping.  Operates
 * identically in both directions because it reads raw event coordinates
 * rather than GTK's clamped proposed values.
 */
  static gboolean
on_phase_slider_motion_notify(GtkWidget *widget, GdkEventMotion *event,
                              gpointer user_data)
{
  static gdouble prev_x = NAN;

  (void)user_data;

  if( !(event->state & GDK_BUTTON1_MASK) )
  {
    prev_x = NAN;
    return( FALSE );
  }

  GtkRange *range = GTK_RANGE(widget);
  GtkAdjustment *adj = gtk_range_get_adjustment( range );
  gdouble lower = gtk_adjustment_get_lower( adj );
  gdouble upper = gtk_adjustment_get_upper( adj );
  gdouble span = upper - lower;

  GdkRectangle rect;
  gtk_range_get_range_rect( range, &rect );

  if( rect.width <= 0 || span <= 0.0 )
    return( FALSE );

  /* First motion of a new drag: record baseline, no movement */
  if( isnan(prev_x) )
  {
    prev_x = event->x;
    return( FALSE );
  }

  gdouble delta_px = event->x - prev_x;
  prev_x = event->x;

  gdouble degrees_per_px = span / (gdouble)rect.width;
  gdouble delta_deg = delta_px * degrees_per_px;

  gdouble current = gtk_range_get_value( range );
  gdouble new_val = current + delta_deg;

  new_val = phase_wrap( new_val, lower, span );

  SIGNAL_BLOCK( range, on_animate_phase_slider_change_value );
  gtk_range_set_value( range, new_val );
  SIGNAL_UNBLOCK( range, on_animate_phase_slider_change_value );

  flow_phase = (float)( new_val * TORAD );
  animation_set_scrubbed();
  apply_animation_phase();

  return( FALSE );
}

/* Snap window around a slider mark as a fraction of the slider span */
#define FAM_MARK_SNAP_FRACTION 0.025

/** on_color_fam_format_value() - Render a slider value as the natural parameter
 * @scale: emitting family slider
 * @value: slider-domain value
 * @user_data: the slider's color_tone_row_t
 *
 * Returns a heap string in the family's natural-parameter format; GTK
 * frees it after display.
 */
  static gchar *
on_color_fam_format_value(GtkScale *scale, gdouble value, gpointer user_data)
{
  const color_tone_row_t *row = user_data;

  (void)scale;

  return g_strdup_printf(row->value_fmt, row->param_map(value));
}

/** on_color_fam_change_value() - Snap a family slider onto its marks
 * @range: emitting family slider
 * @scroll: scroll type, unused
 * @value: proposed slider-domain value
 * @user_data: the slider's color_tone_row_t
 *
 * Snaps within FAM_MARK_SNAP_FRACTION of the slider span onto the row's
 * mark; the set_value call emits the final value-changed.
 */
  static gboolean
on_color_fam_change_value(GtkRange *range, GtkScrollType scroll,
    gdouble value, gpointer user_data)
{
  const color_tone_row_t *row = user_data;
  GtkAdjustment *adj = gtk_range_get_adjustment(range);
  double span = gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj);
  gboolean snapped = FALSE;
  int i;

  (void)scroll;

  for( i = 0; !snapped && !isnan(row->marks[i]); i++ )
    if( fabs(value - row->marks[i]) <= span * FAM_MARK_SNAP_FRACTION )
    {
      gtk_range_set_value(range, row->marks[i]);
      snapped = TRUE;
    }

  return snapped;
}

/** anim_fam_marks_attach() - Mark and wire the family sliders at creation
 *
 * Adds each family's snap marks labeled with the natural parameter and
 * connects the shared snap and value-format handlers with the row as
 * user data.
 */
  static void
anim_fam_marks_attach(void)
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

/** on_anim_projsel_select() - Preview a hovered projection menu row
 * @menuitem: hovered radio item, unused
 * @user_data: the row's chroma_proj_t value
 *
 * Renders the hovered projection at once without committing it; leaving
 * the menu without activating reverts via on_anim_projsel_menu_hide.
 */
  static void
on_anim_projsel_select(GtkMenuItem *menuitem, gpointer user_data)
{
  (void)menuitem;

  chroma_proj_preview_set( GPOINTER_TO_INT(user_data) );
  hook_color_vis();
}

/** on_anim_projsel_menu_hide() - Revert an uncommitted projection preview
 * @menu: collapsing projection menu, unused
 * @user_data: unused
 *
 * A click commits through the config machinery while the preview equals
 * the clicked value, so the clear resolves to the committed state; a
 * collapse on a mere hover restores the prior selection.
 */
  static void
on_anim_projsel_menu_hide(GtkWidget *menu, gpointer user_data)
{
  (void)menu;
  (void)user_data;

  if( !chroma_proj_preview_active() )
    return;

  chroma_proj_preview_clear();
  hook_color_vis();
}

/** on_color_famsel_select() - Preview a hovered scale-family menu row
 * @menuitem: hovered radio item, unused
 * @user_data: the row's color_tone_t value
 *
 * Renders the hovered family at once, including its slider row and
 * formula; leaving the menu without activating reverts on menu hide.
 */
  static void
on_color_famsel_select(GtkMenuItem *menuitem, gpointer user_data)
{
  (void)menuitem;

  color_tone_preview_set( GPOINTER_TO_INT(user_data) );
  hook_color_family();
}

/** on_color_famsel_menu_hide() - Revert an uncommitted family preview
 * @menu: collapsing family menu, unused
 * @user_data: unused
 */
  static void
on_color_famsel_menu_hide(GtkWidget *menu, gpointer user_data)
{
  (void)menu;
  (void)user_data;

  if( !color_tone_preview_active() )
    return;

  color_tone_preview_clear();
  hook_color_family();
}

/** anim_select_preview_attach() - Wire hover preview on the dialog selectors
 *
 * Connects each projection and family radio item's select edge and each
 * menu's hide edge, with the enum value as user data.
 */
  static void
anim_select_preview_attach(void)
{
  int i;

  for( i = 0; i < CHROMA_PROJ_NUM; i++ )
    g_signal_connect( Builder_Get_Object(animate_dialog_builder,
          chroma_proj_rows[i].sel_id), "select",
                     G_CALLBACK(on_anim_projsel_select), GINT_TO_POINTER(i) );

  for( i = 0; i < COLOR_TONE_NUM; i++ )
    g_signal_connect( Builder_Get_Object(animate_dialog_builder,
          color_tones[i].sel_id), "select",
        G_CALLBACK(on_color_famsel_select), GINT_TO_POINTER(i) );

  g_signal_connect( Builder_Get_Object(animate_dialog_builder,
        "anim_color_proj_menu"), "hide",
      G_CALLBACK(on_anim_projsel_menu_hide), NULL );
  g_signal_connect( Builder_Get_Object(animate_dialog_builder,
        "anim_color_family_menu"), "hide",
      G_CALLBACK(on_color_famsel_menu_hide), NULL );
}

/** color_family_menu_attach() - Wire hover preview on the main-window family radios
 * @builder: main window builder
 *
 * Connects the Visualization menu's family radio items to the shared
 * family preview handlers and the submenu's hide edge to the revert.
 */
  void
color_family_menu_attach(GtkBuilder *builder)
{
  int i;

  for( i = 0; i < COLOR_TONE_NUM; i++ )
    g_signal_connect( Builder_Get_Object(builder, color_tones[i].main_id),
        "select", G_CALLBACK(on_color_famsel_select), GINT_TO_POINTER(i) );

  g_signal_connect( Builder_Get_Object(builder, "main_flow_dir_menu_menu"),
      "hide", G_CALLBACK(on_color_famsel_menu_hide), NULL );
}

/** on_anim_color_reset_clicked() - Restore the color controls' defaults
 * @button: emitting button, unused
 * @user_data: unused
 *
 * Resets the projection, scale family, brightness floor, width carrier,
 * and every family parameter to their compiled-in defaults, reruns the
 * color hooks, then syncs the bound widgets from the fields.  Overlay
 * gates keep their user state.
 */
  void
on_anim_color_reset_clicked(GtkButton *button, gpointer user_data)
{
  int fam;

  (void)button;
  (void)user_data;

  rc_config_set_default( rc_config_find_by_field(&rc_config.anim_color_proj) );
  rc_config_set_default( rc_config_find_by_field(&rc_config.color_scale) );
  rc_config_set_default( rc_config_find_by_field(&rc_config.color_lum_floor) );
  rc_config_set_default( rc_config_find_by_field(&rc_config.color_width_amp) );

  for( fam = 0; fam < COLOR_TONE_NUM; fam++ )
  {
    /* Only slider-backed families persist a parameter row */
    if( color_tones[fam].scale_id == NULL )
      continue;

    rc_config_set_default(
        rc_config_find_by_field(&rc_config.color_fam_param[fam]) );
  }

  /* Reset sequence: defaults above, hooks, then widget sync */
  hook_color_family();
  config_widget_sync_all();
}

/* Create the animation dialog on first use, wire the phase slider drag
 * handler, then show it and reflect the mirrored visualization state. */
  static void
show_animate_dialog(void)
{
  if( animate_dialog == NULL )
  {
    animate_dialog = create_animate_dialog( &animate_dialog_builder );

    /* Connect motion-notify for symmetric wrap-around mouse drag;
     * change-value handles keyboard only (GTK3 clamps leftward drag). */
    g_signal_connect( Builder_Get_Object(animate_dialog_builder,
          "animate_phase_slider"), "motion-notify-event",
        G_CALLBACK(on_phase_slider_motion_notify), NULL );

    anim_fam_marks_attach();
    anim_select_preview_attach();
  }
  gtk_widget_show( animate_dialog );
  config_widget_sync_builder( &animate_dialog_builder );
  config_widget_run_hooks( &animate_dialog_builder );
  anim_panel_sensitivity();
  nf_static_menu_sync();
}


  void
on_rdpattern_animate_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if(!rdpat_ehfield_active())
  {
    if( fpat.nfeh )
    {
      /* Near-field data validates; enable the EH display for animation */
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
          Builder_Get_Object(rdpattern_window_builder, "rdpattern_eh_togglebutton")),
        TRUE );
    }
    else if( !(overlay_struct_active() && (data.n > 0 || data.m > 0))
        && !Validate_Nearfield_Animation() )
      return; /* no near-field data and no structure overlay to animate */
  }

  show_animate_dialog();
}


  void
on_structure_animate_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  show_animate_dialog();
}


static guint animation_apply_timer = 0;

  static void
update_animation_parameters(void)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, fps;

  spinbutton = GTK_SPIN_BUTTON(
      Builder_Get_Object(animate_dialog_builder, "animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON(
      Builder_Get_Object(animate_dialog_builder, "animate_steps_spinbutton") );
  fps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / fps);
  flow_phase_step = (double)M_2PI * freq / fps;

  if( anim_tag > 0 )
    g_source_remove( anim_tag );
  anim_tag = 0;

  if( isFlagSet(ANIMATE) )
    anim_tag = g_timeout_add( intval, Animate_Phase, NULL );
}

  static gboolean
apply_animation_delayed(gpointer user_data)
{
  if( isFlagSet(ANIMATE) )
    update_animation_parameters();

  animation_apply_timer = 0;
  return( G_SOURCE_REMOVE );
}

  void
on_animate_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );

  /* Skip live update if no animation is active */
  if( isFlagClear(ANIMATE) )
    return;

  if( animation_apply_timer != 0 )
    g_source_remove( animation_apply_timer );

  animation_apply_timer = g_timeout_add( 500, apply_animation_delayed, NULL );
}

  gboolean
on_animate_spinbutton_focus_out_event(
    GtkWidget       *widget,
    GdkEventFocus   *event,
    gpointer         user_data)
{
  if( animation_apply_timer != 0 )
  {
    g_source_remove( animation_apply_timer );
    animation_apply_timer = 0;
  }

  if( isFlagSet(ANIMATE) )
    update_animation_parameters();

  return( FALSE );
}

/** anim_phase_slider_sync_sensitivity() - Project ANIMATE onto the slider
 *
 * The manual phase slider is interactive only while the timer animation is
 * stopped; during animation it is greyed and frozen, and the live phase is
 * shown by the decoupled readout label instead.
 */
  static void
anim_phase_slider_sync_sensitivity(void)
{
  if( animate_dialog == NULL )
    return;

  gtk_widget_set_sensitive(
      Builder_Get_Object(animate_dialog_builder, "animate_phase_slider"),
      isFlagClear(ANIMATE) );
}

  void
on_animation_applybutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Reject when nothing can animate */
  if( data.n == 0 && data.m == 0 && !(fpat.nfeh & (NEAR_EFIELD | NEAR_HFIELD)) )
  {
    Notice( GTK_BUTTONS_OK, _("Animation"),
        _("Animation requires wire currents or charges, surface patches"
          " (SP/SM NEC cards), or near-field data (NE/NH NEC cards)") );
    return;
  }

  /* Validate near-field setup only when no structure content can animate */
  if( data.n == 0 && data.m == 0 &&
      rdpat_ehfield_active() && !Validate_Nearfield_Animation() )
    return;

  SetFlag( ANIMATE );
  anim_phase_slider_sync_sensitivity();
  update_animation_parameters();

  /* Playback goes live: swap in the animated projection before the
   * first timer tick so the flip is immediate at any frame rate. */
  hook_color_vis();
}


  void
on_animation_cancelbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  ClearFlag( ANIMATE );
  reset_animation_phase();

  if( anim_tag )
    g_source_remove( anim_tag );
  anim_tag = 0;

  /* Re-enable manual scrubbing and return the slider and readout to the reset
   * phase. */
  if( animate_dialog != NULL )
  {
    GtkRange *slider = GTK_RANGE(
        Builder_Get_Object(animate_dialog_builder, "animate_phase_slider") );
    SIGNAL_BLOCK( slider, on_animate_phase_slider_change_value );
    gtk_range_set_value( slider, 0.0 );
    SIGNAL_UNBLOCK( slider, on_animate_phase_slider_change_value );
  }
  anim_phase_slider_sync_sensitivity();
  apply_animation_phase();

  /* Playback ended: return to the static projection everywhere,
   * including the legend strip. */
  hook_color_vis();
}


  void
on_animation_okbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  gtk_widget_destroy( animate_dialog );
}


/*-----------------------------------------------------------------------*/

  void
on_animate_dialog_destroy(
    GObject       *object,
    gpointer       user_data)
{
  /* Stop all animations when dialog closes */
  ClearFlag( ANIMATE );
  reset_animation_phase();

  if( anim_tag )
    g_source_remove( anim_tag );
  anim_tag = 0;

  animate_dialog = NULL;
  g_object_unref( animate_dialog_builder );
  animate_dialog_builder = NULL;

  /* Re-enable the near-field static-baseline menu now the phase no longer
   * drives the vectors. */
  nf_static_menu_sync();

  /* Playback ended with the dialog; rebake and redraw under the static
   * selection now that it is gone. */
  if( main_window_builder != NULL )
    hook_color_vis();
}


  void
on_quit_dialog_destroy(
    GObject       *object,
    gpointer       user_data)
{
  ClearFlag( MAIN_QUIT );
  quit_dialog = NULL;
  g_object_unref( quit_dialog_builder );
  quit_dialog_builder = NULL;
}


  gboolean
on_error_dialog_delete_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  return( TRUE );
}


  void
on_error_dialog_destroy(
    GObject       *object,
    gpointer       user_data)
{
  error_dialog = NULL;
}


  void
on_nec2_edit_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  /* Abort if no open input file */
  if( input_fp == NULL )
  {
    Stop( ERR_OK, _("No open NEC2 input file") );
    return;
  }

  if( nec2_edit_window == NULL )
    Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
  selected_treeview = cmnt_treeview;
}


  gboolean
on_nec2_editor_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  if( (event->keyval == GDK_KEY_q) &&
      (event->state & GDK_CONTROL_MASK) )
  {
    kill_window = main_window;
    /* Boundary quit intent: MAIN_QUIT is read upstream by Nec2_Edit_Save and the
     * confirm-dialog re-prompt; xnec2c_request_quit sets it again unconditionally. */
    SetFlag( MAIN_QUIT );

    /* Prompt user to save NEC2 data */
    if( Nec2_Edit_Save() ) return( TRUE );

    /* Save GUI state for restoring windows */
    Get_GUI_State();
    Save_Config();

    /* Quit without confirmation dialog */
    if( !rc_config.confirm_quit )
    {
      xnec2c_request_quit();
      return( TRUE );
    }

    Delete_Event( _("Are you sure you wish to quit xnec2c?") );
    return( TRUE );
  }

  return( FALSE );
}


  gboolean
on_nec2_editor_delete_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  kill_window = nec2_edit_window;

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return( TRUE );

  /* Close freq plots window without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
    Gtk_Widget_Destroy( &nec2_edit_window );
    return( TRUE );
  }

  Delete_Event( _("Are you sure you wish to close this window?") );
  return( TRUE );
}


  void
on_nec2_editor_destroy(
    GObject       *object,
    gpointer       user_data)
{
  rc_config.nec2_edit_width  = 0;
  rc_config.nec2_edit_height = 0;
  cmnt_store = NULL;
  geom_store = NULL;
  cmnd_store = NULL;
  nec2_edit_window = NULL;
  g_object_unref( nec2_editor_builder );
  nec2_editor_builder = NULL;
  kill_window = NULL;
}


  void
on_nec2_save_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  gboolean new = FALSE;

  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
        _("NEC2 Editor's data may not be saved\n"
          "while the Frequency Loop is running")) )
    return;

  /* Open file selector to specify file  */
  /* name for saving a new NEC2 input file */
  if( strlen(rc_config.input_file) == 0 )
  {
    /* Open file chooser to save NEC2 input file */
    SetFlag( NEC2_SAVE );
    ClearFlag( OPEN_NEW_NEC2 );
    file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
        "*.nec", NULL, _("untitled.nec"), rc_config.working_dir );
    return;
  }

  /* Save NEC2 editor data */
  Save_Nec2_Input_File( nec2_edit_window, rc_config.input_file );
  if( Nec2_Apply_Checkbutton() )
    Open_Input_File( (gpointer)(&new) );
}


  void
on_nec2_save_as_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
        _("NEC2 Editor's data may not be saved\n"
          "while the Frequency Loop is running")) )
    return;

  /* Open file chooser to save NEC2 input file */
  SetFlag( NEC2_SAVE );
  ClearFlag( OPEN_NEW_NEC2 );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
      "*.nec", NULL, _("untitled.nec"), rc_config.working_dir );
}


  void
on_nec2_save_dialog_response(
    GtkDialog       *dialog,
    gint             response_id,
    gpointer         user_data)
{
  Gtk_Widget_Destroy( &nec2_save_dialog );

  /* Discard edited data */
  if( response_id == GTK_RESPONSE_NO )
  {
    /* Cancel NEC2 data save */
    ClearFlag( NEC2_EDIT_SAVE );
    ClearFlag( NEC2_SAVE );

    /* Open file chooser if user requested an input file to be opened */
    if( isFlagSet(OPEN_INPUT) )
    {
      /* Open file chooser to select a NEC2 input file */
      file_chooser = Open_Filechooser(
          GTK_FILE_CHOOSER_ACTION_OPEN, "*.nec", NULL, NULL, rc_config.working_dir );
      Gtk_Widget_Destroy( &nec2_save_dialog );
      return;
    }

    /* Open a new NEC2 project */
    if( isFlagSet(OPEN_NEW_NEC2) )
    {
      /* Open editor window if needed */
      if( nec2_edit_window == NULL )
      {
        Close_File( &input_fp );
        Open_Nec2_Editor( NEC2_EDITOR_NEW );
      }
      else Nec2_Input_File_Treeview( NEC2_EDITOR_NEW );

      rc_config.input_file[0] = '\0';
      selected_treeview = cmnt_treeview;
      ClearFlag( OPEN_NEW_NEC2 );
    }
  } /* if( response_id == GTK_RESPONSE_NO ) */
  else if( response_id == GTK_RESPONSE_YES )
  {
    /* Open file chooser to specify file name to save
     * NEC2 editor data to, if no file is already open */
    SetFlag( NEC2_SAVE );
    if( strlen(rc_config.input_file) == 0 )
    {
      file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
          "*.nec", NULL, "untitled.nec", rc_config.working_dir );
      return;
    }
    else /* Save to already open input file */
      Save_Nec2_Input_File( nec2_edit_window, rc_config.input_file );

    /* Re-open NEC2 input file */
    gboolean new = FALSE;
    if( Nec2_Apply_Checkbutton() && isFlagClear(MAIN_QUIT) )
      Open_Input_File( (gpointer)(&new) );

    /* Open file chooser if user requested an input file to be opened */
    if( isFlagSet(OPEN_INPUT) )
    {
      file_chooser = Open_Filechooser(
          GTK_FILE_CHOOSER_ACTION_OPEN, "*.nec", NULL, NULL, rc_config.working_dir );
      return;
    }

    /* Open a new NEC2 project */
    if( isFlagSet(OPEN_NEW_NEC2) )
    {
      /* Open editor window if needed */
      if( nec2_edit_window == NULL )
      {
        Close_File( &input_fp );
        Open_Nec2_Editor( NEC2_EDITOR_NEW );
      }
      else Nec2_Input_File_Treeview( NEC2_EDITOR_NEW );

      rc_config.input_file[0] = '\0';
      selected_treeview = cmnt_treeview;
    }
  } /* if( response_id == GTK_RESPONSE_YES ) */

  /* Save GUI state data for restoring
   * windows if user is quitting xnec2c */
  if( isFlagSet(MAIN_QUIT) )
  {
    Get_GUI_State();
    Save_Config();
  }

  /* Kill window that initiated the save dialog.
   * If it was the main window, xnec2c will exit */
  Gtk_Widget_Destroy( &kill_window );
}


  void
on_nec2_row_add_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter, sibling;
  int ncols;

  if( selected_treeview == NULL )
    return;

  /* Find selected row and add new after */
  selection = gtk_tree_view_get_selection( selected_treeview );
  if( !gtk_tree_selection_get_selected(selection, &model, &sibling) )
  {
    /* Empty tree view case */
    model = gtk_tree_view_get_model( selected_treeview );
    gtk_list_store_insert( GTK_LIST_STORE(model), &iter, 0 );
  }
  else gtk_list_store_insert_after(
      GTK_LIST_STORE(model), &iter, &sibling);
  gtk_tree_selection_select_iter( selection, &iter );

  /* Prime columns of new row */
  ncols = gtk_tree_model_get_n_columns( model );
  if( ncols == CMNT_NUM_COLS ) /* Comments treeview */
  {
    gtk_list_store_set( GTK_LIST_STORE(model), &iter, CMNT_COL_NAME, "CM", -1 );
    Zero_Store(GTK_LIST_STORE(model), &iter, ncols, CMNT_COL_COMMENT, CMNT_COL_COMMENT);
  }
  else if( ncols == GEOM_NUM_COLS )
  {
    Zero_Store(GTK_LIST_STORE(model), &iter, ncols, GEOM_COL_NAME, -1);
  }
  else if( ncols == CMND_NUM_COLS )
  {
    Zero_Store(GTK_LIST_STORE(model), &iter, ncols, CMND_COL_NAME, -1);
  }

  SetFlag( NEC2_EDIT_SAVE );
}


  void
on_nec2_row_remv_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkTreeSelection *selection;

  if( selected_treeview == NULL )
    return;

  selection = gtk_tree_view_get_selection( selected_treeview );
  gtk_tree_selection_get_selected( selection, &model, &iter);
  SetFlag( EDITOR_QUIT );
  SetFlag( NEC2_EDIT_SAVE );
  Open_Editor( selected_treeview );
  gtk_list_store_remove( GTK_LIST_STORE(model), &iter );
  selected_treeview = NULL;
}


  void
on_nec2_treeview_clear_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  if( selected_treeview != NULL )
  {
    SetFlag( EDITOR_QUIT );
    Open_Editor( selected_treeview );
    gtk_list_store_clear( GTK_LIST_STORE(
          gtk_tree_view_get_model(selected_treeview)) );
  }
}


  gboolean
on_nec2_cmnt_treeview_button_press_event(
    GtkWidget       *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW( widget );
  return( FALSE );
}


  gboolean
on_nec2_geom_treeview_button_press_event(
    GtkWidget       *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW( widget );
  if( event->button == 3 )
  {
    editor_action = EDITOR_EDIT;
    Open_Editor( selected_treeview );
    editor_action = EDITOR_NEW;
    return( TRUE );
  }
  return( FALSE );
}


  gboolean
on_nec2_geom_treeview_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  selected_treeview = GTK_TREE_VIEW( widget );
  if( ( (event->keyval == GDK_KEY_Return) ||
        (event->keyval == GDK_KEY_KP_Enter) ) &&
      (event->state & GDK_CONTROL_MASK) )
  {
    editor_action = EDITOR_EDIT;
    Open_Editor( selected_treeview );
    editor_action = EDITOR_NEW;
    return( TRUE );
  }
  return( FALSE );
}


  gboolean
on_nec2_cmnd_treeview_button_press_event(
    GtkWidget       *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW(widget);
  if( event->button == 3 )
  {
    editor_action = EDITOR_EDIT;
    Open_Editor( selected_treeview );
    editor_action = EDITOR_NEW;
    return( TRUE );
  }
  return( FALSE );
}


  gboolean
on_nec2_cmnd_treeview_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  selected_treeview = GTK_TREE_VIEW(widget);
  if( ( (event->keyval == GDK_KEY_Return) ||
        (event->keyval == GDK_KEY_KP_Enter) ) &&
      (event->state & GDK_CONTROL_MASK) )
  {
    editor_action = EDITOR_EDIT;
    Open_Editor( selected_treeview );
    editor_action = EDITOR_NEW;
    return( TRUE );
  }
  return( FALSE );
}


  void
on_nec2_revert_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Open NEC2 input file */
  if( strlen(rc_config.input_file) == 0 ) return;
  Open_File( &input_fp, rc_config.input_file, "r" );
  Nec2_Input_File_Treeview( NEC2_EDITOR_REVERT );
}


  void
on_nec2_save_dialog_destroy(
    GtkDialog       *dialog,
    gpointer         user_data)
{
  nec2_save_dialog = NULL;
  g_object_unref( nec2_save_dialog_builder );
  nec2_save_dialog_builder = NULL;
}


void on_error_stopbutton_clicked(
    GtkButton *button, gpointer user_data) __attribute__((noreturn));
  void
on_error_stopbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  ClearFlag( ERROR_CONDX );
  Gtk_Widget_Destroy( &error_dialog );
  gtk_main();
  exit(0);
}


  void
on_error_okbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  ClearFlag( ERROR_CONDX );
  Gtk_Widget_Destroy( &error_dialog );
}


  void
on_error_quitbutton_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Gtk_Widget_Destroy( &error_dialog );
  xnec2c_request_quit();
}


  void
on_wire_editor_destroy(
    GObject       *object,
    gpointer         user_data)
{
  wire_editor = NULL;
  g_object_unref( wire_editor_builder );
  wire_editor_builder = NULL;
}


  void
on_wire_pcl_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_SEGPC );
}


  void
on_wire_data_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_DATA );
}


  void
on_wire_tagnum_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_TAGNUM );
}


  void
on_wire_len_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_WLEN );
}


  void
on_wire_taper_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Wire_Editor( WIRE_EDITOR_TAPR );
}


  void
on_wire_rlen_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_RLEN );
}


  void
on_wire_rdia_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_RDIA );
}


  void
on_wire_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Wire_Editor( EDITOR_NEW );
}


  void
on_wire_res_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_LOAD );
}


  void
on_wire_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Wire_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &wire_editor );
}


  void
on_wire_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Wire_Editor( EDITOR_APPLY );
}


  void
on_wire_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Wire_Editor( EDITOR_OK );
  Gtk_Widget_Destroy( &wire_editor );
}


  void
on_gw_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Abort if FR cards have not been processed */
  if( calc_data.FR_cards < 1 )
  {
    Stop(ERR_OK,
        "Frequency (FR) cards not yet processed.\n"
        "You may need to save the NEC2 Editor data first.");
    return;
  }

  Card_Clicked(
      &wire_editor, &wire_editor_builder,
      create_wire_editor, Wire_Editor, &editor_action );
}


  void
on_ga_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Abort if FR cards have not been processed */
  if( calc_data.FR_cards < 1 )
  {
    Stop(ERR_OK,
        "Frequency (FR) cards not yet processed.\n"
        "You may need to save the NEC2 Editor data first.");
    return;
  }

  Card_Clicked(
      &arc_editor, &arc_editor_builder,
      create_arc_editor, Arc_Editor, &editor_action );
}


  void
on_gh_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  /* Abort if FR cards have not been processed */
  if( calc_data.FR_cards < 1 )
  {
    Stop(ERR_OK,
        "Frequency (FR) cards not yet processed.\n"
        "You may need to save the NEC2 Editor data first.");
    return;
  }

  Card_Clicked(
      &helix_editor, &helix_editor_builder,
      create_helix_editor, Helix_Editor, &editor_action );
}


  void
on_sp_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &patch_editor, &patch_editor_builder,
      create_patch_editor, Patch_Editor, &editor_action );
}


  void
on_gr_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &cylinder_editor, &cylinder_editor_builder,
      create_cylinder_editor, Cylinder_Editor, &editor_action );
}


  void
on_gm_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &transform_editor, &transform_editor_builder,
      create_transform_editor, Transform_Editor, &editor_action );
}


  void
on_gx_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &reflect_editor, &reflect_editor_builder,
      create_reflect_editor, Reflect_Editor, &editor_action );
}


  void
on_gs_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &scale_editor, &scale_editor_builder,
      create_scale_editor, Scale_Editor, &editor_action );
}


  void
on_ex_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &excitation_command, &excitation_editor_builder,
      create_excitation_command, Excitation_Command, &editor_action );
}


  void
on_fr_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &frequency_command, &frequency_editor_builder,
      create_frequency_command, Frequency_Command, &editor_action );
}


  void
on_gn_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &ground_command, &ground_editor_builder,
      create_ground_command, Ground_Command, &editor_action );
}


  void
on_gd_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &ground2_command, &ground2_editor_builder,
      create_ground2_command, Ground2_Command, &editor_action );
}


  void
on_rp_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &radiation_command, &radiation_editor_builder,
      create_radiation_command, Radiation_Command, &editor_action );
}


  void
on_ld_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &loading_command, &loading_editor_builder,
      create_loading_command, Loading_Command, &editor_action );
}


  void
on_nt_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &network_command, &network_editor_builder,
      create_network_command, Network_Command, &editor_action );
}


  void
on_tl_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &txline_command, &txline_editor_builder,
      create_txline_command, Txline_Command, &editor_action );
}


  void
on_ne_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &nearfield_command, &nearfield_editor_builder,
      create_nearfield_command, Nearfield_Command, &editor_action );
}


  void
on_ek_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &kernel_command, &kernel_editor_builder,
      create_kernel_command, Kernel_Command, &editor_action );
}


  void
on_kh_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &intrange_command, &intrange_editor_builder,
      create_intrange_command, Intrange_Command, &editor_action );
}


  void
on_zo_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &zo_command, &zo_editor_builder,
      create_zo_command, Zo_Command, &editor_action );
}


  void
on_xq_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Card_Clicked(
      &execute_command, &execute_editor_builder,
      create_execute_command, Execute_Command, &editor_action );
}


/* on_sy_geom_clicked()
 *
 * Adds a new SY card in the geometry section.
 */

  void
on_sy_geom_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  selected_treeview = geom_treeview;
  Sy_Card_Editor( EDITOR_NEW );
}


/* on_sy_cmnd_clicked()
 *
 * Adds a new SY card in the command section.
 */

  void
on_sy_cmnd_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  selected_treeview = cmnd_treeview;
  Sy_Card_Editor( EDITOR_NEW );
}


  void
on_patch_data_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Patch_Editor( EDITOR_DATA );
}


  void
on_patch_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Patch_Editor( EDITOR_NEW );
}


  void
on_patch_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Patch_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &patch_editor );
}


  void
on_patch_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Patch_Editor( EDITOR_APPLY );
}


  void
on_patch_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Patch_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &patch_editor );
}


  void
on_patch_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  patch_editor = NULL;
  g_object_unref( patch_editor_builder );
  patch_editor_builder = NULL;
}


  void
on_patch_arbitrary_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Patch_Editor( PATCH_EDITOR_ARBT );
  else
    Patch_Editor( PATCH_EDITOR_SCCD );
}


  void
on_patch_rectangular_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Patch_Editor( PATCH_EDITOR_RECT );
}


  void
on_patch_triangular_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Patch_Editor( PATCH_EDITOR_TRIA );
}


  void
on_patch_quadrilateral_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Patch_Editor( PATCH_EDITOR_QUAD );
}


  void
on_patch_surface_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Patch_Editor( PATCH_EDITOR_SURF );
}


  void
on_arc_data_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_DATA );
}


  void
on_arc_tagnum_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_TAGNUM );
}


  void
on_arc_res_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_LOAD );
}


  void
on_arc_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Arc_Editor( EDITOR_NEW );
}


  void
on_arc_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Arc_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &arc_editor );
}


  void
on_arc_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Arc_Editor( EDITOR_APPLY );
}


  void
on_arc_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Arc_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &arc_editor );
}


  void
on_arc_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  arc_editor = NULL;
  g_object_unref( arc_editor_builder );
  arc_editor_builder = NULL;
}


  void
on_arc_pcl_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_SEGPC );
}


  void
on_helix_tagnum_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_TAGNUM );
}


  void
on_helix_pcl_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_SEGPC );
}


  void
on_helix_nturns_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( HELIX_EDITOR_NTURN );
}


  void
on_helix_tspace_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( HELIX_EDITOR_TSPACE );
}


  void
on_helix_res_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_LOAD );
}


  void
on_helix_data_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_DATA );
}


  void
on_helix_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Helix_Editor( EDITOR_NEW );
}


  void
on_helix_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Helix_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &helix_editor );
}


  void
on_helix_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Helix_Editor( EDITOR_APPLY );
}


  void
on_helix_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Helix_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &helix_editor );
}


  void
on_helix_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  helix_editor = NULL;
  g_object_unref( helix_editor_builder );
  helix_editor_builder = NULL;
}


  void
on_helix_right_hand_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Helix_Editor( HELIX_EDITOR_RH_HELIX );
}


  void
on_helix_left_hand_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Helix_Editor( HELIX_EDITOR_LH_HELIX );
}


  gboolean
on_helix_link_a1b1_button_press_event(
    GtkWidget      *widget,
    GdkEventButton *event,
    gpointer        user_data)
{
  Helix_Editor( HELIX_EDITOR_LINK_A1B1 );
  return( FALSE );
}


  gboolean
on_helix_link_b1a2_button_press_event(
    GtkWidget      *widget,
    GdkEventButton *event,
    gpointer        user_data)
{
  Helix_Editor( HELIX_EDITOR_LINK_B1A2 );
  return( FALSE );
}


  gboolean
on_helix_link_a2b2_button_press_event(
    GtkWidget      *widget,
    GdkEventButton *event,
    gpointer        user_data)
{
  Helix_Editor( HELIX_EDITOR_LINK_A2B2 );
  return( FALSE );
}


  void
on_spiral_right_hand_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Helix_Editor( HELIX_EDITOR_RH_SPIRAL );
}


  void
on_spiral_left_hand_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
    Helix_Editor( HELIX_EDITOR_LH_SPIRAL );
}


  void
on_reflect_taginc_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Reflect_Editor( EDITOR_DATA );
}


  void
on_reflect_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Reflect_Editor( REFLECT_EDITOR_TOGGLE );
}


  void
on_reflect_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Reflect_Editor( EDITOR_NEW );
}


  void
on_reflect_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Reflect_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &reflect_editor );
}


  void
on_reflect_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Reflect_Editor( EDITOR_APPLY );
}


  void
on_reflect_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Reflect_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &reflect_editor );
}


  void
on_reflect_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  reflect_editor = NULL;
  g_object_unref( reflect_editor_builder );
  reflect_editor_builder = NULL;
}


  void
on_scale_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  scale_editor = NULL;
  g_object_unref( scale_editor_builder );
  scale_editor_builder = NULL;
}


  void
on_scale_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Scale_Editor( EDITOR_DATA );
}



  void
on_scale_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Scale_Editor( EDITOR_NEW );
}


  void
on_scale_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Scale_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &scale_editor );
}


  void
on_scale_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Scale_Editor( EDITOR_APPLY );
}


  void
on_scale_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Scale_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &scale_editor );
}


  void
on_cylinder_taginc_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Cylinder_Editor( EDITOR_DATA );
}


  void
on_cylinder_total_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Cylinder_Editor( EDITOR_DATA );
}


  void
on_cylinder_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Cylinder_Editor( EDITOR_NEW );
}


  void
on_cylinder_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Cylinder_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &cylinder_editor );
}


  void
on_cylinder_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Cylinder_Editor( EDITOR_APPLY );
}


  void
on_cylinder_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Cylinder_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &cylinder_editor );
}


  void
on_cylinder_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  cylinder_editor = NULL;
  g_object_unref( cylinder_editor_builder );
  cylinder_editor_builder = NULL;
}


  void
on_transform_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Transform_Editor( EDITOR_DATA );
}


  void
on_transform_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Transform_Editor( EDITOR_NEW );
}


  void
on_transform_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Transform_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &transform_editor );
}


  void
on_transform_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Transform_Editor( EDITOR_APPLY );
}


  void
on_transform_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Transform_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &transform_editor );
}


  void
on_transform_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  transform_editor = NULL;
  g_object_unref( transform_editor_builder );
  transform_editor_builder = NULL;
}



  void
on_gend_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Gend_Editor( EDITOR_RDBUTTON );
}


  void
on_gend_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Gend_Editor( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &gend_editor );
}


  void
on_gend_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Gend_Editor( EDITOR_APPLY );
}


  void
on_gend_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Gend_Editor( EDITOR_APPLY );
  Gtk_Widget_Destroy( &gend_editor );
}


  void
on_gend_editor_destroy(
    GObject       *object,
    gpointer      user_data)
{
  gend_editor = NULL;
  g_object_unref( gend_editor_builder );
  gend_editor_builder = NULL;
}


  void
on_kernel_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  kernel_command = NULL;
  g_object_unref( kernel_editor_builder );
  kernel_editor_builder = NULL;
}


  void
on_kernel_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Kernel_Command( COMMAND_CKBUTTON );
}


  void
on_kernel_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Kernel_Command( EDITOR_NEW );
}


  void
on_kernel_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Kernel_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &kernel_command );
}


  void
on_kernel_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Kernel_Command( EDITOR_APPLY );
}


  void
on_kernel_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Kernel_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &kernel_command );
}


  void
on_execute_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  execute_command = NULL;
  g_object_unref( execute_editor_builder );
  execute_editor_builder = NULL;
}


  void
on_execute_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Execute_Command( COMMAND_RDBUTTON );
}


  void
on_execute_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Execute_Command( EDITOR_NEW );
}


  void
on_execute_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Execute_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &execute_command );
}


  void
on_execute_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Execute_Command( EDITOR_APPLY );
}


  void
on_execute_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Execute_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &execute_command );
}


  void
on_intrange_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  intrange_command = NULL;
  g_object_unref( intrange_editor_builder );
  intrange_editor_builder = NULL;
}


  void
on_intrange_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Intrange_Command( EDITOR_DATA );
}


  void
on_intrange_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Intrange_Command( EDITOR_NEW );
}


  void
on_intrange_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Intrange_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &intrange_command );
}


  void
on_intrange_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Intrange_Command( EDITOR_APPLY );
}


  void
on_intrange_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Intrange_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &intrange_command );
}


  void
on_zo_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  zo_command = NULL;
  g_object_unref( zo_editor_builder );
  zo_editor_builder = NULL;
}


  void
on_zo_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Zo_Command( EDITOR_DATA );
}


  void
on_zo_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Zo_Command( EDITOR_NEW );
}


  void
on_zo_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Zo_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &zo_command );
}


  void
on_zo_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Zo_Command( EDITOR_APPLY );
}


  void
on_zo_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Zo_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &zo_command );
}


  void
on_ground_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  ground_command = NULL;
  g_object_unref( ground_editor_builder );
  ground_editor_builder = NULL;
}


  void
on_ground_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Ground_Command( COMMAND_RDBUTTON );
}


  void
on_ground_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Ground_Command( COMMAND_CKBUTTON );
}


  void
on_ground_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Ground_Command( EDITOR_DATA );
}


  void
on_ground_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground_Command( EDITOR_NEW );
}


  void
on_ground_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &ground_command );
}


  void
on_ground_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground_Command( EDITOR_APPLY );
}


  void
on_ground_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &ground_command );
}


  void
on_nearfield_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  nearfield_command = NULL;
  g_object_unref( nearfield_editor_builder );
  nearfield_editor_builder = NULL;
}



  void
on_nearfield_nh_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Nearfield_Command( NEARFIELD_NH_CKBUTTON );
}


  void
on_nearfield_ne_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Nearfield_Command( NEARFIELD_NE_CKBUTTON );
}


  void
on_nearfield_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Nearfield_Command( COMMAND_RDBUTTON );
}


  void
on_nearfield_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Nearfield_Command( EDITOR_DATA );
}


  void
on_nearfield_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Nearfield_Command( EDITOR_NEW );
}


  void
on_nearfield_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Nearfield_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &nearfield_command );
}


  void
on_nearfield_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Nearfield_Command( EDITOR_APPLY );
}


  void
on_nearfield_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Nearfield_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &nearfield_command );
}


  void
on_radiation_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  radiation_command = NULL;
  g_object_unref( radiation_editor_builder );
  radiation_editor_builder = NULL;
}


  void
on_radiation_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Radiation_Command( COMMAND_RDBUTTON );
}


  void
on_radiation_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Radiation_Command( EDITOR_DATA );
}


  void
on_radiation_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Radiation_Command( EDITOR_NEW );
}


  void
on_radiation_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Radiation_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &radiation_command );
}


  void
on_radiation_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Radiation_Command( EDITOR_APPLY );
}


  void
on_radiation_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Radiation_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &radiation_command );
}


  void
on_excitation_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  excitation_command = NULL;
  g_object_unref( excitation_editor_builder );
  excitation_editor_builder = NULL;
}


  void
on_excitation_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Excitation_Command( COMMAND_RDBUTTON );
}


  void
on_excitation_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Excitation_Command( COMMAND_CKBUTTON );
}


  void
on_excitation_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Excitation_Command( EDITOR_DATA );
}


  void
on_excitation_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Excitation_Command( EDITOR_NEW );
}


  void
on_excitation_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Excitation_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &excitation_command );
}


  void
on_excitation_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Excitation_Command( EDITOR_APPLY );
}


  void
on_excitation_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Excitation_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &excitation_command );
}


  void
on_frequency_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  frequency_command = NULL;
  g_object_unref( frequency_editor_builder );
  frequency_editor_builder = NULL;
}


  void
on_frequency_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Frequency_Command( COMMAND_RDBUTTON );
}


  void
on_frequency_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Frequency_Command( EDITOR_DATA );
}


  void
on_frequency_step_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Frequency_Command( FREQUENCY_EDITOR_FSTEP );
}


  void
on_frequency_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Frequency_Command( EDITOR_NEW );
}


  void
on_frequency_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Frequency_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &frequency_command );
}


  void
on_frequency_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Frequency_Command( EDITOR_APPLY );
}


  void
on_frequency_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Frequency_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &frequency_command );
}


  void
on_loading_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  loading_command = NULL;
  g_object_unref( loading_editor_builder );
  loading_editor_builder = NULL;
}


  void
on_loading_radiobutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Loading_Command( COMMAND_RDBUTTON );
}


  void
on_loading_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Loading_Command( EDITOR_DATA );
}


  void
on_loading_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Loading_Command( EDITOR_NEW );
}


  void
on_loading_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Loading_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &loading_command );
}


  void
on_loading_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Loading_Command( EDITOR_APPLY );
}


  void
on_loading_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Loading_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &loading_command );
}


  void
on_network_command_destroy(
    GObject       *object,
    gpointer      user_data)
{
  network_command = NULL;
  g_object_unref( network_editor_builder );
  network_editor_builder = NULL;
}


  void
on_network_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Network_Command( EDITOR_DATA );
}


  void
on_network_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Network_Command( EDITOR_NEW );
}


  void
on_network_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Network_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &network_command );
}


  void
on_network_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Network_Command( EDITOR_APPLY );
}


  void
on_network_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Network_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &network_command );
}


  void
on_txline_command_destroy(
    GObject       *object,
    gpointer       user_data)
{
  txline_command = NULL;
  g_object_unref( txline_editor_builder );
  txline_editor_builder = NULL;
}


  void
on_txline_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Txline_Command( EDITOR_DATA );
}


  void
on_txline_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Txline_Command( EDITOR_NEW );
}


  void
on_txline_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Txline_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &txline_command );
}


  void
on_txline_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Txline_Command( EDITOR_APPLY );
}


  void
on_txline_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Txline_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &txline_command );
}


  void
on_txline_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Txline_Command( COMMAND_CKBUTTON );
}


  void
on_ground2_command_destroy(
    GObject       *object,
    gpointer       user_data)
{
  ground2_command = NULL;
  g_object_unref( ground2_editor_builder );
  ground2_editor_builder = NULL;
}


  void
on_ground2_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Ground2_Command( EDITOR_DATA );
}


  void
on_ground2_new_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground2_Command( EDITOR_NEW );
}


  void
on_ground2_cancel_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground2_Command( EDITOR_CANCEL );
  Gtk_Widget_Destroy( &ground2_command );
}


  void
on_ground2_apply_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground2_Command( EDITOR_APPLY );
}


  void
on_ground2_ok_button_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  Ground2_Command( EDITOR_APPLY );
  Gtk_Widget_Destroy( &ground2_command );
}


  void
on_loop_start_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  freq_loop_toggle();
}


  void
on_loop_reset_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  freq_loop_rewind();
}

static GtkWidget *aboutdialog = NULL;
  void
on_about_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  if( aboutdialog == NULL )
  {
    GtkBuilder *builder;
    aboutdialog = create_aboutdialog( &builder );
    gtk_widget_show( aboutdialog );
    gtk_about_dialog_set_program_name(
        GTK_ABOUT_DIALOG(aboutdialog), PACKAGE );
    gtk_about_dialog_set_version(
        GTK_ABOUT_DIALOG(aboutdialog), VERSION );
    g_object_unref( builder );
  }
}


  void
on_aboutdialog_close(
    GtkDialog       *dialog,
    gpointer         user_data)
{
  Gtk_Widget_Destroy( &aboutdialog );
}


  void
on_aboutdialog_destroy(
    GObject       *object,
    gpointer       user_data)
{
  aboutdialog = NULL;
}


  void
on_aboutdialog_response(
    GtkDialog       *dialog,
    gint             response_id,
    gpointer         user_data)
{
  Gtk_Widget_Destroy( &aboutdialog );
}


/* on_freqplots_theme_activate()
 *
 * Base color-theme radio selection.  Sets the active base theme name and
 * updates the Inverted item's sensitivity to match whether the chosen theme
 * carries an inverted variant, then repaints through the single orchestration
 * path. */
  void
on_freqplots_theme_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  const char *base;
  GtkWidget  *invert;

  if( !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    return;

  base = g_object_get_data( G_OBJECT(menuitem), THEME_DATA_BASE );
  if( base == NULL )
    return;

  Strlcpy( rc_config.freqplots_theme, base, sizeof(rc_config.freqplots_theme) );

  /* A committed selection supersedes any hover preview so theme_active reads
   * the persisted rc_config value rather than the transient override. */
  theme_preview_clear();

  invert = g_object_get_data( G_OBJECT(menuitem), THEME_DATA_INVERT_ITEM );
  if( invert != NULL )
    freqplots_invert_item_sync( invert, base );

  config_widget_field_changed( rc_config.freqplots_theme );
}

/* on_freqplots_theme_invert_toggled()
 *
 * Inverted-variant toggle, the orthogonal axis to the base theme; repaints
 * through the single orchestration path. */
  void
on_freqplots_theme_invert_toggled(
    GtkCheckMenuItem *menuitem,
    gpointer          user_data)
{
  rc_config.freqplots_theme_invert =
      gtk_check_menu_item_get_active(menuitem) ? 1 : 0;

  config_widget_field_changed( &rc_config.freqplots_theme_invert );
}

/* on_freqplots_theme_select()
 *
 * Menu-hover preview.  Highlighting a base-theme item paints that theme at
 * once without committing it, so the user previews before choosing.  An
 * uncommitted preview is reverted by on_freqplots_theme_menu_hide when the
 * theme list collapses. */
  void
on_freqplots_theme_select(
    GtkMenuItem     *menuitem,
    gpointer         user_data)
{
  const char *base = g_object_get_data( G_OBJECT(menuitem), THEME_DATA_BASE );

  if( base == NULL )
    return;

  theme_preview_set( base );
  hook_theme_change();
}

/* on_freqplots_theme_menu_hide()
 *
 * Color Theme submenu collapse.  A click commits through
 * on_freqplots_theme_activate, which clears the preview before the menu hides;
 * a preview still active here means the list collapsed on a mere hover, so the
 * committed selection is restored. */
  void
on_freqplots_theme_menu_hide(
    GtkWidget       *menu,
    gpointer         user_data)
{
  if( !theme_preview_active() )
    return;

  theme_preview_clear();
  hook_theme_change();
}


  gboolean
on_structure_drawingarea_button_press_event(
    GtkWidget      *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  drag_button_t button = (event->button == 1) ? VIEW_DRAG_ROTATE : VIEW_DRAG_PAN;

  if( structure_view != NULL )
    view_begin_drag( structure_view, button, (float)event->x, (float)event->y );

  return( FALSE );
}


  gboolean
on_structure_drawingarea_button_release_event(
    GtkWidget      *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  if( structure_view != NULL )
    view_end_drag( structure_view );

  return( FALSE );
}


  void
on_main_zoom_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  if( structure_view == NULL )
    return;

  /* Spin value is a percentage; view_t stores a unit-scale factor. */
  view_set_zoom( structure_view,
      (float)(gtk_spin_button_get_value( spinbutton ) / 100.0) );
}


  void
on_zoom_plus_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  GtkSpinButton *z = (window_type_from_widget(GTK_WIDGET(button)) == MAIN_WINDOW)
      ? structure_zoom : rdpattern_zoom;

  gtk_spin_button_set_value( z, gtk_spin_button_get_value( z ) * 1.1 );
}


  void
on_zoom_minus_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  GtkSpinButton *z = (window_type_from_widget(GTK_WIDGET(button)) == MAIN_WINDOW)
      ? structure_zoom : rdpattern_zoom;

  gtk_spin_button_set_value( z, gtk_spin_button_get_value( z ) / 1.1 );
}


  void
on_zoom_reset_clicked(
    GtkButton       *button,
    gpointer         user_data)
{
  view_t *target;
  GtkSpinButton *z;

  if( window_type_from_widget(GTK_WIDGET(button)) == MAIN_WINDOW )
  {
    target = structure_view;
    z = structure_zoom;
  }
  else
  {
    target = rdpattern_view;
    z = rdpattern_zoom;
  }

  if( target == NULL )
    return;

  gtk_spin_button_set_value( z, 100.0 );
  view_reset_pan( target );
}


/**
 * Fit_View() - Fit a view's active rendered content, syncing its zoom spinbutton
 * @target:       the view_t to fit (structure_view or rdpattern_view)
 * @zoom_handler: that view's zoom spinbutton value-changed callback, blocked
 *                during the fit so it doesn't re-trigger on the resulting
 *                zoom change
 *
 * Shared by on_fit_view_clicked() (mouse) and the Home-key handlers in
 * on_main_window_key_press_event()/on_rdpattern_window_key_press_event(),
 * so both input paths go through one implementation.
 */
  static void
Fit_View( view_t *target, GCallback zoom_handler )
{
  view_fit_t fit = {0};

  if( target == NULL || !canvas_fit_view(target, &fit) )
    return;

  if( target->zoom_spin != NULL )
    SIGNAL_BLOCK(target->zoom_spin, zoom_handler);

  view_apply_fit(target, &fit);

  if( target->zoom_spin != NULL )
    SIGNAL_UNBLOCK(target->zoom_spin, zoom_handler);
}


/**
 * on_fit_view_clicked() - Fit the clicked window's active rendered content
 * @button:     fit-view button identifying the target window
 * @_user_data: unused Glade callback data
 */
  void
on_fit_view_clicked(
    GtkButton       *button,
    gpointer         _user_data)
{
  view_t *target = NULL;
  GCallback zoom_handler = NULL;

  (void)_user_data;

  if( window_type_from_widget(GTK_WIDGET(button)) == MAIN_WINDOW )
  {
    target = structure_view;
    zoom_handler = G_CALLBACK(on_main_zoom_spinbutton_value_changed);
  }
  else
  {
    target = rdpattern_view;
    zoom_handler = G_CALLBACK(on_rdpattern_zoom_spinbutton_value_changed);
  }

  Fit_View( target, zoom_handler );
}


  gboolean
on_rdpattern_drawingarea_button_press_event(
    GtkWidget      *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  drag_button_t button = (event->button == 1) ? VIEW_DRAG_ROTATE : VIEW_DRAG_PAN;

  if( rdpattern_view != NULL )
    view_begin_drag( rdpattern_view, button, (float)event->x, (float)event->y );

  return( FALSE );
}


  gboolean
on_rdpattern_drawingarea_button_release_event(
    GtkWidget      *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  if( rdpattern_view != NULL )
    view_end_drag( rdpattern_view );

  return( FALSE );
}


  void
on_rdpattern_zoom_spinbutton_value_changed(
    GtkSpinButton   *spinbutton,
    gpointer         user_data)
{
  if( rdpattern_view == NULL )
    return;

  view_set_zoom( rdpattern_view,
      (float)(gtk_spin_button_get_value( spinbutton ) / 100.0) );
}




  gboolean
on_structure_drawingarea_scroll_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  int viewport_width, viewport_height;
  double zoom_pct, scale;

  viewport_width  = gtk_widget_get_allocated_width(widget);
  viewport_height = gtk_widget_get_allocated_height(widget);

  scroll_step_t ss = scroll_step_from_deltas(event);

  zoom_pct = gtk_spin_button_get_value( structure_zoom );
  scale    = compute_zoom_scale( viewport_width, viewport_height, zoom_pct );

  if( !ss.active ||
      (ss.direction != GDK_SCROLL_UP && ss.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  if( ss.direction == GDK_SCROLL_UP )
    zoom_pct *= (1.0 + 0.1 * ss.step * scale);
  else if( ss.direction == GDK_SCROLL_DOWN )
    zoom_pct /= (1.0 + 0.1 * ss.step * scale);

  gtk_spin_button_set_value( structure_zoom, zoom_pct );
  return( FALSE );
}


  gboolean
on_rdpattern_drawingarea_scroll_event(
    GtkWidget       *widget,
    GdkEvent        *event,
    gpointer         user_data)
{
  int viewport_width, viewport_height;
  double zoom_pct, scale;

  viewport_width  = gtk_widget_get_allocated_width(widget);
  viewport_height = gtk_widget_get_allocated_height(widget);

  scroll_step_t ss = scroll_step_from_deltas(event);

  if( !ss.active ||
      (ss.direction != GDK_SCROLL_UP && ss.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  /* Shift+scroll adjusts overlay structure scale; zoom is unaffected */
  if( event->scroll.state & GDK_SHIFT_MASK )
    return rdpattern_overlay_shift_scroll(ss.direction,
        viewport_width, viewport_height,
        rc_config.rdpattern_overlay_scale_adj * 100.0);

  zoom_pct = gtk_spin_button_get_value( rdpattern_zoom );
  scale    = compute_zoom_scale( viewport_width, viewport_height, zoom_pct );

  if( ss.direction == GDK_SCROLL_UP )
    zoom_pct *= (1.0 + 0.1 * ss.step * scale);
  else if( ss.direction == GDK_SCROLL_DOWN )
    zoom_pct /= (1.0 + 0.1 * ss.step * scale);

  gtk_spin_button_set_value( rdpattern_zoom, zoom_pct );
  return( FALSE );
}


  gboolean
on_escape_key_press_event(
    GtkWidget    *widget,
    GdkEventKey  *event,
    gpointer      user_data)
{
  if( event->keyval == GDK_KEY_Escape )
  {
    Gtk_Widget_Destroy( &widget );
    return( TRUE );
  }
  else return( FALSE );
}

