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

#ifndef SHARED_H
#define SHARED_H    1

#include "common.h"
#include "fork.h"
#include "console.h"

/*------------------------------------------------------------------------*/

/* Runtime config file */
extern rc_config_t rc_config;

/* Rad-pattern content predicates.  Each field selects its overlay only while
 * the rad-pattern window holds DRAW_ENABLED, so a closed window draws none. */
static inline int draw_efield_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_e_field; }
static inline int draw_hfield_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_h_field; }
static inline int draw_poynting_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_poynting_vector; }
static inline int overlay_struct_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_overlay_structure; }

/* Structure-view content predicates.  The main window always draws, so the
 * enum member alone selects the current or charge overlay. */
static inline int struct_view_currents(void)
  { return rc_config.structure_view == STRUCT_VIEW_CURRENTS; }
static inline int struct_view_charges(void)
  { return rc_config.structure_view == STRUCT_VIEW_CHARGES; }

/* Rad-pattern field-mode predicates.  Each selects its field only while the
 * rad-pattern window holds DRAW_ENABLED, so a closed window draws none. */
static inline int rdpat_gain_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_mode == RDPAT_FIELD_GAIN; }
static inline int rdpat_ehfield_active(void)
  { return isFlagSet(DRAW_ENABLED) && rc_config.rdpattern_mode == RDPAT_FIELD_EHFIELD; }

/* Flag to control verify_segments check */
extern gboolean skip_verify_segments;
extern gboolean show_seg_labels;

extern char *orig_numeric_locale;

/* Editor windows */
extern GtkWidget
  *wire_editor,     /* Wire designer window  */
  *arc_editor,      /* Arc designer window   */
  *helix_editor,    /* Helix designer window */
  *patch_editor,    /* Patch designer window */
  *reflect_editor,  /* Reflect design window */
  *scale_editor,    /* Scale designer window */
  *cylinder_editor, /* Cylinder designer window  */
  *transform_editor,/* Transform designer window */
  *gend_editor;     /* Geom End designer window  */

/* Command windows */
extern GtkWidget
  *excitation_command,  /* Ecitation command editor */
  *frequency_command,   /* Frequency command editor */
  *ground_command,      /* Ground command editor (GN) */
  *ground2_command,     /* Ground command editor (GD) */
  *loading_command,     /* Loading command editor   */
  *network_command,     /* Network command editor   */
  *txline_command,      /* Tran Line command editor */
  *radiation_command,   /* Rad Pattern command editor */
  *nearfield_command,   /* Near Fields command editor */
  *kernel_command,      /* Thin-wire Kernel command editor  */
  *intrange_command,    /* Interaction Range command editor */
  *execute_command,     /* Execute (EX card) command editor */
  *zo_command;          /* Tx Line Zo (ZO card) command editor */

/* Frequency step entry */
extern GtkEntry *rdpattern_fstep_entry ;

/* Per-notch angular step for rotation and incline spin wheel scroll. */
#define SCROLL_ANGLE_INCREMENT  5.0

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

/* Child process descriptors */
extern child_proc_t **child_procs;

extern view_t
  *rdpattern_view,
  *structure_view;

/* Number of forked child processes */
extern int num_child_procs;

/* Recursive lock for frequency data to prevent use of data populated by
   Get_Freq_Data() and New_Frequency() before it is done filling the data
   buffers.  Recursive so the idle wrapper can hold the lock while flushing
   GTK events that fire draw handlers which re-acquire it. */
extern GRecMutex freq_data_lock;

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

/* Main, frequency plots and radiation pattern windows */
extern GtkWidget
  *main_window,
  *freqplots_window,
  *rdpattern_window,
  *nec2_edit_window,
  *sy_overrides_window;

/* Structure drawing widgets */
extern GtkWidget
  *structure_drawingarea,
  *structure_cairo_da,
  *structure_gl_area;

/* Radiation pattern drawing widgets */
extern GtkWidget
  *rdpattern_drawingarea,
  *rdpattern_gl_area,
  *rdpattern_cairo_da;

/* Frequency plots and save-as */
extern GtkWidget
  *saveas_drawingarea;

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
  *nec2_save_dialog_builder,
  *sy_overrides_builder,
  *render_settings_builder;

/* Drawing area widgets */
extern GtkWidget
  *rdpattern_drawingarea,
  *rdpattern_gl_area,
  *rdpattern_cairo_da;

extern GtkWidget *saveas_drawingarea;

/* Sizes of above */
extern int
  structure_width,
  structure_height,
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

/* Shared flow phase advanced by unified tick, read by all backends */
extern float flow_phase;

/* Frequency loop idle function tag */
extern guint floop_tag;

/* Radiation pattern data */
extern rad_pattern_t *rad_pattern ;

/* Near E/H field data */
extern near_field_t near_field;

/* Per-frequency-step near field storage */
extern near_field_t *near_field_fstep;

/* Global tag number for geometry editors */
extern gint tag_num;

/* Tree list stores */
extern GtkListStore
  *cmnt_store,
  *geom_store,
  *cmnd_store ;

extern filechooser_t *filechooser_callback;

extern data_t data;

extern complex double *cm;

/* Frequency step entry widget */
extern GtkEntry *structure_fstep_entry;

/* Needed data; array-of-structs indexed [fstep], each member sized to
 * Num_Feedpoint_Ports() and indexed [port] within one fstep */
extern impedance_data_t *impedance_data;

/* Data for various calculations */
extern calc_data_t calc_data;

/* Magnitude of seg/patch current/charge */
extern double *cmag, *ct1m, *ct2m;

/* common  /crnt/ */
extern crnt_t crnt;

/* Per-frequency-step crnt storage */
extern crnt_t *crnt_fstep;

/* common  /dataj/ */
extern dataj_t dataj;

/* common  /data/ */
extern data_t data;

/* pointers to input/output files */
extern FILE *input_fp, *output_fp, *plot_fp;

/* common  /fpat/ */
extern fpat_t fpat;

/* True when the excitation defines a feedpoint: applied-E voltage source or
 * current-slope discontinuity.  False for incident-field and elementary-
 * current-source excitations, which drive no segment and leave input
 * impedance, VSWR, and feedpoint gain undefined. */
  static inline gboolean
fpat_has_feedpoint( void )
{
  return (fpat.ixtyp == 0) || (fpat.ixtyp == 5);
}

/*common  /ggrid/ */
extern ggrid_t ggrid;

/* common  /gnd/ */
extern gnd_t gnd;

/* Real ground predicate: TRUE when NEC2 GN card defined a real or
 * perfect ground (ksymp==2) with a valid ground type (iperf>=0).
 * Parse-time immutable after input processing. */
static inline gboolean
gnd_has_real_ground(void) { return( gnd.ksymp == 2 && gnd.iperf >= 0 ); }

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

/* Count of excitation feedpoint ports: applied-field voltage sources followed
 * by current-slope discontinuity sources.  Single source of truth for the
 * port axis, shared by allocation, IPC sizing, combo population, and dumps. */
  static inline int
Num_Feedpoint_Ports( void )
{
  return vsorc.nsant + vsorc.nvqd;
}

/* One-based segment number owning port p. */
  static inline int
Feedpoint_Port_Seg( int p )
{
  return (p < vsorc.nsant) ? vsorc.isant[p] : vsorc.ivqd[p - vsorc.nsant];
}

/* Frequency-constant EX voltage driving port p. */
  static inline complex double
Feedpoint_Port_Voltage( int p )
{
  return (p < vsorc.nsant) ? vsorc.vsant[p] : vsorc.vqd[p - vsorc.nsant];
}

/* NEC2 tag number of the segment owning port p. */
  static inline int
Feedpoint_Port_Tag( int p )
{
  return data.segments[Feedpoint_Port_Seg(p) - 1].itag;
}

/* common  /zload/ */
extern zload_t zload;

/* Comment cards storage */
extern comments_t comments;


/*------------------------------------------------------------------------*/

/* compute_zoom_scale()
 *
 * Compute zoom scaling factor based on viewport dimensions and current zoom level.
 * Accounts for three factors:
 * - Diagonal normalization: larger viewports get proportionally larger increments
 * - Base 2x multiplier: makes zoom approximately twice as fast as original
 * - Logarithmic damping: prevents jarring jumps at high zoom levels
 */
  static inline double
compute_zoom_scale(int viewport_width, int viewport_height, double zoom_percent)
{
  double diagonal, zoom_normalized;

  diagonal = sqrt((double)(viewport_width * viewport_width + viewport_height * viewport_height));
  zoom_normalized = zoom_percent / 100.0;

  if( zoom_normalized < 1.0 )
  {
    zoom_normalized = 1.0;
  }

  return (diagonal / 707.0) * (2.0 / (1.0 + log2(zoom_normalized)));

} /* compute_zoom_scale() */

/*------------------------------------------------------------------------*/

#endif
