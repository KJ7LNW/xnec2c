#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H 1

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

	MEAS_COUNT
};

// Defined in measurements.c:
extern const char *meas_names[];
extern const char *meas_display_names[];
extern const char *meas_descriptions[];

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
		};

		double a[MEAS_COUNT];
	};

} measurement_t;


void meas_calc(measurement_t *m, int idx);
int meas_name_idx(char *name, int len);
void meas_format(measurement_t *m, char *format, char *out, int outlen);
int meas_write_format(measurement_t *m, char *format, FILE *fp);

void meas_write_header(FILE *fp, char *delim);
void meas_write_data(FILE *fp, char *delim);

void meas_write_header_enc(FILE *fp, char *delim, char *left, char *right);
void meas_write_data_enc(FILE *fp, char *delim, char *left, char *right);

#endif
