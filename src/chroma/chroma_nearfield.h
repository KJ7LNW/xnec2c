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

#ifndef CHROMA_NEARFIELD_H
#define CHROMA_NEARFIELD_H  1

#include "../common.h"
#include "../prerender/prerender_state.h"
#include "chroma_field.h"

/* Select the frame one near-field channel resolves.  The two static
 * baselines serve a closed phase context; the instantaneous frame serves an
 * open one. */
typedef enum
{
  NF_FRAME_PEAK = 0,
  NF_FRAME_SNAPSHOT,
  NF_FRAME_INSTANT,
  NF_FRAME_COUNT
} nf_frame_mode_t;

/**
 * nf_poynting() - Poynting magnitude |E×H| from two real vectors
 * @e:  E real vector
 * @h:  H real vector
 * @px: cross x (out); @py: cross y (out); @pz: cross z (out)
 *
 * The single Poynting-magnitude source: its cross vector feeds the geometry
 * direction and its magnitude feeds the color, identical on any phase source.
 */
static inline double
nf_poynting(const double e[3], const double h[3],
    double *px, double *py, double *pz)
{
  *px = e[1] * h[2] - h[1] * e[2];
  *py = e[2] * h[0] - h[2] * e[0];
  *pz = e[0] * h[1] - h[0] * e[1];
  return sqrt((*px) * (*px) + (*py) * (*py) + (*pz) * (*pz));
}

/**
 * nf_real_vector() - Real field vector at the resolved frame
 * @p:     immutable near-field phasor point
 * @chan:  NF_CHAN_E or NF_CHAN_H (NF_CHAN_POV composes both externally)
 * @mode:  frame the vector resolves
 * @phase: animation phase in radians, read by the instantaneous frame alone
 * @out:   real vector (out) [3]
 *
 * Derives from the phasor: instantaneous -> amp·cos(phase+φ); snapshot ->
 * that same form at phase zero; peak -> Nf_Peak_Vector envelope.  Returns
 * the vector magnitude.
 */
double nf_real_vector(const near_field_point_t *p, nf_channel_t chan,
    nf_frame_mode_t mode, double phase, double out[3]);

/**
 * chroma_proj_frame_nearfield() - Parent draw-time near-field resolver
 * @fstep:       frequency step index
 * @chan:        near-field channel to resolve
 * @mode:        resolved peak, snapshot, or instantaneous frame
 * @frame_phase: effective class phase supplied by render dispatch
 *
 * Sibling of chroma_proj_frame_wire.  Grows the channel's entry buffer,
 * gates composition on an input edge, and returns the resolved set carrying
 * each sample position, displacement, and color.  Returns an empty set when
 * the step is unavailable.
 */
field_vector_set_t chroma_proj_frame_nearfield(int fstep, nf_channel_t chan,
    nf_frame_mode_t mode, double frame_phase);

/**
 * chroma_nf_free() - Release every channel's resolver buffers
 */
void chroma_nf_free(void);

#endif
