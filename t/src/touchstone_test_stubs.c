/*
 *  Test stubs for the Touchstone export unit test.
 *
 *  Supplies the runtime and pattern-geometry entry points measurements.c
 *  calls, so the test drives meas_calc() against synthetic NEC results.
 *
 *  Copyright (C) 2026 eWheeler, Inc. <https://www.linuxglobal.com/>
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "console.h"

#include "touchstone_test_stubs.h"

char *orig_numeric_locale = NULL;

double stub_viewer_gain_dbi = 0.0;
int stub_pattern_cell_calls = 0;

/* Flag word standing in for the runtime state utils.c owns. */
static unsigned long long stub_flags = 0;

int
isFlagClear(unsigned long long int flag)
{
	return (stub_flags & flag) == 0;
}

void
SetFlag(unsigned long long int flag)
{
	stub_flags |= flag;
}

/* The single-cell test pattern samples no rear azimuth, so front-to-back
 * sampling never runs; a call here means the case setup changed. */
int
fpat_theta_cell(double theta_deg)
{
	(void)theta_deg;
	stub_pattern_cell_calls++;
	return 0;
}

int
fpat_phi_cell(double phi_deg)
{
	(void)phi_deg;
	stub_pattern_cell_calls++;
	return 0;
}

/* Total polarization applies no correction to the stored gain, so the
 * test controls gain entirely through rad_pattern gtot. */
double
Polarization_Factor(int pol_type, int fstep, int idx)
{
	(void)pol_type;
	(void)fstep;
	(void)idx;
	return 0.0;
}

double
Viewer_Gain(view_t *v, int fstep)
{
	(void)v;
	(void)fstep;
	return stub_viewer_gain_dbi;
}

const char *
freq_sweep_state_name(void)
{
	return "test";
}

void
print_backtrace(char *msg)
{
	(void)msg;
}

void
_print_backtrace(char **strings)
{
	(void)strings;
}

char **
_get_backtrace(void)
{
	return NULL;
}

int
Stop(int err, const char *format, ...)
{
	(void)err;
	(void)format;
	return 0;
}

gboolean
isChild(void)
{
	return FALSE;
}
