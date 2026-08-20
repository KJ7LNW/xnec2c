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

#include "prerender_state.h"
#include "../shared.h"

/* Singleton geometry-derived state (Tier 1) */
geom_pre_t    geom_pre;

/* Per-fstep prerender arrays (Tier 3) */
ff_pre_t     *ff_pre     = NULL;

/*-----------------------------------------------------------------------*/

/**
 * free_ff_pre_step() - Release one ff_pre_t entry
 * @elem: pointer to one ff_pre_t element
 */
static void
free_ff_pre_step(void *elem)
{
  ff_pre_t *fp = elem;
  mem_array_free(&fp->vertices);
  mem_array_free(&fp->theta_rgb);
  mem_array_free(&fp->phi_rgb);
  mem_array_free(&fp->vertex_rgb);
}

/*-----------------------------------------------------------------------*/

  void
prerender_state_alloc(int steps_total)
{
  if( steps_total <= 0 )
    return;

  /* Resize each array, freeing only the shrink tail; surviving entries
   * keep their sub-buffers for reuse by the inner alloc loop. */
  mem_array_resize(&ff_pre, steps_total, free_ff_pre_step);
}

/*-----------------------------------------------------------------------*/

  void
prerender_state_free(void)
{
  int i;
  int n = mem_array_count(ff_pre);

  for( i = 0; i < n; i++ )
    free_ff_pre_step(&ff_pre[i]);
  mem_array_free(&ff_pre);

  mem_array_free(&geom_pre.sin_theta);
  mem_array_free(&geom_pre.cos_theta);
  mem_array_free(&geom_pre.sin_phi);
  mem_array_free(&geom_pre.cos_phi);
  mem_array_free(&geom_pre.solid_angle);
  mem_array_free(&geom_pre.theta_topo);
  mem_array_free(&geom_pre.phi_topo);
  geom_pre.n_theta_edges = 0;
  geom_pre.n_phi_edges   = 0;

  /* Retain GE-card patch geometry across sweep restarts, then release both
   * arrays at process teardown. */
  mem_array_free(&geom_pre.patch_corners);
  mem_array_free(&geom_pre.patch_tangent_frame);
}

/*-----------------------------------------------------------------------*/
