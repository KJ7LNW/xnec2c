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
 * Consumes a vertex stream and a projection kind; folds a bounding box and,
 * for perspective, a depth-aware maximum; then solves for a centroid and a
 * framing measure.  Holds no mode branch and no radius term.  Each engine
 * feeds it the vertices of the geometry that engine draws and inverts the
 * solved frame into its own zoom and pan.
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

  void
render_fit_add_persp(render_fit_acc_t *acc, float cx, float cy,
                     float x, float y, float z, const render_proj_t *proj)
{
  float tan_half = tanf(proj->fov_rad * 0.5f);
  float lateral_x;
  float lateral_y;
  float distance;

  if( !fl_fgt(tan_half, 0.0f) || !fl_fgt(proj->aspect, 0.0f) )
    return;

  /* Compute the camera distance required by this centered vertex. */
  lateral_x = fabsf(x - cx) / (tan_half * proj->aspect);
  lateral_y = fabsf(y - cy) / tan_half;
  distance = z + fmaxf(lateral_x, lateral_y);

  if( !acc->depth_any || fl_fgt(distance, acc->dmax) )
  {
    acc->dmax = distance;
    acc->depth_any = TRUE;
  }

} /* render_fit_add_persp() */

/*-----------------------------------------------------------------------*/

  gboolean
render_fit_solve(const render_fit_acc_t *acc, const render_proj_t *proj,
                 int viewport_height, render_fit_frame_t *out)
{
  gboolean solved = FALSE;

  if( acc == NULL || proj == NULL || out == NULL )
  {
    BUG("render_fit_solve: NULL input\n");
    return FALSE;
  }

  if( !acc->any )
  {
    pr_warn("render_fit_solve: no geometry to fit\n");
    return FALSE;
  }

  out->cx = 0.5f * (acc->minx + acc->maxx);
  out->cy = 0.5f * (acc->miny + acc->maxy);

  switch( proj->kind )
  {
    case RENDER_PROJ_PARALLEL:
      out->half_x = 0.5f * (acc->maxx - acc->minx);
      out->half_y = 0.5f * (acc->maxy - acc->miny);
      out->dist = 0.0f;
      solved = TRUE;
      break;

    case RENDER_PROJ_PERSPECTIVE:
    {
      float available;
      float border_factor;

      if( acc->depth_any
          && viewport_height > 2 * RENDER_FIT_BORDER_PX )
      {
        available = (float)viewport_height
            - 2.0f * (float)RENDER_FIT_BORDER_PX;
        border_factor = (float)viewport_height / available;
        out->half_x = 0.0f;
        out->half_y = 0.0f;
        out->dist = acc->dmax * border_factor;
        solved = TRUE;
      }
      break;
    }

    case RENDER_PROJ_COUNT:
      BUG("render_fit_solve: invalid projection kind %d\n", proj->kind);
      break;
  }

  return solved;

} /* render_fit_solve() */

/*-----------------------------------------------------------------------*/
