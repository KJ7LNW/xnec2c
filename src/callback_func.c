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
#include "prerender/prerender_state.h"
#include "prerender/prerender_color.h"
#include "color/color_palette.h"
#include "chroma/chroma.h"
#include "structure_ui.h"
#include "mem/mem_track.h"

#include "sy_expr.h"
#include "optimizers/opt_session.h"

/*-----------------------------------------------------------------------*/

/* Save_Pixbuf()
 *
 * Saves pixbufs as png files
 */
  gboolean
Save_Pixbuf( gpointer save_data )
{
  save_data_t *data = save_data;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  pixbuf = canvas_capture(data->canvas, data->width, data->height);
  if( pixbuf == NULL )
  {
    pr_err("failed to capture image for %s\n", data->filename);
    return FALSE;
  }

  if( !gdk_pixbuf_save(pixbuf, data->filename, "png", &error, NULL) )
  {
    pr_err("failed to save image %s: %s\n", data->filename, error->message);
    g_error_free(error);
  }

  g_object_unref(pixbuf);
  return FALSE;

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

  if(freq_sweep_active())
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
  /* Feedpoint-undefined excitations still plot the radiation-derived panels;
   * notify the user that the VSWR and impedance panels are suppressed. */
  if( !fpat_has_feedpoint() )
    pr_notice(_("Excitation type %d has no feedpoint: VSWR and impedance plots are hidden.\n"), fpat.ixtyp);

  /* Enable freq data graph plotting; per-series predicates suppress the
   * feedpoint-undefined panels while the radiation-derived panels render. */
  SetFlag( PLOT_ENABLED );

  return( TRUE );
} /* Main_Freqplots_Activate() */

/*-----------------------------------------------------------------------*/

/** rdpattern_mode_apply() - Apply the radiation-pattern field mode
 *
 * Config post_apply hook for rc_config.rdpattern_mode.  Runs at window
 * create, on restore, and on every field-mode toggle change; the enum
 * member selects far-field gain, near E/H field, or neither.  Without valid
 * card data the renderer shows the status message on the queued redraw.
 */
  void
rdpattern_mode_apply( void )
{
  gboolean have_data;

  if( rc_config.rdpattern_mode == RDPAT_FIELD_GAIN )
    have_data = isFlagSet( ENABLE_RDPAT );
  else if( rc_config.rdpattern_mode == RDPAT_FIELD_EHFIELD )
    have_data = ( fpat.nfeh != 0 );
  else if( rc_config.rdpattern_mode == RDPAT_FIELD_DISABLED )
    have_data = FALSE;
  else
  {
    BUG("rdpattern_mode_apply: invalid rdpattern_mode=%d\n",
        rc_config.rdpattern_mode);
    have_data = FALSE;
  }

  if( have_data )
  {
    if( isFlagSet(DRAW_ENABLED) && !freq_sweep_active())
      fetch_freq_data();
  }
  else
    Queue_Radiation_Redraw( TRUE );

  Set_Window_Labels();
} /* rdpattern_mode_apply() */

/*-----------------------------------------------------------------------*/

/** structure_view_apply() - Apply the main-window structure view
 *
 * Config post_apply hook for rc_config.structure_view.  Runs at window
 * create, on restore, and on every view radio change; the enum member
 * selects geometry, charges, or currents.  Currents and charges fetch the
 * step data.  The rad-pattern structure overlay tracks the same view.
 */
  void
structure_view_apply( void )
{
  const char *label;

  if( rc_config.structure_view == STRUCT_VIEW_CURRENTS )
    label = _("View Currents");
  else if( rc_config.structure_view == STRUCT_VIEW_CHARGES )
    label = _("View Charges");
  else /* STRUCT_VIEW_DISABLED: geometry only */
    label = _("View Geometry");

  gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
          main_window_builder, "struct_label")), label );

  if( rc_config.structure_view == STRUCT_VIEW_DISABLED )
  {
    /* Geometry view: redraw the structure if a frequency loop is not
     * running */
    if(!freq_sweep_active())
      Queue_Structure_Rebuild( TRUE );
  }
  else
  {
    /* Currents or charges: fetch the step data; a cache hit rebuilds the
     * structure through freq_step_update_ui() */
    fetch_freq_data();
  }

  /* The rad-pattern structure overlay tracks the structure view */
  if( overlay_struct_active() )
    Queue_Radiation_Redraw( TRUE );

} /* structure_view_apply() */

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

      canvas_queue_redraw( saveas_canvas, TRUE );

      save_data.canvas = saveas_canvas;
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

/* engine_buffers_free()
 *
 * Frees every mem-tracked owner shared by parent and child: engine data and
 * scratch buffers, the symbol table, child_procs (inherited across fork), and
 * the locale, then emits the report.  Both cleanup paths end here.
 */
  static void
engine_buffers_free( void )
{
  /* Free the engine data buffers owned by parent and child alike. */
  input_data_free();
  geometry_data_free();
  ggrid_free();
  calc_data_free();

  /* Free the engine scratch buffers kept in file-scope statics. */
  fields_data_free();
  ground_data_free();
  somnec_data_free();
  matrix_data_free();
  calc_scratch_free();
  gnuplot_data_free();

  /* Free the symbol table now that every reader has stopped. */
  sy_cleanup();

  /* child_procs is inherited across fork(); both processes free their copy. */
  child_procs_free();

  mem_free( &orig_numeric_locale );

  /* Emit the report now; an empty registry makes any survivor an ownership bug. */
  if( rc_config.mem_report_enabled )
    mem_report("exit");

} /* engine_buffers_free() */

/*-----------------------------------------------------------------------*/

/* parent_cleanup()
 *
 * Parent teardown, run once after gtk_main() returns.  Stops both optimizers
 * and tears down the windows, views, and frequency-plot state the child never
 * builds, then frees the shared engine buffers.
 */
  void
parent_cleanup( void )
{
  /* Stop both optimizers before any structure they read is torn down:
   * opt_shutdown cancels and joins the built-in simplex/PSO worker;
   * optimizer_output_stop signals the external inotify watcher's run flag
   * and joins it. */
  opt_shutdown();
  optimizer_output_stop();

  /* Destroy every top-level window to halt drawing and run each destroy
   * chain; the SY window's chain frees its renderer. */
  main_windows_destroy();

  /* Release the structure surfaces, whose widgets lived inside the main
   * window and so reached no canvas-clearing destroy chain of their own.
   * Every widget is finalized by now, so each engine releases what its
   * unrealize left behind. */
  canvas_clear( CANVAS_STRUCTURE );

  /* Free the views and prerendered colors that no widget can reach now. */
  view_free( &structure_view );
  view_free( &rdpattern_view );
  prerender_state_free();
  free_struct_colors();

  /* Free the frequency-plot views before the freq_loop_data array their
   * entries point into. */
  freqplots_cleanup();

  engine_buffers_free();

} /* parent_cleanup() */

/*-----------------------------------------------------------------------*/

/* child_cleanup()
 *
 * Forked-child teardown, run from child_exit() on command-pipe EOF.  The child
 * builds no windows, views, or optimizers, so it frees only the shared engine
 * buffers and emits its report.
 */
  void
child_cleanup( void )
{
  engine_buffers_free();

} /* child_cleanup() */

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

/**
 * Draw_Colorcode() - Draw the radiation pattern gain legend strip
 * @cr: cairo context of the radiation-pattern color-code drawing area
 *
 * Sweeps the magnitude ramp linearly across the strip so each column
 * carries the color the far-field surface shows at that fraction of its
 * gain range.
 */
  void
Draw_Colorcode( cairo_t *cr )
{
  const palette_t *ramp = palette_get( PALETTE_RAMP );
  int idx;

  for( idx = 0; idx < COLORCODE_WIDTH; idx++ )
  {
    double gain_fraction =
      (double)idx / (double)(COLORCODE_WIDTH - 1);

    cairo_set_source_rgb_f( cr,
        palette_lookup_scaled( ramp, gain_fraction, 1.0 ) );
    Cairo_Draw_Line( cr, idx, 0, idx, COLORCODE_HEIGHT );
  }

} /* Draw_Colorcode() */

/*-----------------------------------------------------------------------*/

/**
 * draw_colorcode_projected() - Draw the structure legend strip
 * @cr: cairo context of the main-window color-code drawing area
 *
 * Colorizes each strip position through the active color projection and
 * scale so the legend matches the displayed wire/patch mapping; bounds
 * for the dB floor come from the displayed quantity's cmin/cmax.
 */
  void
draw_colorcode_projected( cairo_t *cr )
{
  chroma_proj_t proj = color_proj_active();
  color_tone_t fam = color_tone_active();
  const chroma_proj_row_t *row = &chroma_proj_rows[proj];
  const palette_t *pal = palette_get(chroma_proj_palette_kind(row->hue_enc));
  tone_param_t tp;
  int idx;

  tone_param_init( &tp, fam );

  for( idx = 0; idx < COLORCODE_WIDTH; idx++ )
  {
    double p = (double)idx / (double)(COLORCODE_WIDTH - 1);
    /* Ramp maps the value axis through the tone transfer s(n); the diverging
     * and cyclic wheels sweep their domain linearly. */
    double coord = (row->hue_enc == HUE_MAG_RAMP)
        ? color_tones[fam].transfer( p, &tp ) : p;
    rgb_f_t c = palette_lookup_scaled( pal, coord, 1.0 );

    cairo_set_source_rgb( cr, c.r, c.g, c.b );
    Cairo_Draw_Line( cr, idx, 0, idx, COLORCODE_HEIGHT );
  }

} /* draw_colorcode_projected() */

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

