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
 * cairo_nearfield: Cairo renderer for near E/H fields and Poynting vector.
 *
 * Reads resolver-provided nf_vector_t displacements and their parallel
 * palette colors from dispatch-assembled nf_field_set_t[]. Origin
 * coordinates come from the origins array passed by dispatch (Tier 2).
 * No field math, no Poynting cross product, no Value_to_Color.
 */
#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/**
 * cairo_draw_nearfield() - Draw near-field vectors via Cairo
 * @surface:  Cairo surface presenting the radiation pattern view
 * @origins:  near-field sample point coordinates [npts]
 * @npts:     number of sample points
 * @fields:   dispatch-assembled field vector sets
 * @n_fields: number of active field sets (0-3)
 * @dr:       normalized displacement scale
 * @r_max:    maximum distance for view scaling
 *
 * Iterates each field set and draws prerendered vectors as colored line
 * segments from origin to origin+displacement.
 * Returns TRUE on success.
 */
  gboolean
cairo_draw_nearfield(render_surface_t *surface,
    const near_field_point_t *origins, int npts,
    const nf_field_set_t *fields, int n_fields,
    double dr, double r_max)
{
  cairo_engine_surface_t *cs = cairo_engine_surface(surface);
  view_t *v = surface->view;
  Segment_t segm;
  int f, idx;

  double scale = view_projection_scale(v, (float)r_max, v->zoom);

  (void)dr;

  /* Deposit each dispatch-assembled field vector into the scenebuffer */
  for( f = 0; f < n_fields; f++ )
  {
    const nf_vector_t *vecs = fields[f].vecs;
    if( vecs == NULL )
      continue;

    for( idx = 0; idx < npts; idx++ )
    {
      double px = origins[idx].px;
      double py = origins[idx].py;
      double pz = origins[idx].pz;

      /* Tip = origin + precomputed displacement */
      double fx = px + (double)vecs[idx].dx;
      double fy = py + (double)vecs[idx].dy;
      double fz = pz + (double)vecs[idx].dz;

      Set_Gdk_Segment(&segm, v, scale, px, py, pz, fx, fy, fz, &segm.z_mid);
      segm.r     = fields[f].colors[idx].r;
      segm.g     = fields[f].colors[idx].g;
      segm.b     = fields[f].colors[idx].b;
      segm.width = 2.0f;
      scenebuffer_add(&cs->scenebuffer, &segm);
    }
  }

  return TRUE;
}

/*-----------------------------------------------------------------------*/
