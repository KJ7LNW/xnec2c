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
 * render_dispatch: presentation-layer orchestration for all render views.
 *
 * Single entry point render() acquires freq_data_lock, evaluates all content
 * flags via render_check(), and calls through the render_ops_t vtable.
 * This module is the presentation layer: it resolves what to draw, resolves
 * resources (gradient cache), and drives backend leaf renderers.  Backends
 * (Cairo scenebuffer, OpenGL scene graph) are rendering-layer functions:
 * zero flag evaluation, zero lock management.
 */

#include "render_dispatch.h"
#include "render_geometry.h"
#include "render_message.h"
#include "render_structure_frame.h"
#include "gradient_cache.h"
#include "../anim/anim_phase.h"
#include "../shared.h"
#include "../prerender/prerender_farfield.h"
#include "../themes/theme.h"

/* Smallest overlay model scale that maps structure extent to pattern space */
#define OVERLAY_MODEL_SCALE_MIN 0.001f

/* Last render_check result for the rdpattern view; render() stores it on each rdpattern call */
static render_check_result_t last_rdpat_check;

/**
 * render_last_rdpattern_check() - Return the stored rdpattern precondition result
 *
 * Returns the result of the most recent render(VIEW_RDPATTERN) call, which
 * render() resolves while holding freq_data_lock.  The overlay shift-scroll
 * handler in rdpattern_ui.c reads mode and overlay_active from here holding
 * no lock; both sites run on the GTK main thread, so the read meets no
 * concurrent writer.
 *
 * mode is assigned only after every precondition passes, so a call that
 * produced no frame leaves RENDER_MODE_NONE for the reader to decline.
 */
const render_check_result_t *
render_last_rdpattern_check(void)
{
  return &last_rdpat_check;
}

/**
 * render_deposit_colors() - Resolve and deposit the colors of the active theme
 * @surface: engine surface receiving the colors
 */
  static void
render_deposit_colors(render_surface_t *surface)
{
  const render_ops_t *ops = surface->engine->render;
  const theme_t *theme = theme_active();
  render_frame_colors_t colors =
  {
    .background      = theme->colors[THEME_ROLE_BACKGROUND],
    .view_axis       = theme->colors[THEME_ROLE_VIEW_AXIS],
    .view_axis_label = theme->colors[THEME_ROLE_VIEW_AXIS_LABEL],
  };

  ops->set_colors(surface, &colors);

} /* render_deposit_colors() */

/*-----------------------------------------------------------------------*/

/**
 * render() - Produce one frame through the active engine
 * @surface: engine surface carrying the view and typed render operations
 *
 * Returns TRUE when a frame or status presentation was produced and FALSE
 * when the caller must retain the preceding frame.
 */
gboolean
render(render_surface_t *surface)
{
  const render_ops_t *ops;
  view_t *view;
  render_check_result_t r;
  double phase;
  gboolean ok = FALSE;

  if( surface == NULL || surface->view == NULL || surface->engine == NULL
      || surface->engine->render == NULL )
    return FALSE;

  ops = surface->engine->render;
  view = surface->view;

  render_deposit_colors(surface);

  if( isFlagSet(ERROR_CONDX) )
    return FALSE;

  g_rec_mutex_lock(&freq_data_lock);

  phase = anim_phase_get();
  r = render_check(view->type);

  /* r is immutable past this point; cache for external consumers */
  if( view->type == VIEW_RDPATTERN )
    last_rdpat_check = r;

  if( r.status == RENDER_SUPPRESS )
  {
    g_rec_mutex_unlock(&freq_data_lock);
    return FALSE;
  }

  if( r.status != RENDER_OK )
  {
    if( ops->init_empty != NULL )
      ops->init_empty(surface);
    ops->draw_axes(surface, RENDER_EMPTY_AXIS_EXTENT);
    ops->set_status(surface, r.message);
    g_rec_mutex_unlock(&freq_data_lock);
    return TRUE;
  }

  switch( r.mode )
  {
    case RENDER_MODE_FARFIELD:
    {
      ff_draw_params_t ff = { .x = 0.0f, .y = 0.0f, .z = 0.0f,
        .pattern_radius = 0.0f, .off_len = 0.0f };

      ff_presentation_recompute(r.fstep);
      ff.pattern_radius = (ff_pre != NULL) ? ff_pre[r.fstep].pattern_radius : 1.0f;

      /* model_scale maps structure-space meters to pattern-space units;
       * resolved once by the render presentation layer. */
      float model_scale = render_overlay_model_scale(r.fstep);

      /* Excitation translation resolved once; shared with the Cairo fit fold. */
      render_overlay_excitation_offset(model_scale, r.overlay_active, &ff);

      /* overlay_extent: structure-space extent that maps to the same pixel
       * positions as GL's model_scale matrix transform.
       * Derivation: p/R == p*model_scale/pattern_radius -> R = pattern_radius/model_scale */
      float overlay_extent = fl_fgt(model_scale, OVERLAY_MODEL_SCALE_MIN - FL_EPS)
          ? ff.pattern_radius / model_scale
          : (float)geom_pre.scene_radius;
      ops->draw_axes(surface, ff.pattern_radius);

      /* Deposit secondary structure before the primary pattern. */
      if( r.overlay_active )
      {
        struct_draw_params_t sparams =
            render_structure_frame(r.fstep, model_scale, phase);
        ops->draw_structure_overlay(surface, overlay_extent, &sparams);
      }

      ok = ops->draw_farfield(surface, r.fstep, &ff);

      /* Deposit the animated far-zone field onto the surface just drawn */
      if( ok )
      {
        field_vector_set_t sets[2] = {{0}};

        render_farfield_vectors(r.fstep, phase, &ff, &sets[0]);

        if( sets[0].entries != NULL )
          ok = ops->draw_field_vectors(surface, sets, ff.pattern_radius);
      }

      /* Resolve gradient legend for farfield mode; surface and version
       * travel as a cohesive result through the vtable to backends. */
      if( ok )
      {
        gradient_result_t gr = gradient_cache_get_overlay(
            view->width, view->height);

        if( gr.surface != NULL )
          ops->set_gradient(surface, &gr);
      }

      break;
    }

    case RENDER_MODE_NEARFIELD:
    {
      near_field_t *nf = &near_field_fstep[r.fstep];
      field_vector_set_t sets[NF_CHAN_NUM + 1];

      /* Near-field overlay: structure in meters, same space as field vectors */
      float nf_overlay_extent = (float)nf->r_max;

      render_nearfield_fields(r.fstep, phase, sets);
      ops->draw_axes(surface, nf_overlay_extent);

      if( r.overlay_active )
      {
        struct_draw_params_t sparams =
            render_structure_frame(r.fstep, 1.0f, phase);
        ops->draw_structure_overlay(surface, nf_overlay_extent, &sparams);
      }

      if( sets[0].entries != NULL )
        ok = ops->draw_field_vectors(surface, sets, nf->r_max);
      else
        ok = FALSE;

      break;
    }

    case RENDER_MODE_STRUCTURE:
    {
      struct_draw_params_t params =
          render_structure_frame(r.fstep, 1.0f, phase);

      ops->draw_axes(surface, params.geometry_extent);
      ok = ops->draw_structure(surface, params.geometry_extent, &params);
      break;
    }

    case RENDER_MODE_NONE:
    case RENDER_MODE_COUNT:
      BUG("render: unhandled mode %d\n", r.mode);
      ok = FALSE;
      break;
  }

  if( !ok )
  {
    /* Data dependency not satisfied (async compute, draw-style transition,
     * or transient buffer allocation failure).
     *
     * During optimization, freeze the previous frame to avoid flicker
     * while the freq loop is in flight. */
    if( isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
    {
      g_rec_mutex_unlock(&freq_data_lock);
      return FALSE;
    }

    /* Returning TRUE with an empty scene causes the render loop to
     * proceed to glClear, replacing stale content with a diagnostic.
     * Returning FALSE would skip the clear and freeze the last valid
     * frame on screen (desirable only during optimization above). */
    if( ops->init_empty != NULL )
      ops->init_empty(surface);
    ops->set_status(surface,
        freq_sweep_complete()
        ? STATUS_MSG_NOT_READY
        : STATUS_MSG_START_FREQLOOP);
    g_rec_mutex_unlock(&freq_data_lock);
    return TRUE;
  }

  g_rec_mutex_unlock(&freq_data_lock);
  return TRUE;
}

/*-----------------------------------------------------------------------*/
