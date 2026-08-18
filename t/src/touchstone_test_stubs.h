/*
 *  Controls for the touchstone test stubs.
 *
 *  Copyright (C) 2026 eWheeler, Inc. <https://www.linuxglobal.com/>
 */

#ifndef TOUCHSTONE_TEST_STUBS_H
#define TOUCHSTONE_TEST_STUBS_H  1

/* Gain the Viewer_Gain() stub reports, in dBi. */
extern double stub_viewer_gain_dbi;

/* Count of front-to-back pattern cell lookups the stubs served. */
extern int stub_pattern_cell_calls;

#endif
