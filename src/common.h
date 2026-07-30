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

#ifndef COMMON_H
#define COMMON_H    1

#include <complex.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <locale.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_OPENMP
#include <omp.h>
#endif

#include "measurements.h"

#include "branch_hints.h"

#include "mem/mem.h"
#include "i18n.h"
#include "view/view_core.h"
#include "color/color_tone.h"

// Define GSourceOnceFunc if compiling against an older version of GLIB:
#if GLIB_VERSION_CUR_STABLE < G_ENCODE_VERSION(2,74)
typedef void (* GSourceOnceFunc) (gpointer user_data);
#endif

#undef _Complex_I
#define _Complex_I  (__extension__ 1.0i)
#undef I
#define I _Complex_I

/* GNU/Hurd has no hard PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// __LOCATION__ and STRINGIFY are defined in location.h, already pulled in
// above through mem.h, so headers can use __LOCATION__ without depending on
// common.h include order:
#include "location.h"

typedef struct Segment
{
  gint  x1;
  gint  x2;
  gint  y1;
  gint  y2;
  float z_mid; /* camera-axis midpoint depth from Set_Gdk_Segment */
  float r, g, b; /* RGB color [0.0, 1.0] */
  float width;   /* line width in pixels */
} Segment_t;

/* commonly used complex constants */
#define CPLX_00 (0.0+I*0.0)
#define CPLX_01 (0.0+I*1.0)
#define CPLX_10 (1.0+I*0.0)
#define CPLX_11 (1.0+I*1.0)

#ifndef M_2PI
  #define M_2PI     6.28318530717958647692
#endif

/* common constants */
#define TORAD   1.745329252E-02
#define TODEG   57.29577951
#define ETA     376.73
#define CVEL    299.8
#define CONST1  (0.0+I*4.771341189)
#define CONST4  (0.0+I*188.365)
#define SMIN    1.0e-3

#define cmplx(r, i) ((r)+I*(i))

/* Replaces the "10000" limit used to */
/* identify segment/patch connections */
#define PCHCON  100000

/* Max length of a line read from input file */
#define LINE_LEN    512

/* Max length of strings used for creating file names */
#define FILENAME_LEN    PATH_MAX

/* Size of char arrays (strings) for error messages etc */
#define MESG_SIZE   128

/* Type of projection parameters struct */
#define STRUCTURE_DRAWINGAREA   1
#define RDPATTERN_DRAWINGAREA   2

/* Per-vertex RGB color (float precision, renderer-agnostic) */
typedef struct { float r, g, b; } rgb_f_t;

/* Named color constants for rendering */
extern const rgb_f_t COLOR_WHITE;
extern const rgb_f_t COLOR_CYAN;
extern const rgb_f_t COLOR_MAGENTA;
extern const rgb_f_t COLOR_GREY;
extern const rgb_f_t COLOR_BLACK;

/* Comma-separated expansion for legacy cairo_set_source_rgb(cr, COLOR) calls */
#define WHITE      (double)(COLOR_WHITE.r),      (double)(COLOR_WHITE.g),      (double)(COLOR_WHITE.b)
#define GREY       (double)(COLOR_GREY.r),       (double)(COLOR_GREY.g),       (double)(COLOR_GREY.b)
#define BLACK      (double)(COLOR_BLACK.r),      (double)(COLOR_BLACK.g),      (double)(COLOR_BLACK.b)

/**
 * seg_set_color() - Assign a named color to a Segment_t
 * @s: target segment
 * @c: color constant
 */
static inline void
seg_set_color(Segment_t *s, rgb_f_t c)
{
  s->r = c.r;
  s->g = c.g;
  s->b = c.b;
}

/* Float-comparison helpers with ε tolerance (1e-6 default).
 * fl_feq(a,b) = |a-b| <= ε; fl_flt: a < b-ε; fl_fgt: a > b+ε */
#define FL_EPS 1e-6f
static inline int fl_feq(float a, float b) { return fabsf(a - b) <= FL_EPS; }
static inline int fl_flt(float a, float b) { return a < b - FL_EPS; }
static inline int fl_fgt(float a, float b) { return a > b + FL_EPS; }

/* Double-precision comparison helpers (same semantics as fl_* above) */
#define DL_EPS 1e-9
static inline int dl_feq(double a, double b) { return fabs(a - b) <= DL_EPS; }
static inline int dl_flt(double a, double b) { return a < b - DL_EPS; }
static inline int dl_fgt(double a, double b) { return a > b + DL_EPS; }

/* Parameterized double-precision equality with caller-specified tolerance */
static inline int dl_feq_eps(double a, double b, double eps) { return fabs(a - b) <= eps; }

/* For read/write pipes */
#define READ    0
#define WRITE   1

/* Response to error dialog box */
#define ERR_OK      FALSE
#define ERR_STOP    TRUE

/* Frequency comparison tolerance: 1 Hz expressed in MHz */
#define FREQ_EPSILON_MHZ  1e-6
#define FREQ_EQ(a, b)     (fabs((a) - (b)) <= FREQ_EPSILON_MHZ)

/*** Flow control flags ***/
/* Freq Loop Control flags */
#define FREQ_LOOP_RUNNING   0x0000000000000001ll
#define FREQ_LOOP_DONE      0x0000000000000002ll
#define FREQ_LOOP_INIT      0x0000000000000004ll
#define FREQ_LOOP_STOP      0x0000000000000008ll

/* Main Window Control flags */
#define MAIN_QUIT           0x0000000000000200ll

/* Freq Plot Control flags */
#define PLOT_SELECT         0x0000000000040000ll
#define PLOT_ENABLED        0x0000000000080000ll
#define PLOT_QUIT           0x0000000000100000ll

/* Radiation Pattern Control flags */
#define DRAW_ENABLED        0x0000000008000000ll
#define DRAW_QUIT           0x0000000010000000ll
#define DRAW_NEW_RDPAT      0x0000000020000000ll
#define DRAW_NEW_EHFIELD    0x0000000040000000ll
#define ANIMATE             0x0000000200000000ll
#define ENABLE_RDPAT        0x0000000400000000ll
#define ENABLE_NEAREH       0x0000000800000000ll
#define DRAW_FLAGS ( \
  DRAW_ENABLED      | \
  DRAW_QUIT         | \
  DRAW_NEW_RDPAT    | \
  DRAW_NEW_EHFIELD  | \
  ANIMATE )

/*** Filechooser response related flags ***/
#define NEC2_EDIT_SAVE      0x0000001000000000ll /* Save NEC2 editor data to a file */
#define NEC2_SAVE           0x0000002000000000ll /* Saving to a NEC2 input file */
#define IMAGE_SAVE          0x0000004000000000ll /* Struct/rad pattern/freq plots image save */
#define RDPAT_GNUPLOT_SAVE  0x0000008000000000ll /* Save rad pattern as gnuplot data */
#define PLOTS_GNUPLOT_SAVE  0x0000010000000000ll /* Save feq plots as gnuplot data */
#define STRUCT_GNUPLOT_SAVE 0x0000020000000000ll /* Save structure as gnuplot data */
#define OPEN_INPUT          0x0000040000000000ll /* Open a NEC2 input file flag */
#define OPEN_NEW_NEC2       0x0000080000000000ll /* Open a new NEC2 project flag */
#define ALL_CHOOSER_FLAGS ( \
  NEC2_EDIT_SAVE      | \
  NEC2_SAVE           | \
  IMAGE_SAVE          | \
  RDPAT_GNUPLOT_SAVE  | \
  PLOTS_GNUPLOT_SAVE  | \
  STRUCT_GNUPLOT_SAVE | \
  OPEN_INPUT          | \
  OPEN_NEW_NEC2 )

/* Other flags */
#define ERROR_CONDX         0x0000100000000000ll
#define INPUT_PENDING       0x0000200000000000ll
#define ENABLE_EXCITN       0x0000400000000000ll

/* Allocation control flags */
#define ALLOC_NEAREH_BUFF   0x0000800000000000ll
#define ALLOC_RDPAT_BUFF    0x0001000000000000ll

/* Force editors to quit */
#define EDITOR_QUIT         0x0004000000000000ll

/* Frequency loop results ready */
#define FREQ_LOOP_READY     0x0008000000000000ll

/* Block motion events till ready */
#define BLOCK_MOTION_EV     0x0010000000000000ll


/* Signal start of xnec2c */
#define XNEC2C_START        0x0040000000000000ll

// Suppress intermediate redraws during frequency loops
#define SUPPRESS_INTERMEDIATE_REDRAWS    0x0080000000000000ll

/* SY optimizer evaluation in progress — suppresses inotify reload */
#define SY_OPTIMIZER_ACTIVE 0x0200000000000000ll

#define ALL_FLAGS           0xFFFFFFFFFFFFFFFFll

/* Type of near field data requested */
#define NEAR_EFIELD     0x01
#define NEAR_HFIELD     0x02
#define NEAR_EHFIELD    0x03

#define CHILD   isChild()

/* Maximum length for mathlib ID strings (including null terminator) */
#define MATHLIB_ID_LEN 32

typedef enum {
  RDPAT_PNG_FORMAT_ISO,
  RDPAT_PNG_FORMAT_X,
  RDPAT_PNG_FORMAT_Y,
  RDPAT_PNG_FORMAT_Z,
  RDPAT_PNG_FORMAT_QUAD,
  RDPAT_PNG_FORMAT_COUNT,
} rdpat_png_format_t;

typedef struct {
  const char *name;
  rdpat_png_format_t format;
} rdpat_png_format_spec_t;

/* Runtime configuration data */
typedef struct
{
  /* Current working directory */
  char working_dir[FILENAME_LEN];

  /* Current NEC2 input file */
  char input_file[FILENAME_LEN];

  /* Custom config file path */
  char config_file[FILENAME_LEN];

  /* If set true, then use GTK loops for the frequency loop iteration
   * instead of spawning a pthread */
  int disable_pthread_freqloop;

  /* Main (structure) window position and size */
  int
    main_x,
    main_y,
    main_width,
    main_height;

  /* Main (structure) window state of widgets.  structure_view holds a
   * struct_view_t member selecting geometry, charges, or currents. */
  int
    structure_view,
    main_loop_start;

  int
    main_rotate_spinbutton,
    main_incline_spinbutton,
    main_zoom_spinbutton;

  /* Radiation pattern window position and size */
  int
    rdpattern_is_open,
    rdpattern_x,
    rdpattern_y,
    rdpattern_width,
    rdpattern_height,
    rdpattern_zoom_spinbutton;

  /* Radiation pattern window state of widgets.  rdpattern_mode holds a
   * rdpat_mode_t member selecting far-field gain or near E/H field. */
  int
    rdpattern_mode,
    rdpattern_e_field,
    rdpattern_h_field,
    rdpattern_poynting_vector,
    rdpattern_gradient_key;

  /* Frequency plots window state of widgetsposition and size */
  int
    freqplots_is_open,
    freqplots_x,
    freqplots_y,
    freqplots_width,
    freqplots_height;

  /* Frequency plots window state of widgets */
  int
    freqplots_gmax_togglebutton,
    freqplots_gdir_togglebutton,
    freqplots_gviewer_togglebutton,
    freqplots_vswr_togglebutton,
    freqplots_zrlzim_togglebutton,
    freqplots_zmgzph_togglebutton,
    freqplots_smith_togglebutton,
    freqplots_ant_temp_togglebutton,
    freqplots_show_ant_temp,
    freqplots_min_max,
    freqplots_s11,
    freqplots_clamp_vswr,
    freqplots_net_gain,
    freqplots_swap_click;

  /* Frequency plots color theme base name and inversion toggle; the composed
   * lookup key is derived at access time, never stored. */
  char freqplots_theme[64];
  int  freqplots_theme_invert;

  /* NEC2 editor window position and size */
  int
    nec2_edit_x,
    nec2_edit_y,
    nec2_edit_width,
    nec2_edit_height;

  /* Enable Quit dialog */
  int confirm_quit;

  /* Apply frequency spinbutton changes immediately vs. display-only
   * preview; session-only, no persistence row. */
  int freq_apply;

  /* Preferred mathlib identifiers for persistence */
  char mathlib_id[MATHLIB_ID_LEN];
  char mathlib_batch_id[MATHLIB_ID_LEN];

  /* See enum GAIN_SCALE */
  int gain_style;

  /* If true, then call Fit_to_Scale() on the X-axis
   * fr_plot->min_fscale/fr_plot->max_fscale values. */
  int freqplots_round_x_axis;

  /* if true, exit after the first frequency loop iteration */
  int batch_mode;

  /* true to skip verify_segments check */
  int skip_verify_segments;

  /* force overlap check for large models */
  int force_verify_segments;

  /* verbose and debug levels, see console.h */
  int verbose, debug;

  /* Report managed-allocator live bytes per call site at instrumented
   * checkpoints (--mem-report); global allocator diagnostics */
  int mem_report_enabled;

  /* true if ~/.xnec2c/xnec2c.conf does not exist, false otherwise */
  int first_run;

  /* optimization file write flags */
  int opt_write_csv;
  int opt_write_s1p;
  int opt_write_s2p_max_gain;
  int opt_write_s2p_viewer_gain;
  int opt_write_rdpat;
  int opt_write_currents;
  int opt_write_gnuplot_structure;
  int opt_write_patch_currents;

  /* If for example --write-csv <file> is added to the cmdline then write
   * the file after each frequency iteration */
  char *filename_csv;
  char *filename_s1p;
  char *filename_s2p_max_gain;
  char *filename_s2p_viewer_gain;
  char *filename_rdpat;
  char *filename_currents;
  char *filename_gnuplot_structure;
  char *filename_patch_currents;
  char *filename_rdpat_png;
  rdpat_png_format_spec_t *rdpat_png_formats;

  /* Symbol overrides window */
  int sy_overrides_is_open;
  int sy_overrides_x;
  int sy_overrides_y;
  int sy_overrides_width;
  int sy_overrides_height;

  /* Antenna temperature calculation */
  int ant_temp_sky;       /* ant_temp_sky_t index */
  int ant_temp_earth;     /* ant_temp_earth_t index */
  int ant_temp_interp;    /* ant_temp_method_t value */
  double ant_temp_elevation; /* Antenna elevation tilt (degrees), +=up */
  double ant_temp_custom_t_sky;   /* Custom sky brightness temp (K) */
  double ant_temp_custom_t_earth; /* Custom earth brightness temp (K) */
  /* Use OpenGL rendering for radiation patterns */
  int use_opengl_renderer;

  /* Use constrained (WR/WI-delta) rotation for Cairo and GL drag */
  int view_drag_constrained;

  /* OpenGL MSAA sample count (0, 2, 4, 8, or 16) */
  int opengl_msaa_samples;

  /* OpenGL cylinder radius display scale factor */
  double opengl_cylinder_radius_scale;

  /* Overlay structure scale adjustment (user shift+scroll) */
  double rdpattern_overlay_scale_adj;

  /* Main window common projection toggle */
  int main_common_projection;

  /* Rdpattern overlay structure toggle */
  int rdpattern_overlay_structure;

  /* Patch current flow visualization mode (View menu, not OpenGL settings) */
  int current_flow_visualization_mode;

  /* Near-field static baseline mode (View menu total-field submenu);
   * holds an nf_static_mode_t peak/snapshot selection. */
  int nf_static_mode;

  /* Animated color projection (animate dialog, chroma_proj_t) */
  int anim_color_proj;

  /* Tone family selector (color_tone_t), shared by menu and animate dialog */
  int color_scale;

  /* Per-family tone parameter in the slider domain, indexed by color_tone_t */
  double color_fam_param[COLOR_TONE_NUM];

  /* Brightness floor in [0,1] keeping geometry visible at wave nulls */
  double color_lum_floor;

  /* Width-from-amplitude carrier gate */
  int color_width_amp;

  /* Wire overlay gates: comet crest, node/antinode marks */
  int overlay_comet, overlay_nodes;

  /* Whether transparency is triggered by click/drag (1) or always on (0) */
  int opengl_transparent_on_click;

  /* Orthographic projection toggle (0=perspective, 1=orthographic) */
  int opengl_orthographic;

  /* See enum RDPAT_STYLE */
  int rdpattern_draw_style;

  /* Per-type brightness (0.0=black, 1.0=full) */
  float brightness_segments;
  float brightness_patches;
  float brightness_rdpat_surface;
  float brightness_rdpat_wire;
  float brightness_nearfield;
  float brightness_ground_plane;
  float brightness_axes;

  /* Per-type transparency (0.0=opaque, 1.0=fully transparent) */
  float transparency_segments;
  float transparency_patches;
  float transparency_rdpat_surface;
  float transparency_rdpat_wire;
  float transparency_nearfield;
  float transparency_ground_plane;
  float transparency_axes;

  /* Cairo rendering settings */
  cairo_antialias_t cairo_antialias;   /* CAIRO_ANTIALIAS_{DEFAULT,FAST,NONE} */
  int               cairo_depth_bins;  /* >=1 z quantization bins for painter's algorithm depth sorting */
  int               cairo_color_quant; /* 0=off, N=quantization levels (64,128,256) */
  cairo_line_cap_t  cairo_line_cap;    /* CAIRO_LINE_CAP_{BUTT,ROUND,SQUARE} */
} rc_config_t;

typedef struct {
	void (*callback)(char *);
	char *extension;
} filechooser_t;

/* Radiation pattern draw style */
enum RDPAT_STYLE
{
  RDPAT_STYLE_SURFACE = 0,
  RDPAT_STYLE_WIREFRAME,
  RDPAT_STYLE_BOTH,
  NUM_RDPAT_STYLES
};

/* Main-window structure display selection, stored under the reused
 * "currents" config key so an old file's 0/1 value keeps its meaning. */
typedef enum
{
  STRUCT_VIEW_DISABLED = -1, /* geometry only; fresh default; released via the toggle group; never written by old files */
  STRUCT_VIEW_CHARGES  = 0,  /* reuses the currents-key value 0 */
  STRUCT_VIEW_CURRENTS = 1   /* reuses the currents-key value 1 */
} struct_view_t;

/* Radiation-pattern field display selection, stored under the reused
 * "gain" config key so an old file's 0/1 value keeps its meaning. */
typedef enum
{
  RDPAT_FIELD_DISABLED = -1, /* neither field; released via the toggle group; never written by old files */
  RDPAT_FIELD_EHFIELD  = 0,  /* reuses the gain-key value 0 */
  RDPAT_FIELD_GAIN     = 1   /* reuses the gain-key value 1; fresh default (far field) */
} rdpat_mode_t;

/* Gain Scaling style */
enum GAIN_SCALE
{
  GS_LINP = 0,
  GS_LINV,
  GS_ARRL,
  GS_LOG,
  GS_NOISE,
  GS_NOISE_LOG,
  NUM_SCALES
};

#define IS_NOISE_MODE(gs) ((gs) == GS_NOISE || (gs) == GS_NOISE_LOG)
/* OpenGL MSAA sample counts */
enum OPENGL_MSAA
{
  MSAA_OFF = 0,
  MSAA_2X = 2,
  MSAA_4X = 4,
  MSAA_8X = 8,
  MSAA_16X = 16
};

/* Minimum gain value used for color mapping */
#define COLOR_MIN_GAIN -60.0

/* Polarization type */
enum POL_TYPE
{
  POL_TOTAL = 0,
  POL_HORIZ,
  POL_VERT,
  POL_RHCP,
  POL_LHCP,
  NUM_POL
};

/** gl_draw_batch_t - Self-contained vertex batch for a single glDrawArrays call
 * @vertices: owned vertex allocation (caller manages lifetime)
 * @vertex_count: number of vertices to draw
 * @draw_mode: GL primitive mode (GL_TRIANGLES, GL_LINES, etc.) as unsigned int
 *             to avoid GL header dependency outside HAVE_OPENGL scope
 */
typedef struct
{
  void *vertices;
  int vertex_count;
  unsigned int draw_mode;
  /* When TRUE, glPolygonOffset pushes this batch behind non-offset
   * batches (e.g. surface triangles behind wireframe lines). */
  gboolean polygon_offset;

  /* Per-batch RGB brightness multiplier (0.0=black, 1.0=full) */
  float color_dim;

  /* Per-batch transparency (0.0=invisible, 1.0=opaque) */
  float alpha;

  /* GL_LINES draw width in pixels; <=0 falls back to 1.0 */
  float line_width;

} gl_draw_batch_t;

/* Maximum independent draw batches per view content
 * (structure: segments, patches, network/transmission-line outlines,
 * two-port network polygon fills, node/antinode glyph overlay) */
#define GL_VIEW_MAX_BATCHES 5

/* OpenGL types */
#ifdef HAVE_OPENGL
typedef struct
{
  float r, g, b, a;

} rgba_f_t;

typedef struct
{
  float x, y, z, r;

} point_f_3d_t;

typedef struct
{
  point_f_3d_t point;
  rgba_f_t color;

} color_point_t;

typedef struct
{
  point_f_3d_t point;
  point_f_3d_t normal;
  rgba_f_t color;

} lit_color_point_t;

typedef struct
{
  color_point_t cp[3];

} color_triangle_t;

typedef struct
{
  lit_color_point_t cp[3];

} lit_color_triangle_t;

typedef struct
{
  float x, y, z;
  float u, v;

} text_vertex_t;
#endif /* HAVE_OPENGL */

/*** Per-element structs for wire, patch, and near-field data ***/

/* Per-wire-segment geometry, connectivity, and tag data */
typedef struct
{
  double x1, y1, z1;       /* End 1 coordinates */
  double x2, y2, z2;       /* End 2 coordinates */
  double x, y, z;          /* Segment center coordinates */
  double si, bi;            /* Length and radius */
  double cab, sab, salp;   /* Direction cosines */
  int icon1, icon2;         /* End connectivity */
  int itag;                 /* Tag number */
} wire_segment_t;

/* Per-surface-patch geometry and orientation */
typedef struct
{
  double t1x, t1y, t1z;    /* Tangent vector 1 */
  double t2x, t2y, t2z;    /* Tangent vector 2 */
  double px, py, pz;       /* Patch center coordinates */
  double pbi;               /* Surface area */
  double psalp;             /* Z component - sin(a) */
} surface_patch_t;

/* Per-patch 4 rectangle corners, 1 entry per patch */
typedef struct
{
  double c0x, c0y, c0z;    /* center + s*t1 + s*t2 */
  double c1x, c1y, c1z;    /* center - s*t1 + s*t2 */
  double c2x, c2y, c2z;    /* center - s*t1 - s*t2 */
  double c3x, c3y, c3z;    /* center + s*t1 - s*t2 */
} patch_corners_t;

/* Near-field static-baseline selector for the peak/snapshot mode; the
 * rc_config.nf_static_mode field is the single source, read at draw. */
typedef enum { NF_STATIC_PEAK = 0, NF_STATIC_SNAPSHOT } nf_static_mode_t;

/* Per-sample near-field E/H amplitude, phase, and coordinates.  The phasor
 * (amplitude + phase) is the sole stored near-field truth; the real vectors
 * and their magnitudes derive at draw from it. */
typedef struct
{
  double ex, ey, ez;            /* E-field magnitude and phase */
  double hx, hy, hz;            /* H-field magnitude and phase */
  double fex, fey, fez;         /* E-field phasor */
  double fhx, fhy, fhz;         /* H-field phasor */
  double px, py, pz;            /* Field point coordinates */
} near_field_point_t;

/*** Structs encapsulating global ("common") variables ***/
/* common  /crnt/ */
typedef struct
{
  double
    *air,   /* Ai/lambda, real part */
    *aii,   /* Ai/lambda, imaginary part */
    *bir,   /* Bi/lambda, real part */
    *bii,   /* Bi/lambda, imaginary part */
    *cir,   /* Ci/lambda, real part */
    *cii;   /* Ci/lambda, imaginary part */

  complex double *cur; /* Amplitude of basis function */

} crnt_t;

/* common  /data/ (geometry data) */
typedef struct
{
  int
    n,      /* Number of wire segments */
    np,     /* Number of wire segments in symmetry cell */
    m,      /* Number of surface patches */
    mp,     /* Number of surface patches in symmetry cell */
    npm,    /* = n+m  */
    np2m,   /* = n+2m */
    np3m,   /* = n+3m */
    ipsym;  /* Symmetry flag */

  /* Wavelength in meters */
  double wlam;

  wire_segment_t *segments;
  surface_patch_t *patches;

} data_t;

/* common  /dataj/ */
typedef struct
{
  int
    iexk,
    ind1,
    indd1,
    ind2,
    indd2,
    ipgnd;

  double
    s,
    b,
    xj,
    yj,
    zj,
    cabj,
    sabj,
    salpj,
    rkh,
    t1xj,
    t1yj,
    t1zj,
    t2xj,
    t2yj,
    t2zj;

  complex double
    exk,
    eyk,
    ezk,
    exs,
    eys,
    ezs,
    exc,
    eyc,
    ezc;

} dataj_t;

/* common  /fpat/ */
typedef struct
{
  int
    near,
    nfeh,
    nrx,
    nry,
    nrz,
    nth,
    nph,
    ipd,
    iavp,
    inor,
    iax,
    ixtyp;

  /* xpr6 moved to exec_data_t */
  double
    thets,
    phis,
    dth,
    dph,
    rfld,
    gnor,
    clt,
    cht,
    epsr2,
    sig2,
    pinr,
    pnlr,
    ploss,
    xnr,
    ynr,
    znr,
    dxnr,
    dynr,
    dznr;

} fpat_t;

/* Nearest theta grid cell for an angle in degrees. No azimuth wrap.
   dth == 0 selects the single sample, cell 0. Rounds, clamps to [0, nth). */
int fpat_theta_cell(double theta_deg);

/* Nearest phi grid cell for an angle in degrees. Normalizes into [0,360) first.
   dph == 0 selects the single sample, cell 0. Rounds, clamps to [0, nph). */
int fpat_phi_cell(double phi_deg);

/*common  /ggrid/ */
typedef struct
{
  int
    *nxa,
    *nya;

  double
    *dxa,
    *dya,
    *xsa,
    *ysa;

  complex double
    epscf,
    *ar1,
    *ar2,
    *ar3;

} ggrid_t;

/* common  /gnd/ */
typedef struct
{
  int
    ksymp,  /* Ground flag */
    ifar,   /* Int flag in RP card, for far field calculations */
    iperf,  /* Type of ground flag */
    nradl,  /* Number of radials in ground screen */
    gpflag; /* The gpflag from the GE card */

  double
    t2,     /* Const for radial wire ground impedance */
    cl,     /* Distance in wavelengths of cliff edge from origin */
    ch,     /* Cliff height in wavelengths */
    scrwl,  /* Wire length in radial ground screen normalized to w/length */
    scrwr;  /* Radius of wires in screen in wavelengths */

  complex double
    zrati,  /* Ground medium [Er-js/wE0]^-1/2 */
    zrati2, /* As above for 2nd ground medium */
    t1,     /* Const for radial wire ground impedance */
    frati;  /* (k1^2-k2^2)/(k1^2+k2^2), k1=w(E0Mu0)^1/2, k1=k2/ZRATI */

} gnd_t;

/* common  /gwav/ */
typedef struct
{
  double
    r1, /* Distance from current element to point where field is evaluated  */
    r2, /* Distance from image of element to point where field is evaluated */
    zmh, /* Z-Z', Z is height of field evaluation point */
    zph; /* Z+Z', Z' is height of current element */

  complex double
    u,  /* (Er-jS/WE0)^-1/2 */
    u2, /* u^2 */
    xx1, /* G1*exp(jkR1*r[i])  */
    xx2; /* G2*exp(jkR2*r'[i]) */

} gwav_t;

/* common  /incom/ */
typedef struct
{
  int isnor;

  double
    xo,
    yo,
    zo,
    sn,
    xsn,
    ysn;

} incom_t;

/* common  /matpar/ (matrix parameters) */
typedef struct
{
  int
    icase,  /* Storage mode of primary matrix */
    npblk,  /* Num of blocks in first (NBLOKS-1) blocks */
    nlast,  /* Num of blocks in last block */
    imat;   /* Storage reserved in CM for primary NGF matrix A */

} matpar_t;

/* common  /netcx/ */
typedef struct
{
  int
    masym,  /* Matrix symmetry flags */
    neq,
    npeq,
    neq2,
    nonet,  /* Number of two-port networks */
    ntsol,  /* "Network equations are solved" flag */
    nprint, /* Print control flag */
    *iseg1, /* Num of seg to which port 1 of network is connected */
    *iseg2, /* Num of seg to which port 2 of network is connected */
    *ntyp;  /* Type of networks */

  double
    *x11r,  /* Real and imaginary parts of network impedances */
    *x11i,
    *x12r,
    *x12i,
    *x22r,
    *x22i,
    pin,    /* Total input power from sources */
    pnls;   /* Power lost in networks */

  complex double zped;
  complex double *zped_port; /* per-port scratch; index 0..nsant-1 = voltage
                             * sources, nsant..nsant+nvqd-1 = current-slope */

} netcx_t;

/* common  /save/ */
typedef struct
{
  int *ip;  /* Vector of indices of pivot elements used to factor matrix */

  double
    epsr,       /* Relative dielectric constant of ground */
    sig,        /* Conductivity of ground */
    scrwlt,     /* Length of radials in ground screen approximation */
    scrwrt,     /* Radius of wires in ground screen approximation */
    fmhz,       /* Saved frequency of operation */
    last_freq;  /* My addition, Last frequency used in calculations */

  /* My addition, replace xtemp, ytemp etc in freq. loop */
  double
    *xtemp,
    *ytemp,
    *ztemp,
    *sitemp,
    *bitemp;

  double *freq; /* My addition, frequencies used in freq loop */
  char *fstep;  /* My addition, freq loop steps that returned results */

} save_t;

/* common  /segj/ */
typedef struct
{
  int
    *jco,   /* Stores connection data */
    jsno,   /* Total number of entries in ax, bx, cx */
    maxcon; /* Max. no. connections */

  double
    *ax, *bx, *cx; /* Store constants A, B, C used in current expansion */

} segj_t;

/* common  /smat/ */
typedef struct
{
  int nop; /* My addition */

  complex double *ssx;

} smat_t;

/* common  /vsorc/ */
typedef struct
{
  int
    *isant, /* Num of segs on which an applied field source is located */
    *ivqd,  /* Num of segs on which a current-slope discontinuity source is located */
    *iqds,  /* Same as above (?) */
    nsant,  /* Number of applied field voltage sources */
    nvqd,   /* Number of applied current-slope discontinuity sources */
    nqds;   /* Same as above (?) */

  complex double
    *vqd,   /* Voltage of applied-current slope discontinuity sources */
    *vqds,  /* Same as above (?) */
    *vsant; /* Voltages of applied field voltage sources */

} vsorc_t;

/* common  /yparm/ */
/* CP card support has been dropped */

/* common  /zload/ */
typedef struct
{
  int
    *ldsegn,    /* My addition, loaded segment numbers */
    *ldtype,    /* My addition, loading type */
    nldseg,     /* My addition, number of loaded segs  */
    nload;      /* Number of loading networks */

  complex double *zarray;   /* = Zi/(Di/lambda) */

} zload_t;

/* Comment cards storage */
typedef struct
{
  int    num;     /* Number of CM comment lines */
  char **lines;   /* Array of comment line strings */

} comments_t;

/* My addition, a struct for frequency loop related data.
 * This is needed to implement multiple FR cards so
 * that a frequency sweep can be made in separate bands
 */
/* Raw FR card parameters as read from the file, used to detect a duplicate
 * FR card identically in the parent and the fork child before either derives
 * the normalized frequency fields */
typedef struct
{
  int    type;    /* raw FR frequency-spec flag (itmp1): 0,1,2,3 */
  int    nsteps;  /* raw FR step count (itmp2) */
  double flo;     /* raw first/low frequency (tmp1) */
  double fhi;     /* raw delta-or-high frequency (tmp2) */
} fr_card_sig_t;

typedef struct
{
  int
    ifreq,       /* Frequency specification flag */
    freq_steps;  /* Number of frequency steps */

  double
    delta_freq,  /* Frequency step in freq loop */
    min_freq,    /* Min frequency in an FR card */
    max_freq;    /* Max frequency in an FR card */

  fr_card_sig_t sig;  /* raw card identity for duplicate detection */

} freq_loop_data_t;

/* Graph-type identity keying each frequency-plot panel.  The numbered members
 * follow dispatch-emission order; FP_PANEL_ALL is the primary window's
 * all-panels sentinel and FP_PANEL_COUNT bounds the per-type popup registry. */
typedef enum {
	FP_PANEL_ALL = -1,
	FP_PANEL_GAIN = 0, FP_PANEL_GAIN_DIR, FP_PANEL_VIEWER, FP_PANEL_VSWR,
	FP_PANEL_ZRLZIM, FP_PANEL_ZMGZPH, FP_PANEL_SMITH, FP_PANEL_ANT_TEMP,
	FP_PANEL_COUNT
} fp_panel_t;

/* Per-view click-resolution registry record; defined in freqplots_locus.h. */
typedef struct fp_locus_s fp_locus_t;

/*
 * fr_plot_t structure and related defines used in plot_freqdata.c
 */
#define FR_PLOT_T_MAGIC 				0xc2bca3083893e65eULL
#define FR_PLOT_T_IS_VALID(fr_plot_ptr)	((fr_plot_ptr)->valid == FR_PLOT_T_MAGIC)

typedef struct {
	GdkRectangle plot_rect;
	int posn;			// Position in the frequency plots
	int fr;				// index into calc_data.freq_loop_data[fr]

	// Which graph type this panel renders, written during draw
	fp_panel_t panel_type;

	// Pointer to &calc_data.freq_loop_data[fr]
	freq_loop_data_t *freq_loop_data;

	// Scaled versions of fr_plot->freq_loop_data->min_freq/max_freq for plot display
	// for use with rc_config.freqplots_round_x_axis
	double min_fscale, max_fscale;

	// Because we are using realloc it is hard to know if the structure has
	// been initialized or if it needs to be set to sane values.  The
	// value will equal 0xc2bca3083893e65e (just a 64-bit random number) if
	// it is valid.
	uint64_t valid;
} fr_plot_t;

/* Per-window render and input context for the frequency plots.  The primary
 * window is the sole persistent instance; popups are heap instances holding a
 * single filtered graph.  Only genuinely per-window divergent state lives
 * here; measurement and frequency state stay global and shared. */
typedef struct {
	// focus and destroy target
	GtkWidget *window;

	// render target, hit-test, font source
	GtkWidget *drawingarea;

	// base font source for text runs, 1:1 with drawingarea
	PangoLayout *text_layout;

	// this area's pixel size
	int        width, height;

	// this window's layout and hit-test table
	fr_plot_t *fr_plots;

	// resize caches
	int        prev_width_available, prev_ngraphs;

	// primary: calc_data.ngraph; popup: 1
	int        ngraph;

	// fp_panel_t shown, or FP_PANEL_ALL
	fp_panel_t filter;

	// Click deferred until this view has a layout table; replayed from the
	// draw path against this view's own geometry.
	GdkEvent  *prev_click_event;

	// Per-frame click-resolution registry, rebuilt each draw by every plot
	// producer and consulted on click; per-view so each window resolves
	// clicks against its own geometry.  Buffers persist at high-water
	// capacity across frames.
	fp_locus_t *loci;
	int         loci_n;

	// Popup selected-frequency readout bar: one cell box and its value label
	// per displayed field, refreshed each draw.  Managed arrays sized to the
	// popup's column count; NULL for the primary view, which uses the builder
	// entry grid instead.
	GtkWidget  **readout_field;
	GtkWidget  **readout_value;

	// Popup excitation-port selection and its menu button; the primary view
	// resolves its port from calc_data.ex_port and leaves ex_port unused.
	// port_saved and port_committed carry this view's hover-preview state.
	int        ex_port;
	GtkWidget *port_button;
	int        port_saved;
	gboolean   port_committed;
} freqplots_view_t;

/* My addition, struct to hold data needed
 * to execute various calculations requested
 * by commands like EX, RP, NH, NE etc.
 */
typedef struct
{
  int
    *ldtyp,
    *ldtag,
    *ldtagf,
    *ldtagt,
    nthi,
    nphi,
    iexk,
    iped,
    ngraph,     /* Number of graphs to be plotted */
    pol_type,   /* User-specified Polarization type for plots and patterns */
    ex_port,    /* Selected excitation port (0-based) for single-port consumers */
    num_jobs;   /* Number of child processes (jobs) to fork */

  double
    *zlr,
    *zli,
    *zlc,
    xpr1,
    xpr2,
    xpr3,
    xpr4,
    xpr5,
    xpr6,
    rkh,
    zpnorm,
    thetis,
    phiss,
    zo;     /* Characteristic impedance used in VSWR calcs */

  int
    FR_cards,     /* Number of FR cards in input file */
    freq_step,    /* Frequency step in frequency loop */
    steps_total;  /* Total number of frequency steps */

  double
    fmhz_save,  /* Saved value of frequency clicked on by user in plots window */
    freq_mhz;   /* Current Frequency in MHz, moved from save_t */

  freq_loop_data_t *freq_loop_data;

} calc_data_t;

/* Impedance data */
typedef struct
{
  double
    *zreal,     /* Real part of input impedance */
    *zimag,     /* Imag part of input impedance */
    *zmagn,     /* Magnitude of input impedance */
    *zphase;    /* Phase ang of input impedance */

} impedance_data_t;

/* Radiation pattern data */
typedef struct
{
  double
    *gtot,          /* Radiation pattern (total gain) buffer */
    *max_gain,      /* Maximum gain for each polarization type */
    *min_gain,      /* Minimum gain for each polarization type */
    *max_gain_tht,  /* Theta angle where maximum gain occurs */
    *max_gain_phi,  /*   Phi angle where maximum gain occurs */
    *tilt,          /* Tilt angle of polarization ellipse  */
    *axrt;          /* Elliptic axial ratio of pol ellipse */

  double
    efficiency;     /* Radiation efficiency = prad / pinr, per freq step */

  double
    noise_scaled_max,  /* True max of Scale_Gain in noise mode (K/sr) */
    noise_scaled_min;  /* True min of Scale_Gain in noise mode (K/sr) */

  int
    *max_gain_idx,  /* Where in rad_pattern.gtot the max value occurs */
    *min_gain_idx,  /* Where in rad_pattern.gtot the min value occurs */
    *sens;          /* Polarization sense (vertical, horizontal, elliptic etc) */

} rad_pattern_t;

/* Near E/H field data */
typedef struct
{
  /* Max distance from xyz origin */
  double r_max;

  /* Animation step in rads */
  double anim_step;

  near_field_point_t *points;

} near_field_t;

/* Child process descriptor */
typedef struct
{
  int   idx;             /* Index into child_procs[]; used by Write_Pipe/Get_Freq_Data */
  pid_t pid;             /* Child PID */
  int   to_child[2];     /* Pipe parent→child: [READ]=child reads, [WRITE]=parent writes */
  int   from_child[2];   /* Pipe child→parent: [READ]=parent reads, [WRITE]=child writes */
  int    assigned_step;   /* Frequency step in progress; -1 = idle */
  double assigned_freq;   /* Frequency dispatched; validated at collect */

} child_proc_t;


typedef enum
{
  MAIN_WINDOW = 1,
  FREQPLOTS_WINDOW,
  RDPATTERN_WINDOW
} window_t;

/* Function prototypes produced by cproto */
/* calculations.c */
void qdsrc(int is, complex double v, complex double *e);
void cabc(complex double *curx);
double db10(double x);
double db20(double x);
void intrp(double x, double y, complex double *f1, complex double *f2, complex double *f3, complex double *f4);
void intx(double el1, double el2, double b, int ij, double *sgr, double *sgi);
int min(int a, int b);
void test(double f1r, double f2r, double *tr, double f1i, double f2i, double *ti, double dmin);
void trio(int j);
double cang(complex double z);
void zint(double sigl, double rolam, complex double *zint);
/* callback_func.c */
gboolean Save_Pixbuf(gpointer save_data);
void Motion_Event(GdkEventMotion *event, view_t *v);
gboolean Nec2_Edit_Save(void);
void Delete_Event(gchar *mesg);
gboolean Open_Editor(GtkTreeView *view);
void
Card_Clicked(GtkWidget **editor, GtkBuilder **editor_builder, GtkWidget *create_fun(GtkBuilder **), void editor_fun(int), int *editor_action);
void Main_Rdpattern_Activate(gboolean from_menu);
gboolean Main_Freqplots_Activate(void);
void rdpattern_mode_apply(void);
void structure_view_apply(void);
GtkWidget *Open_Filechooser(GtkFileChooserAction action, char *pattern, char *prefix, char *filename, char *foldername);
void Filechooser_Response(GtkDialog *dialog, gint response_id, int saveas_width, int saveas_height);
void Open_Nec2_Editor(int action);
gboolean Nec2_Apply_Checkbutton(void);
void Gtk_Quit(void);
/* Exit-cleanup orchestration: named by-owner teardown invoked once after the
 * event loop stops, before mem_report. */
void parent_cleanup(void);
void child_cleanup(void);
void main_windows_destroy(void);
void xnec2c_request_quit(void);
void xnec2c_quit(gpointer user_data);
gboolean xnec2c_quit_if_pending(void);
void input_data_free(void);
void geometry_data_free(void);
void ggrid_free(void);
void near_field_data_free(void);
void calc_data_free(void);
void matrix_data_free(void);
void fields_data_free(void);
void calc_scratch_free(void);
void somnec_data_free(void);
void ground_data_free(void);
void gnuplot_data_free(void);
void child_procs_free(void);
void close_child_command_pipes(void);
void freqplots_cleanup(void);
void Nf_Peak_Vector(double exm, double eym, double ezm, double fx, double fy, double fz, double *rx, double *ry, double *rz, double *r);
void Draw_Colorcode(cairo_t *cr);
void draw_colorcode_projected(cairo_t *cr);
void Gtk_Widget_Destroy(GtkWidget **widget);
/* callbacks.c */
void on_main_window_destroy(GObject *object, gpointer user_data);
gboolean on_main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_new_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_open_input_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_save_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_save_as_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_struct_save_as_gnuplot_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_optimizer_output_toggled(GtkMenuItem *menuitem, gpointer user_data);
void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_rdpattern_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_freqplots_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_view_preset_clicked(GtkButton *button, gpointer user_data);
void on_main_rotate_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_main_incline_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
gboolean on_colorcode_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_anim_color_reset_clicked(GtkButton *button, gpointer user_data);
void on_new_freq_clicked(GtkButton *button, gpointer user_data);
gboolean on_structure_drawingarea_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gboolean on_structure_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
gboolean on_structure_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_filechooserdialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_filechooserdialog_destroy(GObject *object, gpointer user_data);
gboolean on_freqplots_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_freqplots_window_destroy(GObject *object, gpointer user_data);
void on_freqplots_save_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_freqplots_save_as_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_freqplots_save_as_gnuplot_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_freqplots_zo_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void freqplots_populate_port_combo(void);
GtkWidget *freqplots_port_combo_new(freqplots_view_t *view);
void freqplots_refresh_port_combos(void);
void freqplots_reload_port_combos(void);
void freqplots_gate_feedpoint_widgets(void);
const char *freqplots_panel_select_id(fp_panel_t panel);
void Alloc_Impedance_Buffers(int nfrq, int n_ports);
void Free_Impedance_Buffers(void);
void Rescan_Zpnorm(void);
gboolean on_freqplots_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_freqplots_drawingarea_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gboolean on_freqplots_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_freqplots_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void on_rdpattern_window_destroy(GObject *object, gpointer user_data);
gboolean on_rdpattern_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_rdpattern_save_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_save_as_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_save_as_gnuplot_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_linear_power_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_linear_voltage_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_arrl_style_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_logarithmic_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_noise_temp_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_noise_temp_log_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_elevation_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void Check_Noise_Warnings(int fstep);
void opengl_set_renderer(gboolean enable);
void opengl_set_constrained_rotation(gboolean constrained);

/* render_settings.c — Rendering Settings window */
void on_render_settings_show_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_render_settings_close_clicked(GtkButton *button, gpointer user_data);
gboolean on_render_settings_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_render_settings_window_destroy(GtkWidget *widget, gpointer user_data);
void on_general_reset_clicked(GtkButton *button, gpointer user_data);
void on_opengl_tab_reset_clicked(GtkButton *button, gpointer user_data);
void on_cairo_tab_reset_clicked(GtkButton *button, gpointer user_data);

void on_rdpattern_rotate_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_rdpattern_incline_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
gboolean on_rdpattern_colorcode_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_zoom_plus_clicked(GtkButton *button, gpointer user_data);
void on_zoom_minus_clicked(GtkButton *button, gpointer user_data);
void on_zoom_reset_clicked(GtkButton *button, gpointer user_data);
void freq_display_update(double fmhz);
void user_set_frequency(double fmhz);
gboolean on_rdpattern_drawingarea_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gboolean on_rdpattern_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_rdpattern_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void on_quit_cancelbutton_clicked(GtkButton *button, gpointer user_data);
void on_quit_okbutton_clicked(GtkButton *button, gpointer user_data);
gboolean on_rdpattern_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_rdpattern_animate_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_structure_animate_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_animate_phase_slider_change_value(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer user_data);
void on_animate_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
gboolean on_animate_spinbutton_focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
void on_animation_applybutton_clicked(GtkButton *button, gpointer user_data);
void on_animation_cancelbutton_clicked(GtkButton *button, gpointer user_data);
void on_animation_okbutton_clicked(GtkButton *button, gpointer user_data);
void on_animate_dialog_destroy(GObject *object, gpointer user_data);
void on_quit_dialog_destroy(GObject *object, gpointer user_data);
gboolean on_error_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_nec2_edit_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_nec2_editor_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_nec2_editor_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_nec2_editor_destroy(GObject *object, gpointer user_data);
void on_nec2_save_clicked(GtkButton *button, gpointer user_data);
void on_nec2_save_as_clicked(GtkButton *button, gpointer user_data);
void on_nec2_row_add_clicked(GtkButton *button, gpointer user_data);
void on_nec2_row_remv_clicked(GtkButton *button, gpointer user_data);
void on_nec2_treeview_clear_clicked(GtkButton *button, gpointer user_data);
gboolean on_nec2_cmnt_treeview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_nec2_geom_treeview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_nec2_geom_treeview_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_nec2_cmnd_treeview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_nec2_cmnd_treeview_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_nec2_revert_clicked(GtkButton *button, gpointer user_data);
void on_nec2_save_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_nec2_save_dialog_destroy(GtkDialog *dialog, gpointer user_data);
void on_error_stopbutton_clicked(GtkButton *button, gpointer user_data);
void on_error_okbutton_clicked(GtkButton *button, gpointer user_data);
void on_error_dialog_destroy( GObject *object, gpointer user_data);
void on_error_quitbutton_clicked(GtkButton *button, gpointer user_data);
void on_wire_editor_destroy(GObject *object, gpointer user_data);
void on_wire_pcl_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_data_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_tagnum_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_len_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_taper_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_wire_rlen_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_rdia_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_new_button_clicked(GtkButton *button, gpointer user_data);
void on_wire_res_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_wire_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_wire_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_wire_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_gw_clicked(GtkButton *button, gpointer user_data);
void on_ga_clicked(GtkButton *button, gpointer user_data);
void on_gh_clicked(GtkButton *button, gpointer user_data);
void on_sp_clicked(GtkButton *button, gpointer user_data);
void on_gr_clicked(GtkButton *button, gpointer user_data);
void on_gm_clicked(GtkButton *button, gpointer user_data);
void on_gx_clicked(GtkButton *button, gpointer user_data);
void on_gs_clicked(GtkButton *button, gpointer user_data);
void on_ex_clicked(GtkButton *button, gpointer user_data);
void on_fr_clicked(GtkButton *button, gpointer user_data);
void on_gn_clicked(GtkButton *button, gpointer user_data);
void on_gd_clicked(GtkButton *button, gpointer user_data);
void on_rp_clicked(GtkButton *button, gpointer user_data);
void on_ld_clicked(GtkButton *button, gpointer user_data);
void on_nt_clicked(GtkButton *button, gpointer user_data);
void on_tl_clicked(GtkButton *button, gpointer user_data);
void on_ne_clicked(GtkButton *button, gpointer user_data);
void on_ek_clicked(GtkButton *button, gpointer user_data);
void on_kh_clicked(GtkButton *button, gpointer user_data);
void on_zo_clicked(GtkButton *button, gpointer user_data);
void on_xq_clicked(GtkButton *button, gpointer user_data);
void on_patch_data_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_patch_new_button_clicked(GtkButton *button, gpointer user_data);
void on_patch_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_patch_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_patch_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_patch_editor_destroy(GObject *object, gpointer user_data);
void on_patch_arbitrary_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_patch_rectangular_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_patch_triangular_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_patch_quadrilateral_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_patch_surface_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_arc_data_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_arc_tagnum_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_arc_res_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_arc_new_button_clicked(GtkButton *button, gpointer user_data);
void on_arc_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_arc_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_arc_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_arc_editor_destroy(GObject *object, gpointer user_data);
void on_arc_pcl_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_tagnum_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_pcl_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_nturns_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_tspace_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_res_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_data_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_helix_new_button_clicked(GtkButton *button, gpointer user_data);
void on_helix_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_helix_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_helix_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_helix_editor_destroy(GObject *object, gpointer user_data);
void on_helix_right_hand_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_helix_left_hand_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
gboolean on_helix_link_a1b1_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_helix_link_b1a2_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_helix_link_a2b2_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_spiral_right_hand_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_spiral_left_hand_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_reflect_taginc_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_reflect_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_reflect_new_button_clicked(GtkButton *button, gpointer user_data);
void on_reflect_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_reflect_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_reflect_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_reflect_editor_destroy(GObject *object, gpointer user_data);
void on_scale_editor_destroy(GObject *object, gpointer user_data);
void on_scale_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_scale_new_button_clicked(GtkButton *button, gpointer user_data);
void on_scale_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_scale_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_scale_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_cylinder_taginc_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_cylinder_total_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_cylinder_new_button_clicked(GtkButton *button, gpointer user_data);
void on_cylinder_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_cylinder_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_cylinder_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_cylinder_editor_destroy(GObject *object, gpointer user_data);
void on_transform_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_transform_new_button_clicked(GtkButton *button, gpointer user_data);
void on_transform_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_transform_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_transform_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_transform_editor_destroy(GObject *object, gpointer user_data);
void on_gend_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_gend_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_gend_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_gend_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_gend_editor_destroy(GObject *object, gpointer user_data);
void on_kernel_command_destroy(GObject *object, gpointer user_data);
void on_kernel_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_kernel_new_button_clicked(GtkButton *button, gpointer user_data);
void on_kernel_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_kernel_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_kernel_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_execute_command_destroy(GObject *object, gpointer user_data);
void on_execute_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_execute_new_button_clicked(GtkButton *button, gpointer user_data);
void on_execute_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_execute_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_execute_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_intrange_command_destroy(GObject *object, gpointer user_data);
void on_intrange_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_intrange_new_button_clicked(GtkButton *button, gpointer user_data);
void on_intrange_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_intrange_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_intrange_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_zo_command_destroy(GObject *object, gpointer user_data);
void on_zo_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_zo_new_button_clicked(GtkButton *button, gpointer user_data);
void on_zo_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_zo_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_zo_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_ground_command_destroy(GObject *object, gpointer user_data);
void on_ground_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_ground_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_ground_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_ground_new_button_clicked(GtkButton *button, gpointer user_data);
void on_ground_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_ground_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_ground_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_nearfield_command_destroy(GObject *object, gpointer user_data);
void on_nearfield_nh_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_nearfield_ne_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_nearfield_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_nearfield_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_nearfield_new_button_clicked(GtkButton *button, gpointer user_data);
void on_nearfield_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_nearfield_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_nearfield_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_radiation_command_destroy(GObject *object, gpointer user_data);
void on_radiation_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiation_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_radiation_new_button_clicked(GtkButton *button, gpointer user_data);
void on_radiation_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_radiation_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_radiation_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_excitation_command_destroy(GObject *object, gpointer user_data);
void on_excitation_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_excitation_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_excitation_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_excitation_new_button_clicked(GtkButton *button, gpointer user_data);
void on_excitation_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_excitation_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_excitation_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_frequency_command_destroy(GObject *object, gpointer user_data);
void on_frequency_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_frequency_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_frequency_step_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_frequency_new_button_clicked(GtkButton *button, gpointer user_data);
void on_frequency_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_frequency_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_frequency_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_loading_command_destroy(GObject *object, gpointer user_data);
void on_loading_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_loading_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_loading_new_button_clicked(GtkButton *button, gpointer user_data);
void on_loading_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_loading_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_loading_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_network_command_destroy(GObject *object, gpointer user_data);
void on_network_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_network_new_button_clicked(GtkButton *button, gpointer user_data);
void on_network_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_network_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_network_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_txline_command_destroy(GObject *object, gpointer user_data);
void on_txline_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_txline_new_button_clicked(GtkButton *button, gpointer user_data);
void on_txline_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_txline_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_txline_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_txline_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_ground2_command_destroy(GObject *object, gpointer user_data);
void on_ground2_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_ground2_new_button_clicked(GtkButton *button, gpointer user_data);
void on_ground2_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_ground2_apply_button_clicked(GtkButton *button, gpointer user_data);
void on_ground2_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_loop_start_clicked(GtkButton *button, gpointer user_data);
void on_loop_pause_clicked(GtkButton *button, gpointer user_data);
void on_loop_reset_clicked(GtkButton *button, gpointer user_data);
void on_about_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_aboutdialog_close(GtkDialog *dialog, gpointer user_data);
void on_aboutdialog_destroy(GObject *object, gpointer user_data);
void on_aboutdialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_freqplots_theme_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_freqplots_theme_invert_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void freqplots_invert_item_sync(GtkWidget *invert, const char *base);
void freqplots_theme_menu_build(GtkBuilder *builder);
void color_family_menu_attach(GtkBuilder *builder);
void on_freqplots_theme_select(GtkMenuItem *menuitem, gpointer user_data);
void on_freqplots_theme_menu_hide(GtkWidget *menu, gpointer user_data);
gboolean on_structure_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_main_zoom_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
gboolean on_rdpattern_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_rdpattern_zoom_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
gboolean on_structure_drawingarea_scroll_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_freqplots_drawingarea_scroll_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_rdpattern_drawingarea_scroll_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_escape_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
/* cmnd_edit.c */
void Excitation_Command(int action);
void Frequency_Command(int action);
void Ground_Command(int action);
void Ground2_Command(int action);
void Radiation_Command(int action);
void Loading_Command(int action);
void Network_Command(int action);
void Txline_Command(int action);
void Nearfield_Command(int action);
void Kernel_Command(int action);
void Intrange_Command(int action);
void Execute_Command(int action);
void Zo_Command(int action);
/* cairo/cairo_project.c */
void Set_Gdk_Segment(Segment_t *segm, view_t *v, double scale, double x1, double y1, double z1, double x2, double y2, double z2, float *z_mid_out);
void Project_on_Screen(view_t *v, double x, double y, double z, double *xs, double *ys, float *zs);
/* Availability guards for per-frequency-step data */
#define CRNT_FSTEP_AVAILABLE(fs) \
    ((fs) >= 0 && (fs) <= calc_data.steps_total \
     && save.fstep != NULL && save.fstep[(fs)] \
     && crnt_fstep != NULL && crnt_fstep[(fs)].cur != NULL)

#define NF_FSTEP_AVAILABLE(fs) \
    ((fs) >= 0 && (fs) <= calc_data.steps_total \
     && save.fstep != NULL && save.fstep[(fs)] \
     && near_field_fstep != NULL && near_field_fstep[(fs)].points != NULL)

/* rdpattern_ui.c */
gboolean Validate_Nearfield_Animation(void);
double Scale_Gain_Resolved(double gain, int fstep, int idx,
    double t_sky, double t_earth);
double Polarization_Factor(int pol_type, int fstep, int idx);
void Set_Polarization(int pol);
void Set_Gain_Style(int gs);
void Queue_Radiation_Redraw(void);
void Update_Rdpattern_UI(void);
double Viewer_Gain(view_t *v, int fstep);
double Viewer_Noise_Value(view_t *v, int fstep);
void Rdpattern_Window_Killed(void);
void Set_Window_Labels(void);
void Alloc_Rdpattern_Buffers(int nfrq, int nth, int nph);
void Alloc_Nearfield_Buffers(int n1, int n2, int n3);
void Alloc_Nearfield_Fstep_Buffers(int nfrq);
void Free_Nearfield_Fstep_Buffers(void);
void Save_Nearfield_Data(int fstep);
void Free_Draw_Buffers(void);
double Scale_Gain( double gain, int fstep, int idx );
/* fields.c */
void efld(double xi, double yi, double zi, double ai, int ij);
void gf(double zk, double *co, double *si);
void gh(double zk, double *hr, double *hi);
void gwave(complex double *erv, complex double *ezv, complex double *erh, complex double *ezh, complex double *eph);
void gx(double zz, double rh, double xk, complex double *gz, complex double *gzp);
void hintg(double xi, double yi, double zi);
void hsfld(double xi, double yi, double zi, double ai);
void nfpat(int nfeh);
void pcint(double xi, double yi, double zi, double cabi, double sabi, double salpi, complex double *e);
void unere(double xob, double yob, double zob);
/* fork.c */
void Child_Process(int num_child);
ssize_t Write_Pipe(int idx, char *str, ssize_t len, gboolean err);
int Get_Freq_Data(int idx, int fstep);
/* geom_edit.c */
void Wire_Editor(int action);
void Patch_Editor(int action);
void Arc_Editor(int action);
void Helix_Editor(int action);
void Reflect_Editor(int action);
void Scale_Editor(int action);
void Cylinder_Editor(int action);
void Transform_Editor(int action);
void Gend_Editor(int action);
void Sy_Card_Editor(int action);
gboolean Check_Card_Name(GtkListStore *store, GtkTreeIter *iter, gboolean next, const gchar *name);
gboolean Give_Up(gboolean *busy, GtkWidget *widget);
void Remove_Row(GtkListStore *store, GtkTreeIter *iter);
gboolean Get_Selected_Row(GtkTreeView *view, GtkListStore *store, GtkTreeIter *iter, gchar *name);
/* geometry.c */
gboolean arc(int itg, int ns, double rada, double ang1, double ang2, double rad);
gboolean conect(int ignd);
void helix(double tsp, double hl, double a1, double b1, double a2, double b2, double rad, int ns, int itg);
int isegno(int itagi, int mx);
gboolean move(double rox, double roy, double roz, double xs, double ys, double zs, int its, int nrpt, int itgi);
gboolean patch(int nx, int ny, double ax1, double ay1, double az1, double ax2, double ay2, double az2, double ax3, double ay3, double az3, double ax4, double ay4, double az4);
gboolean reflc(int ix, int iy, int iz, int iti, int nop);
void wire(double xw1, double yw1, double zw1, double xw2, double yw2, double zw2, double rad, double rdel, double rrad, int ns, int itg);
/* gnuplot.c */
void Save_FreqPlots_S1P(char *filename);
void Save_FreqPlots_S2P_Max_Gain(char *filename);
void Save_FreqPlots_S2P_Viewer_Gain(char *filename);
void Save_FreqPlots_CSV(char *filename);
void Save_FreqPlots_Gnuplot_Data(char *filename);
void Save_RadPattern_Gnuplot_Data(char *filename);
void Save_RadPattern_CSV(char *filename);
void Save_Struct_Gnuplot_Data(char *filename);
void Save_Currents_CSV(char *filename);
void Save_Patch_Currents_CSV(char *filename);
/* ground.c */
void rom2(double a, double b, complex double *sum, double dmin);
void sflds(double t, complex double *e);
/* input.c */
gboolean Read_Comments(void);
gboolean Read_Geometry(void);
gboolean Read_Commands(void);
gboolean readmn(char *mn, int *i1, int *i2, int *i3, int *i4, double *f1, double *f2, double *f3, double *f4, double *f5, double *f6);
gboolean readgm(char *gm, int *i1, int *i2, double *x1, double *y1, double *z1, double *x2, double *y2, double *z2, double *rad);
/* interface.c */
GtkWidget *Builder_Get_Object(GtkBuilder *builder, const gchar *name);
GtkWidget *create_main_window(GtkBuilder **builder);
GtkWidget *create_filechooserdialog(GtkBuilder **builder);
GtkWidget *create_freqplots_window(GtkBuilder **builder);
GtkWidget *create_freqplots_popup_window(freqplots_view_t *view, const char *graph_name);
GtkWidget *freqplots_readout_bar_new(freqplots_view_t *view);
GtkWidget *create_rdpattern_window(GtkBuilder **builder);
GtkWidget *create_quit_dialog(GtkBuilder **builder);
GtkWidget *create_error_dialog(GtkBuilder **builder);
GtkWidget *create_animate_dialog(GtkBuilder **builder);
GtkWidget *create_nec2_editor(GtkBuilder **builder);
GtkWidget *create_wire_editor(GtkBuilder **builder);
GtkWidget *create_patch_editor(GtkBuilder **builder);
GtkWidget *create_arc_editor(GtkBuilder **builder);
GtkWidget *create_transform_editor(GtkBuilder **builder);
GtkWidget *create_helix_editor(GtkBuilder **builder);
GtkWidget *create_reflect_editor(GtkBuilder **builder);
GtkWidget *create_scale_editor(GtkBuilder **builder);
GtkWidget *create_cylinder_editor(GtkBuilder **builder);
GtkWidget *create_kernel_command(GtkBuilder **builder);
GtkWidget *create_execute_command(GtkBuilder **builder);
GtkWidget *create_intrange_command(GtkBuilder **builder);
GtkWidget *create_zo_command(GtkBuilder **builder);
GtkWidget *create_ground_command(GtkBuilder **builder);
GtkWidget *create_nearfield_command(GtkBuilder **builder);
GtkWidget *create_radiation_command(GtkBuilder **builder);
GtkWidget *create_excitation_command(GtkBuilder **builder);
GtkWidget *create_frequency_command(GtkBuilder **builder);
GtkWidget *create_loading_command(GtkBuilder **builder);
GtkWidget *create_network_command(GtkBuilder **builder);
GtkWidget *create_txline_command(GtkBuilder **builder);
GtkWidget *create_ground2_command(GtkBuilder **builder);
GtkWidget *create_gend_editor(GtkBuilder **builder);
GtkWidget *create_aboutdialog(GtkBuilder **builder);
GtkWidget *create_nec2_save_dialog(GtkBuilder **builder);
/* main.c */
int main(int argc, char *argv[]);
gboolean Open_Input_File(gpointer udata);
gboolean isChild(void);
/* matrix.c */
void cmset(int nrow, complex double *cmx, double rkhx, int iexkx);
void cmsw(int j1, int j2, int i1, int i2, complex double *cmx, complex double *cw, int ncw, int nrow, int itrp);
void etmns(double p1, double p2, double p3, double p4, double p5, double p6, int ipr, complex double *e);
int factr(int n, complex double *a, int *ip, int ndim);
int factr_gauss_elim( int n, complex double *a, int *ip, int ndim);
void factrs(int np, int nrow, complex double *a, int *ip);
void fblock(int nrow, int ncol, int imax, int ipsym);
int solve(int n, complex double *a, int *ip, complex double *b, int ndim);
int solve_gauss_elim( int n, complex double *a, int *ip, complex double *b, int ndim );
void solves(complex double *a, int *ip, complex double *b, int neq, int nrh, int np, int n, int mp, int m);
/* nec2_model.c */
void Zero_Store(GtkListStore *store, GtkTreeIter *iter, int ncols, int start_idx, int stop_idx);
void Nec2_Input_File_Treeview(int action);
void cell_edited_callback(GtkCellRendererText *cell, gchar *path, gchar *new_text, gpointer user_data);
void Save_Nec2_Input_File(GtkWidget *treeview_window, char *nec2_file);
/* network.c */
void netwk(complex double *cmx, int *ip, complex double *einc);
void load(int *ldtyp, int *ldtag, int *ldtagf, int *ldtagt, double *zlr, double *zli, double *zlc);
/* optimize.c */
void Write_Optimizer_Data(void);
void *Optimizer_Output(void *arg);
void optimizer_output_start(void);
void optimizer_output_stop(void);
int opt_have_files_to_save(void);
/* freqplots */
void Plot_Frequency_Data(freqplots_view_t *view, cairo_t *cr);
void Plots_Window_Killed(void);
void Set_Frequency_On_Click(freqplots_view_t *view, GdkEvent *event);
fp_panel_t freqplots_panel_at(freqplots_view_t *view, double px, double py);
freqplots_view_t *freqplots_main_view(void);
void freqplots_redraw_all(gboolean force);
int freqplots_count_selected(void);
void freqplots_open_panel(fp_panel_t panel);
void freqplots_close_panel(fp_panel_t panel);
void freqplots_destroy_all_popups(void);
gboolean freqplots_popup_open(fp_panel_t panel);
void on_freqplots_popup_destroy(GtkWidget *widget, gpointer user_data);
gboolean on_freqplots_popup_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
/* radiation.c */
void rdpat(void);
/* rc_config.c */
gboolean Create_Default_Config(void);
void Set_Window_Geometry(GtkWidget *window, gint x, gint y, gint width, gint height);
gboolean Read_Config(void);
void get_main_window_state(void);
void get_rdpattern_window_state(void);
void get_freqplots_window_state(void);
void get_nec2_edit_window_state(void);
void get_sy_overrides_window_state(void);
void Get_GUI_State(void);
gboolean Save_Config(void);
/* shared.c */
/* somnec.c */
void somnec(double epr, double sig, double fmhz);
void fbar(complex double p, complex double *fbar);
/* utils.c */
void usage(void);
int Stop(int err, const char *format, ...) __attribute__((format(printf, 2, 3)));
int Notice(GtkButtonsType buttons, const char *title, const char *msg_fmt, ...) __attribute__((format(printf, 3, 4)));
void Notice_Deferred(GtkButtonsType buttons, const char *title, const char *msg_fmt, ...) __attribute__((format(printf, 3, 4)));
gboolean Nec2_Save_Warn(const gchar *mesg);
int Load_Line(char *buff, FILE *pfile);
void xnec2c_set_omp_cpus(void);
void clock_print_elapsed(char *msg, clockid_t clk_id, struct timespec *start);
void clock_print_elapsed_when(char *msg, clockid_t clk_id, struct timespec *start, float min_sec);
gboolean Open_File(FILE **fp, char *fname, const char *mode);
void Close_File(FILE **fp);
void Display_Fstep(GtkEntry *entry, int fstep);
int isFlagSet(unsigned long long int flag);
int isFlagClear(unsigned long long int flag);
void SetFlag(unsigned long long int flag);
void ClearFlag(unsigned long long int flag);
void ToggleFlag(unsigned long long int flag);
void SaveFlag(unsigned long long int *flag, unsigned long long int mask);
void Strlcpy(char *dest, const char *src, size_t n);
void Strlcat(char *dest, const char *src, size_t n);
double Strtod(char *nptr, char **endptr);
char *str_append(char *dst, char *a, char *b, size_t n);
void Get_Dirname(char *fpath, char *dirname, int *fname_idx);
void xnec2_widget_queue_draw(GtkWidget *w, gboolean force);
#if GLIB_VERSION_CUR_STABLE < G_ENCODE_VERSION(2,74)
guint g_idle_add_once(GSourceOnceFunc function, gpointer data);
#endif
guint g_idle_add_once_sync(GSourceOnceFunc function, gpointer data);
void print_backtrace(char *msg);
void Window_Title_Subject(char *buf, size_t len);
void Update_Window_Titles(void);

/* xnec2c.c */
void freq_step_update_ui(int new_step, gboolean force);
void Near_Field_Pattern(void);
void Frequency_Scale_Geometry(void);
void New_Frequency(void);
void New_Frequency_Reset_Prev(void);

gboolean Frequency_Loop(gpointer udata);
void batch_finish_no_steps(void);
gboolean Start_Frequency_Loop(void);
gboolean Start_Frequency_Loop_Greenline(void);
void Stop_Frequency_Loop(void);
void Incident_Field_Loop(void);
int set_freq_step(void);
gboolean fetch_freq_data(void);

#endif
