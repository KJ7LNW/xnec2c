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
 * render_fit: generic geometric reducer for fit-to-view.
 *
 * Fold projected vertices into a bounding box and derive its center and
 * half-extents.  Each parallel renderer converts that frame to zoom and pan.
 */

#include <math.h>

#include "render_fit.h"
#include "../common.h"
#include "../console.h"

/*-----------------------------------------------------------------------*/

  void
render_fit_add(render_fit_acc_t *acc, float x, float y)
{
  if( !acc->any )
  {
    acc->minx = acc->maxx = x;
    acc->miny = acc->maxy = y;
    acc->any = TRUE;
  }
  else
  {
    if( fl_flt(x, acc->minx) ) acc->minx = x;
    if( fl_fgt(x, acc->maxx) ) acc->maxx = x;
    if( fl_flt(y, acc->miny) ) acc->miny = y;
    if( fl_fgt(y, acc->maxy) ) acc->maxy = y;
  }

} /* render_fit_add() */

/*-----------------------------------------------------------------------*/

  gboolean
render_fit_solve(const render_fit_acc_t *acc, render_fit_frame_t *out)
{
  if( acc == NULL || out == NULL )
  {
    BUG("NULL input\n");
    return FALSE;
  }

  if( !acc->any )
  {
    pr_warn("no geometry to fit\n");
    return FALSE;
  }

  out->cx = 0.5f * (acc->minx + acc->maxx);
  out->cy = 0.5f * (acc->miny + acc->maxy);
  out->half_x = 0.5f * (acc->maxx - acc->minx);
  out->half_y = 0.5f * (acc->maxy - acc->miny);
  return TRUE;

} /* render_fit_solve() */

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
