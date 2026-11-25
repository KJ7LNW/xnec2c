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
 * structure_crnt: Per-frequency-step current buffers.
 *
 * Allocates, resizes and releases crnt_fstep[], which holds the
 * current and charge data of every frequency step.
 */
#include "structure_crnt.h"
#include "shared.h"
#include "prerender/prerender_color.h"

/*-----------------------------------------------------------------------*/

/**
 * free_crnt_step() - Release one fstep's crnt sub-buffers
 * @elem: pointer to one crnt_t element
 */
static void
free_crnt_step(void *elem)
{
  crnt_t *c = elem;
  mem_array_free(&c->air);
  mem_array_free(&c->aii);
  mem_array_free(&c->bir);
  mem_array_free(&c->bii);
  mem_array_free(&c->cir);
  mem_array_free(&c->cii);
  mem_array_free(&c->cur);
}

/*-----------------------------------------------------------------------*/

/**
 * Alloc_Crnt_Fstep_Buffers() - Allocate per-frequency-step crnt storage
 *
 * @nfrq: Number of frequency steps (steps_total + 1)
 *
 * Allocates crnt_fstep[] array so each frequency step holds the
 * current/charge data its solve writes.
 */
  void
Alloc_Crnt_Fstep_Buffers( int nfrq )
{
  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their sub-buffers for reuse by the inner alloc loop. */
  mem_array_resize(&crnt_fstep, nfrq, free_crnt_step);

  for( int i = 0; i < nfrq; i++ )
  {
    mem_array_realloc(&crnt_fstep[i].air, data.npm);
    mem_array_realloc(&crnt_fstep[i].aii, data.npm);
    mem_array_realloc(&crnt_fstep[i].bir, data.npm);
    mem_array_realloc(&crnt_fstep[i].bii, data.npm);
    mem_array_realloc(&crnt_fstep[i].cir, data.npm);
    mem_array_realloc(&crnt_fstep[i].cii, data.npm);
    mem_array_realloc(&crnt_fstep[i].cur, data.np3m);

  }

  alloc_struct_colors(nfrq);

} /* Alloc_Crnt_Fstep_Buffers() */

/*-----------------------------------------------------------------------*/

/**
 * free_crnt_fstep_buffers() - Release per-frequency current storage
 */
  void
free_crnt_fstep_buffers(void)
{
  int nfrq = mem_array_count(crnt_fstep);

  for( int i = 0; i < nfrq; i++ )
    free_crnt_step(&crnt_fstep[i]);
  mem_array_free(&crnt_fstep);

} /* free_crnt_fstep_buffers() */

/*-----------------------------------------------------------------------*/
