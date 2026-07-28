#include "common.h"
#include "shared.h"

#define clog10(z) (clog(z) / log(10))

/* Interpolation method names for config and UI */
noise_temp_t *noise_temp = NULL;

const char *ant_temp_method_names[ANT_TEMP_METHOD_COUNT] = {
	[ANT_TEMP_SNAP]     = "Snap",
	[ANT_TEMP_INTERP]   = "Interpolate",
	[ANT_TEMP_FORMULA]  = "Formula",
	[ANT_TEMP_GALACTIC] = "Galactic",
};

/* DG7YBN Era 3 (2025-08-29) — [env][band]: 0=Rural 1=Residential 2=City;
 * 0=50MHz 1=144MHz 2=432MHz
 *
 * Sky: galactic average from DG7YBN Ant_Temp page
 * Earth 50/144: ITU-R P.372-16 §6 pp.98-100
 * Earth 432 Rural: URSI No.334 (Lefering/Ghent/OFCOM 2010)
 * dg7ybn.de/GT_Tables/Download/VE7BQH_Tables_White_Paper_2025_09_07.pdf */
static const double ve7bqh_sky[3]    = { 5640.0,  290.0,  27.0 };
static const double ve7bqh_mhz[3]    = {   50.0,  144.0, 432.0 };
static const double ve7bqh_earth[3][3] = {
	/* Rural */       {  29640.0,  1600.0,  760.0 },
	/* Residential */ { 100600.0,  5400.0, 1800.0 },
	/* City */        { 271000.0, 14550.0, 7900.0 },
};

/* G4CQM historical baseline (pre-2016) — single environment
 * g4cqm.co.uk (labeled "VK3UM's Calc compliance") */
#define G4CQM_NPTS 5
static const double g4cqm_mhz[G4CQM_NPTS]   = {   50.0, 144.0, 222.0,  432.0, 1296.0 };
static const double g4cqm_sky[G4CQM_NPTS]    = { 2200.0, 250.0,  70.0,   20.0,   10.0 };
static const double g4cqm_earth[G4CQM_NPTS]  = { 3000.0,1000.0, 600.0,  350.0,  290.0 };

/* VK3UM minimum quiet sky (pre-2016)
 * Doug McArthur VK3UM (SK 2016), sm2cew.com/rpc.pdf */
#define VK3UM_NPTS 6
static const double vk3um_mhz[VK3UM_NPTS] = {   50.0, 144.0, 220.0,  432.0,  900.0, 1296.0 };
static const double vk3um_sky[VK3UM_NPTS]  = { 2200.0, 250.0, 150.0,   15.0,   10.0,    5.0 };

/* VE7BQH Base Reference (pre-2018, frozen)
 * Original VE7BQH estimates before ITU-R P.372 adoption.
 * Used as normalization baseline in F5FOD AGTC-JS. */
#define BASE_REF_NPTS 3
static const double base_ref_mhz[BASE_REF_NPTS]   = {   50.0, 144.0, 432.0 };
static const double base_ref_sky[BASE_REF_NPTS]    = { 1700.0, 200.0,  20.0 };
static const double base_ref_earth[BASE_REF_NPTS]  = { 9000.0,1000.0, 350.0 };

/* Synthesized sky models — shared frequency grid (2026-04 analysis)
 * Frequencies from NTIA/ITS measurement campaigns:
 *   112.5, 221.5, 401 MHz: TR-11-478 (Wepman & Sanders, 2011)
 *   137.5, 402.5, 761 MHz: TR-02-390 (Achatz & Dalke, 2001)
 * Values derived in debug/plot_proposed_models.py */
#define SYNTH_NPTS 17
static const double synth_mhz[SYNTH_NPTS] = {
	50.0, 100.0, 112.5, 137.5, 144.0, 220.0, 222.0, 250.0,
	401.0, 402.5, 408.0, 432.0, 500.0, 761.0, 900.0, 1000.0, 1296.0
};
static const double synth_avg_sky[SYNTH_NPTS] = {
	5640.0, 1139.0, 830.0, 480.0, 290.0, 138.0, 131.0, 97.0,
	28.0, 27.0, 25.0, 27.0, 22.0, 12.0, 11.0, 11.0, 10.0
};
static const double synth_min_sky[SYNTH_NPTS] = {
	2200.0, 480.0, 370.0, 280.0, 250.0, 70.0, 70.0, 56.0,
	22.0, 21.0, 21.0, 20.0, 17.0, 13.0, 12.0, 12.0, 10.0
};

/* Sky model registry (ascending year) */
const ant_temp_model_t sky_models[ANT_TEMP_SKY_COUNT] = {
	[ANT_TEMP_SKY_G4CQM_MIN] = {
		.name = "G4CQM Min Quiet (pre-2016)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = g4cqm_mhz, .temp = g4cqm_sky, .freq_count = G4CQM_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_VK3UM_MIN] = {
		.name = "VK3UM Min Quiet (pre-2016)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = vk3um_mhz, .temp = vk3um_sky, .freq_count = VK3UM_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_BASE_REF] = {
		.name = "VE7BQH Base Ref (pre-2018)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = base_ref_mhz, .temp = base_ref_sky, .freq_count = BASE_REF_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_GALACTIC] = {
		.name = "Galactic P.372 (2022)",
		.method = ANT_TEMP_GALACTIC,
		.c = 52.0, .d = 23.0,
		.freq_break = 100.0, .power_exponent = -2.75, .power_offset = 2.7,

	},
	[ANT_TEMP_SKY_DG7YBN_AVG] = {
		.name = "DG7YBN Galactic Avg (2025)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = ve7bqh_mhz, .temp = ve7bqh_sky, .freq_count = 3,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_SYNTH_AVG] = {
		.name = "Synth Practical Avg (2026)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = synth_mhz, .temp = synth_avg_sky, .freq_count = SYNTH_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_SYNTH_MIN] = {
		.name = "Synth Min Quiet (2026)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = synth_mhz, .temp = synth_min_sky, .freq_count = SYNTH_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_SKY_CUSTOM] = {
		.name = "Custom",
		/* Other fields unused: ant_temp_resolve() bypasses ant_temp_eval()
		 * for Custom and reads rc_config.ant_temp_custom_t_sky directly. */
	},
};

/* Earth model registry (ascending year) */
const ant_temp_model_t earth_models[ANT_TEMP_EARTH_COUNT] = {
	[ANT_TEMP_EARTH_ITU_BUSINESS] = {
		.name = "ITU-R Business (1974)",
		.method = ANT_TEMP_FORMULA,
		.c = 76.8, .d = 27.7,

		/* Formula is intrinsic; no user-selectable interp override */
	},
	[ANT_TEMP_EARTH_ITU_RESIDENTIAL] = {
		.name = "ITU-R Residential (1974)",
		.method = ANT_TEMP_FORMULA,
		.c = 72.5, .d = 27.7,

		/* Formula is intrinsic; no user-selectable interp override */
	},
	[ANT_TEMP_EARTH_ITU_RURAL] = {
		.name = "ITU-R Rural (1974)",
		.method = ANT_TEMP_FORMULA,
		.c = 67.2, .d = 27.7,

		/* Formula is intrinsic; no user-selectable interp override */
	},
	[ANT_TEMP_EARTH_ITU_QUIET_RURAL] = {
		.name = "ITU-R Quiet Rural (1974)",
		.method = ANT_TEMP_FORMULA,
		.c = 53.6, .d = 28.6,

		/* Formula is intrinsic; no user-selectable interp override */
	},
	[ANT_TEMP_EARTH_G4CQM_MIXED] = {
		.name = "G4CQM Mixed (pre-2016)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = g4cqm_mhz, .temp = g4cqm_earth, .freq_count = G4CQM_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_EARTH_BASE_REF] = {
		.name = "VE7BQH Base Ref (pre-2018)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = base_ref_mhz, .temp = base_ref_earth, .freq_count = BASE_REF_NPTS,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_EARTH_DG7YBN_RURAL] = {
		.name = "DG7YBN Rural (2025)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = ve7bqh_mhz, .temp = ve7bqh_earth[0], .freq_count = 3,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_EARTH_DG7YBN_RESIDENTIAL] = {
		.name = "DG7YBN Residential (2025)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = ve7bqh_mhz, .temp = ve7bqh_earth[1], .freq_count = 3,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_EARTH_DG7YBN_CITY] = {
		.name = "DG7YBN City (2025)",
		.method = ANT_TEMP_INTERP,
		.freq_mhz = ve7bqh_mhz, .temp = ve7bqh_earth[2], .freq_count = 3,

		.valid_interp = ANT_TEMP_TABLE_METHODS,
	},
	[ANT_TEMP_EARTH_CUSTOM] = {
		.name = "Custom",
		/* Other fields unused: ant_temp_resolve() bypasses ant_temp_eval()
		 * for Custom and reads rc_config.ant_temp_custom_t_earth directly. */
	},
};

/**
 * ant_temp_log_interp() - log-log interpolation on a table
 * @freq_mhz:  operating frequency in MHz
 * @tbl_mhz:   anchor frequency array
 * @tbl_val:   anchor value array
 * @n:         number of table entries
 *
 * Clamps to endpoint values for out-of-range frequencies.
 * Returns interpolated value.
 */
static double ant_temp_log_interp(double freq_mhz,
	const double *tbl_mhz, const double *tbl_val, int n)
{
	double lf = log10(freq_mhz);

	/* Clamp below table */
	if (lf <= log10(tbl_mhz[0]))
		return tbl_val[0];

	/* Clamp above table */
	if (lf >= log10(tbl_mhz[n - 1]))
		return tbl_val[n - 1];

	/* Find bracketing interval */
	int i = 0;
	while (i < n - 2 && log10(tbl_mhz[i + 1]) < lf)
		i++;

	double t = (lf - log10(tbl_mhz[i]))
		/ (log10(tbl_mhz[i + 1]) - log10(tbl_mhz[i]));

	return pow(10.0, log10(tbl_val[i])
		+ t * (log10(tbl_val[i + 1]) - log10(tbl_val[i])));
}

/**
 * ant_temp_eval() - evaluate a single noise model at a frequency
 * @m:       model descriptor (sky or earth)
 * @freq:    operating frequency in MHz
 * @interp:  user-selected interpolation override (ANT_TEMP_SNAP or ANT_TEMP_INTERP);
 *           ignored when model method is FORMULA or GALACTIC
 *
 * Returns temperature in Kelvin, or -1.0 on error.
 */
static double ant_temp_eval(const ant_temp_model_t *m, double freq, int interp)
{
	/* Defense-in-depth: log10(freq) is undefined for freq <= 0 */
	if (freq <= 0.0)
		return -1.0;

	/* Determine effective method: formula/galactic models override interp;
	 * table models honor the user's snap/interp choice */
	ant_temp_method_t method = m->method;
	if (method == ANT_TEMP_SNAP || method == ANT_TEMP_INTERP)
	{
		if (interp == ANT_TEMP_SNAP || interp == ANT_TEMP_INTERP)
			method = (ant_temp_method_t)interp;
	}

	switch (method)
	{
	case ANT_TEMP_SNAP:
		{
			int b = 0;
			double best = fabs(freq - m->freq_mhz[0]);
			for (int i = 1; i < m->freq_count; i++)
			{
				double dist = fabs(freq - m->freq_mhz[i]);
				if (dist < best)
				{
					best = dist;
					b = i;
				}
			}
			return m->temp[b];
		}

	case ANT_TEMP_INTERP:
		return ant_temp_log_interp(freq, m->freq_mhz, m->temp, m->freq_count);

	case ANT_TEMP_FORMULA:
		{
			/* F_am = c - d·log10(f), T = 290·(10^(F/10) - 1) */
			double fam = m->c - m->d * log10(freq);
			double t = 290.0 * (pow(10.0, fam / 10.0) - 1.0);

			/* Thermodynamic floor: earth brightness cannot fall below
			 * ground thermal emission (~290 K) */
			if (t < 290.0)
				t = 290.0;
			return t;
		}

	case ANT_TEMP_GALACTIC:
		{
			if (freq <= m->freq_break)
			{
				/* P.372-16 eq.13: log-linear, same form as man-made */
				double fam = m->c - m->d * log10(freq);
				return 290.0 * pow(10.0, fam / 10.0);
			}
			else
			{
				/* P.372-16 eq.14: power-law scaling from break frequency */
				double fam_break = m->c - m->d * log10(m->freq_break);
				double t_break = 290.0 * pow(10.0, fam_break / 10.0);
				return (t_break - m->power_offset)
					* pow(freq / m->freq_break, m->power_exponent)
					+ m->power_offset;
			}
		}

	default:
		return -1.0;
	}
}

/**
 * ant_temp_resolve() - resolve sky and earth brightness temperatures
 * @freq_mhz:  operating frequency in MHz
 * @sky:        ant_temp_sky_t index
 * @earth:      ant_temp_earth_t index
 * @interp:     ant_temp_method_t value (user override for table models)
 * @t_sky:      output sky brightness temperature (K)
 * @t_earth:    output earth/man-made noise temperature (K)
 *
 * Returns 0 on success, -1 if index or frequency is invalid.
 */
int ant_temp_resolve(double freq_mhz, int sky, int earth, int interp,
	double *t_sky, double *t_earth)
{
	if (sky < 0 || sky >= ANT_TEMP_SKY_COUNT)
		return -1;
	if (earth < 0 || earth >= ANT_TEMP_EARTH_COUNT)
		return -1;

	/* Custom models bypass table evaluation; return rc_config constants */
	double ts = (sky == ANT_TEMP_SKY_CUSTOM)
		? rc_config.ant_temp_custom_t_sky
		: ant_temp_eval(&sky_models[sky], freq_mhz, interp);
	double te = (earth == ANT_TEMP_EARTH_CUSTOM)
		? rc_config.ant_temp_custom_t_earth
		: ant_temp_eval(&earth_models[earth], freq_mhz, interp);

	if (ts < 0.0 || te < 0.0)
		return -1;

	*t_sky = ts;
	*t_earth = te;

	return 0;
}

/**
 * ant_temp_fill_fstep() - Fill per-fstep noise temperature table
 * @fstep: frequency step index
 *
 * Iterates all (sky × method) and (earth × method) combinations via the
 * static ant_temp_eval().  Custom model slots store sentinel -1.0; the
 * parent substitutes rc_config.ant_temp_custom_t_* at display time.
 * Called in the child process after NEC engine completes.
 */
  void
ant_temp_fill_fstep(int fstep)
{
  int sky, earth, method;
  double freq;

  if( noise_temp == NULL || fstep < 0 )
    return;

  /* Child receives frequency via pipe into calc_data.freq_mhz;
   * save.freq[] is parent-only and uninitialized in the child. */
  freq = calc_data.freq_mhz;

  for( sky = 0; sky < ANT_TEMP_SKY_COUNT; sky++ )
  {
    for( method = 0; method < ANT_TEMP_METHOD_COUNT; method++ )
    {
      if( sky == ANT_TEMP_SKY_CUSTOM )
        noise_temp[fstep].t_sky[sky][method] = -1.0;
      else
        noise_temp[fstep].t_sky[sky][method] =
          ant_temp_eval(&sky_models[sky], freq, method);
    }
  }

  for( earth = 0; earth < ANT_TEMP_EARTH_COUNT; earth++ )
  {
    for( method = 0; method < ANT_TEMP_METHOD_COUNT; method++ )
    {
      if( earth == ANT_TEMP_EARTH_CUSTOM )
        noise_temp[fstep].t_earth[earth][method] = -1.0;
      else
        noise_temp[fstep].t_earth[earth][method] =
          ant_temp_eval(&earth_models[earth], freq, method);
    }
  }
}

/**
 * noise_temp_resolve() - Resolve per-fstep noise temperatures from table
 * @fstep:   frequency step index
 * @t_sky:   output sky noise temperature (K); set to 0.0 when table not allocated
 * @t_earth: output earth noise temperature (K); set to 0.0 when table not allocated
 *
 * Reads rc_config sky/earth model indices and interpolation method, looks
 * up precomputed values in noise_temp[fstep], and substitutes
 * rc_config.ant_temp_custom_t_* for sentinel -1.0 (custom model slots).
 * Called only when IS_NOISE_MODE(rc_config.gain_style) is true.
 */
  void
noise_temp_resolve(int fstep, double *t_sky, double *t_earth)
{
  *t_sky   = 0.0;
  *t_earth = 0.0;

  if( noise_temp == NULL )
  {
    static gboolean warned = FALSE;
    if( !warned )
    {
      warned = TRUE;
      pr_warn("noise_temp_resolve: noise_temp table not allocated; t_sky/t_earth defaulting to 0\n");
    }
    return;
  }

  int sky   = rc_config.ant_temp_sky;
  int earth = rc_config.ant_temp_earth;
  int meth  = rc_config.ant_temp_interp;

  double ts = noise_temp[fstep].t_sky[sky][meth];
  double te = noise_temp[fstep].t_earth[earth][meth];

  *t_sky   = (ts < 0.0) ? rc_config.ant_temp_custom_t_sky   : ts;
  *t_earth = (te < 0.0) ? rc_config.ant_temp_custom_t_earth : te;
}

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
	[MEAS_FR_RATIO]         =  "fr_ratio",
	[MEAS_AGT]              =  "agt",
	[MEAS_AGT_EFFICIENCY]   =  "agt_efficiency",
	[MEAS_GAIN_DEV_PX]     =  "gain_dev_px",
	[MEAS_GAIN_DEV_NX]     =  "gain_dev_nx",
	[MEAS_GAIN_DEV_PY]     =  "gain_dev_py",
	[MEAS_GAIN_DEV_NY]     =  "gain_dev_ny",
	[MEAS_GAIN_DEV_PZ]     =  "gain_dev_pz",
	[MEAS_GAIN_DEV_NZ]     =  "gain_dev_nz",
	[MEAS_ANT_TEMP]        =  "ant_temp",
	[MEAS_ANT_TEMP_TOT]   =  "ant_temp_tot",
	[MEAS_GT]              =  "gt",
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
	[MEAS_FR_RATIO]         =  "F/R Ratio",
	[MEAS_AGT]              =  "Average Gain",
	[MEAS_AGT_EFFICIENCY]   =  "AGT Efficiency",
	[MEAS_GAIN_DEV_PX]     =  "Gain Dev +X",
	[MEAS_GAIN_DEV_NX]     =  "Gain Dev −X",
	[MEAS_GAIN_DEV_PY]     =  "Gain Dev +Y",
	[MEAS_GAIN_DEV_NY]     =  "Gain Dev −Y",
	[MEAS_GAIN_DEV_PZ]     =  "Gain Dev +Z",
	[MEAS_GAIN_DEV_NZ]     =  "Gain Dev −Z",
	[MEAS_ANT_TEMP]        =  "Ta",
	[MEAS_ANT_TEMP_TOT]   =  "TA",
	[MEAS_GT]              =  "G/Ta",
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
	[MEAS_FR_RATIO]         =  "Front to worst-case rear-lobe ratio in dB (rear = >90 deg off boresight)",
	[MEAS_AGT]              =  "Average Gain Test: linear ratio, ~1.0 in free space or ~2.0 over perfect ground for a lossless model (requires RP card averaging A=1 or A=2)",
	[MEAS_AGT_EFFICIENCY]   =  "Average Gain Test result as % of theoretical maximum gain for the covered solid angle",
	[MEAS_GAIN_DEV_PX]     =  "Angular deviation of peak gain from +X axis (degrees)",
	[MEAS_GAIN_DEV_NX]     =  "Angular deviation of peak gain from -X axis (degrees)",
	[MEAS_GAIN_DEV_PY]     =  "Angular deviation of peak gain from +Y axis (degrees)",
	[MEAS_GAIN_DEV_NY]     =  "Angular deviation of peak gain from -Y axis (degrees)",
	[MEAS_GAIN_DEV_PZ]     =  "Angular deviation of peak gain from +Z axis (degrees)",
	[MEAS_GAIN_DEV_NZ]     =  "Angular deviation of peak gain from -Z axis (degrees)",
	[MEAS_ANT_TEMP]        =  "Antenna noise temperature Ta from sky/earth brightness (K)",
	[MEAS_ANT_TEMP_TOT]   =  "Total system noise temperature TA including ohmic loss (K)",
	[MEAS_GT]              =  "Gain-to-antenna-temperature ratio (dB), excludes loss",
	[MEAS_COUNT]            =  NULL
};

// Calculates measurement data.
// m:   a measurement_t structure to fill
// idx: the index into the calculated data structures.
//
// Warning: idx is not checked to make sure it is valid.
static void _meas_calc(measurement_t *m, int idx, int port)
{
	int pol = calc_data.pol_type;
	int mgidx;
	int i;

	int fbidx, nth, nph;

	// Start with invalidated values (-1) in case something cannot be
	// calculated due to NEC state or card configuration:
	for (i = 0; i < MEAS_COUNT; i++)
		m->a[i] = -1;

	m->mhz = save.freq[idx];

	/* Feedpoint-less excitations leave the per-port impedance buffers
	 * NULL, so impedance-derived fields keep the invalid value (-1)
	 * and the mismatch adjustment stays unset. */
	double net_gain_adjust = NAN;

	if (fpat_has_feedpoint())
	{
		/* Single-port consumers read the caller-selected excitation port. */
		impedance_data_t *imp = &impedance_data[idx];

		double Zr, Zi, Zo = calc_data.zo;

		double zrpro2 = imp->zreal[port] + calc_data.zo;
		zrpro2 *= zrpro2;

		double zrmro2 = imp->zreal[port] - calc_data.zo;
		zrmro2 *= zrmro2;

		double zimag2 = imp->zimag[port] * imp->zimag[port];
		double gamma = sqrt( (zrmro2 + zimag2) / (zrpro2 + zimag2) );

		double complex z_load = imp->zreal[port] + I*imp->zimag[port];
		double complex cgamma = (z_load-Zo) / (z_load+Zo);

		double complex cs11 = 20*clog10( cgamma );

		Zr = m->zreal = imp->zreal[port];
		Zi = m->zimag = imp->zimag[port];

		m->zmag = imp->zmagn[port];
		m->zphase = imp->zphase[port];

		m->vswr = (1 + gamma) / (1 - gamma);
		m->s11 = 20*log10( gamma );

		// Note that creal(cs11) == creal(20*clog10( cgamma )) == 20*log10(cabs(cgamma)):
		m->s11_real = creal(cs11);
		m->s11_imag = cimag(cs11);
		m->s11_ang = cang(cgamma);

		net_gain_adjust = 10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );
	}

	// Everything below here is dependent on the radiation pattern
	// having been calculated, so fields will remain invalid (-1).
	// The rad_pattern outer array is always allocated; ENABLE_RDPAT is
	// the authoritative signal that its per-fstep gain sub-buffers exist.
	if (isFlagClear(ENABLE_RDPAT))
		return;

	/* Validate pol before indexing into NUM_POL-sized arrays */
	if (pol < 0 || pol >= NUM_POL)
	{
		BUG("BUG: pol=%d out of range [0,%d): idx=%d freq_step=%d thr=%lu\n",
			pol, NUM_POL, idx, calc_data.freq_step,
			(unsigned long)pthread_self());
		return;
	}

	mgidx = rad_pattern[idx].max_gain_idx[pol];

	// This should never happen, but please report it with your .NEC file if it does.
	//
	// It should be fixed in commit 42afbe3a3, but just in case:
	int nthph = fpat.nth * fpat.nph;
	if (mgidx < 0 || mgidx >= nthph)
	{
		BUG("BUG: mgidx=%d out of range [0,%d): idx=%d pol=%d freq_step=%d thr=%lu\n",
			mgidx, nthph, idx, pol, calc_data.freq_step,
			(unsigned long)pthread_self());
		BUG("BUG: save.fstep[%d]=%d FREQ_LOOP_STOP=%d FREQ_LOOP_DONE=%d\n",
			idx, save.fstep[idx],
			isFlagSet(FREQ_LOOP_STOP), isFlagSet(FREQ_LOOP_DONE));
		BUG("BUG: rad_pattern[%d]: efficiency=%.6e gtot=%p max_gain_idx=%p\n",
			idx, rad_pattern[idx].efficiency,
			(void *)rad_pattern[idx].gtot,
			(void *)rad_pattern[idx].max_gain_idx);
		BUG("BUG: max_gain_idx[0]=%d [1]=%d [2]=%d\n",
			rad_pattern[idx].max_gain_idx[0],
			rad_pattern[idx].max_gain_idx[1],
			rad_pattern[idx].max_gain_idx[2]);
		mem_backtrace(rad_pattern[idx].max_gain_idx);
		return;
	}
	m->gain_max = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
	if (fpat_has_feedpoint())
		m->gain_net = m->gain_max + net_gain_adjust;

	m->gain_viewer = Viewer_Gain(structure_view, idx);
	if (fpat_has_feedpoint())
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

	/* Antenna temperature: spherical integration of gain-weighted brightness */
	if (fpat.dth != 0.0 && fpat.dph != 0.0 && fpat.nth > 0 && fpat.nph > 0)
	{
		double t_sky, t_earth;
		if (ant_temp_resolve(save.freq[idx],
				rc_config.ant_temp_sky, rc_config.ant_temp_earth,
				rc_config.ant_temp_interp,
				&t_sky, &t_earth) == 0)
		{
			double t_pattern = 0.0;
			double dth_rad = fpat.dth * M_PI / 180.0;
			double dph_rad = fpat.dph * M_PI / 180.0;

			double tht_mg = rad_pattern[idx].max_gain_tht[pol] * M_PI / 180.0;
			double phi_mg = rad_pattern[idx].max_gain_phi[pol] * M_PI / 180.0;
			double elev_rad = rc_config.ant_temp_elevation * M_PI / 180.0;

			for (int iph = 0; iph < fpat.nph; iph++)
			{
				double phi_rad = (fpat.phis + iph * fpat.dph) * M_PI / 180.0;
				for (int ith = 0; ith < fpat.nth; ith++)
				{
					int cell = ith + iph * fpat.nth;
					double tht_rad = (fpat.thets + ith * fpat.dth) * M_PI / 180.0;
					double z_w = ant_temp_z_world(tht_rad, phi_rad,
							tht_mg, phi_mg, elev_rad);
					double g_dbi = rad_pattern[idx].gtot[cell]
							+ Polarization_Factor(pol, idx, cell);
					double g_lin = pow(10.0, g_dbi / 10.0);
					double t_bright = (z_w > ANT_TEMP_Z_EPSILON) ? t_sky : t_earth;

					t_pattern += g_lin * t_bright * sin(tht_rad) * dth_rad * dph_rad;
				}
			}

			m->ant_temp     = t_pattern / (4.0 * M_PI);
			/* NEC2-computed ohmic loss temperature from radiation efficiency */
			double eta = rad_pattern[idx].efficiency;
			double t_loss_nec2 = (eta > 0.0 && eta < 1.0)
				? 290.0 * (1.0 - eta) / eta : 0.0;

			m->ant_temp_tot = m->ant_temp + t_loss_nec2;

			/* G/Ta ratio — figure of merit using antenna temperature only,
			 * excluding ohmic and external losses */
			if (m->ant_temp > 0.0)
			{
				int mg_cell = rad_pattern[idx].max_gain_idx[pol];
				double gmax_dbi = rad_pattern[idx].gtot[mg_cell]
						+ Polarization_Factor(pol, idx, mg_cell);
				m->gt = gmax_dbi - 10.0 * log10(m->ant_temp);
			}
		}
	}

	/* Back direction: antipode in free space; over a ground plane the antipode
	   elevation is below the sampled horizon, so keep the max-gain elevation and
	   swing azimuth by 180 (rear lobe at the same take-off angle). */
	if (fpat.nph < 2)
	{
		// no distinct rear azimuth is sampled
		m->fb_ratio = -1;
	}
	else
	{
		gboolean over_ground = (gnd.ksymp == 2) && (gnd.ifar != 1);
		double back_tht;
		if (over_ground)
			back_tht = rad_pattern[idx].max_gain_tht[pol];
		else
			back_tht = 180.0 - rad_pattern[idx].max_gain_tht[pol];

		nth = fpat_theta_cell(back_tht);
		nph = fpat_phi_cell(m->gain_max_phi + 180.0);
		fbidx = nth + nph * fpat.nth;

		m->fb_ratio = pow(10.0, m->gain_max / 10.0);
		m->fb_ratio /= pow(10.0, (rad_pattern[idx].gtot[fbidx]
						+ Polarization_Factor(pol, idx, fbidx)) / 10.0);
		m->fb_ratio = 10.0 * log10(m->fb_ratio);

		/* Front-to-Rear: worst-case (max gain) lobe anywhere more than
		 * 90 degrees off boresight AZIMUTH, at the SAME take-off theta as
		 * the main lobe -- i.e. the "rear half-plane" as conventionally
		 * defined by AO/EZNEC and used in K6STI's published Yagi work
		 * ("F/R is the ratio of forward power to that of the worst
		 * backlobe in the rear half-plane"). A full 3D theta+phi search
		 * is more thorough but no longer matches this standard metric,
		 * since it can find weaker-rejection lobes off at other elevation
		 * angles that the conventional definition deliberately excludes. */
		{
			double rear_gain_max = -1e30;
			double main_phi = m->gain_max_phi;

			for (int iph = 0; iph < fpat.nph; iph++)
			{
				double phi_cell = fpat.phis + iph * fpat.dph;
				double dphi = fmod(fabs(phi_cell - main_phi), 360.0);
				if (dphi > 180.0)
					dphi = 360.0 - dphi;

				/* "Rear" = more than 90 degrees off boresight azimuth */
				if (dphi <= 90.0)
					continue;

				int cell = nth + iph * fpat.nth;
				double g = rad_pattern[idx].gtot[cell]
						+ Polarization_Factor(pol, idx, cell);
				if (g > rear_gain_max)
					rear_gain_max = g;
			}

			if (rear_gain_max > -1e30)
			{
				m->fr_ratio = pow(10.0, m->gain_max / 10.0);
				m->fr_ratio /= pow(10.0, rear_gain_max / 10.0);
				m->fr_ratio = 10.0 * log10(m->fr_ratio);
			}
			else
				m->fr_ratio = -1;
		}

		/* Average Gain Test: only meaningful if the RP card requested
		 * averaging (A=1 or A=2) for this run; sentinel -1 otherwise so
		 * CSV/plot readers can tell "not computed" from a real 0.0. */
		if (rad_pattern[idx].agt_valid)
		{
			m->agt            = rad_pattern[idx].agt_ratio;
			m->agt_efficiency = rad_pattern[idx].agt_efficiency_pct;
		}
		else
		{
			m->agt            = -1;
			m->agt_efficiency = -1;
		}
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

/**
 * meas_calc - calculate measurement data with freq_data_lock held
 * @m:   measurement_t structure to fill
 * @idx: index into calculated data structures (not bounds-checked)
 */
/**
 * ant_temp_sky_name() - return display name of a sky noise model
 * @idx: ant_temp_sky_t index
 */
const char *ant_temp_sky_name(int idx)
{
	if (idx < 0 || idx >= ANT_TEMP_SKY_COUNT)
		return "Unknown";
	return sky_models[idx].name;
}

/**
 * ant_temp_earth_name() - return display name of an earth noise model
 * @idx: ant_temp_earth_t index
 */
const char *ant_temp_earth_name(int idx)
{
	if (idx < 0 || idx >= ANT_TEMP_EARTH_COUNT)
		return "Unknown";
	return earth_models[idx].name;
}

void meas_calc(measurement_t *m, int idx, int port)
{
	g_rec_mutex_lock(&freq_data_lock);
	_meas_calc(m, idx, port);
	g_rec_mutex_unlock(&freq_data_lock);
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
	mem_alloc(&s, mreq);

	meas_format(m, format, s, mreq-1);
	ret = fputs(s, fp);
	mem_free(&s);
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
		meas_calc(&meas, idx, calc_data.ex_port);
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

