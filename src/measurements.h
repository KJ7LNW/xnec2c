#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H 1

#include <math.h>
#include <stdint.h>
#include <stdio.h>

// The enum, and the struct measurement_t values must be in
// the same order or this will not work!
enum MEASUREMENT_INDEXES
{
	MEAS_MHZ,
	MEAS_ZREAL,
	MEAS_ZIMAG,
	MEAS_ZMAG,
	MEAS_ZPHASE,
	MEAS_VSWR,
	MEAS_S11,
	MEAS_S11_REAL,
	MEAS_S11_IMAG,
	MEAS_S11_ANG,
	MEAS_GAIN_MAX,
	MEAS_GAIN_NET,
	MEAS_GAIN_THETA,
	MEAS_GAIN_PHI,
	MEAS_GAIN_VIEWER,
	MEAS_GAIN_VIEWER_NET,
	MEAS_FB_RATIO,
	MEAS_GAIN_DEV_PX,
	MEAS_GAIN_DEV_NX,
	MEAS_GAIN_DEV_PY,
	MEAS_GAIN_DEV_NY,
	MEAS_GAIN_DEV_PZ,
	MEAS_GAIN_DEV_NZ,
	MEAS_ANT_TEMP,
	MEAS_ANT_TEMP_TOT,
	MEAS_GT,

	MEAS_COUNT
};

// Defined in measurements.c:
extern const char *meas_names[];
extern const char *meas_display_names[];
extern const char *meas_descriptions[];

/**
 * ant_temp_z_world() - world-frame z-component of a pattern cell after
 *     virtual rotation to the desired antenna elevation.
 * @tht:      NEC theta of the cell (radians)
 * @phi:      NEC phi of the cell (radians)
 * @tht_mg:   NEC theta of the max-gain direction (radians)
 * @phi_mg:   NEC phi of the max-gain direction (radians)
 * @elev_rad: desired boresight elevation above horizontal (radians)
 *
 * Rodrigues rotation about the horizontal axis perpendicular to the
 * max-gain azimuth: k = (-sin phi_mg, cos phi_mg, 0).
 * The rotation angle places the boresight at elevation elev_rad.
 *
 * Returns positive when the cell faces sky, negative for earth.
 *
 * Callers compare against ANT_TEMP_Z_EPSILON rather than 0.0 to avoid
 * IEEE754 noise at exact boundary angles (e.g. cos(pi/2) ≈ 6e-17).
 */
#define ANT_TEMP_Z_EPSILON 1e-10

/* Custom-temperature edit thresholds and seed values.
 *
 * ANT_TEMP_K_MIN: strict lower bound for a valid Kelvin entry; values
 * at or below this are treated as parse failure / out-of-range.
 *
 * ANT_TEMP_K_EPSILON: minimum delta (K) between an edited entry and
 * the resolved model value before switching to the Custom model.
 *
 * ANT_TEMP_CUSTOM_T_*_DEFAULT: initial constants used when no rc file
 * value is present or the persisted value fails validation.
 */
#define ANT_TEMP_K_MIN                   0.0
#define ANT_TEMP_K_EPSILON               0.5
#define ANT_TEMP_CUSTOM_T_SKY_DEFAULT    200.0
#define ANT_TEMP_CUSTOM_T_EARTH_DEFAULT  1000.0
static inline double ant_temp_z_world(
		double tht, double phi,
		double tht_mg, double phi_mg,
		double elev_rad)
{
	double alpha = tht_mg + elev_rad;
	return cos(tht) * sin(alpha)
		- sin(tht) * cos(phi - phi_mg) * cos(alpha);
}

/**
 * ant_temp_rotate_point() - Rodrigues rotation of a pattern cell direction
 * @tht:      NEC theta of the cell (radians)
 * @phi:      NEC phi of the cell (radians)
 * @tht_mg:   NEC theta of the max-gain direction (radians)
 * @phi_mg:   NEC phi of the max-gain direction (radians)
 * @elev_rad: desired boresight elevation above horizontal (radians)
 * @xr:       output rotated x component
 * @yr:       output rotated y component
 * @zr:       output rotated z component
 *
 * Applies the same Rodrigues rotation as ant_temp_z_world() but returns
 * the full rotated Cartesian direction vector.  Used by the noise-mode
 * renderer to place pattern cells at visually rotated positions so the
 * sky/earth boundary appears horizontal while the pattern tilts upward.
 *
 * Rotation axis: k = (-sin phi_mg, cos phi_mg, 0)
 * Rotation angle: elev_rad
 * Formula: v' = v cos E + (k x v) sin E + k (k . v)(1 - cos E)
 */
static inline void ant_temp_rotate_point(
		double tht, double phi,
		double tht_mg, double phi_mg,
		double elev_rad,
		double *xr, double *yr, double *zr)
{
	/* Original Cartesian direction from spherical (θ, φ) */
	double st = sin(tht), ct = cos(tht);
	double sp = sin(phi), cp = cos(phi);
	double vx = st * cp;
	double vy = st * sp;
	double vz = ct;

	/* Rotation axis k = (-sin φ_mg, cos φ_mg, 0) */
	double kx = -sin(phi_mg);
	double ky =  cos(phi_mg);

	/* k × v  (kz = 0) */
	double cx = -ky * vz;
	double cy =  kx * vz;
	double cz =  ky * vx - kx * vy;

	/* k · v  (kz = 0) */
	double dot = kx * vx + ky * vy;

	double ce = cos(elev_rad);
	double se = sin(elev_rad);

	*xr = vx * ce + cx * se + kx * dot * (1.0 - ce);
	*yr = vy * ce + cy * se + ky * dot * (1.0 - ce);
	*zr = vz * ce + cz * se;
}

/* Interpolation / resolution method for noise temperature models.
 * FORMULA: ITU-R P.372-16 Table 1 (Spaulding & Disney 1974)
 * GALACTIC: ITU-R P.372-16 §4.1 eq.13/14 (Reich & Reich 1988) */
typedef enum
{
	ANT_TEMP_SNAP,       /* nearest-band snap */
	ANT_TEMP_INTERP,     /* log-log interpolation, clamp at edges */
	ANT_TEMP_FORMULA,    /* F_am = c - d·log10(f), T = T0·(10^(Fa/10) - 1) */
	ANT_TEMP_GALACTIC,   /* P.372-16 eq.13 below break, eq.14 above */
	ANT_TEMP_METHOD_COUNT,
} ant_temp_method_t;

/* Sky noise model selector
 *
 * DG7YBN:   dg7ybn.de/GT_Tables/ (Kluver, Dubus 2/2019 pp.116-117)
 * G4CQM:    g4cqm.co.uk (VK3UM Calc compliance, pre-2016)
 * VK3UM:    sm2cew.com/rpc.pdf (McArthur, SK 2016)
 * Galactic: ITU-R P.372-16 §4.1 eq.13/14 (2022)
 * Base Ref: VE7BQH Era 1 / F5FOD AGTC-JS normalization (pre-2018)
 * Synth:    debug/plot_proposed_models.py (2026-04 analysis)
 */
typedef enum
{
	ANT_TEMP_SKY_G4CQM_MIN,      /* minimum quiet, 5 bands (pre-2016) */
	ANT_TEMP_SKY_VK3UM_MIN,      /* minimum quiet, 6 bands (pre-2016) */
	ANT_TEMP_SKY_BASE_REF,        /* VE7BQH Era 1 frozen values (pre-2018) */
	ANT_TEMP_SKY_GALACTIC,        /* P.372-16 eq.13/14, continuous (2022) */
	ANT_TEMP_SKY_DG7YBN_AVG,     /* galactic average, 3 bands (2025) */
	ANT_TEMP_SKY_SYNTH_AVG,       /* synthesized practical average, 17 bands (2026) */
	ANT_TEMP_SKY_SYNTH_MIN,       /* synthesized minimum quiet, 17 bands (2026) */
	ANT_TEMP_SKY_CUSTOM,          /* user-defined constant, single value */
	ANT_TEMP_SKY_COUNT,
} ant_temp_sky_t;

/* Earth noise model selector
 *
 * DG7YBN:   dg7ybn.de/GT_Tables/ Era 3 (2025-08-29)
 *           50/144 MHz: ITU-R P.372-16 §6 pp.98-100
 *           432 MHz: URSI Radio Science Bulletin No.334, 09.2010
 *           (Lefering et al, Ghent University INTEC / OFCOM)
 * G4CQM:   g4cqm.co.uk (pre-2016, matches VK3UM Calc defaults)
 * ITU:     ITU-R P.372-16 Table 1, F_am = c - d·log10(f_MHz)
 *          Coefficients from Spaulding & Disney OT-74-38 (1974)
 * Base Ref: VE7BQH Era 1 (pre-2018 frozen)
 */
typedef enum
{
	ANT_TEMP_EARTH_ITU_BUSINESS,
	ANT_TEMP_EARTH_ITU_RESIDENTIAL,
	ANT_TEMP_EARTH_ITU_RURAL,
	ANT_TEMP_EARTH_ITU_QUIET_RURAL,
	ANT_TEMP_EARTH_G4CQM_MIXED,
	ANT_TEMP_EARTH_BASE_REF,
	ANT_TEMP_EARTH_DG7YBN_RURAL,
	ANT_TEMP_EARTH_DG7YBN_RESIDENTIAL,
	ANT_TEMP_EARTH_DG7YBN_CITY,
	ANT_TEMP_EARTH_CUSTOM,        /* user-defined constant, single value */
	ANT_TEMP_EARTH_COUNT,
} ant_temp_earth_t;

/* Unified noise temperature model descriptor.
 * One struct type serves both sky_models[] and earth_models[]. */
typedef struct
{
	const char *name;

	ant_temp_method_t method;        /* default resolution method */

	/* Table anchors (SNAP and INTERP) */
	const double *freq_mhz;
	const double *temp;
	int freq_count;

	/* Formula coefficients: F_am = c - d·log10(f_MHz) */
	double c, d;

	/* Galactic power-law (ANT_TEMP_GALACTIC only) */
	double freq_break;               /* eq.13/14 transition, MHz */
	double power_exponent;           /* -2.75 */
	double power_offset;             /* CMB 2.7 K */

	/* Bitmask of interp methods this model supports as user overrides */
	uint8_t valid_interp;
} ant_temp_model_t;

#define ANT_TEMP_METHOD_BIT(m) (1u << (m))
#define ANT_TEMP_TABLE_METHODS \
	(ANT_TEMP_METHOD_BIT(ANT_TEMP_SNAP) | ANT_TEMP_METHOD_BIT(ANT_TEMP_INTERP))

extern const ant_temp_model_t sky_models[ANT_TEMP_SKY_COUNT];
extern const ant_temp_model_t earth_models[ANT_TEMP_EARTH_COUNT];

extern const char *ant_temp_method_names[ANT_TEMP_METHOD_COUNT];

/* Per-fstep noise temperature table (Tier 2).
 * Filled by ant_temp_fill_fstep() in the child; read by
 * ff_presentation_recompute() in the parent.
 * Custom model slots store sentinel -1.0; parent substitutes
 * rc_config.ant_temp_custom_t_* at display time. */
typedef struct
{
  double t_sky[ANT_TEMP_SKY_COUNT][ANT_TEMP_METHOD_COUNT];
  double t_earth[ANT_TEMP_EARTH_COUNT][ANT_TEMP_METHOD_COUNT];
} noise_temp_t;

extern noise_temp_t *noise_temp;

typedef struct
{
	union {
		struct
		{
			double mhz;
			double zreal, zimag;
			double zmag, zphase;
			double vswr;
			double s11;
			double s11_real, s11_imag, s11_ang;
			double gain_max, gain_net;
			double gain_max_theta, gain_max_phi;
			double gain_viewer, gain_viewer_net;
			double fb_ratio;
			double gain_dev_px, gain_dev_nx;
			double gain_dev_py, gain_dev_ny;
			double gain_dev_pz, gain_dev_nz;
			double ant_temp, ant_temp_tot;
			double gt;
		};

		double a[MEAS_COUNT];
	};

} measurement_t;


int ant_temp_resolve(double freq_mhz, int sky, int earth, int interp,
	double *t_sky, double *t_earth);
void ant_temp_fill_fstep(int fstep);
void noise_temp_resolve(int fstep, double *t_sky, double *t_earth);
const char *ant_temp_sky_name(int idx);
const char *ant_temp_earth_name(int idx);
int meas_has_impedance(int idx);
void meas_calc(measurement_t *m, int idx, int port);
int meas_name_idx(const char *name, int len);
void meas_format(measurement_t *m, const char *format, char *out, int outlen);
int meas_write_format(measurement_t *m, const char *format, FILE *fp);

void meas_write_header(FILE *fp, char *delim);
void meas_write_data(FILE *fp, char *delim);

void meas_write_header_enc(FILE *fp, char *delim, char *left, char *right);
void meas_write_data_enc(FILE *fp, char *delim, char *left, char *right);

#endif
