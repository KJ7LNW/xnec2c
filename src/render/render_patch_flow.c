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

#include "render_patch_flow.h"
#include "render_patch_flow_resolve.h"
#include "../mem/mem.h"
#include "../prerender/prerender_color.h"
#include "../prerender/prerender_state.h"
#include "../shared.h"

/* Decompose one selection into its three orthogonal coordinates. */
typedef struct
{
  patch_fill_t           fill;
  patch_flow_direction_t direction;
  patch_flow_mark_t      mark;
} patch_flow_mode_row_t;

/* Derivation inputs of the published frame, compared and assigned whole. */
typedef struct
{
  int               fstep;
  int               patches;
  uint32_t          content;
  double            phase;
  patch_flow_mode_t mode;
  uint32_t          color_generation;
  gboolean          valid;
} patch_flow_edge_t;

static patch_flow_entry_t *patch_flow_entries;
static patch_flow_arrow_t *patch_flow_arrows;
static patch_flow_edge_t patch_flow_edge;
static unsigned int patch_flow_generation;

/* Decompose each selection into the three coordinates it fixes.  Fill
 * geometry, direction formula, and mark technique vary independently, so
 * every selection names one member of each axis. */
static const patch_flow_mode_row_t patch_flow_mode_rows[] = {
  [PATCH_FLOW_MODE_REFERENCE_PHASE] = {
      .fill      = PATCH_FILL_SOLID,
      .direction = PATCH_FLOW_DIRECTION_INSTANT,
      .mark      = PATCH_FLOW_MARK_DIRECTIONAL },
  [PATCH_FLOW_MODE_POLARIZATION_AXIS] = {
      .fill      = PATCH_FILL_SOLID,
      .direction = PATCH_FLOW_DIRECTION_POLARIZATION_AXIS,
      .mark      = PATCH_FLOW_MARK_BIDIRECTIONAL },
  [PATCH_FLOW_MODE_PEAK_MAGNITUDE] = {
      .fill      = PATCH_FILL_SOLID,
      .direction = PATCH_FLOW_DIRECTION_PEAK_MAGNITUDE,
      .mark      = PATCH_FLOW_MARK_DIRECTIONAL },
  [PATCH_FLOW_MODE_LIC] = {
      .fill      = PATCH_FILL_SOLID,
      .direction = PATCH_FLOW_DIRECTION_INSTANT,
      .mark      = PATCH_FLOW_MARK_LIC },
  [PATCH_FLOW_MODE_WIREFRAME] = {
      .fill      = PATCH_FILL_WIREFRAME,
      .direction = PATCH_FLOW_DIRECTION_INSTANT,
      .mark      = PATCH_FLOW_MARK_ARROW },
};

_Static_assert(G_N_ELEMENTS(patch_flow_mode_rows) == PATCH_FLOW_MODE_COUNT,
    "Patch flow mode rows must cover every selection");

/**
 * patch_flow_mode_active() - Report the selected patch flow visualization
 *
 * Returns the persisted selection.
 */
patch_flow_mode_t
patch_flow_mode_active(void)
{
  return (patch_flow_mode_t)rc_config.current_flow_visualization_mode;
}

/**
 * patch_flow_mode_row() - Report the row decomposing @mode
 * @mode: selection to decompose
 *
 * Returns the fill, direction, and mark coordinates @mode fixes, or NULL
 * when @mode names no selection.
 */
static const patch_flow_mode_row_t *
patch_flow_mode_row(patch_flow_mode_t mode)
{
  if( mode < PATCH_FLOW_MODE_REFERENCE_PHASE || mode >= PATCH_FLOW_MODE_COUNT )
  {
    BUG("Patch flow selection %d lies outside [0, %d)\n",
        mode, PATCH_FLOW_MODE_COUNT);
    return NULL;
  }

  return &patch_flow_mode_rows[mode];
}

/**
 * patch_flow_mode_varies_with_phase() - Report whether @mode reads phase
 * @mode: selection to classify
 *
 * The instantaneous formula is the one direction reading animation phase;
 * the polarization axis and peak magnitude formulas resolve from the patch
 * phasor pair alone.
 *
 * Returns TRUE when the selection resolves its direction from phase.
 */
gboolean
patch_flow_mode_varies_with_phase(patch_flow_mode_t mode)
{
  const patch_flow_mode_row_t *row = patch_flow_mode_row(mode);

  return (row != NULL)
      && (row->direction == PATCH_FLOW_DIRECTION_INSTANT);
}

/**
 * patch_flow_edge_eq() - Report two derivation snapshots as equal
 * @a: snapshot recorded by the last resolve
 * @b: snapshot the caller presents now
 *
 * Returns TRUE when both snapshots are filled and every derivation input
 * matches, so the published entries and arrows still stand.
 */
static gboolean
patch_flow_edge_eq(const patch_flow_edge_t *a, const patch_flow_edge_t *b)
{
  return a->valid && b->valid
      && a->fstep == b->fstep
      && a->patches == b->patches
      && a->content == b->content
      && a->mode == b->mode
      && a->color_generation == b->color_generation
      && dl_feq(a->phase, b->phase);
}

/**
 * resolve_patch_flow_scratch() - Refill the entry and arrow scratch
 * @fstep: frequency step holding authoritative patch phasors
 * @direction: formula selected by the active mode row
 * @phase: effective patch-class phase in radians
 * @patch_colors: resolved patch colors indexed by patch
 */
static void
resolve_patch_flow_scratch(int fstep, patch_flow_direction_t direction,
    double phase, const rgb_f_t *patch_colors)
{
  int arrow_count = 0;

  if( data.m > 0 )
  {
    int idx;

    mem_array_realloc(&patch_flow_entries, data.m);
    mem_array_realloc(&patch_flow_arrows, data.m);

    for( idx = 0; idx < data.m; idx++ )
    {
      resolve_patch_flow_entry(fstep, idx, direction, phase,
          &patch_flow_entries[idx]);
      arrow_count += resolve_patch_flow_arrow(&patch_flow_entries[idx],
          &geom_pre.patch_tangent_frame[idx], &patch_colors[idx], idx,
          &patch_flow_arrows[arrow_count]);
    }
  }
  else
  {
    mem_array_free(&patch_flow_entries);
  }

  if( arrow_count > 0 )
    mem_array_realloc(&patch_flow_arrows, arrow_count);
  else
    mem_array_free(&patch_flow_arrows);
}

/**
 * render_patch_flow_idle() - Publish the selection without resolved content
 *
 * A view presenting no patch current still draws the selected patch fill,
 * so the frame carries the selection while its arrays stay empty.
 *
 * Returns a frame naming the active fill and mark, holding no entries.
 */
patch_flow_frame_t
render_patch_flow_idle(void)
{
  const patch_flow_mode_row_t *mode_row =
    patch_flow_mode_row(patch_flow_mode_active());

  if( mode_row == NULL )
    return (patch_flow_frame_t){
        .fill = PATCH_FILL_COUNT,
        .mark = PATCH_FLOW_MARK_COUNT,
    };

  return (patch_flow_frame_t){
      .fill = mode_row->fill,
      .mark = mode_row->mark,
  };
}

/**
 * render_patch_flow_frame() - Resolve one renderer-neutral patch flow frame
 * @fstep: frequency step holding authoritative patch phasors
 * @phase: effective patch-class phase in radians
 * @patch_colors: resolved patch colors indexed by patch
 * @color_generation: publication version of @patch_colors
 *
 * Resolves the selected direction formula once per patch and appends a
 * world-space arrow for every patch above the arrow threshold, so both
 * engines consume one direction and one arrow list.  The publication tokens
 * the phasor producer and the color projection stamp enter the derivation
 * snapshot, so a frame repeating its inputs reuses the scratch, while a
 * rebake into the same scratch still rebuilds.
 *
 * Returns a frame borrowing reusable managed resolver scratch.
 */
patch_flow_frame_t
render_patch_flow_frame(int fstep, double phase, const rgb_f_t *patch_colors,
    uint32_t color_generation)
{
  patch_flow_mode_t mode = patch_flow_mode_active();
  const patch_flow_mode_row_t *mode_row = patch_flow_mode_row(mode);
  patch_flow_edge_t want;

  if( mode_row == NULL )
    return (patch_flow_frame_t){
        .fill = PATCH_FILL_COUNT,
        .mark = PATCH_FLOW_MARK_COUNT,
    };

  /* Phase leaves the snapshot for the formulas resolving from the stored
   * phasor pair alone, so scrubbing rebuilds nothing under those marks. */
  want = (patch_flow_edge_t){
      .fstep   = fstep,
      .patches = data.m,
      .content = (struct_colors == NULL) ? 0 : struct_colors[fstep].generation,
      .phase   = (mode_row->direction == PATCH_FLOW_DIRECTION_INSTANT)
          ? phase : 0.0,
      .mode    = mode,
      .color_generation = color_generation,
      .valid   = TRUE };

  if( !patch_flow_edge_eq(&patch_flow_edge, &want) )
  {
    resolve_patch_flow_scratch(fstep, mode_row->direction, phase,
        patch_colors);
    patch_flow_generation++;
    patch_flow_edge = want;
  }

  return (patch_flow_frame_t){
      .entries = patch_flow_entries,
      .arrows = patch_flow_arrows,
      .fill = mode_row->fill,
      .mark = mode_row->mark,
      .scroll_phase = phase,
      .generation = patch_flow_generation,
  };
}

/**
 * render_patch_flow_free() - Release renderer-neutral patch flow scratch
 */
void
render_patch_flow_free(void)
{
  mem_array_free(&patch_flow_entries);
  mem_array_free(&patch_flow_arrows);
  patch_flow_edge = (patch_flow_edge_t){0};
}
