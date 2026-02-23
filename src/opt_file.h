/*
 *  Optimizer configuration file I/O.
 *
 *  Saves and loads optimizer panel state (algorithm settings, fitness
 *  goals) to/from a GKeyFile-format .opt file alongside the .nec model.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPT_FILE_H
#define OPT_FILE_H 1

#include "common.h"

/**
 * opt_file_save - write optimizer configuration to .opt file
 *
 * Derives the .opt path from rc_config.input_file by replacing
 * the .nec extension.  Reads current state from optimizer UI
 * widgets and writes via GKeyFile.
 *
 * Returns TRUE on success, FALSE if no input file or write fails.
 */
gboolean opt_file_save(void);

/**
 * opt_file_load - load optimizer configuration from .opt file
 *
 * Derives the .opt path from rc_config.input_file.  If the file
 * exists, clears existing goal rows, populates UI widgets from
 * the saved state, and rebuilds the formula display.
 *
 * Returns TRUE if file was loaded, FALSE if not found or parse error.
 */
gboolean opt_file_load(void);

#endif
