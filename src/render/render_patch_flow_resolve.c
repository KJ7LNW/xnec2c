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

#include "render_patch_flow_resolve.h"
#include "../prerender/prerender_color.h"
#include "../shared.h"

/* Squared magnitude below which a patch carries no resolvable direction.
 * Paired with FLOW_MAG_THRESHOLD, which admits an arrow at a higher bound. */
#define FLOW_MAG_GUARD_SQ 1e-6f

/**
 * resolve_patch_flow_angle() - Resolve one tangent-plane direction angle
 * @direction: formula selected by the active mode row
 * @flow_data: tangent-axis phasors normalized by the patch current range
 * @phase: effective patch-class phase in radians
 *
 * Returns the canonical tangent-plane angle in radians.
 */
static double
resolve_patch_flow_angle(patch_flow_direction_t direction,
    const float flow_data[4], double phase)
{
  double angle = NAN;

  switch( direction )
  {
    case PATCH_FLOW_DIRECTION_INSTANT:
    {
      double cos_phase = cos(phase);
      double sin_phase = sin(phase);
      double re1 = (double)flow_data[0] * cos_phase
                 - (double)flow_data[1] * sin_phase;
      double re2 = (double)flow_data[2] * cos_phase
                 - (double)flow_data[3] * sin_phase;

      angle = atan2(re2, re1);
      break;
    }

    case PATCH_FLOW_DIRECTION_POLARIZATION_AXIS:
    {
      double ct1_sq = (double)flow_data[0] * flow_data[0]
                    + (double)flow_data[1] * flow_data[1];
      double ct2_sq = (double)flow_data[2] * flow_data[2]
                    + (double)flow_data[3] * flow_data[3];
      double cross = (double)flow_data[0] * flow_data[2]
                   + (double)flow_data[1] * flow_data[3];

      angle = 0.5 * atan2(2.0 * cross, ct1_sq - ct2_sq);
      break;
    }

    case PATCH_FLOW_DIRECTION_PEAK_MAGNITUDE:
    {
      double p_re = ((double)flow_data[0] * flow_data[0]
                   - (double)flow_data[1] * flow_data[1])
                  + ((double)flow_data[2] * flow_data[2]
                   - (double)flow_data[3] * flow_data[3]);
      double p_im = 2.0 * ((double)flow_data[0] * flow_data[1]
                         + (double)flow_data[2] * flow_data[3]);
      double alpha = -0.5 * atan2(p_im, p_re);
      double cos_alpha = cos(alpha);
      double sin_alpha = sin(alpha);
      double peak_re1 = (double)flow_data[0] * cos_alpha
                      - (double)flow_data[1] * sin_alpha;
      double peak_re2 = (double)flow_data[2] * cos_alpha
                      - (double)flow_data[3] * sin_alpha;

      angle = atan2(peak_re2, peak_re1);
      break;
    }

    case PATCH_FLOW_DIRECTION_COUNT:
      BUG("Patch flow direction %d is not a selectable formula\n", direction);
      angle = 0.0;
      break;
  }

  return angle;
}

/**
 * resolve_patch_flow_entry() - Resolve one dense patch-flow entry
 * @fstep: frequency step holding authoritative patch phasors
 * @idx: patch index
 * @direction: formula selected by the active mode row
 * @phase: effective patch-class phase in radians
 * @entry: destination entry in managed resolver scratch
 */
void
resolve_patch_flow_entry(int fstep, int idx, patch_flow_direction_t direction,
    double phase, patch_flow_entry_t *entry)
{
  float flow_data[4];
  float magnitude_sq;
  double angle;

  get_precomputed_flow_data(fstep, idx, flow_data);
  magnitude_sq = flow_data[0] * flow_data[0]
               + flow_data[1] * flow_data[1]
               + flow_data[2] * flow_data[2]
               + flow_data[3] * flow_data[3];

  *entry = (patch_flow_entry_t){0};

  /* Offset the bound by the helper tolerance so the tolerant comparison
   * admits exactly the magnitudes the stored guard admits. */
  if( !fl_fgt(magnitude_sq, FLOW_MAG_GUARD_SQ - FL_EPS) )
    return;

  angle = resolve_patch_flow_angle(direction, flow_data, phase);
  entry->direction[0] = (float)cos(angle);
  entry->direction[1] = (float)sin(angle);
  entry->magnitude = (float)sqrt((double)magnitude_sq);
}

/**
 * resolve_patch_flow_arrow() - Resolve one admitted arrow into world space
 * @entry: dense patch-flow entry supplying direction and magnitude
 * @frame: tangent frame placing the arrow on its patch
 * @color: resolved patch color
 * @patch: patch index the arrow marks
 * @arrow: destination arrow in managed resolver scratch
 *
 * Maps the authoritative arrow template through the entry direction and the
 * patch tangent frame:
 *   world = center + 2 * (rotated_u * st1 + rotated_v * st2)
 *
 * Returns one when the entry exceeds the arrow threshold, otherwise zero.
 */
int
resolve_patch_flow_arrow(const patch_flow_entry_t *entry,
    const patch_tangent_frame_t *frame, const rgb_f_t *color, int patch,
    patch_flow_arrow_t *arrow)
{
  double cos_angle = entry->direction[0];
  double sin_angle = entry->direction[1];
  int idx;

  /* Offset the bound by the helper tolerance so the tolerant comparison
   * admits exactly the magnitudes the stored threshold admits. */
  if( !fl_fgt(entry->magnitude, FLOW_MAG_THRESHOLD - FL_EPS) )
    return 0;

  arrow->color = *color;
  arrow->patch = patch;

  for( idx = 0; idx < ARROW_LINE_COUNT; idx++ )
  {
    double u1 = (double)arrow_template[idx].u1 - 0.5;
    double v1 = (double)arrow_template[idx].v1 - 0.5;
    double rotated_u1 = u1 * cos_angle - v1 * sin_angle;
    double rotated_v1 = u1 * sin_angle + v1 * cos_angle;
    double u2 = (double)arrow_template[idx].u2 - 0.5;
    double v2 = (double)arrow_template[idx].v2 - 0.5;
    double rotated_u2 = u2 * cos_angle - v2 * sin_angle;
    double rotated_v2 = u2 * sin_angle + v2 * cos_angle;

    arrow->segments[idx] = (flow_seg_3d_t){
        .x1 = frame->cx + 2.0 * (rotated_u1 * frame->st1x
                               + rotated_v1 * frame->st2x),
        .y1 = frame->cy + 2.0 * (rotated_u1 * frame->st1y
                               + rotated_v1 * frame->st2y),
        .z1 = frame->cz + 2.0 * (rotated_u1 * frame->st1z
                               + rotated_v1 * frame->st2z),
        .x2 = frame->cx + 2.0 * (rotated_u2 * frame->st1x
                               + rotated_v2 * frame->st2x),
        .y2 = frame->cy + 2.0 * (rotated_u2 * frame->st1y
                               + rotated_v2 * frame->st2y),
        .z2 = frame->cz + 2.0 * (rotated_u2 * frame->st1z
                               + rotated_v2 * frame->st2z),
    };
  }

  return 1;
}
