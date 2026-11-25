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
 * cairo_scenebuffer_flush: Depth-sort and batched-emit pipeline.
 *
 * Each primitive lane is quantized onto one shared depth lattice and sorted
 * independently.  A z-frontier sweep then visits each depth back-to-front,
 * running a stroke pass (segments and stroked arcs, batched by colour and
 * width), a fill pass (filled discs and polygons, batched by colour), and a
 * text pass, so depth dominates overlay and kind breaks equal-depth ties in
 * the order segment, arc, polygon, text.
 */
#include "cairo_scenebuffer.h"
#include "../common.h"
#include "../shared.h"

#include <stdlib.h>
#include <math.h>

/* Use shared float-comparison helpers from common.h (fl_feq, fl_flt, fl_fgt) */

/*-----------------------------------------------------------------------*/

/* Lexicographic ordering over a stroke key (r,g,b,width): the single source of
 * within-depth stroke ordering.  cmp_z_color and cmp_arc_z delegate here so the
 * sort order and the stroke-pass merge stay identical.  Bitwise == on the
 * quantized lattice: see cmp_z_color. */
static int
cmp_stroke_key(float r1, float g1, float b1, float w1,
    float r2, float g2, float b2, float w2)
{
  if( r1 != r2 ) return (r1 < r2) ? -1 : 1;
  if( g1 != g2 ) return (g1 < g2) ? -1 : 1;
  if( b1 != b2 ) return (b1 < b2) ? -1 : 1;
  if( w1 != w2 ) return (w1 < w2) ? -1 : 1;
  return 0;
}

/*-----------------------------------------------------------------------*/

/* Lexicographic ordering over a fill key (r,g,b): the single source of
 * within-depth fill ordering, delegated to by cmp_polygon_z and the fill-pass
 * merge. */
static int
cmp_fill_key(float r1, float g1, float b1, float r2, float g2, float b2)
{
  if( r1 != r2 ) return (r1 < r2) ? -1 : 1;
  if( g1 != g2 ) return (g1 < g2) ? -1 : 1;
  if( b1 != b2 ) return (b1 < b2) ? -1 : 1;
  return 0;
}

/*-----------------------------------------------------------------------*/

/* qsort comparator: ascending quantized z_mid (painter's algorithm), then the
 * shared stroke key so identical-attribute segments cluster within each depth
 * band for longer batch runs.
 *
 * Bitwise == instead of epsilon comparison: qsort requires a strict weak
 * ordering (transitivity).  Epsilon-equality is not transitive and causes
 * undefined behavior in qsort.  After scenebuffer_quantize_depth() and
 * scenebuffer_quantize_colors(), values land on a discrete lattice produced
 * by floorf(); identical quantization inputs yield identical bit patterns. */
static int
cmp_z_color(const void *a, const void *b)
{
  const Segment_t *sa = (const Segment_t *)a;
  const Segment_t *sb_seg = (const Segment_t *)b;

  if( sa->z_mid != sb_seg->z_mid )
    return (sa->z_mid < sb_seg->z_mid) ? -1 : 1;

  return cmp_stroke_key(sa->r, sa->g, sa->b, sa->width,
      sb_seg->r, sb_seg->g, sb_seg->b, sb_seg->width);
}

/*-----------------------------------------------------------------------*/

/* qsort comparator: ascending quantized z_mid, then mode so the stroked
 * partition precedes the filled partition, then (r,g,b,width) for batch runs.
 * Bitwise == on quantized depth/colour: same rationale as cmp_z_color. */
static int
cmp_arc_z(const void *a, const void *b)
{
  const Arc_t *aa = (const Arc_t *)a;
  const Arc_t *ab = (const Arc_t *)b;

  if( aa->z_mid != ab->z_mid )
    return (aa->z_mid < ab->z_mid) ? -1 : 1;
  if( aa->mode != ab->mode )
    return (aa->mode < ab->mode) ? -1 : 1;

  return cmp_stroke_key(aa->r, aa->g, aa->b, aa->width,
      ab->r, ab->g, ab->b, ab->width);
}

/*-----------------------------------------------------------------------*/

/* qsort comparator: ascending quantized z_mid, then (r,g,b) for batch runs.
 * Bitwise == on quantized depth/colour: same rationale as cmp_z_color. */
static int
cmp_polygon_z(const void *a, const void *b)
{
  const Polygon_t *pa = (const Polygon_t *)a;
  const Polygon_t *pb = (const Polygon_t *)b;

  if( pa->z_mid != pb->z_mid )
    return (pa->z_mid < pb->z_mid) ? -1 : 1;

  return cmp_fill_key(pa->r, pa->g, pa->b, pb->r, pb->g, pb->b);
}

/*-----------------------------------------------------------------------*/

/* qsort comparator: ascending quantized z_mid only.  Equal-depth text order
 * is invisible, so no secondary key is needed.  Bitwise == on quantized
 * depth: same rationale as cmp_z_color. */
static int
cmp_text_z(const void *a, const void *b)
{
  const Text_t *ta = (const Text_t *)a;
  const Text_t *tb = (const Text_t *)b;

  if( ta->z_mid != tb->z_mid )
    return (ta->z_mid < tb->z_mid) ? -1 : 1;

  return 0;
}

/*-----------------------------------------------------------------------*/

/* Flush-local applied-state cache: suppresses redundant Cairo/Pango calls so
 * adjacent emissions sharing colour, width, or font reuse the last applied
 * value.  Reset at flush start so stale state from a prior frame is never
 * assumed. */
typedef struct
{
  gboolean has_color;
  rgb_f_t  color;
  gboolean has_width;
  float    width;
  int      font_size;   /* last applied Pango size, -1 = unset */
  int      font_weight; /* last applied Pango weight, -1 = unset */
} scene_paint_state_t;

static void
apply_color(cairo_t *cr, scene_paint_state_t *st, float r, float g, float b)
{
  if( st->has_color && st->color.r == r && st->color.g == g && st->color.b == b )
    return;

  cairo_set_source_rgb(cr, (double)r, (double)g, (double)b);
  st->color.r = r;
  st->color.g = g;
  st->color.b = b;
  st->has_color = TRUE;
}

static void
apply_width(cairo_t *cr, scene_paint_state_t *st, float w)
{
  if( st->has_width && st->width == w )
    return;

  cairo_set_line_width(cr, (double)w);
  st->width = w;
  st->has_width = TRUE;
}

/*-----------------------------------------------------------------------*/

/**
 * scenebuffer_quantize_colors() - Round RGB values to discrete levels
 * @sb:          scenebuffer with deposited primitives
 * @color_quant: quantization levels (0 = disabled)
 *
 * Applied to every stroked and filled colour-bearing lane so the batch keys
 * coincide; text colour is left untouched since text never batches.
 */
static void
scenebuffer_quantize_colors(cairo_scenebuffer_t *sb, int color_quant)
{
  if( color_quant <= 0 )
    return;

  float levels = (float)color_quant;
  int qi;

  for( qi = 0; qi < sb->count; qi++ )
  {
    Segment_t *s = &sb->segs[qi];
    s->r = floorf(s->r * levels + 0.5f) / levels;
    s->g = floorf(s->g * levels + 0.5f) / levels;
    s->b = floorf(s->b * levels + 0.5f) / levels;
  }
  for( qi = 0; qi < sb->arc_count; qi++ )
  {
    Arc_t *a = &sb->arcs[qi];
    a->r = floorf(a->r * levels + 0.5f) / levels;
    a->g = floorf(a->g * levels + 0.5f) / levels;
    a->b = floorf(a->b * levels + 0.5f) / levels;
  }
  for( qi = 0; qi < sb->poly_count; qi++ )
  {
    Polygon_t *p = &sb->polys[qi];
    p->r = floorf(p->r * levels + 0.5f) / levels;
    p->g = floorf(p->g * levels + 0.5f) / levels;
    p->b = floorf(p->b * levels + 0.5f) / levels;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * scenebuffer_quantize_depth() - Bin z_mid values for painter's algorithm
 * @sb:         scenebuffer with deposited primitives
 * @depth_bins: number of z bins (>=1)
 *
 * Folds every populated lane into one depth extent so the four lanes share a
 * single quantization lattice and stay depth-comparable during the frontier
 * sweep.  When all depths are equal (zero range) the early return preserves
 * natural order.
 */
static void
scenebuffer_quantize_depth(cairo_scenebuffer_t *sb, int depth_bins)
{
  float z_min, z_max;
  int   n;

  /* Seed the extent from whichever lane holds the first primitive; the flush
   * guard guarantees at least one lane is non-empty. */
  if( sb->count > 0 )
    z_min = z_max = sb->segs[0].z_mid;
  else if( sb->arc_count > 0 )
    z_min = z_max = sb->arcs[0].z_mid;
  else if( sb->poly_count > 0 )
    z_min = z_max = sb->polys[0].z_mid;
  else
    z_min = z_max = sb->texts[0].z_mid;

  for( n = 0; n < sb->count; n++ )
  {
    if( fl_flt(sb->segs[n].z_mid, z_min) ) z_min = sb->segs[n].z_mid;
    if( fl_fgt(sb->segs[n].z_mid, z_max) ) z_max = sb->segs[n].z_mid;
  }
  for( n = 0; n < sb->arc_count; n++ )
  {
    if( fl_flt(sb->arcs[n].z_mid, z_min) ) z_min = sb->arcs[n].z_mid;
    if( fl_fgt(sb->arcs[n].z_mid, z_max) ) z_max = sb->arcs[n].z_mid;
  }
  for( n = 0; n < sb->poly_count; n++ )
  {
    if( fl_flt(sb->polys[n].z_mid, z_min) ) z_min = sb->polys[n].z_mid;
    if( fl_fgt(sb->polys[n].z_mid, z_max) ) z_max = sb->polys[n].z_mid;
  }
  for( n = 0; n < sb->text_count; n++ )
  {
    if( fl_flt(sb->texts[n].z_mid, z_min) ) z_min = sb->texts[n].z_mid;
    if( fl_fgt(sb->texts[n].z_mid, z_max) ) z_max = sb->texts[n].z_mid;
  }

  float z_range = z_max - z_min;
  if( !fl_fgt(z_range, 0.0f) )
    return;

  float inv_range = 1.0f / z_range;
  float bins_f    = (float)depth_bins;

  for( n = 0; n < sb->count; n++ )
  {
    float norm = (sb->segs[n].z_mid - z_min) * inv_range;
    sb->segs[n].z_mid = z_min + floorf(norm * bins_f) / bins_f * z_range;
  }
  for( n = 0; n < sb->arc_count; n++ )
  {
    float norm = (sb->arcs[n].z_mid - z_min) * inv_range;
    sb->arcs[n].z_mid = z_min + floorf(norm * bins_f) / bins_f * z_range;
  }
  for( n = 0; n < sb->poly_count; n++ )
  {
    float norm = (sb->polys[n].z_mid - z_min) * inv_range;
    sb->polys[n].z_mid = z_min + floorf(norm * bins_f) / bins_f * z_range;
  }
  for( n = 0; n < sb->text_count; n++ )
  {
    float norm = (sb->texts[n].z_mid - z_min) * inv_range;
    sb->texts[n].z_mid = z_min + floorf(norm * bins_f) / bins_f * z_range;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * emit_stroke_pass() - Stroke segments and stroked arcs at one depth
 * @cr:      Cairo context
 * @st:      paint-state cache
 * @segs:    segment lane
 * @s0, @s1: half-open segment range at this depth
 * @arcs:    arc lane
 * @a0, @a1: half-open stroked-arc range at this depth
 *
 * Merges the two sub-ranges (each already ordered by colour then width) by
 * stroke key, accumulating all primitives sharing one (colour, width) into a
 * single path and stroking it once.  Returns the number of cairo_stroke
 * invocations.
 */
static int
emit_stroke_pass(cairo_t *cr, scene_paint_state_t *st,
    const Segment_t *segs, int s0, int s1,
    const Arc_t *arcs, int a0, int a1)
{
  int groups = 0;
  int si = s0, qi = a0;

  while( si < s1 || qi < a1 )
  {
    float kr, kg, kb, kw;
    gboolean use_seg;

    if( si < s1 && qi < a1 )
      use_seg = ( cmp_stroke_key(segs[si].r, segs[si].g, segs[si].b, segs[si].width,
                                 arcs[qi].r, arcs[qi].g, arcs[qi].b, arcs[qi].width) <= 0 );
    else
      use_seg = ( si < s1 );

    if( use_seg )
    {
      kr = segs[si].r; kg = segs[si].g; kb = segs[si].b; kw = segs[si].width;
    }
    else
    {
      kr = arcs[qi].r; kg = arcs[qi].g; kb = arcs[qi].b; kw = arcs[qi].width;
    }

    apply_color(cr, st, kr, kg, kb);
    apply_width(cr, st, kw);
    cairo_new_path(cr);

    while( si < s1 &&
        segs[si].r == kr && segs[si].g == kg &&
        segs[si].b == kb && segs[si].width == kw )
    {
      cairo_move_to(cr, (double)segs[si].x1, (double)segs[si].y1);
      cairo_line_to(cr, (double)segs[si].x2, (double)segs[si].y2);
      si++;
    }

    while( qi < a1 &&
        arcs[qi].r == kr && arcs[qi].g == kg &&
        arcs[qi].b == kb && arcs[qi].width == kw )
    {
      cairo_new_sub_path(cr);
      cairo_arc(cr, arcs[qi].cx, arcs[qi].cy, arcs[qi].radius,
          arcs[qi].a0, arcs[qi].a1);
      qi++;
    }

    cairo_stroke(cr);
    groups++;
  }

  return groups;
}

/*-----------------------------------------------------------------------*/

/**
 * emit_fill_pass() - Fill discs and polygons at one depth
 * @cr:      Cairo context
 * @st:      paint-state cache
 * @arcs:    arc lane
 * @a0, @a1: half-open filled-arc range at this depth
 * @polys:   polygon lane
 * @p0, @p1: half-open polygon range at this depth
 *
 * Merges the two sub-ranges (each ordered by colour) by fill key,
 * accumulating all primitives sharing one colour into a single path and
 * filling it once.  Returns the number of cairo_fill invocations.
 */
static int
emit_fill_pass(cairo_t *cr, scene_paint_state_t *st,
    const Arc_t *arcs, int a0, int a1,
    const Polygon_t *polys, int p0, int p1)
{
  int groups = 0;
  int qi = a0, pi = p0;

  while( qi < a1 || pi < p1 )
  {
    float kr, kg, kb;
    gboolean use_arc;

    if( qi < a1 && pi < p1 )
      use_arc = ( cmp_fill_key(arcs[qi].r, arcs[qi].g, arcs[qi].b,
                               polys[pi].r, polys[pi].g, polys[pi].b) <= 0 );
    else
      use_arc = ( qi < a1 );

    if( use_arc )
    {
      kr = arcs[qi].r; kg = arcs[qi].g; kb = arcs[qi].b;
    }
    else
    {
      kr = polys[pi].r; kg = polys[pi].g; kb = polys[pi].b;
    }

    apply_color(cr, st, kr, kg, kb);
    cairo_new_path(cr);

    while( qi < a1 && arcs[qi].r == kr && arcs[qi].g == kg && arcs[qi].b == kb )
    {
      cairo_new_sub_path(cr);
      cairo_arc(cr, arcs[qi].cx, arcs[qi].cy, arcs[qi].radius,
          arcs[qi].a0, arcs[qi].a1);
      qi++;
    }

    while( pi < p1 && polys[pi].r == kr && polys[pi].g == kg && polys[pi].b == kb )
    {
      const Polygon_t *p = &polys[pi];
      int v;

      cairo_move_to(cr, (double)p->pts[0].x, (double)p->pts[0].y);
      for( v = 1; v < p->n; v++ )
        cairo_line_to(cr, (double)p->pts[v].x, (double)p->pts[v].y);
      cairo_close_path(cr);
      pi++;
    }

    cairo_fill(cr);
    groups++;
  }

  return groups;
}

/*-----------------------------------------------------------------------*/

/**
 * emit_text_pass() - Render text runs at one depth
 * @cr:          Cairo context
 * @st:          paint-state cache
 * @layout:      base Pango layout (font source)
 * @work_desc:   reusable font description scratch, sized/weighted per run
 * @base_size:   base glyph size in Pango units
 * @base_abs:    whether base_size is an absolute (device) size
 * @base_weight: base font weight applied when a run is not bold
 * @texts:       text lane
 * @t0, @t1:     half-open text range at this depth
 *
 * Each run sets the font only when its (size, weight) differs from the last
 * applied, then justifies against its anchor and shows the layout.
 */
static void
emit_text_pass(cairo_t *cr, scene_paint_state_t *st,
    PangoLayout *layout, PangoFontDescription *work_desc,
    int base_size, gboolean base_abs, PangoWeight base_weight,
    const Text_t *texts, int t0, int t1)
{
  int ti;

  for( ti = t0; ti < t1; ti++ )
  {
    const Text_t *t = &texts[ti];
    int x = t->x, y = t->y, w, h;
    int size = (int)( (float)base_size * t->scale + 0.5f );
    PangoWeight weight = ( t->justify & JUSTIFY_BOLD )
        ? PANGO_WEIGHT_BOLD : base_weight;

    apply_color(cr, st, t->r, t->g, t->b);

    if( size != st->font_size || (int)weight != st->font_weight )
    {
      if( base_abs )
        pango_font_description_set_absolute_size(work_desc, (double)size);
      else
        pango_font_description_set_size(work_desc, size);
      pango_font_description_set_weight(work_desc, weight);
      pango_layout_set_font_description(layout, work_desc);
      st->font_size   = size;
      st->font_weight = (int)weight;
    }

    pango_layout_set_text(layout, t->text, -1);
    pango_layout_get_pixel_size(layout, &w, &h);

    switch( t->justify & JUSTIFY_HMASK )
    {
      case JUSTIFY_RIGHT:  x -= w;     break;
      case JUSTIFY_CENTER: x -= w / 2; break;
      case JUSTIFY_LEFT:
      default:                         break;
    }

    switch( t->justify & JUSTIFY_VMASK )
    {
      case JUSTIFY_ABOVE:  y -= h;     break;
      case JUSTIFY_MIDDLE: y -= h / 2; break;
      case JUSTIFY_BELOW:
      default:                         break;
    }

    cairo_move_to(cr, (double)x, (double)y);
    pango_cairo_show_layout(cr, layout);
  }
}

/*-----------------------------------------------------------------------*/

/**
 * scenebuffer_flush() - Sort and emit all accumulated primitives depth-first
 * @sb:    scenebuffer to flush
 * @cr:    Cairo context to draw into
 * @stats: out-param populated with flush metrics; NULL to skip
 *
 * Reads rc_config.cairo_depth_bins and rc_config.cairo_color_quant directly.
 */
  void
scenebuffer_flush(cairo_scenebuffer_t *sb, cairo_t *cr,
    cairo_flush_stats_t *stats)
{
  int depth_bins  = rc_config.cairo_depth_bins;
  int color_quant = rc_config.cairo_color_quant;

  if( depth_bins < 1 )
  {
    BUG("depth_bins=%d\n", depth_bins);
    return;
  }

  if( stats != NULL )
  {
    stats->segments     = sb->count;
    stats->batch_groups = 0;
    stats->sort_us      = 0;
    stats->stroke_us    = 0;
    stats->capacity     = mem_array_capacity(sb->segs);
  }

  if( sb->count == 0 && sb->arc_count == 0 &&
      sb->poly_count == 0 && sb->text_count == 0 )
    return;

  scenebuffer_quantize_colors(sb, color_quant);
  scenebuffer_quantize_depth(sb, depth_bins);

  gint64 t_sort_start = 0;
  if( stats != NULL )
    t_sort_start = g_get_monotonic_time();

  if( sb->count > 0 )
    qsort(sb->segs, (size_t)sb->count, sizeof(Segment_t), cmp_z_color);
  if( sb->arc_count > 0 )
    qsort(sb->arcs, (size_t)sb->arc_count, sizeof(Arc_t), cmp_arc_z);
  if( sb->poly_count > 0 )
    qsort(sb->polys, (size_t)sb->poly_count, sizeof(Polygon_t), cmp_polygon_z);
  if( sb->text_count > 0 )
    qsort(sb->texts, (size_t)sb->text_count, sizeof(Text_t), cmp_text_z);

  gint64 t_stroke_start = 0;
  if( stats != NULL )
  {
    stats->sort_us = g_get_monotonic_time() - t_sort_start;
    t_stroke_start = g_get_monotonic_time();
  }

  scene_paint_state_t st = {
    .has_color = FALSE, .has_width = FALSE,
    .font_size = -1, .font_weight = -1
  };

  /* Derive the text font once from the base layout; left NULL (text pass
   * skipped) when no run is present or the layout exposes no base font. */
  PangoFontDescription *work_desc = NULL;
  int        base_size   = 0;
  gboolean   base_abs    = FALSE;
  PangoWeight base_weight = PANGO_WEIGHT_NORMAL;

  if( sb->text_count > 0 && sb->text_layout != NULL )
  {
    PangoContext *pctx = pango_layout_get_context(sb->text_layout);
    const PangoFontDescription *base_desc =
        pango_context_get_font_description(pctx);

    if( base_desc != NULL )
    {
      work_desc   = pango_font_description_copy(base_desc);
      base_size   = pango_font_description_get_size(base_desc);
      base_abs    = pango_font_description_get_size_is_absolute(base_desc);
      base_weight = pango_font_description_get_weight(base_desc);
    }
    else
      BUG("layout has no base font description\n");
  }

  int groups = 0;
  int si = 0, ai = 0, pi = 0, ti = 0;

  while( si < sb->count || ai < sb->arc_count ||
         pi < sb->poly_count || ti < sb->text_count )
  {
    /* Frontier depth: smallest head among the in-range lane cursors. */
    float zstar = INFINITY;
    if( si < sb->count      && sb->segs[si].z_mid  < zstar ) zstar = sb->segs[si].z_mid;
    if( ai < sb->arc_count  && sb->arcs[ai].z_mid  < zstar ) zstar = sb->arcs[ai].z_mid;
    if( pi < sb->poly_count && sb->polys[pi].z_mid < zstar ) zstar = sb->polys[pi].z_mid;
    if( ti < sb->text_count && sb->texts[ti].z_mid < zstar ) zstar = sb->texts[ti].z_mid;

    /* Half-open ranges of each lane at this depth.  Quantized z_mid lands on
     * an exact lattice, so zstar (a stored value) compares bitwise. */
    int seg_end = si;
    while( seg_end < sb->count && sb->segs[seg_end].z_mid == zstar )
      seg_end++;

    int arc_end = ai;
    while( arc_end < sb->arc_count && sb->arcs[arc_end].z_mid == zstar )
      arc_end++;

    int arc_stroke_end = ai;
    while( arc_stroke_end < arc_end && sb->arcs[arc_stroke_end].mode == SCENE_STROKE )
      arc_stroke_end++;

    int poly_end = pi;
    while( poly_end < sb->poly_count && sb->polys[poly_end].z_mid == zstar )
      poly_end++;

    int text_end = ti;
    while( text_end < sb->text_count && sb->texts[text_end].z_mid == zstar )
      text_end++;

    groups += emit_stroke_pass(cr, &st, sb->segs, si, seg_end,
        sb->arcs, ai, arc_stroke_end);

    groups += emit_fill_pass(cr, &st, sb->arcs, arc_stroke_end, arc_end,
        sb->polys, pi, poly_end);

    if( work_desc != NULL )
      emit_text_pass(cr, &st, sb->text_layout, work_desc,
          base_size, base_abs, base_weight, sb->texts, ti, text_end);

    si = seg_end;
    ai = arc_end;
    pi = poly_end;
    ti = text_end;
  }

  if( work_desc != NULL )
    pango_font_description_free(work_desc);

  if( stats != NULL )
  {
    stats->batch_groups = groups;
    stats->stroke_us    = g_get_monotonic_time() - t_stroke_start;
  }
}
