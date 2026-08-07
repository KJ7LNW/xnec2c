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

#ifndef OPENGL_AXES_H
#define OPENGL_AXES_H 1

#include "common.h"
#include "opengl_view.h"

#ifdef HAVE_OPENGL
#include "opengl_renderer.h"

/* Captures the inputs that produce cached axis vertex buffers. */
typedef struct
{
  float r_max;
  rgb_f_t axis;

} axes_build_key_t;

typedef struct
{
  GLuint lines_vao;
  GLuint lines_vbo;
  gl_shader_t line_shader;
  GLint line_mvp_loc;
  GLint line_u_alpha_loc;
  GLint line_u_color_dim_loc;
  GLint line_pos_loc;
  GLint line_col_loc;

  GLuint labels_vao;
  GLuint labels_vbo;
  GLuint label_texture;
  gl_shader_t label_shader;
  GLint label_mvp_loc;
  GLint label_tex_loc;
  GLint label_u_color_loc;
  GLint label_pos_loc;
  GLint label_uv_loc;

  axes_build_key_t built;
  gboolean initialized;

} opengl_axes_t;

opengl_axes_t* opengl_axes_new(void);
void opengl_axes_free(void *ctx);
void opengl_axes_prepare(void *ctx, const gl_render_params_t *params);
void opengl_axes_render(void *ctx, const gl_render_params_t *params);
gboolean opengl_axes_is_active(void *ctx);
float opengl_axes_far_extent(void *ctx, float r_max);
float opengl_axes_get_alpha(void *ctx);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_AXES_H */
