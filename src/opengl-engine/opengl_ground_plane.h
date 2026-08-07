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

#ifndef OPENGL_GROUND_PLANE_H
#define OPENGL_GROUND_PLANE_H 1

#include "common.h"

#ifdef HAVE_OPENGL
#include "opengl_view.h"

typedef struct
{
  GLuint vao;
  GLuint vbo;
  gl_shader_t shader;
  GLint mvp_location;
  GLint u_alpha_location;
  GLint u_color_dim_location;
  gl_peel_uniform_locs_t peel_locs;
  GLint attrib_locations[3];
  float scale;
  gboolean initialized;

} opengl_ground_plane_t;

opengl_ground_plane_t* opengl_ground_plane_new(void);
void opengl_ground_plane_free(void *ctx);
void opengl_ground_plane_prepare(void *ctx, const gl_render_params_t *params);
void opengl_ground_plane_render(void *ctx, const gl_render_params_t *params);
float opengl_ground_plane_far_extent(void *ctx, float r_max);
float opengl_ground_plane_get_alpha(void *ctx);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_GROUND_PLANE_H */
