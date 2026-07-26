/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __CAIRO_FIT_H
#define __CAIRO_FIT_H 1

#include "../render/render_engine.h"

/** cairo_fit_view() - Fit a Cairo view to its drawn geometry
 * @view: structure or radiation-pattern view
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Returns FALSE when no geometry is available or the viewport is degenerate.
 */
gboolean cairo_fit_view(view_t *view, view_fit_t *fit);

/* Cairo engine control-operation vtable. */
extern const render_engine_ops_t cairo_engine_ops;

#endif /* __CAIRO_FIT_H */
