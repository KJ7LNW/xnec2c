/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "fork.h"
#include "xnec2c.h"
#include "editors.h"
#include "nec2_model.h"
#include "shared.h"
#include <gdk/gdkkeysyms.h>

/* File chooser/select widgets */
static GtkWidget *file_chooser   = NULL;
static GtkWidget *file_selection = NULL;

/* Tree view clicked on by user */
static GtkTreeView *selected_treeview = NULL;

/* Pixmap for saving drawable images */
static GdkPixmap *saveas_pixmap = NULL;
static int saveas_pixmap_width, saveas_pixmap_height;

/* Action flag for NEC2 "card" editors */
static int action = EDITOR_NEW;

/*-----------------------------------------------------------------------*/

  void
on_main_window_destroy                 (GtkObject       *object,
										gpointer         user_data)
{
  Gtk_Quit();
}


  gboolean
on_main_window_delete_event            (GtkWidget       *widget,
										GdkEvent        *event,
										gpointer         user_data)
{
  kill_window = main_window;
  SetFlag( MAIN_QUIT );
  Delete_Event( _("Really quit xnec2c?") );
  return TRUE;
}


  void
on_new_activate     	              (GtkMenuItem     *menuitem,
									   gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
		_("A new NEC2 input file may not be created\n"\
		  "while the Frequency Loop is running") ) )
	return;

  /* Save open file, if any */
  if( (input_fp != NULL) && (nec2_edit_window != NULL) )
  {
	stop( _("Saving already open NEC2 file\n"\
		  "before opening new default file"), ERR_OK );
	Save_Nec2_Input_File( nec2_edit_window, infile );
	infile[0] = '\0';
  }

  /* Open editor window if needed */
  if( nec2_edit_window == NULL )
  {
	Close_File( &input_fp );
	infile[0] = '\0';
	Open_Nec2_Editor( NEC2_EDITOR_NEW );
  }
  else Nec2_Input_File_Treeview( NEC2_EDITOR_NEW );
}


  void
on_open_input_activate     		       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
		_("A new NEC2 input file may not be opened\n"\
		  "while the Frequency Loop is running")) )
	return;

  /* Open file chooser to select a NEC2 input file */
  file_chooser = create_filechooserdialog();
  gtk_widget_show( file_chooser );

  /* Set filechooser file name to previously selected, if any */
  if( (strlen(infile) != 0) && (infile[0] == '/') )
	gtk_file_chooser_set_filename(
		GTK_FILE_CHOOSER(file_chooser), infile );
}


  void
on_main_save_activate        (GtkMenuItem     *menuitem,
							  gpointer         user_data)
{
  char saveas[96];
  size_t s = sizeof( saveas );

  /* Count number of structure image files saved of geometry,
   * currents or charges, to avoid over-writing saved files */
  static int cgm = 0, ccr = 0, cch = 0;

  if( strlen(infile) == 0 )
	return;

  /* Make the structure image save file name from input file
   * name. The count of each image type saved is incremented */
  if( isFlagSet(DRAW_CURRENTS) )
	snprintf( saveas, s, "%s-%s_%03d.%s", infile, "current", ++ccr, "png" );
  else if( isFlagSet(DRAW_CHARGES) )
	snprintf( saveas, s, "%s-%s_%03d.%s", infile, "charge", ++cch, "png" );
  else
	snprintf( saveas, s, "%s-%s_%03d.%s", infile, "geometry", ++cgm, "png" );

  saveas_pixmap = structure_pixmap;
  saveas_pixmap_width  = structure_pixmap_width;
  saveas_pixmap_height = structure_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify file  */
  /* name for saving the structure image */
  file_selection = create_fileselection();
  gtk_file_selection_set_filename( GTK_FILE_SELECTION(file_selection), saveas );
  gtk_widget_show( file_selection );
}


  void
on_main_save_as_activate               (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  saveas_pixmap = structure_pixmap;
  saveas_pixmap_width  = structure_pixmap_width;
  saveas_pixmap_height = structure_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify file  */
  /* name for saving the structure image */
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


  void
on_struct_save_as_gnuplot_activate     (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Open file selector to specify a file
   * name for saving the rad pattern image */
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( STRCT_GNUPLOT_SAVE );
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


  void
on_quit_activate                       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  kill_window = main_window;
  SetFlag( MAIN_QUIT );
  Delete_Event( _("Really quit xnec2c?") );
}


  void
on_main_rdpattern_activate             (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Open radiation pattern rendering window */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	rdpattern_window = create_rdpattern_window();
	gtk_widget_show( rdpattern_window );
	rdpattern_drawingarea = lookup_widget(
		rdpattern_window, "rdpattern_drawingarea" );
	rdpattern_motion_handler = g_signal_connect (
		(gpointer) rdpattern_drawingarea,
		"motion_notify_event",
		G_CALLBACK (on_rdpattern_drawingarea_motion_notify_event),
		NULL);
	gtk_widget_add_events(
		GTK_WIDGET(rdpattern_drawingarea),
		GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK );
	rotate_rdpattern  = GTK_SPIN_BUTTON(lookup_widget(
		  rdpattern_window, "rdpattern_rotate_spinbutton"));
	incline_rdpattern = GTK_SPIN_BUTTON(lookup_widget(
		  rdpattern_window, "rdpattern_incline_spinbutton"));
	rdpattern_frequency = GTK_SPIN_BUTTON(lookup_widget(
		  rdpattern_window, "rdpattern_freq_spinbutton"));
	rdpattern_zoom = GTK_SPIN_BUTTON(lookup_widget(
		  rdpattern_window, "rdpattern_zoom_spinbutton"));
	rdpattern_fstep_entry = GTK_ENTRY(lookup_widget(
		  rdpattern_window, "rdpattern_fstep_entry")) ;

	Main_Rdpattern_Activate( TRUE );
  }
  else if( isFlagSet(DRAW_ENABLED) )
	  gtk_widget_destroy( rdpattern_window );
}


  void
on_main_freqplots_activate             (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Open window for plotting frequency
   * related data (gain, vswr etc) */
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
	if( Main_Freqplots_Activate() )
	{
	  freqplots_window = create_freqplots_window();
	  gtk_widget_show( freqplots_window );
	  freqplots_drawingarea = lookup_widget(
		  freqplots_window, "freqplots_drawingarea" );
	  Set_Window_Labels();
	  calc_data.ngraph = 0;
	} /* if( Main_Freqplots_Activate() */
	else gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(menuitem), FALSE );
  }
  else if( isFlagSet(PLOT_ENABLED) )
	gtk_widget_destroy( freqplots_window );
}


  void
on_rdpattern_total_activate            (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Polarization( POL_TOTAL );
}


  void
on_rdpattern_horizontal_activate       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Polarization( POL_HORIZ );
}


  void
on_rdpattern_vertical_activate         (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Polarization( POL_VERT );
}


  void
on_rdpattern_right_hand_activate       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Polarization( POL_RHCP );
}


  void
on_rdpattern_left_hand_activate        (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Polarization( POL_LHCP );
}


  void
on_common_projection_activate          (GtkMenuItem     *menuitem,
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
on_common_freq_activate                (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Enable syncing of frequency spinbuttons
   * between main and rad pattern windows */
  if( gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( COMMON_FREQUENCY );
  else
	ClearFlag( COMMON_FREQUENCY );
}


  void
on_main_x_axis_clicked                 (GtkButton       *button,
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
on_main_y_axis_clicked                 (GtkButton       *button,
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
on_main_z_axis_clicked                 (GtkButton       *button,
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
on_main_default_view_clicked           (GtkButton       *button,
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
on_main_currents_togglebutton_toggled  (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  /* Enable calculation and rendering of structure curents */
  Main_Currents_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  void
on_main_charges_togglebutton_toggled   (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  /* Enable calculation and rendering of structure charge density */
  Main_Charges_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


gboolean
on_main_colorcode_drawingarea_expose_event(	GtkWidget       *widget,
											GdkEventExpose  *event,
											gpointer         user_data)
{
  double red = 0.0, grn = 0.0, blu = 0.0;
  int idx;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  cairo_t *cr = gdk_cairo_create( widget-> window );

  /* Draw color-code bar in main window */
  for( idx = 0; idx < 160; idx++ )
  {
	Value_to_Color( &red, &grn, &blu, (double) (8*idx), 1280.0 );
	cairo_set_source_rgb( cr, red, grn, blu );
	Cairo_Draw_Line( cr, idx, 0, idx, 28 );
  }

  cairo_destroy( cr );
  return TRUE;
}


  void
on_main_freq_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
										gpointer         user_data)
{
  static gdouble fmhz_save = 0.0;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Frequency spinbutton value changed by frequency loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) )
	Draw_Structure( structure_drawingarea );
  else /* by user */
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
	if( isFlagSet(DRAW_ENABLED) 	&&
		isFlagSet(COMMON_FREQUENCY) &&
		isFlagSet(MAIN_NEW_FREQ) )
	  /* Show current frequency */
	  gtk_spin_button_set_value( rdpattern_frequency, fmhz );

  } /* else */

  gtk_spin_button_update( spinbutton );
}


void
on_main_freq_checkbutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	SetFlag(MAIN_NEW_FREQ);
  else
	ClearFlag(MAIN_NEW_FREQ);
}


  void
on_main_new_freq_clicked               (GtkButton       *button,
										gpointer         user_data)
{
  /* Recalculate (and redraw) currents on user command */
  if( isFlagClear(FREQ_LOOP_RUNNING) )
  {
	calc_data.fmhz =
	  (double)gtk_spin_button_get_value( mainwin_frequency );
	g_idle_add( Redo_Currents, NULL );
  }
}


gboolean
on_structure_drawingarea_configure_event( GtkWidget       *widget,
										  GdkEventConfigure *event,
										  gpointer         user_data)
{
  /* Create or resize structure drawing pixmap */
  Create_Pixmap(
	  &structure_pixmap,
	  &structure_pixmap_width,
	  &structure_pixmap_height,
	  widget, event,
	  &structure_proj_params );
  return TRUE;
}


gboolean
on_structure_drawingarea_motion_notify_event( GtkWidget       *widget,
											  GdkEventMotion  *event,
											  gpointer         user_data)
{
  /* Use only 1 in MOTION_EVENTS_COUNT event */
  static int cnt = 0;
  if( cnt++ < MOTION_EVENTS_COUNT )
	return FALSE;
  cnt = 0;

  /* Block motion events */
  g_signal_handler_block
	( (gpointer)structure_drawingarea, structure_motion_handler );

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  /* Handle motion events */
  Motion_Event( event, &structure_proj_params );

  /* Unblock motion events */
  g_signal_handler_unblock(
	  (gpointer)structure_drawingarea, structure_motion_handler );

  return TRUE;
}


  gboolean
on_structure_drawingarea_expose_event  (GtkWidget       *widget,
										GdkEventExpose  *event,
										gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  Draw_Structure( widget );
  return TRUE;
}


  void
on_filechooserdialog_response          (GtkDialog       *dialog,
										gint             response_id,
										gpointer         user_data)
{
  if( response_id == GTK_RESPONSE_OK )
  {
	gchar *filename;
	gboolean new = TRUE;

	/* Save any changes to an open file */
	Save_Nec2_Input_File( nec2_edit_window, infile );

	/* Get filename of NEC2 input file */
	filename =
	  gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(dialog) );
	Strlcpy( infile, filename, sizeof(infile) );
	g_free( filename );
	gtk_widget_destroy( file_chooser );

	/* Open new file */
	Open_Input_File( (gpointer)&new );
  }
}


  void
on_fileselection_response(  GtkDialog       *dialog,
							gint             response_id,
							gpointer         user_data)
{
  gchar filename[81];
  size_t s = sizeof( filename );
  char *str;
  gboolean new = FALSE;

  /* User selects a file name to save a pixmap to file */
  if( response_id == GTK_RESPONSE_OK )
  {
	/* Get the "save as" file name */
	Strlcpy( filename,
		gtk_file_selection_get_filename(
		  GTK_FILE_SELECTION(dialog)), s );

	if( isFlagSet(IMAGE_SAVE) )
	{
	  /* cat a file extension if not already there */
	  str = strstr( filename, ".png" );
	  if( (str == NULL) || (str[4] != '\0') )
		Strlcat( filename, ".png", s );
	  Save_Pixmap(
		  saveas_pixmap, saveas_pixmap_width,
		  saveas_pixmap_height, filename );
	}
	else if( isFlagSet(NEC2_SAVE) )
	{
	  /* cat a file extension if not already there */
	  str = strstr( filename, ".nec" );
	  if( (str == NULL) || (str[4] != '\0') )
		Strlcat( filename, ".nec", s );

	  /* Use new file name as input file */
	  Strlcpy( infile, filename, sizeof(infile) );
	  Save_Nec2_Input_File( nec2_edit_window, filename );

	  /* Re-open NEC2 input file */
	  if( Nec2_Apply_Checkbutton() )
		Open_Input_File( (gpointer)&new );
	}
	else if( isFlagSet(RDPAT_GNUPLOT_SAVE) )
	{
	  /* cat a file extension if not already there */
	  str = strstr( filename, ".gplot" );
	  if( (str == NULL) || (str[6] != '\0') )
		Strlcat( filename, ".gplot", s );
	  Save_RadPattern_Gnuplot_Data( filename );
	}
	else if( isFlagSet(PLOTS_GNUPLOT_SAVE) )
	{
	  /* cat a file extension if not already there */
	  str = strstr( filename, ".gplot" );
	  if( (str == NULL) || (str[6] != '\0') )
		Strlcat( filename, ".gplot", s );
	  Save_FreqPlots_Gnuplot_Data( filename );
	}
	else if( isFlagSet(STRCT_GNUPLOT_SAVE) )
	{
	  /* cat a file extension if not already there */
	  str = strstr( filename, ".gplot" );
	  if( (str == NULL) || (str[6] != '\0') )
		Strlcat( filename, ".gplot", s );
	  Save_Struct_Gnuplot_Data( filename );
	}

	gtk_widget_destroy( file_selection );
	if( (kill_window == nec2_edit_window) &&
		(kill_window != NULL) )
	  gtk_widget_destroy( nec2_edit_window );
  } /* if( response_id == GTK_RESPONSE_OK ) */
}


  gboolean
on_freqplots_window_delete_event       (GtkWidget       *widget,
										GdkEvent        *event,
										gpointer         user_data)
{
  kill_window = freqplots_window;
  Delete_Event( _("Really close window?") );
  return TRUE;
}


  void
on_freqplots_window_destroy            (GtkObject       *object,
										gpointer         user_data)
{
  Plots_Window_Killed();
}


  void
on_freqplots_save_activate             (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  char saveas[96];
  size_t s = sizeof( saveas );
  static int cnt = 0;

  if( (strlen(infile) == 0) || isFlagClear(PLOT_SELECT) )
	return;

  /* Make file name from input file name,
   * to save frequency plots drawing */
  snprintf( saveas, s, "%s-%s_%03d.%s", infile, "plots", ++cnt, "png" );
  saveas_pixmap = freqplots_pixmap;
  saveas_pixmap_width  = freqplots_pixmap_width;
  saveas_pixmap_height = freqplots_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify file  */
  /* name for saving the structure image */
  file_selection = create_fileselection();
  gtk_file_selection_set_filename( GTK_FILE_SELECTION(file_selection), saveas );
  gtk_widget_show( file_selection );
}


  void
on_freqplots_save_as_activate          (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  saveas_pixmap = freqplots_pixmap;
  saveas_pixmap_width  = freqplots_pixmap_width;
  saveas_pixmap_height = freqplots_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify a file
   * name for saving the freq plots image */
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


void
on_freqplots_save_as_gnuplot_activate  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  /* Open file selector to specify a file
   * name for saving the rad pattern image */
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( PLOTS_GNUPLOT_SAVE );
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


  void
on_freqplots_gmax_togglebutton_toggled (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  /* Enable or not max gain plotting */
  Plot_Select( togglebutton, PLOT_GMAX );
}


  void
on_freqplots_gdir_togglebutton_toggled (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  /* Enable or not gain direction plotting */
  Plot_Select( togglebutton, PLOT_GAIN_DIR );
}


void
on_freqplots_gviewer_togglebutton_toggled(GtkToggleButton *togglebutton,
										  gpointer         user_data)
{
  /* Enable or not "gain toward user" plotting */
  Plot_Select( togglebutton, PLOT_GVIEWER );
}


  void
on_freqplots_vswr_togglebutton_toggled (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  /* Enable or not VSWR plotting */
  Plot_Select( togglebutton, PLOT_VSWR );
}


void
on_freqplots_zo_spinbutton_value_changed( GtkSpinButton   *spinbutton,
										  gpointer         user_data)
{
  /* Set the value of Z0 used for VSWR calculations */
  calc_data.zo = gtk_spin_button_get_value(spinbutton);
  if( isFlagSet(PLOT_ENABLED) )
	Plot_Frequency_Data();
  gtk_spin_button_update( spinbutton );
}


void
on_freqplots_zrlzim_togglebutton_toggled( GtkToggleButton *togglebutton,
										  gpointer         user_data)
{
  /* Enable or not Z-real/Z-imag plotting */
  Plot_Select( togglebutton, PLOT_ZREAL_ZIMAG );
}


void
on_freqplots_zmgzph_togglebutton_toggled( GtkToggleButton *togglebutton,
										  gpointer         user_data)
{
  /* Enable or not Z-mag/Z-phase plotting */
  Plot_Select( togglebutton, PLOT_ZMAG_ZPHASE );
}


  gboolean
on_freqplots_drawingarea_expose_event  (GtkWidget       *widget,
										GdkEventExpose  *event,
										gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  /* Enable drawing of freq line */
  Plot_Frequency_Data();
  return TRUE;
}


gboolean
on_freqplots_drawingarea_configure_event( GtkWidget       *widget,
										  GdkEventConfigure *event,
										  gpointer         user_data)
{
  /* Create or resize pixmap for frequency plots */
  Create_Pixmap(
	  &freqplots_pixmap,
	  &freqplots_pixmap_width,
	  &freqplots_pixmap_height,
	  widget, event, NULL );
  return TRUE;
}


gboolean
on_freqplots_drawingarea_button_press_event(GtkWidget       *widget,
											GdkEventButton  *event,
											gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  Set_Frequency_On_Click( event );
  return TRUE;
}


  void
on_rdpattern_window_destroy            (GtkObject       *object,
										gpointer         user_data)
{
  Rdpattern_Window_Killed();
}


  gboolean
on_rdpattern_window_delete_event       (GtkWidget       *widget,
										GdkEvent        *event,
										gpointer         user_data)
{
  kill_window = rdpattern_window;
  Delete_Event( _("Really close window?") );
  return TRUE;
}


  void
on_rdpattern_save_activate             (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  char saveas[96];
  size_t s = sizeof( saveas );
  static int cgn = 0, ceh = 0;;

  if( strlen(infile) == 0 )
	return;

  /* Make the rad pattern save
   * file name from input name */
  if( isFlagSet(DRAW_GAIN) )
	snprintf( saveas, s, "%s-%s_%03d.%s", infile, "gain", ++cgn, "png" );
  else if( isFlagSet(DRAW_EHFIELD) )
	snprintf( saveas, s, "%s-%s_%03d.%s", infile, "fields", ++ceh, "png" );
  else return;

  saveas_pixmap = rdpattern_pixmap;
  saveas_pixmap_width  = rdpattern_pixmap_width;
  saveas_pixmap_height = rdpattern_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify file  */
  /* name for saving the pattern image */
  file_selection = create_fileselection();
  gtk_file_selection_set_filename( GTK_FILE_SELECTION(file_selection), saveas );
  gtk_widget_show( file_selection );
}


  void
on_rdpattern_save_as_activate          (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  saveas_pixmap = rdpattern_pixmap;
  saveas_pixmap_width  = rdpattern_pixmap_width;
  saveas_pixmap_height = rdpattern_pixmap_height;
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( IMAGE_SAVE );

  /* Open file selector to specify a file
   * name for saving the rad pattern image */
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


void
on_rdpattern_save_as_gnuplot_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  /* Open file selector to specify a file
   * name for saving the rad pattern image */
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( RDPAT_GNUPLOT_SAVE );
  file_selection = create_fileselection();
  gtk_widget_show( file_selection );
}


void
on_rdpattern_linear_power_activate     (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	Set_Gain_Style( GS_LINP );
}


  void
on_rdpattern_linear_voltage_activate   (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	Set_Gain_Style( GS_LINV );
}


  void
on_rdpattern_arrl_style_activate       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	Set_Gain_Style( GS_ARRL );
}


  void
on_rdpattern_logarithmic_activate      (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	Set_Gain_Style( GS_LOG );
}


  void
on_rdpattern_e_field_activate          (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_EFIELD );
  else
	ClearFlag( DRAW_EFIELD );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
	Draw_Radiation( rdpattern_drawingarea );
}


  void
on_rdpattern_h_field_activate          (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_HFIELD );
  else
	ClearFlag( DRAW_HFIELD );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
	Draw_Radiation( rdpattern_drawingarea );
}


  void
on_rdpattern_poynting_vector_activate  (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( DRAW_POYNTING );
  else
	ClearFlag( DRAW_POYNTING );
  Set_Window_Labels();
  if( isFlagSet(DRAW_EHFIELD) )
	Draw_Radiation( rdpattern_drawingarea );
}


  void
on_rdpattern_overlay_structure_activate (GtkMenuItem     *menuitem,
	gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( OVERLAY_STRUCT );
  else
	ClearFlag( OVERLAY_STRUCT );
  Draw_Radiation( rdpattern_drawingarea );
}


  void
on_rdpattern_x_axis_clicked            (GtkButton       *button,
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
on_rdpattern_y_axis_clicked            (GtkButton       *button,
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
on_rdpattern_z_axis_clicked            (GtkButton       *button,
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
on_rdpattern_default_view_clicked
  (GtkButton       *button,
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
on_rdpattern_gain_togglebutton_toggled (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  Rdpattern_Gain_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  void
on_rdpattern_eh_togglebutton_toggled   (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  Rdpattern_EH_Togglebutton_Toggled(
	  gtk_toggle_button_get_active(togglebutton) );
}


  gboolean
on_rdpattern_colorcode_drawingarea_expose_event(GtkWidget       *widget,
												GdkEventExpose  *event,
												gpointer         user_data)
{
  double red = 0.0, grn = 0.0, blu = 0.0;
  int idx;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  cairo_t *cr = gdk_cairo_create( widget-> window );

  /* Draw color code bar in rad pattern window */
  for( idx = 0; idx < 160; idx++ )
  {
	Value_to_Color( &red, &grn, &blu, (double) (8*idx), 1280.0 );
	cairo_set_source_rgb( cr, red, grn, blu );
	Cairo_Draw_Line( cr, idx, 0, idx, 28 );
  }

  cairo_destroy( cr );
  return TRUE;
}


void
on_rdpattern_freq_spinbutton_value_changed(	GtkSpinButton   *spinbutton,
											gpointer         user_data)
{
  static gdouble fmhz_save = 0.0;

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return;

  /* Frequency spinbutton value changed by frequency loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) && isFlagSet(DRAW_ENABLED) )
	Draw_Radiation( rdpattern_drawingarea );
  else
  {
	/* Get freq from spin button, avoid double signal by GTK */
	gdouble fmhz = (gdouble)gtk_spin_button_get_value(spinbutton);
	if( fmhz == fmhz_save ) return; /* to avoid double signal by GTK */
	fmhz_save = fmhz;

	/* If new freq calculations are enabled
	 * by checkbutton next to freq spinbutton */
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
			lookup_widget(rdpattern_window,
			  "rdpattern_freq_checkbutton"))) &&
		(isFlagSet(DRAW_GAIN) || isFlagSet(DRAW_EHFIELD)) )
	{
	  /* Recalc currents in structure and rad pattern */
	  calc_data.fmhz = (double)fmhz;
	  g_idle_add( Redo_Radiation_Pattern, NULL );
	}

	/* Sync main window frequency spinbutton */
	if( isFlagSet(COMMON_FREQUENCY) )
	  gtk_spin_button_set_value( mainwin_frequency, fmhz );
  } /* else */
  gtk_spin_button_update( spinbutton );
}


  void
on_rdpattern_new_freq_clicked          (GtkButton       *button,
										gpointer         user_data)
{
  /* Recalculate and draw rad pattern after user command */
  if( isFlagClear(FREQ_LOOP_RUNNING) )
  {
	calc_data.fmhz =
	  (double)gtk_spin_button_get_value( rdpattern_frequency );
	Redo_Radiation_Pattern( NULL );
  }
}


gboolean
on_rdpattern_drawingarea_configure_event( GtkWidget       *widget,
										  GdkEventConfigure *event,
										  gpointer         user_data)
{
  /* Create or resize rad pattern pixmap */
  Create_Pixmap(
	  &rdpattern_pixmap,
	  &rdpattern_pixmap_width,
	  &rdpattern_pixmap_height,
	  widget, event,
	  &rdpattern_proj_params );
  return TRUE;
}


  gboolean
on_rdpattern_drawingarea_expose_event  (GtkWidget       *widget,
										GdkEventExpose  *event,
										gpointer         user_data)
{
  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  Draw_Radiation( widget );
  return TRUE;
}


gboolean
on_rdpattern_drawingarea_motion_notify_event( GtkWidget       *widget,
											  GdkEventMotion  *event,
											  gpointer         user_data)
{
  /* Use only 1 in MOTION_EVENTS_COUNT event */
  static int cnt = 0;
  if( cnt++ < MOTION_EVENTS_COUNT )
	return FALSE;
  cnt = 0;

  /* Block motion events */
  g_signal_handler_block(
	  (gpointer)rdpattern_drawingarea, rdpattern_motion_handler );

  /* No redraws if new input pending */
  if( isFlagSet(INPUT_PENDING) )
	return FALSE;

  /* Handle motion events */
  Motion_Event( event, &rdpattern_proj_params );

  /* Unblock motion events */
  g_signal_handler_unblock(
	  (gpointer)rdpattern_drawingarea, rdpattern_motion_handler );

  return TRUE;
}


  void
on_quit_cancelbutton_clicked           (GtkButton       *button,
										gpointer         user_data)
{
  gtk_widget_destroy( quit_dialog );
  ClearFlag( MAIN_QUIT );
}


  void
on_quit_okbutton_clicked               (GtkButton       *button,
										gpointer         user_data)
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	if( isFlagSet(MAIN_QUIT) )
	{
	  Stop_Frequency_Loop();
	  gtk_label_set_text( GTK_LABEL(
			lookup_widget( quit_dialog, "quit_label")),
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

  gtk_widget_destroy( quit_dialog );
  gtk_widget_destroy( kill_window );
}


  void
main_view_menuitem_activate            (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Sync common projection checkbuttons */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(main_window,
			"common_projection")), TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(main_window,
			"common_projection")), FALSE );

  /* Sync common frequency checkbuttons */
  if( isFlagSet(COMMON_FREQUENCY) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(main_window,
			"common_freq")), TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(main_window,
			"common_freq")), FALSE );

}


  void
main_pol_menu_activate                 (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Pol_Menuitem( menuitem );
}


  void
freqplots_pol_menu_activate            (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Pol_Menuitem( menuitem );
}


  void
rdpattern_view_menuitem_activate       (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Sync common projection checkbuttons */
  if( isFlagSet(COMMON_PROJECTION) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"common_projection")), TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"common_projection")), FALSE );

  /* Sync common frequency checkbuttons */
  if( isFlagSet(COMMON_FREQUENCY) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"common_freq")), TRUE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"common_freq")), FALSE );
}


  void
rdpattern_pol_menu_activate            (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  Set_Pol_Menuitem( menuitem );
}


  void
on_filechoser_cancel_clicked           (GtkButton       *button,
										gpointer         user_data)
{
  gtk_widget_destroy( file_chooser );
}


  void
on_fileselection_cancel_clicked        (GtkButton       *button,
										gpointer         user_data)
{
  gtk_widget_destroy( file_selection );
}


void
on_near_peak_value_activate            (GtkMenuItem     *menuitem,
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
	  Draw_Radiation( rdpattern_drawingarea );
	}
  }
  else SetFlag( NEAREH_SNAPSHOT );

  /* Reset child near field flags */
  Pass_EH_Flags();
}


void
on_near_snapshot_activate              (GtkMenuItem     *menuitem,
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
	  Draw_Radiation( rdpattern_drawingarea );
	}
  }
  else ClearFlag( NEAREH_SNAPSHOT );

  /* Reset child near field flags */
  Pass_EH_Flags();
}


  void
on_rdpattern_animate_activate          (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( isFlagClear(DRAW_EHFIELD) )
	return;

  if( animate_dialog == NULL )
	animate_dialog = create_animate_dialog();
  gtk_widget_show( animate_dialog );
}


  void
on_animation_applybutton_clicked       (GtkButton       *button,
										gpointer         user_data)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, steps;

  spinbutton = GTK_SPIN_BUTTON( lookup_widget(animate_dialog,
		"animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON( lookup_widget(animate_dialog,
		"animate_steps_spinbutton") );
  steps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / steps / freq);
  near_field.anim_step = (double)TP / steps;

  SetFlag( NEAREH_ANIMATE );
  if( anim_tag > 0 )
	g_source_remove( anim_tag );
  anim_tag = g_timeout_add( intval, Animate_Near_Field, NULL );
}


  void
on_animation_cancelbutton_clicked      (GtkButton       *button,
									  	gpointer         user_data)
{
  ClearFlag( NEAREH_ANIMATE );
  g_source_remove( anim_tag );
  anim_tag = 0;
}


  void
on_animation_okbutton_clicked          (GtkButton       *button,
										gpointer         user_data)
{
  GtkSpinButton *spinbutton;
  guint intval;
  gdouble freq, steps;

  spinbutton = GTK_SPIN_BUTTON( lookup_widget(animate_dialog,
		"animate_freq_spinbutton") );
  freq = gtk_spin_button_get_value( spinbutton );
  spinbutton = GTK_SPIN_BUTTON( lookup_widget(animate_dialog,
		"animate_steps_spinbutton") );
  steps = gtk_spin_button_get_value( spinbutton );
  intval = (guint)(1000.0 / steps / freq);
  near_field.anim_step = (double)TP / steps;

  SetFlag( NEAREH_ANIMATE );
  if( anim_tag > 0 )
	g_source_remove( anim_tag );
  anim_tag = g_timeout_add( intval, Animate_Near_Field, NULL );

  gtk_widget_hide( animate_dialog );
}


  void
on_animate_dialog_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  animate_dialog = NULL;
}


void
on_quit_dialog_destroy                 (GtkObject       *object,
                                        gpointer         user_data)
{
  quit_dialog = NULL;
}


gboolean
on_error_dialog_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  return TRUE;
}


  void
on_nec2_edit_activate                  (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  /* Abort if no open input file */
  if( input_fp == NULL )
  {
	stop( _("No open NEC2 input file"), ERR_OK );
	return;
  }

  if( nec2_edit_window == NULL )
	Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
}


gboolean
on_nec2_editor_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  if( isFlagSet(NEC2_EDIT_SAVE) )
  {
	/* Open file selector to specify file    */
	/* name for saving a new NEC2 input file */
	if( strlen(infile) == 0 )
	{
	  file_selection = create_fileselection();
	  gtk_file_selection_set_filename(
		  GTK_FILE_SELECTION(file_selection), _("untitled") );
	  gtk_widget_show( file_selection );
	  ClearFlag( ALL_SAVE_FLAGS );
	  ClearFlag( NEC2_EDIT_SAVE );
	  SetFlag( NEC2_SAVE );
	  kill_window = nec2_edit_window;
	  return TRUE;
	}

	Save_Nec2_Input_File( nec2_edit_window, infile );
  }

  kill_window = nec2_edit_window;
  Delete_Event( _("Really close NEC2 Editor?") );
  return TRUE;
}


void
on_nec2_editor_destroy                 (GtkObject       *object,
                                        gpointer         user_data)
{
  cmnt_store = NULL,
  geom_store = NULL,
  cmnd_store = NULL;
  nec2_edit_window = NULL;
  kill_window = NULL;
}


  void
on_nec2_save_clicked(	GtkButton       *button,
						gpointer         user_data)
{
  gboolean new = FALSE;

  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
		_("NEC2 Editor's data may not be saved\n"\
		  "while the Frequency Loop is running")) )
	return;

  /* Open file selector to specify file  */
  /* name for saving a new NEC2 input file */
  if( strlen(infile) == 0 )
  {
	file_selection = create_fileselection();
	gtk_file_selection_set_filename(
		GTK_FILE_SELECTION(file_selection), _("untitled") );
	gtk_widget_show( file_selection );
	ClearFlag( ALL_SAVE_FLAGS );
	SetFlag( NEC2_SAVE );
	return;
  }

  /* Save NEC2 editor data */
  Save_Nec2_Input_File( nec2_edit_window, infile );
  if( Nec2_Apply_Checkbutton() )
	Open_Input_File( (gpointer)&new );
}


void
on_nec2_save_as_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  /* No save/open file while freq loop is running */
  if( !Nec2_Save_Warn(
		_("NEC2 Editor's data may not be saved\n"\
		  "while the Frequency Loop is running")) )
	return;

  /* Open file selector to specify file   */
  /* name for saving the edited NEC2 file */
  file_selection = create_fileselection();
  gtk_file_selection_set_filename(
	  GTK_FILE_SELECTION(file_selection), _("untitled") );
  gtk_widget_show( file_selection );
  ClearFlag( ALL_SAVE_FLAGS );
  SetFlag( NEC2_SAVE );
}


void
on_nec2_row_add_clicked                (GtkButton       *button,
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
  if( ! gtk_tree_selection_get_selected(selection, &model, &sibling) )
  {
	/* Empty tree view case */
	model = gtk_tree_view_get_model( selected_treeview );
	gtk_list_store_insert( GTK_LIST_STORE(model), &iter, 0 );
  }
  else gtk_list_store_insert_after(
	  GTK_LIST_STORE(model), &iter, &sibling);

  /* Prime columns of new row */
  ncols = gtk_tree_model_get_n_columns( model );
  if( ncols == 2 ) /* Comments treeview */
	gtk_list_store_set(
		GTK_LIST_STORE(model), &iter, 0, "CM", -1 );
  else
  {
	int idx;
	for( idx = 0; idx < ncols; idx++ )
	  gtk_list_store_set(
		  GTK_LIST_STORE(model), &iter, idx, "--", -1 );
  }
}


  void
on_nec2_row_remv_clicked               (GtkButton       *button,
										gpointer         user_data)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkTreeSelection *selection;

  if( selected_treeview == NULL )
	return;

  selection = gtk_tree_view_get_selection( selected_treeview );
  gtk_tree_selection_get_selected( selection, &model, &iter);
  gtk_list_store_remove( GTK_LIST_STORE(model), &iter );
  selected_treeview = NULL;
}


void
on_nec2_treeview_clear_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  if( selected_treeview != NULL )
	gtk_list_store_clear( GTK_LIST_STORE(
		  gtk_tree_view_get_model(selected_treeview)) );
}


gboolean
on_nec2_cmnt_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW(widget);
  if( event->button == 3 )
	Open_Editor(selected_treeview);
  return FALSE;
}


gboolean
on_nec2_geom_treeview_button_press_event
                                        (GtkWidget       *widget,
										 GdkEventButton  *event,
										 gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW(widget);
  if( event->button == 3 )
  {
	action = EDITOR_EDIT;
	Open_Editor(selected_treeview);
	action = EDITOR_NEW;
  }
  return FALSE;
}


gboolean
on_nec2_cmnd_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
										gpointer         user_data)
{
  selected_treeview = GTK_TREE_VIEW(widget);
  if( event->button == 3 )
  {
	action = EDITOR_EDIT;
	Open_Editor( selected_treeview );
	action = EDITOR_NEW;
  }
  return FALSE;
}


  void
on_nec2_revert_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  /* Open NEC2 input file */
  Open_File( &input_fp, infile, "r" );
  Nec2_Input_File_Treeview( NEC2_EDITOR_REVERT );
}

void on_error_stopbutton_clicked(
	GtkButton *button, gpointer user_data) __attribute__ ((noreturn));
void
on_error_stopbutton_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  ClearFlag( ERROR_CONDX );
  gtk_widget_destroy( error_dialog );
  gtk_main();
  exit(0);
}


void
on_error_okbutton_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  ClearFlag( ERROR_CONDX );
  gtk_widget_destroy( error_dialog );
}


void
on_error_quitbutton_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy( error_dialog );
  gtk_widget_destroy( main_window );
}


void
on_wire_editor_destroy               (GtkObject       *object,
									  gpointer         user_data)
{
  wire_editor = NULL;
}


void
on_wire_pcl_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_SEGPC );
}


void
on_wire_data_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_DATA );
}


void
on_wire_tagnum_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_TAGNUM );
}


void
on_wire_len_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_WLEN );
}


void
on_wire_taper_checkbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Wire_Editor( WIRE_EDITOR_TAPR );
}


void
on_wire_rlen_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_RLEN );
}


void
on_wire_rdia_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( WIRE_EDITOR_RDIA );
}


void
on_wire_new_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  Wire_Editor( EDITOR_NEW );
}


void
on_wire_res_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Wire_Editor( EDITOR_LOAD );
}


void
on_wire_cancel_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Wire_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( wire_editor );
}


void
on_wire_apply_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Wire_Editor( EDITOR_APPLY );
}


void
on_wire_ok_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  Wire_Editor( EDITOR_APPLY );
  gtk_widget_destroy( wire_editor );
}


void
on_gw_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( wire_editor == NULL )
  {
	wire_editor = create_wire_editor();
	gtk_widget_hide( lookup_widget(wire_editor, "wire_taperframe") );
	gtk_widget_show( wire_editor );
  }
  Wire_Editor( action );
  action = EDITOR_NEW;
}


void
on_ga_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( arc_editor == NULL )
  {
	arc_editor = create_arc_editor();
	gtk_widget_show( arc_editor );
  }
  Arc_Editor( action );
  action = EDITOR_NEW;
}


void
on_gh_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( helix_editor == NULL )
  {
	helix_editor = create_helix_editor();
	gtk_widget_show( helix_editor );
  }
  Helix_Editor( action );
  action = EDITOR_NEW;
}


void
on_sp_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( patch_editor == NULL )
  {
	patch_editor = create_patch_editor();
	gtk_widget_show( patch_editor );
  }
  Patch_Editor( action );
  action = EDITOR_NEW;
}


void
on_gr_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( cylinder_editor == NULL )
  {
	cylinder_editor = create_cylinder_editor();
	gtk_widget_show( cylinder_editor );
  }
  Cylinder_Editor( action );
  action = EDITOR_NEW;
}


void
on_gm_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( transform_editor == NULL )
  {
	transform_editor = create_transform_editor();
	gtk_widget_show( transform_editor );
  }
  Transform_Editor( action );
  action = EDITOR_NEW;
}


void
on_gx_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( reflect_editor == NULL )
  {
	reflect_editor = create_reflect_editor();
	gtk_widget_show( reflect_editor );
  }
  Reflect_Editor( action );
  action = EDITOR_NEW;
}


void
on_gs_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( scale_editor == NULL )
  {
	scale_editor = create_scale_editor();
	gtk_widget_show( scale_editor );
  }
  Scale_Editor( action );
  action = EDITOR_NEW;
}


void
on_ex_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( excitation_command == NULL )
  {
	excitation_command = create_excitation_command();
	gtk_widget_show( excitation_command );
  }
  Excitation_Command( action );
  action = EDITOR_NEW;
}


void
on_fr_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( frequency_command == NULL )
  {
	frequency_command = create_frequency_command();
	gtk_widget_show( frequency_command );
  }
  Frequency_Command( action );
  action = EDITOR_NEW;
}


void
on_gn_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( ground_command == NULL )
  {
	ground_command = create_ground_command();
	gtk_widget_show( ground_command );
  }
  Ground_Command( action );
  action = EDITOR_NEW;
}


void
on_gd_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( ground2_command == NULL )
  {
	ground2_command = create_ground2_command();
	gtk_widget_show( ground2_command );
  }
  Ground2_Command( action );
  action = EDITOR_NEW;
}


void
on_rp_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( radiation_command == NULL )
  {
	radiation_command = create_radiation_command();
	gtk_widget_show( radiation_command );
  }
  Radiation_Command( action );
  action = EDITOR_NEW;
}


void
on_ld_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( loading_command == NULL )
  {
	loading_command = create_loading_command();
	gtk_widget_show( loading_command );
  }
  Loading_Command( action );
  action = EDITOR_NEW;
}


void
on_nt_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( network_command == NULL )
  {
	network_command = create_network_command();
	gtk_widget_show( network_command );
  }
  Network_Command( action );
  action = EDITOR_NEW;
}


void
on_tl_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( txline_command == NULL )
  {
	txline_command = create_txline_command();
	gtk_widget_show( txline_command );
  }
  Txline_Command( action );
  action = EDITOR_NEW;
}


void
on_ne_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( nearfield_command == NULL )
  {
	nearfield_command = create_nearfield_command();
	gtk_widget_show( nearfield_command );
  }
  Nearfield_Command( action );
  action = EDITOR_NEW;
}


void
on_ek_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( kernel_command == NULL )
  {
	kernel_command = create_kernel_command();
	gtk_widget_show( kernel_command );
  }
  Kernel_Command( action );
  action = EDITOR_NEW;
}


void
on_kh_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( intrange_command == NULL )
  {
	intrange_command = create_intrange_command();
	gtk_widget_show( intrange_command );
  }
  Intrange_Command( action );
  action = EDITOR_NEW;
}


void
on_xq_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  if( execute_command == NULL )
  {
	execute_command = create_execute_command();
	gtk_widget_show( execute_command );
  }
  Execute_Command( action );
  action = EDITOR_NEW;
}


void
on_patch_data_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Patch_Editor( EDITOR_DATA );
}


void
on_patch_new_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  Patch_Editor( EDITOR_NEW );
}


void
on_patch_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Patch_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( patch_editor );
}


void
on_patch_apply_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Patch_Editor( EDITOR_APPLY );
}


void
on_patch_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  Patch_Editor( EDITOR_APPLY );
  gtk_widget_destroy( patch_editor );
}


void
on_patch_editor_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  patch_editor = NULL;
}


void
on_patch_arbitrary_radiobutton_toggled (GtkToggleButton *togglebutton,
										gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Patch_Editor( PATCH_EDITOR_ARBT );
  else
	Patch_Editor( PATCH_EDITOR_SCCD );
}


void
on_patch_rectangular_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Patch_Editor( PATCH_EDITOR_RECT );
}


void
on_patch_triangular_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Patch_Editor( PATCH_EDITOR_TRIA );
}


void
on_patch_quadrilateral_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Patch_Editor( PATCH_EDITOR_QUAD );
}


void
on_patch_surface_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Patch_Editor( PATCH_EDITOR_SURF );
}


void
on_arc_data_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_DATA );
}


void
on_arc_tagnum_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_TAGNUM );
}


void
on_arc_res_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_LOAD );
}


void
on_arc_new_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  Arc_Editor( EDITOR_NEW );
}


void
on_arc_cancel_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Arc_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( arc_editor );
}


void
on_arc_apply_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  Arc_Editor( EDITOR_APPLY );
}


void
on_arc_ok_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  Arc_Editor( EDITOR_APPLY );
  gtk_widget_destroy( arc_editor );
}


void
on_arc_editor_destroy                  (GtkObject       *object,
                                        gpointer         user_data)
{
  arc_editor = NULL;
}


void
on_arc_pcl_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Arc_Editor( EDITOR_SEGPC );
}


void
on_helix_tagnum_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_TAGNUM );
}


void
on_helix_pcl_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_SEGPC );
}


void
on_helix_nturns_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
 gtk_spin_button_update( spinbutton );
 Helix_Editor( HELIX_EDITOR_NTURN );
}


void
on_helix_res_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_LOAD );
}


  void
on_helix_lh_checkbutton_toggled        (GtkToggleButton *togglebutton,
	gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Helix_Editor( HELIX_EDITOR_LH );
  else
	Helix_Editor( HELIX_EDITOR_RH );
}


void
on_helix_data_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Helix_Editor( EDITOR_DATA );
}


void
on_helix_new_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Helix_Editor( EDITOR_NEW );
}


void
on_helix_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Helix_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( helix_editor );
}


void
on_helix_apply_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Helix_Editor( EDITOR_APPLY );
}


void
on_helix_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  Helix_Editor( EDITOR_APPLY );
  gtk_widget_destroy( helix_editor );
}


void
on_helix_editor_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{
  helix_editor = NULL;
}


void
on_helix_linkall_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Helix_Editor( HELIX_EDITOR_LINKALL );
}


void
on_helix_linkzo_radiobutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Helix_Editor( HELIX_EDITOR_LINKZO );
}


void
on_helix_linkzhl_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
	Helix_Editor( HELIX_EDITOR_LINKZHL );
}


void
on_reflect_taginc_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Reflect_Editor( EDITOR_DATA );
}


void
on_reflect_checkbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Reflect_Editor( REFLECT_EDITOR_TOGGLE );
}


void
on_reflect_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Reflect_Editor( EDITOR_NEW );
}


void
on_reflect_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Reflect_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( reflect_editor );
}


void
on_reflect_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Reflect_Editor( EDITOR_APPLY );
}


void
on_reflect_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Reflect_Editor( EDITOR_APPLY );
  gtk_widget_destroy( reflect_editor );
}


void
on_reflect_editor_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  reflect_editor = NULL;
}


void
on_scale_editor_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{
  scale_editor = NULL;
}


void
on_scale_spinbutton_value_changed      (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Scale_Editor( EDITOR_DATA );
}



void
on_scale_new_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Scale_Editor( EDITOR_NEW );
}


void
on_scale_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Scale_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( scale_editor );
}


void
on_scale_apply_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Scale_Editor( EDITOR_APPLY );
}


void
on_scale_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  Scale_Editor( EDITOR_APPLY );
  gtk_widget_destroy( scale_editor );
}


void
on_cylinder_taginc_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Cylinder_Editor( EDITOR_DATA );
}


void
on_cylinder_total_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Cylinder_Editor( EDITOR_DATA );
}


void
on_cylinder_new_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Cylinder_Editor( EDITOR_NEW );
}


void
on_cylinder_cancel_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  Cylinder_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( cylinder_editor );
}


void
on_cylinder_apply_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Cylinder_Editor( EDITOR_APPLY );
}


void
on_cylinder_ok_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Cylinder_Editor( EDITOR_APPLY );
  gtk_widget_destroy( cylinder_editor );
}


void
on_cylinder_editor_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  cylinder_editor = NULL;
}


void
on_transform_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Transform_Editor( EDITOR_DATA );
}


void
on_transform_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Transform_Editor( EDITOR_NEW );
}


void
on_transform_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
  Transform_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( transform_editor );
}


void
on_transform_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  Transform_Editor( EDITOR_APPLY );
}


void
on_transform_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Transform_Editor( EDITOR_APPLY );
  gtk_widget_destroy( transform_editor );
}


void
on_transform_editor_destroy            (GtkObject       *object,
                                        gpointer         user_data)
{
  transform_editor = NULL;
}



void
on_gend_radiobutton_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Gend_Editor( EDITOR_RDBUTTON );
}


void
on_gend_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Gend_Editor( EDITOR_CANCEL );
  gtk_widget_destroy( gend_editor );
}


void
on_gend_apply_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Gend_Editor( EDITOR_APPLY );
}


void
on_gend_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  Gend_Editor( EDITOR_APPLY );
  gtk_widget_destroy( gend_editor );
}


void
on_gend_editor_destroy                 (GtkObject       *object,
                                        gpointer         user_data)
{
  gend_editor = NULL;
}


void
on_kernel_command_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  kernel_command = NULL;
}


void
on_kernel_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Kernel_Command( COMMAND_CKBUTTON );
}


void
on_kernel_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Kernel_Command( EDITOR_NEW );
}


void
on_kernel_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Kernel_Command( EDITOR_CANCEL );
  gtk_widget_destroy( kernel_command );
}


void
on_kernel_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Kernel_Command( EDITOR_APPLY );
}


void
on_kernel_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Kernel_Command( EDITOR_APPLY );
  gtk_widget_destroy( kernel_command );
}


void
on_execute_command_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  execute_command = NULL;
}


void
on_execute_radiobutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Execute_Command( COMMAND_RDBUTTON );
}


void
on_execute_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Execute_Command( EDITOR_NEW );
}


void
on_execute_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Execute_Command( EDITOR_CANCEL );
  gtk_widget_destroy( execute_command );
}


void
on_execute_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Execute_Command( EDITOR_APPLY );
}


void
on_execute_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Execute_Command( EDITOR_APPLY );
  gtk_widget_destroy( execute_command );
}


void
on_intrange_command_destroy               (GtkObject       *object,
		                                  gpointer         user_data)
{
  intrange_command = NULL;
}


void
on_intrange_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Intrange_Command( EDITOR_DATA );
}


void
on_intrange_new_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Intrange_Command( EDITOR_NEW );
}


void
on_intrange_cancel_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  Intrange_Command( EDITOR_CANCEL );
  gtk_widget_destroy( intrange_command );
}


void
on_intrange_apply_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Intrange_Command( EDITOR_APPLY );
}


void
on_intrange_ok_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Intrange_Command( EDITOR_APPLY );
  gtk_widget_destroy( intrange_command );
}


void
on_ground_command_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  ground_command = NULL;
}


void
on_ground_radiobutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Ground_Command( COMMAND_RDBUTTON );
}


void
on_ground_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Ground_Command( COMMAND_CKBUTTON );
}


void
on_ground_spinbutton_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Ground_Command( EDITOR_DATA );
}


void
on_ground_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground_Command( EDITOR_NEW );
}


void
on_ground_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground_Command( EDITOR_CANCEL );
  gtk_widget_destroy( ground_command );
}


void
on_ground_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground_Command( EDITOR_APPLY );
}


void
on_ground_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground_Command( EDITOR_APPLY );
  gtk_widget_destroy( ground_command );
}


void
on_nearfield_command_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  nearfield_command = NULL;
}



void
on_nearfield_nh_checkbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Nearfield_Command( NEARFIELD_NH_CKBUTTON );
}


void
on_nearfield_ne_checkbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Nearfield_Command( NEARFIELD_NE_CKBUTTON );
}


void
on_nearfield_radiobutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Nearfield_Command( COMMAND_RDBUTTON );
}


void
on_nearfield_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Nearfield_Command( EDITOR_DATA );
}


void
on_nearfield_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Nearfield_Command( EDITOR_NEW );
}


void
on_nearfield_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Nearfield_Command( EDITOR_CANCEL );
  gtk_widget_destroy( nearfield_command );
}


void
on_nearfield_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Nearfield_Command( EDITOR_APPLY );
}


void
on_nearfield_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Nearfield_Command( EDITOR_APPLY );
  gtk_widget_destroy( nearfield_command );
}


void
on_radiation_command_destroy           (GtkObject       *object,
                                        gpointer         user_data)
{
  radiation_command = NULL;
}


void
on_radiation_radiobutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Radiation_Command( COMMAND_RDBUTTON );
}


void
on_radiation_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Radiation_Command( EDITOR_DATA );
}


void
on_radiation_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Radiation_Command( EDITOR_NEW );
}


void
on_radiation_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
  Radiation_Command( EDITOR_CANCEL );
  gtk_widget_destroy( radiation_command );
}


void
on_radiation_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  Radiation_Command( EDITOR_APPLY );
}


void
on_radiation_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Radiation_Command( EDITOR_APPLY );
  gtk_widget_destroy( radiation_command );
}


void
on_excitation_command_destroy          (GtkObject       *object,
                                        gpointer         user_data)
{
  excitation_command = NULL;
}


void
on_excitation_radiobutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Excitation_Command( COMMAND_RDBUTTON );
}


void
on_excitation_checkbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Excitation_Command( COMMAND_CKBUTTON );
}


void
on_excitation_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Excitation_Command( EDITOR_DATA );
}


void
on_excitation_new_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Excitation_Command( EDITOR_NEW );
}


void
on_excitation_cancel_button_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
  Excitation_Command( EDITOR_CANCEL );
  gtk_widget_destroy( excitation_command );
}


void
on_excitation_apply_button_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
  Excitation_Command( EDITOR_APPLY );
}


void
on_excitation_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Excitation_Command( EDITOR_APPLY );
  gtk_widget_destroy( excitation_command );
}


void
on_frequency_command_destroy           (GtkObject       *object,
                                        gpointer         user_data)
{
  frequency_command = NULL;
}


void
on_frequency_radiobutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Frequency_Command( COMMAND_RDBUTTON );
}


void
on_frequency_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Frequency_Command( EDITOR_DATA );
}


void
on_frequency_step_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
 gtk_spin_button_update( spinbutton );
 Frequency_Command( FREQUENCY_EDITOR_FSTEP );
}


void
on_frequency_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Frequency_Command( EDITOR_NEW );
}


void
on_frequency_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
  Frequency_Command( EDITOR_CANCEL );
  gtk_widget_destroy( frequency_command );
}


void
on_frequency_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  Frequency_Command( EDITOR_APPLY );
}


void
on_frequency_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Frequency_Command( EDITOR_APPLY );
  gtk_widget_destroy( frequency_command );
}


void
on_loading_command_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  loading_command = NULL;
}


void
on_loading_radiobutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Loading_Command( COMMAND_RDBUTTON );
}


void
on_loading_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Loading_Command( EDITOR_DATA );
}


void
on_loading_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Loading_Command( EDITOR_NEW );
}


void
on_loading_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Loading_Command( EDITOR_CANCEL );
  gtk_widget_destroy( loading_command );
}


void
on_loading_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Loading_Command( EDITOR_APPLY );
}


void
on_loading_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Loading_Command( EDITOR_APPLY );
  gtk_widget_destroy( loading_command );
}


void
on_network_command_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  network_command = NULL;
}


void
on_network_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Network_Command( EDITOR_DATA );
}


void
on_network_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Network_Command( EDITOR_NEW );
}


void
on_network_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Network_Command( EDITOR_CANCEL );
  gtk_widget_destroy( network_command );
}


void
on_network_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Network_Command( EDITOR_APPLY );
}


void
on_network_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Network_Command( EDITOR_APPLY );
  gtk_widget_destroy( network_command );
}


void
on_txline_command_destroy              (GtkObject       *object,
                                        gpointer         user_data)
{
  txline_command = NULL;
}


void
on_txline_spinbutton_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Txline_Command( EDITOR_DATA );
}


void
on_txline_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Txline_Command( EDITOR_NEW );
}


void
on_txline_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Txline_Command( EDITOR_CANCEL );
  gtk_widget_destroy( txline_command );
}


void
on_txline_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Txline_Command( EDITOR_APPLY );
}


void
on_txline_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Txline_Command( EDITOR_APPLY );
  gtk_widget_destroy( txline_command );
}


void
on_txline_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Txline_Command( COMMAND_CKBUTTON );
}


void
on_ground2_command_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
  ground2_command = NULL;
}


void
on_ground2_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  gtk_spin_button_update( spinbutton );
  Ground2_Command( EDITOR_DATA );
}


void
on_ground2_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground2_Command( EDITOR_NEW );
}


void
on_ground2_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground2_Command( EDITOR_CANCEL );
  gtk_widget_destroy( ground2_command );
}


void
on_ground2_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground2_Command( EDITOR_APPLY );
}


void
on_ground2_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Ground2_Command( EDITOR_APPLY );
  gtk_widget_destroy( ground2_command );
}


  void
on_loop_start_clicked                  (GtkButton       *button,
										gpointer         user_data)
{
  Start_Frequency_Loop();
}


  void
on_loop_pause_clicked                  (GtkButton       *button,
										gpointer         user_data)
{
  if( isFlagSet(FREQ_LOOP_RUNNING) )
	SetFlag( FREQ_LOOP_STOP );
}


  void
on_loop_reset_clicked                  (GtkButton       *button,
										gpointer         user_data)
{
  if( isFlagClear(FREQ_LOOP_RUNNING) )
  {
	SetFlag( FREQ_LOOP_INIT );
	ClearFlag( FREQ_LOOP_RUNNING );
  }
}

static GtkWidget *aboutdialog = NULL;
void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( aboutdialog == NULL )
  {
	aboutdialog = create_aboutdialog();
	gtk_widget_show( aboutdialog );
	gtk_about_dialog_set_program_name(
		GTK_ABOUT_DIALOG(aboutdialog), PACKAGE );
	gtk_about_dialog_set_version(
		GTK_ABOUT_DIALOG(aboutdialog), VERSION );
  }
}


void
on_aboutdialog_close                   (GtkDialog       *dialog,
                                        gpointer         user_data)
{
  gtk_widget_destroy( aboutdialog );
  aboutdialog = NULL;
}


void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data)
{
  gtk_widget_destroy( aboutdialog );
  aboutdialog = NULL;
}


void
on_net_gain_activate                   (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
	SetFlag( PLOT_NETGAIN );
  else
	ClearFlag( PLOT_NETGAIN );

  /* Trigger a redraw of frequency plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(FREQ_LOOP_DONE) )
	Plot_Frequency_Data();
}


gboolean
on_structure_drawingarea_button_press_event
                                        (GtkWidget      *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  structure_proj_params.reset = TRUE;
  return FALSE;
}


void
on_structure_zoom_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  structure_proj_params.xy_zoom = gtk_spin_button_get_value( spinbutton );
  structure_proj_params.xy_zoom /= 100.0;
  structure_proj_params.xy_scale =
	structure_proj_params.xy_scale1 * structure_proj_params.xy_zoom;

  /* Trigger a redraw of structure drawingarea */
  Draw_Structure( structure_drawingarea );
}


void
on_structure_plus_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  structure_proj_params.xy_zoom =
	gtk_spin_button_get_value( structure_zoom );
  structure_proj_params.xy_zoom *= 1.1;
  gtk_spin_button_set_value(
	  structure_zoom, structure_proj_params.xy_zoom );
}


void
on_structure_minus_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  structure_proj_params.xy_zoom =
	gtk_spin_button_get_value( structure_zoom );
  structure_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  structure_zoom, structure_proj_params.xy_zoom );
}


void
on_structure_one_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_spin_button_set_value( structure_zoom, 100.0 );
  structure_proj_params.reset = TRUE;
  New_Projection_Parameters(
	  structure_pixmap_width,
	  structure_pixmap_height,
	  &structure_proj_params );
  Draw_Structure( structure_drawingarea );
}


gboolean
on_rdpattern_drawingarea_button_press_event
                                        (GtkWidget      *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  rdpattern_proj_params.reset = TRUE;
  return FALSE;
}


void
on_rdpattern_zoom_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom = gtk_spin_button_get_value( spinbutton );
  rdpattern_proj_params.xy_zoom /= 100.0;
  rdpattern_proj_params.xy_scale =
	rdpattern_proj_params.xy_scale1 * rdpattern_proj_params.xy_zoom;

  /* Trigger a redraw of structure drawingarea */
  Draw_Radiation( rdpattern_drawingarea );
}


void
on_rdpattern_plus_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom =
	gtk_spin_button_get_value( rdpattern_zoom );
  rdpattern_proj_params.xy_zoom *= 1.1;
  gtk_spin_button_set_value(
	  rdpattern_zoom, rdpattern_proj_params.xy_zoom );
}


void
on_rdpattern_minus_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  rdpattern_proj_params.xy_zoom =
	gtk_spin_button_get_value( rdpattern_zoom );
  rdpattern_proj_params.xy_zoom /= 1.1;
  gtk_spin_button_set_value(
	  rdpattern_zoom, rdpattern_proj_params.xy_zoom );
}


void
on_rdpattern_one_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_spin_button_set_value( rdpattern_zoom, 100.0 );
  rdpattern_proj_params.reset = TRUE;
  New_Projection_Parameters(
	  rdpattern_pixmap_width,
	  rdpattern_pixmap_height,
	  &rdpattern_proj_params );
  Draw_Radiation( rdpattern_drawingarea );
}


gboolean
on_structure_drawingarea_scroll_event  (GtkWidget       *widget,
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
  return FALSE;
}


gboolean
on_rdpattern_drawingarea_scroll_event  (GtkWidget       *widget,
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
  return FALSE;
}

