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
#include "mathlib.h"

#include <getopt.h>

static void sig_handler(int signal);

/* Child process pid returned by fork() */
static pid_t child_pid = (pid_t)(-1);

enum XNEC2C_OPTS {
	// Start at 128 after all single-digit opts:
	OPT_FIRST_OPT = 128,

	OPT_ENABLE_OPTIMIZE,

	OPT_WRITE_CSV,
	OPT_WRITE_S1P,
	OPT_WRITE_S2P_MAX_GAIN,
	OPT_WRITE_S2P_VIEWER_GAIN,

	OPT_MAX_OPTS
};

static struct option long_options[] = {
		{  "input",                  required_argument,   NULL,  'i'                        },
		{  "jobs",                   required_argument,   NULL,  'j'                        },
		{  "help",                   no_argument,         NULL,  'h'                        },
		{  "verbose",                no_argument,         NULL,  'v'                        },
		{  "debug",                  no_argument,         NULL,  'd'                        },
		{  "quiet",                  no_argument,         NULL,  'q'                        },
		{  "version",                no_argument,         NULL,  'V'                        },
		{  "no-pthreads",            no_argument,         NULL,  'P'                        },
		{  "batch",                  no_argument,         NULL,  'b'                        },

		{  "optimize",               no_argument,         NULL,  OPT_ENABLE_OPTIMIZE        },

		{  "write-csv",              required_argument,   NULL,  OPT_WRITE_CSV              },
		{  "write-s1p",              required_argument,   NULL,  OPT_WRITE_S1P              },
		{  "write-s2p-max-gain",     required_argument,   NULL,  OPT_WRITE_S2P_MAX_GAIN     },
		{  "write-s2p-viewer-gain",  required_argument,   NULL,  OPT_WRITE_S2P_VIEWER_GAIN  },

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

static gint opt_start_optimizer_thread()
{
	GtkWidget *w;

	w = Builder_Get_Object(main_window_builder, "main_freqplots");
	if (!gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(w)))
	{
		Notice(_("Xnec2c Optimize"),
			_("Frequency plots window is not configured. "
			"Select your desired graphs and click the triangle \"Play\" button."), GTK_BUTTONS_OK);
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(w), TRUE);
	}

	w = Builder_Get_Object(main_window_builder, "optimizer_output");
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(w), TRUE);

	return FALSE;
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
  bindtextdomain(GETTEXT_PACKAGE, PROGRAMNAME_LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  char *l = setlocale(LC_NUMERIC, NULL);
  mem_alloc((void**)&orig_numeric_locale, strlen(l)+1, __LOCATION__);
  strcpy(orig_numeric_locale, l);

  /* Register function to handle signals */
  sigaction( SIGINT,  &sa_new, &sa_old );
  sigaction( SIGSEGV, &sa_new, NULL );
  sigaction( SIGFPE,  &sa_new, NULL );
  sigaction( SIGTERM, &sa_new, NULL );
  sigaction( SIGABRT, &sa_new, NULL );
  sigaction( SIGCHLD, &sa_new, NULL );

  gtk_init (&argc, &argv);

  /* Create a default config if needed, abort on error */
  if( !Create_Default_Config() ) exit( -1 );

  /* Process command line options */
  calc_data.num_jobs  = 1;
  rc_config.input_file[0] = '\0';

  // default to show warnings or more important errors.
  rc_config.verbose = 4; 

  int option_index = 0;
  while( (option = getopt_long(argc, argv, build_optstring(long_options), long_options, &option_index) ) != -1 )
  {
    switch( option )
    {
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
        rc_config.main_loop_start = 1;
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
          SetFlag( OPTIMIZER_OUTPUT );
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

      default:
        usage();
        exit(0);
        break;

    } /* switch( option ) */
  } /* while( (option = getopt(argc, argv, "i:o:hv") ) != -1 ) */

  if (rc_config.batch_mode && isFlagSet(OPTIMIZER_OUTPUT))
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

  /* When forking is useful, e.g. if more than 1 processor is
   * available, the parent process handles the GUI and delegates
   * calculations to the child processes, one per processor. The
   * requested number of child processes = number of processors */

  /* Allocate buffers for fork data */
  if( calc_data.num_jobs >= 1 && enable_forking )
  {
    size_t mreq = (size_t)calc_data.num_jobs * sizeof(forked_proc_data_t *);
    mem_alloc( (void **)&forked_proc_data, mreq, "in main.c" );
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      forked_proc_data[idx] = NULL;
      mreq = sizeof(forked_proc_data_t);
      mem_alloc( (void **)&forked_proc_data[idx], mreq, "in main.c" );
    }

    /* Fork child processes */
    for( idx = 0; idx < calc_data.num_jobs; idx++ )
    {
      /* Make pipes to transfer data */
      err = pipe( forked_proc_data[idx]->pnt2child_pipe );
      if( err )
      {
        perror( "pipe()" );
        pr_crit("exiting after fatal error: pipe() failed");
        exit(-1);
      }

      err = pipe( forked_proc_data[idx]->child2pnt_pipe );
      if( err )
      {
        perror( "pipe()" );
        pr_crit("exiting after fatal error: pipe() failed");
        exit(-1);
      }

      /* Fork child process */
      forked_proc_data[idx]->child_pid = fork();
      if( forked_proc_data[idx]->child_pid == -1 )
      {
        perror( "fork()" );
        pr_crit("exiting after fatal error: fork() failed");
        exit(-1);
      }
      else child_pid = forked_proc_data[idx]->child_pid;

      /* Child get out of forking loop! */
      if( CHILD ) Child_Process( idx );

      /* Ready to accept a job */
      forked_proc_data[idx]->busy = FALSE;

      /* Close unwanted pipe ends */
      close( forked_proc_data[idx]->pnt2child_pipe[READ] );
      close( forked_proc_data[idx]->child2pnt_pipe[WRITE] );

      /* Set file descriptors for select() */
      FD_ZERO( &forked_proc_data[idx]->read_fds );
      FD_SET( forked_proc_data[idx]->child2pnt_pipe[READ],
          &forked_proc_data[idx]->read_fds );
      FD_ZERO( &forked_proc_data[idx]->write_fds );
      FD_SET( forked_proc_data[idx]->pnt2child_pipe[WRITE],
          &forked_proc_data[idx]->write_fds );

      /* Count child processes */
      num_child_procs++;
    } /* for( idx = 0; idx < calc_data.num_jobs; idx++ ) */

    FORKED = TRUE;
  } /* if( calc_data.num_jobs > 1 ) */

  /* Create the main window */
  main_window = create_main_window( &main_window_builder );
  gtk_window_set_title( GTK_WINDOW(main_window), PACKAGE_STRING );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_hbox1") );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_hbox2") );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_grid1") );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_view_menuitem") );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "structure_frame") );
  gtk_widget_hide( Builder_Get_Object(main_window_builder, "optimizer_output") );
  calc_data.zo = 50.0;
  calc_data.freq_loop_data = NULL;

  /* Read GUI state config file and reset geometry */
  Read_Config();

  /* If input file is specified, get the working directory */
  if( strlen(rc_config.input_file) )
    Get_Dirname( rc_config.input_file, rc_config.working_dir, NULL );

  /* Main window freq spinbutton */
  mainwin_frequency = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_freq_spinbutton") );

  /* Get the structure drawing area and allocation */
  structure_drawingarea =
    Builder_Get_Object( main_window_builder, "structure_drawingarea" );
  GtkAllocation allocation;
  gtk_widget_get_allocation ( structure_drawingarea, &allocation );
  structure_width  = allocation.width;
  structure_height = allocation.height;
  New_Projection_Parameters(
      structure_width, structure_height, &structure_proj_params );

  /* Initialize structure projection angles */
  rotate_structure  = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_rotate_spinbutton") );
  incline_structure = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_incline_spinbutton") );
  structure_zoom = GTK_SPIN_BUTTON(
      Builder_Get_Object(main_window_builder, "main_zoom_spinbutton") );
  structure_fstep_entry = GTK_ENTRY(
      Builder_Get_Object(main_window_builder, "structure_fstep_entry") );

  structure_proj_params.Wr =
    gtk_spin_button_get_value(rotate_structure);
  structure_proj_params.Wi =
    gtk_spin_button_get_value(incline_structure);

  structure_proj_params.xy_zoom = 1.0;
  structure_proj_params.reset = TRUE;
  structure_proj_params.type = STRUCTURE_DRAWINGAREA;

  rdpattern_proj_params.xy_zoom = 1.0;
  rdpattern_proj_params.reset = TRUE;
  rdpattern_proj_params.type = RDPATTERN_DRAWINGAREA;

  /* Signal start of xnec2c */
  SetFlag( XNEC2C_START );

  /* Open input file if specified */
  gboolean new = TRUE;
  if( strlen(rc_config.input_file) > 0 )
    g_idle_add( Open_Input_File, (gpointer)(&new) );
  else
    SetFlag( INPUT_PENDING );

  init_mathlib_menu();

  if (isFlagSet(OPTIMIZER_OUTPUT))
	  g_idle_add_once((GSourceOnceFunc)opt_start_optimizer_thread, NULL);

  gtk_main ();

  free_ptr((void**)&orig_numeric_locale);

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
  gboolean ok, new;
  GtkWidget *widget;

  /* Suppress activity while input file opened.  Set this before calling
   * Stop_Frequency_Loop() so the Frequency_Loop_Thread() knows not to 
   * make any synchronous GTK calls which would hang waiting for
   * this Open_Input_File() to return, but Open_Input_File() would be
   * waiting for Stop_Frequency_Loop() and deadlock. */
  ClearFlag( OPEN_INPUT_FLAGS );
  SetFlag( INPUT_PENDING );

  /* Stop freq loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) )
    Stop_Frequency_Loop();

  /* Close open files if any */
  Close_File( &input_fp );


  /* Open NEC2 input file */
  if( strlen(rc_config.input_file) == 0 )
    return( FALSE );

  g_mutex_lock(&freq_data_lock);
  calc_data.freq_step = -1;
  calc_data.FR_cards    = 0;
  calc_data.FR_index    = 0;
  calc_data.steps_total = 0;
  calc_data.last_step   = 0;

  free_ptr((void**)&fr_plots);
  g_mutex_unlock(&freq_data_lock);

  Open_File( &input_fp, rc_config.input_file, "r");

  /* Read input file, record failures */
  ok = Read_Comments() && Read_Geometry() && Read_Commands();
  if( !ok )
  {
    /* Hide main control buttons etc */
    gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_hbox1") );
    gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_hbox2") );
    gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_grid1") );
    gtk_widget_hide( Builder_Get_Object(main_window_builder, "main_view_menuitem") );
    gtk_widget_hide( Builder_Get_Object(main_window_builder, "structure_frame") );

    /* Close plot/rdpat windows if open */
    Gtk_Widget_Destroy( &rdpattern_window );
    Gtk_Widget_Destroy( &freqplots_window );

    if( nec2_edit_window == NULL )
      Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
    else
      Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );

    return( FALSE );
  } /* if( !ok ) */

  // The optimizer can queue multiple calls to this function so protect it with a lock
  g_mutex_lock(&global_lock);

  SetFlag( INPUT_OPENED );
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
  SetFlag( COMMON_PROJECTION );
  SetFlag( COMMON_FREQUENCY );
  SetFlag( MAIN_NEW_FREQ );
  if( isFlagSet(PLOT_ENABLED) ) SetFlag( FREQ_LOOP_INIT );
  floop_tag = 0;
  crnt.newer = 0;
  crnt.valid = 0;
  near_field.newer = 0;
  near_field.valid = 0;

  New_Frequency_Reset_Prev();

  /* Allow re-draws on expose events etc */
  ClearFlag( INPUT_PENDING );

  /* Set projection at 45 deg rotation and
   * inclination if NEC2 editor window is not open, but
   * not while optimizing because so the view stays where it is */
  if( (nec2_edit_window == NULL) && isFlagClear(OPTIMIZER_OUTPUT) )
  {
    New_Viewer_Angle( 45.0, 45.0, rotate_structure,
        incline_structure, &structure_proj_params );
    New_Structure_Projection_Angle();
  }

  /* Show current frequency */
  gtk_spin_button_set_value( mainwin_frequency, (gdouble)calc_data.freq_mhz );

  /* Show main control buttons etc */
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_hbox2") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_grid1") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "structure_frame") );
  gtk_widget_show( Builder_Get_Object(main_window_builder, "main_view_menuitem") );
  gtk_widget_show( structure_drawingarea );

  /* If currents or charges draw button is active
   * re-initialize structure currents/charges drawing */
  if( isFlagSet(DRAW_CURRENTS) )
    Main_Currents_Togglebutton_Toggled( TRUE );
  if( isFlagSet(DRAW_CHARGES) )
    Main_Charges_Togglebutton_Toggled( TRUE );

  /* Set input file to NEC2 editor. It will only
   * happen if the NEC2 editor window is open */
  new = *( (gboolean *)arg );
  if( new && isFlagClear(OPTIMIZER_OUTPUT) )
    Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );
  else
    Nec2_Input_File_Treeview( NEC2_EDITOR_RELOAD );

  /* Re-initialize Rad Pattern drawing if window open */
  if( rdpattern_window != NULL )
  {
    // Don't reset the zoom during optimization:
    if( isFlagClear(OPTIMIZER_OUTPUT) )
    {
      widget = Builder_Get_Object(
          rdpattern_window_builder, "rdpattern_zoom_spinbutton" );

      gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(widget), (gdouble)rc_config.rdpattern_zoom_spinbutton );
    }

    /* Simulate activation of main rdpattern button */
    if( isFlagClear(OPTIMIZER_OUTPUT) )
      Main_Rdpattern_Activate( FALSE );

    /* Select display of radiation or EH pattern */
    if( isFlagSet(DRAW_GAIN) )
    {
      if( isFlagClear(OPTIMIZER_OUTPUT) )
        Rdpattern_Gain_Togglebutton_Toggled( TRUE );
    }
    else if( isFlagSet(DRAW_EHFIELD) )
    {
      if( isFlagClear(OPTIMIZER_OUTPUT) )
        Rdpattern_EH_Togglebutton_Toggled( TRUE );
    }
    else
    {
      Rdpattern_Gain_Togglebutton_Toggled( FALSE );
      Rdpattern_EH_Togglebutton_Toggled( FALSE );
    }

    GtkWidget *box = Builder_Get_Object( rdpattern_window_builder, "rdpattern_box" );
    gtk_widget_show( box );
  }

  /* Re-initiate frequency plots if window open */
  if( isFlagSet(PLOT_ENABLED) )
  {
    GtkWidget *box = Builder_Get_Object( freqplots_window_builder, "freqplots_box" );
    gtk_widget_show( box );
    if( rc_config.main_loop_start )
    {
      Main_Freqplots_Activate();
      Start_Frequency_Loop();
    }
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

  /* Open NEC2 editor if there is a saved geometry state */
  GtkMenuItem *menu_item;
  if( rc_config.nec2_edit_width &&
      rc_config.nec2_edit_height )
  {
    menu_item = GTK_MENU_ITEM(
        Builder_Get_Object(main_window_builder, "nec2_edit") );
    gtk_menu_item_activate( menu_item );
  }

  // Unlock the mutex:
  g_mutex_unlock(&global_lock);

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
            if( forked_proc_data[idx]->child_pid == pid )
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
      kill( forked_proc_data[num_child_procs]->child_pid, SIGKILL );
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

