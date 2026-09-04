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

/*
 * cairo_field_vectors: Cairo renderer for resolved field vectors, the near
 * E/H fields with their Poynting vector and the far-zone instantaneous field.
 *
 * Reads resolver-provided entries carrying origin, displacement, and palette
 * color from a sentinel-terminated field_vector_set_t list.
 * No field math, no Poynting cross product, no Value_to_Color.
 */
#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/**
 * cairo_draw_field_vectors() - Draw resolved field vectors via Cairo
 * @surface: Cairo surface presenting the radiation pattern view
 * @sets:    dispatch-assembled field vector sets, terminated by absent entries
 * @r_max:   maximum distance for view scaling
 *
 * Iterates each set and draws prerendered vectors as colored line segments
 * from origin to origin+displacement.
 * Returns TRUE on success.
 */
  gboolean
cairo_draw_field_vectors(render_surface_t *surface,
    const field_vector_set_t *sets, double r_max)
{
  cairo_engine_surface_t *cs = cairo_engine_surface(surface);
  view_t *v = surface->view;
  Segment_t segm;
  int f, idx;

  double scale = view_projection_scale(v, (float)r_max, v->zoom);

  /* Deposit each dispatch-assembled field vector into the scenebuffer */
  for( f = 0; sets[f].entries != NULL; f++ )
  {
    const field_vector_entry_t *entries = sets[f].entries;
    int count = mem_array_count(entries);

    for( idx = 0; idx < count; idx++ )
    {
      double px = entries[idx].origin.x;
      double py = entries[idx].origin.y;
      double pz = entries[idx].origin.z;

      /* Tip = origin + precomputed displacement */
      double fx = px + (double)entries[idx].vector.dx;
      double fy = py + (double)entries[idx].vector.dy;
      double fz = pz + (double)entries[idx].vector.dz;

      Set_Gdk_Segment(&segm, v, scale, px, py, pz, fx, fy, fz, &segm.z_mid);
      segm.r     = entries[idx].color.r;
      segm.g     = entries[idx].color.g;
      segm.b     = entries[idx].color.b;
      segm.width = 2.0f;
      scenebuffer_add(&cs->scenebuffer, &segm);
    }
  }

  return TRUE;
}

/*-----------------------------------------------------------------------*/
