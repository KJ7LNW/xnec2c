/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <math.h>

#include "opengl_view_fit_geometry.h"

#ifdef HAVE_OPENGL

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

typedef struct
{
  render_fit_acc_t    *acc;
  const render_proj_t *projection;
  float                center_x;
  float                center_y;

} gl_fit_perspective_ctx_t;

typedef struct
{
  render_fit_acc_t    *acc;
  const render_proj_t *projection;
  float                distance;
  float                tangent;

} gl_fit_ndc_ctx_t;

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
            + (size_t)vertex_idx * (size_t)content->vertex_stride);
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

/** gl_fit_sink_perspective() - Fold one perspective distance requirement */
  static void
gl_fit_sink_perspective(void *user, float x, float y, float z)
{
  gl_fit_perspective_ctx_t *ctx = user;

  render_fit_add_persp(ctx->acc, ctx->center_x, ctx->center_y,
      x, y, z, ctx->projection);

} /* gl_fit_sink_perspective() */

/** gl_fit_sink_ndc() - Fold one perspective-projected vertex */
  static void
gl_fit_sink_ndc(void *user, float x, float y, float z)
{
  gl_fit_ndc_ctx_t *ctx = user;
  float denominator = ctx->distance - z;

  if( fl_flt(denominator, FL_EPS) )
    denominator = FL_EPS;

  render_fit_add(ctx->acc,
      x / (ctx->tangent * ctx->projection->aspect * denominator),
      y / (ctx->tangent * denominator));

} /* gl_fit_sink_ndc() */

/** gl_view_fit_fold_box() - Fold active vertices into a rotated box */
  gboolean
gl_view_fit_fold_box(view_t *view, gl_view_state_t *state,
    render_fit_acc_t *acc)
{
  return gl_fit_walk_vertices(view, state, gl_fit_sink_box, acc);

} /* gl_view_fit_fold_box() */

/** gl_view_fit_fold_perspective() - Fold perspective distance requirements */
  gboolean
gl_view_fit_fold_perspective(view_t *view, gl_view_state_t *state,
    const render_proj_t *projection, render_fit_acc_t *acc,
    float center_x, float center_y)
{
  gl_fit_perspective_ctx_t ctx =
  {
    .acc = acc,
    .projection = projection,
    .center_x = center_x,
    .center_y = center_y
  };

  return gl_fit_walk_vertices(view, state, gl_fit_sink_perspective, &ctx);

} /* gl_view_fit_fold_perspective() */

/** gl_view_fit_fold_ndc() - Fold projected normalized-device coordinates */
  gboolean
gl_view_fit_fold_ndc(view_t *view, gl_view_state_t *state,
    const render_proj_t *projection, float distance, render_fit_acc_t *acc)
{
  gl_fit_ndc_ctx_t ctx =
  {
    .acc = acc,
    .projection = projection,
    .distance = distance,
    .tangent = tanf(projection->fov_rad * 0.5f)
  };

  if( !fl_fgt(ctx.tangent, 0.0f) )
    return FALSE;

  return gl_fit_walk_vertices(view, state, gl_fit_sink_ndc, &ctx);

} /* gl_view_fit_fold_ndc() */

#endif /* HAVE_OPENGL */
