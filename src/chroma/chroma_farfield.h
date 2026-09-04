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

#ifndef CHROMA_FARFIELD_H
#define CHROMA_FARFIELD_H  1

#include "../common.h"
#include "chroma_field.h"

/**
 * chroma_proj_frame_farfield() - Parent draw-time far-zone field resolver
 * @fstep:       frequency step index
 * @phase:       effective class phase supplied by render dispatch
 * @translation: rigid displacement applied to every pattern vertex
 *
 * Sibling of chroma_proj_frame_nearfield.  Evaluates the stored far-zone
 * phasor pair at @phase into the instantaneous field vector tangent to the
 * pattern sphere, attaches it at the translated pattern vertex, and colors
 * it on the ramp palette.  Returns an empty set when the step carries no
 * pattern.
 */
field_vector_set_t chroma_proj_frame_farfield(int fstep, double phase,
    const point_3d_t *translation);

/**
 * chroma_ff_free() - Release the resolver buffers
 */
void chroma_ff_free(void);

#endif
