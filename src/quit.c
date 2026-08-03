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

/*
 *  Application quit state machine.
 *
 *  Interactive entry points (Quit menu, window delete, Ctrl-Q, error
 *  dialog, confirm dialog) route through the one coordinator
 *  xnec2c_request_quit().  Successful batch completion from either
 *  frequency-loop driver posts capture and teardown to the GTK main thread.
 *  That endpoint delivers xnec2c_quit() after the sweep has quiesced;
 *  it does not re-enter the interactive coordinator.
 *
 *  MAIN_QUIT is the single quit-state datum.  It is set as intent at the
 *  interactive boundary (Nec2_Edit_Save and the confirm-dialog re-prompt read
 *  it upstream of the coordinator) and set again unconditionally in the
 *  coordinator; the coordinator's bit-or is idempotent over the boundary set
 *  and is never conditioned on optimizer liveness.
 *
 *  The coordinator quiesces two coupled request-phase subsystems by optimizer
 *  liveness.  The optimizer worker drives the frequency loop through
 *  nec2_eval, so stopping the loop directly while the optimizer runs would
 *  join a thread that still needs the live main loop and deadlock:
 *
 *    running -> opt_cancel(); return.  The worker settles its own loop and
 *      fires its generic completion notifier; opt_finished (opt_ui_session.c),
 *      the registered on_complete handler, resolves the pending quit via
 *      xnec2c_quit_if_pending() -> xnec2c_quit().
 *    idle    -> Stop_Frequency_Loop(); xnec2c_quit(NULL) at once.
 *
 *  Worker completion is event-based, never polled.  The worker fires
 *  on_complete via a single g_idle_add_once after it has cleared running and
 *  finished all nec2_eval work; that lone cross-thread edge lands on the
 *  serialized GTK main thread.  Both the coordinator and opt_finished run on
 *  that thread, and running transitions monotonically true->false under the
 *  worker's best_lock, so the completion is race-free.
 *
 *  Every path converges on xnec2c_quit(), which destroys main_window and
 *  enters the destroy chain (on_main_window_destroy -> Gtk_Quit ->
 *  gtk_main_quit).  When gtk_main() returns, parent_cleanup() (callback_func.c)
 *  is the one post-loop teardown; its opt_shutdown() pthread_joins a worker
 *  that has already exited.  The inotify watcher and forked children join
 *  without the main loop, so parent_cleanup remains their single teardown site.
 */

#include "shared.h"
#include "optimizers/opt_session.h"

/*-----------------------------------------------------------------------*/

/**
 * xnec2c_quit - complete application shutdown on the GTK main thread
 * @user_data: unused callback data
 *
 * The coordinator, optimizer completion, successful normal-batch completion,
 * and the synchronous batch-error path converge here after workers quiesce.
 */
  void
xnec2c_quit( gpointer user_data )
{
  (void)user_data;

  Gtk_Widget_Destroy( &main_window );

} /* xnec2c_quit() */

/*-----------------------------------------------------------------------*/

/* xnec2c_quit_if_pending()
 *
 * Boundary resolver run by opt_finished on the worker-completion event: if a
 * quit is pending on MAIN_QUIT, complete it via the shared primitive and
 * report TRUE so the caller skips its finished-transition UI restore.
 */
  gboolean
xnec2c_quit_if_pending( void )
{
  if( isFlagClear( MAIN_QUIT ) ) return( FALSE );

  xnec2c_quit( NULL );
  return( TRUE );

} /* xnec2c_quit_if_pending() */

/*-----------------------------------------------------------------------*/

/* xnec2c_request_quit()
 *
 * Single interactive quit coordinator.  A synchronous teardown deadlocks while
 * the optimizer runs: the worker evaluates on the GTK main thread via
 * g_idle_add_once_sync and settles only while the main loop runs.  So set the
 * quit intent unconditionally, then dispatch on optimizer liveness: a running
 * optimizer is cancelled and its completion delivers the quit through
 * opt_finished; otherwise the standalone frequency loop is stopped and quit
 * completes at once.
 */
  void
xnec2c_request_quit( void )
{
  SetFlag( MAIN_QUIT );

  if( opt_is_running() )
  {
    opt_cancel();
    return;
  }

  Stop_Frequency_Loop();
  xnec2c_quit( NULL );

} /* xnec2c_request_quit() */

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

  close_child_command_pipes();

  /* Kill possibly nested loops */
  k = (int)gtk_main_level();
  for( i = 0; i < k; i++ )
    gtk_main_quit();

} /* Gtk_Quit() */

/*-----------------------------------------------------------------------*/

/* main_windows_destroy()
 *
 * Destroys every top-level window, the SY overrides window included, to
 * halt drawing and run each window's destroy chain before engine and view
 * data are freed.  The SY window's chain frees its renderer.
 */
  void
main_windows_destroy( void )
{
  Gtk_Widget_Destroy( &sy_overrides_window );
  Gtk_Widget_Destroy( &nec2_edit_window );
  Gtk_Widget_Destroy( &freqplots_window );
  Gtk_Widget_Destroy( &rdpattern_window );
  Gtk_Widget_Destroy( &main_window );

} /* main_windows_destroy() */

/*-----------------------------------------------------------------------*/
