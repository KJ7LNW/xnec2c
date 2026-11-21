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

#define AXIS_LENGTH_SCALE 1.1f
#define LABEL_OFFSET_SCALE 1.15f
#define LABEL_SIZE_SCALE 0.05f
#define LABEL_TEX_SCALE  4
#define LABEL_TEX_WIDTH  (32 * LABEL_TEX_SCALE)
#define LABEL_TEX_MARGIN (8 * LABEL_TEX_SCALE)

/* Axis labels indexed by axis number (0=X, 1=Y, 2=Z) */
static const char *axis_labels[GL_AXIS_COUNT] = {"X", "Y", "Z"};

/* Select the along-axis and transverse coordinates for each label quad. */
static const int axis_coord_map[GL_AXIS_COUNT][2] = {
  {0, 1},
  {1, 0},
  {2, 0}
};

static const gl_vertex_layout_t axes_fit_layout = {
  .stride = sizeof(axes_vertex_t),
};

/* Quad vertex pattern for billboards (2 triangles = 6 vertices)
 * Coordinates are normalized offsets [-1,1] scaled by label_half_size */
static const float quad_offsets[GL_AXIS_LABEL_VERTICES][2] = {
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

  width = GL_AXIS_COUNT * LABEL_TEX_WIDTH;
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

  for( int i = 0; i < GL_AXIS_COUNT; i++ )
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
 * @scene: parent-prepared axes and model scale borrowed for the renderer lifetime
 */
  opengl_axes_t*
opengl_axes_new(const gl_view_content_t *scene)
{
  opengl_axes_t *axes;
  gboolean line_ok, label_ok;

  axes = g_new0(opengl_axes_t, 1);
  axes->scene = scene;
  axes->fit_content = (gl_view_content_t){
    .batches = {
      { .vertices = axes->lines, .vertex_count = G_N_ELEMENTS(axes->lines) },
      { .vertices = axes->labels, .vertex_count = G_N_ELEMENTS(axes->labels) },
    },
    .batch_count = 2,
    .layout = &axes_fit_layout,
  };

  const gl_shader_spec_t line_spec = {
    .vertex_path = "/gl/color-vertex.glsl",
    .fragment_path = "/gl/color-fragment.glsl" };
  const gl_shader_spec_t label_spec = {
    .vertex_path = "/gl/text-vertex.glsl",
    .fragment_path = "/gl/text-fragment.glsl" };

  line_ok = gl_shader_load(&axes->line_shader, &line_spec);

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

  label_ok = gl_shader_load(&axes->label_shader, &label_spec);

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

/** opengl_axes_build_vertices() - Build the axis segments and label quads
 * @axes: axes context owning the shared vertex arrays
 */
  static void
opengl_axes_build_vertices(opengl_axes_t *axes)
{
  float extent = axes->scene->axes.extent;
  float axis_length = extent * AXIS_LENGTH_SCALE;
  float label_offset = extent * LABEL_OFFSET_SCALE;
  float label_half_size = extent * LABEL_SIZE_SCALE;
  int i;

  // Keep the coordinates outside each axis or label plane at zero.
  for( i = 0; i < GL_AXIS_COUNT; i++ )
  {
    axes_vertex_t *label = &axes->labels[i * GL_AXIS_LABEL_VERTICES];
    float u_min = (float)i / GL_AXIS_COUNT;
    float u_max = (float)(i + 1) / GL_AXIS_COUNT;
    int j;

    axes->lines[i * GL_AXIS_LINE_VERTICES + 1].position[i] = axis_length;
    for( j = 0; j < GL_AXIS_LABEL_VERTICES; j++ )
    {
      label[j].position[axis_coord_map[i][0]] =
        label_offset + quad_offsets[j][0] * label_half_size;
      label[j].position[axis_coord_map[i][1]] = quad_offsets[j][1] * label_half_size;
      label[j].texcoord[0] = u_min + quad_uvs[j][0] * (u_max - u_min);
      label[j].texcoord[1] = quad_uvs[j][1];
    }
  }

} /* opengl_axes_build_vertices() */

/** opengl_axes_get_content() - Expose drawn axis vertices to the fit solver
 * @ctx: axes context
 */
  const gl_view_content_t *
opengl_axes_get_content(void *ctx)
{
  opengl_axes_t *axes = ctx;

  if( axes->scene->batch_count == 0 || fl_feq(axes->scene->axes.extent, 0.0f) )
    return NULL;

  opengl_axes_build_vertices(axes);
  axes->fit_content.model_scale = axes->scene->model_scale;
  return &axes->fit_content;

} /* opengl_axes_get_content() */

/** opengl_axes_prepare() - Rebuild axis buffers when their inputs change
 * @ctx: axes context
 * @params: per-frame parameters carrying axis geometry and line color
 */
  void
opengl_axes_prepare(void *ctx, const gl_render_params_t *params)
{
  opengl_axes_t *axes = ctx;
  axes_build_key_t key;
  size_t i;

  if( !axes || !axes->initialized )
    return;

  key = (axes_build_key_t){
    .r_max = axes->scene->axes.extent,
    .axis = params->view_axis,
  };

  if( memcmp(&axes->built, &key, sizeof(key)) == 0 )
    return;

  opengl_axes_build_vertices(axes);
  for( i = 0; i < G_N_ELEMENTS(axes->lines); i++ )
    axes->lines[i].color = (rgba_f_t){ key.axis.r, key.axis.g, key.axis.b, 1.0f };

  /* Upload axis line geometry. */
  glBindVertexArray(axes->lines_vao);
  glBindBuffer(GL_ARRAY_BUFFER, axes->lines_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axes->lines), axes->lines, GL_STATIC_DRAW);

  setup_vertex_attrib(axes->line_pos_loc, 3, sizeof(axes_vertex_t),
      offsetof(axes_vertex_t, position));
  setup_vertex_attrib(axes->line_col_loc, 4, sizeof(axes_vertex_t),
      offsetof(axes_vertex_t, color));

  /* Upload label geometry. */
  glBindVertexArray(axes->labels_vao);
  glBindBuffer(GL_ARRAY_BUFFER, axes->labels_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axes->labels), axes->labels, GL_STATIC_DRAW);

  setup_vertex_attrib(axes->label_pos_loc, 3, sizeof(axes_vertex_t),
      offsetof(axes_vertex_t, position));
  setup_vertex_attrib(axes->label_uv_loc, 2, sizeof(axes_vertex_t),
      offsetof(axes_vertex_t, texcoord));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  axes->built = key;

} /* opengl_axes_prepare() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_is_active() - Report parent-prepared axes activity
 * @ctx: axes context
 */
  gboolean
opengl_axes_is_active(void *ctx)
{
  opengl_axes_t *axes = ctx;

  return( axes->scene->axes.active );

} /* opengl_axes_is_active() */

/*-----------------------------------------------------------------------*/

/** opengl_axes_far_extent() - Report how far the prepared axes reach
 * @ctx: axes context
 * @_r_max: scene extent, unread here: the axes span the extent the parent
 *          prepared for them
 */
  float
opengl_axes_far_extent(void *ctx, float _r_max)
{
  opengl_axes_t *axes = ctx;

  return( axes->scene->axes.extent * AXIS_LENGTH_SCALE );

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
  glDrawArrays(GL_LINES, 0, G_N_ELEMENTS(axes->lines));
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
  glDrawArrays(GL_TRIANGLES, 0, G_N_ELEMENTS(axes->labels));
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);

} /* opengl_axes_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
