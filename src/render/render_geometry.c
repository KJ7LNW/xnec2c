/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "render_geometry.h"
#include "render_structure_frame.h"

#include "../chroma/chroma_farfield.h"
#include "../chroma/chroma_nearfield.h"
#include "../anim/anim_class.h"
#include "../anim/near_field_anim.h"
#include "../rdpattern_ui.h"
#include "../shared.h"

/**
 * geom_walk_structure() - Emit structure endpoints and patch corners
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

/**
 * render_nearfield_fields() - Resolve active near-field vector sets
 * @fstep:       frequency step index
 * @frame_phase: phase read once by the frame owner
 * @sets:        receives active sets followed by an empty terminator
 */
  void
render_nearfield_fields(int fstep, double frame_phase,
    field_vector_set_t sets[NF_CHAN_NUM + 1])
{
  const near_field_t *nf = &near_field_fstep[fstep];
  nf_frame_mode_t mode = near_field_anim_frame_mode();
  double phase = anim_class_phase(ANIM_CLASS_NEAR_FIELD, frame_phase);
  int npts = fpat.nrx * fpat.nry * fpat.nrz;
  int count = 0;
  nf_channel_t channel;

  sets[0] = (field_vector_set_t){0};

  if( npts <= 0 || nf->points == NULL )
    return;

  for( channel = NF_CHAN_E; channel < NF_CHAN_NUM; channel++ )
  {
    field_vector_set_t set;

    if( !near_field_anim_channel_active(channel) )
      continue;

    set = chroma_proj_frame_nearfield(fstep, channel, mode, phase);
    if( set.entries == NULL )
      continue;

    sets[count] = set;
    count++;
  }

  sets[count] = (field_vector_set_t){0};

} /* render_nearfield_fields() */

/**
 * render_farfield_vectors() - Resolve the far-zone instantaneous field set
 * @fstep:       frequency step index
 * @frame_phase: phase read once by the frame owner
 * @ff:          far-field draw parameters supplying excitation translation
 * @set:         receives the resolved set or an empty set
 */
  void
render_farfield_vectors(int fstep, double frame_phase,
    const ff_draw_params_t *ff, field_vector_set_t *set)
{
  point_3d_t translation = {0};
  double phase;

  *set = (field_vector_set_t){0};

  if( !anim_class_active(ANIM_CLASS_FAR_FIELD) || ff_pre == NULL )
    return;

  phase = anim_class_phase(ANIM_CLASS_FAR_FIELD, frame_phase);

  if( fl_fgt(ff->off_len, FF_EXCITATION_OFFSET_MIN - FL_EPS) )
  {
    translation.x = ff->x;
    translation.y = ff->y;
    translation.z = ff->z;
  }

  *set = chroma_proj_frame_farfield(fstep, phase, &translation);

} /* render_farfield_vectors() */

/**
 * geom_walk_nearfield() - Emit active near-field segment endpoints
 * @fstep:       frequency step index
 * @frame_phase: phase read once by the frame owner
 * @sink:        point receiver
 * @user:        opaque sink context
 */
  static void
geom_walk_nearfield(int fstep, double frame_phase,
    render_geom_point_fn sink, void *user)
{
  field_vector_set_t sets[NF_CHAN_NUM + 1] = {{0}};
  int set_idx;
  int point_idx;

  render_nearfield_fields(fstep, frame_phase, sets);

  for( set_idx = 0; sets[set_idx].entries != NULL; set_idx++ )
  {
    const field_vector_entry_t *entries = sets[set_idx].entries;
    int count = mem_array_count(entries);

    for( point_idx = 0; point_idx < count; point_idx++ )
    {
      const point_3d_t *origin = &entries[point_idx].origin;
      const field_vector_t *vector = &entries[point_idx].vector;

      sink(user, origin->x, origin->y, origin->z, 1.0f);
      sink(user, origin->x + (double)vector->dx,
          origin->y + (double)vector->dy,
          origin->z + (double)vector->dz, 1.0f);
    }
  }

} /* geom_walk_nearfield() */

/**
 * render_geom_walk() - Emit the active view's drawn world geometry
 * @view:        view selecting structure, far-field, or near-field content
 * @frame_phase: phase read once by the frame owner
 * @sink:        receives each drawn vertex and its model scale
 * @user:        opaque pointer passed to @sink
 *
 * Returns the active content reference extent, or zero when unavailable.
 * The caller holds freq_data_lock.
 */
  float
render_geom_walk(view_t *view, double frame_phase,
    render_geom_point_fn sink, void *user)
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
      const near_field_t *nf = &near_field_fstep[result.fstep];

      if( nf->points == NULL )
        break;

      extent = (float)nf->r_max;
      geom_walk_nearfield(result.fstep, frame_phase, sink, user);
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
