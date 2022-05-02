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

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glu.h>

static void sig_handler(int signal);

/* Child process pid returned by fork() */
static pid_t child_pid = (pid_t)(-1);

/*------------------------------------------------------------------------*/

char *orig_numeric_locale = NULL;

extern rgba_t *rdpat_colors;
extern point_3d_t *point_3d;
extern color_triangle_t *rdpat_triangles;

void Gtk_Builder( GtkBuilder **builder, gchar **object_ids );
void gl_box_init(GtkBox *box);
GtkWidget * create_gl_window( GtkBuilder **builder );
GtkWidget *gl_window = NULL;
GtkWidget *gl_area = NULL;

// class members:
static GLuint program;
static GLuint mvp_location;
static GLuint position_idx;
static GLuint color_idx;
static GLuint vao;
float mvp[16];


struct vertex_info {
  float pos[3];
  float color[3];
};

typedef enum {
  GLAREA_ERROR_SHADER_COMPILATION,
  GLAREA_ERROR_SHADER_LINK
} GlareaError;

  int
main (int argc, char *argv[])
{
  /* getopt() variables */
  int option, idx, err;
  int enable_forking = 1;

  /*** Signal handler related code ***/
  /* new and old actions for sigaction() */
  struct sigaction sa_new, sa_old;

  /* initialize new actions */
  sa_new.sa_handler = sig_handler;
  sigemptyset( &sa_new.sa_mask );
  sa_new.sa_flags = 0;

  // Setup locales so we can switch between C and the system locale.
  // The pointer returned from setlocale() seems to be stack sensitive
  // so make a copy of it for later:
  setlocale(LC_ALL, "");
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
  while( (option = getopt(argc, argv, "i:j:hdqvVPb") ) != -1 )
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

      case 'V': /* print xnec2c version */
        puts( PACKAGE_STRING );
        exit(0);

      default:
        usage();
        exit(0);

    } /* switch( option ) */
  } /* while( (option = getopt(argc, argv, "i:o:hv") ) != -1 ) */

  /* Initialize the external math libraries */
  init_mathlib();

  /* Read input file path name if not supplied by -i option */
  if( (strlen(rc_config.input_file) == 0) &&
      (strstr(argv[argc - 1], ".nec") ||
       strstr(argv[argc - 1], ".NEC")) )
  {
    size_t siz = sizeof( rc_config.input_file );
    if( strlen(argv[argc - 1]) >= siz )
    {
      pr_crit("input file path name too long ( > %d char )\n", (int)siz - 1);
      exit(-1);
    }
     /* For null termination */
    Strlcpy( rc_config.input_file, argv[argc-1], siz );
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

	//opengl
	{
		GtkBuilder *gl_builder = NULL;
		gl_window = create_gl_window(&gl_builder);
		gl_box_init(GTK_BOX(gl_window));
	  gtk_widget_show( gl_window );
	}


  gtk_main ();

  free_ptr((void**)&orig_numeric_locale);

  return 0;
} // main()

static guint create_shader(int shader_type, const char *source,
	GError **error, guint *shader_out)
{
	guint shader = glCreateShader(shader_type);

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int status;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int log_len;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

		char *buffer = g_malloc(log_len + 1);

		glGetShaderInfoLog(shader, log_len, NULL, buffer);

		pr_err("Compilation failure in %s shader: %s",
			    shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);

		g_free(buffer);

		glDeleteShader(shader);
		shader = 0;
	}

	if (shader_out != NULL)
		*shader_out = shader;

	return shader != 0;
}

static gboolean init_shaders(guint *program_out, guint *mvp_location_out,
	guint *position_location_out, guint *color_location_out, GError **error)
{
	GBytes *source;
	guint program = 0;
	guint mvp_location = 0;
	guint vertex = 0, fragment = 0;
	guint position_location = 0;
	guint color_location = 0;

	// load the vertex shader 
	source = g_resources_lookup_data("/gl/rdpat-vertex.glsl", 0, NULL);
	create_shader(GL_VERTEX_SHADER,
		g_bytes_get_data(source, NULL), error, &vertex);

	g_bytes_unref(source);
	if (vertex == 0)
	{
		pr_err("unable to load resource /gl/rdpat-vertex.glsl\n");
		goto out;
	}

	// load the fragment shader 
	source = g_resources_lookup_data("/gl/rdpat-fragment.glsl", 0, NULL);
	create_shader(GL_FRAGMENT_SHADER,
		g_bytes_get_data(source, NULL), error, &fragment);

	g_bytes_unref(source);
	if (fragment == 0)
	{
		pr_err("unable to load resource /gl/rdpat-fragment.glsl\n");
		goto out;
	}

	// link the vertex and fragment shaders together 
	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	int status = 0;

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int log_len = 0;

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

		char *buffer = g_malloc(log_len + 1);

		glGetProgramInfoLog(program, log_len, NULL, buffer);

		pr_err("Linking failure in program: %s", buffer);
		g_free(buffer);

		glDeleteProgram(program);
		program = 0;

		goto out;
	}

	// get the location of the "mvp" uniform 
	mvp_location = glGetUniformLocation(program, "mvp");

	// get the location of the "position" and "color" attributes 
	position_location = glGetAttribLocation(program, "position");
	color_location = glGetAttribLocation(program, "color");

	// the individual shaders can be detached and destroyed 
	glDetachShader(program, vertex);
	glDetachShader(program, fragment);

out:
	if (vertex != 0)
		glDeleteShader(vertex);
	if (fragment != 0)
		glDeleteShader(fragment);

	if (program_out != NULL)
		*program_out = program;
	if (mvp_location_out != NULL)
		*mvp_location_out = mvp_location;
	if (position_location_out != NULL)
		*position_location_out = position_location;
	if (color_location_out != NULL)
		*color_location_out = color_location;

	return program != 0;
}

static void init_buffers(guint position_idx, guint color_idx, guint *vao_out)
{
	guint vao, buffer;
	pr_debug("fpat nth=%d nph=%d\n", fpat.nph, fpat.nth);

	// we need to create a VAO to store the other buffers 
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// this is the VBO that holds the vertex data 
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		2*fpat.nph*(fpat.nth-1)*sizeof(color_triangle_t),
		rdpat_triangles, GL_STATIC_DRAW);

	// enable and set the position attribute 
	glEnableVertexAttribArray(position_idx);
	glVertexAttribPointer(position_idx, 3, GL_FLOAT, GL_FALSE,
		sizeof(color_point_t),
		(void*)G_STRUCT_OFFSET(color_point_t, point));

	// enable and set the color attribute 
	// TODO: Convert colors to bytes not doubles, use GL_RGBA?
	glEnableVertexAttribArray(color_idx);
	glVertexAttribPointer(color_idx, 3, GL_FLOAT, GL_FALSE,
		sizeof(color_point_t),
		(void*)G_STRUCT_OFFSET(color_point_t, color));

	// reset the state; we will re-enable the VAO when needed 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// the VBO is referenced by the VAO 
	glDeleteBuffers(1, &buffer);

	if (vao_out != NULL)
		*vao_out = vao;
}

void init_mvp(float *res)
{
  /* initialize a matrix as an identity matrix */
  res[0] = 1.f; res[4] = 0.f;  res[8] = 0.f; res[12] = 0.f;
  res[1] = 0.f; res[5] = 1.f;  res[9] = 0.f; res[13] = 0.f;
  res[2] = 0.f; res[6] = 0.f; res[10] = 1.f; res[14] = 0.f;
  res[3] = 0.f; res[7] = 0.f; res[11] = 0.f; res[15] = 1.f;
}

void compute_mvp(float *res, float  phi, float  theta, float  psi)
{
  float x = phi * (G_PI / 180.f);
  float y = theta * (G_PI / 180.f);
  float z = psi * (G_PI / 180.f);
  float c1 = cosf (x), s1 = sinf (x);
  float c2 = cosf (y), s2 = sinf (y);
  float c3 = cosf (z), s3 = sinf (z);
  float c3c2 = c3 * c2;
  float s3c1 = s3 * c1;
  float c3s2s1 = c3 * s2 * s1;
  float s3s1 = s3 * s1;
  float c3s2c1 = c3 * s2 * c1;
  float s3c2 = s3 * c2;
  float c3c1 = c3 * c1;
  float s3s2s1 = s3 * s2 * s1;
  float c3s1 = c3 * s1;
  float s3s2c1 = s3 * s2 * c1;
  float c2s1 = c2 * s1;
  float c2c1 = c2 * c1;
  
  /* apply all three Euler angles rotations using the three matrices:
   *
   * ⎡  c3 s3 0 ⎤ ⎡ c2  0 -s2 ⎤ ⎡ 1   0  0 ⎤
   * ⎢ -s3 c3 0 ⎥ ⎢  0  1   0 ⎥ ⎢ 0  c1 s1 ⎥
   * ⎣   0  0 1 ⎦ ⎣ s2  0  c2 ⎦ ⎣ 0 -s1 c1 ⎦
   */
  res[0] = c3c2;  res[4] = s3c1 + c3s2s1;  res[8] = s3s1 - c3s2c1; res[12] = 0.f;
  res[1] = -s3c2; res[5] = c3c1 - s3s2s1;  res[9] = c3s1 + s3s2c1; res[13] = 0.f;
  res[2] = s2;    res[6] = -c2s1;         res[10] = c2c1;          res[14] = 0.f;
  res[3] = 0.f;   res[7] = 0.f;           res[11] = 0.f;           res[15] = 1.f;
}

void gl_fini(GtkGLArea *area, GdkGLContext *context)
{
}

void gl_init(GtkGLArea * area)
{
	pr_debug("\n");
	gtk_gl_area_make_current(GTK_GL_AREA(gl_area));
	if (gtk_gl_area_get_error(GTK_GL_AREA(gl_area)) != NULL)
		return;

	init_mvp(mvp);

	// initialize the shaders and retrieve the program data 
	GError *error = NULL;

	if (!init_shaders(&program, &mvp_location, &position_idx, &color_idx, &error))
	{
		// set the GtkGLArea in error state, so we'll
		// see the error message rendered inside the viewport 
		gtk_gl_area_set_error(GTK_GL_AREA(gl_area), error);
		g_error_free(error);
		return;
	}

	// set the window title 
	const char *renderer = (char *) glGetString(GL_RENDERER);
	pr_info("glarea renderer: %s\n", renderer ? renderer : "Unknown");
}

gboolean gl_draw(GtkGLArea * area)
{
	pr_debug("\n");
	gtk_gl_area_make_current(GTK_GL_AREA(area));
	if (gtk_gl_area_get_error(GTK_GL_AREA(area)) != NULL)
	{
		pr_err("gtk_gl_area_get_error() failed\n");
		return FALSE;
	}

	if (!rdpat_triangles)
	{
		pr_debug("rdpat_triangles is NULL\n");
		return FALSE;
	}

	glClearColor(0, 0, 0, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	printf("gl_draw: program=%d point_3d=%p rdpat_colors=%p n=%d area=%p\n",
	       program,
		   (void *) point_3d,
		   (void *) rdpat_colors,
		   fpat.nph * fpat.nth,
		   (void*)area);

	for (int i = 0; i < fpat.nph * fpat.nth; i++)
	{
		// rdpat_colors[i].r *= 255;
		// rdpat_colors[i].g *= 255;
		// rdpat_colors[i].b *= 255;
		// rdpat_colors[i].a *= 255;
	}
	/*
	*/
	rdpat_triangles[0].cp[0].point.x = -1;
	rdpat_triangles[0].cp[0].point.y = -1;
	rdpat_triangles[0].cp[0].point.z = 0;
	rdpat_triangles[0].cp[0].color.r = 1;
	rdpat_triangles[0].cp[0].color.g = 0;
	rdpat_triangles[0].cp[0].color.b = 0;
	rdpat_triangles[0].cp[0].color.a = 0.1;
	
	
	rdpat_triangles[0].cp[1].point.x = 1;
	rdpat_triangles[0].cp[1].point.y = -1;
	rdpat_triangles[0].cp[1].point.z = 0;
	rdpat_triangles[0].cp[1].color.r = 0;
	rdpat_triangles[0].cp[1].color.g = 1;
	rdpat_triangles[0].cp[1].color.b = 0;
	rdpat_triangles[0].cp[1].color.a = 0.1;

	rdpat_triangles[0].cp[2].point.x = 0;
	rdpat_triangles[0].cp[2].point.y = 1;
	rdpat_triangles[0].cp[2].point.z = 0;
	rdpat_triangles[0].cp[2].color.r = 0;
	rdpat_triangles[0].cp[2].color.g = 0;
	rdpat_triangles[0].cp[2].color.b = 1;
	rdpat_triangles[0].cp[2].color.a = 0.1;

	// initialize the vertex buffers because rdpat may have changed:
	init_buffers(position_idx, color_idx, &vao);

	// load our program 
	glUseProgram(program);

	// update the "mvp" matrix we use in the shader 
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &(mvp[0]));

	// use the buffers in the VAO 
	glBindVertexArray(vao);

	// draw the three vertices as a triangle 
	// glDrawArrays (GL_TRIANGLES, 0, 3);
	//glDrawArrays(GL_TRIANGLES, 0, fpat.nph * fpat.nth);
	glDrawArrays(GL_TRIANGLES, 0, 3*2*fpat.nph*(fpat.nth-1));
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	// we finished using the buffers and program 
	glBindVertexArray(0);
	glUseProgram(0);

	glFlush();

	return FALSE;
}

void gl_box_init(GtkBox *box)
{
	pr_debug("box add\n");
	gl_area = gtk_gl_area_new();
	gtk_widget_set_hexpand(gl_area, TRUE);
	gtk_widget_set_vexpand(gl_area, TRUE);

	gtk_container_add(GTK_CONTAINER(box), gl_area);

	g_signal_connect(gl_area, "realize", G_CALLBACK (gl_init), NULL);
	g_signal_connect(gl_area, "unrealize", G_CALLBACK (gl_fini), NULL);
	g_signal_connect(gl_area, "render", G_CALLBACK (gl_draw), NULL);

	gtk_widget_show_all(GTK_WIDGET(box));
}

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

