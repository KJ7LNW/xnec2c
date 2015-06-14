/*
 * Shared global variables for nec2c.c
 */

#include "shared.h"

/*------------------------------------------------------------------------*/

/* Editor windows */
GtkWidget
  *wire_editor		= NULL,	/* Wire designer window  */
  *arc_editor		= NULL,	/* Arc designer window   */
  *helix_editor		= NULL, /* Helix designer window */
  *patch_editor		= NULL,	/* Patch designer window */
  *reflect_editor	= NULL,	/* Reflect design window */
  *scale_editor		= NULL,	/* Scale designer window */
  *cylinder_editor	= NULL,	/* Cylinder designer window  */
  *transform_editor	= NULL, /* Transform designer window */
  *gend_editor		= NULL;	/* Geom End designer window  */

/* Command windows */
GtkWidget
  *excitation_command= NULL, /* Ecitation command editor */
  *frequency_command = NULL, /* Frequency command editor */
  *ground_command	 = NULL, /* Ground command editor (GN) */
  *ground2_command	 = NULL, /* Ground command editor (GD) */
  *loading_command	 = NULL, /* Loading command editor 	*/
  *network_command	 = NULL, /* Network command editor	*/
  *txline_command	 = NULL, /* Tran Line command editor */
  *radiation_command = NULL, /* Rad Pattern command editor */
  *nearfield_command = NULL, /* Near Fields command editor */
  *kernel_command	 = NULL, /* Thin-wire Kernel command editor  */
  *intrange_command	 = NULL, /* Interaction Range command editor */
  *execute_command	 = NULL; /* Execute (EX card) command editor */

/* Frequency step entry */
GtkEntry *rdpattern_fstep_entry = NULL;

/* Radiation pattern rotation, freq and zoom spin buttons */
GtkSpinButton
  *rotate_rdpattern    = NULL,
  *incline_rdpattern   = NULL,
  *rotate_structure    = NULL,
  *incline_structure   = NULL,
  *rdpattern_zoom      = NULL,
  *structure_zoom      = NULL,
  *rdpattern_frequency = NULL,
  *mainwin_frequency = NULL;

projection_parameters_t
  rdpattern_proj_params,
  structure_proj_params;

/* Scroll adjustments of geometry
 * and command treeview windows */
GtkAdjustment
  *geom_adjustment = NULL,
  *cmnd_adjustment = NULL;

GtkTreeView
  *geom_treeview = NULL,
  *cmnd_treeview = NULL;

/* Main, frequency plots and radation pattern windows */
GtkWidget
  *main_window		= NULL,
  *freqplots_window = NULL,
  *rdpattern_window = NULL,
  *nec2_edit_window = NULL;

/* Drawing area widgets */
GtkWidget *structure_drawingarea = NULL;
GtkWidget *freqplots_drawingarea = NULL;
GtkWidget *rdpattern_drawingarea = NULL;

/* Motion event handler id */
gulong rdpattern_motion_handler;

/* Dialog widgets */
GtkWidget *quit_dialog	  = NULL;
GtkWidget *animate_dialog = NULL;

/* Frequency step entry widget */
GtkEntry *structure_fstep_entry = NULL;

/* Motion event handler id */
gulong structure_motion_handler;

/* Commands between parent and child processes */
char *fork_commands[] = FORK_CMNDS;

/* Forked process data */
forked_proc_data_t **forked_proc_data = NULL;

/* Number of forked child processes */
int num_child_procs = 0;

/* Program forked flag */
gboolean FORKED = FALSE;

/* Used to kill window deleted by user */
GtkWidget *kill_window = NULL;

/* Animation timeout callback tag */
guint anim_tag = 0;

/* Pixmap for drawing structures */
GdkPixmap *structure_pixmap = NULL;
int structure_pixmap_width, structure_pixmap_height;

/* Pixmap for drawing plots */
GdkPixmap *freqplots_pixmap = NULL;
int freqplots_pixmap_width, freqplots_pixmap_height;

/* Pixmap for drawing radiation patterns */
GdkPixmap *rdpattern_pixmap = NULL;
int rdpattern_pixmap_width, rdpattern_pixmap_height;

/* Frequency loop idle function tag */
guint floop_tag = 0;

/* Radiation pattern data */
rad_pattern_t *rad_pattern = NULL;

/* Near E/H field data */
near_field_t near_field;

/* Segments for drawing structure */
GdkSegment *structure_segs = NULL;

/* Global tag number for geometry editors */
gint gbl_tag_num = 0;

GtkWidget *error_dialog = NULL;

/* Tree list stores */
GtkListStore
  *cmnt_store = NULL,
  *geom_store = NULL,
  *cmnd_store = NULL;

complex double *cm = NULL;

/* Needed data */
impedance_data_t impedance_data;

/* Data for various calculations */
calc_data_t calc_data;
data_t data;

/* Magnitude of seg/patch current/charge */
double *cmag = NULL, *ct1m = NULL, *ct2m = NULL;

/* common  /crnt/ */
crnt_t crnt;

/* common  /dataj/ */
dataj_t dataj;

/* pointers to input/output files */
FILE *input_fp = NULL;

/* Input file name */
char infile[81] = "";

/* common  /fpat/ */
fpat_t fpat;

/*common  /ggrid/ */
ggrid_t ggrid;

/* common  /gnd/ */
gnd_t gnd;

/* common  /gwav/ */
gwav_t gwav;

/* common  /incom/ */
incom_t incom;

/* common  /matpar/ */
matpar_t matpar;

/* common  /netcx/ */
netcx_t netcx;

/* common  /save/ */
save_t save;

/* common  /segj/ */
segj_t segj;

/* common  /smat/ */
smat_t smat;

/* common  /vsorc/ */
vsorc_t vsorc;

/* common  /zload/ */
zload_t zload;

/*------------------------------------------------------------------------*/

