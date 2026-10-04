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

#ifndef __RENDER_MESSAGE_PAINT_H
#define __RENDER_MESSAGE_PAINT_H 1

#include "../common.h"
#include "render_engine.h"

/**
 * overlay_text_paint() - Paint the image the painted key describes
 * @surface: surface whose store holds the painted key
 *
 * Returns a drawable-sized image, or NULL when no line shows or the drawable
 * holds no area.
 */
cairo_surface_t *overlay_text_paint(const render_surface_t *surface);

#endif /* __RENDER_MESSAGE_PAINT_H */
