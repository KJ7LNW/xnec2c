/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

/*
 * freqplots_locus: per-view click-resolution registry.
 *
 * Every plot producer deposits its rendered frequency samples here during
 * draw; every click consumer resolves a pixel against the deposited geometry.
 * The registry is the boundary between producers and consumers: the rendered
 * sample set is the single source for click-to-frequency resolution, so a
 * click snaps to whatever the panel actually drew, including the off-grid
 * green-line extra slot once it enters the sample set.
 */

#include "freqplots_locus.h"
#include "../shared.h"

#include <string.h>

/* True when (px,py) lies within the locus bounding rectangle. */
  static gboolean
locus_contains( const fp_locus_t *l, double px, double py )
{
  return px >= l->rect.x
      && px <= l->rect.x + l->rect.width
      && py >= l->rect.y
      && py <= l->rect.y + l->rect.height;
}

/* locus_resolve()
 *
 * Resolves a pixel on one locus to a frequency.  snap picks the nearest
 * sample vertex by pixel distance; otherwise the pixel is projected onto the
 * nearest segment and the bracketing sample frequencies are interpolated by
 * the clamped projection fraction.  A single-sample locus returns its lone
 * frequency, the projection never extrapolating beyond the outermost sample.
 */
  static double
locus_resolve( const fp_locus_t *l, double px, double py, gboolean snap )
{
  int idx;

  /* Secondary click: nearest snap vertex by pixel distance.  The snap set is
   * the discrete existing slots a secondary click selects. */
  if( snap )
  {
    int    best = 0;
    double dx = px - l->snap_pts[0].x;
    double dy = py - l->snap_pts[0].y;
    double best_d2 = dx * dx + dy * dy;

    for( idx = 1; idx < l->snap_n; idx++ )
    {
      dx = px - l->snap_pts[idx].x;
      dy = py - l->snap_pts[idx].y;
      double d2 = dx * dx + dy * dy;
      if( dl_flt(d2, best_d2) )
      {
        best_d2 = d2;
        best = idx;
      }
    }

    return l->snap_freq[best];
  }

  /* Single sample: no segment to project onto */
  if( l->n == 1 )
    return l->freq[0];

  /* Primary click: project onto the nearest segment and interpolate the
   * bracketing sample frequencies by the clamped projection fraction */
  int    best_i = 0;
  double best_t = 0.0;
  double best_d2 = 0.0;

  for( idx = 0; idx < l->n - 1; idx++ )
  {
    double ax = l->pts[idx].x;
    double ay = l->pts[idx].y;
    double vx = l->pts[idx + 1].x - ax;
    double vy = l->pts[idx + 1].y - ay;
    double len2 = vx * vx + vy * vy;
    double t = 0.0;

    if( dl_fgt(len2, 0.0) )
        t = ( (px - ax) * vx + (py - ay) * vy ) / len2;

    if( t < 0.0 ) t = 0.0;
    else if( t > 1.0 ) t = 1.0;

    double cx = ax + t * vx;
    double cy = ay + t * vy;
    double dx = px - cx;
    double dy = py - cy;
    double d2 = dx * dx + dy * dy;

    if( idx == 0 || dl_flt(d2, best_d2) )
    {
      best_d2 = d2;
      best_i  = idx;
      best_t  = t;
    }
  }

  return l->freq[best_i]
      + best_t * ( l->freq[best_i + 1] - l->freq[best_i] );

} /* locus_resolve() */

/*-----------------------------------------------------------------------*/

/* First valid locus whose rect contains the pixel, or NULL.  Panel rects are
 * disjoint, so the first containing match is unambiguous. */
  static const fp_locus_t *
locus_at( freqplots_view_t *v, double px, double py )
{
  int i;

  for( i = 0; i < v->loci_n; i++ )
  {
    const fp_locus_t *l = &v->loci[i];
    if( l->valid && l->n > 0 && locus_contains(l, px, py) )
      return l;
  }

  return NULL;
}

/*-----------------------------------------------------------------------*/

  void
fp_locus_frame_begin( freqplots_view_t *v )
{
  v->loci_n = 0;
}

/*-----------------------------------------------------------------------*/

/* Reallocate one owned vertex/frequency pair to n entries and copy the
 * borrowed source arrays in.  Both locus roles deposit through this one path. */
  static void
locus_copy( GdkPoint **dst_pts, double **dst_freq,
    const GdkPoint *src_pts, const double *src_freq, int n )
{
  mem_array_realloc( dst_pts,  n );
  mem_array_realloc( dst_freq, n );
  mem_array_cpy(*dst_pts, src_pts, n);
  mem_array_cpy(*dst_freq, src_freq, n);
}

/*-----------------------------------------------------------------------*/

  void
fp_locus_add( freqplots_view_t *v, const fp_locus_input_t *in )
{
  fp_locus_t *l;

  if( in->cont_n == 0 )
    return;

  /* Grow the registry through the shared resize helper; surviving slots keep
   * their owned buffers and the allocator zeroes newly exposed slots so their
   * buffers start NULL for mem_realloc reuse on this and later frames. */
  mem_array_reserve(&v->loci, v->loci_n + 1, 4);

  l = &v->loci[v->loci_n];

  /* Deep-copy both roles so the view owns its samples against the shared fplot
   * static the last drawn view overwrites. */
  locus_copy( &l->pts, &l->freq, in->cont_pts, in->cont_freq, in->cont_n );
  locus_copy( &l->snap_pts, &l->snap_freq,
      in->snap_pts, in->snap_freq, in->snap_n );

  l->panel  = in->panel;
  l->rect   = in->rect;
  l->n      = in->cont_n;
  l->snap_n = in->snap_n;
  l->valid  = TRUE;

  v->loci_n++;
}

/*-----------------------------------------------------------------------*/

  gboolean
fp_locus_freq_at_pixel( freqplots_view_t *v,
    double px, double py, gboolean snap, double *fmhz )
{
  const fp_locus_t *l = locus_at( v, px, py );

  if( l == NULL )
    return FALSE;

  *fmhz = locus_resolve( l, px, py, snap );
  return TRUE;
}

/*-----------------------------------------------------------------------*/

  fp_panel_t
fp_locus_panel_at( freqplots_view_t *v, double px, double py )
{
  const fp_locus_t *l = locus_at( v, px, py );

  if( l == NULL )
    return FP_PANEL_ALL;

  return l->panel;
}

/*-----------------------------------------------------------------------*/

  void
fp_locus_free( freqplots_view_t *v )
{
  int i;

  for( i = 0; i < mem_array_count(v->loci); i++ )
  {
    mem_array_free( &v->loci[i].pts );
    mem_array_free( &v->loci[i].freq );
    mem_array_free( &v->loci[i].snap_pts );
    mem_array_free( &v->loci[i].snap_freq );
  }

  mem_array_free(&v->loci);
  v->loci_n = 0;
}
