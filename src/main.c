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
 */

#include "main.h"
#include "shared.h"

static void sig_handler(int signal);

/* Child process pid returned by fork() */
static pid_t child_pid = (pid_t)(-1);

/*------------------------------------------------------------------------*/

  int
main (int argc, char *argv[])
{
  /* getopt() variables */
  int option, idx, err;
  gboolean new = TRUE;

  /*** Signal handler related code ***/
  /* new and old actions for sigaction() */
  struct sigaction sa_new, sa_old;

  /* initialize new actions */
  sa_new.sa_handler = sig_handler;
  sigemptyset( &sa_new.sa_mask );
  sa_new.sa_flags = 0;

  /* Register function to handle signals */
  sigaction( SIGINT,  &sa_new, &sa_old );
  sigaction( SIGSEGV, &sa_new, NULL );
  sigaction( SIGFPE,  &sa_new, NULL );
  sigaction( SIGTERM, &sa_new, NULL );
  sigaction( SIGABRT, &sa_new, NULL );
  sigaction( SIGCHLD, &sa_new, NULL );

  gtk_init (&argc, &argv);

  /* Create folder path for glade file */
  snprintf( xnec2c_glade, sizeof(xnec2c_glade),
	  "/usr/share/xnec2c/xnec2c.glade" );

  /* Create a default config if needed, abort on error */
  if( !Create_Default_Config() ) exit( -1 );

  /* Process command line options */
  calc_data.num_jobs  = 1;
  rc_config.infile[0] = '\0';
  while( (option = getopt(argc, argv, "i:j:hv") ) != -1 )
  {
	switch( option )
	{
	  case 'i': /* specify input file name */
		{
		  size_t siz = sizeof( rc_config.infile );
		  if( strlen(optarg) >= siz )
		  {
			fprintf ( stderr,
				_("xnec2c: input file name too long ( > %d char )\n"), (int)siz - 1 );
			exit(-1);
		  }
		  /* For null term. */
		  Strlcpy( rc_config.infile, optarg, siz );
		}
		break;

	  case 'j': /* number of child processes = num of processors */
		calc_data.num_jobs = atoi( optarg );
		break;

	  case 'h': /* print usage and exit */
		usage();
		exit(0);

	  case 'v': /* print xnec2c version */
		puts( PACKAGE_STRING );
		exit(0);

	  default:
		usage();
		exit(0);

	} /* switch( option ) */
  } /* while( (option = getopt(argc, argv, "i:o:hv") ) != -1 ) */

  /* Read input file path name if not supplied by -i option */
  if( (strlen(rc_config.infile) == 0) &&
	  (strstr(argv[argc - 1], ".nec") ||
	   strstr(argv[argc - 1], ".NEC")) )
  {
	size_t siz = sizeof( rc_config.infile );
	if( strlen(argv[argc - 1]) >= siz )
	{
	  fprintf ( stderr,
		  _("xnec2c: input file path name too long ( > %d char )\n"), (int)siz - 1 );
	  exit(-1);
	}
	 /* For null termination */
	Strlcpy( rc_config.infile, argv[argc-1], siz );
  }

  /* When forking is useful, e.g. if more than 1 processor is
   * available, the parent process handles the GUI and delegates
   * calculations to the child processes, one per processor. The
   * requested number of child processes = number of processors */

  /* Allocate buffers for fork data */
  if( calc_data.num_jobs > 1 )
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
		perror( "xnec2c: pipe()" );
		fprintf( stderr,
			_("xnec2c: exiting after fatal error (pipe() failed)") );
		exit(-1);
	  }

	  err = pipe( forked_proc_data[idx]->child2pnt_pipe );
	  if( err )
	  {
		perror( "xnec2c: pipe()" );
		fprintf( stderr,
			_("xnec2c: exiting after fatal error (pipe() failed)") );
		exit(-1);
	  }

	  /* Fork child process */
	  forked_proc_data[idx]->child_pid = fork();
	  if( forked_proc_data[idx]->child_pid == -1 )
	  {
		perror( "xnec2c: fork()" );
		fprintf( stderr,
			_("xnec2c: exiting after fatal error (fork() failed)") );
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
  calc_data.zo = 50.0;

  /* Read GUI state config file and reset geometry */
  Read_Config();

  /* If input file is specified, get the working directory */
  if( strlen(rc_config.infile) )
	Get_Dirname( rc_config.infile, rc_config.working_dir, NULL );

  /* Main window freq spinbutton */
  mainwin_frequency = GTK_SPIN_BUTTON(
	  Builder_Get_Object(main_window_builder, "main_freq_spinbutton") );

  /* Get the structure drawing area and allocation */
  structure_drawingarea =
	Builder_Get_Object(main_window_builder, "structure_drawingarea");
  GtkAllocation allocation;
  gtk_widget_get_allocation ( structure_drawingarea, &allocation);
  structure_width  = allocation.width;
  structure_height = allocation.height;
  New_Projection_Parameters(
	  structure_width, structure_height,
	  &structure_proj_params );

  /* Initialize structure projection angles */
  rotate_structure  = GTK_SPIN_BUTTON(
	  Builder_Get_Object(main_window_builder, "main_rotate_spinbutton"));
  incline_structure = GTK_SPIN_BUTTON(
	  Builder_Get_Object(main_window_builder, "main_incline_spinbutton"));
  structure_zoom = GTK_SPIN_BUTTON(
	  Builder_Get_Object(main_window_builder, "main_zoom_spinbutton"));
  structure_fstep_entry = GTK_ENTRY(
	  Builder_Get_Object(main_window_builder, "structure_fstep_entry")) ;

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
  if( strlen(rc_config.infile) > 0 )
	g_idle_add( Open_Input_File, (gpointer)(&new) );
  else
	SetFlag( INPUT_PENDING );

  gtk_main ();

  return 0;
}

/*-----------------------------------------------------------------------*/

/* Open_Input_File()
 *
 * Opens NEC2 input file
 */

  gboolean
Open_Input_File( gpointer udata )
{
  gboolean ok, new;
  GtkWidget *widget;

  /* Stop freq loop */
  if( isFlagSet(FREQ_LOOP_RUNNING) )
	Stop_Frequency_Loop();

  /* Close open files if any */
  Close_File( &input_fp );

  /* Suppress activity while input file opened */
  ClearFlag( OPEN_INPUT_FLAGS );
  SetFlag( INPUT_PENDING );
  calc_data.fstep = -1;

  /* Open NEC2 input file */
  if( strlen(rc_config.infile) == 0 ) return( FALSE );
  Open_File( &input_fp, rc_config.infile, "r");

  /* Read input file, record failures */
  ok = Read_Comments() && Read_Geometry() && Read_Commands();
  if( !ok )
  {
	/* Hide main control buttons etc */
	gtk_widget_hide(
		Builder_Get_Object(main_window_builder, "main_hbox1") );
	gtk_widget_hide(
		Builder_Get_Object(main_window_builder, "main_hbox2") );
	gtk_widget_hide(
		Builder_Get_Object(main_window_builder, "main_grid1") );
	gtk_widget_hide(
		Builder_Get_Object(main_window_builder, "main_view_menuitem") );
	gtk_widget_hide(
		Builder_Get_Object(main_window_builder, "structure_frame") );

	/* Close plot/rdpat windows if open */
	Gtk_Widget_Destroy( rdpattern_window );
	Gtk_Widget_Destroy( freqplots_window );

	if( nec2_edit_window == NULL )
	  Open_Nec2_Editor( NEC2_EDITOR_RELOAD );
	else
	  Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );

	return( FALSE );
  } /* if( !ok ) */
  SetFlag( INPUT_OPENED );

  /* Ask child processes to read input file */
  if( FORKED )
  {
	int idx;
	size_t lenc, leni;

	lenc = strlen( fork_commands[INFILE] );
	leni = strlen( rc_config.infile );
	for( idx = 0; idx < num_child_procs; idx++ )
	{
	  Write_Pipe( idx, fork_commands[INFILE], (ssize_t)lenc, TRUE );
	  Write_Pipe( idx, rc_config.infile, (ssize_t)leni, TRUE );
	}
  } /* if( FORKED ) */

  /* Initialize xnec2c */
  SetFlag( COMMON_PROJECTION );
  SetFlag( COMMON_FREQUENCY );
  SetFlag( MAIN_NEW_FREQ );
  if( isFlagSet(PLOT_ENABLED) )
	SetFlag( FREQ_LOOP_INIT );
  floop_tag = 0;
  save.last_freq = 0.0;
  crnt.newer = 0;
  crnt.valid = 0;
  near_field.newer = 0;
  near_field.valid = 0;

  /* Allow redraws on expose events etc */
  ClearFlag( INPUT_PENDING );

  /* Set projection at 45 deg rotation and
   * inclination if NEC2 editor window is not open */
  if( nec2_edit_window == NULL )
	New_Viewer_Angle(
		45.0, 45.0, rotate_structure,
		incline_structure, &structure_proj_params );
  New_Structure_Projection_Angle();

  /* Show current frequency */
  gtk_spin_button_set_value( mainwin_frequency, (gdouble)calc_data.fmhz );

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
  new = *( (gboolean *)udata );
  if( new )
	Nec2_Input_File_Treeview( NEC2_EDITOR_CLEAR );
  else
	Nec2_Input_File_Treeview( NEC2_EDITOR_RELOAD );

  /* Re-initialize Rad Pattern drawing if window open */
  if( rdpattern_window != NULL )
  {
	  widget = Builder_Get_Object(
		  rdpattern_window_builder, "rdpattern_zoom_spinbutton" );
	  gtk_spin_button_set_value(
		  GTK_SPIN_BUTTON(widget), (gdouble)rc_config.rdpattern_zoom_spinbutton );

	/* Simulate activation of main rdpattern button */
	Main_Rdpattern_Activate( FALSE );
	crnt.valid = 0;

	if( isFlagSet(DRAW_GAIN) )
	  Rdpattern_Gain_Togglebutton_Toggled( TRUE );
	else if( isFlagSet(DRAW_EHFIELD) )
	  Rdpattern_EH_Togglebutton_Toggled( TRUE );
	else
	{
	  Rdpattern_Gain_Togglebutton_Toggled( FALSE );
	  Rdpattern_EH_Togglebutton_Toggled( FALSE );
	}

	GtkWidget *box =
	  Builder_Get_Object( rdpattern_window_builder, "rdpattern_box" );
	gtk_widget_show( box );
  }

  /* Re-initiate plots if window open */
  if( freqplots_window != NULL )
  {
	GtkWidget *box =
	  Builder_Get_Object( freqplots_window_builder, "freqplots_box" );
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

  return( FALSE );
} /* Open_Input_File() */

/*------------------------------------------------------------------------*/

static void sig_handler( int signal )
{
  switch( signal )
  {
	case SIGINT:
	  /* Save GUI state for restoring windows */
	  Get_GUI_State();
	  Save_Config();
	  fprintf( stderr, _("xnec2c: exiting via user interrupt\n") );
	  break;

	case SIGSEGV:
	  fprintf( stderr, _("xnec2c: segmentation fault, exiting\n") );
	  break;

	case SIGFPE:
	  fprintf( stderr, _("xnec2c: floating point exception, exiting\n") );
	  break;

	case SIGABRT:
	  fprintf( stderr, _("xnec2c: abort signal received, exiting\n") );
	  break;

	case SIGTERM:
	  /* Save GUI state for restoring windows */
	  Get_GUI_State();
	  Save_Config();
	  fprintf( stderr, _("xnec2c: termination request received, exiting\n") );
	  break;

	case SIGCHLD:
	  {
		int idx;
		pid_t pid = getpid();

		if( !FORKED )
		{
		  fprintf( stderr,
			  _("xnec2c: not forked, ignoring SIGCHLD from pid %d\n"), pid );
		  return;
		}
		else
		{
		  for( idx = 0; idx < calc_data.num_jobs; idx++ )
			if( forked_proc_data[idx]->child_pid == pid )
			{
			  fprintf( stderr, _("xnec2c: child process pid %d exited\n"), pid );
			  if( isFlagSet(MAIN_QUIT) ) return;
			  else break;
			}
		  return;
		}
	  }

	default:
	  fprintf( stderr, _("xnec2c: default exit with signal: %d\n"), signal );
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

