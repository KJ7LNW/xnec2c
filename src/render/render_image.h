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

#ifndef __RENDER_IMAGE_H
#define __RENDER_IMAGE_H 1

#include <cairo.h>
#include <stdint.h>

/**
 * render_image_t - Painted image an engine composites
 * @surface: ARGB32 image; NULL when no image shows
 * @version: monotonic counter; increments on surface rebuild
 *
 * Couples surface with its version so consumers can gate re-upload
 * without inspecting cache internals or relying on pointer identity.
 */
typedef struct
{
  cairo_surface_t *surface;
  uint64_t         version;
} render_image_t;

#endif /* __RENDER_IMAGE_H */
