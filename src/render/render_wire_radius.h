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

#ifndef RENDER_WIRE_RADIUS_H
#define RENDER_WIRE_RADIUS_H 1

#include "render_surface_input.h"

/* Scale below which segments render as lines instead of solids with 0.001 epsilon for slider */
#define WIRE_FORM_LINE_THRESHOLD 0.101

/* Maximum allowed radius scale */
#define WIRE_RADIUS_SCALE_MAX 100.0

/* Wire presentation the radius scale selects: fixed-width lines below
 * WIRE_FORM_LINE_THRESHOLD, radius-scaled solids otherwise */
typedef enum
{
  WIRE_FORM_LINE = 0,
  WIRE_FORM_SOLID,
  WIRE_FORM_COUNT

} wire_form_t;

/* Ctrl+scroll capability adjusting wire radius scale.
 * Offered by any view that presents structure geometry. */
extern surface_capability_t wire_radius_cap;

#endif /* RENDER_WIRE_RADIUS_H */
