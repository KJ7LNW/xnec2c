#include "common.h"
#include "shared.h"

#define clog10(z) (clog(z) / log(10))

// Ordering doesn't matter here because we use [] indexes,
// but it still would be nice to keep the same order as the
// enum MEASUREMENT_INDEXES.
const char *meas_names[] = {
	[MEAS_MHZ]              =  "mhz",
	[MEAS_ZREAL]            =  "zreal",
	[MEAS_ZIMAG]            =  "zimag",
	[MEAS_ZMAG]             =  "zmag",
	[MEAS_ZPHASE]           =  "zphase",
	[MEAS_VSWR]             =  "vswr",
	[MEAS_S11]              =  "s11",
	[MEAS_S11_REAL]         =  "s11_real",
	[MEAS_S11_IMAG]         =  "s11_imag",
	[MEAS_S11_ANG]          =  "s11_ang",
	[MEAS_GAIN_MAX]         =  "gain_max",
	[MEAS_GAIN_NET]         =  "gain_net",
	[MEAS_GAIN_THETA]       =  "gain_max_theta",
	[MEAS_GAIN_PHI]         =  "gain_max_phi",
	[MEAS_GAIN_VIEWER]      =  "gain_viewer",
	[MEAS_GAIN_VIEWER_NET]  =  "gain_viewer_net",
	[MEAS_FB_RATIO]         =  "fb_ratio",
	[MEAS_GAIN_DEV_PX]     =  "gain_dev_px",
	[MEAS_GAIN_DEV_NX]     =  "gain_dev_nx",
	[MEAS_GAIN_DEV_PY]     =  "gain_dev_py",
	[MEAS_GAIN_DEV_NY]     =  "gain_dev_ny",
	[MEAS_GAIN_DEV_PZ]     =  "gain_dev_pz",
	[MEAS_GAIN_DEV_NZ]     =  "gain_dev_nz",
	[MEAS_COUNT]            =  NULL
};

const char *meas_display_names[] = {
	[MEAS_MHZ]              =  "MHz",
	[MEAS_ZREAL]            =  "Z Real",
	[MEAS_ZIMAG]            =  "Z Imaginary",
	[MEAS_ZMAG]             =  "Z Magnitude",
	[MEAS_ZPHASE]           =  "Z Phase",
	[MEAS_VSWR]             =  "VSWR",
	[MEAS_S11]              =  "S11",
	[MEAS_S11_REAL]         =  "S11 Real",
	[MEAS_S11_IMAG]         =  "S11 Imaginary",
	[MEAS_S11_ANG]          =  "S11 Angle",
	[MEAS_GAIN_MAX]         =  "Max Gain",
	[MEAS_GAIN_NET]         =  "Net Gain",
	[MEAS_GAIN_THETA]       =  "Gain Theta",
	[MEAS_GAIN_PHI]         =  "Gain Phi",
	[MEAS_GAIN_VIEWER]      =  "Viewer Gain",
	[MEAS_GAIN_VIEWER_NET]  =  "Viewer Net Gain",
	[MEAS_FB_RATIO]         =  "F/B Ratio",
	[MEAS_GAIN_DEV_PX]     =  "Gain Dev +X",
	[MEAS_GAIN_DEV_NX]     =  "Gain Dev \xe2\x88\x92X",
	[MEAS_GAIN_DEV_PY]     =  "Gain Dev +Y",
	[MEAS_GAIN_DEV_NY]     =  "Gain Dev \xe2\x88\x92Y",
	[MEAS_GAIN_DEV_PZ]     =  "Gain Dev +Z",
	[MEAS_GAIN_DEV_NZ]     =  "Gain Dev \xe2\x88\x92Z",
	[MEAS_COUNT]            =  NULL
};

const char *meas_descriptions[] = {
	[MEAS_MHZ]              =  "Frequency in MHz (independent variable)",
	[MEAS_ZREAL]            =  "Real part of feed-point impedance (ohms)",
	[MEAS_ZIMAG]            =  "Imaginary part of feed-point impedance (ohms)",
	[MEAS_ZMAG]             =  "Magnitude of feed-point impedance (ohms)",
	[MEAS_ZPHASE]           =  "Phase angle of feed-point impedance (degrees)",
	[MEAS_VSWR]             =  "Voltage standing wave ratio (1.0 = perfect match)",
	[MEAS_S11]              =  "Return loss in dB (more negative = better match)",
	[MEAS_S11_REAL]         =  "Real part of S11 in dB",
	[MEAS_S11_IMAG]         =  "Imaginary part of S11 in dB",
	[MEAS_S11_ANG]          =  "Phase angle of reflection coefficient (degrees)",
	[MEAS_GAIN_MAX]         =  "Peak gain across all angles in dBi",
	[MEAS_GAIN_NET]         =  "Peak gain adjusted for mismatch loss in dBi",
	[MEAS_GAIN_THETA]       =  "Elevation angle of peak gain (degrees above horizon)",
	[MEAS_GAIN_PHI]         =  "Azimuth angle of peak gain (degrees)",
	[MEAS_GAIN_VIEWER]      =  "Gain toward current viewer angle in dBi",
	[MEAS_GAIN_VIEWER_NET]  =  "Viewer gain adjusted for mismatch loss in dBi",
	[MEAS_FB_RATIO]         =  "Front-to-back ratio in dB",
	[MEAS_GAIN_DEV_PX]     =  "Angular deviation of peak gain from +X axis (degrees)",
	[MEAS_GAIN_DEV_NX]     =  "Angular deviation of peak gain from -X axis (degrees)",
	[MEAS_GAIN_DEV_PY]     =  "Angular deviation of peak gain from +Y axis (degrees)",
	[MEAS_GAIN_DEV_NY]     =  "Angular deviation of peak gain from -Y axis (degrees)",
	[MEAS_GAIN_DEV_PZ]     =  "Angular deviation of peak gain from +Z axis (degrees)",
	[MEAS_GAIN_DEV_NZ]     =  "Angular deviation of peak gain from -Z axis (degrees)",
	[MEAS_COUNT]            =  NULL
};

// Calculates measurement data.
// m:   a measurement_t structure to fill
// idx: the index into the calculated data structures.
//
// Warning: idx is not checked to make sure it is valid.
void meas_calc(measurement_t *m, int idx)
{
	int pol = calc_data.pol_type;
	int mgidx;
	int i;

	double Zr, Zi, Zo = calc_data.zo;

	double zrpro2 = impedance_data.zreal[idx] + calc_data.zo;
	zrpro2 *= zrpro2;

	double zrmro2 = impedance_data.zreal[idx] - calc_data.zo;
	zrmro2 *= zrmro2;

	double zimag2 = impedance_data.zimag[idx] * impedance_data.zimag[idx];
	double gamma = sqrt( (zrmro2 + zimag2) / (zrpro2 + zimag2) );

	double complex z_load = impedance_data.zreal[idx] + I*impedance_data.zimag[idx];
	double complex cgamma = (z_load-Zo) / (z_load+Zo);

	double complex cs11 = 20*clog10( cgamma );

	double fbdir;
	int fbidx, nth, nph;

	// Start with invalidated values (-1) in case something cannot be
	// calculated due to NEC state or card configuration:
	for (i = 0; i < MEAS_COUNT; i++)
		m->a[i] = -1;

	m->mhz = save.freq[idx];

	Zr = m->zreal = impedance_data.zreal[idx];
	Zi = m->zimag = impedance_data.zimag[idx];

	m->zmag = impedance_data.zmagn[idx];
	m->zphase = impedance_data.zphase[idx];

	m->vswr = (1 + gamma) / (1 - gamma);
	m->s11 = 20*log10( gamma );

	// Note that creal(cs11) == creal(20*clog10( cgamma )) == 20*log10(cabs(cgamma)):
	m->s11_real = creal(cs11);
	m->s11_imag = cimag(cs11);
	m->s11_ang = cang(cgamma);

	double net_gain_adjust = 10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );

	// Everything below here is dependent on the radiation pattern
	// having been calculated, so fields will remain invalid (-1).
	if (rad_pattern == NULL)
		return;

	mgidx = rad_pattern[idx].max_gain_idx[pol];

	// This should never happen, but please report it with your .NEC file if it does.
	//
	// It should be fixed in commit 42afbe3a3, but just in case:
	if (mgidx < 0)
	{
		BUG("BUG: invalid mgidx=%d: idx=%d pol=%d fstep=%d last_step=%d freq_step=%d\n",
			mgidx, idx, pol, calc_data.last_step + 1,
			calc_data.last_step, calc_data.freq_step);
		BUG("BUG: save.fstep[%d]=%d FREQ_LOOP_STOP=%d\n", idx,
			save.fstep[idx], isFlagSet(FREQ_LOOP_STOP));
		mem_backtrace(rad_pattern[idx].max_gain_idx);
		return;
	}
	m->gain_max = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	m->gain_net = m->gain_max + net_gain_adjust;

	m->gain_viewer = Viewer_Gain(structure_proj_params, idx);
	m->gain_viewer_net = m->gain_viewer + net_gain_adjust;

	m->gain_max_theta = 90.0 - rad_pattern[idx].max_gain_tht[pol];
	m->gain_max_phi = rad_pattern[idx].max_gain_phi[pol];

	// Gain deviation from axis directions (great-circle angular distance).
	// NEC2 theta is from +Z axis; convert stored elevation back.
	{
		double nec_tht = (90.0 - m->gain_max_theta) * M_PI / 180.0;
		double nec_phi = m->gain_max_phi * M_PI / 180.0;
		double sin_tht = sin(nec_tht);
		double cos_tht = cos(nec_tht);

		// Axis target directions: {nec_theta, nec_phi} in radians
		//   +X: (pi/2, 0)     -X: (pi/2, pi)
		//   +Y: (pi/2, pi/2)  -Y: (pi/2, 3pi/2)
		//   +Z: (0, 0)        -Z: (pi, 0)
		static const double axis_tht[] = {
			M_PI/2, M_PI/2, M_PI/2, M_PI/2, 0.0, M_PI
		};
		static const double axis_phi[] = {
			0.0, M_PI, M_PI/2, 3*M_PI/2, 0.0, 0.0
		};
		int ax;

		for (ax = 0; ax < 6; ax++)
		{
			double cos_delta = cos_tht * cos(axis_tht[ax])
				+ sin_tht * sin(axis_tht[ax]) * cos(nec_phi - axis_phi[ax]);

			// Clamp to [-1,1] for numerical safety
			if (cos_delta > 1.0)
			{
				cos_delta = 1.0;
			}
			else if (cos_delta < -1.0)
			{
				cos_delta = -1.0;
			}

			m->a[MEAS_GAIN_DEV_PX + ax] = acos(cos_delta) * 180.0 / M_PI;
		}
	}

	// Find F/B direction in theta
	fbdir = 180.0 - rad_pattern[idx].max_gain_tht[pol];
	if (fpat.dth == 0.0)
		nth = 0;
	else
		nth = (int) (fbdir / fpat.dth + 0.5);

	/* If the antenna is modelled over ground, then use the same
	   theta as the max gain direction, relying on phi alone to take
	   us to the back. Patch supplied by Rik van Riel AB1KW */
	if ((nth >= fpat.nth) || (nth < 0))
	{
		fbdir = rad_pattern[idx].max_gain_tht[pol];
		if (fpat.dth == 0.0)
			nth = 0;
		else
			nth = (int) (fbdir / fpat.dth + 0.5);
	}

	// Find F/B direction in phi
	fbdir = m->gain_max_phi + 180.0;
	if (fbdir >= 360.0)
		fbdir -= 360.0;
	nph = (int) (fbdir / fpat.dph + 0.5);

	// No F/B calc. possible if no phi step at +180 from max gain
	if ((nph >= fpat.nph) || (nph < 0))
	{
		m->fb_ratio = -1;
	}
	else
	{
		// Index to gtot buffer for gain in back direction
		fbidx = nth + nph * fpat.nth;

		// Front to back ratio 
		m->fb_ratio = pow(10.0, m->gain_max / 10.0);
		m->fb_ratio /= pow(10.0, (rad_pattern[idx].gtot[fbidx] + Polarization_Factor(pol, idx, fbidx)) / 10.0);
		m->fb_ratio = 10.0 * log10(m->fb_ratio);
	}

}

// Return the index into meas_names if name matches.
int meas_name_idx(char *name, int len)
{
	int i;
	for (i = 0; meas_names[i] != NULL; i++)
	{
		if (strlen(meas_names[i]) == len && strncmp(name, meas_names[i], len) == 0)
		{
			return i;
		}
	}

	if (i == MEAS_COUNT)
	{
		BUG("meas_name_idx: Invalid name with length=%d: %s\n", len, name);
		print_backtrace(NULL);
	}

	return MEAS_COUNT;
}

// Format a string with values from the measurement.
//         m: The measurement provided by meas_calc()
//    format: The format string. For example "{mhz} {vswr}" becomes "1.8 2.0"
//            Format is %.17g, so you need about 19 chars per formatted value.
//            Available format specifiers are above in meas_names[].
//
//        out: The output buffer, eg: char out[MEAS_COUNT*25];
//     outlen: The length of the output buffer. 
void meas_format(measurement_t *m, char *format, char *out, int outlen)
{
	char *o, *p, *name;
	setlocale(LC_NUMERIC, "C");

	o = out;
	name = NULL;

	for (p = format; p && *p; p++)
	{
		if (o >= out+outlen)
		{
			BUG("meas_format: output overflow\n");
			break;
		}

		// Found a {name} so set name=p+1
		if (*p == '{')
		{
			name = p+1;
		}
		// Found close brace so populate the value.
		else if (*p == '}' && name != NULL)
		{
			int idx = meas_name_idx(name, (p-name));

			if (idx != MEAS_COUNT)
			{
				o += snprintf(o, outlen-(o-out)-1, "%.17g", m->a[idx]);
				*o = 0;
				name = NULL;
			}
		}
		// Just copy if not in a name
		else if (name == NULL)
		{
			*o = *p;
			o++;
			*o = 0;
		}
	}

	setlocale(LC_NUMERIC, orig_numeric_locale);
}

int meas_write_format(measurement_t *m, char *format, FILE *fp)
{
	char *s = NULL;
	int ret;
	int i, len, count; 
	size_t mreq;

	len = strlen(format);

	for (count = i = 0; i < len; i++)
	{
		if (format[i] == '{')
			count++;
	}

	// mreq length is 20*count of formatted strings to get all the floating point digits
	// plus the total number of chars in `format` should be more than enough.
	mreq = 20*count+len;
	mem_alloc((void**)&s, 20*count+len, __LOCATION__);

	meas_format(m, format, s, mreq-1);
	ret = fputs(s, fp);
	free_ptr((void**)&s);
	return ret;
}

// Print headers:
// Enclose the headerin the strings left and right.  For example, if 
// left and right are both "\"" then it will quote the header name.
void meas_write_header_enc(FILE *fp, char *delim, char *left, char *right)
{
	int i;

	if (left == NULL) left = "";
	if (right == NULL) right = "";

	// Print names
	for (i = 0; i < MEAS_COUNT; i++)
	{
		fprintf(fp, "%s%s%s", left, meas_names[i], right);
		if (i < MEAS_COUNT-1)
			fputs(delim, fp);
	}
	fprintf(fp, "\n");
}

void meas_write_header(FILE *fp, char *delim)
{
	meas_write_header_enc(fp, delim, "", "");
}

// Print frequency-dependent data corresponding to graphs in plot of
// frequency-dependent data.
// Enclose the data in the strings left and right.  For example, if 
// left and right are both "\"" then it will quote the value.
void meas_write_data_enc(FILE *fp, char *delim, char *left, char *right)
{
	measurement_t meas;
	int i, idx;
	setlocale(LC_NUMERIC, "C");

	if (left == NULL) left = "";
	if (right == NULL) right = "";

	for (idx = 0; idx < calc_data.steps_total; idx++)
	{
		meas_calc(&meas, idx);
		for (i = 0; i < MEAS_COUNT; i++)
		{
			fprintf(fp, "%s%.17g%s", left, meas.a[i], right);
			if (i < MEAS_COUNT-1)
				fputs(delim, fp);
		}
		fprintf(fp, "\n");
	}

	setlocale(LC_NUMERIC, orig_numeric_locale);
}

void meas_write_data(FILE *fp, char *delim)
{
	meas_write_data_enc(fp, delim, "", "");
}

