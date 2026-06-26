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

#include "callback_func.h"
#include "cairo/cairo_draw.h"
#include "shared.h"
#include "wl_session.h"
#include "prerender/prerender_state.h"
#include "prerender/prerender_color.h"
#include "mem_track.h"

#include "opengl/opengl_structure.h"

/*-----------------------------------------------------------------------*/

/* Save_Pixbuf()
 *
 * Saves pixbufs as png files
 */
  gboolean
Save_Pixbuf( gpointer save_data )
{
  GdkPixbuf *pixbuf;
  GError *error = NULL;
  GdkWindow *window = gtk_widget_get_window(
      ((save_data_t *)save_data)->drawingarea );

  /* Get image from pixbuf */
  pixbuf = gdk_pixbuf_get_from_window( window, 0, 0,
      ((save_data_t *)save_data)->width,
      ((save_data_t *)save_data)->height );

  /* Save image as PNG file */
  gdk_pixbuf_save( pixbuf,
      ((save_data_t *)save_data)->filename, "png", &error, NULL );
  g_object_unref( pixbuf );

  return( FALSE );

} /* Save_Pixbuf() */

/*-----------------------------------------------------------------------*/

/* Motion_Event()
 *
 * Handles pointer motion event on drawingareas.
 *
 * Drag state is set by on_*_drawingarea_button_press_event and cleared
 * by on_*_drawingarea_button_release_event.  This handler only
 * accumulates pointer deltas; observers handle redraw and spin display.
 */
  void
Motion_Event(
    GdkEventMotion *event,
    view_t *v )
{
  SetFlag( BLOCK_MOTION_EV );
  view_update_drag( v, (float)event->x, (float)event->y );
  ClearFlag( BLOCK_MOTION_EV );

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
  {
    freqplots_redraw_all(TRUE);
  }

} /* Plot_Select() */

/*-----------------------------------------------------------------------*/

/* Nec2_Edit_Save()
 *
 * Prompts user to save NEC2 data if edited
 */
  gboolean
Nec2_Edit_Save( void )
{
  if( isFlagSet(NEC2_EDIT_SAVE) )
  {
    if( nec2_save_dialog == NULL )
    {
      nec2_save_dialog = create_nec2_save_dialog( &nec2_save_dialog_builder );
      gtk_widget_show( nec2_save_dialog );
    }
    return( TRUE );
  }
  else return( FALSE );

} /* Nec2_Edit_Save() */

/*-----------------------------------------------------------------------*/

/* Delete_Event()
 *
 * Handles user request to delete a window
 */
  void
Delete_Event( gchar *mesg )
{
  quit_dialog = create_quit_dialog( &quit_dialog_builder );
  gtk_widget_show( quit_dialog );

  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
    if( isFlagSet(MAIN_QUIT) )
      gtk_label_set_text( GTK_LABEL(
            Builder_Get_Object(quit_dialog_builder, "quit_label")),
          _("The frequency loop is running\n"
            "Are you sure you wish to end the calculation?") );
    else gtk_label_set_text( GTK_LABEL(
          Builder_Get_Object(quit_dialog_builder, "quit_label")),
        _("The frequency loop is running\n"
          "Are you sure you wish to close this window?") );
  }
  else gtk_label_set_text( GTK_LABEL(
        Builder_Get_Object(quit_dialog_builder, "quit_label")), mesg );

} /* Delete_Event() */

/*-----------------------------------------------------------------------*/

/* Set_Pol_Menuitem()
 *
 * Sets the polarization type menuitem to current setting
 */
  void
Set_Pol_Menuitem( window_t window )
{
  gchar *main_pol_menu[NUM_POL] =
  {
    "main_total",
    "main_horizontal",
    "main_vertical",
    "main_right_hand",
    "main_left_hand",
  };

  gchar *freqplots_pol_menu[NUM_POL] =
  {
    "freqplots_total",
    "freqplots_horizontal",
    "freqplots_vertical",
    "freqplots_right_hand",
    "freqplots_left_hand",
  };

  gchar *rdpattern_pol_menu[NUM_POL] =
  {
    "rdpattern_total",
    "rdpattern_horizontal",
    "rdpattern_vertical",
    "rdpattern_right_hand",
    "rdpattern_left_hand",
  };

  switch( window )
  {
    case MAIN_WINDOW:
      gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
            Builder_Get_Object(main_window_builder,
              main_pol_menu[calc_data.pol_type])), TRUE );
      break;

    case FREQPLOTS_WINDOW:
      gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
            Builder_Get_Object(freqplots_window_builder,
              freqplots_pol_menu[calc_data.pol_type])), TRUE );
      break;

    case RDPATTERN_WINDOW:
      gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
            Builder_Get_Object(rdpattern_window_builder,
              rdpattern_pol_menu[calc_data.pol_type])), TRUE );
      break;
  } /* switch( window ) */

} /* Set_Pol_Menuitem() */

/*-----------------------------------------------------------------------*/

/* Open_Editor()
 *
 * Pops up an Editor window on user
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
  size_t s = strlen( card ) + 1;

  /* Ignore cards from comments */
  if( (strcmp(card, "CM") == 0) ||
      (strcmp(card, "CE") == 0) )
    return( TRUE );

  /* SY cards use a dedicated text editor (no glade button) */
  if( strcmp(card, "SY") == 0 )
  {
    Sy_Card_Editor( EDITOR_EDIT );
    g_free(card);
    return( TRUE );
  }

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
  button = Builder_Get_Object( nec2_editor_builder, card );
  g_free(card);
  if( button != NULL )
    g_signal_emit_by_name( button, "clicked" );
  else return( FALSE );

  return( TRUE );
} /* Open_Editor() */

/*-----------------------------------------------------------------------*/

/* Card_Clicked()
 *
 * Performs actions needed when a "card" editor is to be opened
 */
  void
Card_Clicked(
    GtkWidget **editor,
    GtkBuilder **editor_builder,
    GtkWidget *create_fun(GtkBuilder **),
    void editor_fun(int),
    int *editor_action )
{

  if( isFlagSet(EDITOR_QUIT) )
  {
    if( *editor ) editor_fun( EDITOR_APPLY );
    ClearFlag( EDITOR_QUIT );
    return;
  }

  if( *editor == NULL )
  {
    *editor = create_fun( editor_builder );
    gtk_widget_show( *editor );
  }
  else editor_fun( EDITOR_APPLY );

  editor_fun( *editor_action );
  *editor_action = EDITOR_NEW;

}/* Card_Clicked() */

/*-----------------------------------------------------------------------*/

/* Main_Rdpattern_Activate()
 *
 * Callback function for the Radiation Pattern draw button
 */
  void
Main_Rdpattern_Activate( gboolean from_menu )
{
  /* Set structure overlay in Rad Pattern window */
  if( isFlagClear(OVERLAY_STRUCT) )
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object(rdpattern_window_builder,
            "rdpattern_overlay_structure")), FALSE );
  else
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object(rdpattern_window_builder,
            "rdpattern_overlay_structure")), TRUE );

  /* Sync common projection spinbuttons from the master view. */
  if( rdpattern_view != NULL && rdpattern_view->rotation_master != NULL )
  {
    view_update_spin_display( rdpattern_view );
  }
  else if( rdpattern_view != NULL )
  {
    /* Initialize radiation pattern projection from its spin widgets */
    double wr = gtk_spin_button_get_value( rotate_rdpattern );
    double wi = gtk_spin_button_get_value( incline_rdpattern );
    view_set_angles( rdpattern_view, wr, wi );
  }

  /* Reset zoom value */
  if( rdpattern_view != NULL )
    view_set_zoom( rdpattern_view,
        (float)(gtk_spin_button_get_value( rdpattern_zoom ) / 100.0) );

  /* Redo currents if not reaching this function
   * from the menu callback (e.g. not user action) */
  if( !from_menu ) fetch_freq_data();

  /* Display frequency in freq spinbutton */
  if( from_menu && calc_data.FR_cards )
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(rdpattern_frequency), calc_data.freq_mhz );

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
    /* If window is already open, close it automatically */
    if( freqplots_window != NULL )
    {
      Gtk_Widget_Destroy( &freqplots_window );
      Stop( ERR_OK, _("Frequency plots window closed: excitation type %d is incompatible.\n"
            "Use type 0 (Applied E Field) or 5 (Current Discontinuity) instead."), fpat.ixtyp );
    }
    else
    {
      /* Window not open - show detailed error message */
      Stop( ERR_OK, _("Frequency plots are not available with excitation type %d.\n"
            "This model uses Incident Field or Elementary Current Source excitation.\n"
            "Change the EX card to type 0 (Applied E Field) or 5 (Current Discontinuity)."), fpat.ixtyp );
    }
    return( FALSE );
  }

  /* Enable freq data graph plotting */
  SetFlag( PLOT_ENABLED );

  return( TRUE );
} /* Main_Freqplots_Activate() */

/*-----------------------------------------------------------------------*/

/** rdpattern_trigger_redraw() - Shared tail for rdpattern mode activation:
 *  fetch frequency data when drawing is enabled and no frequency loop is
 *  running, then update window labels.
 */
  static void
rdpattern_trigger_redraw(void)
{
  if( isFlagSet(DRAW_ENABLED) && isFlagClear(FREQ_LOOP_RUNNING) )
    fetch_freq_data();

  Set_Window_Labels();
}

/*-----------------------------------------------------------------------*/

/* Rdpattern_Gain_Togglebutton_Toggled()
 *
 * Callback function for Rad Pattern window Gain button
 */
  void
Rdpattern_Gain_Togglebutton_Toggled( gboolean flag )
{
  if( flag )
  {
    SetFlag( DRAW_GAIN );
    ClearFlag( DRAW_EHFIELD );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
          Builder_Get_Object(rdpattern_window_builder, "rdpattern_eh_togglebutton")),
        FALSE );

    /* No RP card: flags are set so the renderer shows the status message;
     * skip data operations that require valid radiation pattern data */
    if( isFlagClear(ENABLE_RDPAT) )
    {
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
      return;
    }

    /* Enable gain (radiation) pattern plotting */
    rdpattern_trigger_redraw();
  }
  else
  {
    /* Disable gain pattern; clear drawingarea if no mode active */
    ClearFlag( DRAW_GAIN );
    if( isFlagClear(DRAW_EHFIELD) && isFlagSet(DRAW_ENABLED) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
    Free_Draw_Buffers();
  }
} /* Rdpattern_Gain_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Rdpattern_EH_Togglebutton_Toggled()
 *
 * Callback function for Rad Pattern window E/H field button
 */
  void
Rdpattern_EH_Togglebutton_Toggled( gboolean flag )
{
  if( flag )
  {
    SetFlag( DRAW_EHFIELD );
    ClearFlag( DRAW_GAIN );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Builder_Get_Object(
            rdpattern_window_builder, "rdpattern_gain_togglebutton")), FALSE );

    /* No NE/NH cards: flags are set so the renderer shows the status message;
     * skip data operations that require valid near-field configuration */
    if( !fpat.nfeh )
    {
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
      return;
    }

    /* Delegate near field calculations to child
     * processes if forked and near field data valid */
    if( FORKED )
      Alloc_Nearfield_Buffers(fpat.nrx, fpat.nry, fpat.nrz);

    /* Mark nearfield data stale before triggering redraw;
     * flag must be set inside the same guard as fetch_freq_data */
    if( isFlagSet(DRAW_ENABLED) && isFlagClear(FREQ_LOOP_RUNNING) )
      SetFlag( DRAW_NEW_EHFIELD );

    /* Enable E/H field plotting */
    rdpattern_trigger_redraw();
  }
  else
  {
    /* Disable E/H field; clear drawingarea if no mode active */
    ClearFlag( DRAW_EHFIELD );
    if( isFlagClear(DRAW_GAIN) && isFlagSet(DRAW_ENABLED) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
  }
} /* Rdpattern_EH_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Alloc_Crnt_Buffs()
 *
 * Allocates memory for current/charge draw buffers
 */
  static void
Alloc_Crnt_Buffs( void )
{
  /* Patch currents buffer */
  if( data.m > 0 )
  {
    mem_array_realloc(&ct1m, data.m);
    mem_array_realloc(&ct2m, data.m);
  }

  /* Segment currents buffer */
  if( data.n > 0 )
  {
    mem_array_realloc(&cmag, data.n);
  }

} /* Alloc_Crnt_Buffs() */

/*-----------------------------------------------------------------------*/

/* Free_Crnt_Buffs()
 *
 * Frees current/charge draw buffers
 */
  static void
Free_Crnt_Buffs( void )
{
  mem_free(&ct1m);
  mem_free(&ct2m);
  mem_free(&cmag);
} /* Free_Crnt_Buffs() */

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
    Alloc_Crnt_Buffs();

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(Builder_Get_Object(
            main_window_builder, "main_charges_togglebutton")), FALSE );
    gtk_label_set_text(GTK_LABEL(Builder_Get_Object(
            main_window_builder, "struct_label")), _("View Currents") );

    if( fetch_freq_data() )
      xnec2_widget_queue_draw( structure_drawingarea, TRUE );

    if( isFlagSet(OVERLAY_STRUCT) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
  }
  else
  {
    ClearFlag( DRAW_CURRENTS );
    if( isFlagClear(DRAW_CHARGES) )
    {
      /* Redraw structure on screen if frequency loop is not running */
      gtk_label_set_text( GTK_LABEL(
            Builder_Get_Object(main_window_builder, "struct_label")),
          _("View Geometry") );
      if( isFlagClear(FREQ_LOOP_RUNNING) )
      {
        xnec2_widget_queue_draw( structure_drawingarea, TRUE );
      }
      Free_Crnt_Buffs();
    }
    if( isFlagSet(OVERLAY_STRUCT) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
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
    Alloc_Crnt_Buffs();

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(
          Builder_Get_Object(main_window_builder, "main_currents_togglebutton")),
        FALSE );
    gtk_label_set_text(GTK_LABEL(
          Builder_Get_Object(main_window_builder, "struct_label")),
        _("View Charges") );

    if( fetch_freq_data() )
      xnec2_widget_queue_draw( structure_drawingarea, TRUE );

    if( isFlagSet(OVERLAY_STRUCT) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
  }
  else
  {
    ClearFlag( DRAW_CHARGES );
    if( isFlagClear(DRAW_CURRENTS) )
    {
      /* Redraw structure on screen if frequency loop is not running */
      gtk_label_set_text(
          GTK_LABEL(Builder_Get_Object(
              main_window_builder, "struct_label")), _("View Geometry") );

      if( isFlagClear(FREQ_LOOP_RUNNING) )
      {
        xnec2_widget_queue_draw( structure_drawingarea, TRUE );
      }

      Free_Crnt_Buffs();
    }

    if( isFlagSet(OVERLAY_STRUCT) )
      xnec2_widget_queue_draw( rdpattern_drawingarea, TRUE );
  }

} /* Main_Charges_Togglebutton_Toggled() */

/*-----------------------------------------------------------------------*/

/* Open_Filechooser()
 *
 * Opens the file chooser dialog to select open or save files
 */
  GtkWidget *
Open_Filechooser(
    GtkFileChooserAction action,
    char *pattern,
    char *prefix,
    char *filename,
    char *foldername )
{
  /* Create file chooser and set action */
  GtkBuilder *builder;
  GtkWidget *chooser = create_filechooserdialog( &builder );
  gtk_file_chooser_set_action( GTK_FILE_CHOOSER(chooser), action );

  /* Create and set a filter for the file pattern */
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern( filter, pattern );

  /* Set the filter name */
  if( strcmp(pattern, "*.png") == 0 )
    gtk_file_filter_set_name( filter, _("PNG images (*.png)") );
  else if( strcmp(pattern, "*.gplot") == 0 )
    gtk_file_filter_set_name( filter, _("GNUplot files (*.gplot)") );
  else if( strcmp(pattern, "*.nec") == 0 )
    gtk_file_filter_set_name( filter, _("NEC2 files (*.nec)") );
  else if( strcmp(pattern, "*.s1p") == 0 )
    gtk_file_filter_set_name( filter, _("Touchstone 1-port files (*.s1p)") );
  else if( strcmp(pattern, "*.s2p") == 0 )
    gtk_file_filter_set_name( filter, _("Touchstone 2-port files (*.s2p)") );
  else if( strcmp(pattern, "*.csv") == 0 )
    gtk_file_filter_set_name( filter, _("Comma-separated CSV files (*.csv)") );

  /* Add and set filter */
  gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(chooser), filter );
  gtk_file_chooser_set_filter( GTK_FILE_CHOOSER(chooser), filter );

  /* Set current filename if given */
  if( filename != NULL )
  {
    char fname[144];
    if( prefix != NULL )
    {
      Strlcpy( fname, rc_config.working_dir, sizeof(fname) );
      Strlcat( fname, filename, sizeof(fname) );
    }
    else
      Strlcpy( fname, filename, sizeof(fname) );

    gtk_file_chooser_set_current_name(
        GTK_FILE_CHOOSER(chooser), fname );
  }

  /* Set folder name if given */
  if( foldername != NULL )
    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(chooser), foldername );
  gtk_widget_show( chooser);
  g_object_unref( builder );

  return( chooser );

} /* Open_Filechooser() */

/*-----------------------------------------------------------------------*/

/* Filechooser_Response()
 *
 * Handles the on_filechooserdialog_response callback
 */
  void
Filechooser_Response(
    GtkDialog *dialog,
    gint response_id,
    int saveas_width,
    int saveas_height )
{
  /* User selects a file name to save a pixbuf to file */
  if( response_id == GTK_RESPONSE_OK )
  {
    gchar *fname;
    gchar filename[LINE_LEN];
    gboolean new;
    char *str;

    /* Get the "save as" file name */
    fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(dialog) );
    Strlcpy( filename, fname, sizeof(filename) );
    Gtk_Widget_Destroy( (GtkWidget **)&dialog );

    if( isFlagSet(NEC2_SAVE) )
    {
      /* Cat a file extension if not already there */
      str = strstr( filename, ".nec" );
      if( (str == NULL) || (str[4] != '\0') )
        Strlcat( filename, ".nec", sizeof(filename) );

      /* Use new file name as input file */
      Strlcpy( rc_config.input_file, filename, sizeof(rc_config.input_file) );
      Save_Nec2_Input_File( nec2_edit_window, rc_config.input_file );

      /* Re-open NEC2 input file */
      new = FALSE;
      if( Nec2_Apply_Checkbutton() && isFlagClear(MAIN_QUIT) )
        Open_Input_File( (gpointer)(&new) );
    }
    else if( isFlagSet(OPEN_INPUT) )
    {
      ClearFlag( FREQ_LOOP_READY );

      /* Save any changes to an open file */
      Strlcpy( rc_config.input_file, fname, sizeof(rc_config.input_file) );

      /* Open new file */
      new = TRUE;
      Open_Input_File( (gpointer)(&new) );
      ClearFlag( OPEN_INPUT );
    }
    else if( isFlagSet(IMAGE_SAVE) )
    {
      /* cat a file extension if not already there */
      str = strstr( filename, ".png" );
      if( (str == NULL) || (str[4] != '\0') )
        Strlcat( filename, ".png", sizeof(filename) );

      /* Save screen shots after redraw and when GTK is finished tasks */
      static save_data_t save_data;

      xnec2_widget_queue_draw( saveas_drawingarea, TRUE );

      save_data.drawingarea = saveas_drawingarea;
      save_data.width  = saveas_width;
      save_data.height = saveas_height;
      Strlcpy( save_data.filename, filename, sizeof(save_data.filename) );
      g_idle_add( Save_Pixbuf, (gpointer)&save_data );
      ClearFlag( IMAGE_SAVE );
    }
    else if( isFlagSet(RDPAT_GNUPLOT_SAVE) )
    {
      /* cat a file extension if not already there */
      str = strstr( filename, ".gplot" );
      if( (str == NULL) || (str[6] != '\0') )
        Strlcat( filename, ".gplot", sizeof(filename) );
      Save_RadPattern_Gnuplot_Data( filename );
      ClearFlag( RDPAT_GNUPLOT_SAVE );
    }
    else if( isFlagSet(PLOTS_GNUPLOT_SAVE) )
    {
      /* cat a file extension if not already there */
      str = strstr( filename, ".gplot" );
      if( (str == NULL) || (str[6] != '\0') )
        Strlcat( filename, ".gplot", sizeof(filename) );
      Save_FreqPlots_Gnuplot_Data( filename );
      ClearFlag( PLOTS_GNUPLOT_SAVE );
    }
    else if( isFlagSet(STRUCT_GNUPLOT_SAVE) )
    {
      /* cat a file extension if not already there */
      str = strstr( filename, ".gplot" );
      if( (str == NULL) || (str[6] != '\0') )
        Strlcat( filename, ".gplot", sizeof(filename) );
      Save_Struct_Gnuplot_Data( filename );
      ClearFlag( STRUCT_GNUPLOT_SAVE );
    }
    else if ( filechooser_callback != NULL )
    {
      str = strstr( filename, filechooser_callback->extension );
      if( (str == NULL) || (str[strlen(filechooser_callback->extension)] != '\0') )
        Strlcat( filename, filechooser_callback->extension, sizeof(filename) );
      filechooser_callback->callback(filename);
      mem_free(&filechooser_callback);
    }
    g_free( fname );

    /* Open file chooser if user requested an input file to be opened */
    if( isFlagSet(OPEN_INPUT) )
    {
      file_chooser = Open_Filechooser(
          GTK_FILE_CHOOSER_ACTION_OPEN,
          "*.nec", NULL, NULL, rc_config.working_dir );
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

    /* Save GUI state data for restoring
     * windows if user is quitting xnec2c */
    if( isFlagSet(MAIN_QUIT) )
    {
      Get_GUI_State();
      Save_Config();
    }

    /* Kill window that initiated edited data save */
    Gtk_Widget_Destroy( &kill_window );

  } /* if( response_id == GTK_RESPONSE_OK ) */
  else
  {
    mem_free(&filechooser_callback);
    ClearFlag( ALL_CHOOSER_FLAGS );
  }

} /* Filechooser_Response() */

/*-----------------------------------------------------------------------*/

/* Open_Nec2_Editor()
 *
 * Opens NEC2 editor window and fills
 * tree view according to action flag
 */
  void
Open_Nec2_Editor( int action )
{
  nec2_edit_window = create_nec2_editor( &nec2_editor_builder );
  wl_session_register_window( nec2_edit_window, "nec2_edit" );
  Set_Window_Geometry( nec2_edit_window,
      rc_config.nec2_edit_x, rc_config.nec2_edit_y,
      rc_config.nec2_edit_width, rc_config.nec2_edit_height );
  gtk_widget_show( nec2_edit_window );
  Update_Window_Titles();

  cmnt_treeview = GTK_TREE_VIEW(
      Builder_Get_Object(nec2_editor_builder, "nec2_cmnt_treeview") );
  geom_treeview = GTK_TREE_VIEW(
      Builder_Get_Object(nec2_editor_builder, "nec2_geom_treeview") );
  cmnd_treeview = GTK_TREE_VIEW(
      Builder_Get_Object(nec2_editor_builder, "nec2_cmnd_treeview") );

  Nec2_Input_File_Treeview( action );

  geom_adjustment =
    gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(
          Builder_Get_Object(nec2_editor_builder, "geom_scrolledwindow")) );
  cmnd_adjustment =
    gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(
          Builder_Get_Object(nec2_editor_builder, "cmnd_scrolledwindow")) );

  Update_Window_Titles();

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
          Builder_Get_Object(nec2_editor_builder, "nec2_apply_checkbutton" ))) );
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
      kill( child_procs[num_child_procs]->pid, SIGKILL );
    }

  /* Release prerender allocations */
  prerender_state_free();
  free_struct_colors();

  /* Program-wide managed-allocator snapshot at teardown */
  if( rc_config.mem_report_enabled )
    mem_report("Gtk_Quit");

  /* Kill possibly nested loops */
  k = (int)gtk_main_level();
  for( i = 0; i < k; i++ )
    gtk_main_quit();

} /* Gtk_Quit() */

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

/* Draw_Colorcode( cairo_t *cr )
 *
 * Draws the color code bar fopr structure
 * currents and radiation pattern gain range
 */
  void
Draw_Colorcode( cairo_t *cr )
{
  double red = 0.0, grn = 0.0, blu = 0.0;
  int idx;

  /* Draw color-code bar in main window */
  for( idx = 0; idx < COLORCODE_WIDTH; idx++ )
  {
    Value_to_Color( &red, &grn, &blu, (double)(8 * idx), COLORCODE_MAX );
    cairo_set_source_rgb( cr, red, grn, blu );
    Cairo_Draw_Line( cr, idx, 0, idx, COLORCODE_HEIGHT );
  }

} /* Draw_Colorcode() */

/*-----------------------------------------------------------------------*/

/* Gtk_Widget_Destroy()
 *
 * A safety wrapper around gtk_widget_destroy()
 * Checks that the widget is not null before killing it
 */
  void
Gtk_Widget_Destroy( GtkWidget **widget )
{
  if( *widget != NULL )
  {
    gtk_widget_destroy( *widget );
    *widget = NULL;
  }

} /* Gtk_Widget_Destroy() */

/*------------------------------------------------------------------*/

