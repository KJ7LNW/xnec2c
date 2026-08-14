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
 * cairo_farfield: Cairo renderer for far-field radiation pattern.
 *
 * Reads ff_pre_t (Tier 3) for vertex positions and edge colors.
 * Topology (va/vb) comes from geom_pre (Tier 1).
 * No domain math; pure geometric projection and drawing.
 */
#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"

/**
 * cairo_draw_farfield() - Draw far-field gain pattern via Cairo
 * @surface: Cairo surface presenting the radiation pattern view
 * @fstep: frequency step index
 * @ff:    far-field draw parameters from dispatch
 *
 * Reads ff_pre[fstep] for vertex positions and per-edge colors.
 * Edge connectivity from geom_pre.theta_topo / phi_topo (Tier 1).
 * Returns TRUE on success, FALSE if data not ready.
 */
  gboolean
cairo_draw_farfield(render_surface_t *surface, int fstep,
    const ff_draw_params_t *ff)
{
  cairo_engine_surface_t *cs = cairo_engine_surface(surface);
  view_t *v = surface->view;
  Segment_t segm;
  int idx;

  if( fstep < 0 || ff_pre == NULL )
    return FALSE;

  ff_pre_t *fp = &ff_pre[fstep];

  if( fp->vertices == NULL || fp->theta_rgb == NULL || fp->phi_rgb == NULL )
    return FALSE;

  double scale = view_projection_scale(v, ff->pattern_radius, v->zoom);

  /* Deposit theta-direction edges into scenebuffer */
  for( idx = 0; idx < geom_pre.n_theta_edges; idx++ )
  {
    const ff_edge_topo_t *topo = &geom_pre.theta_topo[idx];
    const point_3d_t *va = &fp->vertices[topo->va];
    const point_3d_t *vb = &fp->vertices[topo->vb];

    Set_Gdk_Segment(&segm, v, scale,
        va->x + (double)ff->x, va->y + (double)ff->y, va->z + (double)ff->z,
        vb->x + (double)ff->x, vb->y + (double)ff->y, vb->z + (double)ff->z,
        &segm.z_mid);
    segm.r     = fp->theta_rgb[idx].r;
    segm.g     = fp->theta_rgb[idx].g;
    segm.b     = fp->theta_rgb[idx].b;
    segm.width = 2.0f;
    scenebuffer_add(&cs->scenebuffer, &segm);
  }

  /* Deposit phi-direction edges into scenebuffer */
  for( idx = 0; idx < geom_pre.n_phi_edges; idx++ )
  {
    const ff_edge_topo_t *topo = &geom_pre.phi_topo[idx];
    const point_3d_t *va = &fp->vertices[topo->va];
    const point_3d_t *vb = &fp->vertices[topo->vb];

    Set_Gdk_Segment(&segm, v, scale,
        va->x + (double)ff->x, va->y + (double)ff->y, va->z + (double)ff->z,
        vb->x + (double)ff->x, vb->y + (double)ff->y, vb->z + (double)ff->z,
        &segm.z_mid);
    segm.r     = fp->phi_rgb[idx].r;
    segm.g     = fp->phi_rgb[idx].g;
    segm.b     = fp->phi_rgb[idx].b;
    segm.width = 2.0f;
    scenebuffer_add(&cs->scenebuffer, &segm);
  }

  return TRUE;
}

/*-----------------------------------------------------------------------*/
