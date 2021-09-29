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

#include <ctype.h>
#include "rc_config.h"
#include "mathlib.h"


/* Add configuration options here. To add new variables:
 *    Add a new section and description
 *    Set the format string
 *    Create references to the .vars structure to load/save values
 */

rc_config_vars_t rc_config_vars[] = {
	{ .desc = "Application Vesrsion", .format = "%s", .ro = 1, 
		.vars = { PACKAGE_STRING } },

	{ .desc = "Current Working Directory", .format = "%s",
		.vars = { rc_config.working_dir }, .size = FILENAME_LEN },

	{ .desc = "Main Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.main_width, &rc_config.main_height } },

	{ .desc = "Main Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.main_x, &rc_config.main_y } },

	{ .desc = "Main Window Currents toggle button state", .format = "%d",
		.vars = { &rc_config.main_currents_togglebutton } },

	{ .desc = "Main Window Charges toggle button state", .format = "%d",
		.vars = { &rc_config.main_charges_togglebutton } },

	{ .desc = "Main Window Polarization menu total state", .format = "%d",
		.vars = { &rc_config.main_total } },

	{ .desc = "Main Window Polarization menu horizontal state", .format = "%d",
		.vars = { &rc_config.main_horizontal } },

	{ .desc = "Main Window Polarization menu vertical state", .format = "%d",
		.vars = { &rc_config.main_vertical } },

	{ .desc = "Main Window Polarization menu right hand state", .format = "%d",
		.vars = { &rc_config.main_right_hand } },

	{ .desc = "Main Window Polarization menu left hand state", .format = "%d",
		.vars = { &rc_config.main_left_hand } },

	{ .desc = "Main Window Frequency loop start state", .format = "%d",
		.vars = { &rc_config.main_loop_start } },

	{ .desc = "Main Window Rotate spinbutton state", .format = "%d",
		.vars = { &rc_config.main_rotate_spinbutton } },

	{ .desc = "Main Window Incline spinbutton state", .format = "%d",
		.vars = { &rc_config.main_incline_spinbutton } },

	{ .desc = "Main Window Zoom spinbutton state", .format = "%d",
		.vars = { &rc_config.main_zoom_spinbutton } },

	{ .desc = "Radiation Pattern Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.rdpattern_width, &rc_config.rdpattern_height } },

	{ .desc = "Radiation Pattern Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.rdpattern_x, &rc_config.rdpattern_y } },

	{ .desc = "Radiation Pattern Window Gain toggle button state", .format = "%d",
		.vars = { &rc_config.rdpattern_gain_togglebutton } },

	{ .desc = "Radiation Pattern Window EH toggle button state", .format = "%d",
		.vars = { &rc_config.rdpattern_eh_togglebutton } },

	{ .desc = "Radiation Pattern Window Menu E-field state", .format = "%d",
		.vars = { &rc_config.rdpattern_e_field } },

	{ .desc = "Radiation Pattern Window Menu H-field state", .format = "%d",
		.vars = { &rc_config.rdpattern_h_field } },

	{ .desc = "Radiation Pattern Window Menu Poynting vector state", .format = "%d",
		.vars = { &rc_config.rdpattern_poynting_vector } },

	{ .desc = "Radiation Pattern Window Zoom spinbutton state", .format = "%d",
		.vars = { &rc_config.rdpattern_zoom_spinbutton } },

	{ .desc = "Frequency Plots Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.freqplots_width, &rc_config.freqplots_height } },

	{ .desc = "Frequency Plots Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.freqplots_x, &rc_config.freqplots_y } },

	{ .desc = "Frequency Plots Window Max Gain toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gmax_togglebutton } },

	{ .desc = "Frequency Plots Window Gain Direction toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gdir_togglebutton } },

	{ .desc = "Frequency Plots Window Viewer Direction Gain toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gviewer_togglebutton } },

	{ .desc = "Frequency Plots Window VSWR toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_vswr_togglebutton } },

	{ .desc = "Frequency Plots Window Z-real/Z-imag toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_zrlzim_togglebutton } },

	{ .desc = "Frequency Plots Window Z-mag/Z-phase toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_zmgzph_togglebutton } },

	{ .desc = "Frequency Plots Window Smith toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_smith_togglebutton } },

	{ .desc = "Frequency Plots Window Net Gain checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_net_gain } },

	{ .desc = "Frequency Plots Window Min/Max checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_min_max } },

	{ .desc = "NEC2 Editor Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.nec2_edit_width, &rc_config.nec2_edit_height } },

	{ .desc = "NEC2 Editor Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.nec2_edit_x, &rc_config.nec2_edit_y } },

	{ .desc = "Structure Projection Center x and y Offset", .format = "%lf,%lf",
		.vars = { &structure_proj_params.dx_center, &structure_proj_params.dy_center } },

	{ .desc = "Rdpattern Projection Center x and y Offset", .format = "%lf,%lf",
		.vars = { &rdpattern_proj_params.dx_center, &rdpattern_proj_params.dy_center } },

	{ .desc = "Enable Confirm Quit Dialog", .format = "%d",
		.vars = { &rc_config.confirm_quit } },

	{ .desc = "Selected Mathlib", .format = "%d",
		.vars = { &rc_config.mathlib_idx }, .init = mathlib_config_init }
};


rc_config_vars_t rc_config_vars[];
int num_rc_config_vars = sizeof(rc_config_vars) / sizeof(rc_config_vars_t);


// Trim the newline
void chomp(char *line)
{
	int i;
	int len = strlen(line);

	for (i = 0; i < len; i++)
		if (line[i] == '\r' || line[i] == '\n')
		{
			line[i] = 0;
			break;
		}
}

// Find a variable defined by its comment
rc_config_vars_t *find_var(char *s)
{
	while (s[0] && (isspace(s[0]) || s[0] == '#'))
		s++;

	for (int i = 0; i < num_rc_config_vars; i++)
		if (!strcmp(rc_config_vars[i].desc, s))
			return &rc_config_vars[i];

	return NULL;
}

// Rease the line into the variable's .vars reference(s)
int parse_var(rc_config_vars_t *v, char *line)
{
	int count = 0;
	
	// Skip read-only or missing vars:
	if (v->ro || (v->vars[0] == NULL && v->vars[1] == NULL))
		return 0;

	if (strcmp(v->format, "%d") == 0)
		count = sscanf(line, v->format, (int*)v->vars[0]);
	else if (strcmp(v->format, "%s") == 0)
	{
		strncpy((char*)v->vars[0], line, v->size - 1);
		count = 1;
	}
	else if (strcmp(v->format, "%f") == 0)
		count = sscanf(line, v->format, (double*)v->vars[0]);
	else if (strcmp(v->format, "%d,%d") == 0)
		count = sscanf(line, v->format, (int*)v->vars[0], (int*)v->vars[1]);
	else if (strcmp(v->format, "%f,%f") == 0)
		count = sscanf(line, v->format, (float*)v->vars[0], (float*)v->vars[1]);
	else if (strcmp(v->format, "%lf,%lf") == 0)
		count = sscanf(line, v->format, (double*)v->vars[0], (double*)v->vars[1]);

	// `count` contains the number of vars parsed, return true
	// if the count matches the number of vars, otherwise it failed
	// to parse.  Make this a loop if you add support for more than
	// two vars being parsed (see struct rc_config_vars_t.vars[2]).
	return (v->vars[0] == NULL && count == 0)
		|| (v->vars[1] == NULL && count == 1)
		|| (v->vars[1] != NULL && count == 2);
}

// Print a var to a file descriptor based on its type:
int fprint_var(FILE *fp, rc_config_vars_t *v)
{
	int count = 0;
	
	if (v->vars[0] == NULL && v->vars[1] == NULL)
		return 0;

	if (strcmp(v->format, "%d") == 0)
		count = fprintf(fp, v->format, *(int*)v->vars[0]);
	else if (strcmp(v->format, "%s") == 0)
		count = fprintf(fp, v->format, (char*)v->vars[0]);
	else if (strcmp(v->format, "%f") == 0)
		count = fprintf(fp, v->format, *(double*)v->vars[0]);
	else if (strcmp(v->format, "%d,%d") == 0)
		count = fprintf(fp, v->format, *(int*)v->vars[0], *(int*)v->vars[1]);
	else if (strcmp(v->format, "%f,%f") == 0)
		count = fprintf(fp, v->format, *(float*)v->vars[0], *(float*)v->vars[1]);
	else if (strcmp(v->format, "%lf,%lf") == 0)
		count = fprintf(fp, v->format, *(double*)v->vars[0], *(double*)v->vars[1]);

	return count;
}

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
 * Provide a warning if the version changes.
 * Set defaults, they will be saved later.  Read_Config() is called after 
 * this function so it will override any settings if they are available in the config
 * and save a new file if it is missing using the defaults defined below.
 */
  gboolean
Create_Default_Config( void )
{
  char
    line[LINE_LEN],
    cfg_file[FILENAME_LEN];   /* Path to config file */

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
      printf( _("xnec2c: warning: existing config file version differs: %s != %s\n"), line, PACKAGE_STRING );

    Close_File( &fp );
  } /* if( (fp = fopen(cfg_file, "r")) != NULL ) */

  /* For main window */
  Strlcpy( rc_config.working_dir, getenv("HOME"), sizeof(rc_config.working_dir) );
  Strlcat( rc_config.working_dir, "/", sizeof(rc_config.working_dir) );
  rc_config.main_width  = 600;
  rc_config.main_height = 400;
  rc_config.main_x = 50;
  rc_config.main_y = 50;
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
  rc_config.freqplots_smith_togglebutton   = 0;
  rc_config.freqplots_net_gain = 0;
  rc_config.freqplots_min_max = 0;

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
  /* gtk_widget_hide( window ); this leads to an undecorated window in icewm */
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

  /* Open frequency plots window if state data available */
  if( rc_config.freqplots_width &&
      rc_config.freqplots_height )
  {
    widget = Builder_Get_Object( main_window_builder, "main_freqplots" );
    gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  /* Open radiation pattern window if state data available */
  if( rc_config.rdpattern_width &&
      rc_config.rdpattern_height )
  {
    widget = Builder_Get_Object( main_window_builder, "main_rdpattern" );
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
    fpath[FILENAME_LEN], /* File path to xnec2crc */
    line[LINE_LEN];
  int lnum;

  /* Config and mnemonics file pointer */
  FILE *fp = NULL;

  /* Setup file path to xnec2c rc file */
  snprintf( fpath, sizeof(fpath), "%s/%s", getenv("HOME"), CONFIG_FILE );

  /* Create the file if missing */
  if( access(fpath, R_OK) < 0 && errno == ENOENT)
	  Save_Config();

  /* Open xnec2c config file */
  if( !Open_File(&fp, fpath, "r") ) return( FALSE );

  // Iterate over each line and parse the variables into
  // their references defined by rc_config_vars[].
  lnum = 0;
  while ( fgets(line, LINE_LEN, fp) != NULL)
  {
	  lnum++;
      
	  chomp(line);

	  rc_config_vars_t *v = find_var(line);
	  if (!v)
	  {
		  if (line[0] != '#')
			  printf("%s:%d: Line not parsed: %s\n", fpath, lnum, line);
		  continue;
	  }
	  
	  if ( fgets(line, LINE_LEN, fp) == NULL)
	  {
		  printf("%s:%d: Early end of file for %s: %s \n", fpath, lnum, v->desc, line);
		  break;
	  }

	  lnum++;

	  // Skip read-only vars:
	  if (v->ro)
		  continue;

	  chomp(line);

	  if (!parse_var(v, line) && line[0] != '#')
		  printf("%s:%d: parse error (%s): %s \n", fpath, lnum, v->desc, line);
	  else if (v->init != NULL)
		  v->init(v);
  }

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
  rc_config.freqplots_smith_togglebutton   = 0;
  rc_config.freqplots_net_gain = 0;
  rc_config.freqplots_min_max = 0;
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

    widget = Builder_Get_Object(
        freqplots_window_builder, "freqplots_min_max" );
    if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) )
      rc_config.freqplots_min_max = 1;

    widget = Builder_Get_Object(
        freqplots_window_builder, "freqplots_smith_togglebutton" );
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) )
      rc_config.freqplots_smith_togglebutton = 1;
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
  FILE *fp = NULL;  /* File pointer to write config file */

  char
    err_str[300],             /* Error messages string */
    cfg_file[FILENAME_LEN];   /* Path to config file */

  /* Setup file path to xnec2c working directory */
  snprintf( cfg_file, sizeof(cfg_file), "%s/%s", getenv("HOME"), CONFIG_FILE );

  /* Open config file for writing */
  if( !Open_File( &fp, cfg_file, "w" ) )
  {
    snprintf( err_str, sizeof(err_str), "xnec2c: %s", cfg_file );
    perror( err_str );
    fprintf( stderr,
        _("xnec2c: cannot open xnec2c's config file: %s\n"), cfg_file );
    return( FALSE );
  }

  fprintf(fp, "# Xnec2c configuration file\n#\n");
  for (int i = 0; i < num_rc_config_vars; i++)
  {
	  fprintf(fp, "# %s\n", rc_config_vars[i].desc);
	  fprint_var(fp, &rc_config_vars[i]);
	  fprintf(fp, "\n");
  }

  Close_File( &fp );

  return( TRUE );
} /* Save_Config() */

/*------------------------------------------------------------------------*/

