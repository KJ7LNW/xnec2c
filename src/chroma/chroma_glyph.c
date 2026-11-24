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
 * chroma_glyph: node/antinode current-marker overlay.
 *
 * One owner for the glyph presentation: the enum-indexed shape table
 * pairing each mark's tick strokes in the unit [-1,1] plane with its theme
 * color role, and the phase-invariant marking of envelope extrema along wire
 * connectivity.  Each structure backend maps the unit strokes through
 * its own 2D-screen or 3D-world basis.
 */
#include "chroma_glyph.h"
#include "../shared.h"

/* Envelope thresholds selecting extrema as marks: a node sits near the
 * current null, an antinode near the peak. */
#define GLYPH_NODE_ENV_MAX      0.2f
#define GLYPH_ANTINODE_ENV_MIN  0.8f

/* Plus tick: arms along the unit axes, marking a current null */
static const glyph_stroke_t glyph_node_strokes[] = {
  { -1.0f, 0.0f, 1.0f, 0.0f },
  { 0.0f, -1.0f, 0.0f, 1.0f } };

/* Cross tick: arms along the pre-normalized unit diagonals, marking a
 * current peak; M_SQRT1_2 lets a single uniform scale reproduce both
 * backends' corner distances exactly */
static const glyph_stroke_t glyph_antinode_strokes[] = {
  { -M_SQRT1_2, -M_SQRT1_2, M_SQRT1_2, M_SQRT1_2 },
  { -M_SQRT1_2,  M_SQRT1_2, M_SQRT1_2, -M_SQRT1_2 } };

/* Node and antinode marks take gradient roles the legacy theme holds at cyan
 * and red; an unmarked segment draws no stroke, so its color is never drawn */
const glyph_shape_t glyph_shapes[GLYPH_NUM] = {
  [GLYPH_NONE]     = { .n_strokes = 0 },
  [GLYPH_NODE]     = { .role = THEME_ROLE_GRAD_2,
                       .strokes = glyph_node_strokes,     .n_strokes = 2 },
  [GLYPH_ANTINODE] = { .role = THEME_ROLE_GRAD_5,
                       .strokes = glyph_antinode_strokes, .n_strokes = 2 },
};

/*-----------------------------------------------------------------------*/

  void
chroma_glyph_mark_nodes(const chroma_source_t *cs, unsigned char *flags)
{
  int i;

  for( i = 0; i < cs->n; i++ )
  {
    int prev = chroma_seg_conn(data.segments[i].icon1, i + 1);
    int next = chroma_seg_conn(data.segments[i].icon2, i + 1);
    float e = cs->env[i];
    float ep, en;

    if( prev >= 0 && !chroma_seg_linked(prev, i + 1) )
      prev = -1;
    if( next >= 0 && !chroma_seg_linked(next, i + 1) )
      next = -1;

    /* A missing neighbor never blocks its side of the extremum test */
    ep = (prev >= 0) ? cs->env[prev] : e;
    en = (next >= 0) ? cs->env[next] : e;

    if( e <= ep && e <= en && e < GLYPH_NODE_ENV_MAX )
      flags[i] = GLYPH_NODE;
    else if( e >= ep && e >= en && e > GLYPH_ANTINODE_ENV_MIN )
      flags[i] = GLYPH_ANTINODE;
    else
      flags[i] = GLYPH_NONE;
  }
}

/*-----------------------------------------------------------------------*/
