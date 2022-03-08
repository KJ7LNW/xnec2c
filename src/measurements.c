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
	[MEAS_GAIN_MAX]         =  "gain_max",
	[MEAS_GAIN_NET]         =  "gain_net",
	[MEAS_GAIN_THETA]       =  "gain_max_theta",
	[MEAS_GAIN_PHI]         =  "gain_max_phi",
	[MEAS_GAIN_VIEWER]      =  "gain_viewer",
	[MEAS_GAIN_VIEWER_NET]  =  "gain_viewer_net",
	[MEAS_FB_RATIO]         =  "fb_ratio",
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
	int mgidx = rad_pattern[idx].max_gain_idx[pol];

	// This should never happen, but please report it with your .NEC file if it does.
	//
	// It should be fixed in commit 42afbe3a3, but just in case:
	if (mgidx < 0)
	{
		printf("BUG: invalid mgidx=%d: idx=%d pol=%d fstep=%d last_step=%d freq_step=%d\n",
				mgidx, idx, pol, calc_data.last_step + 1,
			   calc_data.last_step, calc_data.freq_step);
		printf("BUG: save.fstep[%d]=%d FREQ_LOOP_STOP=%d\n", idx, save.fstep[idx], isFlagSet(FREQ_LOOP_STOP));
		mem_backtrace(rad_pattern[idx].max_gain_idx);
		return;
	}

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

	m->mhz = save.freq[idx];

	Zr = m->zreal = impedance_data.zreal[idx];
	Zi = m->zimag = impedance_data.zimag[idx];

	m->zmag = impedance_data.zmagn[idx];
	m->zphase = impedance_data.zphase[idx];

	m->vswr = (1 + gamma) / (1 - gamma);
	m->s11 = 20*log10( gamma );

	m->s11_real = creal(cs11);
	m->s11_imag = cimag(cs11);

	double net_gain_adjust = 10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );

	m->gain_max = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	m->gain_net = m->gain_max + net_gain_adjust;

	m->gain_viewer = Viewer_Gain(structure_proj_params, idx);
	m->gain_viewer_net = m->gain_viewer + net_gain_adjust;

	m->gain_max_theta = 90.0 - rad_pattern[idx].max_gain_tht[pol];
	m->gain_max_phi = rad_pattern[idx].max_gain_phi[pol];

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
		printf("meas_name_idx: Invald name with length=%d: %s\n", len, name);
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

	o = out;
	name = NULL;

	for (p = format; p && *p; p++)
	{
		if (o >= out+outlen)
		{
			printf("meas_format: output overflow\n");
			break;
		}

		// Found a {name} so set name=p+1
		if (*p == '{')
		{
			name = p+1;
		}
		// Found close brace so populate the value.
		else if (*p == '}')
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
}

int meas_write_format(measurement_t *m, char *format, FILE *fp)
{
	char *s = NULL;
	int ret;
	mem_alloc((void**)&s, strlen(format)*2, __LOCATION__);

	meas_format(m, format, s, strlen(format)*2-1);
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
}

void meas_write_data(FILE *fp, char *delim)
{
	meas_write_data_enc(fp, delim, "", "");
}

