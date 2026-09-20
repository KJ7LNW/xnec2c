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
 * prerender_aggregate: Tier 1 structure-geometry prerender.
 *
 * Computes scene_radius (max origin distance over wire endpoints and
 * patch corners) and precomputes patch_corners (4 rectangle corners
 * per surface patch).  Called once at GE-card time.
 */
#include "prerender_aggregate.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

static inline double
dist3(double x, double y, double z)
{
  return sqrt(x*x + y*y + z*z);
}

/**
 * scan_wire_radius() - Find max distance from origin over wire endpoints
 * @r_max: current maximum, updated in place
 */
static void
scan_wire_radius(double *r_max)
{
  int idx;
  double r;

  for( idx = 0; idx < data.n; idx++ )
  {
    r = dist3(data.segments[idx].x1,
              data.segments[idx].y1,
              data.segments[idx].z1);
    if( r > *r_max )
      *r_max = r;

    r = dist3(data.segments[idx].x2,
              data.segments[idx].y2,
              data.segments[idx].z2);
    if( r > *r_max )
      *r_max = r;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * New_Patch_Data() - Precompute patch rectangle corners and return patch r_max
 *
 * Allocates geom_pre.patch_corners[data.m]. In a single loop per patch,
 * stores all four corners (center ± s*t1 ± s*t2) and scans them for the
 * maximum distance from origin, which is returned for use by Prerender_Aggregate().
 */
  double
New_Patch_Data(void)
{
  int idx;
  double s, r, r_max = 0.0;
  double px, py, pz;
  double s1x, s1y, s1z, s2x, s2y, s2z;

  if( data.m == 0 )
    return 0.0;
  mem_array_realloc(&geom_pre.patch_corners, data.m);
  mem_array_realloc(&geom_pre.patch_tangent_frame, data.m);

  for( idx = 0; idx < data.m; idx++ )
  {
    s = sqrt(data.patches[idx].pbi) / 2.0;
    px = data.patches[idx].px;
    py = data.patches[idx].py;
    pz = data.patches[idx].pz;

    s1x = s * data.patches[idx].t1x;
    s1y = s * data.patches[idx].t1y;
    s1z = s * data.patches[idx].t1z;

    s2x = s * data.patches[idx].t2x;
    s2y = s * data.patches[idx].t2y;
    s2z = s * data.patches[idx].t2z;

    geom_pre.patch_tangent_frame[idx].cx   = px;
    geom_pre.patch_tangent_frame[idx].cy   = py;
    geom_pre.patch_tangent_frame[idx].cz   = pz;
    geom_pre.patch_tangent_frame[idx].st1x = s1x;
    geom_pre.patch_tangent_frame[idx].st1y = s1y;
    geom_pre.patch_tangent_frame[idx].st1z = s1z;
    geom_pre.patch_tangent_frame[idx].st2x = s2x;
    geom_pre.patch_tangent_frame[idx].st2y = s2y;
    geom_pre.patch_tangent_frame[idx].st2z = s2z;

    /* c0: center + s*t1 + s*t2 */
    geom_pre.patch_corners[idx].c0x = px + s1x + s2x;
    geom_pre.patch_corners[idx].c0y = py + s1y + s2y;
    geom_pre.patch_corners[idx].c0z = pz + s1z + s2z;

    /* c1: center - s*t1 + s*t2 */
    geom_pre.patch_corners[idx].c1x = px - s1x + s2x;
    geom_pre.patch_corners[idx].c1y = py - s1y + s2y;
    geom_pre.patch_corners[idx].c1z = pz - s1z + s2z;

    /* c2: center - s*t1 - s*t2 */
    geom_pre.patch_corners[idx].c2x = px - s1x - s2x;
    geom_pre.patch_corners[idx].c2y = py - s1y - s2y;
    geom_pre.patch_corners[idx].c2z = pz - s1z - s2z;

    /* c3: center + s*t1 - s*t2 */
    geom_pre.patch_corners[idx].c3x = px + s1x - s2x;
    geom_pre.patch_corners[idx].c3y = py + s1y - s2y;
    geom_pre.patch_corners[idx].c3z = pz + s1z - s2z;

    /* Scan all four corners for maximum distance from origin */
    r = dist3(geom_pre.patch_corners[idx].c0x,
              geom_pre.patch_corners[idx].c0y,
              geom_pre.patch_corners[idx].c0z);
    if( r > r_max ) r_max = r;

    r = dist3(geom_pre.patch_corners[idx].c1x,
              geom_pre.patch_corners[idx].c1y,
              geom_pre.patch_corners[idx].c1z);
    if( r > r_max ) r_max = r;

    r = dist3(geom_pre.patch_corners[idx].c2x,
              geom_pre.patch_corners[idx].c2y,
              geom_pre.patch_corners[idx].c2z);
    if( r > r_max ) r_max = r;

    r = dist3(geom_pre.patch_corners[idx].c3x,
              geom_pre.patch_corners[idx].c3y,
              geom_pre.patch_corners[idx].c3z);
    if( r > r_max ) r_max = r;
  }

  return r_max;
}

/*-----------------------------------------------------------------------*/

  void
Prerender_Aggregate(void)
{
  double r_max = 0.0;

  if( data.n > 0 )
    scan_wire_radius(&r_max);

  if( data.m > 0 )
  {
    double patch_r_max = New_Patch_Data();
    if( patch_r_max > r_max )
      r_max = patch_r_max;
  }

  geom_pre.scene_radius = r_max;

  /* Publish once the state it names is filled, so a consumer that read
   * mid-fill holds a version this deck no longer matches */
  geom_pre.generation++;
}

/*-----------------------------------------------------------------------*/
