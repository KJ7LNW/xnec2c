/*
 *  Synchronous NEC2 frequency loop evaluation for optimizer.
 *
 *  Provides a blocking interface for the optimizer thread to trigger
 *  a full frequency loop and collect measurement_t results.  Uses a
 *  GCond signaled when the frequency loop sets FREQ_LOOP_DONE.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_NEC2_EVAL_H
#define OPT_NEC2_EVAL_H 1

#include <glib.h>

#include "../measurements.h"
#include "opt_simple.h"

/**
 * nec2_eval_init - initialize evaluation synchronization primitives
 *
 * Must be called once before any nec2_eval_run() calls.
 */
void nec2_eval_init(void);

/**
 * nec2_eval_cleanup - release synchronization primitives
 */
void nec2_eval_cleanup(void);

/**
 * nec2_eval_signal - signal that the frequency loop has completed
 *
 * Called from Frequency_Loop() when FREQ_LOOP_DONE is set.
 * Wakes the optimizer thread blocked in nec2_eval_run().
 */
void nec2_eval_signal(void);

/**
 * nec2_eval_run - synchronously evaluate antenna with given variables
 * @vars: simple_var_t array with current optimizer values
 * @num_vars: length of vars array
 * @meas_out: output array, caller-allocated, sized for calc_data.steps_total
 * @max_steps: capacity of meas_out array
 *
 * Applies SY overrides from vars, triggers Open_Input_File via the GTK
 * main loop, blocks until the frequency loop completes, then fills
 * meas_out with measurement_t for each frequency step.
 *
 * Returns the number of frequency steps evaluated, or -1 on error.
 */
int nec2_eval_run(const simple_var_t *vars, int num_vars,
	measurement_t *meas_out, int max_steps);

/**
 * nec2_eval_get_freq - get frequency array after evaluation
 * @freq_out: output array, caller-allocated, sized for max_steps
 * @max_steps: capacity of freq_out array
 *
 * Copies save.freq[] into freq_out after a successful nec2_eval_run().
 * Returns the number of steps copied.
 */
int nec2_eval_get_freq(double *freq_out, int max_steps);

#endif
