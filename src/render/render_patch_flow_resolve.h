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

#ifndef __RENDER_PATCH_FLOW_RESOLVE_H
#define __RENDER_PATCH_FLOW_RESOLVE_H     1

#include "render_patch_flow.h"
#include "../prerender/prerender_state.h"

/**
 * resolve_patch_flow_entry() - Resolve one dense patch-flow entry
 * @fstep: frequency step holding authoritative patch phasors
 * @idx: patch index
 * @direction: formula selected by the active mode row
 * @phase: effective patch-class phase in radians
 * @entry: destination entry in managed resolver scratch
 */
void resolve_patch_flow_entry(int fstep, int idx,
    patch_flow_direction_t direction, double phase,
    patch_flow_entry_t *entry);

/**
 * resolve_patch_flow_arrow() - Resolve one admitted arrow into world space
 * @entry: dense patch-flow entry supplying direction and magnitude
 * @frame: tangent frame placing the arrow on its patch
 * @color: resolved patch color
 * @patch: patch index the arrow marks
 * @arrow: destination arrow in managed resolver scratch
 *
 * Returns one when the entry exceeds the arrow threshold, otherwise zero.
 */
int resolve_patch_flow_arrow(const patch_flow_entry_t *entry,
    const patch_tangent_frame_t *frame, const rgb_f_t *color, int patch,
    patch_flow_arrow_t *arrow);

#endif /* __RENDER_PATCH_FLOW_RESOLVE_H */
