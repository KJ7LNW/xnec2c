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

#ifndef COLOR_EDGE_H
#define COLOR_EDGE_H  1

#include "../common.h"

/*-----------------------------------------------------------------------
 * Derivation-input snapshot shared by every color recompute edge: the
 * source prepare edge, the family-apply edge, and the per-output frame
 * producers.  Each edge fills the subset it derives from; unused fields
 * hold a consistent sentinel (0, or -1 for fstep/fam) and compare equal,
 * so one comparison helper serves all edges.
 *----------------------------------------------------------------------*/
typedef struct
{
  int      fstep;         /* prepare: freq step; -1 sentinel (empty)         */
  int      elem;          /* prepare: chroma_elem_t                          */
  int      chan;          /* prepare: chroma_channel_t (frame: base channel) */
  int      fam;           /* apply:   color_tone_t; -1 sentinel              */
  int      proj;          /* frame:   chroma_proj_t                          */
  int      seg_scale_enc; /* frame:   seg_scale_enc_t                        */
  unsigned flags;         /* frame:   producer control bits                  */
  double   freq_mhz;      /* prepare: frequency (FREQ_EQ)                    */
  double   cmax;          /* prepare: normalization peak                     */
  double   param;         /* apply:   family natural parameter               */
  double   phase;         /* frame:   effective phase, 0 if invariant        */
  double   flr;           /* frame:   brightness floor                       */
  uint32_t gen_a;         /* frame:   primary source content generation      */
  uint32_t gen_b;         /* frame:   secondary source content generation    */
  uint32_t palette;       /* frame:   palette LUT generation                 */
} color_edge_t;

/**
 * color_edge_eq() - Whether two edge snapshots carry identical inputs
 * @a: stored snapshot
 * @b: candidate snapshot
 *
 * The single point of truth for edge comparison: exact on the integer
 * fields, FREQ_EQ on the frequency, dl_feq on the remaining doubles.
 */
static inline gboolean
color_edge_eq(const color_edge_t *a, const color_edge_t *b)
{
  return a->fstep == b->fstep && a->elem == b->elem && a->chan == b->chan
      && a->fam == b->fam && a->proj == b->proj
      && a->seg_scale_enc == b->seg_scale_enc
      && a->flags == b->flags
      && a->gen_a == b->gen_a && a->gen_b == b->gen_b
      && a->palette == b->palette
      && FREQ_EQ(a->freq_mhz, b->freq_mhz) && dl_feq(a->cmax, b->cmax)
      && dl_feq(a->param, b->param) && dl_feq(a->phase, b->phase)
      && dl_feq(a->flr, b->flr);
}

#endif
