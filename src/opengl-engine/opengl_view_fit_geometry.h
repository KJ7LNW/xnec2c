/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __OPENGL_VIEW_FIT_GEOMETRY_H
#define __OPENGL_VIEW_FIT_GEOMETRY_H 1

#include "opengl_view.h"
#include "../render/render_fit.h"

#ifdef HAVE_OPENGL

/** gl_view_fit_fold_box() - Fold active vertices into a rotated box */
gboolean gl_view_fit_fold_box(view_t *view, gl_view_state_t *state,
    render_fit_acc_t *acc);

/** gl_view_fit_perspective() - Center active vertices inside the frustum */
gboolean gl_view_fit_perspective(gl_view_state_t *state, view_fit_t *fit);

#endif /* HAVE_OPENGL */
#endif /* __OPENGL_VIEW_FIT_GEOMETRY_H */
