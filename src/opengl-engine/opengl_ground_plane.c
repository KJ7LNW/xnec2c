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

#include "opengl_ground_plane.h"
#include "opengl_view.h"
#include "opengl_view_peel.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_get_alpha() - Classification alpha from rc_config */
float
opengl_ground_plane_get_alpha(void *ctx)
{
  (void)ctx;
  return TRANSPARENCY_TO_ALPHA(rc_config.transparency_ground_plane);
}

#define GROUND_PLANE_EXTENT 100.0f
#define GROUND_PLANE_VERTICES 6

static const rgba_f_t ground_color = {0.4f, 0.7f, 0.3f, 1.0f};
static const point_f_3d_t ground_normal = {0.0f, 0.0f, 1.0f};

/*-----------------------------------------------------------------------*/

/** generate_ground_plane_vertices() - Create two triangles forming a quad in the XY plane at Z=0
 * @vertices: output vertex buffer (must hold GROUND_PLANE_VERTICES entries)
 * @scale: uniform scale factor applied to GROUND_PLANE_EXTENT
 *
 * All vertices have upward normals and green color.
 */
  static void
generate_ground_plane_vertices(lit_color_point_t *vertices, float scale)
{
  /* XY-plane offsets for two triangles forming a quad:
   * triangle 1: (-,-), (-,+), (+,-); triangle 2: (-,+), (+,+), (+,-) */
  static const float ground_quad_offsets[GROUND_PLANE_VERTICES][2] = {
    {-1.0f, -1.0f}, {-1.0f, +1.0f}, {+1.0f, -1.0f},
    {-1.0f, +1.0f}, {+1.0f, +1.0f}, {+1.0f, -1.0f}
  };
  float extent;
  int i;

  extent = scale * GROUND_PLANE_EXTENT;

  for( i = 0; i < GROUND_PLANE_VERTICES; i++ )
  {
    vertices[i].point.x = extent * ground_quad_offsets[i][0];
    vertices[i].point.y = extent * ground_quad_offsets[i][1];
    vertices[i].point.z = 0.0f;
    vertices[i].normal = ground_normal;
    vertices[i].color = ground_color;
  }

} /* generate_ground_plane_vertices() */

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_new() - Allocate and initialize ground plane rendering context
 *
 * Sets up VAO, VBO, and checkerboard shader.
 * Returns allocated context, or NULL on shader load failure.
 */
  opengl_ground_plane_t*
opengl_ground_plane_new(void)
{
  opengl_ground_plane_t *gp;
  lit_color_point_t vertices[GROUND_PLANE_VERTICES];
  gboolean ok;
  int i;
  const char *attrib_names[3] = {"position", "normal", "color"};

  gp = g_new0(opengl_ground_plane_t, 1);

  if( !gp )
    return( NULL );

  ok = gl_shader_load(&gp->shader,
      "/gl/ground-plane-vertex.glsl",
      "/gl/ground-plane-fragment.glsl");

  if( !ok )
  {
    pr_err("Failed to load ground plane shaders\n");
    g_free(gp);
    return( NULL );
  }

  gp->mvp_location = glGetUniformLocation(gp->shader.program, "mvp");
  gp->u_alpha_location = glGetUniformLocation(gp->shader.program, "u_alpha");
  gp->u_color_dim_location = glGetUniformLocation(gp->shader.program, "u_color_dim");
  gl_view_peel_locs_init(&gp->peel_locs, gp->shader.program);

  for( i = 0; i < 3; i++ )
  {
    gp->attrib_locations[i] = glGetAttribLocation(
        gp->shader.program,
        attrib_names[i]);
  }

  glGenVertexArrays(1, &gp->vao);
  glGenBuffers(1, &gp->vbo);

  glBindVertexArray(gp->vao);
  glBindBuffer(GL_ARRAY_BUFFER, gp->vbo);

  /* Generate initial vertices with unit scale */
  generate_ground_plane_vertices(vertices, 1.0f);

  glBufferData(GL_ARRAY_BUFFER,
      sizeof(vertices),
      vertices,
      GL_DYNAMIC_DRAW);

  /* Configure vertex attrib pointers in VAO — retained for all renders */
  glEnableVertexAttribArray(gp->attrib_locations[0]);
  glVertexAttribPointer(gp->attrib_locations[0], 3, GL_FLOAT, GL_FALSE,
      sizeof(lit_color_point_t), (void *)offsetof(lit_color_point_t, point));

  glEnableVertexAttribArray(gp->attrib_locations[1]);
  glVertexAttribPointer(gp->attrib_locations[1], 3, GL_FLOAT, GL_FALSE,
      sizeof(lit_color_point_t), (void *)offsetof(lit_color_point_t, normal));

  glEnableVertexAttribArray(gp->attrib_locations[2]);
  glVertexAttribPointer(gp->attrib_locations[2], 4, GL_FLOAT, GL_FALSE,
      sizeof(lit_color_point_t), (void *)offsetof(lit_color_point_t, color));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  gp->scale = 1.0f;
  gp->initialized = TRUE;

  return( gp );

} /* opengl_ground_plane_new() */

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_free() - Free ground plane GL resources and allocated memory
 * @ctx: pointer to opengl_ground_plane_t returned by opengl_ground_plane_new()
 */
  void
opengl_ground_plane_free(void *ctx)
{
  opengl_ground_plane_t *gp = ctx;

  if( !gp )
    return;

  if( gp->initialized )
  {
    gl_shader_destroy(&gp->shader);
    glDeleteBuffers(1, &gp->vbo);
    glDeleteVertexArrays(1, &gp->vao);
  }

  g_free(gp);

} /* opengl_ground_plane_free() */

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_prepare() - Regenerate ground plane vertices scaled to r_max
 * @ctx: pointer to opengl_ground_plane_t
 * @params: per-frame parameters carrying the scene geometry extent
 *
 * Skip the VBO update when the scale remains equal within float tolerance.
 */
  void
opengl_ground_plane_prepare(void *ctx, const gl_render_params_t *params)
{
  opengl_ground_plane_t *gp = ctx;
  lit_color_point_t vertices[GROUND_PLANE_VERTICES];
  const float minimum_extent = 0.001f;
  float r_max = params->r_max;

  if( !gp || !gp->initialized )
    return;

  /* Normalize tiny extents to unit scale so the ground plane remains visible. */
  r_max = fl_flt(r_max, minimum_extent) ? 1.0f : r_max;

  if( fl_feq(gp->scale, r_max) )
    return;

  gp->scale = r_max;

  generate_ground_plane_vertices(vertices, r_max);

  glBindBuffer(GL_ARRAY_BUFFER, gp->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

} /* opengl_ground_plane_prepare() */

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_far_extent() - Return spatial extent for clip plane calculation
 * @ctx: pointer to opengl_ground_plane_t (unused)
 * @r_max: scene geometry extent
 *
 * Ground plane extends GROUND_PLANE_EXTENT times r_max from origin.
 */
  float
opengl_ground_plane_far_extent(void *ctx, float r_max)
{
  (void)ctx;

  return( r_max * GROUND_PLANE_EXTENT );

} /* opengl_ground_plane_far_extent() */

/*-----------------------------------------------------------------------*/

/** opengl_ground_plane_render() - Render ground plane quad with transparent checkerboard pattern
 * @ctx: pointer to opengl_ground_plane_t
 * @params: per-frame render parameters
 */
  void
opengl_ground_plane_render(void *ctx, const gl_render_params_t *params)
{
  opengl_ground_plane_t *gp = ctx;

  if( !gp || !gp->initialized )
    return;

  glUseProgram(gp->shader.program);
  glUniform1f(gp->u_alpha_location,
      TRANSPARENCY_TO_ALPHA(rc_config.transparency_ground_plane));
  glUniform1f(gp->u_color_dim_location, rc_config.brightness_ground_plane);
  glUniformMatrix4fv(gp->mvp_location, 1, GL_FALSE, (const float *)params->mvp);

  gl_view_set_peel_uniforms(&gp->peel_locs, params);

  /* VAO has attrib config from init — bind and draw.
   * Blend and depth mask managed by the peel render loop. */
  glBindVertexArray(gp->vao);
  glDrawArrays(GL_TRIANGLES, 0, GROUND_PLANE_VERTICES);
  glBindVertexArray(0);

} /* opengl_ground_plane_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
