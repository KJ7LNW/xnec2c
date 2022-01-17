#define _GNU_SOURCE
#include <complex.h> // for clog10

#include "common.h"
#include "shared.h"


// Ordering doesn't matter here because we use [] indexes,
// but it still would be nice to keep the same order as the
// enum MEASUREMENT_INDEXES.
const char *meas_names[] = {
	[MEAS_MHZ]          =  "mhz",
	[MEAS_ZREAL]        =  "zreal",
	[MEAS_ZIMAG]        =  "zimag",
	[MEAS_ZMAG]         =  "zmag",
	[MEAS_ZPHASE]       =  "zphase",
	[MEAS_VSWR]         =  "vswr",
	[MEAS_S11]          =  "s11",
	[MEAS_S11_REAL]     =  "s11_real",
	[MEAS_S11_IMAG]     =  "s11_imag",
	[MEAS_GAIN_MAX]     =  "gain_max",
	[MEAS_GAIN_NET]     =  "gain_net",
	[MEAS_GAIN_THETA]   =  "gain_max_theta",
	[MEAS_GAIN_PHI]     =  "gain_max_phi",
	[MEAS_GAIN_VIEWER]  =  "gain_viewer",
	[MEAS_FB_RATIO]     =  "fb_ratio",
	[MEAS_COUNT]        =  NULL
};

void meas_calc(measurement_t *m, int idx)
{
	int pol = calc_data.pol_type;
	int mgidx = rad_pattern[idx].max_gain_idx[pol];

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

	double gmax = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);

	m->mhz = save.freq[idx];

	Zr = m->zreal = impedance_data.zreal[idx];
	Zi = m->zimag = impedance_data.zimag[idx];

	m->zmag = impedance_data.zmagn[idx];
	m->zphase = impedance_data.zphase[idx];

	m->vswr = (1 + gamma) / (1 - gamma);
	m->s11 = 20*log10( gamma );
	cs11 = 20*clog10( cgamma );

	m->s11_real = creal(cs11);
	m->s11_imag = cimag(cs11);

	m->fb_ratio = rad_pattern[idx].fbratio;

	m->gain_max = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	m->gain_net = gmax + 10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );

	m->gain_max_theta = 90.0 - rad_pattern[idx].max_gain_tht[pol];
	m->gain_max_phi = rad_pattern[idx].max_gain_phi[pol];

	m->gain_viewer = Viewer_Gain( structure_proj_params, idx );
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


void meas_format(measurement_t *m, char *s, char *out, int outlen)
{
	char *o, *p, *name;

	o = out;
	p = s;
	name = NULL;

	for (p = s; p && *p; p++)
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

