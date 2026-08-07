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

#include "opengl_axes.h"
#include "opengl_view.h"  /* gl_render_params_t */
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** opengl_axes_get_alpha() - Classification alpha from rc_config */
float
opengl_axes_get_alpha(void *ctx)
{
  (void)ctx;
  return TRANSPARENCY_TO_ALPHA(rc_config.transparency_axes);
}

#define NUM_AXES 3
#define AXIS_LENGTH_SCALE 1.1f
#define LABEL_OFFSET_SCALE 1.15f
#define LABEL_SIZE_SCALE 0.05f
#define LABEL_TEX_SCALE  4
#define LABEL_TEX_WIDTH  (32 * LABEL_TEX_SCALE)
#define LABEL_TEX_MARGIN (8 * LABEL_TEX_SCALE)

/* Axis labels indexed by axis number (0=X, 1=Y, 2=Z) */
static const char *axis_labels[NUM_AXES] = {"X", "Y", "Z"};

/* Maps axis index to 3D coordinate positions [u_coord, v_coord, zero_coord]
 * Axis 0 (X): label offset along X, label size along Y, zero on Z
 * Axis 1 (Y): label offset along Y, label size along X, zero on Z
 * Axis 2 (Z): label offset along Z, label size along X, zero on Y */
static const int axis_coord_map[NUM_AXES][3] = {
  {0, 1, 2},
  {1, 0, 2},
  {2, 0, 1}
};

/* Quad vertex pattern for billboards (2 triangles = 6 vertices)
 * Coordinates are normalized offsets [-1,1] scaled by label_half_size */
static const float quad_offsets[6][2] = {
  {-1.0f,  1.0f}, {-1.0f, -1.0f}, { 1.0f, -1.0f},
  {-1.0f,  1.0f}, { 1.0f, -1.0f}, { 1.0f,  1.0f}
};

/* UV texture coordinates for quad */
static const float quad_uvs[6][2] = {
  {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
  {0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
};

/*-----------------------------------------------------------------------*/

/** generate_label_texture() - Creates a texture atlas with X, Y, Z labels using Cairo
 */
  static GLuint
generate_label_texture(void)
{
  cairo_surface_t *surface;
  cairo_t *cr;
  unsigned char *data;
  GLuint texture;
  int width, height;
  PangoLayout *layout;
  PangoFontDescription *font_desc;
  char font_str[32];

  width = NUM_AXES * LABEL_TEX_WIDTH;
  height = 32 * LABEL_TEX_SCALE;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cr = cairo_create(surface);

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);

  layout = pango_cairo_create_layout(cr);
  snprintf(font_str, sizeof(font_str), "Sans Bold %d", 20 * LABEL_TEX_SCALE);
  font_desc = pango_font_description_from_string(font_str);
  pango_layout_set_font_description(layout, font_desc);

  for( int i = 0; i < NUM_AXES; i++ )
  {
    int tex_x = LABEL_TEX_MARGIN + i * LABEL_TEX_WIDTH;
    pango_layout_set_text(layout, axis_labels[i], -1);
    cairo_move_to(cr, tex_x, 4 * LABEL_TEX_SCALE);
    pango_cairo_show_layout(cr, layout);
  }

  cairo_surface_flush(surface);
  data = cairo_image_surface_get_data(surface);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
    GL_BGRA, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, 0);

  pango_font_description_free(font_desc);
  g_object_unref(layout);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  pr_debug("Generated label texture atlas (ID=%u)\n", texture);

  return texture;

} /* generate_label_texture() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_new() - Allocate and initialize axes renderer with own shaders
 */
  opengl_axes_t*
opengl_axes_new(void)
{
  opengl_axes_t *axes;
  gboolean line_ok, label_ok;

  axes = g_new0(opengl_axes_t, 1);

  line_ok = gl_shader_load(&axes->line_shader,
    "/gl/color-vertex.glsl",
    "/gl/color-fragment.glsl");

  if( !line_ok )
  {
    pr_err("Failed to load axes line shader\n");
    g_free(axes);
    return( NULL );
  }

  axes->line_mvp_loc = glGetUniformLocation(axes->line_shader.program, "mvp");
  axes->line_u_alpha_loc = glGetUniformLocation(axes->line_shader.program, "u_alpha");
  axes->line_u_color_dim_loc = glGetUniformLocation(axes->line_shader.program, "u_color_dim");
  axes->line_pos_loc = glGetAttribLocation(axes->line_shader.program, "position");
  axes->line_col_loc = glGetAttribLocation(axes->line_shader.program, "color");

  glGenVertexArrays(1, &axes->lines_vao);
  glGenBuffers(1, &axes->lines_vbo);

  label_ok = gl_shader_load(&axes->label_shader,
    "/gl/text-vertex.glsl",
    "/gl/text-fragment.glsl");

  if( label_ok )
  {
    axes->label_mvp_loc = glGetUniformLocation(axes->label_shader.program, "mvp");
    axes->label_tex_loc = glGetUniformLocation(axes->label_shader.program, "tex");
    axes->label_u_color_loc =
      glGetUniformLocation(axes->label_shader.program, "u_color");
    axes->label_pos_loc = glGetAttribLocation(axes->label_shader.program, "position");
    axes->label_uv_loc = glGetAttribLocation(axes->label_shader.program, "texcoord");

    glGenVertexArrays(1, &axes->labels_vao);
    glGenBuffers(1, &axes->labels_vbo);

    axes->label_texture = generate_label_texture();
    axes->initialized = TRUE;

    pr_debug("Axes renderer initialized\n");
  }
  else
  {
    pr_err("Failed to load text shaders\n");
    gl_shader_destroy(&axes->line_shader);
    glDeleteBuffers(1, &axes->lines_vbo);
    glDeleteVertexArrays(1, &axes->lines_vao);
    g_free(axes);
    axes = NULL;
  }

  return( axes );

} /* opengl_axes_new() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_free() - Free axes renderer resources
 * @ctx: axes context
 */
  void
opengl_axes_free(void *ctx)
{
  opengl_axes_t *axes = ctx;

  if( !axes )
    return;

  if( axes->initialized )
  {
    glDeleteTextures(1, &axes->label_texture);
    glDeleteBuffers(1, &axes->labels_vbo);
    glDeleteVertexArrays(1, &axes->labels_vao);
    gl_shader_destroy(&axes->label_shader);
    glDeleteBuffers(1, &axes->lines_vbo);
    glDeleteVertexArrays(1, &axes->lines_vao);
  }

  g_free(axes);

} /* opengl_axes_free() */

/*-----------------------------------------------------------------------*/

/** setup_vertex_attrib() - Configure vertex attribute pointer
 * @loc: attribute location
 * @size: number of components
 * @stride: byte offset between consecutive attributes
 * @offset: byte offset to first component
 */
  static void
setup_vertex_attrib(GLint loc, GLint size, size_t stride, size_t offset)
{
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, (void*)offset);
}

/*-----------------------------------------------------------------------*/

/** opengl_axes_prepare() - Rebuild axis buffers when their inputs change
 * @ctx: axes context
 * @params: per-frame parameters carrying axis geometry and line color
 */
  void
opengl_axes_prepare(void *ctx, const gl_render_params_t *params)
{
  opengl_axes_t *axes = ctx;
  axes_build_key_t key;
  color_point_t lines[NUM_AXES * 2];
  text_vertex_t labels[NUM_AXES * 6];
  int i;

  if( !axes || !axes->initialized )
    return;

  key = (axes_build_key_t){
    .r_max = params->r_max,
    .axis = params->view_axis,
  };

  if( memcmp(&axes->built, &key, sizeof(key)) == 0 )
    return;

  rgba_f_t line_color = {
    key.axis.r, key.axis.g, key.axis.b, 1.0f
  };
  float axis_length = key.r_max * AXIS_LENGTH_SCALE;
  float label_offset = key.r_max * LABEL_OFFSET_SCALE;
  float label_half_size = key.r_max * LABEL_SIZE_SCALE;

  /* Build colored axis line vertices. */
  for( i = 0; i < NUM_AXES; i++ )
  {
    int line_idx = i * 2;
    float *origin_coord = (float*)&lines[line_idx].point;
    float *end_coord = (float*)&lines[line_idx + 1].point;

    lines[line_idx].color = line_color;
    lines[line_idx + 1].color = line_color;

    origin_coord[0] = 0.0f;
    origin_coord[1] = 0.0f;
    origin_coord[2] = 0.0f;

    end_coord[0] = 0.0f;
    end_coord[1] = 0.0f;
    end_coord[2] = 0.0f;
    end_coord[i] = axis_length;
  }

  /* Upload axis line geometry. */
  glBindVertexArray(axes->lines_vao);
  glBindBuffer(GL_ARRAY_BUFFER, axes->lines_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

  setup_vertex_attrib(axes->line_pos_loc, 3, sizeof(color_point_t), 0);
  setup_vertex_attrib(axes->line_col_loc, 4, sizeof(color_point_t), 4 * sizeof(float));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* Build label quads from the shared texture atlas. */
  for( i = 0; i < NUM_AXES; i++ )
  {
    int vtx_idx = i * 6;
    float u_min = (float)i / NUM_AXES;
    float u_max = (float)(i + 1) / NUM_AXES;
    int j;

    for( j = 0; j < 6; j++ )
    {
      float *pos = (float*)&labels[vtx_idx + j];
      float u = label_offset + quad_offsets[j][0] * label_half_size;
      float v = quad_offsets[j][1] * label_half_size;

      pos[axis_coord_map[i][0]] = u;
      pos[axis_coord_map[i][1]] = v;
      pos[axis_coord_map[i][2]] = 0.0f;

      labels[vtx_idx + j].u = u_min + quad_uvs[j][0] * (u_max - u_min);
      labels[vtx_idx + j].v = quad_uvs[j][1];
    }
  }

  /* Upload label geometry. */
  glBindVertexArray(axes->labels_vao);
  glBindBuffer(GL_ARRAY_BUFFER, axes->labels_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(labels), labels, GL_STATIC_DRAW);

  setup_vertex_attrib(axes->label_pos_loc, 3, sizeof(text_vertex_t), 0);
  setup_vertex_attrib(axes->label_uv_loc, 2, sizeof(text_vertex_t), 3 * sizeof(float));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  axes->built = key;

} /* opengl_axes_prepare() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_is_active() - Returns whether axes are active for rendering
 * @_ctx: unused
 */
  gboolean
opengl_axes_is_active(void *ctx)
{
  (void)ctx;

  return( TRUE );

} /* opengl_axes_is_active() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_far_extent() - Returns the spatial extent contribution for clip plane calculation
 * @_ctx: unused
 * @r_max: maximum radius
 */
  float
opengl_axes_far_extent(void *ctx, float r_max)
{
  (void)ctx;

  return( r_max * AXIS_LENGTH_SCALE );

} /* opengl_axes_far_extent() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_render() - Render axes lines and labels
 * @ctx: axes context
 * @params: per-frame render parameters carrying transforms and label color
 */
  void
opengl_axes_render(void *ctx, const gl_render_params_t *params)
{
  opengl_axes_t *axes = ctx;

  if( !axes || !axes->initialized || fl_feq(axes->built.r_max, 0.0f) )
    return;

  /* Render axis lines */
  glUseProgram(axes->line_shader.program);
  glUniformMatrix4fv(axes->line_mvp_loc, 1, GL_FALSE, (float*)params->mvp);
  glUniform1f(axes->line_u_alpha_loc,
      TRANSPARENCY_TO_ALPHA(rc_config.transparency_axes));
  glUniform1f(axes->line_u_color_dim_loc, rc_config.brightness_axes);

  /* Axis lines own their width explicitly so it never inherits leftover
   * global GL state from a previously rendered batch. */
  glLineWidth(1.0f);
  glBindVertexArray(axes->lines_vao);
  glDrawArrays(GL_LINES, 0, NUM_AXES * 2);
  glBindVertexArray(0);

  /* Render labels */
  glUseProgram(axes->label_shader.program);
  glUniformMatrix4fv(axes->label_mvp_loc, 1, GL_FALSE, (float*)params->mvp);
  glUniform1i(axes->label_tex_loc, 0);
  glUniform3f(axes->label_u_color_loc,
      params->view_axis_label.r,
      params->view_axis_label.g,
      params->view_axis_label.b);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, axes->label_texture);

  glBindVertexArray(axes->labels_vao);
  glDrawArrays(GL_TRIANGLES, 0, NUM_AXES * 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);

} /* opengl_axes_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
