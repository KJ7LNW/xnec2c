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

/* callback_func.c
 *
 * Functions to handle GTK callbacks for xnec2c
 */

#include "callback_func.h"
#include "shared.h"

/*-----------------------------------------------------------------------*/

/* Save_Pixmap()
 *
 * Saves pixmaps as png files
 */
  void
Save_Pixmap(
	GdkPixmap *pixmap, int pixmap_width,
	int pixmap_height, char *filename )
{
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  /* Get image from pixmap */
  gtk_widget_grab_focus( structure_drawingarea );
  pixbuf = gdk_pixbuf_get_from_drawable(
	  NULL, pixmap, gdk_drawable_get_colormap(pixmap),
	  0, 0, 0, 0, pixmap_width, pixmap_height );

  /* Save image as PNG file */
  gdk_pixbuf_save( pixbuf, filename, "png", &error, NULL );
  g_object_unref( pixbuf );

} /* Save_Pixmap() */

/*-----------------------------------------------------------------------*/

/* New_Viewer_Angle()
 *
 * Sets parameters for a new viewer angle
 */
void
New_Viewer_Angle(
	double wr, double wi,
	GtkSpinButton *wr_spb,
	GtkSpinButton *wi_spb,
	projection_parameters_t *params )
{
  /* Recalculate projection paramenters */
  params->Wr = wr;
  params->Wi = wi;

  /* Set new value */
  Set_Spin_Button( wr_spb, (gdouble)params->Wr );
  Set_Spin_Button( wi_spb, (gdouble)params->Wi );

} /* New_Viewer_Angle() */

/*-----------------------------------------------------------------------*/

/* Set_Spin_Button()
 *
 * Sets the value of a spin button
 */
  void
Set_Spin_Button( GtkSpinButton *spin, gdouble value )
{
  /* Save original value and set new */
  gdouble sav = gtk_spin_button_get_value( spin );
  gtk_spin_button_set_value( spin, value );

  /* Issue a value_changed signal if needed (given same values) */
  if( sav == value )
	g_signal_emit_by_name( G_OBJECT(spin), "value_changed", NULL );

} /* Set_Spin_Button() */

/*-----------------------------------------------------------------------*/

/* Create_Pixmap()
 *
 * Creates or resizes a pixmap after a configure event
 */
void
Create_Pixmap(
	GdkPixmap **pixmap,
	int *pixmap_width,
	int *pixmap_height,
	GtkWidget *widget,
	GdkEventConfigure *event,
	projection_parameters_t *params )
{
  /* Create or resize pixmap */
  if( *pixmap != NULL )
  {
	g_object_unref( *pixmap );
	*pixmap = NULL;
  }

  *pixmap = gdk_pixmap_new(
	  widget->window,
	  event->width,
	  event->height, -1 );
  *pixmap_width  = event->width;
  *pixmap_height = event->height;

  /* Calculate new projection parameters */
  if( params != NULL )
	New_Projection_Parameters( *pixmap_width, *pixmap_height, params );

} /* Create_Pixmap() */

/*-----------------------------------------------------------------------*/

/* Motion_Event()
 *
 * Handles pointer motion event on drawingareas
 */
void
Motion_Event(
	GdkEventMotion *event,
	projection_parameters_t *params )
{
  /* Save previous pointer position */
  static gdouble x_old = 0.0, y_old = 0.0;

  gdouble x = event->x;
  gdouble y = event->y;
  gdouble dx, dy;
  gchar value[6];
  size_t s = sizeof( value );

  /* Initialize saved x,y */
  if( params->reset )
  {
	x_old = x;
	y_old = y;
	params->reset = FALSE;
  }

  /* Recalculate projection parameters
   * according to pointer motion */
  dx = x - x_old;
  dy = y - y_old;
  x_old = x;
  y_old = y;

  /* Other buttons are used for moving axes on screen */
  if( event->state & GDK_BUTTON1_MASK )
  {
	/* Set the structure rotate/incline spinbuttons */
	if( isFlagSet(COMMON_PROJECTION) ||
		(params->type == STRUCTURE_DRAWINGAREA) )
	{
	  structure_proj_params.Wr -= dx / (gdouble)MOTION_EVENTS_COUNT;
	  structure_proj_params.Wi += dy / (gdouble)MOTION_EVENTS_COUNT;
	  snprintf( value, s, "%d", (int)structure_proj_params.Wr );
	  gtk_entry_set_text( GTK_ENTRY(rotate_structure), value );
	  snprintf( value, s, "%d", (int)structure_proj_params.Wi );
	  gtk_entry_set_text( GTK_ENTRY(incline_structure), value );
	}

	/* Set the rdpattern rotate/incline spinbuttons */
	if( (isFlagSet(DRAW_ENABLED) &&
		  isFlagSet(COMMON_PROJECTION)) ||
		(params->type == RDPATTERN_DRAWINGAREA) )
	{
	  rdpattern_proj_params.Wr -= dx / (gdouble)MOTION_EVENTS_COUNT;
	  rdpattern_proj_params.Wi += dy / (gdouble)MOTION_EVENTS_COUNT;
	  snprintf( value, s, "%d", (int)rdpattern_proj_params.Wr );
	  gtk_entry_set_text( GTK_ENTRY(rotate_rdpattern), value );
	  snprintf( value, s, "%d", (int)rdpattern_proj_params.Wi );
	  gtk_entry_set_text( GTK_ENTRY(incline_rdpattern), value );
	}

	/* Rotate/incline structure */
	if( params->type == STRUCTURE_DRAWINGAREA )
	{
	  New_Structure_Projection_Angle();
	  if( isFlagSet(DRAW_ENABLED) &&
		  isFlagSet(COMMON_PROJECTION) )
		New_Radiation_Projection_Angle();
	}
	else if( params->type == RDPATTERN_DRAWINGAREA )
	{
	  /* Rotate/incline rdpattern */
	  New_Radiation_Projection_Angle();
	  if( isFlagSet(COMMON_PROJECTION) )
		New_Structure_Projection_Angle();
	}
  }    /* if( event->state & GDK_BUTTON1_MASK ) */
  else
  {
	/* Move structure or rdpattern axes on screen */
	params->x_center += dx;
	params->y_center -= dy;
	if( params->type == STRUCTURE_DRAWINGAREA )
	  Draw_Structure( structure_drawingarea );
	if( params->type == RDPATTERN_DRAWINGAREA )
	  Draw_Radiation( rdpattern_drawingarea );
  }

} /* Motion_Event() */

/*-----------------------------------------------------------------------*/

/* Plot_Select()
 *
 * Sets up plotting of requested freq data
 */
  void
Plot_Select( GtkToggleButton *togglebutton, unsigned long long int flag )
{
  if( gtk_toggle_button_get_active(togglebutton) )
  {
	SetFlag( flag | PLOT_SELECT );
	calc_data.ngraph++;
  }
  else
  {
	ClearFlag( flag );
	calc_data.ngraph--;
  }

  /* Trigger a redraw of frequency plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(FREQ_LOOP_DONE) )
	Plot_Frequency_Data();

} /* Plot_Select() */

/*-----------------------------------------------------------------------*/

/* Delete_Event()
 *
 * Handles user request to delete a window
 */
 void
Delete_Event( gchar *mesg )
{
  quit_dialog = create_quit_dialog();
  gtk_widget_show( quit_dialog );

  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	if( isFlagSet(MAIN_QUIT) )
	  gtk_label_set_text( GTK_LABEL(lookup_widget(
			  quit_dialog, "quit_label")),
		  _("The frequency loop is running\n"\
			"Really end operation?") );
	else gtk_label_set_text( GTK_LABEL(lookup_widget(
			quit_dialog, "quit_label")),
		_("The frequency loop is running\n"\
		  "Really close this window?") );
  }
  else gtk_label_set_text( GTK_LABEL(lookup_widget(
		  quit_dialog, "quit_label")), mesg );

} /* Delete_Event() */

/*-----------------------------------------------------------------------*/

/* Set_Pol_Menuitem()
 *
 * Sets the polarization type menuitem to current setting
 */
  void
Set_Pol_Menuitem( GtkMenuItem *menuitem )
{
  gchar *pol_menu[NUM_POL] =
  {
	"rdpattern_total",
	"rdpattern_horizontal",
	"rdpattern_vertical",
	"rdpattern_right_hand",
	"rdpattern_left_hand",
  };

  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(lookup_widget(
		  GTK_WIDGET(menuitem), pol_menu[calc_data.pol_type])), TRUE );

} /* Set_Pol_Menuitem() */

/*-----------------------------------------------------------------------*/

/* Close_Windows()
 *
 * Closes some open windows
 */
  void
Close_Windows( void )
{
  if( isFlagSet(PLOT_ENABLED) )
  {
	gtk_widget_destroy( freqplots_window );
	gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(lookup_widget(
			main_window, "main_freqplots")), FALSE );
  }

  if( isFlagSet(DRAW_ENABLED) )
  {
	gtk_widget_destroy( rdpattern_window );
	gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(lookup_widget(
			main_window, "main_rdpattern")), FALSE );
  }

} /* Close_Windows() */

/*-----------------------------------------------------------------------*/

/* Open_Editor()
 *
 * Pops up a Editor window on user
 * right-click on a NEC2 Editor treeview
 */
  gboolean
Open_Editor( GtkTreeView *view )
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *card;
  GtkWidget *button;

  /* Find the selected treeview row */
  selection = gtk_tree_view_get_selection( view );
  if( !gtk_tree_selection_get_selected(selection, &model, &iter) )
	return( FALSE );

  /* Get the "card" name from first column */
  gtk_tree_model_get( model, &iter, 0, &card, -1);
  size_t s = strlen( card );

  /* Some "cards" have common editors */
  if( strcmp(card, "GC") == 0 )
	Strlcpy( card, "GW", s );
  else if( strcmp(card, "SC") == 0 )
	Strlcpy( card, "SP", s );
  else if( strcmp(card, "SM") == 0 )
	Strlcpy( card, "SP", s );
  else if( strcmp(card, "NH") == 0 )
	Strlcpy( card, "NE", s );
  else if( strcmp(card, "GE") == 0 )
  {
	Gend_Editor( EDITOR_EDIT );
	return( TRUE );
  } /* EN Not editable */
  else if( strcmp(card, "EN") == 0 )
	return( TRUE );

  /* Send a "clicked" signal to the appropriate editor button */
  card[0] = (gchar)tolower((int)card[0]);
  card[1] = (gchar)tolower((int)card[1]);
  button = lookup_widget( GTK_WIDGET(view), card );
  g_free(card);
  if( button != NULL )
	g_signal_emit_by_name( button, "clicked" );
  else return( FALSE );

  return( TRUE );
} /* Open_Editor() */

/*-----------------------------------------------------------------------*/

/* Main_Rdpattern_Activate()
 *
 * Callback function for the Radiation Pattern draw button
 */
  void
Main_Rdpattern_Activate( gboolean from_menu )
{
  /* Set E field check menu item */
  if( fpat.nfeh & NEAR_EFIELD )
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_e_field")), TRUE );
	SetFlag( DRAW_EFIELD );
  }
  else
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_e_field")), FALSE );
	ClearFlag( DRAW_EFIELD );
  }

  /* Set H field check menu item */
  if( fpat.nfeh & NEAR_HFIELD )
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_h_field")), TRUE );
	SetFlag( DRAW_HFIELD );
  }
  else
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_h_field")), FALSE );
	ClearFlag( DRAW_HFIELD );
  }

  /* Set Poynting vector check menu item */
  if( (fpat.nfeh & NEAR_EHFIELD) == NEAR_EHFIELD )
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_poynting_vector")), TRUE );
	SetFlag( DRAW_POYNTING );
  }
  else
  {
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_poynting_vector")), FALSE );
	ClearFlag( DRAW_POYNTING );
  }

  /* Set structure overlay in Rad Pattern window */
  if( isFlagClear(OVERLAY_STRUCT) )
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_overlay_structure")), FALSE );
  else
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
		  lookup_widget(rdpattern_window,
			"rdpattern_overlay_structure")), TRUE );


  /* Sync common projection spinbuttons */
  if( isFlagSet(COMMON_PROJECTION) )
  {
	gchar value[6];
	size_t s = sizeof( value ) - 1;

	rdpattern_proj_params.Wr = structure_proj_params.Wr;
	rdpattern_proj_params.Wi = structure_proj_params.Wi;
	snprintf( value, s, "%d", (int)rdpattern_proj_params.Wr );
	value[s] = '\0';
	gtk_entry_set_text( GTK_ENTRY(rotate_rdpattern), value );
	snprintf( value, s, "%d", (int)rdpattern_proj_params.Wi );
	value[s] = '\0';
	gtk_entry_set_text( GTK_ENTRY(incline_rdpattern), value );
  }
  else	/* Initialize radiation pattern projection angles */
  {
	rdpattern_proj_params.Wr =
	  gtk_spin_button_get_value(rotate_rdpattern);
	rdpattern_proj_params.Wi =
	  gtk_spin_button_get_value(incline_rdpattern);
  }
  New_Radiation_Projection_Angle();

  /* Redo currents if not reaching this function
   * from the menu callback (e.g. not user action) */
  if( !crnt.valid && !from_menu ) Redo_Currents( NULL );

  /* Display frequency in freq spinbutton */
  if( from_menu )
  {
	char value[9];
	size_t s = sizeof( value );
	snprintf( value, s, "%.3f", calc_data.fmhz );
	value[s - 1] = '\0';
	gtk_entry_set_text(
		GTK_ENTRY(rdpattern_frequency), value );
  }

  /* Enable Gain or E/H field drawing */
  SetFlag( DRAW_ENABLED );

} /* Main_Rdpattern_Activate() */

/*-----------------------------------------------------------------------*/

/* Main_Freqplots_Activate()
 *
 * Callback function for for the main Frequency Plots button
 */
  gboolean
Main_Freqplots_Activate( void )
{
  /* No plots for Incident Field and
   * Elementary Current Source Excitation */
  if( (fpat.ixtyp != 0) && (fpat.ixtyp != 5) )
  {
	stop( _("Not available for Incident Field or\n"\
		  "Elementary Current Source Excitation.\n"\
		  "(Excitation Types 1 to 4)"), ERR_OK );
	return( FALSE );
  }

  /* Enable freq data graph plotting */
  SetFlag( PLOT_ENABLED );

  return( TRUE );
} /* Main_Freqplots_Activate() */

/*-----------------------------------------------------------------------*/

/* Rdpattern_Gain_Togglebutton_Toggled()
 *
 * Callback function for Rad Pattern window Gain button
 */
  void
Rdpattern_Gain_Togglebutton_Toggled( gboolean flag )
{
  /* If radiation pattern data do not
   * allow drawing of radiation pattern */
  if( isFlagClear(ENABLE_RDPAT) ) return;

  /* Enable or not gain (radiation) pattern plotting */
  if( flag )
  {
	SetFlag( DRAW_GAIN );
	ClearFlag( DRAW_EHFIELD );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(lookup_widget(
			rdpattern_window, "rdpattern_eh_togglebutton")), FALSE );

	/* Redraw radiation pattern drawingarea */
	if( isFlagSet(DRAW_ENABLED) &&
		isFlagClear(FREQ_LOOP_RUNNING) )
	{
	  if( !crnt.valid ) Redo_Currents( NULL );
	  SetFlag( DRAW_NEW_RDPAT );
	  Draw_Radiation( rdpattern_drawingarea );
	}

	Set_Window_Labels();
  }
  else
  {
	ClearFlag( DRAW_GAIN );
	/* Clear radiation pattern drawingarea */
	if( isFlagClear(DRAW_EHFIELD) &&
		isFlagSet(DRAW_ENABLED) )
	  Draw_Radiation( rdpattern_drawingarea );
	Free_Draw_Buffers();
  }

  return;
} /* Rdpattern_Gain_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Rdpattern_EH_Togglebutton_Toggled()
 *
 * Callback function for Rad Pattern window E/H field button
 */
  void
Rdpattern_EH_Togglebutton_Toggled( gboolean flag )
{
  /* If no near EH data */
  if( !fpat.nfeh ) return;

  /* Enable or not E/H fields plotting */
  if( flag )
  {
	SetFlag( DRAW_EHFIELD );
	ClearFlag( DRAW_GAIN );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(lookup_widget(
			rdpattern_window, "rdpattern_gain_togglebutton")), FALSE );

	/* Delegate near field calcuations to child
	 * processes if forked and near field data valid */
	if( FORKED )
	{
	  Alloc_Nearfield_Buffers(fpat.nrx, fpat.nry, fpat.nrz);
	  Pass_EH_Flags();
	}

	/* Redraw radiation pattern drawingarea */
	if( isFlagSet(DRAW_ENABLED) &&
		isFlagClear(FREQ_LOOP_RUNNING) )
	{
	  if( !near_field.valid ) Redo_Currents(NULL);
	  Near_Field_Pattern();
	  SetFlag( DRAW_NEW_EHFIELD );
	  Draw_Radiation( rdpattern_drawingarea );
	}

	Set_Window_Labels();
  }
  else
  {
	ClearFlag( NEAREH_ANIMATE );
	ClearFlag( DRAW_EHFIELD );

	/* Clear radiation pattern drawingarea */
	if( isFlagClear(DRAW_GAIN)  &&
		isFlagSet(DRAW_ENABLED) )
	  Draw_Radiation( rdpattern_drawingarea );

	/* Disable near field calcuations
	 * by child processes if forked */
	Pass_EH_Flags();
  }

} /* Rdpattern_EH_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Main_Currents_Togglebutton_Toggled()
 *
 * Callback function for Main Currents toggle button
 */
  void
Main_Currents_Togglebutton_Toggled( gboolean flag )
{
  /* Enable calculation and rendering of structure curents */
  if( flag )
  {
	SetFlag( DRAW_CURRENTS );
	ClearFlag( DRAW_CHARGES );
	crnt.newer = 1;
	Alloc_Crnt_Buffs();

	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(lookup_widget(main_window,
			"main_charges_togglebutton")), FALSE );
	gtk_label_set_text(
		GTK_LABEL(lookup_widget(main_window, "struct_label")),
		_("View Currents") );

	if( !crnt.valid && isFlagClear(FREQ_LOOP_RUNNING) )
	  Redo_Currents( NULL );
	else if( crnt.valid )
	  Draw_Structure( structure_drawingarea );

	if( isFlagSet(OVERLAY_STRUCT) )
	  Draw_Radiation( rdpattern_drawingarea );
  }
  else
  {
	ClearFlag( DRAW_CURRENTS );
	if( isFlagClear(DRAW_CHARGES) )
	{
	  /* Redraw structure on screen if frequency loop is not running */
	  gtk_label_set_text(
		  GTK_LABEL(lookup_widget(main_window, "struct_label")),
		  _("View Geometry") );
	  if( isFlagClear(FREQ_LOOP_RUNNING) )
		Draw_Structure( structure_drawingarea );
	  Free_Crnt_Buffs();
	}
	if( isFlagSet(OVERLAY_STRUCT) )
	  Draw_Radiation( rdpattern_drawingarea );
  }
} /* Main_Currents_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Main_Charges_Togglebutton_Toggled()
 *
 * Callback function for Main Charges toggle button
 */
  void
Main_Charges_Togglebutton_Toggled( gboolean flag )
{
  if( flag )
  {
	SetFlag( DRAW_CHARGES );
	ClearFlag( DRAW_CURRENTS );
	crnt.newer = 1;
	Alloc_Crnt_Buffs();

	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(lookup_widget(main_window,
			"main_currents_togglebutton")), FALSE );
	gtk_label_set_text(
		GTK_LABEL(lookup_widget(main_window, "struct_label")),
		_("View Charges") );

	if( !crnt.valid && isFlagClear(FREQ_LOOP_RUNNING) )
	  Redo_Currents( NULL );
	else if( crnt.valid )
	  Draw_Structure( structure_drawingarea );

	if( isFlagSet(OVERLAY_STRUCT) )
	  Draw_Radiation( rdpattern_drawingarea );
  }
  else
  {
	ClearFlag( DRAW_CHARGES );
	if( isFlagClear(DRAW_CURRENTS) )
	{
	  /* Redraw structure on screen if frequency loop is not running */
	  gtk_label_set_text(
		  GTK_LABEL(lookup_widget(main_window, "struct_label")),
		  _("View Geometry") );
	  if( isFlagClear(FREQ_LOOP_RUNNING) )
		Draw_Structure( structure_drawingarea );
	  Free_Crnt_Buffs();
	}
	if( isFlagSet(OVERLAY_STRUCT) )
	  Draw_Radiation( rdpattern_drawingarea );
  }
} /* Main_Charges_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Open_Nec2_Editor()
 *
 * Opens NEC2 editor window and fills
 * tree view according to action flag
 */
  void
Open_Nec2_Editor( int action )
{
  nec2_edit_window = create_nec2_editor();
  gtk_widget_show( nec2_edit_window );

  geom_treeview = GTK_TREE_VIEW(
	  lookup_widget(nec2_edit_window, "nec2_geom_treeview") );
  cmnd_treeview = GTK_TREE_VIEW(
	  lookup_widget(nec2_edit_window, "nec2_cmnd_treeview") );

  Nec2_Input_File_Treeview( action );

  geom_adjustment = gtk_scrolled_window_get_vadjustment(
	  GTK_SCROLLED_WINDOW(lookup_widget(nec2_edit_window,
		  "scrolledwindow4")) );
  cmnd_adjustment = gtk_scrolled_window_get_vadjustment(
	  GTK_SCROLLED_WINDOW(lookup_widget(nec2_edit_window,
		  "scrolledwindow3")) );
} /* Open_Nec2_Editor() */

/*-----------------------------------------------------------------------*/

/* Nec2_Apply_Checkbuton()
 *
 * Checks whether the NEC2 editor's "Apply" check button is active
 */

  gboolean
Nec2_Apply_Checkbutton( void )
{
  return( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
		  lookup_widget(nec2_edit_window,
			"nec2_apply_checkbutton" ))) );
}

/*-----------------------------------------------------------------------*/

/* Gtk_Quit()
 *
 * Quits gtk main
 */
  void
Gtk_Quit( void )
{
  int i, k;

  Close_File( &input_fp );
  SetFlag( MAIN_QUIT );

  /* Kill child processes */
  if( FORKED && !CHILD )
	while( num_child_procs )
	{
	  num_child_procs--;
	  kill( forked_proc_data[num_child_procs]->child_pid, SIGKILL );
	}

  /* Kill possibly nested loops */
  k = (int)gtk_main_level();
  for( i = 0; i < k; i++ )
	gtk_main_quit();

} /* Gtk_Quit() */

/*-----------------------------------------------------------------------*/

/* Pass_EH_Flags
 *
 * Passes near field related flags to child processes
 */
  void
Pass_EH_Flags( void )
{
  char flag;
  size_t cnt;
  int idx;

  /* Abort if not forked */
  if( !FORKED ) return;

  /* Tell child process to calculate near field data */
  cnt = strlen( fork_commands[EHFIELD] );
  for( idx = 0; idx < calc_data.num_jobs; idx++ )
	Write_Pipe( idx, fork_commands[EHFIELD], (ssize_t)cnt, TRUE );

  /* Tell child to set near field flags */
  flag = 0;
  if( isFlagSet(DRAW_EHFIELD) )		flag |= 0x01;
  if( isFlagSet(NEAREH_SNAPSHOT) )	flag |= 0x02;
  if( isFlagSet(DRAW_EFIELD) )		flag |= 0x04;
  if( isFlagSet(DRAW_HFIELD) )		flag |= 0x08;

  cnt = sizeof( flag );
  for( idx = 0; idx < calc_data.num_jobs; idx++ )
	Write_Pipe( idx, &flag, (ssize_t)cnt, TRUE );

} /* Pass_EH_Flags */

/*-----------------------------------------------------------------------*/

/* Alloc_Crnt_Buffs()
 *
 * Allocates memory for current/charge draw buffers
 */
  void
Alloc_Crnt_Buffs( void )
{
  /* Patch currents buffer */
  if( data.m > 0 )
  {
	size_t mreq = (size_t)data.m * sizeof( double );
	mem_realloc( (void **)&ct1m, mreq, "in callback_func.c" );
	mem_realloc( (void **)&ct2m, mreq, "in callback_func.c" );
  }

  /* Segment currents buffer */
  if( data.n > 0 )
  {
	size_t mreq = (size_t)data.n * sizeof( double );
	mem_realloc( (void **)&cmag, mreq, "in callback_func.c" );
  }

} /* Alloc_Crnt_Buffs() */

/*-----------------------------------------------------------------------*/

/* Free_Crnt_Buffs()
 *
 * Frees current/charge draw buffers
 */
  void
Free_Crnt_Buffs( void )
{
  free_ptr( (void **)&ct1m );
  free_ptr( (void **)&ct2m );
  free_ptr( (void **)&cmag );
} /* Free_Crnt_Buffs() */

/*-----------------------------------------------------------------------*/

