/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#ifndef __RENDER_FIT_H
#define __RENDER_FIT_H 1

#include <glib.h>

/*
 * render_fit: generic geometric reducer for fit-to-view.
 *
 * Folds a stream of vertices into a centroid and framing measure with no
 * awareness of far-field / near-field / structure mode and no awareness of
 * segment radius.  Each engine feeds it the vertices of the geometry that
 * engine actually draws and inverts the reduced frame into its own zoom and
 * pan for parallel projection.
 */

/* Fixed per-side viewport border reserved by the fit, in pixels.  Tuned and
 * committed statically; no rc_config field and no user-facing setting. */
#define RENDER_FIT_BORDER_PX 5

/* Generic reduction accumulator; folds a vertex stream with no mode or radius
 * awareness.  Zero-initialize before the first fold (any == FALSE). */
typedef struct
{
  float minx, miny, maxx, maxy;   /* bounding box in the fed basis */
  gboolean any;

} render_fit_acc_t;

/* Reduced framing the engine leaf inverts into its own zoom and pan. */
typedef struct
{
  float cx, cy;         /* centroid in the fed basis */
  float half_x, half_y; /* raw half-extents; engine reserves border */

} render_fit_frame_t;

/**
 * render_fit_add() - Fold one vertex into the bounding box
 * @acc: accumulator
 * @x:   vertex x in the fed basis
 * @y:   vertex y in the fed basis
 */
void render_fit_add(render_fit_acc_t *acc, float x, float y);

/**
 * render_fit_solve() - Find the center and half-extents of the folded box
 * @acc: folded accumulator
 * @out: receives center and raw half-extents before the viewport border
 *
 * Return: FALSE when the accumulator folded no vertex.
 */
gboolean render_fit_solve(const render_fit_acc_t *acc, render_fit_frame_t *out);

#endif /* __RENDER_FIT_H */
