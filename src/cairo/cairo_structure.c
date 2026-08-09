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
 * cairo_structure: Cairo renderer for wire segments and surface patches.
 *
 * draw_wire_segments and draw_surface_patches receive pre-selected colors
 * via struct_draw_params_t.  Zero flag evaluation for content selection;
 * dispatch decides currents vs charges vs geometry mode.
 */
#include "cairo_draw.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"
#include "../prerender/prerender_patch_arrow.h"
#include "../prerender/prerender_state.h"
#include "../prerender/prerender_color.h"
#include "../rdpattern_ui.h"
#include "../chroma/chroma_glyph.h"

/* Node/antinode tick geometry in screen pixels: a floor plus a multiple of
 * the wire width so a thick wire never occludes the mark. */
#define GLYPH_TICK_MIN_PX    6.0f
#define GLYPH_TICK_WIDTH_K   2.5f
#define GLYPH_TICK_LINE_PX   1.5f

/**
 * patch_z_mid() - Average z_mid across four patch-corner segments
 * @segm: segment array
 * @base: index of first corner segment (stride of 4)
 */
static inline float
patch_z_mid(const Segment_t *segm, int base)
{
  return 0.25f * (segm[base].z_mid   + segm[base+1].z_mid
                + segm[base+2].z_mid + segm[base+3].z_mid);
}

/**
 * seg_pair_z_mid() - Midpoint depth of two segments
 * @segm: segment array
 * @i1:   first segment index
 * @i2:   second segment index
 */
static inline float
seg_pair_z_mid(const Segment_t *segm, int i1, int i2)
{
  return 0.5f * (segm[i1].z_mid + segm[i2].z_mid);
}

/* Cairo-local world-space arrow segment (3D endpoints, one edge per arrow line) */
typedef struct
{
  double x1, y1, z1;
  double x2, y2, z2;
} arrow_seg_3d_t;


/** patch_arrow_to_world() - Rotate arrow template and map to world coordinates
 * @idx:   patch index (0-based into data.m)
 * @fd:    precomputed flow data {Re(ct1),Im(ct1),Re(ct2),Im(ct2)} / cmax
 * @phase: current animation phase (radians)
 * @segs:  output array of ARROW_LINE_COUNT world-space segments
 *
 * Reads center and tangent axes from geom_pre.patch_tangent_frame[idx].
 * Computes instantaneous flow direction at phase, rotates arrow template,
 * maps each UV vertex to world via:
 *   world = center + 2*(rot_u * st1 + rot_v * st2)
 */
static void
patch_arrow_to_world(int idx, const float fd[4], float phase,
    arrow_seg_3d_t segs[ARROW_LINE_COUNT])
{
  const patch_tangent_frame_t *tf = &geom_pre.patch_tangent_frame[idx];
  double cp = cos((double)phase);
  double sp = sin((double)phase);
  double re1 = (double)fd[0] * cp - (double)fd[1] * sp;
  double re2 = (double)fd[2] * cp - (double)fd[3] * sp;
  double angle = atan2(re2, re1);
  double ca = cos(angle);
  double sa = sin(angle);

  int k;
  for( k = 0; k < ARROW_LINE_COUNT; k++ )
  {
    double u0 = (double)arrow_template[k].u1 - 0.5;
    double v0 = (double)arrow_template[k].v1 - 0.5;
    double ru0 = u0 * ca - v0 * sa;
    double rv0 = u0 * sa + v0 * ca;

    double u1 = (double)arrow_template[k].u2 - 0.5;
    double v1 = (double)arrow_template[k].v2 - 0.5;
    double ru1 = u1 * ca - v1 * sa;
    double rv1 = u1 * sa + v1 * ca;

    segs[k].x1 = tf->cx + 2.0 * (ru0 * tf->st1x + rv0 * tf->st2x);
    segs[k].y1 = tf->cy + 2.0 * (ru0 * tf->st1y + rv0 * tf->st2y);
    segs[k].z1 = tf->cz + 2.0 * (ru0 * tf->st1z + rv0 * tf->st2z);
    segs[k].x2 = tf->cx + 2.0 * (ru1 * tf->st1x + rv1 * tf->st2x);
    segs[k].y2 = tf->cy + 2.0 * (ru1 * tf->st1y + rv1 * tf->st2y);
    segs[k].z2 = tf->cz + 2.0 * (ru1 * tf->st1z + rv1 * tf->st2z);
  }
}

/*-----------------------------------------------------------------------*/

/**
 * draw_wire_segments() - Draw all wire segments with dispatch-selected colors
 * @v:      view for segment coordinates
 * @segm:   projected segment array [nseg]
 * @nseg:   number of segments
 * @params: dispatch-resolved draw parameters (colors, cmax, show_flow)
 *
 * Single pass with precomputed per-segment colors from dispatch.
 * seg_rgb encodes type color (blue/yellow/red) in geometry mode.
 */
  static void
draw_wire_segments(cairo_scenebuffer_t *sb, view_t *v, Segment_t *segm, gint nseg,
    const struct_draw_params_t *params)
{
  if( !nseg )
    return;

  if( params->wire_widths == NULL )
    BUG("draw_wire_segments: wire_widths is NULL\n");

  int idx;

  /* Draw networks */
  for( idx = 0; idx < netcx.nonet; idx++ )
  {
    int i1, i2;

    i1 = netcx.iseg1[idx] - 1;
    i2 = netcx.iseg2[idx] - 1;

    switch( netcx.ntyp[idx] )
    {
      case 1: /* Two-port network: outline polygon + connecting lines */
        {
          int xs[4], ys[4];
          float zn_mid = seg_pair_z_mid(segm, i1, i2);

          xs[0] = segm[i1].x1 + (segm[i2].x1 - segm[i1].x1) / 3;
          ys[0] = segm[i1].y1 + (segm[i2].y1 - segm[i1].y1) / 3;
          xs[1] = segm[i2].x1 + (segm[i1].x1 - segm[i2].x1) / 3;
          ys[1] = segm[i2].y1 + (segm[i1].y1 - segm[i2].y1) / 3;
          xs[2] = segm[i2].x2 + (segm[i1].x2 - segm[i2].x2) / 3;
          ys[2] = segm[i2].y2 + (segm[i1].y2 - segm[i2].y2) / 3;
          xs[3] = segm[i1].x2 + (segm[i2].x2 - segm[i1].x2) / 3;
          ys[3] = segm[i1].y2 + (segm[i2].y2 - segm[i1].y2) / 3;

          Segment_t net_tmpl;
          net_tmpl.z_mid = zn_mid;
          seg_set_color(&net_tmpl, COLOR_MAGENTA);
          net_tmpl.width = 2.0f;
          scenebuffer_add_polygon_outline(sb, &net_tmpl, xs, ys);

          net_tmpl.x1 = segm[i1].x1; net_tmpl.y1 = segm[i1].y1;
          net_tmpl.x2 = segm[i2].x1; net_tmpl.y2 = segm[i2].y1;
          scenebuffer_add(sb, &net_tmpl);
          net_tmpl.x1 = segm[i1].x2; net_tmpl.y1 = segm[i1].y2;
          net_tmpl.x2 = segm[i2].x2; net_tmpl.y2 = segm[i2].y2;
          scenebuffer_add(sb, &net_tmpl);
        }
        break;

      case 2: /* Straight transmission line */
        {
          Segment_t tl;
          tl.z_mid = seg_pair_z_mid(segm, i1, i2);
          seg_set_color(&tl, COLOR_CYAN);
          tl.width = 2.0f;
          tl.x1 = segm[i1].x1; tl.y1 = segm[i1].y1;
          tl.x2 = segm[i2].x1; tl.y2 = segm[i2].y1;
          scenebuffer_add(sb, &tl);
          tl.x1 = segm[i1].x2; tl.y1 = segm[i1].y2;
          tl.x2 = segm[i2].x2; tl.y2 = segm[i2].y2;
          scenebuffer_add(sb, &tl);
        }
        break;

      case 3: /* Crossed transmission line */
        {
          Segment_t tl;
          tl.z_mid = seg_pair_z_mid(segm, i1, i2);
          seg_set_color(&tl, COLOR_CYAN);
          tl.width = 2.0f;
          tl.x1 = segm[i1].x1; tl.y1 = segm[i1].y1;
          tl.x2 = segm[i2].x2; tl.y2 = segm[i2].y2;
          scenebuffer_add(sb, &tl);
          tl.x1 = segm[i1].x2; tl.y1 = segm[i1].y2;
          tl.x2 = segm[i2].x1; tl.y2 = segm[i2].y1;
          scenebuffer_add(sb, &tl);
        }
        break;

      default:
        BUG("draw_wire_segments: unknown ntyp=%d\n", netcx.ntyp[idx]);

    } /* switch( netcx.ntyp ) */

  } /* for( idx = 0; idx < netcx.nonet ) */

  /* Per-segment precomputed colors and widths deposited into scenebuffer */
  for( idx = 0; idx < nseg; idx++ )
  {
    seg_set_color(&segm[idx], params->wire_colors[idx]);
    segm[idx].width = (float)params->wire_widths[idx];
    scenebuffer_add(sb, &segm[idx]);

    /* Overlay node/antinode tick marks through the screen basis, scaled
     * above the wire pixel width so a thick wire never occludes them. */
    if( params->wire_glyphs != NULL && params->wire_glyphs[idx] != GLYPH_NONE )
    {
      const glyph_shape_t *gs = &glyph_shapes[params->wire_glyphs[idx]];
      float cx  = 0.5f * (float)(segm[idx].x1 + segm[idx].x2);
      float cy  = 0.5f * (float)(segm[idx].y1 + segm[idx].y2);
      float ext = fmaxf(GLYPH_TICK_MIN_PX,
          segm[idx].width * GLYPH_TICK_WIDTH_K);
      int k;

      for( k = 0; k < gs->n_strokes; k++ )
      {
        const glyph_stroke_t *st = &gs->strokes[k];
        Segment_t tick = segm[idx];   /* inherit z_mid for depth sorting */

        seg_set_color(&tick, gs->color);
        tick.width = GLYPH_TICK_LINE_PX;
        tick.x1 = (gint)(cx + st->ax * ext);
        tick.y1 = (gint)(cy + st->ay * ext);
        tick.x2 = (gint)(cx + st->bx * ext);
        tick.y2 = (gint)(cy + st->by * ext);
        scenebuffer_add(sb, &tick);
      }
    }
  }

} /* draw_wire_segments() */

/*-----------------------------------------------------------------------*/

/**
 * draw_surface_patches() - Draw patch segments with dispatch-selected colors
 * @v:      view for segment coordinates
 * @segm:   projected patch segment array [npatch*2]
 * @npatch: number of patches (four rectangle edges each)
 * @params: dispatch-resolved draw parameters (colors)
 *
 * When params->cmax > 0 (current mode): draws each patch rectangle
 * in precomputed per-patch colors from struct_colors.
 * When params->cmax == 0 (geometry mode): draws all patches in the
 * base color from params->patch_colors[0].
 */
  static void
draw_surface_patches(cairo_scenebuffer_t *sb, view_t *v, double scale, Segment_t *segm,
    gint npatch, const struct_draw_params_t *params)
{
  if( !npatch )
    return;

  int idx;

  /* Current mode: per-patch precomputed colors */
  if( params->cmax > 0.0 )
  {
    for( idx = 0; idx < npatch; idx++ )
    {
      int base = 4 * idx;
      int k;
      float fd[4];
      float mag_ratio;
      float pz = patch_z_mid(segm, base);

      /* 4 rectangle edges per patch deposited into scenebuffer */
      for( k = 0; k < 4; k++ )
      {
        segm[base + k].z_mid = pz;
        seg_set_color(&segm[base + k], params->patch_colors[idx]);
        segm[base + k].width = 1.0f;
        scenebuffer_add(sb, &segm[base + k]);
      }

      /* Deposit flow arrow segments when current data is above threshold */
      if( params->show_flow )
      {
        int fstep = params->fstep;
        get_precomputed_flow_data(fstep, idx, fd);
        mag_ratio = (float)sqrt(
            fd[0] * fd[0] + fd[1] * fd[1] +
            fd[2] * fd[2] + fd[3] * fd[3]);

        if( mag_ratio > FLOW_MAG_THRESHOLD )
        {
          arrow_seg_3d_t arrow[ARROW_LINE_COUNT];
          patch_arrow_to_world(idx, fd, flow_phase, arrow);

          for( k = 0; k < ARROW_LINE_COUNT; k++ )
          {
            Segment_t s;
            Set_Gdk_Segment(&s, v, scale,
                arrow[k].x1, arrow[k].y1, arrow[k].z1,
                arrow[k].x2, arrow[k].y2, arrow[k].z2,
                &s.z_mid);
            seg_set_color(&s, params->patch_colors[idx]);
            s.width = 1.0f;
            scenebuffer_add(sb, &s);
          }
        }
      }
    }
    return;
  }

  /* Geometry mode: uniform base color for all patch edges */
  {
    int k;
    for( idx = 0; idx < npatch; idx++ )
    {
      int   base = 4 * idx;
      float pz   = patch_z_mid(segm, base);
      for( k = 0; k < 4; k++ )
      {
        int e = base + k;
        segm[e].z_mid = pz;
        seg_set_color(&segm[e], params->patch_colors[0]);
        segm[e].width = 1.0f;
        scenebuffer_add(sb, &segm[e]);
      }
    }
  }

} /* draw_surface_patches() */

/*-----------------------------------------------------------------------*/

/**
 * cairo_draw_structure() - Cairo renderer entry for structure window
 * @ctx:    cairo_render_ctx_t* with cr and structure_view
 * @params: dispatch-resolved draw parameters
 *
 * Projects geometry, draws axes, segments, and patches.
 * Returns TRUE.
 */
  gboolean
cairo_draw_structure(void *ctx, float extent,
    const struct_draw_params_t *params)
{
  cairo_render_ctx_t *cc = (cairo_render_ctx_t *)ctx;
  view_t *v = cc->view;
  double scale = view_projection_scale(v, extent, v->zoom);

  /* Project geometry to screen coordinates */
  Process_Wire_Segments(v, scale);
  Process_Surface_Patches(v, scale);

  /* Deposit patches below wires (painter's order handled by scenebuffer z_mid) */
  draw_surface_patches(cc->sb, v, scale, structure_segs + data.n, data.m, params);
  draw_wire_segments(cc->sb, v, structure_segs, data.n, params);

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/**
 * cairo_set_status() - Store status message for deferred rendering
 * @ctx: cairo_render_ctx_t*
 * @msg: UTF-8 status message (painted by render_cairo after flush)
 */
  void
cairo_set_status(void *ctx, const char *msg)
{
  cairo_render_ctx_t *cc = (cairo_render_ctx_t *)ctx;
  cc->status_message = msg;
}

/*-----------------------------------------------------------------------*/

/**
 * cairo_set_gradient() - Store pre-resolved gradient legend surface
 * @ctx:     cairo_render_ctx_t*
 * @result:  gradient legend result from gradient_cache; version unused by cairo
 */
  void
cairo_set_gradient(void *ctx, const gradient_result_t *result)
{
  cairo_render_ctx_t *cc = (cairo_render_ctx_t *)ctx;
  cc->gradient = result->surface;
}

/*-----------------------------------------------------------------------*/
