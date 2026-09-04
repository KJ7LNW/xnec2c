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

#ifndef __RENDER_PATCH_FLOW_H
#define __RENDER_PATCH_FLOW_H     1

#include "../common.h"
#include "../prerender/prerender_patch_arrow.h"

/* Define the persisted patch current flow selection with contiguous values. */
typedef enum
{
  PATCH_FLOW_MODE_REFERENCE_PHASE = 0,
  PATCH_FLOW_MODE_POLARIZATION_AXIS,
  PATCH_FLOW_MODE_PEAK_MAGNITUDE,
  PATCH_FLOW_MODE_LIC,
  PATCH_FLOW_MODE_WIREFRAME,
  PATCH_FLOW_MODE_COUNT
} patch_flow_mode_t;

/* Select the patch body geometry a selection draws. */
typedef enum
{
  PATCH_FILL_SOLID = 0,
  PATCH_FILL_WIREFRAME,
  PATCH_FILL_COUNT
} patch_fill_t;

/* Select the formula resolving one patch's tangent-plane direction. */
typedef enum
{
  PATCH_FLOW_DIRECTION_INSTANT = 0,
  PATCH_FLOW_DIRECTION_POLARIZATION_AXIS,
  PATCH_FLOW_DIRECTION_PEAK_MAGNITUDE,
  PATCH_FLOW_DIRECTION_COUNT
} patch_flow_direction_t;

/* Select the presentation technique marking the resolved direction. */
typedef enum
{
  PATCH_FLOW_MARK_DIRECTIONAL = 0,
  PATCH_FLOW_MARK_BIDIRECTIONAL,
  PATCH_FLOW_MARK_LIC,
  PATCH_FLOW_MARK_ARROW,
  PATCH_FLOW_MARK_COUNT
} patch_flow_mark_t;

/* Carry one world-space arrow line segment. */
typedef struct
{
  double x1, y1, z1;
  double x2, y2, z2;
} flow_seg_3d_t;

/* Carry one patch's resolved tangent-plane flow. */
typedef struct
{
  float direction[2];
  float magnitude;
} patch_flow_entry_t;

/* Carry one patch's arrow after resolving the template into world space. */
typedef struct
{
  rgb_f_t        color;
  int            patch;
  flow_seg_3d_t  segments[ARROW_LINE_COUNT];
} patch_flow_arrow_t;

/* Publish one renderer-neutral frame over managed resolver scratch. */
typedef struct
{
  const patch_flow_entry_t *entries;
  const patch_flow_arrow_t *arrows;
  patch_fill_t              fill;
  patch_flow_mark_t         mark;
  double                    scroll_phase;

  /* Advances when the published entries and arrows change, so a consumer
   * rebuilding from them compares one scalar */
  unsigned int              generation;
} patch_flow_frame_t;

/**
 * patch_flow_mode_active() - Report the selected patch flow visualization
 *
 * Returns the persisted selection.
 */
patch_flow_mode_t patch_flow_mode_active(void);

/**
 * patch_flow_mode_varies_with_phase() - Report whether @mode reads phase
 * @mode: selection to classify
 *
 * Returns TRUE when the selection resolves its direction from animation
 * phase, FALSE when it resolves from the patch phasor pair alone.
 */
gboolean patch_flow_mode_varies_with_phase(patch_flow_mode_t mode);

/**
 * render_patch_flow_idle() - Publish the selection without resolved content
 *
 * Returns a frame naming the active fill and mark, holding no entries.
 */
patch_flow_frame_t render_patch_flow_idle(void);

/**
 * render_patch_flow_frame() - Resolve one renderer-neutral patch flow frame
 * @fstep: frequency step holding authoritative patch phasors
 * @phase: effective patch-class phase in radians
 * @patch_colors: resolved patch colors indexed by patch
 * @color_generation: publication version of @patch_colors
 *
 * Returns a frame borrowing reusable managed resolver scratch.
 */
patch_flow_frame_t render_patch_flow_frame(int fstep, double phase,
    const rgb_f_t *patch_colors, uint32_t color_generation);

/**
 * render_patch_flow_free() - Release renderer-neutral patch flow scratch
 */
void render_patch_flow_free(void);

#endif /* __RENDER_PATCH_FLOW_H */
