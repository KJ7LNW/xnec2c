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
#include "gradient_cache.h"
#include "../shared.h"
#include "../chroma/chroma.h"
#include "../chroma/chroma_nearfield.h"
#include "../prerender/prerender_farfield.h"
#include "../structure_ui.h"
#include "../themes/theme.h"

/* Last render_check result for the rdpattern view; updated by render() on every call */
static render_check_result_t last_rdpat_check;

/**
 * render_check_rdpat() - Return cached rdpattern precondition result
 *
 * Returns a pointer to the result of the most recent render(VIEW_RDPATTERN)
 * call.  Consumers (overlay provider, shift-scroll handler, draw handlers)
 * read mode and overlay_active from this cache instead of re-evaluating
 * content-selection flags.  Valid after the first rdpattern render() call.
 */
const render_check_result_t *
render_check_rdpat(void)
{
  return &last_rdpat_check;
}

/*-----------------------------------------------------------------------*/

/**
 * render_overlay_model_scale() - Resolve the effective overlay model scale
 * @fstep: frequency step index
 *
 * Owns the derived product of the per-fstep prerender base scale and the
 * interactive scale_adj; every engine consumes this value rather than
 * recomputing it.
 */
float
render_overlay_model_scale(int fstep)
{
  if( ff_pre == NULL || fstep < 0 )
    return 1.0f;

  return ff_pre[fstep].overlay_base_scale
      * (float)rc_config.rdpattern_overlay_scale_adj;
}

/*-----------------------------------------------------------------------*/

/**
 * render_overlay_excitation_offset() - Resolve the far-field excitation translation
 * @model_scale:    resolved overlay model scale for the fstep
 * @overlay_active: whether the structure overlay is shown
 * @ff:             receives the pattern-space offset in x,y,z and its length
 *                  off_len; all zeroed when no excitation translation applies
 *
 * Owns the derived excitation centroid prescale so the pattern draw and the fit
 * fold consume one authoritative translation.
 */
void
render_overlay_excitation_offset(float model_scale, gboolean overlay_active,
    ff_draw_params_t *ff)
{
  if( !overlay_active || !isFlagSet(ENABLE_EXCITN) )
  {
    ff->x = 0.0f;
    ff->y = 0.0f;
    ff->z = 0.0f;
    ff->off_len = 0.0f;
    return;
  }

  ff->x = (float)geom_pre.excitation_cx * model_scale;
  ff->y = (float)geom_pre.excitation_cy * model_scale;
  ff->z = (float)geom_pre.excitation_cz * model_scale;
  ff->off_len = sqrtf(ff->x * ff->x + ff->y * ff->y + ff->z * ff->z);
}

/*-----------------------------------------------------------------------*/

  static const char *
render_rdpattern_mode_message(void)
{
  gboolean has_rp = isFlagSet(ENABLE_RDPAT);
  gboolean has_nf = isFlagSet(ENABLE_NEAREH);

  if( !has_rp && !has_nf )
    return STATUS_MSG_NO_RP_NO_NEAREH;

  if( !has_rp )
    return STATUS_MSG_SELECT_NEARFIELD;

  if( !has_nf )
    return STATUS_MSG_SELECT_GAINPAT;

  return STATUS_MSG_SELECT_MODE;
}

/*-----------------------------------------------------------------------*/

/** render_check_nearfield() - Resolve near-field preconditions
 * @r: result struct with fstep already set; populated on return
 *
 * Near E/H field mode is already confirmed active by caller.
 */
  static void
render_check_nearfield(render_check_result_t *r)
{
  /* Near-field view needs at least one field component selected; with none
   * selected the draw yields nothing, so direct the user to enable one. */
  if( !draw_efield_active() && !draw_hfield_active() &&
      !draw_poynting_active() )
  {
    r->status = RENDER_NO_NF_FIELD;
    r->message = STATUS_MSG_SELECT_NF_FIELD;
    return;
  }

  if( isFlagSet(ENABLE_NEAREH) && NF_FSTEP_AVAILABLE(r->fstep) )
  {
    r->mode = RENDER_MODE_NEARFIELD;
    return;
  }

  if( isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    r->status = RENDER_SUPPRESS;
    return;
  }

  if( isFlagClear(ENABLE_NEAREH) )
  {
    r->status = RENDER_NO_NF_CARD;
    r->message = STATUS_MSG_NO_NEAREH_CARDS;
  }
  else
  {
    r->status = RENDER_NF_NOT_READY;
    r->message = STATUS_MSG_START_FREQLOOP;
  }
}

/*-----------------------------------------------------------------------*/

/** render_check_farfield() - Resolve far-field preconditions
 * @r: result struct with fstep already set; populated on return
 *
 * Far-field gain mode is already confirmed active by caller.
 */
  static void
render_check_farfield(render_check_result_t *r)
{
  if( isFlagClear(ENABLE_RDPAT) )
  {
    r->status = RENDER_NO_RP_CARD;
    r->message = STATUS_MSG_NO_RP_CARD;
    return;
  }

  if( r->fstep < 0 )
  {
    r->status = RENDER_NO_DATA;
    r->message = STATUS_MSG_NO_RDPAT_DATA;
    return;
  }

  r->mode = RENDER_MODE_FARFIELD;
}

/*-----------------------------------------------------------------------*/

/** render_check_rdpattern() - Resolve radiation pattern mode and preconditions
 * @r: result struct with fstep already set; populated on return
 *
 * Near-field takes priority over far-field.
 */
  static void
render_check_rdpattern(render_check_result_t *r)
{
  if(rdpat_ehfield_active())
    render_check_nearfield(r);
  else if(rdpat_gain_active())
    render_check_farfield(r);
  else
  {
    r->status = RENDER_NO_MODE;
    r->message = render_rdpattern_mode_message();
  }

  r->overlay_active = overlay_struct_active();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

  render_check_result_t
render_check(view_type_t view_type)
{
  render_check_result_t r = { .status = RENDER_OK, .mode = RENDER_MODE_NONE,
    .fstep = -1, .message = NULL, .overlay_active = FALSE };

  /* No content available while input file is being parsed */
  if( isFlagSet(INPUT_PENDING) )
  {
    r.status = RENDER_NO_GEOMETRY;
    r.message = STATUS_MSG_OPEN_FILE;
    return r;
  }

  r.fstep = calc_data.freq_step;

  if( view_type == VIEW_STRUCTURE )
  {
    if( data.n == 0 && data.m == 0 )
    {
      r.status = RENDER_NO_GEOMETRY;
      r.message = STATUS_MSG_OPEN_FILE;
      return r;
    }
    r.mode = RENDER_MODE_STRUCTURE;
    return r;
  }

  render_check_rdpattern(&r);
  return r;
}

/*-----------------------------------------------------------------------*/

/** build_struct_draw_params() - Resolve structure draw colors from current flags
 * @fstep: frequency step index
 * @model_scale: structure-to-presentation scale resolved by the caller
 *
 * Selects wire_colors and patch_colors from precomputed struct_colors
 * per the current structure view (currents or charges), or falls back to
 * geometry-mode seg_rgb / patch_rgb.
 */
  static struct_draw_params_t
build_struct_draw_params(int fstep, float model_scale)
{
  struct_draw_params_t params;
  int fs = fstep;

  chroma_proj_t proj = color_proj_active();
  color_tone_t fam = color_tone_active();

  if(struct_view_currents() && CRNT_FSTEP_AVAILABLE(fs) && struct_colors )
  {
    params.wire_colors  = chroma_proj_frame_wire(fs, (double)flow_phase,
        proj, fam, CHAN_CURRENT);
    params.wire_widths  = chroma_proj_frame_wire_widths(fs, proj, fam,
        CHAN_CURRENT);
    params.patch_colors = chroma_proj_frame_patch(fs, (double)flow_phase,
        proj, fam);
    params.wire_glyphs  = chroma_proj_frame_wire_glyphs(fs, proj, fam,
        CHAN_CURRENT);
    params.cmax = fmax((double)struct_colors[fs].wire_crnt_cmax,
                       (double)struct_colors[fs].patch_crnt_cmax);
    params.show_flow = TRUE;
    params.color_generation = chroma_proj_generation();
  }
  else if(struct_view_charges() && CRNT_FSTEP_AVAILABLE(fs) && struct_colors )
  {
    /* Patches carry no charge quantity; fill stays the static geometry color */
    params.wire_colors  = chroma_proj_frame_wire(fs, (double)flow_phase,
        proj, fam, CHAN_CHARGE);
    params.wire_widths  = chroma_proj_frame_wire_widths(fs, proj, fam,
        CHAN_CHARGE);
    params.patch_colors = patch_rgb;
    params.wire_glyphs  = chroma_proj_frame_wire_glyphs(fs, proj, fam,
        CHAN_CHARGE);
    params.cmax = (double)struct_colors[fs].wire_chrg_cmax;
    params.show_flow = FALSE;
    params.color_generation = chroma_proj_generation();
  }
  else
  {
    params.wire_colors  = seg_rgb;
    params.wire_widths  = seg_width;
    params.patch_colors = patch_rgb;
    params.wire_glyphs  = NULL;
    params.cmax = 0.0;
    params.show_flow = FALSE;
    params.color_generation = 0;
  }

  params.geometry_extent = (float)geom_pre.scene_radius;
  params.model_scale = model_scale;
  params.fstep = fs;
  params.freq_mhz = calc_data.freq_mhz;
  return params;
}

/*-----------------------------------------------------------------------*/

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

  gboolean
render(render_surface_t *surface)
{
  const render_ops_t *ops;
  view_t *view;
  render_check_result_t r;
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
      float overlay_extent = (model_scale > 0.001f)
          ? ff.pattern_radius / model_scale
          : (float)geom_pre.scene_radius;
      ops->draw_axes(surface, ff.pattern_radius);

      /* Deposit secondary structure before the primary pattern. */
      if( r.overlay_active )
      {
        struct_draw_params_t sparams =
            build_struct_draw_params(r.fstep, model_scale);
        ops->draw_structure_overlay(surface, overlay_extent, &sparams);
      }

      ok = ops->draw_farfield(surface, r.fstep, &ff);

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
      int npts = fpat.nrx * fpat.nry * fpat.nrz;
      nf_field_set_t fields[NF_FIELD_SETS_MAX] = {{0}};
      int n_fields = render_nearfield_fields(r.fstep, fields);
      double dr = geom_pre.nf_dr_norm;

      /* Near-field overlay: structure in meters, same space as field vectors */
      float nf_overlay_extent = (float)nf->r_max;
      ops->draw_axes(surface, nf_overlay_extent);

      if( r.overlay_active )
      {
        struct_draw_params_t sparams =
            build_struct_draw_params(r.fstep, 1.0f);
        ops->draw_structure_overlay(surface, nf_overlay_extent, &sparams);
      }

      if( n_fields > 0 )
      {
        ok = ops->draw_nearfield(surface, nf->points, npts,
            fields, n_fields, dr, nf->r_max);
      }
      else
        ok = FALSE;

      break;
    }

    case RENDER_MODE_STRUCTURE:
    {
      struct_draw_params_t params = build_struct_draw_params(r.fstep, 1.0f);
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
