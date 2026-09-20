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

#ifndef OPENGL_STRUCTURE_GEOMETRY_H
#define OPENGL_STRUCTURE_GEOMETRY_H 1

#include "opengl_structure.h"
#include "../render/render_dispatch.h"

#ifdef HAVE_OPENGL

/* Free vertex buffer and reset geometry state */
void opengl_structure_geometry_cleanup(void);

/* Check staleness and regenerate shared geometry using dispatch-resolved params */
void opengl_structure_update_shared_geometry_with_params(const struct_draw_params_t *params);


#endif /* HAVE_OPENGL */
#endif /* OPENGL_STRUCTURE_GEOMETRY_H */
