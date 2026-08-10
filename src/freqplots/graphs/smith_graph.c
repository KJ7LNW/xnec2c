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
 * smith_graph: Smith-chart frequency plot type.
 *
 * The reactance/resistance background arcs are sampled into grid-layer
 * segments, the impedance locus and its markers deposit on the magenta
 * layer, and the current-frequency marker on the top layer.
 */

#include "../freqplots_internal.h"
#include "../freqplots_locus.h"
#include "../../shared.h"

#include <math.h>

/* Adjacent locus step indices spanning a frequency, with the
 * interpolation fraction between them. */
typedef struct {
  int    lo;
  int    hi;
  double frac;
} fp_locus_bracket_t;

  static void
Calculate_Smith( double zr, double zi, double z0, double *re, double *im )
{
  complex double z = zr / z0 + I * zi / z0;
  complex double r = ( z - 1.0 ) / ( z + 1.0 );
  *re = creal( r );
  *im = cimag( r );
}

/*-----------------------------------------------------------------------*/

/* fp_locus_bracket()
 *
 * Resolves the two adjacent locus step indices spanning fmhz in the
 * per-step frequency axis, with the interpolation fraction between them.
 * Clamps to an endpoint outside the range; falls back to the nearest
 * index for a non-monotonic axis (e.g. overlapping FR cards).
 */
  static fp_locus_bracket_t
fp_locus_bracket( const double *freq, int nc, double fmhz )
{
  fp_locus_bracket_t bracket = { 0, 0, 0.0 };
  int    idx;
  int    nearest;
  double best_diff;

  if( nc <= 1 )
    return bracket;

  /* Clamp below the first locus point */
  if( !dl_fgt(fmhz, freq[0]) )
    return bracket;

  /* Clamp above the last locus point */
  if( !dl_flt(fmhz, freq[nc - 1]) )
  {
    bracket.lo = nc - 1;
    bracket.hi = nc - 1;
    return bracket;
  }

  /* Scan for the in-order span bracketing fmhz */
  for( idx = 0; idx < nc - 1; idx++ )
  {
    double lo_f = freq[idx];
    double hi_f = freq[idx + 1];

    if( !dl_fgt(lo_f, fmhz) && !dl_fgt(fmhz, hi_f) )
    {
      double denom = hi_f - lo_f;

      bracket.lo = idx;
      bracket.hi = idx + 1;
      if( dl_fgt(denom, 0.0) )
          bracket.frac = (fmhz - lo_f) / denom;
      return bracket;
    }
  }

  /* Non-monotonic fallback: nearest index by absolute frequency difference */
  nearest = 0;
  best_diff = fabs( fmhz - freq[0] );
  for( idx = 1; idx < nc; idx++ )
  {
    double diff = fabs( fmhz - freq[idx] );
    if( dl_flt(diff, best_diff) )
    {
      best_diff = diff;
      nearest = idx;
    }
  }
  bracket.lo = nearest;
  bracket.hi = nearest;
  return bracket;

} /* fp_locus_bracket() */

/*-----------------------------------------------------------------------*/

/* Smith-chart background grid, expressed as normalised resistance and
 * reactance values.  Each constant-resistance value yields a full circle and
 * each constant-reactance value a mirrored arc pair; the geometry derives
 * from the value so positions stay exact rather than approximated. */

/* Grid colors come from the active theme and widths from the width config;
 * each grid row binds a theme color role and a width purpose by enum key,
 * resolved per row at draw time. */

/* Smith-chart paint order (ascending z paints later, hence on top):
 *   constant-X arcs < constant-R circles < unity/major grid and real axis <
 *   text labels (FP_Z_TEXT) < locus (FP_Z_LEFT) < current-frequency marker
 *   (FP_Z_GREEN).  The three grid sublayers occupy the band reserved between
 *   FP_Z_GRID and FP_Z_TEXT. */
#define SMITH_Z_X      (FP_Z_GRID + 1.0f)
#define SMITH_Z_R      (FP_Z_GRID + 2.0f)
#define SMITH_Z_MAJOR  (FP_Z_GRID + 3.0f)

#define SMITH_MARK_STEP  4
#define SMITH_LABEL_GAP  10.0

typedef struct {
  double       value;  /* normalised resistance or |reactance| */
  theme_role_e color;  /* theme color role */
  fp_width_e   width;  /* width purpose */
  float        depth;  /* painter's-algorithm layer; see paint order above */
  gboolean     label;
} smith_grid_t;

static const smith_grid_t smith_r_grid[] = {
  { 0.0, THEME_ROLE_AXIS,          FP_W_AXIS,          SMITH_Z_MAJOR, FALSE },
  { 0.2, THEME_ROLE_GRID_PRIMARY,  FP_W_GRID,          SMITH_Z_R,     TRUE  },
  { 0.5, THEME_ROLE_GRID_PRIMARY,  FP_W_GRID,          SMITH_Z_R,     TRUE  },
  { 1.0, THEME_ROLE_GRID_EMPHASIS, FP_W_GRID_EMPHASIS, SMITH_Z_MAJOR, FALSE },
  { 2.0, THEME_ROLE_GRID_PRIMARY,  FP_W_GRID,          SMITH_Z_R,     TRUE  },
  { 5.0, THEME_ROLE_GRID_PRIMARY,  FP_W_GRID,          SMITH_Z_R,     TRUE  },
};

static const smith_grid_t smith_x_grid[] = {
  { 0.2, THEME_ROLE_GRID_SECONDARY, FP_W_GRID,          SMITH_Z_X,     FALSE },
  { 0.5, THEME_ROLE_GRID_SECONDARY, FP_W_GRID,          SMITH_Z_X,     TRUE  },
  { 1.0, THEME_ROLE_GRID_EMPHASIS,  FP_W_GRID_EMPHASIS, SMITH_Z_MAJOR, TRUE  },
  { 2.0, THEME_ROLE_GRID_SECONDARY, FP_W_GRID,          SMITH_Z_X,     TRUE  },
  { 5.0, THEME_ROLE_GRID_SECONDARY, FP_W_GRID,          SMITH_Z_X,     TRUE  },
};

/* smith_polar()
 *
 * Screen point on a chart circle: centre (cx, cy), radius in pixels, angle in
 * degrees counter-clockwise from the positive real axis.  Screen y inverts so
 * a positive angle rises into the upper half.  Single placement primitive for
 * every label that rides a circular locus.
 */
  static void
smith_polar( double cx, double cy, double radius, double deg, int *px, int *py )
{
  double a = deg * (M_PI / 180.0);

  *px = (int)( cx + radius * cos(a) );
  *py = (int)( cy - radius * sin(a) );

} /* smith_polar() */

/* smith_draw_resistance()
 *
 * Deposits one constant-resistance circle and its bold value label.  The
 * circle centres at re = r/(r+1) with radius 1/(r+1) in the normalised
 * reflection plane.  The label rides a glyph-width and a half outside its own
 * circle at the angle of a capacitive band point so it clears the circle
 * stroke as well as the real axis and the other grid lines: r below 3 lands
 * between the -j0.5 and -j1 arcs, r of 5 lands between the -j1 and -j2 arcs.
 */
  static void
smith_draw_resistance( fp_render_t *fp, int x0, int y0, double half,
    double dw, const fp_style_t *style, const smith_grid_t *g )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  rgb_f_t color = th->colors[g->color];
  float   width = w->widths[g->width];
  double cx  = x0 + ( g->value / (g->value + 1.0) ) * half;
  double rad = half / (g->value + 1.0);

  fp_add_arc( fp, cx, y0, rad, 0.0, M_2PI,
      (fp_stroke_t){ .color = color, .width = width * density.stroke,
                     .z_mid = g->depth } );

  if( g->label )
  {
    char   buf[16];
    double band = ( g->value < 3.0 ) ? -0.75 : -1.5;
    double u0   = g->value / (g->value + 1.0);
    double gre, gim;
    int    lx, ly;

    Calculate_Smith( g->value, band, 1.0, &gre, &gim );
    smith_polar( cx, y0, rad + 1.5 * dw,
        atan2( gim, gre - u0 ) * (180.0 / M_PI), &lx, &ly );

    g_snprintf( buf, sizeof(buf), "%g", g->value );
    fp_add_text(fp, lx, ly, density.text, buf,
                JUSTIFY_CENTER | JUSTIFY_MIDDLE | JUSTIFY_BOLD, color);
  }

} /* smith_draw_resistance() */

/* smith_draw_reactance()
 *
 * Deposits a mirrored constant-reactance arc pair and its outer-circle value
 * labels.  Both arcs converge at Gamma = 1; each centres one radius off the
 * real axis with radius 1/x.  The arc terminates where it meets the unit
 * circle, the end angle computed exactly from x to avoid edge mismatch.
 */
  static void
smith_draw_reactance( fp_render_t *fp, int x0, int y0, double half,
    double dw, const fp_style_t *style, const smith_grid_t *g )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  rgb_f_t color = th->colors[g->color];
  float   width = w->widths[g->width];
  double x   = g->value;
  double rad = half / x;
  double cx  = x0 + half;
  double end = M_PI + atan2( (x*x - 1.0) / (x * (x*x + 1.0)),
                             2.0 / (x*x + 1.0) );
  fp_stroke_t s = { .color = color, .width = width * density.stroke,
                    .z_mid = g->depth };

  fp_add_arc( fp, cx, y0 - rad, rad, M_PI / 2.0, end, s );

  /* The mirror twin spans the same angular extent below the real axis.
   * Arc angles follow cairo_arc's increasing-sweep convention, so the
   * capacitive arc orders its endpoints low-to-high (-end < -M_PI/2). */
  fp_add_arc( fp, cx, y0 + rad, rad, -end, -M_PI / 2.0, s );

  if( g->label )
  {
    char   buf[16];
    double dir_re = (x*x - 1.0) / (x*x + 1.0);  /* unit-circle landing direction */
    double dir_im = 2.0 * x / (x*x + 1.0);
    double deg = atan2( dir_im, dir_re ) * (180.0 / M_PI);
    int    ux, uy, lx, ly;

    /* Inductive label rides the rim at the landing angle; the capacitive
     * twin mirrors it across the real axis. */
    smith_polar( x0, y0, half + SMITH_LABEL_GAP,  deg, &ux, &uy );
    smith_polar( x0, y0, half + SMITH_LABEL_GAP, -deg, &lx, &ly );

    g_snprintf( buf, sizeof(buf), "j%g", x );
    fp_add_text(fp, ux, uy, density.text, buf,
                JUSTIFY_CENTER | JUSTIFY_MIDDLE, color);
    g_snprintf( buf, sizeof(buf), "-j%g", x );
    fp_add_text(fp, lx - (int)(0.5 * dw), ly, density.text, buf,
                JUSTIFY_CENTER | JUSTIFY_MIDDLE, color);
  }

} /* smith_draw_reactance() */

/* smith_draw_perimeter()
 *
 * Deposits the two outer-rim scales in the reserved annulus: the reflection-
 * coefficient angle (ticks every 10°, degree labels every 30°) and the
 * wavelengths-toward-generator scale (0 at the short, clockwise, 0.5λ per
 * turn).  Screen y is inverted so positive reactance reads in the upper half.
 */
  static void
smith_draw_perimeter( fp_render_t *fp, int x0, int y0, double half, double rh,
    const fp_style_t *style )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  rgb_f_t  perim_c = th->colors[THEME_ROLE_GRID_PERIMETER];
  rgb_f_t  wtg_c   = th->colors[THEME_ROLE_GRID_SCALE];
  int      deg;
  int      k;

  for( deg = 0; deg < 360; deg += 10 )
  {
    double   rad   = deg * M_PI / 180.0;
    double   c     = cos( rad );
    double   s     = sin( rad );
    gboolean major = ( deg % 30 == 0 );
    double   t1    = half + ( major ? 7.0 : 4.0 );
    int      x1    = x0 + (int)( c * half );
    int      y1    = y0 - (int)( s * half );
    int      x2    = x0 + (int)( c * t1 );
    int      y2    = y0 - (int)( s * t1 );

    fp_add_line( fp, x1, y1, x2, y2,
        (fp_stroke_t){ .color = perim_c, .width = w->widths[FP_W_GRID] * density.stroke, .z_mid = SMITH_Z_MAJOR } );

    /* The 0 Ω and ∞ Ω endpoints own the horizontal axis terminals */
    if( major && deg != 0 && deg != 180 )
    {
      char buf[16];
      int  lx, ly;
      int  signed_deg = ( deg <= 180 ) ? deg : deg - 360;

      smith_polar( x0, y0, half + SMITH_LABEL_GAP + 0.75 * rh,
          (double)deg, &lx, &ly );

      g_snprintf( buf, sizeof(buf), "%d°", signed_deg );
      fp_add_text(fp, lx, ly, density.text, buf,
                  JUSTIFY_CENTER | JUSTIFY_MIDDLE, perim_c);
    }
  }

  for( k = 0; k < 10; k++ )
  {
    double wl  = k * 0.05;
    double phi = 180.0 - wl * 720.0;
    int    lx, ly;
    char   buf[16];

    /* Skip the axis-coincident marks; the 0 Ω and ∞ Ω endpoints own those */
    if( ((int)phi) % 180 == 0 )
      continue;

    smith_polar( x0, y0, half + SMITH_LABEL_GAP + 0.75 * rh, phi, &lx, &ly );

    g_snprintf( buf, sizeof(buf), "%.2f", wl );
    fp_add_text(fp, lx, ly, density.text, buf,
                JUSTIFY_CENTER | JUSTIFY_MIDDLE, wtg_c);
  }

} /* smith_draw_perimeter() */

/* smith_draw_grid()
 *
 * Lays down the full Smith-chart background: the real axis, every constant-
 * resistance circle and constant-reactance arc from the grid tables, and the
 * matched-point marker at the chart centre.
 */
  static void
smith_draw_grid( fp_render_t *fp, GtkWidget *area, int x0, int y0, int scale,
    const fp_style_t *style )
{
  const theme_t    *th = style->theme;
  const fp_width_t *w  = style->width;
  fp_density_t      density = style->density;
  double  half   = scale / 2.0;
  rgb_f_t axis_c = th->colors[THEME_ROLE_AXIS];
  int     dw_px, dh_px;
  int     ex, ey;
  double  dw;
  size_t  i;

  pango_text_size( area, &dw_px, &dh_px, "0" );
  dw = (double)dw_px;

  fp_add_line( fp, x0 - (int)half, y0, x0 + (int)half, y0,
      (fp_stroke_t){ .color = axis_c, .width = w->widths[FP_W_AXIS] * density.stroke, .z_mid = SMITH_Z_MAJOR } );

  for( i = 0; i < G_N_ELEMENTS(smith_r_grid); i++ )
    smith_draw_resistance( fp, x0, y0, half, dw, style, &smith_r_grid[i] );

  for( i = 0; i < G_N_ELEMENTS(smith_x_grid); i++ )
    smith_draw_reactance( fp, x0, y0, half, dw, style, &smith_x_grid[i] );

  /* Bright matched-point dot at the chart centre */
  fp_add_filled_circle( fp, x0, y0, 3, SMITH_Z_MAJOR, th->colors[THEME_ROLE_MARKER_EXTREME] );

  /* Real-axis endpoints: left is a short (0 Ω), right an open (∞ Ω) */
  smith_polar( x0, y0, half + SMITH_LABEL_GAP, 180.0, &ex, &ey );
  fp_add_text(fp, ex, ey, density.text, _("0 Ω"),
              JUSTIFY_RIGHT | JUSTIFY_MIDDLE, axis_c);
  smith_polar( x0, y0, half + SMITH_LABEL_GAP, 0.0, &ex, &ey );
  fp_add_text(fp, ex, ey, density.text, _("∞ Ω"),
              JUSTIFY_LEFT | JUSTIFY_MIDDLE, axis_c);

  smith_draw_perimeter( fp, x0, y0, half, (double)dh_px, style );

} /* smith_draw_grid() */

/* smith_draw_legend()
 *
 * Stacks a colour key upward from the lower-left plot corner so the
 * constant-resistance circles, constant-reactance arcs, impedance locus, and
 * rim scales can be told apart without prior Smith-chart familiarity.  Entries
 * read top to bottom; each row pairs a colour swatch with its description so
 * the swatches and the text align in two columns.
 */
  static void
smith_draw_legend( fp_render_t *fp, int x, int y_bottom, int line_h, float scale,
    const theme_t *th )
{
  struct { const char *text; rgb_f_t color; } key[] = {
    { _("constant R"),           th->colors[THEME_ROLE_GRID_PRIMARY]   },
    { _("constant X"),           th->colors[THEME_ROLE_GRID_SECONDARY] },
    { _("unity R, X"),           th->colors[THEME_ROLE_GRID_EMPHASIS]  },
    { _("match (z = 1)"),        th->colors[THEME_ROLE_MARKER_EXTREME] },
    { _("impedance vs freq"),    th->colors[THEME_ROLE_SERIES_PRIMARY] },
    { _("selected frequency"),   th->colors[THEME_ROLE_CURSOR]         },
    { _("reflection angle (°)"), th->colors[THEME_ROLE_GRID_PERIMETER] },
    { _("wavelengths → gen"),    th->colors[THEME_ROLE_GRID_SCALE]     },
  };
  size_t n  = G_N_ELEMENTS(key);
  int    sw = line_h / 2;            /* swatch half-extent */
  int    tx = x + line_h + 4;        /* text column, clear of the swatch */
  size_t i;

  for( i = 0; i < n; i++ )
  {
    int row_y = y_bottom - (int)(n - 1 - i) * line_h;

    /* Swatch centred on the text row's vertical midpoint */
    fp_add_filled_square( fp, x + sw, row_y - sw, sw, FP_Z_LEFT, key[i].color );
    fp_add_text(fp, tx, row_y, scale, key[i].text,
                JUSTIFY_LEFT | JUSTIFY_ABOVE, key[i].color);
  }

} /* smith_draw_legend() */

/*-----------------------------------------------------------------------*/

/* Plot_Graph_Smith()
 *
 * Plots graphs of two functions against a common variable
 */
  void
Plot_Graph_Smith(
	freqplots_view_t *v, fp_render_t *fp,
	double *fa, double *fb, double *fc,
	int nc, int *card_nfsteps, int posn )
{
  int plot_height, plot_y_position;
  int idx;
  GdkPoint *points = NULL;
  int scale, x0, y0, x, y;
  int ohm_w, ohm_h;
  double re, im;
  char z0buf[24];

  GdkRectangle plot_rect;

  /* Pango layout size */
  static int layout_width, layout_height, width1, height;

  pango_text_size(canvas_widget(v->canvas), &layout_width, &layout_height, "000000");

  /* Horizontal margin sized to the real-axis endpoint label plus a small gap
   * so the chart fills the width while ∞ Ω and 0 Ω stay off the window edge */
  pango_text_size(canvas_widget(v->canvas), &ohm_w, &ohm_h, _("∞ Ω"));

  /* Available height for each graph.
   * (np=number of graphs to be plotted) */
  plot_height = v->height / v->ngraph;
  plot_y_position   = ( v->height * posn) / v->ngraph;

  /* Plot box rectangle */
  plot_rect.x = ohm_w + 2;
  plot_rect.y = plot_y_position + 2;
  plot_rect.width = v->width - 2 * ( ohm_w + 2 );
  plot_rect.height = plot_height - 8 - 2 * layout_height;

  /* Reserve vertical space for the chart title row */
  pango_text_size(canvas_widget(v->canvas), &width1, &height, _("Smith Chart") );
  plot_rect.y += height;

  x0 = plot_rect.x + plot_rect.width  / 2;
  y0 = plot_rect.y + plot_rect.height / 2;
  scale = plot_rect.width;
  if( scale > plot_rect.height )
      scale = plot_rect.height;

  /* Reserve an annulus outside the unit circle for the perimeter scales */
  scale -= 6 * layout_height;

  /* Per-graph density resolved once: stroke widths halve and label sizes
   * shrink 1/n so stacked charts stay legible and proportional. */
  fp_density_t density = fp_density_for( v->ngraph );

  /* Active color theme and per-purpose widths resolved once at plot entry and
   * passed down to the leaf drawing helpers. */
  const theme_t    *th = theme_active();
  const fp_width_t *w  = fp_width_active();
  fp_style_t        style = { .theme = th, .width = w, .density = density };

  /* Draw smith background grid */
  smith_draw_grid( fp, canvas_widget(v->canvas), x0, y0, scale, &style );

  /* Corner overlays annotate the plot frame rather than the chart geometry,
   * so they hold base size regardless of how many graphs share the window;
   * only the in-chart grid labels follow the 1/n density scale. */
  const float annot_text = 1.0f;

  /* Normalising-impedance annotation */
  g_snprintf( z0buf, sizeof(z0buf), _("Z₀ = %g Ω"), calc_data.zo );
  fp_add_text(fp, plot_rect.x, plot_rect.y, annot_text, z0buf,
              JUSTIFY_LEFT | JUSTIFY_BELOW, th->colors[THEME_ROLE_MARKER_EXTREME]);

  /* Reactance-sign reminders in the clear plot corners; cyan to match the
   * right-hand series so right-side colors read consistently */
  fp_add_text(fp, plot_rect.x + plot_rect.width, plot_rect.y, annot_text,
              _("INDUCTIVE (+jX)"), JUSTIFY_RIGHT | JUSTIFY_BELOW, th->colors[THEME_ROLE_SERIES_SECONDARY]);
  fp_add_text(fp, plot_rect.x + plot_rect.width,
              plot_rect.y + plot_rect.height, annot_text,
              _("CAPACITIVE (-jX)"), JUSTIFY_RIGHT | JUSTIFY_ABOVE, th->colors[THEME_ROLE_SERIES_SECONDARY]);

  /* Colour key in the lower-left corner; swatch and row spacing track the
   * base label height */
  smith_draw_legend( fp, plot_rect.x,
      plot_rect.y + plot_rect.height, layout_height, annot_text, th );

  /* Calculate points to plot */
  mem_array_alloc(&points, nc);

  if( points == NULL )
  {
    pr_err("memory allocation for points failed\n");
    Stop( ERR_OK, _("Draw_Graph():"
          "Memory allocation for points failed") );
    return;
  }

  for( idx = 0; idx < nc; idx++ )
  {
    Calculate_Smith( fa[idx], fb[idx], calc_data.zo, &re, &im );

    // flip plot vertically because negative imaginary is the bottom half
    im = -im;

    points[idx].x = x0 + (gint)( re * scale / 2 );
    points[idx].y = y0 + (gint)( im * scale / 2 );
    fp_add_filled_square( fp, points[idx].x, points[idx].y, SMITH_MARK_STEP,
        FP_Z_LEFT, th->colors[THEME_ROLE_SERIES_PRIMARY] );
  }

  /* Draw one locus per FR card so the trace does not jump between the
   * disjoint frequency ranges of adjacent cards.  card_nfsteps partitions the
   * flat point array into the same contiguous per-card blocks the XY plots
   * use, summing to nc. */
  int card, coff = 0;
  for( card = 0; card < calc_data.FR_cards; card++ )
  {
    int cn = card_nfsteps[card];
    if( cn > 0 )
      fp_add_polyline( fp, points + coff, cn,
          (fp_stroke_t){ .color = th->colors[THEME_ROLE_SERIES_PRIMARY], .width = w->widths[FP_W_TRACE] * density.stroke,
                         .z_mid = FP_Z_LEFT } );
    coff += cn;
  }

  /* Deposit the impedance curve so a chart click resolves to a frequency; the
   * curve serves both primary projection and secondary snapping, so it fills
   * the continuum and snap roles with the same arrays. */
  fp_locus_add( v, &(fp_locus_input_t){
      .panel = FP_PANEL_SMITH, .rect = plot_rect,
      .cont_pts = points, .cont_freq = fc, .cont_n = nc,
      .snap_pts = points, .snap_freq = fc, .snap_n = nc } );

  mem_array_free(&points);

  /* Draw a vertical line to show current freq if it was
   * changed by a user click on the plots drawingarea */
  if( calc_data.fmhz_save > 0.0 )
  {
    rgb_f_t cursor_c = fp_cursor_color(th);

    /* Interpolate the completed sweep locus at the click frequency so the
     * marker tracks the click without waiting on a pending solve */
    fp_locus_bracket_t bracket = fp_locus_bracket( fc, nc, calc_data.fmhz_save );
    double zr = fa[bracket.lo] + (fa[bracket.hi] - fa[bracket.lo]) * bracket.frac;
    double zi = fb[bracket.lo] + (fb[bracket.hi] - fb[bracket.lo]) * bracket.frac;

    Calculate_Smith( zr, zi, calc_data.zo, &re, &im );

    // flip plot vertically because negative imaginary is the bottom half
    im = -im;

    x = x0 + (gint)( re * scale / 2 );
    y = y0 + (gint)( im * scale / 2 );
    fp_add_filled_square( fp, x, y, 8, FP_Z_GREEN, cursor_c );
  }

} /* Plot_Graph_Smith() */
