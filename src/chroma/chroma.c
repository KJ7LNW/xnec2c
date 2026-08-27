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
 * chroma: typed projection table and per-frame colorize.
 *
 * A projection assigns a (channel, encoding) pair to the hue carrier and
 * one to the brightness carrier.  All phase-invariant derivation lives
 * in the prepared chroma_source_t arrays; chroma_project_frame is the
 * pure per-frame combine t = Re(z·e^{jφ})/cmax plus palette lookup,
 * composed in linear light.  The frame entries resolve the row's sources
 * against the dispatch-selected base channel, gate composition on an
 * input-edge snapshot, and bump the draw generation once per actual
 * output change so a static display costs only scalar comparisons.
 */
#include "chroma.h"
#include "chroma_glyph.h"
#include "../shared.h"

const chroma_proj_row_t chroma_proj_rows[CHROMA_PROJ_NUM] = {
  [CHROMA_PROJ_AMPLITUDE] = {
    .hue_src = CHAN_CURRENT, .hue_enc = HUE_MAG_RAMP,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_CONSTANT,
    .formula = "c = ramp(s(n))" },
  [CHROMA_PROJ_INSTANT] = {
    .hue_src = CHAN_CURRENT, .hue_enc = HUE_MAG_RAMP,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_ABS_INSTANT,
    .formula = "c = ramp(s(n)) · (floor + (1−floor)·|cos(arg z + φ)|)" },
  [CHROMA_PROJ_SIGNED] = {
    .hue_src = CHAN_CURRENT, .hue_enc = HUE_SIGN_DIVERGING,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_ENVELOPE,
    .formula = "hue = ±cos(arg z + φ),  value = s(n)" },
  [CHROMA_PROJ_PHASE] = {
    .hue_src = CHAN_CURRENT, .hue_enc = HUE_PHASE_CYCLIC,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_ENVELOPE,
    .formula = "hue = arg z + φ,  value = s(n)" },
  [CHROMA_PROJ_DUAL] = {
    .hue_src = CHAN_CHARGE,  .hue_enc = HUE_SIGN_DIVERGING,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_ENVELOPE,
    .formula = "hue = ±q(φ),  value = s(|I|)" },
  [CHROMA_PROJ_STANDING] = {
    .hue_src = CHAN_SWR,     .hue_enc = HUE_MAG_RAMP,
    .lum_src = CHAN_CURRENT, .lum_enc = LUM_ENVELOPE,
    .formula = "hue = ramp(SWR),  value = s(n)" },
  [CHROMA_PROJ_FARFIELD] = {
    .hue_src = CHAN_FFCONTRIB, .hue_enc = HUE_MAG_RAMP,
    .lum_src = CHAN_CURRENT,   .lum_enc = LUM_ENVELOPE,
    .formula = "hue = ramp(contribution),  value = s(n)" },
};

/* Hue encoding -> palette kind, the single typing of hue meaning */
static const palette_kind_t hue_palette_kinds[HUE_ENC_NUM] = {
  [HUE_MAG_RAMP]       = PALETTE_RAMP,
  [HUE_SIGN_DIVERGING] = PALETTE_DIVERGING,
  [HUE_PHASE_CYCLIC]   = PALETTE_CYCLIC,
};

/* Neutral fill for patches under a hue channel they cannot express */
#define PATCH_DIM_RGB  ((rgb_f_t){ 0.55f, 0.55f, 0.55f })

/* Comet tail brightness floor keeping the tail visible; the user's
 * brightness floor raises it, and the crest gain is its complement */
#define COMET_HEAD_FLOOR  0.25

/* Width-carrier floor keeping every segment visible at its current null */
#define WIDTH_MIN_RATIO   0.15f

/* Frame scratch, sized to the model by chroma_proj_alloc() */
static rgb_f_t *wire_proj_rgb   = NULL;  /* [data.n] composed wire colors */
static rgb_f_t *patch_proj_rgb  = NULL;  /* [data.m] composed patch colors */
static float   *wire_proj_width = NULL;  /* [data.n] width-carrier widths */
static unsigned char *wire_glyph_flags = NULL; /* [data.n] GLYPH_* marks */

/* Prepared sources; selection happens per frame entry, storage here so
 * free_struct_colors() tears them down with the scratch. */
static chroma_source_t wire_hue_src;
static chroma_source_t wire_lum_src;
static chroma_source_t patch_src;

/* Structure-draw generation, bumped by gate_store once per producer
 * output change; the GL geometry gate keys color staleness on it. */
static uint32_t proj_generation = 0;

/* Producer control bits entering a frame edge's flags */
#define FRAME_FLAG_COMET  (1u << 0)
#define FRAME_FLAG_NODES  (1u << 1)
#define FRAME_FLAG_WIDTH  (1u << 2)

/* Per-output recompute gate: the last composed input edge and returned
 * pointer.  An all-zero edge is the data-off sentinel (a prepared
 * source's gen is nonzero), so availability transitions miss and bump;
 * valid FALSE forces the first compose after a reset. */
typedef struct
{
  color_edge_t edge;
  gboolean     valid;
} proj_gate_t;

static proj_gate_t wire_gate, patch_gate, glyph_gate, width_gate;

static const rgb_f_t       *wire_result  = NULL;
static const rgb_f_t       *patch_result = NULL;
static const unsigned char *glyph_result = NULL;
static const float         *width_result = NULL;

/*-----------------------------------------------------------------------*/

/**
 * gate_hit() - Whether a producer's inputs match its cached compose
 * @g:    producer gate
 * @want: current input snapshot
 */
static gboolean
gate_hit(const proj_gate_t *g, const color_edge_t *want)
{
  return g->valid && color_edge_eq(&g->edge, want);
}

/**
 * gate_store() - Record a changed output and advance the draw generation
 * @g:    producer gate
 * @want: input snapshot the fresh output derives from
 *
 * The single bump site for proj_generation: exactly one per producer
 * output change, none on a cache hit.
 */
static void
gate_store(proj_gate_t *g, const color_edge_t *want)
{
  proj_generation++;
  g->edge  = *want;
  g->valid = TRUE;
}

/**
 * gate_reset() - Drop every producer cache
 *
 * The next frame recomposes each output and bumps the generation.
 */
static void
gate_reset(void)
{
  wire_gate  = (proj_gate_t){ 0 };
  patch_gate = (proj_gate_t){ 0 };
  glyph_gate = (proj_gate_t){ 0 };
  width_gate = (proj_gate_t){ 0 };

  wire_result  = NULL;
  patch_result = NULL;
  glyph_result = NULL;
  width_result = NULL;
}

/*-----------------------------------------------------------------------*/

  void
chroma_ctx_init(chroma_ctx_t *x, double phase)
{
  x->phase  = phase;
  x->c      = cos(phase);
  x->s      = sin(phase);

  /* Latch the cross-cutting carrier controls for one frame */
  x->lum_floor = rc_config.color_lum_floor;
}

/*-----------------------------------------------------------------------*/

  palette_kind_t
chroma_proj_palette_kind(hue_enc_t enc)
{
  return hue_palette_kinds[enc];
}

/*-----------------------------------------------------------------------*/

  void
chroma_project_frame(rgb_f_t *out, const chroma_source_t *hue_cs,
    const chroma_source_t *lum_cs, const chroma_proj_row_t *row,
    const chroma_ctx_t *x, int n)
{
  const palette_t *pal = palette_get(hue_palette_kinds[row->hue_enc]);
  int i;

  for( i = 0; i < n; i++ )
  {
    /* Initializers cover the BUG fall-through of the enum switches */
    double coord = 0.0;
    double v = 1.0;
    double t;

    switch( row->hue_enc )
    {
      case HUE_MAG_RAMP:
        coord = (double)hue_cs->s_env[i];
        break;

      case HUE_SIGN_DIVERGING:
        /* Signed instantaneous value of the hue channel at phase φ,
         * continuous through the diverging midpoint so zero crossings
         * blend instead of snapping between the endpoint hues */
        t = (double)hue_cs->re_n[i] * x->c - (double)hue_cs->im_n[i] * x->s;
        coord = (hue_cs->env[i] > 0.0f)
            ? 0.5 + 0.5 * t / (double)hue_cs->env[i] : 0.5;
        break;

      case HUE_PHASE_CYCLIC:
        coord = ((double)hue_cs->arg[i] + x->phase) / M_2PI;
        break;

      case HUE_ENC_NUM:
        BUG("unhandled hue encoding %d\n", row->hue_enc);
        break;
    }

    switch( row->lum_enc )
    {
      case LUM_ENVELOPE:
        v = (double)lum_cs->s_env[i];
        break;

      case LUM_ABS_INSTANT:
        /* |cos(arg z + φ)| = |t| / env; envelope 0 rides the floor */
        t = (double)lum_cs->re_n[i] * x->c - (double)lum_cs->im_n[i] * x->s;
        v = (lum_cs->env[i] > 0.0f)
            ? (double)lum_cs->s_env[i] * fabs(t) / (double)lum_cs->env[i]
            : 0.0;
        break;

      case LUM_RAISED_COS:
        t = (double)lum_cs->re_n[i] * x->c - (double)lum_cs->im_n[i] * x->s;
        v = (lum_cs->env[i] > 0.0f)
            ? (double)lum_cs->s_env[i]
              * (0.5 + 0.5 * t / (double)lum_cs->env[i])
            : 0.0;
        break;

      case LUM_CONSTANT:
        v = 1.0;
        break;

      case LUM_ENC_NUM:
        BUG("unhandled brightness encoding %d\n", row->lum_enc);
        break;
    }

    /* Affine remap onto [floor, 1]: nulls stay darkest but visible and
     * the brightness ordering survives the whole range. */
    out[i] = palette_lookup_scaled(pal, coord,
        x->lum_floor + (1.0 - x->lum_floor) * v);
  }
}

/*-----------------------------------------------------------------------*/

  chroma_proj_t
chroma_proj_sanitize(int v)
{
  if( v < 0 || v >= CHROMA_PROJ_NUM )
  {
    pr_err("invalid color projection %d, using instant\n", v);
    return CHROMA_PROJ_INSTANT;
  }

  return (chroma_proj_t)v;
}

/*-----------------------------------------------------------------------*/

/** chroma_proj_selected() - Resolve the projection selection now in effect
 *
 * Reads the committed rc_config selection; a hover stages its candidate into
 * that field.  Animation state does not enter (see chroma_proj_animated).
 */
  chroma_proj_t
chroma_proj_selected(void)
{
  return chroma_proj_sanitize(rc_config.anim_color_proj);
}

/*-----------------------------------------------------------------------*/

/**
 * resolve_chan() - Resolve a row channel against the display base channel
 * @row_chan:  channel named by the projection row
 * @base_chan: dispatch-selected display quantity
 * @elem:      element domain the source will be prepared for
 *
 * CHAN_CURRENT in a row means "the displayed quantity"; other channels
 * are absolute.  A channel without support for the element domain falls
 * back to the base channel (eg the dual's charge hue on patches).
 */
static chroma_channel_t
resolve_chan(chroma_channel_t row_chan, chroma_channel_t base_chan,
    chroma_elem_t elem)
{
  chroma_channel_t chan = (row_chan == CHAN_CURRENT) ? base_chan : row_chan;

  if( !chroma_source_supported(elem, chan) )
    chan = base_chan;

  return chan;
}

/*-----------------------------------------------------------------------*/

/**
 * frame_sources() - Prepare a row's hue and brightness sources for wires
 * @row:       projection carrier assignments
 * @fstep:     frequency step index
 * @fam:       scale family selection
 * @base_chan: dispatch-selected display quantity
 *
 * Prepares wire_hue_src and wire_lum_src on their edges and applies the
 * family transfer to both; the two may name one channel, in which case
 * both prepares are the same no-op edge check on distinct storage.
 */
static void
frame_sources(const chroma_proj_row_t *row, int fstep,
    color_tone_t fam, chroma_channel_t base_chan)
{
  chroma_source_prepare(&wire_hue_src, CHROMA_ELEM_WIRE,
                        resolve_chan(row->hue_src, base_chan, CHROMA_ELEM_WIRE),
                        fstep);
  chroma_source_prepare(&wire_lum_src, CHROMA_ELEM_WIRE,
                        resolve_chan(row->lum_src, base_chan, CHROMA_ELEM_WIRE),
                        fstep);

  chroma_source_apply_family(&wire_hue_src, fam);
  chroma_source_apply_family(&wire_lum_src, fam);
}

/*-----------------------------------------------------------------------*/

/**
 * apply_comet() - Modulate colors with a traveling brightness head
 * @out:    composed colors to modulate in place [n]
 * @lum_cs: prepared brightness source carrying the phasors
 * @x:      evaluation context with the frame's cos φ / sin φ
 * @n:      element count
 *
 * Brightness factor base + (1 − base)·max(0, cos(arg z + φ))⁸, with base
 * the greater of the user brightness floor and COMET_HEAD_FLOOR: the head
 * rides the instantaneous positive crest along the wire, applied in
 * linear light so the base projection keeps its meaning and the tail
 * never dims below the floor slider.
 */
static void
apply_comet(rgb_f_t *out, const chroma_source_t *lum_cs,
    const chroma_ctx_t *x, int n)
{
  int i;
  double base = fmax(x->lum_floor, COMET_HEAD_FLOOR);

  for( i = 0; i < n; i++ )
  {
    double t = (double)lum_cs->re_n[i] * x->c
             - (double)lum_cs->im_n[i] * x->s;
    double ct = (lum_cs->env[i] > 0.0f)
        ? fmax(0.0, t / (double)lum_cs->env[i]) : 0.0;
    double c2 = ct * ct;
    double f = base + (1.0 - base) * c2 * c2 * c2 * c2;

    out[i] = rgb_linear_scale(out[i], f);
  }
}

  const rgb_f_t *
chroma_proj_frame_wire(int fstep, double phase,
    chroma_proj_t proj, color_tone_t fam, chroma_channel_t base_chan)
{
  const chroma_proj_row_t *row = &chroma_proj_rows[proj];
  gboolean animated = chroma_proj_animated(proj);
  gboolean comet = rc_config.overlay_comet && animated;
  color_edge_t want = { 0 };
  gboolean have = FALSE;
  chroma_ctx_t x;

  /* Phase leaves the edge for phase-invariant projections */
  chroma_ctx_init(&x, animated ? phase : 0.0);

  if( wire_proj_rgb != NULL && data.n > 0 )
  {
    frame_sources(row, fstep, fam, base_chan);

    if( wire_lum_src.n > 0 )
    {
      have = TRUE;
      want = (color_edge_t){ .proj = (int)proj,
          .flags = comet ? FRAME_FLAG_COMET : 0,
          .phase = x.phase, .flr = x.lum_floor,
          .gen_a = wire_hue_src.gen, .gen_b = wire_lum_src.gen,
          .palette = color_palette_generation() };
    }
  }

  if( gate_hit(&wire_gate, &want) )
    return wire_result;

  if( have )
  {
    chroma_project_frame(wire_proj_rgb, &wire_hue_src, &wire_lum_src,
        row, &x, data.n);

    /* Comet is a phase effect: it applies only under a phase-varying
     * projection, never the static reads or the idle AMPLITUDE
     * fallback, matching the overlay sensitivity model. */
    if( comet )
      apply_comet(wire_proj_rgb, &wire_lum_src, &x, data.n);

    wire_result = wire_proj_rgb;
  }
  else
  {
    /* Degraded modes fall back to the static geometry colors */
    wire_result = seg_rgb;
  }

  gate_store(&wire_gate, &want);
  return wire_result;
}

/*-----------------------------------------------------------------------*/

  const unsigned char *
chroma_proj_frame_wire_glyphs(int fstep,
    chroma_proj_t proj, color_tone_t fam, chroma_channel_t base_chan)
{
  color_edge_t want = { 0 };
  gboolean have = FALSE;

  if( rc_config.overlay_nodes != 0 && wire_glyph_flags != NULL
      && data.n > 0 )
  {
    frame_sources(&chroma_proj_rows[proj], fstep, fam, base_chan);

    if( wire_lum_src.n > 0 )
    {
      have = TRUE;
      want = (color_edge_t){ .flags = FRAME_FLAG_NODES,
          .gen_a = wire_lum_src.gen };
    }
  }

  if( gate_hit(&glyph_gate, &want) )
    return glyph_result;

  if( have )
  {
    chroma_glyph_mark_nodes(&wire_lum_src, wire_glyph_flags);
    glyph_result = wire_glyph_flags;
  }
  else
  {
    /* Overlay off or no data bound: no marks */
    glyph_result = NULL;
  }

  gate_store(&glyph_gate, &want);
  return glyph_result;
}

/*-----------------------------------------------------------------------*/

  const rgb_f_t *
chroma_proj_frame_patch(int fstep, double phase,
    chroma_proj_t proj, color_tone_t fam)
{
  const chroma_proj_row_t *row = &chroma_proj_rows[proj];
  chroma_proj_row_t patch_row;
  color_edge_t want = { 0 };
  gboolean have = FALSE;
  chroma_ctx_t x;

  /* Phase leaves the edge for phase-invariant projections */
  chroma_ctx_init(&x, chroma_proj_animated(proj) ? phase : 0.0);

  if( patch_proj_rgb != NULL && data.m > 0 )
  {
    chroma_source_prepare(&patch_src, CHROMA_ELEM_PATCH, CHAN_CURRENT, fstep);
    chroma_source_apply_family(&patch_src, fam);

    if( patch_src.n > 0 )
    {
      have = TRUE;
      want = (color_edge_t){ .proj = (int)proj,
          .phase = x.phase, .flr = x.lum_floor,
          .gen_a = patch_src.gen,
          .palette = color_palette_generation() };
    }
  }

  if( gate_hit(&patch_gate, &want) )
    return patch_result;

  if( !have )
  {
    /* Degraded modes fall back to the static geometry colors */
    patch_result = patch_rgb;
  }
  else if( !chroma_source_supported(CHROMA_ELEM_PATCH, row->hue_src) )
  {
    /* A hue channel patches cannot express renders as neutral grey dimmed
     * by the current envelope: the ramp never gains a second meaning on
     * the same screen. */
    int i;

    for( i = 0; i < data.m; i++ )
      patch_proj_rgb[i] = rgb_linear_scale(PATCH_DIM_RGB,
          x.lum_floor + (1.0 - x.lum_floor) * (double)patch_src.s_env[i]);

    patch_result = patch_proj_rgb;
  }
  else
  {
    patch_row = *row;
    if( !chroma_source_supported(CHROMA_ELEM_PATCH, row->lum_src) )
      patch_row.lum_src = CHAN_CURRENT;

    chroma_project_frame(patch_proj_rgb, &patch_src, &patch_src,
        &patch_row, &x, data.m);
    patch_result = patch_proj_rgb;
  }

  gate_store(&patch_gate, &want);
  return patch_result;
}

/*-----------------------------------------------------------------------*/

  const float *
chroma_proj_frame_wire_widths(int fstep,
    chroma_proj_t proj, color_tone_t fam, chroma_channel_t base_chan)
{
  color_edge_t want = { 0 };
  gboolean have = FALSE;
  int i;

  if( rc_config.color_width_amp != 0 && wire_proj_width != NULL
      && data.n > 0 )
  {
    frame_sources(&chroma_proj_rows[proj], fstep, fam, base_chan);

    if( wire_lum_src.n > 0 )
    {
      have = TRUE;
      want = (color_edge_t){ .flags = FRAME_FLAG_WIDTH,
          .gen_a = wire_lum_src.gen };
    }
  }

  if( gate_hit(&width_gate, &want) )
    return width_result;

  if( have )
  {
    /* Width rides the brightness source's s(n); a floor keeps every
     * segment visible at its null. */
    for( i = 0; i < data.n; i++ )
      wire_proj_width[i] = seg_width[i]
          * fmaxf(wire_lum_src.s_env[i], WIDTH_MIN_RATIO);

    width_result = wire_proj_width;
  }
  else
  {
    /* Carrier off or no data bound: static geometry widths */
    width_result = seg_width;
  }

  gate_store(&width_gate, &want);
  return width_result;
}

/*-----------------------------------------------------------------------*/

  void
chroma_proj_alloc(void)
{
  if( data.n > 0 )
  {
    mem_array_realloc(&wire_proj_rgb, data.n);
    mem_array_realloc(&wire_proj_width, data.n);
    mem_array_realloc(&wire_glyph_flags, data.n);
  }

  if( data.m > 0 )
    mem_array_realloc(&patch_proj_rgb, data.m);

  /* Buffers resized for a new model: drop the prepared sources */
  chroma_source_free(&wire_hue_src);
  chroma_source_free(&wire_lum_src);
  chroma_source_free(&patch_src);
  gate_reset();
}

/*-----------------------------------------------------------------------*/

  void
chroma_proj_free(void)
{
  mem_array_free(&wire_proj_rgb);
  mem_array_free(&wire_proj_width);
  mem_array_free(&wire_glyph_flags);
  mem_array_free(&patch_proj_rgb);

  chroma_source_free(&wire_hue_src);
  chroma_source_free(&wire_lum_src);
  chroma_source_free(&patch_src);
  gate_reset();
}

/*-----------------------------------------------------------------------*/

  uint32_t
chroma_proj_generation(void)
{
  return proj_generation;
}

/*-----------------------------------------------------------------------*/

  gboolean
chroma_proj_animated(int proj)
{
  const chroma_proj_row_t *row;

  if( proj < 0 || proj >= CHROMA_PROJ_NUM )
    return FALSE;

  /* Phase variation rides either carrier: a diverging or cyclic hue, or an
   * instantaneous brightness encoding.  Derived from the row so the class
   * has one source of truth. */
  row = &chroma_proj_rows[proj];
  return (row->hue_enc == HUE_SIGN_DIVERGING)
      || (row->hue_enc == HUE_PHASE_CYCLIC)
      || (row->lum_enc == LUM_ABS_INSTANT)
      || (row->lum_enc == LUM_RAISED_COS);
}

/*-----------------------------------------------------------------------*/
