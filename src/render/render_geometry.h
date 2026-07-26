/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __RENDER_GEOMETRY_H
#define __RENDER_GEOMETRY_H 1

#include "render_dispatch.h"

/* Point sink receiving one drawn world-space vertex and its model scale. */
typedef void (*render_geom_point_fn)(void *user,
    double x, double y, double z, float scale);

/** render_nearfield_fields() - Resolve active near-field vector sets
 * @fstep:  frequency step index
 * @fields: receives active vector and color pairs
 *
 * Returns the number of populated entries in @fields.
 */
int render_nearfield_fields(int fstep,
    nf_field_set_t fields[NF_FIELD_SETS_MAX]);

/** render_geom_walk() - Emit the active view's drawn world geometry
 * @view: view selecting structure, far-field, or near-field content
 * @sink: receives each drawn vertex and its model scale
 * @user: opaque pointer passed to @sink
 *
 * Returns the active content reference extent, or zero when unavailable.
 * The caller holds freq_data_lock.
 */
float render_geom_walk(view_t *view, render_geom_point_fn sink, void *user);

#endif /* __RENDER_GEOMETRY_H */
