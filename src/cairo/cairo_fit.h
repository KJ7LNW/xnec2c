/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __CAIRO_FIT_H
#define __CAIRO_FIT_H 1

#include "../render/render_engine.h"

/* The Cairo engine: its domain protocol and its active-surface operations.
 * Consumers reach the fit through canvas_fit_view(), which names the engine
 * from the canvas binding. */
extern const render_engine_t cairo_engine;

#endif /* __CAIRO_FIT_H */
