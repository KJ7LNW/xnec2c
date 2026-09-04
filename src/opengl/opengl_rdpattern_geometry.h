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

#ifndef OPENGL_RDPATTERN_GEOMETRY_H
#define OPENGL_RDPATTERN_GEOMETRY_H 1

#include "common.h"
#include "../rdpattern_ui.h"
#include "../render/render_dispatch.h"

#ifdef HAVE_OPENGL
#include "../shared.h"

/* Generate line geometry from dispatch-resolved field vector sets.
 * Walks the set list to its terminator, converting each entry to a
 * lit_color_point_t pair.  Returns total line count, or -1 on failure. */
int opengl_rdpattern_generate_field_vector_lines(
    const field_vector_set_t *sets);

/* Tessellate point_3d buffer into colored triangles.
 * Per-vertex color from precomputed vertex_rgb array.
 * Returns triangle count, or -1 on invalid input. */
int opengl_rdpattern_generate_triangles(
    point_3d_t *points, int nth, int nph,
    const rgb_f_t *vertex_rgb);

/* Return field vector line vertex buffer and count */
lit_color_point_t* opengl_rdpattern_get_field_vector_lines(int *count);

/* Return far-field triangle buffer and count */
lit_color_triangle_t* opengl_rdpattern_get_triangles(int *count);

/* Tessellate point_3d buffer into colored line pairs for wireframe.
 * theta_rgb: precomputed per-edge colors [(nth-1)*nph].
 * phi_rgb: precomputed per-edge colors [nth*(nph-1)].
 * Returns line count, or -1 on invalid input. */
int opengl_rdpattern_generate_lines(
    point_3d_t *points, int nth, int nph,
    const rgb_f_t *theta_rgb, const rgb_f_t *phi_rgb);

/* Return far-field wireframe line buffer and count */
lit_color_point_t* opengl_rdpattern_get_lines(int *count);

/* Return current far-field generation counter */
unsigned int opengl_rdpattern_get_ff_generation(void);

/* Return current field vector generation counter */
unsigned int opengl_rdpattern_get_field_vector_generation(void);

/* Free all geometry buffers and reset state */
void opengl_rdpattern_geometry_cleanup(void);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_RDPATTERN_GEOMETRY_H */
