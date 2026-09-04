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

#ifndef CAIRO_PATCH_FLOW_H
#define CAIRO_PATCH_FLOW_H      1

#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../render/render_patch_flow.h"

/**
 * cairo_patch_flow_draw() - Deposit the resolved patch flow arrows
 * @sb: scenebuffer accumulating depth-sorted primitives
 * @v: view supplying the projection basis
 * @scale: structure-to-presentation scale
 * @flow: renderer-neutral frame carrying world-space arrows
 *
 * Projects every arrow the render layer admitted, in the color that layer
 * attached; an empty arrow array deposits nothing.
 */
void cairo_patch_flow_draw(cairo_scenebuffer_t *sb, view_t *v, double scale,
    const patch_flow_frame_t *flow);

#endif /* CAIRO_PATCH_FLOW_H */
