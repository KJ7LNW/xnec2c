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

/**
 * render_nearfield_fields() - Resolve active near-field vector sets
 * @fstep:       frequency step index
 * @frame_phase: phase read once by the frame owner
 * @sets:        receives active sets followed by an empty terminator
 */
void render_nearfield_fields(int fstep, double frame_phase,
    field_vector_set_t sets[NF_CHAN_NUM + 1]);

/**
 * render_farfield_vectors() - Resolve the far-zone instantaneous field set
 * @fstep:       frequency step index
 * @frame_phase: phase read once by the frame owner
 * @ff:          far-field draw parameters supplying excitation translation
 * @set:         receives the resolved set or an empty set
 */
void render_farfield_vectors(int fstep, double frame_phase,
    const ff_draw_params_t *ff, field_vector_set_t *set);

/**
 * render_geom_walk() - Emit the active view's drawn world geometry
 * @view:        view selecting structure, far-field, or near-field content
 * @frame_phase: phase read once by the frame owner
 * @sink:        receives each drawn vertex and its model scale
 * @user:        opaque pointer passed to @sink
 *
 * Returns the active content reference extent, or zero when unavailable.
 * The caller holds freq_data_lock.
 */
float render_geom_walk(view_t *view, double frame_phase,
    render_geom_point_fn sink, void *user);

#endif /* __RENDER_GEOMETRY_H */
