/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "render_geometry.h"

#include "../chroma/chroma_nearfield.h"
#include "../rdpattern_ui.h"
#include "../shared.h"

typedef struct
{
  gboolean     active;
  nf_channel_t channel;

} nearfield_field_spec_t;

/** geom_walk_structure() - Emit structure endpoints and patch corners
 * @sink:  point receiver
 * @user:  opaque sink context
 * @scale: model scale applied by the renderer
 */
  static void
geom_walk_structure(render_geom_point_fn sink, void *user, float scale)
{
  int idx;

  for( idx = 0; idx < data.n; idx++ )
  {
    sink(user, (double)data.segments[idx].x1, (double)data.segments[idx].y1,
        (double)data.segments[idx].z1, scale);
    sink(user, (double)data.segments[idx].x2, (double)data.segments[idx].y2,
        (double)data.segments[idx].z2, scale);
  }

  if( geom_pre.patch_corners != NULL )
  {
    for( idx = 0; idx < data.m; idx++ )
    {
      const patch_corners_t *pc = &geom_pre.patch_corners[idx];

      sink(user, pc->c0x, pc->c0y, pc->c0z, scale);
      sink(user, pc->c1x, pc->c1y, pc->c1z, scale);
      sink(user, pc->c2x, pc->c2y, pc->c2z, scale);
      sink(user, pc->c3x, pc->c3y, pc->c3z, scale);
    }
  }

} /* geom_walk_structure() */

/** render_nearfield_fields() - Resolve active near-field vector sets
 * @fstep:  frequency step index
 * @fields: receives active vector and color pairs
 *
 * Returns the number of populated entries in @fields.
 */
  int
render_nearfield_fields(int fstep, nf_field_set_t fields[NF_FIELD_SETS_MAX])
{
  const nearfield_field_spec_t specs[NF_FIELD_SETS_MAX] =
  {
    { draw_efield_active() && (fpat.nfeh & NEAR_EFIELD), NF_CHAN_E },
    { draw_hfield_active() && (fpat.nfeh & NEAR_HFIELD), NF_CHAN_H },
    { draw_poynting_active() && (fpat.nfeh & NEAR_EFIELD)
        && (fpat.nfeh & NEAR_HFIELD), NF_CHAN_POV }
  };
  int count = 0;
  int idx;

  for( idx = 0; idx < NF_FIELD_SETS_MAX; idx++ )
  {
    nf_frame_t frame;

    if( !specs[idx].active )
      continue;

    frame = chroma_proj_frame_nearfield(fstep, specs[idx].channel);
    if( frame.vecs == NULL )
      continue;

    fields[count].vecs = frame.vecs;
    fields[count].colors = frame.colors;
    count++;
  }

  return count;

} /* render_nearfield_fields() */

/** geom_walk_nearfield() - Emit active near-field segment endpoints
 * @fstep: frequency step index
 * @sink:  point receiver
 * @user:  opaque sink context
 */
  static void
geom_walk_nearfield(int fstep, render_geom_point_fn sink, void *user)
{
  const near_field_t *nf = &near_field_fstep[fstep];
  nf_field_set_t fields[NF_FIELD_SETS_MAX] = {{0}};
  int n_fields = render_nearfield_fields(fstep, fields);
  int npts = fpat.nrx * fpat.nry * fpat.nrz;
  int field_idx;
  int point_idx;

  for( field_idx = 0; field_idx < n_fields; field_idx++ )
  {
    const nf_vector_t *vecs = fields[field_idx].vecs;

    for( point_idx = 0; point_idx < npts; point_idx++ )
    {
      const near_field_point_t *point = &nf->points[point_idx];

      sink(user, point->px, point->py, point->pz, 1.0f);
      sink(user, point->px + (double)vecs[point_idx].dx,
          point->py + (double)vecs[point_idx].dy,
          point->pz + (double)vecs[point_idx].dz, 1.0f);
    }
  }

} /* geom_walk_nearfield() */

/** render_geom_walk() - Emit the active view's drawn world geometry
 * @view: view selecting structure, far-field, or near-field content
 * @sink: receives each drawn vertex and its model scale
 * @user: opaque pointer passed to @sink
 *
 * Returns the active content reference extent, or zero when unavailable.
 * The caller holds freq_data_lock.
 */
  float
render_geom_walk(view_t *view, render_geom_point_fn sink, void *user)
{
  render_check_result_t result = render_check(view->type);
  float extent = 0.0f;
  int idx;

  if( result.status != RENDER_OK )
    return 0.0f;

  switch( result.mode )
  {
    case RENDER_MODE_FARFIELD:
    {
      ff_pre_t *farfield = (ff_pre != NULL) ? &ff_pre[result.fstep] : NULL;
      ff_draw_params_t params = {0};
      float model_scale;
      int nverts;

      if( farfield == NULL || farfield->vertices == NULL )
        break;

      model_scale = render_overlay_model_scale(result.fstep);
      render_overlay_excitation_offset(model_scale, result.overlay_active,
          &params);
      extent = farfield->pattern_radius;
      nverts = mem_array_count(farfield->vertices);

      for( idx = 0; idx < nverts; idx++ )
        sink(user, farfield->vertices[idx].x + (double)params.x,
            farfield->vertices[idx].y + (double)params.y,
            farfield->vertices[idx].z + (double)params.z, 1.0f);

      if( result.overlay_active )
        geom_walk_structure(sink, user, model_scale);
      break;
    }

    case RENDER_MODE_NEARFIELD:
    {
      const near_field_t *nearfield = &near_field_fstep[result.fstep];

      if( nearfield->points == NULL )
        break;

      extent = (float)nearfield->r_max;
      geom_walk_nearfield(result.fstep, sink, user);
      if( result.overlay_active )
        geom_walk_structure(sink, user, 1.0f);
      break;
    }

    case RENDER_MODE_STRUCTURE:
      extent = (float)geom_pre.scene_radius;
      geom_walk_structure(sink, user, 1.0f);
      break;

    case RENDER_MODE_NONE:
    case RENDER_MODE_COUNT:
      BUG("render_geom_walk: unresolved render mode %d\n", result.mode);
      break;
  }

  return extent;

} /* render_geom_walk() */
