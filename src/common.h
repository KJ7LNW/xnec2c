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

#include "measurements.h"

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

// Use __LOCATION__ anywhere you want the file and line number:
// (Move this to be used in all calls to mem_realloc)
#define __S1(x) #x
#define __S2(x) __S1(x)
#define  __LOCATION__ __FILE__ ":"  __S2(__LINE__)

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  define Q_(String) g_strip_context ((String), gettext (String))
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define Q_(String) g_strip_context ((String), (String))
#  define N_(String) (String)
#endif

typedef struct Segment
{
  gint x1;
  gint x2;
  gint y1;
  gint y2;
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
#define LINE_LEN    135

/* Max length of strings used for creating file names */
#define FILENAME_LEN    256

/* Size of char arrays (strings) for error messages etc */
#define MESG_SIZE   128

/* Type of projection parameters struct */
#define STRUCTURE_DRAWINGAREA   1
#define RDPATTERN_DRAWINGAREA   2

/* Some colors for settinn cairo context */
#define WHITE   1.0, 1.0, 1.0
#define RED     1.0, 0.0, 0.0
#define YELLOW  1.0, 1.0, 0.0
#define GREEN   0.0, 1.0, 0.0
#define CYAN    0.0, 1.0, 1.0
#define BLUE    0.0, 0.0, 1.0
#define MAGENTA 1.0, 0.0, 1.0
#define GREY    0.5, 0.5, 0.5
#define BLACK   0.0, 0.0, 0.0

/* For read/write pipes */
#define READ    0
#define WRITE   1

/* Response to error dialog box */
#define ERR_OK      FALSE
#define ERR_STOP    TRUE

/*** Flow control flags ***/
/* Freq Loop Control flags */
#define FREQ_LOOP_RUNNING   0x0000000000000001ll
#define FREQ_LOOP_DONE      0x0000000000000002ll
#define FREQ_LOOP_INIT      0x0000000000000004ll
#define FREQ_LOOP_STOP      0x0000000000000008ll
#define FREQ_LOOP_FLAGS     0x000000000000000Fll

/* Main Window Control flags */
#define DRAW_CURRENTS       0x0000000000000010ll
#define DRAW_CHARGES        0x0000000000000020ll
#define COMMON_PROJECTION   0x0000000000000040ll
#define COMMON_FREQUENCY    0x0000000000000080ll
#define MAIN_NEW_FREQ       0x0000000000000100ll
#define MAIN_QUIT           0x0000000000000200ll

/* Freq Plot Control flags */
#define PLOT_ZREAL_ZIMAG    0x0000000000000400ll
#define PLOT_VSWR           0x0000000000000800ll
#define PLOT_ZMAG_ZPHASE    0x0000000000001000ll
#define PLOT_GMAX           0x0000000000002000ll
#define PLOT_GVIEWER        0x0000000000004000ll
#define PLOT_NETGAIN        0x0000000000008000ll
#define PLOT_GAIN_DIR       0x0000000000010000ll
#define PLOT_FREQ_LINE      0x0000000000020000ll
#define PLOT_SELECT         0x0000000000040000ll
#define PLOT_ENABLED        0x0000000000080000ll
#define PLOT_QUIT           0x0000000000100000ll
#define PLOT_SMITH          0x0000000000200000ll
#define PLOT_FLAGS          0x00400000003FFC00ll

/* Radiation Pattern Control flags */
#define DRAW_GAIN           0x0000000000400000ll
#define DRAW_EFIELD         0x0000000000800000ll
#define DRAW_HFIELD         0x0000000001000000ll
#define DRAW_EHFIELD        0x0000000002000000ll
#define DRAW_POYNTING       0x0000000004000000ll
#define DRAW_ENABLED        0x0000000008000000ll
#define DRAW_QUIT           0x0000000010000000ll
#define DRAW_NEW_RDPAT      0x0000000020000000ll
#define DRAW_NEW_EHFIELD    0x0000000040000000ll
#define OVERLAY_STRUCT      0x0000000080000000ll
#define NEAREH_SNAPSHOT     0x0000000100000000ll
#define NEAREH_ANIMATE      0x0000000200000000ll
#define ENABLE_RDPAT        0x0000000400000000ll
#define ENABLE_NEAREH       0x0000000800000000ll
#define DRAW_FLAGS          0x00000003FFC00000ll
#define RDPAT_FLAGS         0x0000000C00000000ll

/*** Filechooser response related flags ***/
#define NEC2_EDIT_SAVE      0x0000001000000000ll /* Save NEC2 editor data to a file */
#define NEC2_SAVE           0x0000002000000000ll /* Saving to a NEC2 input file */
#define IMAGE_SAVE          0x0000004000000000ll /* Struct/rad pattern/freq plots image save */
#define RDPAT_GNUPLOT_SAVE  0x0000008000000000ll /* Save rad pattern as gnuplot data */
#define PLOTS_GNUPLOT_SAVE  0x0000010000000000ll /* Save feq plots as gnuplot data */
#define STRUCT_GNUPLOT_SAVE 0x0000020000000000ll /* Save structure as gnuplot data */
#define OPEN_INPUT          0x0000040000000000ll /* Open a NEC2 input file flag */
#define OPEN_NEW_NEC2       0x0000080000000000ll /* Open a new NEC2 project flag */
#define ALL_CHOOSER_FLAGS   0x00000FF000000000ll

/* Other flags */
#define ERROR_CONDX         0x0000100000000000ll
#define INPUT_PENDING       0x0000200000000000ll
#define ENABLE_EXCITN       0x0000400000000000ll

/* Allocation control flags */
#define ALLOC_NEAREH_BUFF   0x0000800000000000ll
#define ALLOC_RDPAT_BUFF    0x0001000000000000ll
#define ALLOC_PNTING_BUFF   0x0002000000000000ll

/* Force editors to quit */
#define EDITOR_QUIT         0x0004000000000000ll

/* Frequency loop results ready */
#define FREQ_LOOP_READY     0x0008000000000000ll

/* Block motion events till ready */
#define BLOCK_MOTION_EV     0x0010000000000000ll

/* Indicate that an input file is open */
#define INPUT_OPENED        0x0020000000000000ll

/* Signal start of xnec2c */
#define XNEC2C_START        0x0040000000000000ll

// Enable Optimizer Output
#define OPTIMIZER_OUTPUT    0x0080000000000000ll

#define ALL_FLAGS           0xFFFFFFFFFFFFFFFFll

/* Type of near field data requested */
#define NEAR_EFIELD     0x01
#define NEAR_HFIELD     0x02
#define NEAR_EHFIELD    0x03

#define CHILD   isChild()

/* Runtime configuration data */
typedef struct
{
  /* Current working directory */
  char working_dir[FILENAME_LEN];

  /* Current NEC2 input file */
  char input_file[FILENAME_LEN];

  /* If set true, then use GTK loops for the frequency loop iteration
   * instead of spawning a pthread */
  int disable_pthread_freqloop;

  /* Main (structure) window position and size */
  int
    main_x,
    main_y,
    main_width,
    main_height;

  /* Main (structure) window state of widgets */
  int
    main_currents_togglebutton,
    main_charges_togglebutton,
    main_total,
    main_horizontal,
    main_vertical,
    main_right_hand,
    main_left_hand,
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

  /* Radiation pattern window state of widgets */
  int
    rdpattern_gain_togglebutton,
    rdpattern_eh_togglebutton,
    rdpattern_e_field,
    rdpattern_h_field,
    rdpattern_poynting_vector;

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
    freqplots_min_max,
    freqplots_s11,
    freqplots_clamp_vswr,
    freqplots_net_gain;

  /* NEC2 editor window position and size */
  int
    nec2_edit_x,
    nec2_edit_y,
    nec2_edit_width,
    nec2_edit_height;

  /* Enable Quit dialog */
  int confirm_quit;

  /* Preferred mathlib index, if available */
  int mathlib_idx, mathlib_batch_idx;

  /* See enum GAIN_SCALE */
  int gain_style;

  /* If true, then call Fit_to_Scale() on the X-axis
   * fr_plot->min_fscale/fr_plot->max_fscale values. */
  int freqplots_round_x_axis;

  /* if true, exit after the first frequency loop iteration */
  int batch_mode;

  /* verbose and debug levels, see console.h */
  int verbose, debug;

  /* true if ~/.xnec2c/xnec2c.conf does not exist, false otherwise */
  int first_run;

  /* optimization file write flags */
  int opt_write_csv;
  int opt_write_s1p;
  int opt_write_s2p_max_gain;
  int opt_write_s2p_viewer_gain;
  int opt_write_rdpat;
  int opt_write_currents;

  /* If for example --write-csv <file> is added to the cmdline then write
   * the file after each frequency iteration */
  char *filename_csv;
  char *filename_s1p;
  char *filename_s2p_max_gain;
  char *filename_s2p_viewer_gain;
  char *filename_rdpat;
  char *filename_currents;
} rc_config_t;

typedef struct {
	void (*callback)(char *);
	char *extension;
} filechooser_t;

/* Gain Scaling style */
enum GAIN_SCALE
{
  GS_LINP = 0,
  GS_LINV,
  GS_ARRL,
  GS_LOG,
  NUM_SCALES
};

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

  char newer; /* New data available */
  char valid; /* Available data valid */

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
    ipsym,  /* Symmetry flag */
    *icon1, /* Segments end 1 connection */
    *icon2, /* Segments end 2 connection */
    *itag;  /* Segments tag number */

  /* Wire segment data */
  double
    *x1, *y1, *z1,  /* End 1 coordinates of wire segments */
    *x2, *y2, *z2,  /* End 2 coordinates of wire segments */
    *x, *y, *z,     /* Coordinates of segment centers */
    *si, *bi,       /* Length and radius of segments  */
    *cab,           /* cos(a)*cos(b) */
    *sab,           /* cos(a)*sin(b) */
    *salp,          /* Z component - sin(a) */

    /* Surface patch data */
    *t1x, *t1y, *t1z,   /* Coordinates of t1 vector */
    *t2x, *t2y, *t2z,   /* Coordinates of t2 vector */
    *px, *py, *pz,      /* Coordinates of patch center */
    *pbi,               /* Patch surface area */
    *psalp,             /* Z component - sin(a) */

    /* Wavelength in meters */
    wlam;

  /* My addition, for plotting patches */
  double
    *px1, *py1, *pz1,
    *px2, *py2, *pz2;

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

/* Parameters for projecting points on screen */
typedef struct
{
  double
    Wi,         /* Angle (inclination) of Z axis to Screen, +ve to viewer */
    Wr,         /* Rotation of X-Y plane around Z axis, +ve clockwise */
    sin_wi,     /* sin(Wi) */
    cos_wi,     /* cos(Wi) */
    sin_wr,     /* sin(Wr) */
    cos_wr,     /* cos(Wr) */
    r_max,      /* Max distance from xyz origin of a point in antenna */
    xy_scale1,  /* Scale factor to fit structure into drawable window */
    xy_scale,   /* Scale factor incorporating zoom factor */
    xy_zoom,    /* Structure Zoom factor */
    x_center,   /* X co-ordinate of xyz axes origin in screen's drawable */
    dx_center,  /* Displacement of center x (move projection) */
    y_center,   /* X co-ordinate of xyz axes origin in screen's drawable */
    dy_center;  /* Displacement of center y (move projection) */

  int
    width,   /*  Width of drawable */
    height;  /* Height of drawable */

  char type; /* Type of projection parameters struct */

  gboolean reset;  /* Reset flag needed in some functions */

} projection_parameters_t;

/* My addition, a struct for frequency loop related data.
 * This is needed to implement multiple FR cards so
 * that a frequency sweep can be made in separate bands
 */
typedef struct
{
  int
    ifreq,       /* Frequency specification flag */
    freq_steps;  /* Number of frequency steps */

  double
    delta_freq,  /* Frequency step in freq loop */
    min_freq,    /* Min frequency in an FR card */
    max_freq;    /* Max frequency in an FR card */

} freq_loop_data_t;

/* 
 * fr_plot_t structure and related defines used in plot_freqdata.c
 */
#define FR_PLOT_T_MAGIC 				0xc2bca3083893e65eULL
#define FR_PLOT_T_IS_VALID(fr_plot_ptr)	((fr_plot_ptr)->valid == FR_PLOT_T_MAGIC)

typedef struct {
	GdkRectangle plot_rect;
	int posn;			// Position in the frequency plots
	int fr;				// index into calc_data.freq_loop_data[fr]

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
    FR_index,     /* Index to FR card data in use */
    freq_step,    /* Frequency step in frequency loop */
    steps_total,  /* Total number of frequency steps */
    last_step;    /* Last frequency step */

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

  int
    *max_gain_idx,  /* Where in rad_pattern.gtot the max value occurs */
    *min_gain_idx,  /* Where in rad_pattern.gtot the min value occurs */
    *sens;          /* Polarization sense (vertical, horizontal, elliptic etc) */

} rad_pattern_t;

/* Near E/H field data */
typedef struct
{
  double
    /* Magnitude and phase of E/H fields */
    *ex, *ey, *ez, *hx, *hy, *hz,
    *fex, *fey, *fez, *fhx, *fhy, *fhz,
    /* Real part of E and H field values */
    *erx, *ery, *erz, *er,
    *hrx, *hry, *hrz, *hr;

  /* Max of E/H field values */
  double
    max_er, max_hr;

  /* Co-ordinates of field points and
   * max distance from xyz origin */
  double
    *px, *py, *pz, r_max;

  /* Animation step in rads */
  double anim_step;

  char newer; /* New data available */
  char valid; /* Available data valid */

} near_field_t;

/* Forked processes data */
typedef struct
{
  pid_t child_pid;          /* Child pid */
  int pnt2child_pipe[2];    /* Parent-to-child write pipe */
  int child2pnt_pipe[2];    /* Child-to-parent write pipe */
  char busy;                /* Child process busy flag */
  int fstep;                /* Frequency step assigned to child */

  /* File descriptor sets for select() */
  fd_set read_fds;
  fd_set write_fds;

} forked_proc_data_t;

enum
{
  MAIN_WINDOW = 1,
  FREQPLOTS_WINDOW,
  RDPATTERN_WINDOW
};

/* Function prototypes produced by cproto */
/* calculations.c */
void qdsrc(int is, _Complex double v, _Complex double *e);
void cabc(_Complex double *curx);
double db10(double x);
double db20(double x);
void intrp(double x, double y, _Complex double *f1, _Complex double *f2, _Complex double *f3, _Complex double *f4);
void intx(double el1, double el2, double b, int ij, double *sgr, double *sgi);
int min(int a, int b);
void test(double f1r, double f2r, double *tr, double f1i, double f2i, double *ti, double dmin);
void trio(int j);
double cang(_Complex double z);
void zint(double sigl, double rolam, _Complex double *zint);
/* callback_func.c */
gboolean Save_Pixbuf(gpointer save_data);
void New_Viewer_Angle(double wr, double wi, GtkSpinButton *wr_spb, GtkSpinButton *wi_spb, projection_parameters_t *params);
void Motion_Event(GdkEventMotion *event, projection_parameters_t *params);
void Plot_Select(GtkToggleButton *togglebutton, unsigned long long int flag);
gboolean Nec2_Edit_Save(void);
void Delete_Event(gchar *mesg);
void Set_Pol_Menuitem(int window);
gboolean Open_Editor(GtkTreeView *view);
void
Card_Clicked(GtkWidget **editor, GtkBuilder **editor_builder, GtkWidget *create_fun(GtkBuilder **), void editor_fun(int), int *editor_action);
void Main_Rdpattern_Activate(gboolean from_menu);
gboolean Main_Freqplots_Activate(void);
void Rdpattern_Gain_Togglebutton_Toggled(gboolean flag);
void Rdpattern_EH_Togglebutton_Toggled(gboolean flag);
void Main_Currents_Togglebutton_Toggled(gboolean flag);
void Main_Charges_Togglebutton_Toggled(gboolean flag);
GtkWidget *Open_Filechooser(GtkFileChooserAction action, char *pattern, char *prefix, char *filename, char *foldername);
void Filechooser_Response(GtkDialog *dialog, gint response_id, int saveas_width, int saveas_height);
void Open_Nec2_Editor(int action);
gboolean Nec2_Apply_Checkbutton(void);
void Gtk_Quit(void);
void Pass_EH_Flags(void);
void Draw_Colorcode(cairo_t *cr);
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
void on_confirm_quit_toggled(GtkMenuItem *menuitem, gpointer user_data);
void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_rdpattern_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_freqplots_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_total_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_horizontal_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_vertical_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_right_hand_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_left_hand_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_common_projection_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_common_freq_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_main_x_axis_clicked(GtkButton *button, gpointer user_data);
void on_main_y_axis_clicked(GtkButton *button, gpointer user_data);
void on_main_z_axis_clicked(GtkButton *button, gpointer user_data);
void on_main_default_view_clicked(GtkButton *button, gpointer user_data);
void on_main_rotate_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_main_incline_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_main_currents_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_main_charges_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
gboolean on_main_colorcode_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_main_freq_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_main_freq_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_main_new_freq_clicked(GtkButton *button, gpointer user_data);
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
void on_freqplots_gmax_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_gdir_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_gviewer_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_vswr_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_zo_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_freqplots_zrlzim_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_zmgzph_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_freqplots_smith_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
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
void on_rdpattern_e_field_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_h_field_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_poynting_vector_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_overlay_structure_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_x_axis_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_y_axis_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_z_axis_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_default_view_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_rotate_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_rdpattern_incline_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_rdpattern_gain_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_rdpattern_eh_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
gboolean on_rdpattern_colorcode_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_rdpattern_freq_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_rdpattern_new_freq_clicked(GtkButton *button, gpointer user_data);
gboolean on_rdpattern_drawingarea_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gboolean on_rdpattern_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_rdpattern_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void on_quit_cancelbutton_clicked(GtkButton *button, gpointer user_data);
void on_quit_okbutton_clicked(GtkButton *button, gpointer user_data);
void main_view_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void main_pol_menu_activate(GtkMenuItem *menuitem, gpointer user_data);
void freqplots_pol_menu_activate(GtkMenuItem *menuitem, gpointer user_data);
void rdpattern_pol_menu_activate(GtkMenuItem *menuitem, gpointer user_data);
void rdpattern_view_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_rdpattern_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_near_peak_value_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_near_snapshot_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rdpattern_animate_activate(GtkMenuItem *menuitem, gpointer user_data);
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
void on_freqplots_net_gain_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_structure_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_main_zoom_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_structure_plus_button_clicked(GtkButton *button, gpointer user_data);
void on_structure_minus_button_clicked(GtkButton *button, gpointer user_data);
void on_structure_one_button_clicked(GtkButton *button, gpointer user_data);
gboolean on_rdpattern_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_rdpattern_zoom_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_rdpattern_plus_button_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_minus_button_clicked(GtkButton *button, gpointer user_data);
void on_rdpattern_one_button_clicked(GtkButton *button, gpointer user_data);
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
/* draw.c */
void Set_Gdk_Segment(Segment_t *segm, projection_parameters_t *params, double x1, double y1, double z1, double x2, double y2, double z2);
void Draw_XYZ_Axes(cairo_t *cr, projection_parameters_t params);
void New_Projection_Parameters(int width, int height, projection_parameters_t *params);
void Value_to_Color(double *red, double *grn, double *blu, double val, double max);
void Cairo_Draw_Polygon(cairo_t *cr, GdkPoint *points, int npoints);
void Cairo_Draw_Segments(cairo_t *cr, Segment_t *segm, int nseg);
void Cairo_Draw_Line(cairo_t *cr, int x1, int y1, int x2, int y2);
void Cairo_Draw_Lines(cairo_t *cr, GdkPoint *points, int npoints);
/* draw_radiation.c */
int Draw_Radiation(cairo_t *cr);
gboolean Animate_Near_Field(gpointer udata);
double Polarization_Factor(int pol_type, int fstep, int idx);
void Set_Polarization(int pol);
void Set_Gain_Style(int gs);
void New_Radiation_Projection_Angle(void);
gboolean Redo_Radiation_Pattern(gpointer udata);
double Viewer_Gain(projection_parameters_t proj_parameters, int fstep);
void Rdpattern_Window_Killed(void);
void Set_Window_Labels(void);
void Alloc_Rdpattern_Buffers(int nfrq, int nth, int nph);
void Alloc_Nearfield_Buffers(int n1, int n2, int n3);
void Free_Draw_Buffers(void);
double Scale_Gain( double gain, int fstep, int idx );
/* draw_structure.c */
void Draw_Structure(cairo_t *cr);
void New_Patch_Data(void);
void Process_Wire_Segments(void);
void Process_Surface_Patches(void);
void Draw_Wire_Segments(cairo_t *cr, Segment_t *segm, gint nseg);
void Draw_Surface_Patches(cairo_t *cr, Segment_t *segm, gint npatch);
gboolean Redo_Currents(gpointer udata);
void New_Structure_Projection_Angle(void);
void Init_Struct_Drawing(void);
void Show_Viewer_Gain(GtkBuilder *builder, gchar *widget, projection_parameters_t proj_params);
/* fields.c */
void efld(double xi, double yi, double zi, double ai, int ij);
void gf(double zk, double *co, double *si);
void gh(double zk, double *hr, double *hi);
void gwave(_Complex double *erv, _Complex double *ezv, _Complex double *erh, _Complex double *ezh, _Complex double *eph);
void gx(double zz, double rh, double xk, _Complex double *gz, _Complex double *gzp);
void hintg(double xi, double yi, double zi);
void hsfld(double xi, double yi, double zi, double ai);
void nfpat(int nfeh);
void pcint(double xi, double yi, double zi, double cabi, double sabi, double salpi, _Complex double *e);
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
/* ground.c */
void rom2(double a, double b, _Complex double *sum, double dmin);
void sflds(double t, _Complex double *e);
/* input.c */
gboolean Read_Comments(void);
gboolean Read_Geometry(void);
gboolean Read_Commands(void);
gboolean readmn(char *mn, int *i1, int *i2, int *i3, int *i4, double *f1, double *f2, double *f3, double *f4, double *f5, double *f6);
gboolean readgm(char *gm, int *i1, int *i2, double *x1, double *y1, double *z1, double *x2, double *y2, double *z2, double *rad);
/* interface.c */
GtkWidget *Builder_Get_Object(GtkBuilder *builder, gchar *name);
GtkWidget *create_main_window(GtkBuilder **builder);
GtkWidget *create_filechooserdialog(GtkBuilder **builder);
GtkWidget *create_freqplots_window(GtkBuilder **builder);
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
void cmset(int nrow, _Complex double *cmx, double rkhx, int iexkx);
void cmsw(int j1, int j2, int i1, int i2, _Complex double *cmx, _Complex double *cw, int ncw, int nrow, int itrp);
void etmns(double p1, double p2, double p3, double p4, double p5, double p6, int ipr, _Complex double *e);
int factr(int n, _Complex double *a, int *ip, int ndim);
int factr_gauss_elim( int n, complex double *a, int *ip, int ndim);
void factrs(int np, int nrow, _Complex double *a, int *ip);
void fblock(int nrow, int ncol, int imax, int ipsym);
int solve(int n, _Complex double *a, int *ip, _Complex double *b, int ndim);
int solve_gauss_elim( int n, complex double *a, int *ip, complex double *b, int ndim );
void solves(_Complex double *a, int *ip, _Complex double *b, int neq, int nrh, int np, int n, int mp, int m);
/* nec2_model.c */
void Zero_Store(GtkListStore *store, GtkTreeIter *iter, int ncols, int start_idx, int stop_idx);
void Nec2_Input_File_Treeview(int action);
void cell_edited_callback(GtkCellRendererText *cell, gchar *path, gchar *new_text, gpointer user_data);
void Save_Nec2_Input_File(GtkWidget *treeview_window, char *nec2_file);
/* network.c */
void netwk(_Complex double *cmx, int *ip, _Complex double *einc);
void load(int *ldtyp, int *ldtag, int *ldtagf, int *ldtagt, double *zlr, double *zli, double *zlc);
/* optimize.c */
void Write_Optimizer_Data(void);
void *Optimizer_Output(void *arg);
int opt_have_files_to_save(void);
/* plot_freqdata.c */
void Plot_Frequency_Data(cairo_t *cr);
void Plots_Window_Killed(void);
void Set_Frequency_On_Click(GdkEvent *event);
int freqplots_click_pending(void);
/* radiation.c */
void rdpat(void);
/* rc_config.c */
gboolean Create_Default_Config(void);
void Set_Window_Geometry(GtkWidget *window, gint x, gint y, gint width, gint height);
gboolean Read_Config(void);
void Get_GUI_State(void);
gboolean Save_Config(void);
/* shared.c */
/* somnec.c */
void somnec(double epr, double sig, double fmhz);
void fbar(_Complex double p, _Complex double *fbar);
/* utils.c */
void usage(void);
int Stop(char *mesg, int err);
int Notice(char *title, char *message,  GtkButtonsType buttons);
gboolean Nec2_Save_Warn(const gchar *mesg);
int Load_Line(char *buff, FILE *pfile);
void mem_alloc(void **ptr, size_t req, gchar *str);
void mem_realloc(void **ptr, size_t req, gchar *str);
void mem_backtrace(void *ptr);
void mem_obj_dump(void *ptr);
void free_ptr(void **ptr);
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
void xnec2_widget_queue_draw(GtkWidget *w);
#if GLIB_VERSION_CUR_STABLE < G_ENCODE_VERSION(2,74)
guint g_idle_add_once(GSourceOnceFunc function, gpointer data);
#endif
guint g_idle_add_once_sync(GSourceOnceFunc function, gpointer data);
void print_backtrace(char *msg);

/* xnec2c.c */
void Near_Field_Pattern(void);
void New_Frequency(void);
void New_Frequency_Reset_Prev(void);

gboolean Frequency_Loop(gpointer udata);
gboolean Start_Frequency_Loop(void);
void Stop_Frequency_Loop(void);
void Incident_Field_Loop(void);
int set_freq_step(void);

#endif
