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

#ifndef CHROMA_H
#define CHROMA_H  1

#include "../prerender/prerender_color.h"
#include "../color/color_tone.h"
#include "chroma_source.h"
#include "../color/color_palette.h"

/*-----------------------------------------------------------------------
 * Color projection axis: a typed (channel, encoding) pair per carrier.
 * AMPLITUDE is the static default and the fall-through baseline; time
 * variation lives in one carrier per row (hue or brightness), never as a
 * global brightness collapse into black.
 *----------------------------------------------------------------------*/
typedef enum
{
  CHROMA_PROJ_AMPLITUDE = 0, /* ramp(s(n)), phase-invariant; static default */
  CHROMA_PROJ_INSTANT,       /* ramp(s(n)) · (floor + (1−floor)·|cos(arg z + φ)|) */
  CHROMA_PROJ_SIGNED,      /* hue sign(cos(arg z + φ)), value s(n) */
  CHROMA_PROJ_PHASE,         /* hue arg z + φ on the cyclic palette, value s(n) */
  CHROMA_PROJ_DUAL,          /* hue sign of charge, value current envelope */
  CHROMA_PROJ_STANDING,      /* hue ramp(per-region SWR), value s(n) */
  CHROMA_PROJ_FARFIELD,      /* hue ramp(far-field contribution), value s(n) */
  CHROMA_PROJ_NUM
} chroma_proj_t;

/* Hue carrier encoding; selects the palette kind via chroma_proj_palette_kind */
typedef enum
{
  HUE_MAG_RAMP = 0,     /* magnitude intensity on the rainbow ramp */
  HUE_SIGN_DIVERGING,   /* instantaneous sign on the diverging palette */
  HUE_PHASE_CYCLIC,     /* phase angle on the cyclic palette */
  HUE_ENC_NUM
} hue_enc_t;

/* Brightness carrier encoding; the projection row owns this choice */
typedef enum
{
  LUM_ENVELOPE = 0,     /* phase-invariant s(n): the flicker-free default */
  LUM_ABS_INSTANT,      /* s(n) · |cos(arg z + φ)|, remapped onto [floor, 1] */
  LUM_RAISED_COS,       /* s(n) · (1 + cos(arg z + φ)) / 2 */
  LUM_CONSTANT,         /* 1: hue carries everything */
  LUM_ENC_NUM
} lum_enc_t;

/* Select a geometry carrier independently from the color projection */
typedef enum
{
  SEG_SCALE_ENC_MODEL = 0,  /* no carrier: the model radius alone */
  SEG_SCALE_ENC_ENVELOPE,   /* phase-invariant s(n) */
  SEG_SCALE_ENC_INSTANT,    /* s(n) · |cos(arg z + φ)| */
  SEG_SCALE_ENC_COUNT
} seg_scale_enc_t;

/* One segment scale encoding: the magnitude evaluation it shares with the
 * brightness carrier, and the closed form shown beside the selector. */
typedef struct
{
  lum_enc_t   lum_enc;  /* magnitude evaluation feeding the gain span */
  const char *formula;  /* Pango markup closed form */
} seg_scale_enc_row_t;

/* Enum-indexed segment scale encoding table */
extern const seg_scale_enc_row_t seg_scale_enc_rows[SEG_SCALE_ENC_COUNT];

/* One projection: typed (channel, encoding) carrier assignments and the
 * closed-form rendering.  A hue_src or lum_src of CHAN_CURRENT resolves
 * to the dispatch-selected base channel (current or charge display);
 * other channels are absolute. */
typedef struct
{
  chroma_channel_t  hue_src;  /* physical channel feeding the hue carrier */
  hue_enc_t       hue_enc;  /* hue encoding, typing the palette */
  chroma_channel_t  lum_src;  /* physical channel feeding brightness */
  lum_enc_t       lum_enc;  /* brightness encoding */
  const char     *formula;  /* Pango markup closed form */
} chroma_proj_row_t;

/* Enum-indexed projection table */
extern const chroma_proj_row_t chroma_proj_rows[CHROMA_PROJ_NUM];

/* Per-frame evaluation context: the animation phase and the cross-cutting
 * carrier controls.  The family transfer is precomputed into the prepared
 * source's s_env, so the context carries no family state. */
typedef struct chroma_ctx_s
{
  double c, s;              /* cos/sin of the animation phase */
  double phase;             /* animation phase in radians */
  double lum_floor;         /* brightness floor in [0,1], linear light */
} chroma_ctx_t;

/**
 * chroma_ctx_init() - Fill an evaluation context from the animation phase
 * @x:      context to fill
 * @phase:  animation phase in radians
 *
 * Precomputes cos/sin of the phase and latches the brightness floor from
 * rc_config for the frame.
 */
void chroma_ctx_init(chroma_ctx_t *x, double phase);

/**
 * chroma_proj_palette_kind() - Palette kind typed by a hue encoding
 * @enc: hue carrier encoding
 */
palette_kind_t chroma_proj_palette_kind(hue_enc_t enc);

/**
 * chroma_project_frame() - Compose one frame of element colors, pure
 * @out:    destination buffer [n]
 * @hue_cs: prepared source feeding the hue carrier
 * @lum_cs: prepared source feeding the brightness carrier
 * @row:    projection carrier assignments
 * @x:      evaluation context
 * @n:      element count
 *
 * Per element: t = re_n·cosφ − im_n·sinφ, hue coordinate per hue_enc,
 * brightness per lum_enc remapped onto [x->lum_floor, 1], composed in linear
 * light via palette_lookup_scaled.  Runs every animation frame; the only
 * per-frame transcendentals are the shared cos φ / sin φ in the context.
 */
void chroma_project_frame(rgb_f_t *out, const chroma_source_t *hue_cs,
    const chroma_source_t *lum_cs, const chroma_proj_row_t *row,
    const chroma_ctx_t *x, int n);

/**
 * chroma_proj_sanitize() - Bound a persisted projection value to the enum
 * @v: raw persisted integer
 *
 * Emits pr_err and returns CHROMA_PROJ_INSTANT when out of range.
 */
chroma_proj_t chroma_proj_sanitize(int v);

/**
 * chroma_proj_selected() - Resolve the projection selection now in effect
 *
 * Reads the committed rc_config selection; a hover stages its candidate into
 * that field.  Animation state does not enter (see chroma_proj_animated).
 */
chroma_proj_t chroma_proj_selected(void);

/**
 * chroma_proj_frame_wire() - Resolve wire colors for one frame
 * @fstep:     frequency step index
 * @phase:     animation phase in radians
 * @proj:      projection selection
 * @fam:       scale family selection
 * @base_chan: dispatch-selected display quantity (CHAN_CURRENT or CHAN_CHARGE)
 *
 * Prepares the row's hue and brightness sources on their edges, then
 * composes the frame into module scratch and returns it.  Falls back to
 * the static geometry colors (seg_rgb) when the scratch, the model, or
 * the prepared source is empty.
 */
const rgb_f_t *chroma_proj_frame_wire(int fstep, double phase,
    chroma_proj_t proj, color_tone_t fam, chroma_channel_t base_chan);

/**
 * chroma_proj_frame_patch() - Resolve patch fill colors for one frame
 * @fstep: frequency step index
 * @phase: animation phase in radians
 * @proj:  projection selection
 * @fam:   scale family selection
 *
 * Patch base channel is always CHAN_CURRENT; a row channel without patch
 * support falls back to the current-envelope ramp.
 */
const rgb_f_t *chroma_proj_frame_patch(int fstep, double phase,
    chroma_proj_t proj, color_tone_t fam);

/**
 * chroma_proj_frame_wire_glyphs() - Resolve node/antinode marks for one frame
 * @fstep:     frequency step index
 * @proj:      projection selection
 * @fam:       scale family selection
 * @base_chan: dispatch-selected display quantity
 *
 * Returns the phase-invariant GLYPH_* marks derived from the brightness
 * source's envelope extrema, or NULL when the overlay is off or no data
 * bound exists.
 */
const unsigned char *chroma_proj_frame_wire_glyphs(int fstep,
    chroma_proj_t proj, color_tone_t fam, chroma_channel_t base_chan);

/**
 * chroma_proj_seg_scale_identity() - Publish unity gains for every segment
 *
 * Serves the display quantities that carry no amplitude envelope, so their
 * wires render at the model radius each backend already draws.
 */
const float *chroma_proj_seg_scale_identity(void);

/**
 * seg_scale_enc_sanitize() - Bound a persisted segment scale encoding to the enum
 * @v: raw persisted integer
 *
 * Emits pr_err and returns SEG_SCALE_ENC_MODEL when out of range.
 */
seg_scale_enc_t seg_scale_enc_sanitize(int v);

/**
 * seg_scale_enc_selected() - Resolve the segment scale encoding selection in effect
 *
 * Reads the committed rc_config selection; a hover stages its candidate
 * into that field, so a previewed row and the row it commits to resolve
 * alike.  Playback state does not enter: a stopped phase still names a
 * definite instantaneous magnitude.
 */
seg_scale_enc_t seg_scale_enc_selected(void);

/**
 * chroma_proj_frame_seg_scale() - Resolve per-segment size gains for one frame
 * @fstep:     frequency step index
 * @phase:     animation phase in radians
 * @proj:      projection selection naming the brightness source
 * @enc:       segment scale carrier encoding
 * @fam:       scale family selection
 * @base_chan: dispatch-selected display quantity
 *
 * Returns dimensionless gains that each backend multiplies into the model
 * radius it draws.  SEG_SCALE_ENC_MODEL and a constant scale family carry no
 * magnitude and yield identity gains, as does an unbound source.
 */
const float *chroma_proj_frame_seg_scale(int fstep, double phase,
    chroma_proj_t proj, seg_scale_enc_t enc, color_tone_t fam,
    chroma_channel_t base_chan);

/**
 * chroma_proj_generation() - Read the structure-draw generation counter
 *
 * Bumps once per producer output change, never on an edge-gated cache
 * hit; GL geometry keys its color staleness on this value beside the
 * color pointer.
 */
uint32_t chroma_proj_generation(void);

/**
 * chroma_proj_animated() - Whether a projection varies with animation phase
 * @proj: projection selection
 *
 * TRUE for the phase-carrying projections (INSTANT, SIGNED, PHASE, DUAL);
 * FALSE for the static reads (AMPLITUDE, STANDING, FARFIELD) and
 * out-of-range values.  Drives overlay-widget sensitivity.
 */
gboolean chroma_proj_animated(int proj);

/**
 * chroma_proj_alloc() - Size the projection scratch buffers to the model
 *
 * Called from alloc_struct_colors(); wire scratch [data.n], patch
 * scratch [data.m].
 */
void chroma_proj_alloc(void);

/**
 * chroma_proj_free() - Release the projection scratch and prepared sources
 *
 * Called from free_struct_colors().
 */
void chroma_proj_free(void);

#endif
