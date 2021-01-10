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

#include "shared.h"

/*------------------------------------------------------------------------*/

/* Runtime config file */
rc_config_t rc_config;

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
  *excitation_command = NULL, /* Ecitation command editor */
  *frequency_command  = NULL, /* Frequency command editor */
  *ground_command	  = NULL, /* Ground command editor (GN) */
  *ground2_command	  = NULL, /* Ground command editor (GD) */
  *loading_command	  = NULL, /* Loading command editor 	*/
  *network_command	  = NULL, /* Network command editor	*/
  *txline_command	  = NULL, /* Tran Line command editor */
  *radiation_command  = NULL, /* Rad Pattern command editor */
  *nearfield_command  = NULL, /* Near Fields command editor */
  *kernel_command	  = NULL, /* Thin-wire Kernel command editor  */
  *intrange_command	  = NULL, /* Interaction Range command editor */
  *execute_command	  = NULL, /* Execute (EX card) command editor */
  *zo_command		  = NULL; /* Transmission Line Zo command editor */

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
  *mainwin_frequency   = NULL;

projection_parameters_t
  rdpattern_proj_params,
  structure_proj_params;

/* Scroll adjustments of geometry
 * and command treeview windows */
GtkAdjustment
  *geom_adjustment = NULL,
  *cmnd_adjustment = NULL;

GtkTreeView
  *cmnt_treeview = NULL,
  *geom_treeview = NULL,
  *cmnd_treeview = NULL;

/* Main, frequency plots and radation pattern windows */
GtkWidget
  *main_window		= NULL,
  *freqplots_window = NULL,
  *rdpattern_window = NULL,
  *nec2_edit_window = NULL;

/* Drawing area widgets */
GtkWidget
  *structure_drawingarea = NULL,
  *freqplots_drawingarea = NULL,
  *rdpattern_drawingarea = NULL;

GtkWidget *saveas_drawingarea = NULL;

/* Sizes of above */
int
  structure_width,
  structure_height,
  freqplots_width,
  freqplots_height,
  rdpattern_width,
  rdpattern_height;

GtkBuilder
  *main_window_builder		 = NULL,
  *freqplots_window_builder  = NULL,
  *rdpattern_window_builder  = NULL,
  *animate_dialog_builder	 = NULL,
  *excitation_editor_builder = NULL,
  *radiation_editor_builder  = NULL,
  *quit_dialog_builder		 = NULL,
  *frequency_editor_builder  = NULL,
  *ground_editor_builder	 = NULL,
  *loading_editor_builder	 = NULL,
  *ground2_editor_builder	 = NULL,
  *network_editor_builder	 = NULL,
  *txline_editor_builder	 = NULL,
  *nearfield_editor_builder  = NULL,
  *kernel_editor_builder	 = NULL,
  *intrange_editor_builder	 = NULL,
  *zo_editor_builder		 = NULL,
  *execute_editor_builder	 = NULL,
  *wire_editor_builder		 = NULL,
  *arc_editor_builder		 = NULL,
  *helix_editor_builder		 = NULL,
  *patch_editor_builder		 = NULL,
  *reflect_editor_builder	 = NULL,
  *scale_editor_builder		 = NULL,
  *cylinder_editor_builder	 = NULL,
  *transform_editor_builder  = NULL,
  *gend_editor_builder		 = NULL,
  *nec2_editor_builder		 = NULL,
  *nec2_save_dialog_builder  = NULL;

/* xnec2c's glade file */
char xnec2c_glade[64];

/* Motion event handler id */
gulong rdpattern_motion_handler;

/* Dialog widgets */
GtkWidget *quit_dialog		= NULL;
GtkWidget *animate_dialog	= NULL;
GtkWidget *error_dialog		= NULL;
GtkWidget *nec2_save_dialog = NULL;

/* File chooser/select widgets */
GtkWidget *file_chooser = NULL;

/* Tree view clicked on by user */
GtkTreeView *selected_treeview = NULL;

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

/* Frequency loop idle function tag */
guint floop_tag = 0;

/* Radiation pattern data */
rad_pattern_t *rad_pattern = NULL;

/* Near E/H field data */
near_field_t near_field;

/* Segments for drawing structure */
Segment_t *structure_segs = NULL;

/* Global tag number for geometry editors */
gint tag_num = 0;

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
double
  *cmag = NULL,
  *ct1m = NULL,
  *ct2m = NULL;

/* common  /crnt/ */
crnt_t crnt;

/* common  /dataj/ */
dataj_t dataj;

/* pointers to input/output files */
FILE *input_fp = NULL;

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

