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

#ifndef SHARED_H
#define SHARED_H	1

#include "common.h"
#include "fork.h"

/*------------------------------------------------------------------------*/

/* Runtime config file */
extern rc_config_t rc_config;

/* Editor windows */
extern GtkWidget
  *wire_editor,		/* Wire designer window  */
  *arc_editor,		/* Arc designer window   */
  *helix_editor,	/* Helix designer window */
  *patch_editor,	/* Patch designer window */
  *reflect_editor,	/* Reflect design window */
  *scale_editor,	/* Scale designer window */
  *cylinder_editor,	/* Cylinder designer window  */
  *transform_editor,/* Transform designer window */
  *gend_editor;		/* Geom End designer window  */

/* Command windows */
extern GtkWidget
  *excitation_command,	/* Ecitation command editor */
  *frequency_command,	/* Frequency command editor */
  *ground_command,		/* Ground command editor (GN) */
  *ground2_command,		/* Ground command editor (GD) */
  *loading_command,		/* Loading command editor 	*/
  *network_command,		/* Network command editor	*/
  *txline_command,		/* Tran Line command editor */
  *radiation_command,	/* Rad Pattern command editor */
  *nearfield_command,	/* Near Fields command editor */
  *kernel_command,		/* Thin-wire Kernel command editor  */
  *intrange_command,	/* Interaction Range command editor */
  *execute_command,		/* Execute (EX card) command editor */
  *zo_command;			/* Tx Line Zo (ZO card) command editor */

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
  *cmnt_treeview,
  *geom_treeview,
  *cmnd_treeview ;

/* Main, frequency plots and radation pattern windows */
extern GtkWidget
  *main_window,
  *freqplots_window,
  *rdpattern_window,
  *nec2_edit_window;

extern GtkBuilder
  *main_window_builder,
  *freqplots_window_builder,
  *rdpattern_window_builder,
  *animate_dialog_builder,
  *quit_dialog_builder,
  *excitation_editor_builder,
  *radiation_editor_builder,
  *frequency_editor_builder,
  *ground2_editor_builder,
  *ground_editor_builder,
  *loading_editor_builder,
  *network_editor_builder,
  *txline_editor_builder,
  *nearfield_editor_builder,
  *kernel_editor_builder,
  *intrange_editor_builder,
  *zo_editor_builder,
  *execute_editor_builder,
  *wire_editor_builder,
  *arc_editor_builder,
  *helix_editor_builder,
  *patch_editor_builder,
  *reflect_editor_builder,
  *scale_editor_builder,
  *cylinder_editor_builder,
  *transform_editor_builder,
  *gend_editor_builder,
  *nec2_editor_builder,
  *nec2_save_dialog_builder;

/* xnec2c's glade file */
extern char xnec2c_glade[64];

/* Drawing area widgets */
extern GtkWidget
  *structure_drawingarea,
  *freqplots_drawingarea,
  *rdpattern_drawingarea;

extern GtkWidget *saveas_drawingarea;

/* Sizes of above */
extern int
  structure_width,
  structure_height,
  freqplots_width,
  freqplots_height,
  rdpattern_width,
  rdpattern_height;

/* Motion event handler id */
extern gulong rdpattern_motion_handler;

/* Dialog widgets */
extern GtkWidget *quit_dialog ;
extern GtkWidget *animate_dialog ;
extern GtkWidget *error_dialog ;
extern GtkWidget *nec2_save_dialog;

/* File chooser/select widgets */
extern GtkWidget *file_chooser;

/* Tree view clicked on by user */
extern GtkTreeView *selected_treeview;

/* Used to kill window deleted by user */
extern GtkWidget *kill_window ;

/* Animation timeout callback tag */
extern guint anim_tag;

/* Frequency loop idle function tag */
extern guint floop_tag;

/* Radiation pattern data */
extern rad_pattern_t *rad_pattern ;

/* Near E/H field data */
extern near_field_t near_field;

/* Segments for drawing structure */
extern Segment_t *structure_segs ;

/* Global tag number for geometry editors */
extern gint tag_num;

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

/* Segments for drawing structure */
extern Segment_t *structure_segs;

#define MOTION_EVENTS_COUNT 8

/*------------------------------------------------------------------------*/

#endif
