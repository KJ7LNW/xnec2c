/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
 *
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

/* xnec2c.h
 *
 * Common header file for xnec2c
 */

#ifndef	NEC2C_H
#define	NEC2C_H 1

#include <complex.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <gtk/gtk.h>

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

/* commonly used complex constants */
#define	CPLX_00	(0.0l+0.0lj)
#define	CPLX_01	(0.0l+1.0lj)
#define	CPLX_10	(1.0l+0.0lj)
#define	CPLX_11	(1.0l+1.0lj)

/* common constants */
#define PI		3.141592654l
#define	POT		1.570796327l
#define	TP		6.283185308l
#define	PTP		.6283185308l
#define	TPJ		(0.0l+6.283185308lj)
#define PI8		25.13274123l
#define PI10	31.41592654l
#define	TA		1.745329252E-02l
#define	TD		57.29577951l
#define	ETA		376.73l
#define	CVEL	299.8l
#define	RETA	2.654420938E-3l
#define	TOSP	1.128379167l
#define ACCS	1.0E-12l
#define	SP		1.772453851l
#define	FPI		12.56637062l
#define	CCJ		(0.0l-0.01666666667lj)
#define	CONST1	(0.0l+4.771341189lj)
#define	CONST2	4.771341188l
#define	CONST3	(0.0l-29.97922085lj)
#define	CONST4	(0.0l+188.365lj)
#define	GAMMA	.5772156649l
#define C1		-.02457850915l
#define C2		.3674669052l
#define C3		.7978845608l
#define P10		.0703125l
#define P20		.1121520996l
#define Q10		.125l
#define Q20		.0732421875l
#define P11		.1171875l
#define P21		.1441955566l
#define Q11		.375l
#define Q21		.1025390625l
#define POF		.7853981635l
#define MAXH	20
#define CRIT	1.0E-4l
#define NM		131072
#define NTS		4
#define	SMIN	1.0e-3l

#define cmplx(r, i) ((r)+(i)*CPLX_01)

/* Replaces the "10000" limit used to */
/* identify segment/patch connections */
#define	PCHCON  100000

/* Carriage return and line feed */
#define	CR	0x0d
#define	LF	0x0a

/* Max length of a line read from input file */
#define	LINE_LEN	135

/* Projection angle increment (degrees) */
#define	PROJ_ANGLE_INCR	1.0

/* Some colors for Set_GC_Attribute() */
#define	WHITE	255, 255, 255
#define	RED		255,   0,   0
#define	YELLOW	255, 255,   0
#define	GREEN	  0, 255,   0
#define	CYAN	  0, 255, 255
#define	BLUE	  0,   0, 255
#define	MAGENTA	255,   0, 255
#define	GREY	127, 127, 127

/* For plotting graphs */
#define	LEFT	1
#define	RIGHT	2

/* For read/write pipes */
#define	READ	0
#define	WRITE	1

/*** Flow control flags ***/
/* Freq Loop Control flags */
#define	FREQ_LOOP_RUNNING	0x0000000000000001ll
#define	FREQ_LOOP_DONE		0x0000000000000002ll
#define	FREQ_LOOP_INIT		0x0000000000000004ll
#define	FREQ_LOOP_STOP		0x0000000000000008ll

/* Main Window Control flags */
#define	DRAW_CURRENTS		0x0000000000000010ll
#define	DRAW_CHARGES		0x0000000000000020ll
#define	COMMON_PROJECTION	0x0000000000000040ll
#define	COMMON_FREQUENCY	0x0000000000000080ll
#define	MAIN_NEW_FREQ		0x0000000000000100ll
#define	MAIN_QUIT			0x0000000000000200ll

/* Freq Plot Control flags */
#define	PLOT_ZREAL_ZIMAG	0x0000000000000400ll
#define	PLOT_VSWR			0x0000000000000800ll
#define	PLOT_ZMAG_ZPHASE	0x0000000000001000ll
#define	PLOT_GMAX			0x0000000000002000ll
#define	PLOT_GVIEWER		0x0000000000004000ll
#define	PLOT_NETGAIN		0x0000000000008000ll
#define	PLOT_GAIN_DIR		0x0000000000010000ll
#define	PLOT_FREQ_LINE		0x0000000000020000ll
#define	PLOT_SELECT			0x0000000000040000ll
#define	PLOT_ENABLED		0x0000000000080000ll
#define	PLOT_QUIT			0x0000000000100000ll
#define	PLOT_FLAGS			0x00000000001FFC00ll

/* Radiation Pattern Control flags */
#define	DRAW_GAIN			0x0000000000200000ll
#define	DRAW_EFIELD			0x0000000000400000ll
#define	DRAW_HFIELD			0x0000000000800000ll
#define	DRAW_EHFIELD		0x0000000001000000ll
#define	DRAW_POYNTING		0x0000000002000000ll
#define	DRAW_ENABLED		0x0000000004000000ll
#define	DRAW_QUIT			0x0000000008000000ll
#define	DRAW_NEWRDPAT		0x0000000010000000ll
#define	OVERLAY_STRUCT		0x0000000020000000ll
#define	NEAREH_SNAPSHOT		0x0000000040000000ll
#define	NEAREH_ANIMATE		0x0000000080000000ll
#define	ENABLE_RDPAT		0x0000000100000000ll
#define	ENABLE_NEAREH		0x0000000200000000ll
#define	DRAW_FLAGS			0x00000000FFE00000ll

/* NEC2 editor flags */
#define	NEC2_EDIT_SAVE		0x0000000400000000ll /* Save NEC2 editor data to a file */
#define	NEC2_SAVE			0x0000000800000000ll /* Saving to a NEC2 input file */
/* Structure/rad pattern/freq. plots image save */
#define	IMAGE_SAVE			0x0000001000000000ll

/* Other flags */
#define	ERROR_CONDX			0x0000002000000000ll
#define	INPUT_PENDING		0x0000004000000000ll
#define	ENABLE_EXCITN		0x0000008000000000ll

/* Allocation control flags */
#define	ALLOC_NEAREH_BUFF	0x0000010000000000ll
#define	ALLOC_RDPAT_BUFF	0x0000020000000000ll
#define	ALLOC_PNTING_BUFF	0x0000040000000000ll

#define	ALL_FLAGS		    ~0ll

/* Type of near field data requested */
#define NEAR_EFIELD	0x01
#define NEAR_HFIELD	0x02

#define CHILD	isChild()

/* Gain Scaling style */
enum
{
  GS_LINP,
  GS_LINV,
  GS_ARRL,
  GS_LOG,
  NUM_SCALES
};

/* Polarization type */
enum
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
  long double
    *air,	/* Ai/lambda, real part */
    *aii,	/* Ai/lambda, imaginary part */
    *bir,	/* Bi/lambda, real part */
    *bii,	/* Bi/lambda, imaginary part */
    *cir,	/* Ci/lambda, real part */
    *cii;	/* Ci/lambda, imaginary part */

  complex long double *cur; /* Amplitude of basis function */

  char newer; /* New data available */
  char valid; /* Available data valid */

} crnt_t;

/* common  /data/ (geometry data) */
typedef struct
{
  int
    n,		/* Number of wire segments */
    np,		/* Number of wire segments in symmetry cell */
    m,		/* Number of surface patches */
    mp,		/* Number of surface patches in symmetry cell */
    npm,	/* = n+m  */
    np2m,	/* = n+2m */
    np3m,	/* = n+3m */
    ipsym,	/* Symmetry flag */
    *icon1, /* Segments end 1 connection */
    *icon2,	/* Segments end 2 connection */
    *itag;	/* Segments tag number */

  /* Wire segment data */
  long double
    *x1, *y1, *z1,	/* End 1 coordinates of wire segments */
    *x2, *y2, *z2,	/* End 2 coordinates of wire segments */
    *x, *y, *z,		/* Coordinates of segment centers */
    *si, *bi,		/* Length and radius of segments  */
    *cab,			/* cos(a)*cos(b) */
    *sab, 	   		/* cos(a)*sin(b) */
    *salp,	   		/* Z component - sin(a) */

    /* Surface patch data */
    *t1x, *t1y, *t1z,	/* Coordinates of t1 vector */
    *t2x, *t2y, *t2z,	/* Coordinates of t2 vector */
    *px, *py, *pz,		/* Coordinates of patch center */
    *pbi,				/* Patch surface area */
    *psalp,				/* Z component - sin(a) */

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

long double
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

complex long double
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
  long double
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

  long double
    *dxa,
    *dya,
    *xsa,
    *ysa;

  complex long double
    epscf,
    *ar1,
    *ar2,
    *ar3;

} ggrid_t;

/* common  /gnd/ */
typedef struct
{
  int
    ksymp,	/* Ground flag */
    ifar,	/* Int flag in RP card, for far field calculations */
    iperf,	/* Type of ground flag */
    nradl;	/* Number of radials in ground screen */

  long double
    t2,		/* Const for radial wire ground impedance */
    cl,		/* Distance in wavelengths of cliff edge from origin */
    ch,		/* Cliff height in wavelengths */
    scrwl,	/* Wire length in radial ground screen normalized to w/length */
    scrwr;	/* Radius of wires in screen in wavelengths */

  complex long double
    zrati,	/* Ground medium [Er-js/wE0]^-1/2 */
    zrati2,	/* As above for 2nd ground medium */
    t1,		/* Const for radial wire ground impedance */
    frati;	/* (k1^2-k2^2)/(k1^2+k2^2), k1=w(E0Mu0)^1/2, k1=k2/ZRATI */

} gnd_t;

/* common  /gwav/ */
typedef struct
{
  long double
	r1,	/* Distance from current element to point where field is evaluated  */
	r2,	/* Distance from image of element to point where field is evaluated */
	zmh, /* Z-Z', Z is height of field evaluation point */
	zph; /* Z+Z', Z' is height of current element */

  complex long double
	u,	/* (Er-jS/WE0)^-1/2 */
	u2,	/* u^2 */
	xx1, /* G1*exp(jkR1*r[i])  */
	xx2; /* G2*exp(jkR2*r'[i]) */

} gwav_t;

/* common  /incom/ */
typedef struct
{
  int isnor;

  long double
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
    icase,	/* Storage mode of primary matrix */
    npblk,	/* Num of blocks in first (NBLOKS-1) blocks */
    nlast,	/* Num of blocks in last block */
    imat;	/* Storage reserved in CM for primary NGF matrix A */

} matpar_t;

/* common  /netcx/ */
typedef struct
{
  int
    masym,	/* Matrix symmetry flags */
    neq,
    npeq,
    neq2,
    nonet,	/* Number of two-port networks */
    ntsol,	/* "Network equations are solved" flag */
    nprint,	/* Print control flag */
    *iseg1,	/* Num of seg to which port 1 of network is connected */
    *iseg2,	/* Num of seg to which port 2 of network is connected */
    *ntyp;	/* Type of networks */

  long double
    *x11r,	/* Real and imaginary parts of network impedances */
    *x11i,
    *x12r,
    *x12i,
    *x22r,
    *x22i,
    pin,	/* Total input power from sources */
    pnls;	/* Power lost in networks */

  complex long double zped;

} netcx_t;

/* common  /save/ */
typedef struct
{
  int *ip;	/* Vector of indices of pivot elements used to factor matrix */

  long double
    epsr,		/* Relative dielectric constant of ground */
    sig,		/* Conductivity of ground */
    scrwlt,		/* Length of radials in ground screen approximation */
    scrwrt,		/* Radius of wires in ground screen approximation */
    fmhz,		/* Saved frequency of operation */
    last_freq;	/* My addition, Last frequency used in calculations */

  /* My addition, replace xtemp, ytemp etc in freq. loop */
  long double
    *xtemp,
    *ytemp,
    *ztemp,
    *sitemp,
    *bitemp;

  double *freq;	/* My addition, frequencies used in freq loop */
  char *fstep;	/* My addition, freq loop steps that returned results */

} save_t;

/* common  /segj/ */
typedef struct
{
  int
    *jco,	/* Stores connection data */
    jsno,	/* Total number of entries in ax, bx, cx */
    maxcon; /* Max. no. connections */

  long double
    *ax, *bx, *cx; /* Store constants A, B, C used in current expansion */

} segj_t;

/* common  /smat/ */
typedef struct
{
  int nop; /* My addition */

  complex long double *ssx;

} smat_t;

/* common  /tmi/ */
typedef struct
{
  int ij;

  long double
    zpk,
    rkb2;

} tmi_t;

/*common  /tmh/ */
typedef struct
{
  long double
    zpka,
    rhks;

} tmh_t;

/* common  /vsorc/ */
typedef struct
{
  int
    *isant,	/* Num of segs on which an aplied field source is located */
    *ivqd,	/* Num of segs on which a current-slope discontinuity source is located */
    *iqds,	/* Same as above (?) */
    nsant,	/* Number of applied field voltage sources */
    nvqd,	/* Number of applied current-slope discontinuity sources */
    nqds;	/* Same as above (?) */

  complex long double
    *vqd,	/* Voltage of applied-current slope discontinuity sources */
    *vqds,	/* Same as above (?) */
    *vsant;	/* Voltages of applied field voltage sources */

} vsorc_t;

/* common  /yparm/ */
/* CP card support has been dropped */

/* common  /zload/ */
typedef struct
{
  int
    *ldsegn,	/* My addition, loaded segment numbers */
    *ldtype,	/* My addition, loading type */
    nldseg,		/* My addition, number of loaded segs  */
    nload;		/* Number of loading networks */

  complex long double *zarray;	/* = Zi/(Di/lambda) */

} zload_t;

/* Parameters for projecting points on screen */
typedef struct
{
  double
    Wi,		/* Angle (inclination) of Z axis to Screen, +ve to viewer */
    Wr,		/* Rotation of X-Y plane around Z axis, +ve clockwise */
    W_incr,	/* Projection angle increment (degrees) */
    sin_wi,	/* sin(Wi) */
    cos_wi,	/* cos(Wi) */
    sin_wr,	/* sin(Wr) */
    cos_wr,	/* cos(Wr) */
    r_max,	/* Max distance from xyz origin of a point in antenna */
    xy_scale,	/* Scale factor to fit structure into drawable window */
    x_center,	/* X co-ordinate of xyz axes origin in screen's drawable */
    y_center;	/* Y co-ordinate of xyz axes origin in screen's drawable */

  int
    pixmap_width,  /*  Width of drawable's backing pixmap */
    pixmap_height; /* Height of drawable's backing pixmap */

} projection_parameters_t;

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
    ifrq,		/* Frequency specification flag */
    nfrq,		/* Number of frequency steps */
    fstep,		/* Frequency step */
	lastf,		/* Last frequency step */
    ngraph,		/* Number of graphs to be plotted */
    pol_type,	/* User-specified Polarization type for plots and patterns */
	nfork;		/* Number of child processes to fork */

  long double
    *zlr,
    *zli,
    *zlc;

  long double
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
    fmhz,		/* Frequency in MHz, moved from save_t */
    delfrq;		/* Frequency step for freq loop */

  double
	mxfrq,	/* Max frequency in freq loop */
	zo;		/* Characteristic impedance used in VSWR calcs */

} calc_data_t;

/* Impedance data */
typedef struct
{
  double
    *zreal,		/* Real part of input impedance */
    *zimag,		/* Imag part of input impedance */
    *zmagn,		/* Magnitude of input impedance */
    *zphase;	/* Phase ang of input impedance */

} impedance_data_t;

/* Radiation pattern data */
typedef struct
{
  double
    *gtot,			/* Radiation pattern (total gain) buffer */
    *max_gain,		/* Maximum gain for each polarization type */
    *min_gain,		/* Minimum gain for each polarization type */
    *max_gain_tht,	/* Theta angle where maximum gain occurs */
    *max_gain_phi,	/*   Phi angle where minimum gain occurs */
    *tilt,			/* Tilt angle of polarization ellipse  */
    *axrt;			/* Elliptic axial ratio of pol ellipse */

  int
    *max_gain_idx,	/* Where in rad_pattern.gtot the max value occurs */
    *min_gain_idx,	/* Where in rad_pattern.gtot the min value occurs */
    *sens;			/* Polarization sense (vertical, horizontal, elliptic etc) */

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

/* Point in 3d xyz axes */
typedef struct
{
  double
    x, y, z, r;

} point_3d_t;

/* Forked processes data */
typedef struct
{
  pid_t chpid;		/* Child pid */
  int p2ch_pipe[2];	/* Parent-to-child write pipe */
  int ch2p_pipe[2];	/* Child-to-parent write pipe */
  char busy;		/* Child process busy flag */
  int fstep;		/* Frequency step assigned to child */

  /* File descriptor sets for select() */
  fd_set read_fds;
  fd_set write_fds;

} forkpc_t;

/* Function prototypes produced by cproto */
/* calculations.c */
void qdsrc(int is, complex long double v, complex long double *e);
void cabc(complex long double *curx);
int couple(complex long double *cur, long double wlam);
long double db10(long double x);
long double db20(long double x);
void intrp(long double x, long double y, complex long double *f1,
	complex long double *f2, complex long double *f3, complex long double *f4);
void intx(long double el1, long double el2, long double b, int ij,
	long double *sgr, long double *sgi);
int min(int a, int b);
void test(long double f1r, long double f2r, long double *tr,
	long double f1i, long double f2i, long double *ti, long double dmin);
int sbf(int i, int is, long double *aa, long double *bb, long double *cc);
int tbf(int i, int icap);
int trio(int j);
long double cang(complex long double z);
void zint(long double sigl, long double rolam, complex long double *zint);
/* callback_func.c */
void Save_Pixmap(GdkPixmap *pixmap, int pixmap_width, int pixmap_height, char *filename);
void New_Viewer_Angle(double wr, double wi, GtkSpinButton *wr_spb,
	GtkSpinButton *wi_spb, projection_parameters_t *params);
void Create_Pixmap(GdkPixmap **pixmap, int *pixmap_width, int *pixmap_height,
	GtkWidget *widget, GdkEventConfigure *event, projection_parameters_t *params);
void Motion_Event(GdkEventMotion *event, projection_parameters_t *params,
	GtkSpinButton *wr_spb, GtkSpinButton *wi_spb);
void Plot_Select(GtkToggleButton *togglebutton, int flag);
void Delete_Event(gchar *message);
void Set_Pol_Menuitem(GtkMenuItem *menuitem);
void Close_Windows(void);
gboolean Open_Editor(GtkTreeView *view);
gboolean Nec2_Apply_Checkbutton(void);
void Gtk_Quit(void);
void Pass_EH_Flags(void);
void Alloc_Crnt_Buffs(void);
void Free_Crnt_Buffs(void);
void About_Dialog(GtkWidget *aboutdialog);
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
void Insert_EN_Card(GtkListStore *store, GtkTreeIter *iter);
void Get_Command_Data(GtkListStore *store, GtkTreeIter *iter, int *iv, double *fv);
void Set_Command_Data(GtkListStore *store, GtkTreeIter *iter, int *iv, double *fv);
void Insert_Blank_Command_Row(GtkTreeView *view,
	GtkListStore *store, GtkTreeIter *iter, const gchar *name);
void Set_Labels(GtkWidget *widget, gchar **labels, gchar **text, gint num);
/* draw.c */
void Set_Gdk_Segment(GdkSegment *segm, projection_parameters_t *param,
	double x1, double y1, double z1, double x2, double y2, double z2);
void Project_on_Screen(projection_parameters_t *param,
	double x, double y, double z, double *xs, double *ys);
void Project_XYZ_Axes(GdkPixmap *pixmap, projection_parameters_t *param, GdkSegment *segm);
void Draw_XYZ_Axes(GdkPixmap *pixmap, projection_parameters_t params);
void New_Projection_Parameters(int width, int height, projection_parameters_t *param);
void Set_GC_Attributes(GdkGC *gc, int red, int green,
	int blue, gint line_width, GtkWidget *widget);
void Value_to_Color(int *red, int *grn, int *blu, double val, double max);
/* draw_radiation.c */
void Draw_Radiation(GtkWidget *drawingarea, GdkGC *gc);
void Draw_Radiation_Pattern(GtkWidget *drawingarea, GdkGC *gc);
void Draw_Near_Field(GtkWidget *drawingarea, GdkGC *gc);
gboolean Animate_Near_Field(gpointer data);
double Scale_Gain(double gain, int fstep, int idx);
double Polarization_Factor(int pol_type, int fstep, int idx);
void Set_Polarization(int pol);
void Set_Gain_Style(int gs);
void New_Radiation_Projection_Angle(void);
gboolean Redo_Radiation_Pattern(gpointer user_data);
double Viewer_Gain(projection_parameters_t proj_parameters, int fstep);
void Rdpattern_Window_Killed(void);
void Set_Window_Labels(void);
void Alloc_Rdpattern_Buffers(int nfrq, int nth, int nph);
void Alloc_Nearfield_Buffers(int n1, int n2, int n3);
void Free_Draw_Buffers(void);
/* draw_structure.c */
void Draw_Structure(GtkWidget *drawingarea, GdkGC *gc);
void New_Wire_Data(void);
void New_Patch_Data(void);
void Process_Wire_Segments(void);
void Process_Surface_Patches(void);
void Draw_Wire_Segments(GtkWidget *drawingarea, GdkGC *gc, GdkSegment *segm, gint nseg);
void Draw_Surface_Patches(GtkWidget *drawingarea, GdkGC *gc,
	GdkSegment *segm, gint npatch);
gboolean Redo_Currents(gpointer userdata);
void New_Structure_Projection_Angle(void);
void Init_Struct_Drawing(void);
void Show_Viewer_Gain(GtkWidget *window, const char *widget,
	projection_parameters_t proj_params);
/* fields.c */
void efld(long double xi, long double yi, long double zi, long double ai, int ij);
void eksc(long double s, long double z, long double rh, long double xk, int ij,
	complex long double *ezs, complex long double *ers, complex long double *ezc,
	complex long double *erc, complex long double *ezk, complex long double *erk);
void ekscx(long double bx, long double s, long double z, long double rhx,
	long double xk, int ij, int inx1, int inx2, complex long double *ezs,
	complex long double *ers, complex long double *ezc, complex long double *erc,
	complex long double *ezk, complex long double *erk);
void gf(long double zk, long double *co, long double *si);
void gh(long double zk, long double *hr, long double *hi);
void gwave(complex long double *erv, complex long double *ezv,
	complex long double *erh, complex long double *ezh, complex long double *eph);
void gx(long double zz, long double rh, long double xk,
	complex long double *gz, complex long double *gzp);
void gxx(long double zz, long double rh, long double a, long double a2,
	long double xk, int ira, complex long double *g1, complex long double *g1p,
	complex long double *g2, complex long double *g2p, complex long double *g3,
	complex long double *gzp);
void hfk(long double el1, long double el2, long double rhk,
	long double zpkx, long double *sgr, long double *sgi);
void hintg(long double xi, long double yi, long double zi);
void hsfld(long double xi, long double yi, long double zi, long double ai);
void hsflx(long double s, long double rh, long double zpx, complex long double *hpk,
	complex long double *hps, complex long double *hpc);
void nefld(long double xob, long double yob, long double zob,
	complex long double *ex, complex long double *ey, complex long double *ez);
void nfpat(int nfeh);
void nhfld(long double xob, long double yob, long double zob,
	complex long double *hx, complex long double *hy, complex long double *hz);
void pcint(long double xi, long double yi, long double zi, long double cabi,
	long double sabi, long double salpi, complex long double *e);
void unere(long double xob, long double yob, long double zob);
void Near_Field_Total(complex long double ex, complex long double ey,
	complex long double ez, int nfeh, int idx);
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
void Insert_GE_Card(GtkListStore *store, GtkTreeIter *iter);
void Get_Geometry_Data(GtkListStore *store, GtkTreeIter *iter, int *iv, double *fv);
void Get_Geometry_Int_Data(GtkListStore *store, GtkTreeIter *iter, int *iv);
void Set_Geometry_Data(GtkListStore *store, GtkTreeIter *iter, int *iv, double *fv);
void Set_Geometry_Int_Data(GtkListStore *store, GtkTreeIter *iter, int *iv);
gboolean Check_Card_Name(GtkListStore *store,
	GtkTreeIter *iter, gboolean next, const gchar *name);
gboolean Give_Up(int *busy, GtkWidget *widget);
void Insert_Blank_Geometry_Row(GtkTreeView *view,
	GtkListStore *store, GtkTreeIter *iter, const gchar *name);
void Remove_Row(GtkTreeView *view, GtkListStore *store, GtkTreeIter *iter);
gboolean Get_Selected_Row(GtkTreeView *view,
	GtkListStore *store, GtkTreeIter *iter, gchar *name);
void Set_Wire_Conductivity(int tag, double s, GtkListStore *store);
/* geometry.c */
int arc(int itg, int ns, long double rada,
	long double ang1, long double ang2, long double rad);
int conect(int ignd);
void helix(long double s, long double hl, long double a1, long double b1,
	long double a2, long double b2, long double rad, int ns, int itg);
int isegno(int itagi, int mx);
void move(long double rox, long double roy, long double roz, long double xs,
	long double ys, long double zs, int its, int nrpt, int itgi);
int patch(int nx, int ny, long double ax1, long double ay1, long double az1,
	long double ax2, long double ay2, long double az2, long double ax3,
	long double ay3, long double az3, long double ax4, long double ay4, long double az4);
void subph(int nx, int ny);
int reflc(int ix, int iy, int iz, int itx, int nop);
void wire(long double xw1, long double yw1, long double zw1, long double xw2,
	long double yw2, long double zw2, long double rad, long double rdel,
	long double rrad, int ns, int itg);
/* ground.c */
int rom2(long double a, long double b, complex long double *sum, long double dmin);
void sflds(long double t, complex long double *e);
/* gtk-fork.c */
void sigchld_handler(int num);
gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void destroy(GtkWidget *widget, gpointer data);
void fork_me(GtkWidget *widget, gpointer data);
int main(int argc, char *argv[]);
/* input.c */
int Read_Comments(void);
int Read_Geometry(void);
int Read_Commands(void);
int readmn(char *gm, int *i1, int *i2, int *i3, int *i4, long double *f1,
	long double *f2, long double *f3, long double *f4, long double *f5,
	long double *f6, int err);
int readgm(char *gm, int *i1, int *i2, long double *x1, long double *y1,
	long double *z1, long double *x2, long double *y2, long double *z2,
	long double *rad, int err);
int datagn(void);
gboolean Tag_Seg_Error(int tag, int segs);
/* main.c */
int main(int argc, char *argv[]);
int isFlagSet(long long int flag);
int isFlagClear(long long int flag);
void SetFlag(long long int flag);
void ClearFlag(long long int flag);
void ToggleFlag(long long int flag);
gboolean Open_Input_File(gpointer data);
gboolean isChild(void);
/* matrix.c */
void cmset(int nrow, complex long double *cm, long double rkhx, int iexkx);
void cmss(int j1, int j2, int im1, int im2,
	complex long double *cm, int nrow, int itrp);
void cmsw(int j1, int j2, int i1, int i2, complex long double *cm,
	complex long double *cw, int ncw, int nrow, int itrp);
void cmws(int j, int i1, int i2, complex long double *cm,
	int nr, complex long double *cw, int nw, int itrp);
void cmww(int j, int i1, int i2, complex long double *cm, int nr,
	complex long double *cw, int nw, int itrp);
void etmns(long double p1, long double p2, long double p3, long double p4,
	long double p5, long double p6, int ipr, complex long double *e);
void factr(int n, complex long double *a, int *ip, int ndim);
void factrs(int np, int nrow, complex long double *a, int *ip);
int fblock(int nrow, int ncol, int imax, int ipsym);
void solve(int n, complex long double *a, int *ip, complex long double *b, int ndim);
void solves(complex long double *a, int *ip, complex long double *b,
	int neq, int nrh, int np, int n, int mp, int m);
/* misc.c */
void usage(void);
int stop(char *mesg, int err);
int load_line(char *buff, FILE *pfile);
void mem_alloc(void **ptr, int req, gchar *str);
void mem_realloc(void **ptr, int req, gchar *str);
void free_ptr(void **ptr);
gboolean Open_File(FILE **fp, char *fname, const char *mode);
void Close_File(FILE **fp);
/* nec2_model.c */
int Nec2_Input_File_Treeview(int action);
void Insert_Columns(GtkWidget *window, gchar *treeview,
	GtkListStore *store, int ncols, char *colname[]);
void cell_edited_callback(GtkCellRendererText *cell,
	gchar *path_string, gchar *new_text, gpointer user_data);
void Save_Nec2_Input_File(GtkWidget *treeview_window, char *nec2_file);
void Save_Treeview_Data(GtkTreeView *tree_view, int ncols, FILE *nec2_fp);
gboolean gtk_tree_model_iter_previous(GtkTreeModel *tree_model, GtkTreeIter *iter);
/* network.c */
void netwk(complex long double *cm, int *ip, complex long double *einc);
int load(int *ldtyp, int *ldtag, int *ldtagf, int *ldtagt,
	long double *zlr, long double *zli, long double *zlc);
void prnt(int in1, int in2, int in3, long double fl1, long double fl2, long double fl3,
	long double fl4, long double fl5, long double fl6, char *ia, int ichar);
/* plot_freqdata.c */
void Plot_Frequency_Data(void);
void Display_Frequency_Data(void);
void Draw_Plotting_Frame(gchar **title, GdkRectangle *rect, int nhor, int nvert);
void Plot_Vertical_Scale(GdkGC *gc, int x, int y,
	int height, double max, double min, int nval);
void Plot_Horizontal_Scale(GdkGC *gc, int x, int y,
	int width, double max, double min, int nval);
void Draw_Graph(GdkGC *gc, GdkRectangle *rect, double *a, double *b,
	double amax, double amin, double bmax, double bmin, int nval, int side);
void Set_Rectangle(GdkRectangle *rect, int x, int y, int w, int h);
void Fit_to_Scale(double *max, double *min, int *nval);
void Fit_to_Scale2(double *max1, double *min1, double *max2, double *min2, int *nval);
void New_Max_Min(double *max, double *min, double sval, int *nval);
void Plot_Graph2(double *fa, double *fb, double *fc,
	int nc, char *titles[], int nplt, int posn);
void Plot_Graph(double *fa, double *fb, int nb, char *titles[], int nplt, int posn);
void Plots_Window_Killed(void);
void Set_Frequency_On_Click(GtkWidget *widget, GdkEventButton *event);
/* radiation.c */
void ffld(long double thet, long double phi,
	complex long double *eth, complex long double *eph);
void fflds(long double rox, long double roy, long double roz,
	complex long double *scur, complex long double *ex,
	complex long double *ey, complex long double *ez);
void gfld(long double rho, long double phi, long double rz,
	complex long double *eth, complex long double *epi,
	complex long double *erd, complex long double ux, int ksymp);
void rdpat(void);
/* somnec.c */
void somnec(long double epr, long double sig, long double fmhz);
void bessel(complex long double z, complex long double *j0, complex long double *j0p);
void evlua(complex long double *erv, complex long double *ezv,
	complex long double *erh, complex long double *eph);
void fbar(complex long double p, complex long double *fbar);
int gshank(complex long double start, complex long double dela,
	complex long double *sum, int nans, complex long double *seed,
	int ibk, complex long double bk, complex long double delb);
int hankel(complex long double z, complex long double *h0, complex long double *h0p);
void lambda(long double t, complex long double *xlam, complex long double *dxlam);
void rom1(int n, complex long double *sum, int nx);
void saoa(long double t, complex long double *ans);
/* xnec2c.c */
void Structure_Impedance_Loading(void);
int Ground_Parameters(void);
void Set_Interaction_Matrix(void);
void Frequency_Scale_Geometry(void);
void Network_Data(void);
void Set_Network_Data(void);
void Set_Excitation(void);
void Power_Budget(void);
void Segment_Current_Header(void);
void Segment_Currents(int inc_fld_cnt);
void Patch_Currents(void);
void Normalized_Receive_Pattern(void);
void Normalized_Impedance(void);
void Frequency_Data(void);
void Near_Field_Pattern(void);
void Radiation_Pattern(void);
void Coupling(void);
void New_Frequency(void);
gboolean Frequency_Loop(gpointer user_data);
gboolean Start_Frequency_Loop(void);
void Stop_Frequency_Loop(void);
void Incident_Field_Loop(void);
void Null_Pointers(void);
/* fork.c */
void Child_Input_File(void);
void Child_Process(void);
void Child_Input_File(void);
int  Fork_Command(const char *cdstr);
int  Read_Pipe(int idx, char *str, size_t len, gboolean err);
int  PRead_Pipe(int idx, char *str, size_t len, gboolean err);
int  Write_Pipe(int idx, char *str, size_t len, gboolean err);
void Pass_Freq_Data(void);
void Get_Freq_Data(int idx, int fstep);
void Mem_Copy(char *buff, char *var, size_t cnt, gboolean wrt);

#endif
