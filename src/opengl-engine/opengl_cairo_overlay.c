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

#include "opengl_cairo_overlay.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/* Fullscreen quad with V-flipped UV mapping for Cairo coordinate compensation.
 *
 * Cairo origin is top-left with Y increasing downward.
 * OpenGL texture origin is bottom-left with Y increasing upward.
 * glTexImage2D uploads Cairo row 0 (visual top) to texel V=0 (visual bottom).
 *
 * This mapping: NDC top (Y=+1) -> UV V=0 -> Cairo row 0 (visual top)
 * Result: Cairo content appears with correct vertical orientation. */
static const text_vertex_t quad_vertices[6] = {
  {-1.0f,  1.0f, 0.0f,  0.0f, 0.0f},
  {-1.0f, -1.0f, 0.0f,  0.0f, 1.0f},
  { 1.0f, -1.0f, 0.0f,  1.0f, 1.0f},
  {-1.0f,  1.0f, 0.0f,  0.0f, 0.0f},
  { 1.0f, -1.0f, 0.0f,  1.0f, 1.0f},
  { 1.0f,  1.0f, 0.0f,  1.0f, 0.0f}
};

/*-----------------------------------------------------------------------*/

/** cairo_gl_overlay_new() - Allocate and initialize Cairo overlay renderer with V-flipped UV mapping
 */
  cairo_gl_overlay_t*
cairo_gl_overlay_new(void)
{
  cairo_gl_overlay_t *overlay;
  gboolean shader_ok;

  overlay = g_new0(cairo_gl_overlay_t, 1);

  shader_ok = gl_shader_load(&overlay->shader,
    "/gl/text-vertex.glsl",
    "/gl/text-fragment.glsl");

  if( !shader_ok )
  {
    pr_err("Failed to load cairo overlay shaders\n");
    g_free(overlay);
    return NULL;
  }

  overlay->mvp_location = glGetUniformLocation(overlay->shader.program, "mvp");
  overlay->tex_location = glGetUniformLocation(overlay->shader.program, "tex");
  overlay->color_location =
    glGetUniformLocation(overlay->shader.program, "u_color");
  overlay->position_location = glGetAttribLocation(overlay->shader.program, "position");
  overlay->texcoord_location = glGetAttribLocation(overlay->shader.program, "texcoord");

  glGenVertexArrays(1, &overlay->vao);
  glGenBuffers(1, &overlay->vbo);

  glBindVertexArray(overlay->vao);
  glBindBuffer(GL_ARRAY_BUFFER, overlay->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(overlay->position_location);
  glVertexAttribPointer(overlay->position_location, 3, GL_FLOAT, GL_FALSE,
    sizeof(text_vertex_t), (void*)0);

  glEnableVertexAttribArray(overlay->texcoord_location);
  glVertexAttribPointer(overlay->texcoord_location, 2, GL_FLOAT, GL_FALSE,
    sizeof(text_vertex_t), (void*)(3 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &overlay->texture);
  glBindTexture(GL_TEXTURE_2D, overlay->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  overlay->width = 0;
  overlay->height = 0;
  overlay->initialized = TRUE;

  return overlay;

} /* cairo_gl_overlay_new() */

/*-----------------------------------------------------------------------*/

/** cairo_gl_overlay_free() - Free Cairo overlay resources
 * @overlay: overlay instance to free
 */
  void
cairo_gl_overlay_free(cairo_gl_overlay_t *overlay)
{
  if( !overlay )
    return;

  if( overlay->initialized )
  {
    glDeleteTextures(1, &overlay->texture);
    glDeleteBuffers(1, &overlay->vbo);
    glDeleteVertexArrays(1, &overlay->vao);
    gl_shader_destroy(&overlay->shader);
  }

  g_free(overlay);

} /* cairo_gl_overlay_free() */

/*-----------------------------------------------------------------------*/

/** cairo_gl_overlay_set_size() - Update overlay dimensions
 * @overlay: overlay instance
 * @width: new width in pixels
 * @height: new height in pixels
 */
  void
cairo_gl_overlay_set_size(cairo_gl_overlay_t *overlay, int width, int height)
{
  if( !overlay )
    return;

  overlay->width = width;
  overlay->height = height;

} /* cairo_gl_overlay_set_size() */

/*-----------------------------------------------------------------------*/

/** cairo_gl_overlay_upload() - Upload Cairo surface data to OpenGL texture
 * @overlay: overlay instance
 * @surface: Cairo surface containing ARGB32 data
 */
  void
cairo_gl_overlay_upload(cairo_gl_overlay_t *overlay, cairo_surface_t *surface)
{
  unsigned char *data;
  int width, height;

  if( !overlay || !overlay->initialized || !surface )
    return;

  cairo_surface_flush(surface);
  data = cairo_image_surface_get_data(surface);
  width = cairo_image_surface_get_width(surface);
  height = cairo_image_surface_get_height(surface);

  glBindTexture(GL_TEXTURE_2D, overlay->texture);

  /* Reuse existing allocation when dimensions match */
  if( width == overlay->tex_width && height == overlay->tex_height )
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
        GL_BGRA, GL_UNSIGNED_BYTE, data);
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, data);
    overlay->tex_width = width;
    overlay->tex_height = height;
  }

  glBindTexture(GL_TEXTURE_2D, 0);

} /* cairo_gl_overlay_upload() */

/*-----------------------------------------------------------------------*/

/** cairo_gl_overlay_render() - Render overlay fullscreen with orthographic projection
 * @overlay: overlay instance
 */
  void
cairo_gl_overlay_render(cairo_gl_overlay_t *overlay)
{
  mat4 ortho;

  if( !overlay || !overlay->initialized )
    return;

  if( overlay->width <= 0 || overlay->height <= 0 )
    return;

  glm_mat4_identity(ortho);
  glm_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, ortho);

  glUseProgram(overlay->shader.program);
  glUniformMatrix4fv(overlay->mvp_location, 1, GL_FALSE, (float*)ortho);
  glUniform1i(overlay->tex_location, 0);

  /* Identity tint: the Cairo surface already carries its own colors */
  glUniform3f(overlay->color_location, 1.0f, 1.0f, 1.0f);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, overlay->texture);

  /* Caller owns blend/depth/coverage state */
  glBindVertexArray(overlay->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

} /* cairo_gl_overlay_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
