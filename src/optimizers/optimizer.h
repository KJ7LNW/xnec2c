/*
 *  Unified optimizer dispatch layer.
 *
 *  Wraps simplex_t and pso_t behind a common vtable so callers
 *  can switch algorithms without changing call sites.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H 1

#include <gsl/gsl_vector.h>

#include "simplex.h"
#include "particleswarm.h"

/** Algorithm selector */
enum optimizer_algo
{
	OPT_SIMPLEX,
	OPT_PSO
};

/** Opaque optimizer handle */
typedef struct optimizer_s optimizer_t;

/**
 * optimizer_new_simplex - wrap a simplex backend
 * @cfg: simplex configuration (deep-copied by simplex_new)
 *
 * Returns NULL if simplex_new rejects the config.
 */
optimizer_t *optimizer_new_simplex(const simplex_config_t *cfg);

/**
 * optimizer_new_pso - wrap a PSO backend
 * @cfg: PSO configuration (deep-copied by pso_new)
 *
 * Returns NULL if pso_new rejects the config.
 */
optimizer_t *optimizer_new_pso(const pso_config_t *cfg);

/**
 * optimizer_optimize - run the optimization loop
 * @o: optimizer handle
 *
 * Delegates to the underlying backend.  Returns best fitness found.
 */
double optimizer_optimize(optimizer_t *o);

/**
 * optimizer_get_best_pos - return best position vector
 * @o: optimizer handle
 *
 * Owned by the backend; do not free.
 */
const gsl_vector *optimizer_get_best_pos(const optimizer_t *o);

/**
 * optimizer_get_best_fit - return best fitness value
 * @o: optimizer handle
 */
double optimizer_get_best_fit(const optimizer_t *o);

/**
 * optimizer_get_iteration_count - return total iterations performed
 * @o: optimizer handle
 */
int optimizer_get_iteration_count(const optimizer_t *o);

/**
 * optimizer_free - release optimizer and underlying backend
 * @o: optimizer handle (NULL safe)
 */
void optimizer_free(optimizer_t *o);

#endif
