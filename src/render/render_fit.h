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
 * pan.  The only mode-shaped input is the projection kind.
 */

/* Fixed per-side viewport border reserved by the fit, in pixels.  Tuned and
 * committed statically; no rc_config field and no user-facing setting. */
#define RENDER_FIT_BORDER_PX 5

/* Projection the reducer branches on; supplied by the engine leaf.  No radius
 * term: a drawn cylinder's wall vertices are already in the buffer. */
typedef enum
{
  RENDER_PROJ_PARALLEL    = 0,
  RENDER_PROJ_PERSPECTIVE = 1,
  RENDER_PROJ_COUNT

} render_proj_kind_t;

typedef struct
{
  render_proj_kind_t kind;
  float fov_rad;      /* vertical field of view; parallel ignores it */
  float aspect;       /* viewport width / height */

} render_proj_t;

/* Generic reduction accumulator; folds a vertex stream with no mode or radius
 * awareness.  Zero-initialize before the first fold (any == FALSE). */
typedef struct
{
  float minx, miny, maxx, maxy;   /* bounding box in the fed basis */
  float dmax;                     /* perspective depth-aware maximum */
  gboolean any;
  gboolean depth_any;

} render_fit_acc_t;

/* Reduced framing the engine leaf inverts into its own zoom and pan. */
typedef struct
{
  float cx, cy;         /* centroid in the fed basis */
  float half_x, half_y; /* raw half-extents (parallel); engine reserves border */
  float dist;           /* perspective depth-aware distance; 0 for parallel */

} render_fit_frame_t;

/**
 * render_fit_add() - Fold one vertex into the bounding box
 * @acc: accumulator
 * @x:   vertex x in the fed basis
 * @y:   vertex y in the fed basis
 */
void render_fit_add(render_fit_acc_t *acc, float x, float y);

/**
 * render_fit_add_persp() - Fold one vertex into the depth-aware maximum
 * @acc:  accumulator (box already folded so @cx,@cy are known)
 * @cx:   centroid x the framing centers on
 * @cy:   centroid y the framing centers on
 * @x:    vertex x in the fed basis
 * @y:    vertex y in the fed basis
 * @z:    vertex depth toward the camera in the fed basis
 * @proj: projection parameters (fov, aspect)
 *
 * Captures the depth/lateral correlation a flat box cannot: the camera must
 * sit at least @z plus the border-widened lateral need behind a centered
 * vertex for it to clear the frustum.
 */
void render_fit_add_persp(render_fit_acc_t *acc, float cx, float cy,
                          float x, float y, float z, const render_proj_t *proj);

/**
 * render_fit_solve() - Reduce the accumulator to a centroid and framing measure
 * @acc:  folded accumulator
 * @proj: projection kind and parameters
 * @viewport_height: viewport height in pixels
 * @out:             receives centroid, half-extents, and distance
 *
 * Returns FALSE with a warning when the accumulator folded no vertex.
 * Parallel yields the raw box half-extents; the engine leaf reserves the pixel
 * border against the viewport.  Perspective yields the depth-aware distance
 * widened by the border viewport fraction.
 */
gboolean render_fit_solve(const render_fit_acc_t *acc, const render_proj_t *proj,
                          int viewport_height, render_fit_frame_t *out);

#endif /* __RENDER_FIT_H */
