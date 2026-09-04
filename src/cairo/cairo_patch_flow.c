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

/*
 * cairo_patch_flow: Cairo deposition of resolved patch current flow marks.
 *
 * The render layer resolves direction, magnitude admission, and world-space
 * arrow geometry, so this leaf projects and deposits the prepared arrows
 * without reading phasors, phase, configuration, or a mark selection.
 */
#include "cairo_patch_flow.h"
#include "../mem/mem.h"

/**
 * cairo_patch_flow_draw() - Deposit the resolved patch flow arrows
 * @sb: scenebuffer accumulating depth-sorted primitives
 * @v: view supplying the projection basis
 * @scale: structure-to-presentation scale
 * @flow: renderer-neutral frame carrying world-space arrows
 *
 * Projects every arrow the render layer admitted, in the color that layer
 * attached; an empty arrow array deposits nothing.  The scenebuffer sorts
 * by depth, so deposition order does not affect the painted result.
 */
void
cairo_patch_flow_draw(cairo_scenebuffer_t *sb, view_t *v, double scale,
    const patch_flow_frame_t *flow)
{
  int arrow_count = mem_array_count(flow->arrows);
  int arrow_idx;

  for( arrow_idx = 0; arrow_idx < arrow_count; arrow_idx++ )
  {
    const patch_flow_arrow_t *arrow = &flow->arrows[arrow_idx];
    int seg_idx;

    for( seg_idx = 0; seg_idx < ARROW_LINE_COUNT; seg_idx++ )
    {
      const flow_seg_3d_t *seg = &arrow->segments[seg_idx];
      Segment_t deposited;

      Set_Gdk_Segment(&deposited, v, scale,
          seg->x1, seg->y1, seg->z1,
          seg->x2, seg->y2, seg->z2,
          &deposited.z_mid);
      seg_set_color(&deposited, arrow->color);
      deposited.width = 1.0f;
      scenebuffer_add(sb, &deposited);
    }
  }
}
