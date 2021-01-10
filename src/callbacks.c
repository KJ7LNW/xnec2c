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
 */

#include "callbacks.h"
#include "shared.h"

/* Action flag for NEC2 "card" editors */
static int action = EDITOR_NEW;

static int saveas_width;
static int saveas_height;

/*-----------------------------------------------------------------------*/

  void
on_main_window_destroy(
	GObject     *object,
	gpointer	user_data)
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
  SetFlag( MAIN_QUIT );

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return( TRUE );

  /* Save GUI state for restoring windows */
  Get_GUI_State();
  Save_Config();

  /* Quit without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
	Gtk_Widget_Destroy( main_window );
	return( TRUE );
  }

  Delete_Event( _("Really quit xnec2c?") );
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

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return;

  /* Open editor window if needed */
  if( nec2_edit_window == NULL )
  {
	Close_File( &input_fp );
	Open_Nec2_Editor( NEC2_EDITOR_NEW );
  }
  else Nec2_Input_File_Treeview( NEC2_EDITOR_NEW );

  rc_config.infile[0] = '\0';
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
  char saveas[FILENAME_LEN + 24];
  size_t s = sizeof( saveas );

  /* Count number of structure image files saved of geometry,
   * currents or charges, to avoid over-writing saved files */
  static int cgm = 0, ccr = 0, cch = 0;

  if( strlen(rc_config.infile) == 0 ) return;

  /* Make the structure image save file name from input file
   * name. The count of each image type saved is incremented */
  if( isFlagSet(DRAW_CURRENTS) )
	snprintf( saveas, s, "%s-%s_%03d.%s",
		rc_config.infile, "current", ++ccr, "png" );
  else if( isFlagSet(DRAW_CHARGES) )
	snprintf( saveas, s, "%s-%s_%03d.%s",
		rc_config.infile, "charge", ++cch, "png" );
  else
	snprintf( saveas, s, "%s-%s_%03d.%s",
		rc_config.infile, "geometry", ++cgm, "png" );

  saveas_drawingarea = structure_drawingarea;
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
  saveas_drawingarea = structure_drawingarea;
  saveas_width  = structure_width;
  saveas_height = structure_height;

  /* Open file chooser to save structure image */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.png", NULL, "untitled.png", rc_config.working_dir );
}

  void
on_struct_save_as_gnuplot_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Open file chooser to save structure image */
  SetFlag( STRUCT_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.gplot", NULL, "untitled.gplot", rc_config.working_dir );
}


  void
on_confirm_quit_toggled(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	rc_config.confirm_quit = 1;
  else
	rc_config.confirm_quit = 0;
}


  void
on_quit_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  kill_window = main_window;
  SetFlag( MAIN_QUIT );

  /* Prompt user to save NEC2 data */
  if( Nec2_Edit_Save() ) return;

  /* Save GUI state for restoring windows */
  Get_GUI_State();
  Save_Config();

  /* Quit without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
	Gtk_Widget_Destroy( main_window );
	return;
  }

  Delete_Event( _("Really quit xnec2c?") );
}


  void
on_main_rdpattern_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Open radiation pattern rendering window */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	GtkAllocation alloc;
	GtkWidget *widget;

	rdpattern_window = create_rdpattern_window( &rdpattern_window_builder );
	Set_Window_Geometry( rdpattern_window,
		rc_config.rdpattern_x, rc_config.rdpattern_y,
		rc_config.rdpattern_width, rc_config.rdpattern_height );
	gtk_widget_show( rdpattern_window );

	rdpattern_drawingarea = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_drawingarea" );
	gtk_widget_get_allocation( rdpattern_drawingarea, &alloc );
	rdpattern_width  = alloc.width;
	rdpattern_height = alloc.height;

	New_Projection_Parameters(
		rdpattern_width,
		rdpattern_height,
		&rdpattern_proj_params );

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

	/* Restore radiation pattern window widgets state */
	if( rc_config.rdpattern_gain_togglebutton )
	{
	  widget = Builder_Get_Object(
		  rdpattern_window_builder, "rdpattern_gain_togglebutton" );
	  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  SetFlag(DRAW_GAIN);
	}

	if( rc_config.rdpattern_eh_togglebutton )
	{
	  widget = Builder_Get_Object(
		  rdpattern_window_builder, "rdpattern_eh_togglebutton" );
	  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  SetFlag(DRAW_EHFIELD);
	}

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_e_field" );
	if( rc_config.rdpattern_e_field )
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	  SetFlag( DRAW_EFIELD );
	}
	else
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );
	  ClearFlag( DRAW_EFIELD );
	}

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_h_field" );
	if( rc_config.rdpattern_h_field )
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	  SetFlag( DRAW_HFIELD );
	}
	else
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );
	  ClearFlag( DRAW_HFIELD );
	}

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_poynting_vector" );
	if( rc_config.rdpattern_poynting_vector )
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	  SetFlag( DRAW_POYNTING );
	}
	else
	{
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );
	  ClearFlag( DRAW_POYNTING );
	}

	if( isFlagClear(INPUT_OPENED) )
	{
	  GtkWidget *box =
		Builder_Get_Object( rdpattern_window_builder, "rdpattern_box" );
	  gtk_widget_hide( box );
	}

	Main_Rdpattern_Activate( TRUE );
  } /* if( gtk_check_menu_item_get_active(...) ) */
  else if( isFlagSet(DRAW_ENABLED) )
	Gtk_Widget_Destroy( rdpattern_window );
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
	  GtkWidget *widget;

	  freqplots_window = create_freqplots_window( &freqplots_window_builder );
	  Set_Window_Geometry( freqplots_window,
		  rc_config.freqplots_x, rc_config.freqplots_y,
		  rc_config.freqplots_width, rc_config.freqplots_height );
	  gtk_widget_show( freqplots_window );
	  freqplots_drawingarea = Builder_Get_Object(
		  freqplots_window_builder, "freqplots_drawingarea" );
	  Set_Window_Labels();
	  calc_data.ngraph = 0;

	  /* Set the Zo spinbutton value */
	  GtkWidget *spin = Builder_Get_Object(
		  freqplots_window_builder, "freqplots_zo_spinbutton" );
	  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin), (gdouble)calc_data.zo );

	  GtkAllocation alloc;
	  gtk_widget_get_allocation( freqplots_drawingarea, &alloc );
	  freqplots_width  = alloc.width;
	  freqplots_height = alloc.height;

	  /* Restore frequency plots window widgets state */
	  if( rc_config.freqplots_gmax_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_gmax_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_gdir_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_gdir_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_gviewer_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_gviewer_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_vswr_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_vswr_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_zrlzim_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_zrlzim_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_zmgzph_togglebutton )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_zmgzph_togglebutton" );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
	  }

	  if( rc_config.freqplots_net_gain )
	  {
		widget = Builder_Get_Object(
			freqplots_window_builder, "freqplots_net_gain" );
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	  }

	  if( isFlagClear(INPUT_OPENED) )
	  {
		GtkWidget *box =
		  Builder_Get_Object( freqplots_window_builder, "freqplots_box" );
		gtk_widget_hide( box );
	  }
	} /* if( Main_Freqplots_Activate() */
	else gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(menuitem), FALSE );
  }
  else if( isFlagSet(PLOT_ENABLED) )
	Gtk_Widget_Destroy( freqplots_window );
}


  void
on_rdpattern_total_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Polarization( POL_TOTAL );
}


  void
on_rdpattern_horizontal_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Polarization( POL_HORIZ );
}


  void
on_rdpattern_vertical_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Polarization( POL_VERT );
}


  void
on_rdpattern_right_hand_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Polarization( POL_RHCP );
}


  void
on_rdpattern_left_hand_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Polarization( POL_LHCP );
}


  void
on_common_projection_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Enable syncing of projection params
   * for structure and rad pattern drawing */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	if( isFlagSet(DRAW_ENABLED) )
	{
	  rdpattern_proj_params.Wr = structure_proj_params.Wr;
	  rdpattern_proj_params.Wi = structure_proj_params.Wi;
	  New_Viewer_Angle(
		  rdpattern_proj_params.Wr,
		  rdpattern_proj_params.Wi,
		  rotate_rdpattern,
		  incline_rdpattern,
		  &rdpattern_proj_params );
	}
	SetFlag( COMMON_PROJECTION );
  }
  else
	ClearFlag( COMMON_PROJECTION );
}


  void
on_common_freq_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Enable syncing of frequency spinbuttons
   * between main and rad pattern windows */
  if( gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( COMMON_FREQUENCY );
  else
	ClearFlag( COMMON_FREQUENCY );
}


  void
on_main_x_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 0.0, 0.0, rotate_structure,
	  incline_structure, &structure_proj_params );
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 0.0, 0.0, rotate_rdpattern,
		incline_rdpattern, &rdpattern_proj_params );
}


  void
on_main_y_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 90.0, 0.0, rotate_structure,
	  incline_structure, &structure_proj_params );
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 90.0, 0.0, rotate_rdpattern,
		incline_rdpattern, &rdpattern_proj_params );
}


  void
on_main_z_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 0.0, 90.0, rotate_structure,
	  incline_structure, &structure_proj_params );
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 0.0, 90.0, rotate_rdpattern,
		incline_rdpattern, &rdpattern_proj_params );
}


  void
on_main_default_view_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Projection at 45 deg rotation and inclination */
  New_Viewer_Angle( 45.0, 45.0, rotate_structure,
	  incline_structure, &structure_proj_params );
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 45.0, 45.0, rotate_rdpattern,
		incline_rdpattern, &rdpattern_proj_params );
}


  void
on_main_rotate_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Get new "rotate" structure angle from spinbutton */
  structure_proj_params.Wr = gtk_spin_button_get_value(spinbutton);

  /* Sync rad pattrern window spinbutton if enabled */
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	gtk_spin_button_set_value(
		rotate_rdpattern, (gdouble)structure_proj_params.Wr );

  New_Structure_Projection_Angle();
  gtk_spin_button_update( spinbutton );
}


  void
on_main_incline_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Get new "incline" structure angle from spinbutton */
  structure_proj_params.Wi = gtk_spin_button_get_value(spinbutton);

  /* Sync rad pattrern window spinbutton if enabled */
  if( isFlagSet(DRAW_ENABLED) && isFlagSet(COMMON_PROJECTION) )
	gtk_spin_button_set_value(
		incline_rdpattern, (gdouble)structure_proj_params.Wi );

  New_Structure_Projection_Angle();
  gtk_spin_button_update( spinbutton );
}


  void
on_main_currents_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable calculation and rendering of structure curents */
  Main_Currents_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  void
on_main_charges_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable calculation and rendering of structure charge density */
  Main_Charges_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  gboolean
on_main_colorcode_drawingarea_draw(
	GtkWidget       *widget,
	cairo_t         *cr,
	gpointer         user_data)
{
  Draw_Colorcode( cr );
  return( TRUE );
}


  void
on_main_freq_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  static gdouble fmhz_save = 0.0;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Frequency spinbutton value changed by frequency loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( structure_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
  else if( isFlagClear(FREQ_LOOP_INIT) ) /* by user */
  {
	/* Get freq from spin button, avoid double signal by GTK */
	gdouble fmhz = (gdouble)gtk_spin_button_get_value(spinbutton);
	if( (fmhz == fmhz_save) && isFlagClear(PLOT_FREQ_LINE) )
	  return; /* to avoid double signal by GTK */
	fmhz_save = fmhz;

	/* If new freq calculations are enabled by
	 * checkbutton next to freq spinbutton or
	 * freq line plotting enabled, redo currents */
	if( isFlagSet(PLOT_FREQ_LINE) ||
		(isFlagSet(MAIN_NEW_FREQ) &&
		 (isFlagSet(DRAW_CURRENTS) ||
		  isFlagSet(DRAW_CHARGES))) )
	{
	  /* Recalc currents in structure */
	  calc_data.fmhz = (double)fmhz;
	  g_idle_add( Redo_Currents, NULL );
	}

	/* Sync rad pattern frequency spinbutton */
	/* Show current frequency */
	if( isFlagSet(DRAW_ENABLED) 	&&
		isFlagSet(COMMON_FREQUENCY) &&
		isFlagSet(MAIN_NEW_FREQ) )
	  gtk_spin_button_set_value( rdpattern_frequency, fmhz );

	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( structure_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  } /* else */

  gtk_spin_button_update( spinbutton );
}


  void
on_main_freq_checkbutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	SetFlag(MAIN_NEW_FREQ);
  else
	ClearFlag(MAIN_NEW_FREQ);
}


  void
on_main_new_freq_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate (and redraw) currents on user command */
  if( isFlagClear(FREQ_LOOP_RUNNING) )
  {
	calc_data.fmhz =
	  (double)gtk_spin_button_get_value( mainwin_frequency );

	/* Sync rad pattern frequency spinbutton */
	/* Show current frequency */
	if( isFlagSet(DRAW_ENABLED) &&
		isFlagSet(COMMON_FREQUENCY) )
	  gtk_spin_button_set_value( rdpattern_frequency, calc_data.fmhz );

	g_idle_add( Redo_Currents, NULL );

	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( structure_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
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
  New_Projection_Parameters(
	  structure_width, structure_height,
	  &structure_proj_params );
  return( TRUE );
}


  gboolean
on_structure_drawingarea_motion_notify_event(
	GtkWidget       *widget,
	GdkEventMotion  *event,
	gpointer         user_data)
{
  static int cnt = 0;

  /* No redraws if new input pending or event blocked */
  /* Use only 1 in MOTION_EVENTS_COUNT event */
  if( (cnt++ < MOTION_EVENTS_COUNT) ||
	  isFlagSet(INPUT_PENDING) ||
	  isFlagSet(BLOCK_MOTION_EV) )
	return( FALSE );

  cnt = 0;

  /* Handle motion events */
  Motion_Event( event, &structure_proj_params );

  return( TRUE );
}


  gboolean
on_structure_drawingarea_draw(
	GtkWidget       *widget,
	cairo_t         *cr,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return( FALSE );

  Draw_Structure( cr );
  return( TRUE );
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
	Gtk_Widget_Destroy( file_chooser );
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
  /* Disable auto setting of freq plots toggle buttons */
  rc_config.freqplots_gmax_togglebutton    = 0;
  rc_config.freqplots_gdir_togglebutton    = 0;
  rc_config.freqplots_gviewer_togglebutton = 0;
  rc_config.freqplots_vswr_togglebutton    = 0;
  rc_config.freqplots_zrlzim_togglebutton  = 0;
  rc_config.freqplots_zmgzph_togglebutton  = 0;

  /* Close freq plots window without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
	Gtk_Widget_Destroy( freqplots_window );
	return( TRUE );
  }

  kill_window = freqplots_window;
  Delete_Event( _("Really close window?") );
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

  if( (strlen(rc_config.infile) == 0) ||
	  isFlagClear(PLOT_SELECT) )
	return;

  saveas_drawingarea = freqplots_drawingarea;
  saveas_width  = freqplots_width;
  saveas_height = freqplots_height;

  /* Make file name from input file name,
   * to save frequency plots drawing */
  snprintf( saveas, s, "%s-%s_%03d%s",
	  rc_config.infile, "plots", ++cnt, ".png" );

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
  saveas_drawingarea = freqplots_drawingarea;
  saveas_width  = freqplots_width;
  saveas_height = freqplots_height;

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.png", NULL, _("untitled.png"), rc_config.working_dir );
}


  void
on_freqplots_save_as_gnuplot_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Open file chooser to save frequency plots */
  SetFlag( PLOTS_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.gplot", NULL, _("untitled.gplot"), rc_config.working_dir );
}


  void
on_freqplots_gmax_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not max gain plotting */
  Plot_Select( togglebutton, PLOT_GMAX );
}


  void
on_freqplots_gdir_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not gain direction plotting */
  Plot_Select( togglebutton, PLOT_GAIN_DIR );
}


  void
on_freqplots_gviewer_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not "gain toward user" plotting */
  Plot_Select( togglebutton, PLOT_GVIEWER );
}


  void
on_freqplots_vswr_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not VSWR plotting */
  Plot_Select( togglebutton, PLOT_VSWR );
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
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( freqplots_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }

  gtk_spin_button_update( spinbutton );
}


  void
on_freqplots_zrlzim_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not Z-real/Z-imag plotting */
  Plot_Select( togglebutton, PLOT_ZREAL_ZIMAG );
}


  void
on_freqplots_zmgzph_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  /* Enable or not Z-mag/Z-phase plotting */
  Plot_Select( togglebutton, PLOT_ZMAG_ZPHASE );
}


  gboolean
on_freqplots_drawingarea_draw(
	GtkWidget       *widget,
	cairo_t         *cr,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return( FALSE );

  /* Enable drawing of freq line */
  Plot_Frequency_Data( cr );
  return( TRUE );
}


  gboolean
on_freqplots_drawingarea_configure_event(
	GtkWidget       *widget,
	GdkEventConfigure *event,
	gpointer         user_data)
{
  freqplots_width  = event->width;
  freqplots_height = event->height;
  return( TRUE );
}


  gboolean
on_freqplots_drawingarea_button_press_event(
	GtkWidget       *widget,
	GdkEventButton  *event,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return( FALSE );

  Set_Frequency_On_Click( event );
  return( TRUE );
}


  void
on_rdpattern_window_destroy(
	GObject       *object,
	gpointer       user_data)
{
  Rdpattern_Window_Killed();
}


  gboolean
on_rdpattern_window_delete_event(
	GtkWidget       *widget,
	GdkEvent        *event,
	gpointer         user_data)
{
  /* Disable auto setting of Gain and EH toggle buttons */
  rc_config.rdpattern_gain_togglebutton = 0;
  rc_config.rdpattern_eh_togglebutton   = 0;

  /* Close rdpattern window without confirmation dialog */
  if( !rc_config.confirm_quit )
  {
	Gtk_Widget_Destroy( rdpattern_window );
	return( TRUE );
  }

  kill_window = rdpattern_window;
  Delete_Event( _("Really close window?") );
  return( TRUE );
}


  void
on_rdpattern_save_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  char saveas[FILENAME_LEN + 24];
  size_t s = sizeof( saveas );
  static int cgn = 0, ceh = 0;;

  if( strlen(rc_config.infile) == 0 ) return;

  saveas_drawingarea = rdpattern_drawingarea;
  saveas_width  = rdpattern_width;
  saveas_height = rdpattern_height;

  /* Make the rad pattern save
   * file name from input name */
  if( isFlagSet(DRAW_GAIN) )
	snprintf( saveas, s, "%s-%s_%03d%s",
		rc_config.infile, "gain", ++cgn, ".png" );
  else if( isFlagSet(DRAW_EHFIELD) )
	snprintf( saveas, s, "%s-%s_%03d%s",
		rc_config.infile, "fields", ++ceh, ".png" );
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
  saveas_drawingarea = rdpattern_drawingarea;
  saveas_width  = rdpattern_width;
  saveas_height = rdpattern_height;

  /* Open file chooser to save frequency plots */
  SetFlag( IMAGE_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.png", NULL, _("untitled.png"), rc_config.working_dir );
}


  void
on_rdpattern_save_as_gnuplot_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Open file chooser to save frequency plots */
  SetFlag( RDPAT_GNUPLOT_SAVE );
  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
	  "*.gplot", NULL, _("untitled.gplot"), rc_config.working_dir );
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
on_rdpattern_e_field_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_EFIELD );
  else
	ClearFlag( DRAW_EFIELD );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
}


  void
on_rdpattern_h_field_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_HFIELD );
  else
	ClearFlag( DRAW_HFIELD );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
}


  void
on_rdpattern_poynting_vector_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_POYNTING );
  else
	ClearFlag( DRAW_POYNTING );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
}


  void
on_rdpattern_overlay_structure_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( OVERLAY_STRUCT );
  else
	ClearFlag( OVERLAY_STRUCT );

  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( rdpattern_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}


  void
on_rdpattern_x_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 0.0, 0.0, rotate_rdpattern,
	  incline_rdpattern, &rdpattern_proj_params );
  if( isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 0.0, 0.0, rotate_structure,
		incline_structure, &structure_proj_params );
}


  void
on_rdpattern_y_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 90.0, 0.0, rotate_rdpattern,
	  incline_rdpattern, &rdpattern_proj_params );
  if( isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 90.0, 0.0, rotate_structure,
		incline_structure, &structure_proj_params );
}


  void
on_rdpattern_z_axis_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate projection paramenters */
  New_Viewer_Angle( 0.0, 90.0, rotate_rdpattern,
	  incline_rdpattern, &rdpattern_proj_params );
  if( isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 0.0, 90.0, rotate_structure,
		incline_structure, &structure_proj_params );
}


  void
on_rdpattern_default_view_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Projection at 45 deg rotation and inclination */
  New_Viewer_Angle( 45.0, 45.0, rotate_rdpattern,
	  incline_rdpattern, &rdpattern_proj_params );
  if( isFlagSet(COMMON_PROJECTION) )
	New_Viewer_Angle( 45.0, 45.0, rotate_structure,
		incline_structure, &structure_proj_params );
}


  void
on_rdpattern_rotate_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Get new value of "rotate pattern" angle from spinbutton */
  rdpattern_proj_params.Wr = gtk_spin_button_get_value(spinbutton);

  /* Sync main window rotate spinbutton */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_spin_button_set_value(
		rotate_structure, (gdouble)rdpattern_proj_params.Wr );

  New_Radiation_Projection_Angle();
  gtk_spin_button_update( spinbutton );
}


  void
on_rdpattern_incline_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Get new value of "incline pattern" angle from spinbutton */
  rdpattern_proj_params.Wi = gtk_spin_button_get_value(spinbutton);

  /* Sync main window incline spinbutton */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_spin_button_set_value(
		incline_structure, (gdouble)rdpattern_proj_params.Wi );

  New_Radiation_Projection_Angle();
  gtk_spin_button_update( spinbutton );
}


  void
on_rdpattern_gain_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  Rdpattern_Gain_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  void
on_rdpattern_eh_togglebutton_toggled(
	GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  Rdpattern_EH_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  gboolean
on_rdpattern_colorcode_drawingarea_draw(
	GtkWidget       *widget,
	cairo_t         *cr,
	gpointer         user_data)
{
  Draw_Colorcode( cr );
  return( TRUE );
}


  void
on_rdpattern_freq_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  static gdouble fmhz_save = 0.0;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Frequency spinbutton value changed by frequency loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) &&
	  isFlagSet(DRAW_ENABLED) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
  else
  {
	/* Get freq from spin button, avoid double signal by GTK */
	gdouble fmhz = (gdouble)gtk_spin_button_get_value(spinbutton);
	if( fmhz == fmhz_save ) return; /* to avoid double signal by GTK */
	fmhz_save = fmhz;

	/* If new freq calculations are enabled
	 * by checkbutton next to freq spinbutton */
	GtkWidget *toggle =
	  Builder_Get_Object( rdpattern_window_builder, "rdpattern_freq_checkbutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)) &&
		(isFlagSet(DRAW_GAIN) || isFlagSet(DRAW_EHFIELD)) )
	{
	  /* Recalc currents in structure and rad pattern */
	  calc_data.fmhz = (double)fmhz;
	  g_idle_add( Redo_Radiation_Pattern, NULL );

	  /* Sync main window frequency spinbutton */
	  if( isFlagSet(COMMON_FREQUENCY) )
		gtk_spin_button_set_value( mainwin_frequency, fmhz );
	}
  } /* else */

  gtk_spin_button_update( spinbutton );
}


  void
on_rdpattern_new_freq_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  /* Recalculate and draw rad pattern after user command */
  if( isFlagClear(FREQ_LOOP_RUNNING) )
  {
	calc_data.fmhz =
	  (double)gtk_spin_button_get_value( rdpattern_frequency );
	Redo_Radiation_Pattern( NULL );

	/* Sync main frequency spinbutton */
	/* Show current frequency */
	if( isFlagSet(COMMON_FREQUENCY) )
	  gtk_spin_button_set_value( mainwin_frequency, calc_data.fmhz );

	g_idle_add( Redo_Currents, NULL );

	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( rdpattern_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
}


  gboolean
on_rdpattern_drawingarea_configure_event(
	GtkWidget       *widget,
	GdkEventConfigure *event,
	gpointer         user_data)
{
  rdpattern_width  = event->width;
  rdpattern_height = event->height;
  New_Projection_Parameters(
	  rdpattern_width,
	  rdpattern_height,
	  &rdpattern_proj_params );
  return( TRUE );
}


  gboolean
on_rdpattern_drawingarea_draw(
	GtkWidget       *widget,
	cairo_t         *cr,
	gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return( FALSE );

  Draw_Radiation( cr );
  return( TRUE );
}


  gboolean
on_rdpattern_drawingarea_motion_notify_event(
	GtkWidget       *widget,
	GdkEventMotion  *event,
	gpointer         user_data)
{
  static int cnt = 0;

  /* No redraws if new input pending or event blocked */
  /* Use only 1 in MOTION_EVENTS_COUNT event */
  if( (cnt++ < MOTION_EVENTS_COUNT) ||
	  isFlagSet(INPUT_PENDING) ||
	  isFlagSet(BLOCK_MOTION_EV) )
	return( FALSE );

  cnt = 0;

  /* Handle motion events */
  Motion_Event( event, &rdpattern_proj_params );

  return( TRUE );
}


  void
on_quit_cancelbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Gtk_Widget_Destroy( quit_dialog );
  ClearFlag( MAIN_QUIT );
  kill_window = NULL;
}


  void
on_quit_okbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	if( isFlagSet(MAIN_QUIT) )
	{
	  Stop_Frequency_Loop();
	  gtk_label_set_text( GTK_LABEL(
			Builder_Get_Object(quit_dialog_builder, "quit_label")),
		  _("Really quit Xnec2c?") );
	  ClearFlag( MAIN_QUIT );
	  return;
	}

	/* Stop freq loop if only one of plots
	 * or radiation pattern windows is open */
	if( (isFlagSet(DRAW_ENABLED) && isFlagClear(PLOT_ENABLED)) ||
		(isFlagClear(DRAW_ENABLED) && isFlagSet(PLOT_ENABLED)) )
	  Stop_Frequency_Loop();

  } /* if( isFlagSet(FREQ_LOOP_RUNNING) ) */

  Gtk_Widget_Destroy( quit_dialog );
  Gtk_Widget_Destroy( kill_window );
}


  void
main_view_menuitem_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Sync common projection checkbuttons */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(main_window_builder, "main_common_projection")),
		TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(main_window_builder, "main_common_projection")),
		FALSE );

  /* Sync common frequency checkbuttons */
  if( isFlagSet(COMMON_FREQUENCY) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(main_window_builder, "main_common_frequency")),
		TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(main_window_builder, "main_common_frequency")),
		FALSE );

}


  void
main_pol_menu_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Pol_Menuitem( MAIN_WINDOW );
}


  void
freqplots_pol_menu_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Pol_Menuitem( FREQPLOTS_WINDOW );
}


  void
rdpattern_pol_menu_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  Set_Pol_Menuitem( RDPATTERN_WINDOW );
}


  void
rdpattern_view_menuitem_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  /* Sync common projection checkbuttons */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(rdpattern_window_builder, "rdpattern_common_projection")),
		TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(rdpattern_window_builder, "rdpattern_common_projection")),
		FALSE );

  /* Sync common frequency checkbuttons */
  if( isFlagSet(COMMON_FREQUENCY) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(rdpattern_window_builder, "rdpattern_common_frequency")),
		TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  Builder_Get_Object(rdpattern_window_builder, "rdpattern_common_frequency")),
		FALSE );
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
  return( FALSE );
}


  void
on_near_peak_value_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	ClearFlag( NEAREH_SNAPSHOT );

	/* Redraw radiation pattern drawingarea */
	if( isFlagSet(DRAW_EHFIELD) )
	{
	  near_field.valid = 0;
	  Near_Field_Pattern();

	  /* Wait for GTK to complete its tasks */
	  gtk_widget_queue_draw( rdpattern_drawingarea );
	  while( g_main_context_iteration(NULL, FALSE) );
	}
  }
  else SetFlag( NEAREH_SNAPSHOT );

  /* Reset child near field flags */
  Pass_EH_Flags();
}


  void
on_near_snapshot_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	SetFlag( NEAREH_SNAPSHOT );
	/* Redraw radiation pattern drawingarea */
	if( isFlagSet(DRAW_EHFIELD) )
	{
	  near_field.valid = 0;
	  Near_Field_Pattern();

	  /* Wait for GTK to complete its tasks */
	  while( g_main_context_iteration(NULL, FALSE) );
	  gtk_widget_queue_draw( rdpattern_drawingarea );
	}
  }
  else ClearFlag( NEAREH_SNAPSHOT );

  /* Reset child near field flags */
  Pass_EH_Flags();
}


  void
on_rdpattern_animate_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( isFlagClear(DRAW_EHFIELD) )
	return;

  if( animate_dialog == NULL )
  {
	animate_dialog = create_animate_dialog( &animate_dialog_builder );
  }
  gtk_widget_show( animate_dialog );
}


  void
on_animation_applybutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, steps;

  spinbutton = GTK_SPIN_BUTTON(
	  Builder_Get_Object(animate_dialog_builder, "animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON(
	  Builder_Get_Object(animate_dialog_builder, "animate_steps_spinbutton") );
  steps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / steps / freq);
  near_field.anim_step = (double)M_2PI / steps;

  SetFlag( NEAREH_ANIMATE );
  if( anim_tag > 0 )
	g_source_remove( anim_tag );
  anim_tag = g_timeout_add( intval, Animate_Near_Field, NULL );
}


  void
on_animation_cancelbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  ClearFlag( NEAREH_ANIMATE );
  if( anim_tag )
	g_source_remove( anim_tag );
  anim_tag = 0;
}


  void
on_animation_okbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, steps;

  spinbutton = GTK_SPIN_BUTTON(
	  Builder_Get_Object(animate_dialog_builder, "animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON(
	  Builder_Get_Object(animate_dialog_builder, "animate_steps_spinbutton") );
  steps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / steps / freq);
  near_field.anim_step = (double)M_2PI / steps;

  SetFlag( NEAREH_ANIMATE );
  if( anim_tag > 0 )
	g_source_remove( anim_tag );
  anim_tag = g_timeout_add( intval, Animate_Near_Field, NULL );

  gtk_widget_hide( animate_dialog );
}


  void
on_animate_dialog_destroy(
	GObject       *object,
	gpointer       user_data)
{
  animate_dialog = NULL;
  g_object_unref( animate_dialog_builder );
  animate_dialog_builder = NULL;
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
	Stop( _("No open NEC2 input file"), ERR_OK );
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
	SetFlag( MAIN_QUIT );

	/* Prompt user to save NEC2 data */
	if( Nec2_Edit_Save() ) return( TRUE );

	/* Save GUI state for restoring windows */
	Get_GUI_State();
	Save_Config();

	/* Quit without confirmation dialog */
	if( !rc_config.confirm_quit )
	{
	  Gtk_Widget_Destroy( main_window );
	  return( TRUE );
	}

	Delete_Event( _("Really quit xnec2c?") );
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
	Gtk_Widget_Destroy( nec2_edit_window );
	return( TRUE );
  }

  Delete_Event( _("Really close window?") );
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
  if( strlen(rc_config.infile) == 0 )
  {
	/* Open file chooser to save NEC2 input file */
	SetFlag( NEC2_SAVE );
	ClearFlag( OPEN_NEW_NEC2 );
	file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
		"*.nec", NULL, _("untitled.nec"), rc_config.working_dir );
	return;
  }

  /* Save NEC2 editor data */
  Save_Nec2_Input_File( nec2_edit_window, rc_config.infile );
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
  if( ncols == 2 ) /* Comments treeview */
	gtk_list_store_set(	GTK_LIST_STORE(model), &iter, 0, "CM", -1 );
  else
  {
	int idx;
	for( idx = 0; idx < ncols; idx++ )
	  gtk_list_store_set( GTK_LIST_STORE(model), &iter, idx, "--", -1 );
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
	action = EDITOR_EDIT;
	Open_Editor( selected_treeview );
	action = EDITOR_NEW;
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
	action = EDITOR_EDIT;
	Open_Editor( selected_treeview );
	action = EDITOR_NEW;
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
	action = EDITOR_EDIT;
	Open_Editor( selected_treeview );
	action = EDITOR_NEW;
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
	action = EDITOR_EDIT;
	Open_Editor( selected_treeview );
	action = EDITOR_NEW;
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
  if( strlen(rc_config.infile) == 0 ) return;
  Open_File( &input_fp, rc_config.infile, "r" );
  Nec2_Input_File_Treeview( NEC2_EDITOR_REVERT );
}


  void
on_nec2_save_dialog_response(
	GtkDialog       *dialog,
	gint             response_id,
	gpointer         user_data)
{
  Gtk_Widget_Destroy( nec2_save_dialog );

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
	  Gtk_Widget_Destroy( nec2_save_dialog );
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

	  rc_config.infile[0] = '\0';
	  selected_treeview = cmnt_treeview;
	  ClearFlag( OPEN_NEW_NEC2 );
	}
  } /* if( response_id == GTK_RESPONSE_NO ) */
  else if( response_id == GTK_RESPONSE_YES )
  {
	/* Open file chooser to specify file name to save
	 * NEC2 editor data to, if no file is already open */
	SetFlag( NEC2_SAVE );
	if( strlen(rc_config.infile) == 0 )
	{
	  file_chooser = Open_Filechooser( GTK_FILE_CHOOSER_ACTION_SAVE,
		  "*.nec", NULL, "untitled.nec", rc_config.working_dir );
	  return;
	}
	else /* Save to already open input file */
	  Save_Nec2_Input_File( nec2_edit_window, rc_config.infile );

	/* Re-open NEC2 input file */
	gboolean new = FALSE;
	if( Nec2_Apply_Checkbutton() &&	isFlagClear(MAIN_QUIT) )
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

	  rc_config.infile[0] = '\0';
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
  Gtk_Widget_Destroy( kill_window );
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
  Gtk_Widget_Destroy( error_dialog );
  gtk_main();
  exit(0);
}


  void
on_error_okbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  ClearFlag( ERROR_CONDX );
  Gtk_Widget_Destroy( error_dialog );
}


  void
on_error_quitbutton_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Gtk_Widget_Destroy( error_dialog );
  Gtk_Widget_Destroy( main_window );
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
  Gtk_Widget_Destroy( wire_editor );
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
  Gtk_Widget_Destroy( wire_editor );
}


  void
on_gw_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &wire_editor, &wire_editor_builder,
	  create_wire_editor, Wire_Editor, action );
}


  void
on_ga_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &arc_editor, &arc_editor_builder,
	  create_arc_editor, Arc_Editor, action );
}


  void
on_gh_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &helix_editor, &helix_editor_builder,
	  create_helix_editor, Helix_Editor, action );
}


  void
on_sp_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &patch_editor, &patch_editor_builder,
	  create_patch_editor, Patch_Editor, action );
}


  void
on_gr_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &cylinder_editor, &cylinder_editor_builder,
	  create_cylinder_editor, Cylinder_Editor, action );
}


  void
on_gm_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &transform_editor, &transform_editor_builder,
	  create_transform_editor, Transform_Editor, action );
}


  void
on_gx_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &reflect_editor, &reflect_editor_builder,
	  create_reflect_editor, Reflect_Editor, action );
}


  void
on_gs_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &scale_editor, &scale_editor_builder,
	  create_scale_editor, Scale_Editor, action );
}


  void
on_ex_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &excitation_command, &excitation_editor_builder,
	  create_excitation_command, Excitation_Command, action );
}


  void
on_fr_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &frequency_command, &frequency_editor_builder,
	  create_frequency_command, Frequency_Command, action );
}


  void
on_gn_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &ground_command, &ground_editor_builder,
	  create_ground_command, Ground_Command, action );
}


  void
on_gd_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &ground2_command, &ground2_editor_builder,
	  create_ground2_command, Ground2_Command, action );
}


  void
on_rp_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &radiation_command, &radiation_editor_builder,
	  create_radiation_command, Radiation_Command, action );
}


  void
on_ld_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &loading_command, &loading_editor_builder,
	  create_loading_command, Loading_Command, action );
}


  void
on_nt_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &network_command, &network_editor_builder,
	  create_network_command, Network_Command, action );
}


  void
on_tl_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &txline_command, &txline_editor_builder,
	  create_txline_command, Txline_Command, action );
}


  void
on_ne_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &nearfield_command, &nearfield_editor_builder,
	  create_nearfield_command, Nearfield_Command, action );
}


  void
on_ek_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &kernel_command, &kernel_editor_builder,
	  create_kernel_command, Kernel_Command, action );
}


  void
on_kh_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &intrange_command, &intrange_editor_builder,
	  create_intrange_command, Intrange_Command, action );
}


  void
on_zo_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &zo_command, &zo_editor_builder,
	  create_zo_command, Zo_Command, action );
}


  void
on_xq_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Card_Clicked(
	  &execute_command, &execute_editor_builder,
	  create_execute_command, Execute_Command, action );
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
  Gtk_Widget_Destroy( patch_editor );
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
  Gtk_Widget_Destroy( patch_editor );
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
  Gtk_Widget_Destroy( arc_editor );
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
  Gtk_Widget_Destroy( arc_editor );
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
  Gtk_Widget_Destroy( helix_editor );
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
  Gtk_Widget_Destroy( helix_editor );
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
  Gtk_Widget_Destroy( reflect_editor );
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
  Gtk_Widget_Destroy( reflect_editor );
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
  Gtk_Widget_Destroy( scale_editor );
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
  Gtk_Widget_Destroy( scale_editor );
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
  Gtk_Widget_Destroy( cylinder_editor );
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
  Gtk_Widget_Destroy( cylinder_editor );
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
  Gtk_Widget_Destroy( transform_editor );
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
  Gtk_Widget_Destroy( transform_editor );
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
  Gtk_Widget_Destroy( gend_editor );
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
  Gtk_Widget_Destroy( gend_editor );
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
  Gtk_Widget_Destroy( kernel_command );
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
  Gtk_Widget_Destroy( kernel_command );
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
  Gtk_Widget_Destroy( execute_command );
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
  Gtk_Widget_Destroy( execute_command );
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
  Gtk_Widget_Destroy( intrange_command );
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
  Gtk_Widget_Destroy( intrange_command );
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
  Gtk_Widget_Destroy( zo_command );
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
  Gtk_Widget_Destroy( zo_command );
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
  Gtk_Widget_Destroy( ground_command );
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
  Gtk_Widget_Destroy( ground_command );
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
  Gtk_Widget_Destroy( nearfield_command );
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
  Gtk_Widget_Destroy( nearfield_command );
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
  Gtk_Widget_Destroy( radiation_command );
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
  Gtk_Widget_Destroy( radiation_command );
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
  Gtk_Widget_Destroy( excitation_command );
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
  Gtk_Widget_Destroy( excitation_command );
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
  Gtk_Widget_Destroy( frequency_command );
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
  Gtk_Widget_Destroy( frequency_command );
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
  Gtk_Widget_Destroy( loading_command );
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
  Gtk_Widget_Destroy( loading_command );
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
  Gtk_Widget_Destroy( network_command );
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
  Gtk_Widget_Destroy( network_command );
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
  Gtk_Widget_Destroy( txline_command );
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
  Gtk_Widget_Destroy( txline_command );
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
  Gtk_Widget_Destroy( ground2_command );
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
  Gtk_Widget_Destroy( ground2_command );
}


  void
on_loop_start_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  Start_Frequency_Loop();
  rc_config.main_loop_start = 1;
}


  void
on_loop_pause_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
	SetFlag( FREQ_LOOP_STOP );
  rc_config.main_loop_start = 0;
}


  void
on_loop_reset_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  if( isFlagClear(FREQ_LOOP_RUNNING) )
	SetFlag( FREQ_LOOP_INIT );
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
  Gtk_Widget_Destroy( aboutdialog );
}


  void
on_aboutdialog_destroy(
	GObject       *object,
	gpointer	   user_data)
{
  aboutdialog = NULL;
}


  void
on_aboutdialog_response(
	GtkDialog       *dialog,
	gint             response_id,
	gpointer         user_data)
{
  Gtk_Widget_Destroy( aboutdialog );
}


  void
on_freqplots_net_gain_activate(
	GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( PLOT_NETGAIN );
  else
	ClearFlag( PLOT_NETGAIN );

  /* Trigger a redraw of frequency plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(FREQ_LOOP_DONE) )
  {
	/* Wait for GTK to complete its tasks */
	gtk_widget_queue_draw( freqplots_drawingarea );
	while( g_main_context_iteration(NULL, FALSE) );
  }
}


  gboolean
on_structure_drawingarea_button_press_event(
	GtkWidget      *widget,
	GdkEventButton  *event,
	gpointer         user_data)
{
  structure_proj_params.reset = TRUE;
  return( FALSE );
}


  void
on_main_zoom_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  structure_proj_params.xy_zoom = gtk_spin_button_get_value( spinbutton );
  structure_proj_params.xy_zoom /= 100.0;
  structure_proj_params.xy_scale =
	structure_proj_params.xy_scale1 * structure_proj_params.xy_zoom;

  /* Trigger a redraw of structure drawingarea */
  /* Wait for GTK to complete its tasks */
  if( structure_drawingarea )
	gtk_widget_queue_draw( structure_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}


  void
on_structure_plus_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  structure_proj_params.xy_zoom =
	gtk_spin_button_get_value( structure_zoom );
  structure_proj_params.xy_zoom *= 1.1;
  gtk_spin_button_set_value(
	  structure_zoom, structure_proj_params.xy_zoom );
}


  void
on_structure_minus_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  structure_proj_params.xy_zoom =
	gtk_spin_button_get_value( structure_zoom );
  structure_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  structure_zoom, structure_proj_params.xy_zoom );
}


  void
on_structure_one_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  gtk_spin_button_set_value( structure_zoom, 100.0 );
  structure_proj_params.reset = TRUE;
  structure_proj_params.dx_center = 0.0;
  structure_proj_params.dy_center = 0.0;
  New_Projection_Parameters(
	  structure_width,
	  structure_height,
	  &structure_proj_params );

  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( structure_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}


  gboolean
on_rdpattern_drawingarea_button_press_event(
	GtkWidget      *widget,
	GdkEventButton  *event,
	gpointer         user_data)
{
  rdpattern_proj_params.reset = TRUE;
  return( FALSE );
}


  void
on_rdpattern_zoom_spinbutton_value_changed(
	GtkSpinButton   *spinbutton,
	gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom  = gtk_spin_button_get_value( spinbutton );
  rdpattern_proj_params.xy_zoom /= 100.0;
  rdpattern_proj_params.xy_scale =
	rdpattern_proj_params.xy_scale1 * rdpattern_proj_params.xy_zoom;

  /* Trigger a redraw of structure drawingarea */
  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( rdpattern_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}


  void
on_rdpattern_plus_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom =
	gtk_spin_button_get_value( rdpattern_zoom );
  rdpattern_proj_params.xy_zoom *= 1.1;
  gtk_spin_button_set_value(
	  rdpattern_zoom, rdpattern_proj_params.xy_zoom );
}


  void
on_rdpattern_minus_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom =
	gtk_spin_button_get_value( rdpattern_zoom );
  rdpattern_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  rdpattern_zoom, rdpattern_proj_params.xy_zoom );
}


  void
on_rdpattern_one_button_clicked(
	GtkButton       *button,
	gpointer         user_data)
{
  gtk_spin_button_set_value( rdpattern_zoom, 100.0 );
  rdpattern_proj_params.reset = TRUE;
  rdpattern_proj_params.dx_center = 0.0;
  rdpattern_proj_params.dy_center = 0.0;
  New_Projection_Parameters(
	  rdpattern_width,
	  rdpattern_height,
	  &rdpattern_proj_params );

  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( rdpattern_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}


  gboolean
on_structure_drawingarea_scroll_event(
	GtkWidget       *widget,
	GdkEvent        *event,
	gpointer         user_data)
{
  structure_proj_params.xy_zoom =
	gtk_spin_button_get_value( structure_zoom );
  if( event->scroll.direction == GDK_SCROLL_UP )
	structure_proj_params.xy_zoom *= 1.1;
  else if( event->scroll.direction == GDK_SCROLL_DOWN )
	structure_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  structure_zoom, structure_proj_params.xy_zoom );
  return( FALSE );
}


  gboolean
on_rdpattern_drawingarea_scroll_event(
	GtkWidget       *widget,
	GdkEvent        *event,
	gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom =
	gtk_spin_button_get_value( rdpattern_zoom );
  if( event->scroll.direction == GDK_SCROLL_UP )
	rdpattern_proj_params.xy_zoom *= 1.1;
  else if( event->scroll.direction == GDK_SCROLL_DOWN )
	rdpattern_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  rdpattern_zoom, rdpattern_proj_params.xy_zoom );
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
	Gtk_Widget_Destroy( widget );
	return( TRUE );
  }
  else return( FALSE );
}

