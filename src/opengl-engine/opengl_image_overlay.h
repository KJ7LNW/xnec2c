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

#ifndef OPENGL_IMAGE_OVERLAY_H
#define OPENGL_IMAGE_OVERLAY_H 1

#include "common.h"
#include <stdint.h>

#ifdef HAVE_OPENGL
#include "opengl_cairo_overlay.h"

typedef struct
{
  cairo_gl_overlay_t  *base;
  uint64_t             last_version; /* version at last texture upload */
} gl_image_overlay_t;

gl_image_overlay_t* gl_image_overlay_new(void);
void gl_image_overlay_free(gl_image_overlay_t *overlay);
void gl_image_overlay_set_viewport(gl_image_overlay_t *overlay, int width, int height);
void gl_image_overlay_upload_surface(gl_image_overlay_t *overlay,
    cairo_surface_t *surface, uint64_t version);
void gl_image_overlay_render(gl_image_overlay_t *overlay);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_IMAGE_OVERLAY_H */
