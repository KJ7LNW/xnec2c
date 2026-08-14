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
 * freqplots_render: Primitive accumulator for the frequency-plots Cairo
 * drawing area.
 *
 * Leaf plot renderers deposit line, arc, polygon, and text primitives via the
 * fp_add_* helpers during a frame.  fp_render_flush() hands the surface's
 * cairo_scenebuffer to the depth-sorted painter's pipeline, which emits every
 * primitive back-to-front; text deposits at FP_Z_TEXT, above the grid but
 * below the data traces, so labels never hide the measurement geometry.
 */

#include "freqplots_render.h"
#include "../cairo/cairo_draw.h"
#include "../shared.h"

#include <math.h>

/*-----------------------------------------------------------------------*/

/**
 * fp_render_reset() - Begin a new frame
 * @fp: render handle to bind to this frame's resources
 * @surface: plot surface owning the retained scenebuffer
 * @cr: active Cairo context for this frame
 * @layout: caller-owned base font layout for this frame's text
 *
 * Binds @layout to the scenebuffer so text primitives have a base font; the
 * view owns the layout's lifetime, so this neither creates nor frees it.
 * Returns FALSE when a required frame resource is missing.
 */
  gboolean
fp_render_reset(fp_render_t *fp, render_surface_t *surface, cairo_t *cr,
    PangoLayout *layout)
{
  if( fp == NULL || surface == NULL || cr == NULL || layout == NULL )
    return FALSE;

  fp->cr = cr;
  fp->sb = &cairo_engine_surface(surface)->scenebuffer;

  scenebuffer_reset(fp->sb);
  scenebuffer_set_text_layout(fp->sb, layout);

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/* Append one line segment carrying the stroke style to the scenebuffer. */
  static void
fp_add_seg(fp_render_t *fp, fp_stroke_t s, int x1, int y1, int x2, int y2)
{
  Segment_t seg;

  seg.x1 = x1;
  seg.y1 = y1;
  seg.x2 = x2;
  seg.y2 = y2;
  seg.z_mid = s.z_mid;
  seg.width = s.width;
  seg_set_color(&seg, s.color);

  scenebuffer_add(fp->sb, &seg);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_line(fp_render_t *fp, int x1, int y1, int x2, int y2, fp_stroke_t s)
{
  fp_add_seg(fp, s, x1, y1, x2, y2);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_polyline(fp_render_t *fp, const GdkPoint *pts, int n, fp_stroke_t s)
{
  int i;

  for( i = 1; i < n; i++ )
    fp_add_seg(fp, s, pts[i-1].x, pts[i-1].y, pts[i].x, pts[i].y);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_quad(fp_render_t *fp, const GdkPoint pts[4], fp_stroke_t s)
{
  Segment_t tmpl;
  int xs[4], ys[4], k;

  tmpl.z_mid = s.z_mid;
  tmpl.width = s.width;
  seg_set_color(&tmpl, s.color);

  for( k = 0; k < 4; k++ )
  {
    xs[k] = pts[k].x;
    ys[k] = pts[k].y;
  }

  scenebuffer_add_polygon_outline(fp->sb, &tmpl, xs, ys);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_rect(fp_render_t *fp, int x, int y, int w, int h, fp_stroke_t s)
{
  GdkPoint p[4];

  p[0].x = x;     p[0].y = y;
  p[1].x = x + w; p[1].y = y;
  p[2].x = x + w; p[2].y = y + h;
  p[3].x = x;     p[3].y = y + h;

  fp_add_quad(fp, p, s);
}

/*-----------------------------------------------------------------------*/

/**
 * fp_add_arc() - Deposit one stroked circular arc
 * @cx, @cy: arc centre
 * @r:       arc radius
 * @a0, @a1: start and end angle in radians
 * @s:       stroke style
 */
  void
fp_add_arc(fp_render_t *fp, double cx, double cy, double r,
    double a0, double a1, fp_stroke_t s)
{
  Arc_t a;

  a.cx     = cx;
  a.cy     = cy;
  a.radius = r;
  a.a0     = a0;
  a.a1     = a1;
  a.z_mid  = s.z_mid;
  a.r      = s.color.r;
  a.g      = s.color.g;
  a.b      = s.color.b;
  a.width  = s.width;
  a.mode   = SCENE_STROKE;

  scenebuffer_add_arc(fp->sb, &a);
}

/*-----------------------------------------------------------------------*/

/* Deposit one filled convex polygon from four screen-space vertices. */
  static void
fp_add_filled_poly(fp_render_t *fp, float z, rgb_f_t c, const GdkPoint pts[4])
{
  Polygon_t p;
  int k;

  for( k = 0; k < 4; k++ )
    p.pts[k] = pts[k];
  p.n     = 4;
  p.z_mid = z;
  p.r     = c.r;
  p.g     = c.g;
  p.b     = c.b;

  scenebuffer_add_polygon(fp->sb, &p);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_filled_square(fp_render_t *fp, int cx, int cy, int size,
    float z, rgb_f_t c)
{
  int x0 = cx - size / 2;
  int y0 = cy - size / 2;
  GdkPoint pts[4] = {
    { x0,        y0        },
    { x0 + size, y0        },
    { x0 + size, y0 + size },
    { x0,        y0 + size }
  };

  fp_add_filled_poly(fp, z, c, pts);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_filled_diamond(fp_render_t *fp, int cx, int cy, int size,
    float z, rgb_f_t c)
{
  int half = size / 2;
  /* The one-pixel vertical extension keeps the diamond visually balanced
   * against the integer-rounded square. */
  GdkPoint pts[4] = {
    { cx - half, cy            },
    { cx,        cy + half + 1 },
    { cx + half, cy            },
    { cx,        cy - half - 1 }
  };

  fp_add_filled_poly(fp, z, c, pts);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_filled_circle(fp_render_t *fp, int cx, int cy, int radius,
    float z, rgb_f_t c)
{
  Arc_t a;

  a.cx     = (double)cx;
  a.cy     = (double)cy;
  a.radius = (double)radius;
  a.a0     = 0.0;
  a.a1     = M_2PI;
  a.z_mid  = z;
  a.r      = c.r;
  a.g      = c.g;
  a.b      = c.b;
  a.width  = 0.0f;
  a.mode   = SCENE_FILL;

  scenebuffer_add_arc(fp->sb, &a);
}

/*-----------------------------------------------------------------------*/

  void
fp_add_text(fp_render_t *fp, int x, int y, float scale,
    const char *text, int justify, rgb_f_t c)
{
  Text_t t;

  t.x       = x;
  t.y       = y;
  t.z_mid   = FP_Z_TEXT;
  t.r       = c.r;
  t.g       = c.g;
  t.b       = c.b;
  t.scale   = scale;
  t.justify = justify;
  g_strlcpy(t.text, text, SCENE_TEXT_LEN);

  scenebuffer_add_text(fp->sb, &t);
}

/*-----------------------------------------------------------------------*/

/**
 * fp_render_flush() - Emit every accumulated primitive depth-sorted
 * @fp: render handle bound to the active Cairo context
 */
  void
fp_render_flush(fp_render_t *fp)
{
  scenebuffer_flush(fp->sb, fp->cr, NULL);
}

/*-----------------------------------------------------------------------*/

/**
 * pango_text_size() - Measure a string in the rendering canvas font
 * @canvas: canvas whose presented surface supplies the text metric source
 */
  void
pango_text_size(canvas_id_t canvas, int *width, int *height, char *s)
{
  PangoLayout *layout;

  layout = canvas_pango_layout(canvas, s);
  pango_layout_get_pixel_size(layout, width, height);
  g_object_unref(layout);
}
