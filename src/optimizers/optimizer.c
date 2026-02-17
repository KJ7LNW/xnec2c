/*
 *  Unified optimizer dispatch layer.
 *
 *  Copyright (C) 2025 eWheeler, Inc. <https://www.linuxglobal.com/>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

#include "optimizer.h"

#include <stdlib.h>

/** Vtable function signatures with void* impl pointer */
typedef double             (*opt_optimize_fn)(void *);
typedef const gsl_vector * (*opt_get_pos_fn)(const void *);
typedef double             (*opt_get_fit_fn)(const void *);
typedef int                (*opt_get_iter_fn)(const void *);
typedef void               (*opt_free_fn)(void *);

/** Full dispatch state */
struct optimizer_s
{
	enum optimizer_algo algo;
	void *impl;

	opt_optimize_fn optimize;
	opt_get_pos_fn  get_best_pos;
	opt_get_fit_fn  get_best_fit;
	opt_get_iter_fn get_iteration_count;
	opt_free_fn     free_fn;
};

/* Type-safe wrappers: avoid casting between incompatible function pointers */

static double _simplex_optimize(void *p)        { return simplex_optimize(p); }
static const gsl_vector *_simplex_get_pos(const void *p)  { return simplex_get_best_pos(p); }
static double _simplex_get_fit(const void *p)   { return simplex_get_best_fit(p); }
static int    _simplex_get_iter(const void *p)  { return simplex_get_iteration_count(p); }
static void   _simplex_free(void *p)            { simplex_free(p); }

static double _pso_optimize(void *p)            { return pso_optimize(p); }
static const gsl_vector *_pso_get_pos(const void *p)      { return pso_get_best_pos(p); }
static double _pso_get_fit(const void *p)       { return pso_get_best_fit(p); }
static int    _pso_get_iter(const void *p)      { return pso_get_iteration_count(p); }
static void   _pso_free(void *p)                { pso_free(p); }

/**
 * optimizer_new_simplex - create dispatch handle wrapping simplex backend
 * @cfg: simplex configuration
 */
optimizer_t *optimizer_new_simplex(const simplex_config_t *cfg)
{
	simplex_t *s = simplex_new(cfg);
	if (!s)
	{
		return NULL;
	}

	optimizer_t *o = calloc(1, sizeof(*o));
	if (!o)
	{
		simplex_free(s);
		return NULL;
	}

	o->algo = OPT_SIMPLEX;
	o->impl = s;
	o->optimize            = _simplex_optimize;
	o->get_best_pos        = _simplex_get_pos;
	o->get_best_fit        = _simplex_get_fit;
	o->get_iteration_count = _simplex_get_iter;
	o->free_fn             = _simplex_free;

	return o;
}

/**
 * optimizer_new_pso - create dispatch handle wrapping PSO backend
 * @cfg: PSO configuration
 */
optimizer_t *optimizer_new_pso(const pso_config_t *cfg)
{
	pso_t *p = pso_new(cfg);
	if (!p)
	{
		return NULL;
	}

	optimizer_t *o = calloc(1, sizeof(*o));
	if (!o)
	{
		pso_free(p);
		return NULL;
	}

	o->algo = OPT_PSO;
	o->impl = p;
	o->optimize            = _pso_optimize;
	o->get_best_pos        = _pso_get_pos;
	o->get_best_fit        = _pso_get_fit;
	o->get_iteration_count = _pso_get_iter;
	o->free_fn             = _pso_free;

	return o;
}

/** optimizer_optimize - delegate to backend */
double optimizer_optimize(optimizer_t *o)
{
	return o->optimize(o->impl);
}

/** optimizer_get_best_pos - delegate to backend */
const gsl_vector *optimizer_get_best_pos(const optimizer_t *o)
{
	return o->get_best_pos(o->impl);
}

/** optimizer_get_best_fit - delegate to backend */
double optimizer_get_best_fit(const optimizer_t *o)
{
	return o->get_best_fit(o->impl);
}

/** optimizer_get_iteration_count - delegate to backend */
int optimizer_get_iteration_count(const optimizer_t *o)
{
	return o->get_iteration_count(o->impl);
}

/**
 * optimizer_free - release dispatch handle and underlying backend
 * @o: handle (NULL safe)
 */
void optimizer_free(optimizer_t *o)
{
	if (!o)
	{
		return;
	}

	o->free_fn(o->impl);
	free(o);
}
