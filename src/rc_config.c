/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#include "rc_config.h"

/*------------------------------------------------------------------------*/

/* Get_Widget_Geometry()
 *
 * Gets a widget's size and position
 */
  static void
Get_Window_Geometry(
	GtkWidget *window,
	gint *x, gint *y, gint *width, gint *height )
{
  if( window ) /* Save window state for restoring later */
  {
	gtk_window_get_size( GTK_WINDOW(window), width, height );
	gtk_window_get_position( GTK_WINDOW(window), x, y );
  }
  else /* Disable window restoring */
  {
	*x = *y = 0;
	*width = *height = 0;
  }

} /* Get_Widget_Geometry() */

/*------------------------------------------------------------------------*/

/* Create_Default_Config()
 *
 * Creates a default xnec2.conf file
 */
  gboolean
Create_Default_Config( void )
{
  char
	line[LINE_LEN], /* Buffer for Load_Line() */
	cfg_file[64];   /* Path to config file */

  FILE *fp = NULL;

  /* Setup file path to xnec2c config file */
  snprintf( cfg_file, sizeof(cfg_file), "%s/%s", getenv("HOME"), CONFIG_FILE );
  fp = fopen( cfg_file, "r" );
  if( fp != NULL )
  {
	/* Read Application Version */
	if( Load_Line(line, fp) == EOF )
	{
	  fprintf( stderr, _("xnec2c: failed to read Application Version\n") );
	  Close_File( &fp );
	  return( FALSE );
	}

	/* Produce fresh default config file if version number new */
	if( strncmp(line, PACKAGE_STRING, sizeof(line)) != 0 )
	  printf( _("xnec2c: existing config file version incopatible: %s\n"), line );
	else
	{
	  Close_File( &fp );
	  return( TRUE );
	}
  } /* if( (fp = fopen(cfg_file, "r")) != NULL ) */

  /* Make a default configuration, will not usually work well */
  printf( _("xnec2c: creating a default config file: %s\n"), cfg_file );

  /* For main window */
  Strlcpy( rc_config.working_dir, getenv("HOME"), sizeof(rc_config.working_dir) );
  Strlcat( rc_config.working_dir, "/", sizeof(rc_config.working_dir) );
  rc_config.main_width  = 600;
  rc_config.main_height = 400;
  rc_config.main_x = 0;
  rc_config.main_y = 0;
  rc_config.main_currents_togglebutton = 0;
  rc_config.main_charges_togglebutton  = 0;
  rc_config.main_total = 0;
  rc_config.main_horizontal = 0;
  rc_config.main_vertical   = 0;
  rc_config.main_right_hand = 0;
  rc_config.main_left_hand  = 0;
  rc_config.main_loop_start = 0;
  rc_config.main_rotate_spinbutton  = 45;
  rc_config.main_incline_spinbutton = 45;
  rc_config.main_zoom_spinbutton    = 100;

  /* For rdpattern window */
  rc_config.rdpattern_width  = 0;
  rc_config.rdpattern_height = 0;
  rc_config.rdpattern_x = 0;
  rc_config.rdpattern_y = 0;
  rc_config.rdpattern_gain_togglebutton = 0;
  rc_config.rdpattern_eh_togglebutton   = 0;
  rc_config.rdpattern_e_field = 1;
  rc_config.rdpattern_h_field = 1;
  rc_config.rdpattern_poynting_vector = 0;
  rc_config.rdpattern_zoom_spinbutton = 100;

  /* For freq plots window */
  rc_config.freqplots_width  = 0;
  rc_config.freqplots_height = 0;
  rc_config.freqplots_x = 0;
  rc_config.freqplots_y = 0;
  rc_config.freqplots_gmax_togglebutton    = 0;
  rc_config.freqplots_gdir_togglebutton    = 0;
  rc_config.freqplots_gviewer_togglebutton = 0;
  rc_config.freqplots_vswr_togglebutton    = 0;
  rc_config.freqplots_zrlzim_togglebutton  = 0;
  rc_config.freqplots_zmgzph_togglebutton  = 0;
  rc_config.freqplots_net_gain = 0;

  /* For NEC2 editor window */
  rc_config.nec2_edit_width  = 0;
  rc_config.nec2_edit_height = 0;
  rc_config.nec2_edit_x = 0;
  rc_config.nec2_edit_y = 0;

  /* Enable Quit Dialog */
  rc_config.confirm_quit = 1;

  /* Structure and rdpattern center offset */
  structure_proj_params.dx_center = 0.0;
  structure_proj_params.dy_center = 0.0;
  rdpattern_proj_params.dx_center = 0.0;
  rdpattern_proj_params.dy_center = 0.0;

  /* Save default config to file */
  if( !Save_Config() )
	fprintf( stderr, _("xnec2c: failed to save default config file\n") );

  Close_File( &fp );
  return( TRUE );

} /* Create_Default_Config() */

/*------------------------------------------------------------------------*/

/* Set_Window_Geometry()
 *
 * Sets the size and position of a window
 */
  void
Set_Window_Geometry(
	GtkWidget *window,
	gint x, gint y, gint width, gint height )
{
  if( (width == 0) || (height == 0) ) return;

  /* Set size and position of window */
  gtk_widget_hide( window );
  gtk_window_resize( GTK_WINDOW(window), width, height );
  gtk_window_move( GTK_WINDOW(window), x, y );

} /* Set_Window_Geometry() */

/*------------------------------------------------------------------------*/

/* Restore_Windows()
 *
 * Restores the rdpattern and freq plots windows
 */
  static gboolean
Restore_Windows( gpointer dat )
{
  GtkWidget *widget;

  /* Open radiation pattern window if state data available */
  if( rc_config.rdpattern_width &&
	  rc_config.rdpattern_height )
  {
	widget = Builder_Get_Object( main_window_builder, "main_rdpattern" );
	gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  /* Open frequency plots window if state data available */
  if( rc_config.freqplots_width &&
	  rc_config.freqplots_height )
  {
	widget = Builder_Get_Object( main_window_builder, "main_freqplots" );
	gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  return( FALSE );
}

/*------------------------------------------------------------------------*/

/* Restore_GUI_State()
 *
 * Restores the state of the GUI including window geometry
 */
  static void
Restore_GUI_State( void )
{
  GtkWidget *widget;

  /* Restore main (structure) window geometry */
  Set_Window_Geometry( main_window,
	  rc_config.main_x, rc_config.main_y,
	  rc_config.main_width, rc_config.main_height );
  gtk_widget_show( main_window );

  /* Restore main (structure) window widgets state */
  if( rc_config.main_currents_togglebutton )
  {
	widget = Builder_Get_Object( main_window_builder, "main_currents_togglebutton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
  }

  if( rc_config.main_charges_togglebutton )
  {
	widget = Builder_Get_Object( main_window_builder, "main_charges_togglebutton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), TRUE );
  }

  if( rc_config.main_total )
  {
	widget = Builder_Get_Object( main_window_builder, "main_total" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  }

  if( rc_config.main_horizontal )
  {
	widget = Builder_Get_Object( main_window_builder, "main_horizontal" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  }

  if( rc_config.main_vertical )
  {
	widget = Builder_Get_Object( main_window_builder, "main_vertical" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  }

  if( rc_config.main_right_hand )
  {
	widget = Builder_Get_Object( main_window_builder, "main_right_hand" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  }

  if( rc_config.main_left_hand )
  {
	widget = Builder_Get_Object( main_window_builder, "main_left_hand" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  }

  if( rc_config.main_loop_start )
  {
	widget = Builder_Get_Object( main_window_builder, "main_loop_start" );
	gtk_button_clicked( GTK_BUTTON(widget) );
  }

  /* Set the "Confirm Quit" menu item */
  widget = Builder_Get_Object( main_window_builder, "confirm_quit" );
  if( rc_config.confirm_quit )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );

  g_idle_add( Restore_Windows, NULL );

} /* Restore_GUI_State() */

/*------------------------------------------------------------------------*/

/* Read_Config()
 *
 * Loads the xnec2crc configuration file
 */

  gboolean
Read_Config( void )
{
  char
	fpath[64],		/* File path to xnec2crc */
	line[LINE_LEN]; /* Buffer for Load_Line() */
  int idx;

  /* Config and mnemonics file pointer */
  FILE *fp = NULL;


  /* Setup file path to xnec2c rc file */
  snprintf( fpath, sizeof(fpath), "%s/%s", getenv("HOME"), CONFIG_FILE );

  /* Open xnec2c config file */
  if( !Open_File(&fp, fpath, "r") ) return( FALSE );

  /* Read Application Version (not used here) */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Application Version \n") );
	return( FALSE );
  }

  /* Read working directory */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Working Directory \n") );
	return( FALSE );
  }
  Strlcpy( rc_config.working_dir, line, sizeof(rc_config.working_dir) );

  /* Read main window size */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window size\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.main_width = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.main_height = atoi( &line[idx + 1] );

  /* Read main window position */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window position\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.main_x = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.main_y = atoi( &line[idx + 1] );

  /* Read main window Currents toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Currents toggle state\n") );
	return( FALSE );
  }
  rc_config.main_currents_togglebutton = (u_int8_t)atoi( line );

  /* Read main window Charges toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Charges toggle state\n") );
	return( FALSE );
  }
  rc_config.main_charges_togglebutton = (u_int8_t)atoi( line );

  /* Read main window Total menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Total menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_total = (u_int8_t)atoi( line );

  /* Read main window Horizontal menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Horizontal menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_horizontal = (u_int8_t)atoi( line );

  /* Read main window Vertical menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Vertical menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_vertical = (u_int8_t)atoi( line );

  /* Read main window Right Hand menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Right Hand menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_right_hand = (u_int8_t)atoi( line );

  /* Read main window Left Hand menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Left Hand menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_left_hand = (u_int8_t)atoi( line );

  /* Read main window Loop Start menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Loop Start menuitem state\n") );
	return( FALSE );
  }
  rc_config.main_loop_start = (u_int8_t)atoi( line );

  /* Read main window Rotate Spinbutton value */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Main Window Rotate Spinbutton value\n") );
	return( FALSE );
  }
  rc_config.main_rotate_spinbutton = atoi( line );

  /* Read main window Incline Spinbutton value */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read main Incline Spinbutton value\n") );
	return( FALSE );
  }
  rc_config.main_incline_spinbutton = atoi( line );

  /* Read main window Zoom Spinbutton value */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read main Zoom Spinbutton value\n") );
	return( FALSE );
  }
  rc_config.main_zoom_spinbutton = atoi( line );

  /* Read radiation pattern window size */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window size\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.rdpattern_width = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.rdpattern_height = atoi( &line[idx + 1] );

  /* Read radiation pattern window position */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window position\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.rdpattern_x = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.rdpattern_y = atoi( &line[idx + 1] );

  /* Read radiation pattern window Gain toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window Gain toggle state\n") );
	return( FALSE );
  }
  rc_config.rdpattern_gain_togglebutton = (u_int8_t)atoi( line );

  /* Read radiation pattern window EH toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window EH toggle state\n") );
	return( FALSE );
  }
  rc_config.rdpattern_eh_togglebutton = (u_int8_t)atoi( line );

  /* Read radiation pattern window E menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window E men uitem state\n") );
	return( FALSE );
  }
  rc_config.rdpattern_e_field = (u_int8_t)atoi( line );

  /* Read radiation pattern window H menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window H menu item state\n") );
	return( FALSE );
  }
  rc_config.rdpattern_h_field = (u_int8_t)atoi( line );

  /* Read radiation pattern window Poynting menuitem state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern window Poynting menu item state\n") );
	return( FALSE );
  }
  rc_config.rdpattern_poynting_vector = (u_int8_t)atoi( line );

  /* Read radiation pattern window Zoom Spinbutton value */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Radiation Pattern Zoom Spinbutton value\n") );
	return( FALSE );
  }
  rc_config.rdpattern_zoom_spinbutton = atoi( line );

  /* Read frequency plots window size */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window size\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.freqplots_width = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.freqplots_height = atoi( &line[idx + 1] );

  /* Read frequency plots window position */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window position\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.freqplots_x = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.freqplots_y = atoi( &line[idx + 1] );

  /* Read frequency plots window Max Gain toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window GMax toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_gmax_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window GAin Direction toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window GDir toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_gdir_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window Gain in Viewer direction toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window GViewer toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_gviewer_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window VSWR toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window VSWR toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_vswr_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window Z-real/Z-imag toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window Z-real/Z-imag toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_zrlzim_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window Z-mag/Z-phase toggle button state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window Z-mag/Z-phase toggle state\n") );
	return( FALSE );
  }
  rc_config.freqplots_zmgzph_togglebutton = (u_int8_t)atoi( line );

  /* Read frequency plots window Net Gain menu item state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Frequency Plots window Net Gain menu item state\n") );
	return( FALSE );
  }
  rc_config.freqplots_net_gain = (u_int8_t)atoi( line );

  /* Read NEC2 editor window size */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read NEC2 Editor window size\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.nec2_edit_width = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.nec2_edit_height = atoi( &line[idx + 1] );

  /* Read NEC2 editor window position */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read NEC2 Editor window position\n") );
	return( FALSE );
  }
  idx = 0;
  rc_config.nec2_edit_x = atoi( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rc_config.nec2_edit_y = atoi( &line[idx + 1] );

  /* Read Structure Projection Center x and y offset */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Structure Projection Center x, y offset\n") );
	return( FALSE );
  }
  idx = 0;
  structure_proj_params.dx_center = atof( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  structure_proj_params.dy_center = atof( &line[idx + 1] );

  /* Read Rdpattern Projection Center x and y offset */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Rdpattern Projection Center x, y offset\n") );
	return( FALSE );
  }
  idx = 0;
  rdpattern_proj_params.dx_center = atof( line );
  do idx++;
  while( (line[idx] != ',') && (idx < LINE_LEN - 2) );
  rdpattern_proj_params.dy_center = atof( &line[idx + 1] );

  /* Read Enable Confirm Quit dialog state */
  if( Load_Line(line, fp) == EOF )
  {
	fprintf( stderr,
		_("xnec2c: failed to read Enable Confirm Quit dialog state\n") );
	return( FALSE );
  }
  rc_config.confirm_quit = (u_int8_t)atoi( line );

  /* Close the config file pointer */
  Close_File( &fp );

  Restore_GUI_State();

  return( TRUE );
} /* Read_Config() */

/*------------------------------------------------------------------------*/

/* Get_GUI_State()
 *
 * Gets the GUI state and stores it in the rc_config buffer
 */
  void
Get_GUI_State( void )
{
  GtkWidget *widget;

  /* Get geometry of main (structure) window */
  Get_Window_Geometry( main_window,
	  &(rc_config.main_x), &(rc_config.main_y),
	  &(rc_config.main_width), &(rc_config.main_height) );

  /* Get state of widgets in main (structure) window */
  rc_config.main_currents_togglebutton = 0;
  rc_config.main_charges_togglebutton  = 0;
  rc_config.main_total = 0;
  rc_config.main_horizontal = 0;
  rc_config.main_vertical   = 0;
  rc_config.main_right_hand = 0;
  rc_config.main_left_hand  = 0;
  widget = Builder_Get_Object( main_window_builder, "main_currents_togglebutton" );
  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	rc_config.main_currents_togglebutton = 1;

  widget = Builder_Get_Object( main_window_builder, "main_charges_togglebutton" );
  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	rc_config.main_charges_togglebutton = 1;

  if( calc_data.pol_type == POL_TOTAL )
	rc_config.main_total = 1;
  else if( calc_data.pol_type == POL_HORIZ )
	rc_config.main_horizontal = 1;
  else if( calc_data.pol_type == POL_VERT )
	rc_config.main_vertical = 1;
  else if( calc_data.pol_type == POL_RHCP )
	rc_config.main_right_hand = 1;
  else if( calc_data.pol_type == POL_LHCP )
	rc_config.main_left_hand = 1;

  widget = Builder_Get_Object( main_window_builder, "main_rotate_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_rotate_spinbutton =
	gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

  widget = Builder_Get_Object( main_window_builder, "main_incline_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_incline_spinbutton =
	gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

  widget = Builder_Get_Object( main_window_builder, "main_zoom_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_zoom_spinbutton =
	gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

  /* Get geometry of radiation patterns window */
  Get_Window_Geometry( rdpattern_window,
	  &(rc_config.rdpattern_x), &(rc_config.rdpattern_y),
	  &(rc_config.rdpattern_width), &(rc_config.rdpattern_height) );

  /* Get state of widgets in radiation patterns window */
  rc_config.rdpattern_gain_togglebutton = 0;
  rc_config.rdpattern_eh_togglebutton = 0;
  rc_config.rdpattern_e_field = 0;
  rc_config.rdpattern_h_field = 0;
  rc_config.rdpattern_poynting_vector = 0;
  if( rdpattern_window )
  {
	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_gain_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.rdpattern_gain_togglebutton = 1;

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_eh_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.rdpattern_eh_togglebutton = 1;

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_e_field" );
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) )
	  rc_config.rdpattern_e_field = 1;

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_h_field" );
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) )
	  rc_config.rdpattern_h_field = 1;

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_poynting_vector" );
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) )
	  rc_config.rdpattern_poynting_vector = 1;

	widget = Builder_Get_Object(
		rdpattern_window_builder, "rdpattern_zoom_spinbutton" );
	gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
	rc_config.rdpattern_zoom_spinbutton =
	  gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );
  }

  /* Get geometry of frequency plots window */
  Get_Window_Geometry( freqplots_window,
	  &(rc_config.freqplots_x), &(rc_config.freqplots_y),
	  &(rc_config.freqplots_width), &(rc_config.freqplots_height) );

  /* Get state of widgets in frequency plots window */
  rc_config.freqplots_gmax_togglebutton    = 0;
  rc_config.freqplots_gdir_togglebutton    = 0;
  rc_config.freqplots_gviewer_togglebutton = 0;
  rc_config.freqplots_vswr_togglebutton    = 0;
  rc_config.freqplots_zrlzim_togglebutton  = 0;
  rc_config.freqplots_zmgzph_togglebutton  = 0;
  rc_config.freqplots_net_gain = 0;
  if( freqplots_window )
  {
	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_gmax_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_gmax_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_gdir_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_gdir_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_gviewer_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_gviewer_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_vswr_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_vswr_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_zrlzim_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_zrlzim_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_zmgzph_togglebutton" );
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
	  rc_config.freqplots_zmgzph_togglebutton = 1;

	widget = Builder_Get_Object(
		freqplots_window_builder, "freqplots_net_gain" );
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) )
	  rc_config.freqplots_net_gain = 1;
  }

  /* Get geometry of NEC2 editor window */
  Get_Window_Geometry( nec2_edit_window,
	  &(rc_config.nec2_edit_x), &(rc_config.nec2_edit_y),
	  &(rc_config.nec2_edit_width), &(rc_config.nec2_edit_height) );

} /* Get_GUI_State */

/*------------------------------------------------------------------------*/

/* Save_Config()
 *
 * Saves the current values in rc_config to xnec2c.conf
 */
  gboolean
Save_Config( void )
{
  FILE *fp = NULL;	/* File pointer to write config file */

  char
	rc_buf[RC_FILE_BUF_SIZE], /* Buffer for config file contents */
	err_str[128], /* Error messages string */
	cfg_file[64]; /* Path to config file */

  size_t fsize;	/* File size of config file */
  int ret_val;


  /* Print config values to buffer */
  snprintf( rc_buf, sizeof(rc_buf),
	  RC_CONFIG_FORMAT,
	  PACKAGE_STRING,
	  rc_config.working_dir,
	  rc_config.main_width,
	  rc_config.main_height,
	  rc_config.main_x,
	  rc_config.main_y,
	  rc_config.main_currents_togglebutton,
	  rc_config.main_charges_togglebutton,
	  rc_config.main_total,
	  rc_config.main_horizontal,
	  rc_config.main_vertical,
	  rc_config.main_right_hand,
	  rc_config.main_left_hand,
	  rc_config.main_loop_start,
	  rc_config.main_rotate_spinbutton,
	  rc_config.main_incline_spinbutton,
	  rc_config.main_zoom_spinbutton,
	  rc_config.rdpattern_width,
	  rc_config.rdpattern_height,
	  rc_config.rdpattern_x,
	  rc_config.rdpattern_y,
	  rc_config.rdpattern_gain_togglebutton,
	  rc_config.rdpattern_eh_togglebutton,
	  rc_config.rdpattern_e_field,
	  rc_config.rdpattern_h_field,
	  rc_config.rdpattern_poynting_vector,
	  rc_config.rdpattern_zoom_spinbutton,
	  rc_config.freqplots_width,
	  rc_config.freqplots_height,
	  rc_config.freqplots_x,
	  rc_config.freqplots_y,
	  rc_config.freqplots_gmax_togglebutton,
	  rc_config.freqplots_gdir_togglebutton,
	  rc_config.freqplots_gviewer_togglebutton,
	  rc_config.freqplots_vswr_togglebutton,
	  rc_config.freqplots_zrlzim_togglebutton,
	  rc_config.freqplots_zmgzph_togglebutton,
	  rc_config.freqplots_net_gain,
	  rc_config.nec2_edit_width,
	  rc_config.nec2_edit_height,
	  rc_config.nec2_edit_x,
	  rc_config.nec2_edit_y,
	  (int)structure_proj_params.dx_center,
	  (int)structure_proj_params.dy_center,
	  (int)rdpattern_proj_params.dx_center,
	  (int)rdpattern_proj_params.dy_center,
	  rc_config.confirm_quit );

  /* Setup file path to xnec2c working directory */
  snprintf( cfg_file, sizeof(cfg_file),
	  "%s/%s", getenv("HOME"), CONFIG_FILE );

  /* Open config file for writing */
  if( !Open_File( &fp, cfg_file, "w" ) )
  {
	snprintf( err_str, sizeof(err_str), "xnec2c: %s", cfg_file );
	perror( err_str );
	fprintf( stderr,
		_("xnec2c: cannot open xnec2c's config file: %s\n"), cfg_file );
	return( FALSE );
  }

  /* Write config buffer to file */
  fsize = strlen( rc_buf );
  ret_val = (int)fwrite( rc_buf, fsize, 1, fp );
  if( ret_val != 1 )
  {
	size_t s = sizeof( err_str );
	snprintf( err_str, s, "xnec2c: %s", cfg_file );
	err_str[s-1] = '\0';
	perror( err_str );
	fprintf( stderr,
		_("xnec2c: cannot write to xnec2c's config file: %s\n"), cfg_file );
	return( FALSE );
  }

  Close_File( &fp );

  return( TRUE );
} /* Save_Config() */

/*------------------------------------------------------------------------*/

