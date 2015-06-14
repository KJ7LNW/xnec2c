#ifndef SHARED_H
#define SHARED_H	1

#include "common.h"
#include "fork.h"

/*------------------------------------------------------------------------*/

/* Editor windows */
extern GtkWidget
  *wire_editor,	/* Wire designer window  */
  *arc_editor,	/* Arc designer window   */
  *helix_editor, /* Helix designer window */
  *patch_editor,	/* Patch designer window */
  *reflect_editor,	/* Reflect design window */
  *scale_editor,	/* Scale designer window */
  *cylinder_editor,	/* Cylinder designer window  */
  *transform_editor, /* Transform designer window */
  *gend_editor;	/* Geom End designer window  */

/* Command windows */
extern GtkWidget
  *excitation_command, /* Ecitation command editor */
  *frequency_command, /* Frequency command editor */
  *ground_command, /* Ground command editor (GN) */
  *ground2_command, /* Ground command editor (GD) */
  *loading_command, /* Loading command editor 	*/
  *network_command, /* Network command editor	*/
  *txline_command, /* Tran Line command editor */
  *radiation_command, /* Rad Pattern command editor */
  *nearfield_command, /* Near Fields command editor */
  *kernel_command, /* Thin-wire Kernel command editor  */
  *intrange_command, /* Interaction Range command editor */
  *execute_command	 ; /* Execute (EX card) command editor */

/* Frequency step entry */
extern GtkEntry *rdpattern_fstep_entry ;

/* Radiation pattern rotation, freq and zoom spin buttons */
extern GtkSpinButton
  *rotate_rdpattern,
  *incline_rdpattern,
  *rotate_structure,
  *incline_structure,
  *rdpattern_frequency,
  *rdpattern_zoom,
  *structure_zoom;

/* Motion event handler id */
extern gulong structure_motion_handler;

/* Commands between parent and child processes */
extern char *fork_commands[];

/* Forked process data */
extern forked_proc_data_t **forked_proc_data;

extern projection_parameters_t
  rdpattern_proj_params,
  structure_proj_params;

/* Number of forked child processes */
extern int num_child_procs;

/* Program forked flag */
extern gboolean FORKED;

/* Main window freq spinbutton */
extern GtkSpinButton *mainwin_frequency;

/* Scroll adjustments of geometry
 * and command treeview windows */
extern GtkAdjustment
  *geom_adjustment,
  *cmnd_adjustment ;

extern GtkTreeView
  *geom_treeview,
  *cmnd_treeview ;

/* Main, frequency plots and radation pattern windows */
extern GtkWidget
  *main_window,
  *freqplots_window,
  *rdpattern_window,
  *nec2_edit_window ;

/* Drawing area widgets */
extern GtkWidget *structure_drawingarea ;
extern GtkWidget *freqplots_drawingarea ;
extern GtkWidget *rdpattern_drawingarea ;

/* Motion event handler id */
extern gulong rdpattern_motion_handler;

/* Dialog widgets */
extern GtkWidget *quit_dialog ;
extern GtkWidget *animate_dialog ;

/* Used to kill window deleted by user */
extern GtkWidget *kill_window ;

/* Animation timeout callback tag */
extern guint anim_tag;

/* Pixmap for drawing structures */
extern GdkPixmap *structure_pixmap ;
extern int structure_pixmap_width, structure_pixmap_height;

/* Pixmap for drawing plots */
extern GdkPixmap *freqplots_pixmap ;
extern int freqplots_pixmap_width, freqplots_pixmap_height;

/* Pixmap for drawing radiation patterns */
extern GdkPixmap *rdpattern_pixmap ;
extern int rdpattern_pixmap_width, rdpattern_pixmap_height;

/* Frequency loop idle function tag */
extern guint floop_tag;

/* Radiation pattern data */
extern rad_pattern_t *rad_pattern ;

/* Near E/H field data */
extern near_field_t near_field;

/* Segments for drawing structure */
extern GdkSegment *structure_segs ;

/* Global tag number for geometry editors */
extern gint gbl_tag_num;

extern GtkWidget *error_dialog ;

/* Tree list stores */
extern GtkListStore
  *cmnt_store,
  *geom_store,
  *cmnd_store ;

extern data_t data;

extern complex double *cm;

/* Frequency step entry widget */
extern GtkEntry *structure_fstep_entry;

/* Needed data */
extern impedance_data_t impedance_data;

/* Data for various calculations */
extern calc_data_t calc_data;

/* Magnitude of seg/patch current/charge */
extern double *cmag, *ct1m, *ct2m;

/* common  /crnt/ */
extern crnt_t crnt;

/* common  /dataj/ */
extern dataj_t dataj;

/* common  /data/ */
extern data_t data;

/* pointers to input/output files */
extern FILE *input_fp, *output_fp, *plot_fp;

/* Input file name */
extern char infile[81];

/* common  /fpat/ */
extern fpat_t fpat;

/*common  /ggrid/ */
extern ggrid_t ggrid;

/* common  /gnd/ */
extern gnd_t gnd;

/* common  /gwav/ */
extern gwav_t gwav;

/* common  /incom/ */
extern incom_t incom;

/* common  /matpar/ */
extern matpar_t matpar;

/* common  /netcx/ */
extern netcx_t netcx;

/* common  /save/ */
extern save_t save;

/* common  /segj/ */
extern segj_t segj;

/* common  /smat/ */
extern smat_t smat;

/* common  /vsorc/ */
extern vsorc_t vsorc;

/* common  /zload/ */
extern zload_t zload;

#define MOTION_EVENTS_COUNT 8

/*------------------------------------------------------------------------*/

#endif
