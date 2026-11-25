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

#include "main.h"
#include "args.h"
#include "validation_dump.h"
#include "shared.h"
#include "mathlib.h"
#include "structure_canvas.h"
#include "structure_view.h"
#include "signal_handler.h"
#include "cairo/cairo_fit.h"
#include "config_hooks.h"
#include "config/config_frequency.h"
#include "config/widget/config_widget_ops.h"
#include "themes/theme.h"
#include "ui_style.h"

#include <getopt.h>

/* Child process pid returned by fork() */
static pid_t child_pid = (pid_t)(-1);

static gint opt_start_optimizer_thread(void)
{
	GtkWidget *w;

	w = Builder_Get_Object(main_window_builder, "optimizer_output");
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(w), TRUE);

	return FALSE;
}

/*------------------------------------------------------------------------*/

char *orig_numeric_locale = NULL;

  int
main (int argc, char *argv[])
{
  int idx, err;

  /*** Signal handler related code ***/
  /* new and old actions for sigaction() */
  struct sigaction sa_new, sa_old;

  // Print all notices that may occur before getopt parsing:
  rc_config.verbose = 9;

  /* initialize new actions */
  sa_new.sa_handler = sig_handler;
  sigemptyset( &sa_new.sa_mask );
  sa_new.sa_flags = 0;

  // Setup locales so we can switch between C and the system locale.
  // The pointer returned from setlocale() seems to be stack sensitive
  // so make a copy of it for later:
  setlocale(LC_ALL, "");
  char *l = setlocale(LC_NUMERIC, NULL);
  orig_numeric_locale = mem_strdup(l);

  // Initialize gettext for internationalization
  const char *localedir = getenv("XNEC2C_LOCALEDIR");
  if (!localedir) localedir = PROGRAMNAME_LOCALEDIR;
  bindtextdomain(PACKAGE, localedir);
  textdomain(PACKAGE);

  /* Register function to handle signals */
  sigaction( SIGINT,  &sa_new, &sa_old );
  sigaction( SIGSEGV, &sa_new, NULL );
  sigaction( SIGFPE,  &sa_new, NULL );
  sigaction( SIGTERM, &sa_new, NULL );
  sigaction( SIGABRT, &sa_new, NULL );
  sigaction( SIGCHLD, &sa_new, NULL );

  /* Use the non-fatal initializer so informational options such as
   * --help and --version remain usable without a display (e.g. when
   * DISPLAY and WAYLAND_DISPLAY are both unset).  A missing display is
   * only fatal once the GUI is actually needed; see the check before
   * create_main_window() below. */
  gboolean gtk_ok = gtk_init_check( &argc, &argv );

  /* Create a default config if needed, abort on error */
  if( !Create_Default_Config() ) exit( -1 );

  /* Report the GDK windowing backend; the GL view gates its
   * native-window isolation on X11 (see gl_view_surface_new()). */
  {
    GdkDisplay *display = gdk_display_get_default();
    const char *type = display ? G_OBJECT_TYPE_NAME(display) : NULL;
    const char *backend;
    if( type && g_strcmp0(type, "GdkX11Display") == 0 )
      backend = "X11";
    else if( type && g_strcmp0(type, "GdkWaylandDisplay") == 0 )
      backend = "Wayland";
    else if( !gtk_ok || !display )
      backend = _("none (no display)");
    else
      backend = type;
    pr_info("Detected %s windowing backend\n", backend);
  }

  /* Process command line options */
  args_parse(argc, argv);

  if (rc_config.batch_mode && isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS))
  {
	  pr_crit("--batch and --optimize are mutual exclusive.\n");
	  exit(1);
  }

  /* Check the PNG targets once all arguments are parsed, honoring --batch
   * regardless of its position */
  batch_png_require_batch();

  /* Initialize the external math libraries */
  init_mathlib();

  /* Read input file path name if not supplied by -i option */
  while (strlen(rc_config.input_file) == 0 && optind < argc)
  {
    if (strstr(argv[optind], ".nec") || strstr(argv[optind], ".NEC"))
    {
      size_t siz = sizeof(rc_config.input_file);

      if (strlen(argv[optind]) >= siz)
      {
        pr_crit("input file path name too long ( > %d char )\n", (int) siz - 1);
        exit(1);
      }

      Strlcpy(rc_config.input_file, argv[optind], siz);
    }
    else
      pr_warn("unexpected argument '%s' does not appear to be a .nec file\n", argv[optind]);

    optind++;
  }

  while (optind < argc)
  {
    pr_warn("unexpected argument: %s\n", argv[optind]);
    optind++;
  }

  if (strlen(rc_config.input_file) == 0 && rc_config.batch_mode)
  {
	  pr_crit("batch mode requires an input file\n");
	  exit(1);
  }

  /* The GUI requires a display; informational options (--help,
   * --version) have already run and exited above, so anything reaching
   * here needs GTK initialized.  Fail cleanly before forking workers or
   * touching GTK when DISPLAY/WAYLAND_DISPLAY are unset. */
  if( !gtk_ok )
  {
    pr_crit("cannot open display: a running X11 or Wayland session is required\n");
    exit( 1 );
  }

  /* Windows resolve their styling once, at construction, so the application
   * stylesheet enters the screen ahead of the first window. */
  ui_style_init();

  /* When forking is useful, e.g. if more than 1 processor is
   * available, the parent process handles the GUI and delegates
   * calculations to the child processes, one per processor. The
   * requested number of child processes = number of processors */

  /* Allocate buffers for fork data */
  if( calc_data.num_jobs > 0 )
  {
    mem_array_alloc(&child_procs, calc_data.num_jobs);
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      child_procs[idx] = NULL;
      mem_new(&child_procs[idx]);
    }

    pr_info("Forking %d jobs across %d processors.\n",
        calc_data.num_jobs, xnec2c_num_procs());
    /* Fork child processes */
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      child_procs[idx]->idx           = idx;
      child_procs[idx]->assigned_step = -1;

      /* Make pipes to transfer data */
      err = pipe( child_procs[idx]->to_child );
      if( err )
      {
        perror( "pipe()" );
        pr_crit("exiting after fatal error: pipe() failed");
        exit(-1);
      }

      err = pipe( child_procs[idx]->from_child );
      if( err )
      {
        perror( "pipe()" );
        pr_crit("exiting after fatal error: pipe() failed");
        exit(-1);
      }

      /* Fork child process */
      child_procs[idx]->pid = fork();
      if( child_procs[idx]->pid == -1 )
      {
        perror( "fork()" );
        pr_crit("exiting after fatal error: fork() failed");
        exit(-1);
      }
      else child_pid = child_procs[idx]->pid;

      /* Child get out of forking loop! */
      if( CHILD ) Child_Process( idx );

      /* Close unwanted pipe ends (parent keeps to_child[WRITE] and from_child[READ]) */
      close( child_procs[idx]->to_child[READ] );
      close( child_procs[idx]->from_child[WRITE] );

      /* Count child processes */
      num_child_procs++;
    } /* for( idx = 0; idx < calc_data.num_jobs; idx++ ) */

    FORKED = TRUE;
  } /* if( calc_data.num_jobs > 0 ) */
  else
  {
    /* No fork: one in-process worker. The length-one child_procs array holds
     * the parent's own slot so the serial path indexes child_procs[0] like the
     * forked path; pid=0 and -1 pipe fds mark it as non-forked. */

    /* Resolve the no-fork request into the single worker count that every
     * frequency-loop bound reads. */
    calc_data.num_jobs = 1;

    mem_array_alloc(&child_procs, 1);
    child_procs[0] = NULL;
    mem_new(&child_procs[0]);
    child_procs[0]->idx           = 0;
    child_procs[0]->pid           = 0;
    child_procs[0]->to_child[0]   = -1;
    child_procs[0]->to_child[1]   = -1;
    child_procs[0]->from_child[0] = -1;
    child_procs[0]->from_child[1] = -1;
    child_procs[0]->assigned_step = -1;
    child_procs[0]->assigned_freq = 0.0;
  }

  /* Create the main window */
  main_window = create_main_window( &main_window_builder );
  gtk_window_set_title( GTK_WINDOW(main_window), PACKAGE_STRING );

  /* The transport buttons carry the sweep state, so a freshly built window
   * takes its face and its tooltips from the readout. */
  freq_sweep_controls_refresh();

  calc_data.zo = 50.0;
  calc_data.freq_loop_data = NULL;

  /* Register session-only and rc_config_vars-rooted widget bindings
   * before any config value is loaded or restored. */
  config_frequency_init();
  rc_config_register_widgets();

  /* Load the theme registry ahead of every theme_active() consumer */
  theme_registry_init();

  /* Read GUI state config file and reset geometry */
  Read_Config();

  /* Build the palettes and canvas border from the persisted theme selection
   * before any canvas draws */
  theme_derived_rebuild();

  /* Write the stored values into the widgets, reached whether or not a
   * config file was read */
  Restore_GUI_State();

  /* Batch captures render off-screen from built windows, so a batch run
   * presents no window and starts its sweep once the input file loads */
  if (rc_config.batch_mode == TRUE)
	  rc_config.main_loop_start = 1;
  else if (rc_config.batch_mode == FALSE)
	  gui_present();
  else
	  BUG("batch_mode=%d is neither TRUE nor FALSE\n", rc_config.batch_mode);

  /* If input file is specified, get the working directory */
  if( strlen(rc_config.input_file) )
    Get_Dirname( rc_config.input_file, rc_config.working_dir, NULL );

  /* Main window freq spinbutton */
  config_widget_capture( &calc_data.fmhz_save );

  /* Create the structure view before the GL widget; observers are
   * installed inside opengl_structure_surface_new and fire during
   * subsequent viewport/angle assignments. */
  structure_view_create();

  /* Force deterministic config for reproducible validation dumps.  Runs after
   * Read_Config() so it overrides rc-file settings, and after structure_view is
   * created so it can reset the viewer to its default orientation. */
  validation_dump_force_config();

  structure_surfaces_add();

#ifdef HAVE_OPENGL
  /* A command-line engine replaces the renderer choice Read_Config() stored */
  render_engine_request_apply();

  opengl_set_renderer(rc_config.use_opengl_renderer);
#else
  canvas_set_engine( CANVAS_STRUCTURE, &cairo_engine );

  config_widget_set_visible( &rc_config.opengl_orthographic, FALSE );
#endif

  /* Record the presented surface allocation as the first structure
   * viewport, once the renderer choice has selected that surface */
  if( !canvas_sync_viewport( CANVAS_STRUCTURE ) )
    BUG("canvas %d presents no surface\n", (int)CANVAS_STRUCTURE);

  /* Signal start of xnec2c */
  SetFlag( XNEC2C_START );

  /* Open input file if specified */
  gboolean new = TRUE;
  if( strlen(rc_config.input_file) > 0 )
    g_idle_add( Open_Input_File, (gpointer)(&new) );

  init_mathlib_menu();

  if (isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS))
	  g_idle_add_once((GSourceOnceFunc)opt_start_optimizer_thread, NULL);

  gtk_main ();

  /* Release every mem-tracked owner by name, then emit the report. */
  parent_cleanup();

  return 0;
} // main()

/*-----------------------------------------------------------------------*/

/* Tests for child process */
  gboolean
isChild(void)
{
  return( child_pid == (pid_t)(0) );
}

/*------------------------------------------------------------------------*/
