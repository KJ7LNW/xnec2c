#ifndef VALIDATION_DUMP_H
#define VALIDATION_DUMP_H

/**
 * validation_dump_set_dir - set the output directory for tree dumps
 * @dir: path to the directory that Save_Validation_Tree() will populate;
 *       the directory is created on first use if it does not exist.
 *       Pass NULL to disable dumping.
 */
void validation_dump_set_dir(char *dir);

/**
 * validation_dump_force_config - force deterministic config for reproducible dumps
 *
 * Must be called after init_mathlib() and Read_Config(), and before the
 * frequency loop.  No-ops if no validation directory was set.  Selects the NEC2
 * built-in Gaussian elimination (overriding the rc-file "Selected Mathlib") so
 * results are bit-identical regardless of installed BLAS libraries, and pins
 * rc_config.ant_temp_elevation to zero so the antenna noise-temperature columns
 * (ant_temp, ant_temp_tot, gt) are reproducible across machines.
 */
void validation_dump_force_config(void);

/**
 * Save_Validation_Tree - write all NEC engine data structures to CSV files
 *
 * Writes one CSV file per structure type under the directory set by
 * validation_dump_set_dir().  No-ops if the directory was not set or
 * the result set is not published.  Gated by freq_data_lock.
 */
void Save_Validation_Tree(void);

#endif /* VALIDATION_DUMP_H */
