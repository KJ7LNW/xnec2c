/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <float.h>
#include <math.h>

#include "opengl_view_fit_geometry.h"
#include "opengl_view_render.h"

#ifdef HAVE_OPENGL

/* Reserve a fraction of a pixel for the float matrix and pan conversion. */
#define GL_FIT_ROUNDING_PX 0.25

/* Refine the center to a fraction of the rounding allowance. */
#define GL_FIT_CENTER_TOLERANCE_PX (GL_FIT_ROUNDING_PX / 16.0)

typedef struct
{
  double aperture;
  double lower_term, upper_term;
  double pan, projected_min, projected_max;

} gl_fit_axis_t;

typedef struct
{
  gl_fit_axis_t axes[2];
  double front, distance;

} gl_fit_perspective_t;


/** gl_fit_batch_vertex() - Rotate and scale one batch vertex
 * @view:        view supplying the rotation basis
 * @position:    vertex position at the start of an interleaved record
 * @model_scale: scale applied to the renderable model
 * @x:           receives rotated x
 * @y:           receives rotated y
 * @z:           receives rotated z
 */
  static inline void
gl_fit_batch_vertex(view_t *view, const float *position, float model_scale,
    float *x, float *y, float *z)
{
  double screen_x;
  double screen_y;
  float screen_z;

  Project_on_Screen(view, (double)position[0], (double)position[1],
      (double)position[2], &screen_x, &screen_y, &screen_z);

  *x = (float)screen_x * model_scale;
  *y = (float)screen_y * model_scale;
  *z = screen_z * model_scale;

} /* gl_fit_batch_vertex() */

/** gl_fit_renderable_content() - Return active foldable renderable content
 * @renderable: renderable supplying optional content and activity capabilities
 */
  static const gl_view_content_t *
gl_fit_renderable_content(const gl_renderable_t *renderable)
{
  if( renderable->get_content == NULL )
    return NULL;

  if( renderable->is_active != NULL
      && !renderable->is_active(renderable->ctx) )
    return NULL;

  return renderable->get_content(renderable->ctx);

} /* gl_fit_renderable_content() */

typedef void (*gl_fit_vertex_fn)(void *user, float x, float y, float z);

/** gl_fit_walk_vertices() - Feed active rendered vertices to a private sink */
  static gboolean
gl_fit_walk_vertices(view_t *view, gl_view_state_t *state,
    gl_fit_vertex_fn sink, void *user)
{
  guint renderable_idx;

  if( view == NULL || state == NULL || state->renderables == NULL
      || sink == NULL )
    return FALSE;

  for( renderable_idx = 0; renderable_idx < state->renderables->len;
      renderable_idx++ )
  {
    const gl_renderable_t *renderable = &g_array_index(state->renderables,
        gl_renderable_t, renderable_idx);
    const gl_view_content_t *content =
        gl_fit_renderable_content(renderable);
    int batch_idx;

    if( content == NULL )
      continue;

    for( batch_idx = 0; batch_idx < content->batch_count; batch_idx++ )
    {
      const gl_draw_batch_t *batch = &content->batches[batch_idx];
      const char *base = (const char *)batch->vertices;
      int vertex_idx;

      if( base == NULL )
        continue;

      for( vertex_idx = 0; vertex_idx < batch->vertex_count; vertex_idx++ )
      {
        const float *position = (const float *)(base
            + (size_t)vertex_idx * (size_t)content->layout->stride);
        float x;
        float y;
        float z;

        gl_fit_batch_vertex(view, position, content->model_scale, &x, &y, &z);
        sink(user, x, y, z);
      }
    }
  }

  return TRUE;

} /* gl_fit_walk_vertices() */

/** gl_fit_sink_box() - Fold one rotated vertex into a bounding box */
  static void
gl_fit_sink_box(void *user, float x, float y, float _z)
{
  render_fit_add((render_fit_acc_t *)user, x, y);

} /* gl_fit_sink_box() */

/** gl_view_fit_fold_box() - Fold active vertices into a rotated box */
  gboolean
gl_view_fit_fold_box(view_t *view, gl_view_state_t *state, render_fit_acc_t *acc)
{
  return gl_fit_walk_vertices(view, state, gl_fit_sink_box, acc);

} /* gl_view_fit_fold_box() */

/** gl_fit_sink_limits() - Fold the affine pan limits of one vertex */
  static void
gl_fit_sink_limits(void *user, float x, float y, float z)
{
  gl_fit_perspective_t *ctx = user;
  const float lateral[] = { x, y };
  size_t i;

  ctx->front = fmax(ctx->front, (double)z);
  for( i = 0; i < G_N_ELEMENTS(ctx->axes); i++ )
  {
    gl_fit_axis_t *axis = &ctx->axes[i];
    double depth_term = axis->aperture * (double)z;

    // At distance d, the pan is bounded by -a*d+lower and a*d+upper.
    axis->lower_term = fmax(axis->lower_term, depth_term - (double)lateral[i]);
    axis->upper_term = fmin(axis->upper_term, -depth_term - (double)lateral[i]);
  }

} /* gl_fit_sink_limits() */

/** gl_fit_sink_projected() - Fold final perspective extrema for trial pan */
  static void
gl_fit_sink_projected(void *user, float x, float y, float z)
{
  gl_fit_perspective_t *ctx = user;
  const float lateral[] = { x, y };
  double depth = ctx->distance - (double)z;
  size_t i;

  for( i = 0; i < G_N_ELEMENTS(ctx->axes); i++ )
  {
    gl_fit_axis_t *axis = &ctx->axes[i];
    double projected = ((double)lateral[i] + axis->pan) / depth;

    axis->projected_min = fmin(axis->projected_min, projected);
    axis->projected_max = fmax(axis->projected_max, projected);
  }

} /* gl_fit_sink_projected() */

/** gl_fit_center_perspective() - Balance projected edges without exceeding the pan limits */
  static void
gl_fit_center_perspective(gl_view_state_t *state, gl_fit_perspective_t *ctx,
    double world_tolerance)
{
  double low[G_N_ELEMENTS(ctx->axes)], high[G_N_ELEMENTS(ctx->axes)];
  double span;
  size_t i;

  // Recover the feasible interval at the camera's representable distance.
  for( i = 0; i < G_N_ELEMENTS(ctx->axes); i++ )
  {
    const gl_fit_axis_t *axis = &ctx->axes[i];
    double extent = axis->aperture * ctx->distance;

    low[i] = axis->lower_term - extent;
    high[i] = axis->upper_term + extent;
  }

  // The sum of the final projected edges increases monotonically with pan.
  do
  {
    span = 0.0;
    for( i = 0; i < G_N_ELEMENTS(ctx->axes); i++ )
    {
      ctx->axes[i].pan = 0.5 * (low[i] + high[i]);
      ctx->axes[i].projected_min = HUGE_VAL;
      ctx->axes[i].projected_max = -HUGE_VAL;
    }
    gl_fit_walk_vertices(state->base.view, state, gl_fit_sink_projected, ctx);
    for( i = 0; i < G_N_ELEMENTS(ctx->axes); i++ )
    {
      const gl_fit_axis_t *axis = &ctx->axes[i];

      if( axis->projected_min + axis->projected_max > 0.0 )
        high[i] = axis->pan;
      else
        low[i] = axis->pan;
      span = fmax(span, high[i] - low[i]);
    }
  } while( span > world_tolerance );

} /* gl_fit_center_perspective() */

/** gl_view_fit_perspective() - Fit the geometry after perspective pan and division */
  gboolean
gl_view_fit_perspective(gl_view_state_t *state, view_fit_t *fit)
{
  view_t *view = state->base.view;
  float tangent = tanf(state->fov_rad * 0.5f);
  float aspect = (float)view->width / (float)view->height;
  double half_width = (double)view->width * 0.5;
  double half_height = (double)view->height * 0.5;
  double border = (double)RENDER_FIT_BORDER_PX + GL_FIT_ROUNDING_PX;
  float camera_base = state->content.r_max * GL_VIEW_BASE_DISTANCE_FACTOR;
  gl_fit_perspective_t ctx =
  {
    .front = -HUGE_VAL,
    .distance = (double)camera_base * (double)FLT_EPSILON,
  };
  float zoom, distance, pan_scale;
  size_t i;

  if( !fl_fgt(tangent, 0.0f) || !fl_fgt(state->content.r_max, 0.0f)
      || !dl_fgt(half_width, border) || !dl_fgt(half_height, border) )
    return FALSE;

  // Locate the first distance where both pan intervals are nonempty.
  ctx.axes[0].aperture = (double)(tangent * aspect) * (1.0 - border / half_width);
  ctx.axes[1].aperture = (double)tangent * (1.0 - border / half_height);
  for( i = 0; i < G_N_ELEMENTS(ctx.axes); i++ )
  {
    ctx.axes[i].lower_term = -HUGE_VAL;
    ctx.axes[i].upper_term = HUGE_VAL;
  }
  if( !gl_fit_walk_vertices(view, state, gl_fit_sink_limits, &ctx)
      || !isfinite(ctx.front) )
    return FALSE;
  for( i = 0; i < G_N_ELEMENTS(ctx.axes); i++ )
  {
    const gl_fit_axis_t *axis = &ctx.axes[i];

    ctx.distance = fmax(ctx.distance,
        (axis->lower_term - axis->upper_term) / (2.0 * axis->aperture));
  }
  ctx.distance = gl_view_near_fit_distance(state, ctx.distance, ctx.front);

  // Round zoom outward, then center using the distance the renderer obtains.
  zoom = nextafterf((float)((double)camera_base / ctx.distance), 0.0f);
  distance = camera_base / zoom;
  ctx.distance = (double)distance;
  pan_scale = 2.0f * distance * tangent / (float)view->height;
  gl_fit_center_perspective(state, &ctx, GL_FIT_CENTER_TOLERANCE_PX
      * (ctx.distance - ctx.front) * (double)tangent / half_height);
  fit->zoom = zoom;
  for( i = 0; i < G_N_ELEMENTS(ctx.axes); i++ )
    fit->pan_offset[i] = (float)(ctx.axes[i].pan / (double)pan_scale);

  return TRUE;

} /* gl_view_fit_perspective() */

#endif /* HAVE_OPENGL */
