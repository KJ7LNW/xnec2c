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

#ifndef CHROMA_GLYPH_H
#define CHROMA_GLYPH_H  1

#include "../common.h"
#include "chroma_source.h"
#include "../themes/theme.h"

/* Node/antinode glyph marks written by chroma_glyph_mark_nodes(); the
 * trailing count makes glyph_shapes[] enum-indexed. */
enum
{
  GLYPH_NONE = 0,
  GLYPH_NODE,      /* envelope local minimum: current null */
  GLYPH_ANTINODE,  /* envelope local maximum: current peak */
  GLYPH_NUM
};

/* One tick stroke in the unit [-1,1] glyph plane; the backend maps it
 * through its own basis and scale. */
typedef struct { float ax, ay, bx, by; } glyph_stroke_t;

/* A glyph marker's full presentation: its theme color role and its tick
 * strokes. */
typedef struct
{
  theme_role_e          role;
  const glyph_stroke_t *strokes;
  int                   n_strokes;
} glyph_shape_t;

/* Enum-indexed marker table: the single source of a glyph's shape and
 * color role.  Dispatch resolves the role through the active theme, and each
 * backend maps the strokes through its own basis. */
extern const glyph_shape_t glyph_shapes[GLYPH_NUM];

/**
 * chroma_glyph_mark_nodes() - Mark envelope extrema along wire connectivity
 * @cs:    prepared wire source whose env array is read
 * @flags: output marks [cs->n], one of the GLYPH_* values
 *
 * A node is a local envelope minimum against both mutually-linked
 * neighbors below 0.2 of full scale (free ends qualify); an antinode is
 * a local maximum above 0.8.  Phase-invariant.
 */
void chroma_glyph_mark_nodes(const chroma_source_t *cs, unsigned char *flags);

#endif
