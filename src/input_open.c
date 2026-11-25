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

#include "common.h"
#include "callbacks.h"
#include "config/widget/config_widget.h"
#include "fork.h"
#include "nec2_model.h"
#include "shared.h"
#include "window_lifecycle.h"

/* Forward declaration — full sy_overrides.h conflicts with openblas via gsl */
extern void sy_overrides_close_if_empty(void);

/* Open_Input_File()
 *
 * Opens NEC2 input file
 */
  gboolean
Open_Input_File( gpointer arg )
{
  static char prev_input_file[PATH_MAX] = "";
  gboolean ok, new;

  /* Reject reentry while a load holds input_fp, so the Close_File below
   * cannot null the handle mid-parse in a nested Stop() event loop. */
  if( isFlagSet(INPUT_PENDING) )
    return( FALSE );

  /* Suppress activity while input file opened.  Set this before calling
   * Stop_Frequency_Loop() so the Frequency_Loop_Thread() knows not to 
   * make any synchronous GTK calls which would hang waiting for
   * this Open_Input_File() to return, but Open_Input_File() would be
   * waiting for Stop_Frequency_Loop() and deadlock. */
  SetFlag( INPUT_PENDING );

  /* Invalidate freq loop preconditions before Stop_Frequency_Loop so that
   * the GTK event flush inside Stop_Frequency_Loop cannot re-entrantly
   * start a new freq loop via Start_Frequency_Loop_Greenline.  The
   * existing steps_total < 1 guard in freq_loop_start_internal rejects
   * any call that arrives during the flush. */
  g_rec_mutex_lock(&freq_data_lock);
  calc_data.FR_cards    = 0;
  calc_data.steps_total = 0;
  g_rec_mutex_unlock(&freq_data_lock);

  /* Always call Stop_Frequency_Loop: the thread retires the sweep state
   * on normal exit, so checking that state alone would skip cleanup,
   * leaking pth_freq_loop and floop_state.  Stop_Frequency_Loop is
   * idempotent — it checks pth_freq_loop internally and no-ops safely. */
  Stop_Frequency_Loop();

  /* Close open files if any */
  Close_File( &input_fp );


  /* Open NEC2 input file */
  if( strlen(rc_config.input_file) == 0 )
    return( FALSE );

  /* Hold freq_data_lock across data reset and reallocation so draw
   * handlers (which may fire during g_idle_add_once_sync flush loops)
   * cannot access data structures mid-transition. */
  g_rec_mutex_lock(&freq_data_lock);

  calc_data.freq_step = -1;

  mem_array_free(&freqplots_main_view()->fr_plots);

  Open_File( &input_fp, rc_config.input_file, "r");

  /* Read input file, record failures */
  ok = Read_Comments() && Read_Geometry() && Read_Commands();

  /* Zero validity flags and invalidate the result set under lock so draw
   * and save handlers cannot observe stale fstep=1 paired with
   * freshly-allocated garbage rad_pattern from Alloc_Rdpattern_Buffers
   * inside Read_Commands. */
  freq_sweep_results_clear();
  if( ok && save.fstep != NULL )
    for( int i = 0; i <= calc_data.steps_total; i++ )
      save.fstep[i] = 0;

  g_rec_mutex_unlock(&freq_data_lock);
  if( !ok )
  {
    /* Close plot/rdpat windows if open */
    Gtk_Widget_Destroy( &rdpattern_window );
    Gtk_Widget_Destroy( &freqplots_window );

    /* A model that failed to load leaves every detached plot window
     * pinned to ports and cards the read never established. */
    freqplots_destroy_all_popups();

    /* Batch mode has no operator to dismiss the editor; Stop() already
     * scheduled the quit, so opening it here only loops the read/allocate
     * path headlessly. Restrict the editor to interactive sessions. */
    if( !rc_config.batch_mode )
    {
      if( nec2_edit_window == NULL )
        Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
      else
        Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );
    }

    return( FALSE );
  } /* if( !ok ) */

  /* Populate the freqplots excitation-port selector for the loaded model. */
  freqplots_populate_port_combo();

  /* Rebuild open popup port selectors for the reloaded model's ports. */
  freqplots_reload_port_combos();

  /* Gray feedpoint-dependent widgets when the reloaded model has no
   * feedpoint. */
  freqplots_gate_feedpoint_widgets();

  /* Serialization relies on GTK single-threadedness: inotify dispatches
   * Open_Input_File to the GTK thread via g_idle_add; Stop_Frequency_Loop
   * at entry joins any compute thread before proceeding. */

  gtk_widget_show( Builder_Get_Object(main_window_builder, "optimizer_output") );

  /* Ask child processes to read input file */
  if( FORKED )
  {
    int idx;

    for( idx = 0; idx < num_child_procs; idx++ )
      fork_send_infile( idx );
  } /* if( FORKED ) */

  /* Initialize xnec2c */
  rc_config.freq_apply = 1;
  /* A batch run builds no frequency plots window yet always starts the
   * load's sweep, so it arms the sweep as an open window does */
  if( window_is_open(FREQPLOTS_WINDOW) || rc_config.batch_mode ) freq_sweep_arm();
  floop_tag = 0;

  /* Scale geometry for current frequency before potential loop start.
   * During forked frequency loop, children scale geometry independently,
   * but parent's data.segments[].bi must be wavelength-normalized for any
   * intermediate renders from children. */
  Frequency_Scale_Geometry();

  /* Allow re-draws on expose events etc */
  ClearFlag( INPUT_PENDING );

  gboolean same_file = (strcmp(rc_config.input_file, prev_input_file) == 0);

  /* Set projection at 45 deg rotation and inclination if NEC2 editor window
   * is not open, not while optimizing, and only when opening a different file */
  if( (nec2_edit_window == NULL) &&
      isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) &&
      !same_file )
  {
    view_set_angles( structure_view,
        VIEW_DEFAULT_WR, VIEW_DEFAULT_WI );
  }

  /* Update tracked filename for next comparison */
  strncpy(prev_input_file, rc_config.input_file, PATH_MAX - 1);
  prev_input_file[PATH_MAX - 1] = '\0';

  /* Show current frequency; prefer fmhz_save (rc_config restored green-line
   * frequency) so the saved position survives startup initialization.  Route
   * through the field so both frequency spins sync and the frequency applies
   * once via hook_frequency. */
  calc_data.fmhz_save = (calc_data.fmhz_save > 0.0)
      ? calc_data.fmhz_save : calc_data.freq_mhz;
  config_widget_field_changed( &calc_data.fmhz_save );

  /* Show main control buttons etc */
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox2") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_grid1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "structure_frame") );

  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_view_menuitem") );

  /* Set input file to NEC2 editor. It will only
   * happen if the NEC2 editor window is open */
  new = *( (gboolean *)arg );
  if( new && isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
    Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );
  else
    Nec2_Input_File_Treeview( NEC2_EDITOR_RELOAD );

  /* Re-initialize Rad Pattern drawing if window open */
  if( window_is_open(RDPATTERN_WINDOW) )
  {
    // Don't reset the zoom during optimization:
    if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
      view_set_zoom( rdpattern_view,
          (float)(rc_config.rdpattern_zoom_spinbutton / 100.0) );

    /* Simulate activation of main rdpattern button */
    if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) && !rc_config.main_loop_start)
      Main_Rdpattern_Activate( FALSE );

    /* Re-apply the persisted radiation-pattern field mode for the reloaded
     * file; the rc_config enum selects far-field gain or near E/H field */
    if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) && !rc_config.main_loop_start )
      rdpattern_mode_apply();

    GtkWidget *box = Builder_Get_Object( rdpattern_window_builder, "rdpattern_box" );
    gtk_widget_show( box );

#ifdef HAVE_OPENGL
    /* Establish view sharing for common projection on new file load */
    if( !same_file )
    {
      opengl_common_projection_sync();
    }
    /* Preserve existing view when re-opening same file */
#endif

    /* Snap rdpattern pan to match structure on new file load, same
     * condition as projection sync above -- renderer-agnostic, so
     * outside the HAVE_OPENGL guard. */
    if( !same_file )
      Common_Pan_Sync();
  }

  /* Re-initiate frequency plots if window open */
  if( window_is_open(FREQPLOTS_WINDOW) )
  {
    GtkWidget *box = Builder_Get_Object( freqplots_window_builder, "freqplots_box" );
    gtk_widget_show( box );
    if( rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
      Main_Freqplots_Activate();
  }

  /* Restore main window projection settings */
  if( isFlagSet(XNEC2C_START) )
  {
    view_set_angles( structure_view,
        (double)rc_config.main_rotate_spinbutton,
        (double)rc_config.main_incline_spinbutton );

    view_set_zoom( structure_view,
        (float)(rc_config.main_zoom_spinbutton / 100.0) );

    ClearFlag( XNEC2C_START );
  }

  if( rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    if( calc_data.steps_total >= 1 )
      Start_Frequency_Loop();
    else if( rc_config.batch_mode )
      g_idle_add_once( (GSourceOnceFunc)batch_finish_no_steps, NULL );
    else
    {
      /* Read_Commands registers a default FR card, so an accepted deck
       * sweeps at least one step; a non-batch context arriving here with
       * none has no sweep to dispatch and stays interactive. */
    }
  }

  /* Release the pending-start announcement raised for the plots window: a
   * sweep that started has already consumed it, and one that never started
   * would otherwise leave it announced for the rest of the session. */
  if( !freq_sweep_active() )
    freq_sweep_results_clear();
  else
  {
    /* The sweep's own start consumed the announcement. */
  }

  /* Open NEC2 editor if there is a saved geometry state */
  GtkMenuItem *menu_item;
  if( rc_config.nec2_edit_width &&
      rc_config.nec2_edit_height )
  {
    menu_item = GTK_MENU_ITEM(
        Builder_Get_Object(main_window_builder, "nec2_edit") );
    gtk_menu_item_activate( menu_item );
  }


  /* Re-apply the persisted structure view for the reloaded geometry; the
   * rc_config enum selects geometry, currents, or charges */
  structure_view_apply();

  /* Redraw structure with updated geometry regardless of overlay state.
   * During optimization freq_step_update_ui_idle_force draws after valid
   * data is ready, so suppress here to prevent expose-driven rebuilds
   * seeing freq_step=-1 (gray flash). */
  if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    canvas_queue_redraw(CANVAS_STRUCTURE, TRUE);
  }

  /* Close symbol overrides window if no symbols defined */
  sy_overrides_close_if_empty();

  return( FALSE );
} /* Open_Input_File() */
