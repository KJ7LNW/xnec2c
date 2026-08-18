/*
 *  Unit tests for the Touchstone S-parameter export column semantics.
 *
 *  Covers github issue #80, that the S21 dB column is a voltage-referenced
 *  magnitude so |S21|^2 recovers a power ratio, and github issue #93, that
 *  S21 is referenced to the power available at port 1 so it carries
 *  realized gain rather than raw NEC gain.
 *
 *  Expectations come from the Touchstone File Format Specification: the
 *  port waves are a = (V + R*I)/(2*sqrt(R)) and b = (V - R*I)/(2*sqrt(R)),
 *  S21 is b2/a1 evaluated at a2 = 0, and the "DB" option-line keyword
 *  selects 20*log10|S| for every magnitude column.  NEC normalizes gain to
 *  the power the feedpoint accepts, which is the available power reduced
 *  by the mismatch factor 1 - |S11|^2.
 *
 *  Copyright (C) 2026 eWheeler, Inc. <https://www.linuxglobal.com/>
 */

#define _GNU_SOURCE

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "shared.h"
#include "measurements.h"
#include "touchstone.h"

#include "touchstone_test_stubs.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Emitted values round-trip through "%.17g", so the only spread is the
 * rounding difference between the production mismatch expression and the
 * complex-arithmetic form this test derives independently. */
#define TOL 1e-9

#define ASSERT_NEAR(actual, expected, tol, msg) \
	do { \
		tests_run++; \
		double _a = (actual), _e = (expected), _t = (tol); \
		if (fabs(_a - _e) <= _t) { \
			tests_passed++; \
		} \
		else { \
			tests_failed++; \
			fprintf(stderr, "FAIL %s:%d: %s\n" \
				"  expected: %.10g  actual: %.10g  diff: %.10g  tol: %.10g\n", \
				__FILE__, __LINE__, (msg), _e, _a, fabs(_a - _e), _t); \
		} \
	} while (0)

#define ASSERT_TRUE(cond, msg) \
	do { \
		tests_run++; \
		if ((cond)) { \
			tests_passed++; \
		} \
		else { \
			tests_failed++; \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
		} \
	} while (0)

/*------------------------------------------------------------------------*/

/* A Touchstone data line for an n-port carries one frequency column plus a
 * magnitude and angle pair for every entry of the n-by-n matrix. */
#define TOUCHSTONE_COLUMNS(ports)  (1 + 2 * (ports) * (ports))
#define TOUCHSTONE_S1P_COLUMNS     TOUCHSTONE_COLUMNS(1)
#define TOUCHSTONE_S2P_COLUMNS     TOUCHSTONE_COLUMNS(2)

/* Column order of a two-port data line: S11, S21, S12, S22. */
enum
{
	COL_MHZ,
	COL_S11_DB, COL_S11_ANG,
	COL_S21_DB, COL_S21_ANG,
	COL_S12_DB, COL_S12_ANG,
	COL_S22_DB, COL_S22_ANG,
};

/* S22 stands in for a matched far-field port. */
#define S22_DB  (-100.0)

#define TEST_MHZ  146.52

/* One sampled pattern cell keeps the max-gain direction unambiguous and
 * leaves the front-to-back and pattern integrals out of the comparison. */
#define TEST_NTH  1
#define TEST_NPH  1

typedef struct
{
	const char *name;
	double zo;          /* reference impedance of the file */
	double zreal;       /* feedpoint resistance */
	double zimag;       /* feedpoint reactance */
	double gtot_dbi;    /* NEC total gain at the max-gain cell */
	double viewer_dbi;  /* gain toward the structure viewer */
} touchstone_case_t;

/* Reflection spans a matched port, resistive steps either side of the
 * reference, reactive mismatch, a severe mismatch, and a reference
 * impedance other than 50 ohm. */
static const touchstone_case_t cases[] = {
	{ "matched",           50.0,  50.0,   0.0,   8.5,   6.0 },
	{ "resistive 2:1 up",  50.0, 100.0,   0.0,  12.0,   9.0 },
	{ "resistive 2:1 dn",  50.0,  25.0,   0.0,   2.0,  -3.0 },
	{ "reactive",          50.0,  35.0,  40.0,  14.2,  11.1 },
	{ "severe mismatch",   50.0,  10.0, -60.0,   5.0,   1.0 },
	{ "75 ohm reference",  75.0,  50.0,  10.0,   0.0,  -1.5 },
};

/*------------------------------------------------------------------------*/

/* Backing store for the globals _meas_calc() reads. */
static double test_freq[1];
static char test_fstep[1];
static double test_zreal[1], test_zimag[1], test_zmagn[1], test_zphase[1];
static double test_gtot[TEST_NTH * TEST_NPH];
static double test_max_gain_tht[NUM_POL], test_max_gain_phi[NUM_POL];
static int test_max_gain_idx[NUM_POL];
static impedance_data_t test_impedance[1];
static rad_pattern_t test_rad_pattern[1];

/**
 * setup_globals() - point the NEC result globals at one synthetic case
 * @tc: case supplying feedpoint impedance and gain
 */
static void setup_globals(const touchstone_case_t *tc)
{
	test_freq[0] = TEST_MHZ;
	test_fstep[0] = 1;

	test_zreal[0] = tc->zreal;
	test_zimag[0] = tc->zimag;
	test_zmagn[0] = hypot(tc->zreal, tc->zimag);
	test_zphase[0] = atan2(tc->zimag, tc->zreal) * 180.0 / M_PI;

	test_impedance[0].zreal = test_zreal;
	test_impedance[0].zimag = test_zimag;
	test_impedance[0].zmagn = test_zmagn;
	test_impedance[0].zphase = test_zphase;

	test_gtot[0] = tc->gtot_dbi;
	test_max_gain_idx[POL_TOTAL] = 0;
	test_max_gain_tht[POL_TOTAL] = 90.0;
	test_max_gain_phi[POL_TOTAL] = 0.0;

	test_rad_pattern[0].gtot = test_gtot;
	test_rad_pattern[0].max_gain_idx = test_max_gain_idx;
	test_rad_pattern[0].max_gain_tht = test_max_gain_tht;
	test_rad_pattern[0].max_gain_phi = test_max_gain_phi;
	test_rad_pattern[0].efficiency = 1.0;

	impedance_data = test_impedance;
	rad_pattern = test_rad_pattern;
	save.freq = test_freq;
	save.fstep = test_fstep;

	calc_data.zo = tc->zo;
	calc_data.pol_type = POL_TOTAL;
	calc_data.steps_total = 1;
	calc_data.freq_step = 0;
	calc_data.ex_port = 0;

	/* Applied-E voltage source: the excitation that defines a feedpoint */
	fpat.ixtyp = 0;
	fpat.nth = TEST_NTH;
	fpat.nph = TEST_NPH;
	fpat.dth = 1.0;
	fpat.dph = 1.0;
	fpat.thets = 90.0;
	fpat.phis = 0.0;

	stub_viewer_gain_dbi = tc->viewer_dbi;

	SetFlag(ENABLE_RDPAT);
}

/**
 * case_gamma() - reflection coefficient of a case at its reference impedance
 * @tc: case supplying the feedpoint impedance and reference impedance
 *
 * Gamma = (Z - Z0) / (Z + Z0), the definition the port waves reduce to
 * when the reference impedance is real.
 */
static double complex case_gamma(const touchstone_case_t *tc)
{
	double complex z = tc->zreal + I * tc->zimag;

	return (z - tc->zo) / (z + tc->zo);
}

/**
 * emit_columns() - render one data line and split it into numbers
 * @m:      measurement feeding the template
 * @format: meas_format() template from a touchstone layout
 * @col:    output column values
 * @max:    capacity of @col
 *
 * Returns the number of columns parsed.
 */
static int emit_columns(measurement_t *m, const char *format, double *col,
	int max)
{
	char *line = NULL;
	size_t len = 0;
	FILE *fp;
	char *tok, *ctx;
	int n = 0;

	fp = open_memstream(&line, &len);
	meas_write_format(m, format, fp);
	fclose(fp);

	for (tok = strtok_r(line, "\t\n", &ctx);
		tok != NULL && n < max;
		tok = strtok_r(NULL, "\t\n", &ctx))
	{
		col[n] = strtod(tok, NULL);
		n++;
	}

	free(line);

	return n;
}

/**
 * count_fields() - tab-separated field count of one line
 * @line: newline-terminated line
 */
static int count_fields(const char *line)
{
	int n = 1;
	const char *p;

	for (p = line; *p != '\0'; p++)
	{
		if (*p == '\t')
			n++;
	}

	return n;
}

/*------------------------------------------------------------------------*/

/**
 * test_layout_table() - every layout is populated and self-describing
 *
 * The layout array is keyed by touchstone_type_t, so a missing row would
 * leave a null template for a menu entry the save dialog still offers.
 */
static void test_layout_table(void)
{
	int type;

	for (type = 0; type < TOUCHSTONE_COUNT; type++)
	{
		const touchstone_layout_t *layout = &touchstone_layouts[type];

		ASSERT_TRUE(layout->comment != NULL, "layout supplies a comment line");
		ASSERT_TRUE(layout->format != NULL, "layout supplies a data template");

		if (layout->comment == NULL || layout->format == NULL)
			continue;

		ASSERT_TRUE(count_fields(layout->comment) == count_fields(layout->format),
			"comment line labels every data column");
	}
}

/**
 * check_s11() - S11 columns decode to the reflection coefficient
 * @tc:  case under test
 * @col: parsed columns of a data line
 */
static void check_s11(const touchstone_case_t *tc, const double *col)
{
	double complex gamma = case_gamma(tc);

	ASSERT_NEAR(col[COL_MHZ], TEST_MHZ, TOL, "frequency column");

	ASSERT_NEAR(pow(10.0, col[COL_S11_DB] / 20.0), cabs(gamma), TOL,
		"S11 magnitude decodes to |Gamma|");

	/* TODEG is the degree conversion the program applies, so comparing
	 * against it keeps this assertion on the meaning of the column
	 * rather than on the precision of the constant. */
	ASSERT_NEAR(col[COL_S11_ANG], carg(gamma) * TODEG, TOL,
		"S11 angle decodes to arg(Gamma) in degrees");
}

/**
 * check_s1p() - one-port layout carries frequency and S11 only
 * @tc: case under test
 * @m:  measurement for the case
 */
static void check_s1p(const touchstone_case_t *tc, measurement_t *m)
{
	double col[TOUCHSTONE_S2P_COLUMNS];
	int ncol;

	ncol = emit_columns(m, touchstone_layouts[TOUCHSTONE_S1P].format,
		col, TOUCHSTONE_S2P_COLUMNS);

	ASSERT_TRUE(ncol == TOUCHSTONE_S1P_COLUMNS,
		"s1p line carries one frequency and one magnitude/angle pair");

	if (ncol != TOUCHSTONE_S1P_COLUMNS)
		return;

	check_s11(tc, col);
}

/**
 * check_s2p() - two-port layout carries realized gain in S21 and S12
 * @tc:           case under test
 * @m:            measurement for the case
 * @type:         two-port layout under test
 * @raw_gain_dbi: NEC gain the layout's measurement derives from
 */
static void check_s2p(const touchstone_case_t *tc, measurement_t *m,
	touchstone_type_t type, double raw_gain_dbi)
{
	double col[TOUCHSTONE_S2P_COLUMNS];
	int ncol;

	double complex gamma = case_gamma(tc);
	double gamma_mag = cabs(gamma);

	/* Mismatch factor: the share of available power the feedpoint accepts */
	double mismatch = 1.0 - gamma_mag * gamma_mag;

	/* NEC gain is a power ratio referenced to accepted power */
	double gain_lin = pow(10.0, raw_gain_dbi / 10.0);

	double s21_mag;

	ncol = emit_columns(m, touchstone_layouts[type].format,
		col, TOUCHSTONE_S2P_COLUMNS);

	ASSERT_TRUE(ncol == TOUCHSTONE_S2P_COLUMNS,
		"s2p line carries one frequency and four magnitude/angle pairs");

	if (ncol != TOUCHSTONE_S2P_COLUMNS)
		return;

	check_s11(tc, col);

	/* Issue #80: the "DB" keyword makes every magnitude column a voltage
	 * ratio, so squaring the decoded magnitude recovers a power ratio. */
	s21_mag = pow(10.0, col[COL_S21_DB] / 20.0);

	ASSERT_NEAR(s21_mag * s21_mag, gain_lin * mismatch, TOL,
		"|S21|^2 is the realized power gain");

	/* Issue #93: a1 carries available power, so the mismatch factor rides
	 * in the transmission term. */
	ASSERT_NEAR(s21_mag, sqrt(gain_lin * mismatch), TOL,
		"S21 is sqrt(G * (1 - |S11|^2))");

	/* The same relation read from the file alone, with |S11|^2 recovered
	 * from its own column rather than from the case inputs. */
	ASSERT_NEAR(s21_mag * s21_mag,
		gain_lin * (1.0 - pow(10.0, col[COL_S11_DB] / 10.0)), TOL,
		"S21 and S11 columns agree on available power");

	/* Mismatch loss must be visible in the transmission term: a matched
	 * port passes the gain through, any reflection lowers it. */
	if (gamma_mag <= TOL)
	{
		ASSERT_NEAR(col[COL_S21_DB], raw_gain_dbi, TOL,
			"matched port emits the raw gain unchanged");
	}
	else
	{
		ASSERT_TRUE(col[COL_S21_DB] < raw_gain_dbi - TOL,
			"reflection lowers S21 below the raw gain");
	}

	ASSERT_NEAR(col[COL_S12_DB], col[COL_S21_DB], TOL,
		"S12 mirrors S21 for a passive antenna");
	ASSERT_NEAR(col[COL_S21_ANG], 0.0, TOL, "S21 angle column");
	ASSERT_NEAR(col[COL_S12_ANG], 0.0, TOL, "S12 angle column");
	ASSERT_NEAR(col[COL_S22_DB], S22_DB, TOL, "S22 magnitude column");
	ASSERT_NEAR(col[COL_S22_ANG], 0.0, TOL, "S22 angle column");
}

/**
 * test_impedance_gate() - the predicate Save_FreqPlots_Touchstone() gates on
 *
 * Every layout sources S11 from the feedpoint impedance and the two-port
 * layouts reference S21 to it through the mismatch factor, so an export
 * without impedance carries no S-parameter at all.  A feedpoint-less
 * excitation and an unallocated per-step buffer are the two ways that
 * arises.
 */
static void test_impedance_gate(void)
{
	setup_globals(&cases[0]);

	/* Applied-E voltage and applied-E current both drive a segment */
	fpat.ixtyp = 0;
	ASSERT_TRUE(meas_has_impedance(0),
		"applied-E voltage excitation reports impedance");

	fpat.ixtyp = 5;
	ASSERT_TRUE(meas_has_impedance(0),
		"applied-E current excitation reports impedance");

	/* Elementary current source: an excitation with no feedpoint */
	fpat.ixtyp = 4;
	ASSERT_TRUE(!meas_has_impedance(0),
		"feedpoint-less excitation reports no impedance");

	fpat.ixtyp = 0;
	test_impedance[0].zreal = NULL;
	ASSERT_TRUE(!meas_has_impedance(0),
		"unallocated per-step buffer reports no impedance");
}

/*------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	unsigned i;

	(void)argc;
	(void)argv;

	test_layout_table();
	test_impedance_gate();

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		const touchstone_case_t *tc = &cases[i];
		measurement_t m;
		int failed_before = tests_failed;

		setup_globals(tc);
		meas_calc(&m, 0, calc_data.ex_port);

		check_s1p(tc, &m);
		check_s2p(tc, &m, TOUCHSTONE_S2P_MAXGAIN, tc->gtot_dbi);
		check_s2p(tc, &m, TOUCHSTONE_S2P_VIEWERGAIN, tc->viewer_dbi);

		/* Name the case that produced any failures reported above */
		if (tests_failed != failed_before)
			fprintf(stderr, "  in case: %s\n", tc->name);
	}

	ASSERT_TRUE(stub_pattern_cell_calls == 0,
		"single-cell pattern leaves front-to-back sampling unreached");

	printf("touchstone_test: %d run, %d passed, %d failed\n",
		tests_run, tests_passed, tests_failed);

	return tests_failed == 0 ? 0 : 1;
}
