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
 * cairo_project: Cairo drawing primitives and structure projection.
 */
#include "cairo_draw.h"
#include "../geometry.h"
#include "../shared.h"

/* 2D projected segments for Cairo structure rendering */
Segment_t *structure_segs = NULL;

/*-----------------------------------------------------------------------*/

/**
 * free_structure_segs() - Release the projected segment buffer
 */
  void
free_structure_segs(void)
{
  mem_array_free(&structure_segs);

} /* free_structure_segs() */

/*-----------------------------------------------------------------------
 * Drawing primitives
 *----------------------------------------------------------------------*/

  void
Cairo_Draw_Polygon(cairo_t *cr, GdkPoint *points, int npoints)
{
  int idx;

  cairo_move_to(cr, (double)points[0].x, (double)points[0].y);
  for( idx = 1; idx < npoints; idx++ )
    cairo_line_to(cr, (double)points[idx].x, (double)points[idx].y);
  cairo_close_path(cr);
}

/*-----------------------------------------------------------------------*/

  void
Cairo_Draw_Line(cairo_t *cr, int x1, int y1, int x2, int y2)
{
  cairo_move_to(cr, (double)x1, (double)y1);
  cairo_line_to(cr, (double)x2, (double)y2);
  cairo_stroke(cr);
}

/*-----------------------------------------------------------------------*/

  void
Cairo_Draw_Lines(cairo_t *cr, GdkPoint *points, int npoints)
{
  int idx;

  cairo_move_to(cr, (double)points[0].x, (double)points[0].y);
  for( idx = 1; idx < npoints; idx++ )
    cairo_line_to(cr, (double)points[idx].x, (double)points[idx].y);
  cairo_stroke(cr);
}

/*-----------------------------------------------------------------------*/

/*  Process_Wire_Segments()
 *
 *  Processes wire segment data so they can be drawn on Screen
 */
  void
Process_Wire_Segments( view_t *v, double scale )
{
  int idx;

  if( structure_segs == NULL )
    return;

  /* Project all wire segs from xyz frame to screen frame */
  for( idx = 0; idx < data.n; idx++ )
    Set_Gdk_Segment(
        &structure_segs[idx],
        v,
        scale,
        (double) data.segments[idx].x1,
        (double) data.segments[idx].y1,
        (double) data.segments[idx].z1,
        (double) data.segments[idx].x2,
        (double) data.segments[idx].y2,
        (double) data.segments[idx].z2,
        &structure_segs[idx].z_mid);

} /* Process_Wire_Segments() */

/*-----------------------------------------------------------------------*/

/*  Process_Surface_Patches()
 *
 *  Processes surface patch data so they can be drawn on Screen.
 *  Each patch is represented by 4 rectangle edges from geom_pre.patch_corners.
 *  Occupies structure_segs[data.n + 4*idx] through [data.n + 4*idx + 3].
 */
  void
Process_Surface_Patches( view_t *v, double scale )
{
  int idx;

  if( geom_pre.patch_corners == NULL || structure_segs == NULL )
    return;

  for( idx = 0; idx < data.m; idx++ )
  {
    int base = data.n + 4 * idx;
    patch_corners_t *pc = &geom_pre.patch_corners[idx];

    /* Edge c0-c1 */
    Set_Gdk_Segment(&structure_segs[base + 0], v, scale,
        pc->c0x, pc->c0y, pc->c0z,
        pc->c1x, pc->c1y, pc->c1z,
        &structure_segs[base + 0].z_mid);

    /* Edge c1-c2 */
    Set_Gdk_Segment(&structure_segs[base + 1], v, scale,
        pc->c1x, pc->c1y, pc->c1z,
        pc->c2x, pc->c2y, pc->c2z,
        &structure_segs[base + 1].z_mid);

    /* Edge c2-c3 */
    Set_Gdk_Segment(&structure_segs[base + 2], v, scale,
        pc->c2x, pc->c2y, pc->c2z,
        pc->c3x, pc->c3y, pc->c3z,
        &structure_segs[base + 2].z_mid);

    /* Edge c3-c0 */
    Set_Gdk_Segment(&structure_segs[base + 3], v, scale,
        pc->c3x, pc->c3y, pc->c3z,
        pc->c0x, pc->c0y, pc->c0z,
        &structure_segs[base + 3].z_mid);
  }

} /* Process_Surface_Patches() */

/*-----------------------------------------------------------------------*/
