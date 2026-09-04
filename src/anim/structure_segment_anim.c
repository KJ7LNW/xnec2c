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
 * structure_segment_anim: the wire segment animation class.
 *
 * Segments carry four colour channels under four effects: colour, segment
 * scale, comet, and node glyphs.  The effect rows live here, so adding an
 * effect adds one row rather than a branch in the registry.
 */
#include "structure_segment_anim.h"
#include "../chroma/chroma.h"
#include "../color/color_tone.h"
#include "../shared.h"

/* Enumerate the effects the segment class folds into its phase answer */
typedef enum
{
  SEG_EFFECT_COLOR = 0,
  SEG_EFFECT_SCALE,
  SEG_EFFECT_COMET,
  SEG_EFFECT_GLYPH,
  SEG_EFFECT_COUNT
} seg_effect_t;

static gboolean seg_color_varies(void);
static gboolean seg_scale_varies(void);
static gboolean seg_comet_available(void);
static gboolean seg_glyph_available(void);

static gboolean structure_segment_anim_available(void);
static gboolean structure_segment_anim_presented(void);
static gboolean structure_segment_anim_varies(void);

/* Colour and scale always apply; comet and glyphs carry their own gate.
 * Glyphs read the envelope extrema alone, so they never carry phase. */
static const anim_member_row_t seg_effect_rows[] = {
  [SEG_EFFECT_COLOR] = {
      .available = NULL, .varies_with_phase = seg_color_varies },
  [SEG_EFFECT_SCALE] = {
      .available = NULL, .varies_with_phase = seg_scale_varies },
  [SEG_EFFECT_COMET] = {
      .available = seg_comet_available,
      .varies_with_phase = seg_color_varies },
  [SEG_EFFECT_GLYPH] = {
      .available = seg_glyph_available, .varies_with_phase = NULL },
};

_Static_assert(G_N_ELEMENTS(seg_effect_rows) == SEG_EFFECT_COUNT,
    "Segment effect rows must cover every effect");

const anim_class_ops_t structure_segment_anim_ops = {
  .available         = structure_segment_anim_available,
  .presented         = structure_segment_anim_presented,
  .varies_with_phase = structure_segment_anim_varies,
};

/**
 * seg_color_varies() - Report a colour selection that carries phase
 */
  static gboolean
seg_color_varies(void)
{
  return chroma_proj_animated(chroma_proj_selected());
}

/**
 * seg_scale_varies() - Report a segment scale selection that carries phase
 *
 * The model radius states no magnitude and a constant tone transfer holds
 * every envelope at one, so both leave the gains at identity.  Of the
 * remaining encodings the instantaneous evaluations alone read phase.
 */
  static gboolean
seg_scale_varies(void)
{
  seg_scale_enc_t enc = seg_scale_enc_selected();
  lum_enc_t lum = seg_scale_enc_rows[enc].lum_enc;

  return (enc != SEG_SCALE_ENC_MODEL) && (color_tone_active() != COLOR_TONE_NONE)
      && ((lum == LUM_ABS_INSTANT) || (lum == LUM_RAISED_COS));
}

/**
 * seg_comet_available() - Report the comet overlay selection
 */
  static gboolean
seg_comet_available(void)
{
  return rc_config.overlay_comet != 0;
}

/**
 * seg_glyph_available() - Report the node and antinode overlay selection
 */
  static gboolean
seg_glyph_available(void)
{
  return rc_config.overlay_nodes != 0;
}

/**
 * structure_segment_anim_available() - Report wire segments in the model
 */
  static gboolean
structure_segment_anim_available(void)
{
  return data.n > 0;
}

/**
 * structure_segment_anim_presented() - Report a view showing segment colour
 */
  static gboolean
structure_segment_anim_presented(void)
{
  return struct_view_currents() || struct_view_charges();
}

/**
 * structure_segment_anim_varies() - Fold the effect rows into one answer
 */
  static gboolean
structure_segment_anim_varies(void)
{
  return anim_member_rows_vary(seg_effect_rows, G_N_ELEMENTS(seg_effect_rows));
}
