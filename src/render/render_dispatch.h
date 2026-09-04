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

#ifndef RENDER_DISPATCH_H
#define RENDER_DISPATCH_H       1

#include "../common.h"
#include "../chroma/chroma_field.h"
#include "../prerender/prerender_color.h"
#include "../prerender/prerender_state.h"
#include "render_check.h"
#include "render_patch_flow.h"
#include "render_surface.h"
#include "gradient_cache.h"

/* Axis half-extent used for empty-scene placeholders (no geometry loaded) */
#define RENDER_EMPTY_AXIS_EXTENT 1.5f

/* Smallest excitation translation the pattern draw moves its content by */
#define FF_EXCITATION_OFFSET_MIN 0.001f

/* Dispatch-resolved structure draw parameters — passed to draw_structure backends */
typedef struct
{
  const rgb_f_t *wire_colors;   /* seg_rgb | composed projection colors */
  const float   *wire_seg_scale;  /* [data.n] dimensionless per-segment size gain */
  const rgb_f_t *patch_colors;  /* patch_rgb | composed projection colors */
  patch_flow_frame_t patch_flow; /* resolved patch directions and marks */
  const unsigned char *wire_glyphs; /* per-segment GLYPH_* code [data.n], or NULL */
  float          geometry_extent; /* unscaled structure-space half-extent */
  float          model_scale;   /* resolved structure-to-presentation scale */
  double         cmax;          /* fmax(wire_crnt_cmax, patch_crnt_cmax) or 0.0 */
  double         freq_mhz;      /* frequency for staleness detection */
  int            fstep;         /* for crnt_fstep[] access */
  uint32_t       color_generation; /* bumped whenever dispatch rebakes wire/patch color */
} struct_draw_params_t;

/* Dispatch-resolved far-field draw parameters — passed to draw_farfield backends.
 * Excitation centroid coordinates are pre-scaled to pattern space by dispatch.
 * Zero coordinates produce identity transform when no excitation center exists. */
typedef struct
{
  float x, y, z;          /* excitation center, pre-scaled to pattern space */
  float pattern_radius;    /* radiation pattern r_max (from ff_pre[fstep]) */
  float off_len;           /* sqrt(x²+y²+z²); clip extent = pattern_radius + off_len */
} ff_draw_params_t;

/* Frame-level colors resolved once per render() from the active theme. */
typedef struct
{
  rgb_f_t background;
  rgb_f_t view_axis;
  rgb_f_t view_axis_label;

} render_frame_colors_t;

/* Backend operations vtable — dispatch decides what to draw; backends draw it */
typedef struct render_ops_s
{
  /* Draw far-field gain pattern; returns TRUE on success, FALSE on data miss */
  gboolean (*draw_farfield)(render_surface_t *surface, int fstep,
      const ff_draw_params_t *ff);

  /* Draw field vectors, near E/H/Poynting or far-zone instantaneous field;
   * returns TRUE on success.  Backend walks the set list to its terminator,
   * one batch per set; r_max scales the view. */
  gboolean (*draw_field_vectors)(render_surface_t *surface,
      const field_vector_set_t *sets, double r_max);

  /* Draw structure geometry; returns TRUE always.
   * extent: content half-extent for projection scaling. */
  gboolean (*draw_structure)(render_surface_t *surface, float extent,
      const struct_draw_params_t *params);

  /* Draw structure geometry as secondary content behind a field view. */
  gboolean (*draw_structure_overlay)(render_surface_t *surface, float extent,
      const struct_draw_params_t *params);

  /* Initialize an empty scene (no geometry).  Optional entry: render()
   * tests presence, as the Cairo backend supplies none. */
  void (*init_empty)(render_surface_t *surface);

  /* Set the status message on the scene */
  void (*set_status)(render_surface_t *surface, const char *msg);

  /* Composite a pre-resolved gradient legend surface.
   * Called by render() when gradient_cache yields a valid surface;
   * never called otherwise — backends paint unconditionally. */
  void (*set_gradient)(render_surface_t *surface,
      const gradient_result_t *result);

  /* Deposit the frame colors render() resolved from the active theme */
  void (*set_colors)(render_surface_t *surface,
      const render_frame_colors_t *colors);

  /* Draw xyz axes for the primary content extent */
  void (*draw_axes)(render_surface_t *surface, float extent);

} render_ops_t;

/**
 * render_last_rdpattern_check() - Return the stored rdpattern precondition result
 *
 * Returns the render_check_result_t from the most recent render() call for
 * VIEW_RDPATTERN, which render() resolves under freq_data_lock.  The overlay
 * shift-scroll handler reads mode and overlay_active from it holding no
 * lock; both sites run on the GTK main thread.  Before the first rdpattern
 * render() the stored mode is RENDER_MODE_NONE.
 */
const render_check_result_t *render_last_rdpattern_check(void);

/**
 * render() - Unified render entry point for all backends
 * @surface: engine surface producing the frame; it carries the backend
 *           vtable through its engine and the view through its own field,
 *           whose type selects the mode and whose zoom scales the frame
 *
 * Acquires freq_data_lock, evaluates preconditions via render_check(), dispatches
 * through the ops vtable, and releases the lock.  Reads overlay scale from
 * rc_config.rdpattern_overlay_scale_adj.
 * Returns TRUE when a frame was produced, FALSE for freeze-frame.
 */
gboolean render(render_surface_t *surface);

#endif
