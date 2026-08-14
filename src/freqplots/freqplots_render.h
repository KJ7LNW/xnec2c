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

#ifndef FREQPLOTS_RENDER_H
#define FREQPLOTS_RENDER_H  1

#include "../common.h"
#include "../cairo/cairo_scenebuffer.h"
#include "../render/render_surface.h"

/* Painter's-algorithm depth layers for the frequency plots.  Ascending
 * z_mid is drawn back-to-front by scenebuffer_flush(), so a larger value
 * paints on top.  Measurement data rides above plot infrastructure: the grid
 * sits furthest back, then text labels, then the right (cyan) and left
 * (magenta) traces, with the current-frequency marker on top so the selected
 * measurement is never hidden by a label or a grid line.  Depths 1.0 - 3.0
 * are reserved for graph-specific grid sublayers (see smith_graph.c). */
#define FP_Z_GRID   0.0f
#define FP_Z_TEXT   4.0f
#define FP_Z_RIGHT  5.0f
#define FP_Z_LEFT   6.0f
#define FP_Z_GREEN  7.0f

/* Cohesive stroke style passed to the line/arc deposit helpers; one object
 * instead of three loose colour, width, and depth scalars. */
typedef struct
{
  rgb_f_t color;
  float   width;
  float   z_mid;
} fp_stroke_t;

/* Per-frame render handle carrying the active Cairo context and the retained
 * scenebuffer of the surface producing the frame. */
typedef struct
{
  cairo_t *cr;
  cairo_scenebuffer_t *sb;
} fp_render_t;

/* Lifecycle: reset binds the frame context, the caller-owned text layout, and
 * the surface scenebuffer it clears; flush depth-sorts and emits every
 * primitive.  The canvas releases the scenebuffer with its surface. */
gboolean fp_render_reset(fp_render_t *fp, render_surface_t *surface,
    cairo_t *cr, PangoLayout *layout);
void fp_render_flush(fp_render_t *fp);

/* Stroked geometry deposit helpers.  The stroke style carries colour, width,
 * and depth; each helper appends line or arc geometry to the scenebuffer. */
void fp_add_line(fp_render_t *fp, int x1, int y1, int x2, int y2,
    fp_stroke_t s);
void fp_add_polyline(fp_render_t *fp, const GdkPoint *pts, int n,
    fp_stroke_t s);
void fp_add_rect(fp_render_t *fp, int x, int y, int w, int h,
    fp_stroke_t s);
void fp_add_quad(fp_render_t *fp, const GdkPoint pts[4],
    fp_stroke_t s);
void fp_add_arc(fp_render_t *fp, double cx, double cy, double r,
    double a0, double a1, fp_stroke_t s);

/* Opaque filled point markers.  Squares and diamonds deposit a filled
 * polygon; circles deposit a filled disc.  size is the full square or
 * diamond extent; radius is the circle radius, in pixels. */
void fp_add_filled_square(fp_render_t *fp, int cx, int cy, int size,
    float z, rgb_f_t c);
void fp_add_filled_diamond(fp_render_t *fp, int cx, int cy, int size,
    float z, rgb_f_t c);
void fp_add_filled_circle(fp_render_t *fp, int cx, int cy, int radius,
    float z, rgb_f_t c);

/* Deposit one text run; scale multiplies the base font glyph size for this
 * run.  Painted during flush in depth order, overlaying same-depth geometry. */
void fp_add_text(fp_render_t *fp, int x, int y, float scale,
    const char *text, int justify, rgb_f_t c);

/* Pixel size of a string laid out in the font of a plot canvas. */
void pango_text_size(canvas_id_t canvas, int *width, int *height, char *s);

#endif /* FREQPLOTS_RENDER_H */
