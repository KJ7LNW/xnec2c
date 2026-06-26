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
#include "shared.h"
#include "gdk_scroll.h"
#include "wl_session.h"
#include "mathlib.h"
#include "structure_ui.h"
#include "opengl/opengl_structure.h"
#include "cairo/cairo_draw.h"

/* Forward declaration — full sy_overrides.h conflicts with openblas via gsl */
extern void sy_overrides_close_if_empty(void);

#include <getopt.h>

static void sig_handler(int signal);

/* Child process pid returned by fork() */
static pid_t child_pid = (pid_t)(-1);

enum XNEC2C_OPTS {
	// Start at 128 after all single-digit opts:
	OPT_FIRST_OPT = 128,

	OPT_ENABLE_OPTIMIZE,

	OPT_OPENBLAS_THREADS,
	OPT_MKL_THREADS,
	OPT_OMP_THREADS,

	OPT_WRITE_CSV,
	OPT_WRITE_S1P,
	OPT_WRITE_S2P_MAX_GAIN,
	OPT_WRITE_S2P_VIEWER_GAIN,
	OPT_WRITE_RDPAT,
	OPT_WRITE_CURRENTS,
	OPT_WRITE_GNUPLOT_STRUCTURE,
	OPT_WRITE_PATCH_CURRENTS,
	OPT_SKIP_VERIFY,
	OPT_FORCE_VERIFY,
	OPT_MEM_REPORT,

	OPT_MAX_OPTS
};

static struct option long_options[] = {
		{  "input",                  required_argument,   NULL,  'i'                        },
		{  "config",                 required_argument,   NULL,  'c'                        },
		{  "new-config",             required_argument,   NULL,  'C'                        },
		{  "jobs",                   required_argument,   NULL,  'j'                        },
		{  "help",                   no_argument,         NULL,  'h'                        },
		{  "verbose",                no_argument,         NULL,  'v'                        },
		{  "debug",                  no_argument,         NULL,  'd'                        },
		{  "quiet",                  no_argument,         NULL,  'q'                        },
		{  "version",                no_argument,         NULL,  'V'                        },
		{  "no-pthreads",            no_argument,         NULL,  'P'                        },
		{  "batch",                  no_argument,         NULL,  'b'                        },

		{  "optimize",               no_argument,         NULL,  OPT_ENABLE_OPTIMIZE        },

		{  "openblas-threads",       required_argument,   NULL,  OPT_OPENBLAS_THREADS       },
		{  "mkl-threads",            required_argument,   NULL,  OPT_MKL_THREADS            },
		{  "omp-threads",            required_argument,   NULL,  OPT_OMP_THREADS            },

		{  "write-csv",              required_argument,   NULL,  OPT_WRITE_CSV              },
		{  "write-s1p",              required_argument,   NULL,  OPT_WRITE_S1P              },
		{  "write-s2p-max-gain",     required_argument,   NULL,  OPT_WRITE_S2P_MAX_GAIN     },
		{  "write-s2p-viewer-gain",  required_argument,   NULL,  OPT_WRITE_S2P_VIEWER_GAIN  },
		{  "write-rdpat",            required_argument,   NULL,  OPT_WRITE_RDPAT            },
		{  "write-currents",         required_argument,   NULL,  OPT_WRITE_CURRENTS         },
		{  "write-gnuplot-structure", required_argument,  NULL,  OPT_WRITE_GNUPLOT_STRUCTURE },
		{  "write-patch-currents",   required_argument,   NULL,  OPT_WRITE_PATCH_CURRENTS   },
		{  "skip-verify",            no_argument,         NULL,  OPT_SKIP_VERIFY            },
		{  "force-verify",           no_argument,         NULL,  OPT_FORCE_VERIFY           },
		{  "mem-report",             no_argument,         NULL,  OPT_MEM_REPORT             },

		{  NULL,                     0,                   NULL,  0                          }
	};

static char *build_optstring(struct option *long_options)
{
	static char optstring[256] = {0};
	char c[2] = {0,0};

	int i;
	for (i = 0; long_options[i].name != NULL; i++)
	{
		if (long_options[i].val < OPT_FIRST_OPT)
			c[0] = (char)long_options[i].val;
		else
			continue;

		strcat(optstring, c);

		if (long_options[i].has_arg == required_argument)
			strcat(optstring, ":");
		else if (long_options[i].has_arg == optional_argument)
			strcat(optstring, "::");
	}

	return optstring;
}

static gint opt_start_optimizer_thread(void)
{
	GtkWidget *w;

	w = Builder_Get_Object(main_window_builder, "optimizer_output");
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(w), TRUE);

	return FALSE;
}

static void validate_thread_count(const char *option_name, const char *optarg)
{
	char *endptr;
	long val = strtol(optarg, &endptr, 10);

	if (*endptr != '\0' || endptr == optarg || val < 1)
	{
		pr_crit("%s requires a positive integer argument\n", option_name);
		exit(1);
	}
}

/*------------------------------------------------------------------------*/

char *orig_numeric_locale = NULL;

  int
main (int argc, char *argv[])
{
  /* getopt() variables */
  int option, idx, err;
  int enable_forking = 1;

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
  mem_alloc(&orig_numeric_locale, strlen(l) + 1);
  strcpy(orig_numeric_locale, l);

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

  /* Process command line options */
#ifdef HAVE_OPENMP
  calc_data.num_jobs = omp_get_num_procs();
  pr_info("Detected %d CPUs\n", calc_data.num_jobs);
#else 
  calc_data.num_jobs = 1;
#endif

  /* Report the GDK windowing backend; the GL view gates its
   * native-window isolation on X11 (see gl_view_create_widget()). */
  {
    GdkDisplay *display = gdk_display_get_default();
    const char *type = display ? G_OBJECT_TYPE_NAME(display) : NULL;
    const char *backend;
    if( type && g_strcmp0(type, "GdkX11Display") == 0 )
      backend = "X11";
    else if( type && g_strcmp0(type, "GdkWaylandDisplay") == 0 )
      backend = "Wayland";
    else if( !gtk_ok || !display )
      backend = "none (no display)";
    else
      backend = type;
    pr_info("Detected %s windowing backend\n", backend);

#ifndef HAVE_GDK_WAYLAND_GET_XDG_TOPLEVEL
    if( g_strcmp0(type, "GdkWaylandDisplay") == 0 )
      pr_notice("Wayland position restore needs GTK >= 3.24.53; ask your distribution to build GTK with gdk_wayland_window_get_xdg_toplevel\n");
#endif
  }

  rc_config.input_file[0] = '\0';

  /* Initialize default config file path */
  char home[PATH_MAX];
  get_conf_dir(home, sizeof(home));
  if (snprintf(rc_config.config_file, sizeof(rc_config.config_file),
        "%s/%s", home, DEFAULT_CONFIG_FILE) >= (int)sizeof(rc_config.config_file)) {
    pr_err("config file path too long\n");
    exit(EXIT_FAILURE);
  }

  // default to show warnings or more important errors.
  rc_config.verbose = 4;

  setenv("OPENBLAS_NUM_THREADS", "1", 0);
  setenv("MKL_NUM_THREADS", "1", 0);
  setenv("OMP_NUM_THREADS", "1", 0);

  int option_index = 0;
  while( (option = getopt_long(argc, argv, build_optstring(long_options), long_options, &option_index) ) != -1 )
  {
    switch( option )
    {
      case 'c': /* specify existing config file path */
        if (access(optarg, R_OK) < 0) {
          pr_crit("config file does not exist or is not readable: %s\n", optarg);
          exit(-1);
        }
        /* fall through */
      case 'C': /* specify new config file path */
        {
          size_t siz = sizeof( rc_config.config_file );
          if( strlen(optarg) >= siz )
          {
            pr_crit("config file path too long ( > %d char )\n", (int)siz - 1);
            exit(-1);
          }
          Strlcpy( rc_config.config_file, optarg, siz );
        }
        break;

      case 'i': /* specify input file name */
        {
          size_t siz = sizeof( rc_config.input_file );
          if( strlen(optarg) >= siz )
          {
            pr_crit("input file name too long ( > %d char )\n", (int)siz - 1);
            exit(-1);
          }
          /* For null term. */
          Strlcpy( rc_config.input_file, optarg, siz );
        }
        break;

      case 'v': /* increase verbosity */
        rc_config.verbose++;
        break;

      case 'd': /* debug */
        rc_config.debug++;
        rc_config.verbose += 3;
        break;

      case 'q': /* quiet */
        rc_config.debug = 0;
        rc_config.verbose = 0;
        break;

      case 'j': /* number of child processes = num of processors */
        calc_data.num_jobs = atoi( optarg );

        if (calc_data.num_jobs == 0)
        {
            pr_notice("Forking disabled!\n");
            enable_forking = 0;
            calc_data.num_jobs = 1;
        }
        break;

      case 'P': /* disable pthread loop */
        rc_config.disable_pthread_freqloop = 1;
        pr_notice("pthread freqloop disabled!\n");
        break;

      case 'b': /* batch mode */
        pr_notice("batch mode enabled, will exit after first loop\n");
        rc_config.batch_mode = 1;
        break;

      case 'h': /* print usage and exit */
        usage();
        exit(0);
        break;

      case 'V': /* print xnec2c version */
        puts( PACKAGE_STRING );
        exit(0);
        break;

      case OPT_ENABLE_OPTIMIZE:
          SetFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
          break;

      case OPT_OPENBLAS_THREADS:
        validate_thread_count("--openblas-threads", optarg);
        setenv("OPENBLAS_NUM_THREADS", optarg, 1);
        break;

      case OPT_MKL_THREADS:
        validate_thread_count("--mkl-threads", optarg);
        setenv("MKL_NUM_THREADS", optarg, 1);
        break;

      case OPT_OMP_THREADS:
        validate_thread_count("--omp-threads", optarg);
        setenv("OMP_NUM_THREADS", optarg, 1);
        break;

      case OPT_WRITE_CSV:
        rc_config.filename_csv = optarg;
        break;

      case OPT_WRITE_S1P:
        rc_config.filename_s1p = optarg;
        break;

      case OPT_WRITE_S2P_MAX_GAIN:
        rc_config.filename_s2p_max_gain = optarg;
        break;

      case OPT_WRITE_S2P_VIEWER_GAIN:
        rc_config.filename_s2p_viewer_gain = optarg;
        break;

      case OPT_WRITE_RDPAT:
        rc_config.filename_rdpat = optarg;
        break;

      case OPT_WRITE_CURRENTS:
        rc_config.filename_currents = optarg;
        break;

      case OPT_WRITE_GNUPLOT_STRUCTURE:
        rc_config.filename_gnuplot_structure = optarg;
        break;

      case OPT_WRITE_PATCH_CURRENTS:
        rc_config.filename_patch_currents = optarg;
        break;

      case OPT_SKIP_VERIFY:
        pr_notice("verify segments check disabled\n");
        rc_config.skip_verify_segments = 1;
        break;

      case OPT_FORCE_VERIFY:
        pr_notice("forcing overlap check on large models\n");
        rc_config.force_verify_segments = 1;
        break;

      case OPT_MEM_REPORT:
        pr_notice("managed allocator leak report enabled\n");
        rc_config.mem_report_enabled = 1;
        break;

      default:
        usage();
        exit(0);
        break;

    } /* switch( option ) */
  } /* while( (option = getopt(argc, argv, "i:o:hv") ) != -1 ) */

  if (rc_config.batch_mode && isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS))
  {
	  pr_crit("--batch and --optimize are mutual exclusive.\n");
	  exit(1);
  }

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

  /* When forking is useful, e.g. if more than 1 processor is
   * available, the parent process handles the GUI and delegates
   * calculations to the child processes, one per processor. The
   * requested number of child processes = number of processors */

  /* Allocate buffers for fork data */
  if( calc_data.num_jobs >= 1 && enable_forking )
  {
    mem_array_alloc(&child_procs, calc_data.num_jobs);
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      child_procs[idx] = NULL;
      mem_new(&child_procs[idx]);
    }

    pr_info("Forking %d jobs.\n", calc_data.num_jobs);
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
  } /* if( calc_data.num_jobs > 1 ) */
  else
  {
    /* No fork: one in-process worker. The length-one child_procs array holds
     * the parent's own slot so the serial path indexes child_procs[0] like the
     * forked path; pid=0 and -1 pipe fds mark it as non-forked. */
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

  /* Wire the session restore hook before the window realizes; the
   * xdg-toplevel-realized signal fires once during the first map, so the
   * connection must precede gtk_widget_show in Restore_GUI_State. */
  wl_session_register_window( main_window, "main" );

  calc_data.zo = 50.0;
  calc_data.freq_loop_data = NULL;

  /* Read GUI state config file and reset geometry */
  Read_Config();

  if (rc_config.batch_mode)
	  rc_config.main_loop_start = 1;

  /* If input file is specified, get the working directory */
  if( strlen(rc_config.input_file) )
    Get_Dirname( rc_config.input_file, rc_config.working_dir, NULL );

  /* Main window freq spinbutton */
  mainwin_frequency = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_freq_spinbutton") );

  /* Get the structure drawing area and allocation */
  structure_cairo_da =
    Builder_Get_Object( main_window_builder, "structure_drawingarea" );

  GtkAllocation allocation;
  gtk_widget_get_allocation( structure_cairo_da, &allocation );
  structure_width  = allocation.width;
  structure_height = allocation.height;

  /* Spin buttons and entry backing the structure view */
  rotate_structure  = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_rotate_spinbutton") );
  incline_structure = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_incline_spinbutton") );
  scroll_install_spin_notches( rotate_structure,  SCROLL_ANGLE_INCREMENT );
  scroll_install_spin_notches( incline_structure, SCROLL_ANGLE_INCREMENT );
  structure_zoom = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_zoom_spinbutton") );
  structure_fstep_entry = GTK_ENTRY(
      Builder_Get_Object(main_window_builder, "structure_fstep_entry") );

  /* Create the structure view before the GL widget; observers are
   * installed inside opengl_structure_create_widget and fire during
   * subsequent viewport/angle assignments. */
  structure_view = view_new( VIEW_STRUCTURE,
      rotate_structure, incline_structure, structure_zoom,
      structure_view_changed_cb, NULL );
  view_set_spin_handlers( structure_view,
      G_CALLBACK(on_main_rotate_spinbutton_value_changed),
      G_CALLBACK(on_main_incline_spinbutton_value_changed) );
  view_set_viewport( structure_view, structure_width, structure_height );
  view_set_angles( structure_view,
      gtk_spin_button_get_value( rotate_structure ),
      gtk_spin_button_get_value( incline_structure ) );
  view_set_drag_mode( structure_view,
      rc_config.view_drag_constrained
          ? VIEW_DRAG_CONSTRAINED : VIEW_DRAG_FREE );

  /* Create GL widget after the view is initialized */
#ifdef HAVE_OPENGL
  {
    GtkWidget *box = Builder_Get_Object(main_window_builder, "structure_box");

    structure_gl_area = opengl_structure_create_widget();
    gtk_box_pack_start(GTK_BOX(box), structure_gl_area, TRUE, TRUE, 0);

    opengl_set_renderer(rc_config.use_opengl_renderer);
  }
#else
  structure_drawingarea = structure_cairo_da;

  hide_widget_by_id(main_window_builder, "main_ortho_button");
#endif

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

  mem_free(&orig_numeric_locale);

  return 0;
} // main()

/*-----------------------------------------------------------------------*/

/* Open_Input_File()
 *
 * Opens NEC2 input file
 */
  gboolean
Open_Input_File( gpointer arg )
{
  static char prev_input_file[PATH_MAX] = "";
  gboolean ok, new;
  GtkWidget *widget;

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

  /* Always call Stop_Frequency_Loop: the thread clears FREQ_LOOP_RUNNING
   * on normal exit, so checking the flag alone would skip cleanup,
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

  /* Zero validity flags and clear FREQ_LOOP_DONE under lock so draw
   * and save handlers cannot observe stale fstep=1 paired with
   * freshly-allocated garbage rad_pattern from Alloc_Rdpattern_Buffers
   * inside Read_Commands. */
  ClearFlag( FREQ_LOOP_DONE );
  if( ok && save.fstep != NULL )
    for( int i = 0; i <= calc_data.steps_total; i++ )
      save.fstep[i] = 0;

  g_rec_mutex_unlock(&freq_data_lock);
  if( !ok )
  {
    /* Close plot/rdpat windows if open */
    Gtk_Widget_Destroy( &rdpattern_window );
    Gtk_Widget_Destroy( &freqplots_window );

    if( nec2_edit_window == NULL )
      Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
    else
      Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );

    return( FALSE );
  } /* if( !ok ) */

  /* Serialization relies on GTK single-threadedness: inotify dispatches
   * Open_Input_File to the GTK thread via g_idle_add; Stop_Frequency_Loop
   * at entry joins any compute thread before proceeding. */

  gtk_widget_show( Builder_Get_Object(main_window_builder, "optimizer_output") );

  /* Ask child processes to read input file */
  if( FORKED )
  {
    int idx;
    size_t lenc, leni;

    lenc = strlen( fork_commands[INFILE] );
    leni = sizeof( rc_config.input_file );
    for( idx = 0; idx < num_child_procs; idx++ )
    {
      Write_Pipe( idx, fork_commands[INFILE], (ssize_t)lenc, TRUE );
      Write_Pipe( idx, rc_config.input_file,  (ssize_t)leni, TRUE );
    }
  } /* if( FORKED ) */

  /* Initialize xnec2c */
  SetFlag( FREQ_APPLY );
  if( isFlagSet(PLOT_ENABLED) ) SetFlag( FREQ_LOOP_INIT );
  floop_tag = 0;

  New_Frequency_Reset_Prev();

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
   * frequency) so the saved position survives startup initialization.
   * Block value-changed to prevent the callback from overwriting fmhz_save. */
  double display_freq = (calc_data.fmhz_save > 0.0)
      ? calc_data.fmhz_save : calc_data.freq_mhz;
  SIGNAL_BLOCK(mainwin_frequency, on_freq_spinbutton_value_changed);
  gtk_spin_button_set_value( mainwin_frequency, display_freq );
  SIGNAL_UNBLOCK(mainwin_frequency, on_freq_spinbutton_value_changed);

  /* Show main control buttons etc */
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox2") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_grid1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "structure_frame") );

  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_view_menuitem") );
  gtk_widget_show( structure_drawingarea );

  /* Set input file to NEC2 editor. It will only
   * happen if the NEC2 editor window is open */
  new = *( (gboolean *)arg );
  if( new && isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
    Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );
  else
    Nec2_Input_File_Treeview( NEC2_EDITOR_RELOAD );

  /* Re-initialize Rad Pattern drawing if window open */
  if( rdpattern_window != NULL )
  {
    // Don't reset the zoom during optimization:
    if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
    {
      widget = Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_zoom_spinbutton" );

      gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), (gdouble)rc_config.rdpattern_zoom_spinbutton );
    }

    /* Simulate activation of main rdpattern button */
    if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) && !rc_config.main_loop_start)
      Main_Rdpattern_Activate( FALSE );

    /* Select display of radiation or EH pattern */
    if( isFlagSet(DRAW_GAIN) )
    {
      if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) && !rc_config.main_loop_start)
        Rdpattern_Gain_Togglebutton_Toggled( TRUE );
    }
    else if( isFlagSet(DRAW_EHFIELD) )
    {
      if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
        Rdpattern_EH_Togglebutton_Toggled( TRUE );
    }
    else
    {
      Rdpattern_Gain_Togglebutton_Toggled( FALSE );
      Rdpattern_EH_Togglebutton_Toggled( FALSE );
    }

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
  }

  /* Re-initiate frequency plots if window open */
  if( isFlagSet(PLOT_ENABLED) )
  {
    GtkWidget *box = Builder_Get_Object( freqplots_window_builder, "freqplots_box" );
    gtk_widget_show( box );
    if( rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
      Main_Freqplots_Activate();
  }

  /* Restore main window projection settings */
  if( isFlagSet(XNEC2C_START) )
  {
    widget = Builder_Get_Object(
        main_window_builder, "main_rotate_spinbutton" );
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), (gdouble)rc_config.main_rotate_spinbutton );

    widget = Builder_Get_Object(
        main_window_builder, "main_incline_spinbutton" );
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), (gdouble)rc_config.main_incline_spinbutton );

    widget = Builder_Get_Object(
        main_window_builder, "main_zoom_spinbutton" );
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), (gdouble)rc_config.main_zoom_spinbutton );

    ClearFlag( XNEC2C_START );
  }

  if( rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
    Start_Frequency_Loop();

  /* Open NEC2 editor if there is a saved geometry state */
  GtkMenuItem *menu_item;
  if( rc_config.nec2_edit_width &&
      rc_config.nec2_edit_height )
  {
    menu_item = GTK_MENU_ITEM(
        Builder_Get_Object(main_window_builder, "nec2_edit") );
    gtk_menu_item_activate( menu_item );
  }


  /* If currents or charges draw button is active
   * re-initialize structure currents/charges drawing */
  if( isFlagSet(DRAW_CURRENTS) )
    Main_Currents_Togglebutton_Toggled( TRUE );
  if( isFlagSet(DRAW_CHARGES) )
    Main_Charges_Togglebutton_Toggled( TRUE );

  /* Redraw structure with updated geometry regardless of overlay state.
   * During optimization freq_step_update_ui_idle_force handles both
   * invalidation and draws after valid data is ready, so suppress here
   * to prevent expose-driven rebuilds seeing freq_step=-1 (gray flash). */
  if( isFlagClear(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
#ifdef HAVE_OPENGL
    opengl_structure_invalidate();
#endif
    Queue_Structure_Redraw();
  }

  /* Close symbol overrides window if no symbols defined */
  sy_overrides_close_if_empty();

  return( FALSE );
} /* Open_Input_File() */

/*------------------------------------------------------------------------*/

static void sig_handler( int signal )
{
  switch( signal )
  {
    case SIGINT:
      if (!CHILD) pr_crit("exiting via user interrupt\n");
      break;

    case SIGSEGV:
      pr_crit("segmentation fault, exiting\n");
      break;

    case SIGFPE:
      pr_crit("floating point exception, exiting\n");
      break;

    case SIGABRT:
      pr_crit("abort signal received, exiting\n");
      break;

    case SIGTERM:
      pr_crit("termination request received, exiting\n");
      break;

    case SIGCHLD:
      {
        int idx;
        pid_t pid = getpid();

        if( !FORKED )
        {
          pr_crit("not forked, ignoring SIGCHLD from pid %d\n", pid);
          return;
        }
        else
        {
          for( idx = 0; idx < calc_data.num_jobs; idx++ )
            if( child_procs[idx]->pid == pid )
            {
              pr_crit("child process pid %d exited\n", pid);
              if( isFlagSet(MAIN_QUIT) ) return;
              else break;
            }
          return;
        }
      }

    default:
      pr_debug("default exit with signal: %d\n", signal);
  } /* switch( signal ) */

  /* Kill child processes */
  if( FORKED && !CHILD )
    while( num_child_procs )
    {
      num_child_procs--;
      kill( child_procs[num_child_procs]->pid, SIGKILL );
    }

  Close_File( &input_fp );

  if( CHILD ) _exit( 0 );
  else exit( signal );

} /* End of sig_handler() */

/*------------------------------------------------------------------------*/

/* Tests for child process */
  gboolean
isChild(void)
{
  return( child_pid == (pid_t)(0) );
}

/*------------------------------------------------------------------------*/
