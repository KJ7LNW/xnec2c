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

#ifndef OPENGL_CAIRO_OVERLAY_H
#define OPENGL_CAIRO_OVERLAY_H 1

#include "common.h"

#ifdef HAVE_OPENGL
#include "opengl_renderer.h"
#include <cairo.h>

/* Generic Cairo-to-OpenGL overlay renderer.
 *
 * Handles coordinate system transformation between Cairo (origin top-left,
 * Y increases downward) and OpenGL textures (origin bottom-left, Y increases
 * upward). The UV mapping compensates for glTexImage2D uploading Cairo row 0
 * (visual top) to texel V=0 (visual bottom).
 *
 * Usage:
 *   1. Create overlay with cairo_gl_overlay_new()
 *   2. Set size with cairo_gl_overlay_set_size()
 *   3. Draw to a Cairo surface, then upload with cairo_gl_overlay_upload()
 *   4. Render with cairo_gl_overlay_render()
 */
typedef struct
{
  GLuint texture;
  GLuint vao;
  GLuint vbo;
  gl_shader_t shader;
  GLint mvp_location;
  GLint tex_location;
  GLint color_location;
  GLint position_location;
  GLint texcoord_location;
  int width;
  int height;
  int tex_width;   /* Current texture allocation dimensions */
  int tex_height;
  gboolean initialized;

} cairo_gl_overlay_t;

cairo_gl_overlay_t* cairo_gl_overlay_new(void);
void cairo_gl_overlay_free(cairo_gl_overlay_t *overlay);
void cairo_gl_overlay_set_size(cairo_gl_overlay_t *overlay, int width, int height);
void cairo_gl_overlay_upload(cairo_gl_overlay_t *overlay, cairo_surface_t *surface);
void cairo_gl_overlay_render(cairo_gl_overlay_t *overlay);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_CAIRO_OVERLAY_H */
