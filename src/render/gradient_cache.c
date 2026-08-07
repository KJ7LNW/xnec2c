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
 * gradient_cache: Self-validating gradient legend surface cache.
 *
 * Single source of truth for the gradient legend rendering.
 * The presentation layer (render_dispatch) calls gradient_cache_get_overlay()
 * during render(); the returned surface is then forwarded to whichever
 * backend (Cairo or GL) is active via render_check_result_t.
 * Correctness is enforced by storing the inputs that produced the surface
 * and comparing against current authoritative state on each update call.
 */
#include "gradient_cache.h"
#include "../color/color_palette.h"
#include "../themes/theme.h"
#include "../shared.h"
#include "../rdpattern_ui.h"

#define TEXT_GRADIENT_SPACING 8
#define LINE_WIDTH 2

/* Module-internal singleton; the presentation layer (render_dispatch)
 * calls gradient_cache_get_overlay() and forwards the result to backends. */
static gradient_cache_t rdpat_cache;

/*-----------------------------------------------------------------------*/

/* Draw_Color_Legend_Overlay()
 *
 * Draws a color gradient legend in the bottom right corner
 * showing gain values and corresponding colors
 */
static void
Draw_Color_Legend_Overlay( cairo_t *cr, int fstep )
{
  /* Declare all variables at start */
  int i, x, y;
  char txt[16];
  int pol = calc_data.pol_type;
  const theme_t *th = theme_active();
  const palette_t *ramp = palette_get(PALETTE_RAMP);

  /* Get drawable area dimensions via clip extents.
   * Works for both image surfaces (OpenGL overlay) and
   * window surfaces (Cairo renderer) without depending on global state. */
  double clip_x1, clip_y1, clip_x2, clip_y2;
  cairo_clip_extents(cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
  int surf_width = (int)(clip_x2 - clip_x1);
  int surf_height = (int)(clip_y2 - clip_y1);

  double max_gain, min_gain, color_min;
  double grad_pos, pos, db_val;
  const int width = 20;
  const int height = 200;
  const int margin = 10;
  const int text_width = 80; /* Increased for "dB" suffix */
  const int num_graduations = 5; /* Number of intermediate graduations */

  /* Validate parameters */
  if (!cr || !rad_pattern) return;


  /* Validate frequency step. Note fstep == calc_data.steps_total is valid
   * because an extra index is used the custom frequency when clicking on
   * the graph at an arbitrary point.
   */
  if (fstep < 0 || fstep > calc_data.steps_total) {
    pr_err("fstep=%d is out of bounds. calc_data.steps_total=%d", fstep, calc_data.steps_total);
    return;
  }

  /* Validate polarization type */
  if (pol < 0 || pol >= NUM_POL) return;

  /* Validate rad_pattern members */
  if (!rad_pattern[fstep].max_gain || !rad_pattern[fstep].min_gain) return;

  /* Position in bottom right corner */
  x = surf_width - width - text_width - margin;
  y = surf_height - height - margin;

  /* Get actual min/max gains and scale them */
  max_gain = rad_pattern[fstep].max_gain[pol];
  min_gain = rad_pattern[fstep].min_gain[pol];
  color_min = (min_gain < COLOR_MIN_GAIN) ? COLOR_MIN_GAIN : min_gain;

  /* Scale the gains for color mapping */
  int noise_legend = IS_NOISE_MODE(rc_config.gain_style);
  double scaled_max, scaled_min, scaled_range;

  if (noise_legend)
  {
    /* Use true noise extremes computed during cell scan */
    scaled_max = rad_pattern[fstep].noise_scaled_max;
    scaled_min = rad_pattern[fstep].noise_scaled_min;
  }
  else
  {
    scaled_max = Scale_Gain(max_gain, fstep, rad_pattern[fstep].max_gain_idx[pol]);
    scaled_min = Scale_Gain(color_min, fstep, rad_pattern[fstep].min_gain_idx[pol]);
  }
  scaled_range = scaled_max - scaled_min;

  /* Calculate relative gain points starting at -3dB and stepping by -6dB */
  int num_rel_marks = 0;
  double rel_gains[10]; // Large enough for all possible marks
  double scaled_vals[10];
  double positions[10];

  // Get text height for spacing calculation
  cairo_text_extents_t text_extents;
  cairo_text_extents(cr, "-88 dB", &text_extents);
  double text_height = text_extents.height;
  double min_mark_spacing = text_height * 1.5; // Minimum pixels between marks

  // Start with -3dB
  rel_gains[num_rel_marks++] = -3.0;

  // Add -6dB steps and -10dB multiples
  double current_gain = -6.0;
  double last_y_pos = 0;

  while (current_gain > COLOR_MIN_GAIN) { // Reasonable lower limit
    double gain_val = max_gain + current_gain;
    double scaled_val = Scale_Gain(gain_val, fstep, rad_pattern[fstep].max_gain_idx[pol]);
    double pos = (scaled_val - scaled_min) / scaled_range;
    double y_pos = (1.0 - pos) * (height - 1);

    // Only add mark if it has enough spacing from previous mark
    if (num_rel_marks == 1 || (y_pos - last_y_pos) >= min_mark_spacing) {
      rel_gains[num_rel_marks++] = current_gain;
      last_y_pos = y_pos;
    }
    current_gain -= 6.0;
  }

  // Calculate scaled values and positions
  for (i = 0; i < num_rel_marks; i++) {
    double gain_val = max_gain + rel_gains[i];
    scaled_vals[i] = Scale_Gain(gain_val, fstep, rad_pattern[fstep].max_gain_idx[pol]);
    positions[i] = (scaled_vals[i] - scaled_min) / scaled_range;
  }

  /* Draw color gradient bar */
  if (x >= 0 && y >= 0 &&
      x + width <= surf_width &&
      y + height <= surf_height)
  {
    /* Draw headings */
    int noise_mode = IS_NOISE_MODE(rc_config.gain_style);
    cairo_set_source_rgb_f(cr, th->colors[THEME_ROLE_TEXT_PRIMARY]);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    if (!noise_mode)
    {
      /* Right-justify "Rel." */
      cairo_text_extents_t rel_extents;
      cairo_text_extents(cr, "Rel.", &rel_extents);
      cairo_move_to(cr, x - 5 - rel_extents.width, y - 7);  /* Moved up 2px */
      cairo_show_text(cr, "Rel.");
    }

    /* Left-justify heading: "K" for noise, "Abs." for gain */
    cairo_move_to(cr, x + width + 5, y - 7);  /* Moved up 2px */
    cairo_show_text(cr, noise_mode ? "K/sr" : "Abs.");

    /* Draw color gradient with proper scaling (min at bottom, max at top) */
    for (i = 0; i < height; i++)
    {
      /* Map position in gradient (i=0 is top, i=height-1 is bottom) */
      pos = 1.0 - ((double)i / (double)(height - 1));  // Invert position

      /* Get scaled value at this position */
      double scaled_val = scaled_min + (pos * scaled_range);

      /* Convert to color; guard zero range from degenerate data */
      double color_frac = (scaled_range > 0.0)
          ? (scaled_val - scaled_min) / scaled_range : 0.0;
      cairo_set_source_rgb_f(cr, palette_lookup_scaled(ramp, color_frac, 1.0));
      cairo_rectangle(cr, x, y + i, width, 1);
      cairo_fill(cr);
    }

    /* Draw border around gradient */
    cairo_set_source_rgb_f(cr, th->colors[THEME_ROLE_TEXT_PRIMARY]);
    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_rectangle(cr, x, y-(LINE_WIDTH/2), width, height+2*(LINE_WIDTH/2));
    cairo_stroke(cr);

    /* Draw graduation marks with matching colors and values (min at bottom, max at top) */
    for (i = 0; i < num_graduations; i++)
    {
      /* Calculate scaled value at this graduation (reverse order) */
      grad_pos = (double)(num_graduations - 1 - i) / (double)(num_graduations - 1);

      /* Get scaled value at this position */
      double scaled_val = scaled_min + (grad_pos * scaled_range);

      /* Convert back to dB for display */
      db_val = Inverse_Scale_Gain(scaled_val);

      /* Draw graduation mark with matching color */
      cairo_set_source_rgb_f(cr, palette_lookup_scaled(ramp, grad_pos, 1.0));

      int grad_y = y + (i * height) / (num_graduations - 1);
      cairo_move_to(cr, x + width - (LINE_WIDTH/2), grad_y);
      cairo_line_to(cr, x + width + TEXT_GRADIENT_SPACING, grad_y);
      cairo_stroke(cr);

      /* Show value in text */
      if (db_val < -999.99) db_val = -999.99;
      /* First measure width of numeric part */
      char num_txt[16];
      snprintf(num_txt, sizeof(num_txt)-1, "%.2f", db_val);
      cairo_text_extents_t num_extents;
      cairo_text_extents(cr, num_txt, &num_extents);

      /* Format with appropriate units */
      if (noise_mode)
        snprintf(txt, sizeof(txt)-1, "%.0f", db_val);
      else
        snprintf(txt, sizeof(txt)-1, "%.2f", db_val);
      cairo_set_source_rgb_f(cr, th->colors[THEME_ROLE_TEXT_PRIMARY]);
      /* Right-justify absolute gain values with normal weight */
      cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

      /* Position text so numeric part aligns with gradient */
      cairo_move_to(cr, x + width + TEXT_GRADIENT_SPACING, grad_y + 4);
      cairo_show_text(cr, txt);
    }

    /* Draw relative gain marks on left side (gain modes only) */
    for (i = 0; !noise_mode && i < num_rel_marks; i++) {
      int mark_y = y + (int)((1.0 - positions[i]) * (height - 1));

      /* Draw mark */
      cairo_set_source_rgb_f(cr, palette_lookup_scaled(ramp, positions[i], 1.0));
      cairo_move_to(cr, x - TEXT_GRADIENT_SPACING, mark_y);
      cairo_line_to(cr, x, mark_y);
      cairo_stroke(cr);

      /* Show value */
      cairo_set_source_rgb_f(cr, th->colors[THEME_ROLE_TEXT_PRIMARY]);
      if (i == 0) {  /* -3dB point */
        /* Ensure -3dB is bold */
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        snprintf(txt, sizeof(txt)-1, "-3 dB");
      } else {
        /* Other relative gain values in normal weight */
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        snprintf(txt, sizeof(txt)-1, "%d dB", (int)rel_gains[i]);
      }
      /* Right-justify relative gain values */
      cairo_text_extents_t extents;
      cairo_text_extents(cr, txt, &extents);
      cairo_move_to(cr, x - TEXT_GRADIENT_SPACING - extents.width, mark_y + 4);
      cairo_show_text(cr, txt);
    }
  }
}

/*-----------------------------------------------------------------------*/

/**
 * gradient_cache_update() - Reconcile cache with current authoritative state
 * @cache: cache instance
 * @w:     current viewport width
 * @h:     current viewport height
 *
 * Compares stored presentation_cache_key and (w, h) against current values.
 * On any mismatch the old surface is destroyed and a fresh ARGB32 surface
 * is created, Draw_Color_Legend_Overlay() is called to populate it, and
 * the input snapshot is stored.  Matching inputs are a no-op.
 */
  static void
gradient_cache_update(gradient_cache_t *cache, int w, int h)
{
  /* Reject invalid dimensions (transient during widget teardown or
   * before the first resize event propagates). */
  if( w <= 0 || h <= 0 )
    return;

  int fstep = calc_data.freq_step;
  presentation_cache_key_t cur_key =
      presentation_cache_key_build(fstep);

  if( cache->surface != NULL &&
      cache->width  == w &&
      cache->height == h &&
      presentation_cache_key_match(&cache->cache_key, &cur_key) )
    return;

  if( cache->surface != NULL )
    cairo_surface_destroy(cache->surface);

  cache->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  if( cairo_surface_status(cache->surface) != CAIRO_STATUS_SUCCESS )
  {
    pr_warn("gradient_cache_update: surface creation failed (%dx%d)\n", w, h);
    cairo_surface_destroy(cache->surface);
    cache->surface    = NULL;
    /* Invalidate input snapshot so a transient retry rebuilds even when
     * dimensions and authoritative state remain unchanged. */
    cache->width  = 0;
    cache->height = 0;
    presentation_cache_key_invalidate(&cache->cache_key);
    return;
  }

  {
    cairo_t *cr = cairo_create(cache->surface);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    Draw_Color_Legend_Overlay(cr, fstep);
    cairo_destroy(cr);
  }

  cache->version++;
  cache->width     = w;
  cache->height    = h;
  cache->cache_key = cur_key;

} /* gradient_cache_update() */

/*-----------------------------------------------------------------------*/

/**
 * gradient_cache_get_overlay() - Presentation entry point for gradient legend
 * @w:       current viewport width
 * @h:       current viewport height
 *
 * Single presentation-layer decision point for the gradient legend.
 * Returns {NULL, 0} when the legend is disabled or dimensions are invalid.
 * Otherwise updates the module-internal singleton and returns the surface
 * coupled with its version for downstream staleness detection.
 */
  gradient_result_t
gradient_cache_get_overlay(int w, int h)
{
  gradient_result_t none = {NULL, 0};

  if( !rc_config.rdpattern_gradient_key )
    return none;

  if( w <= 0 || h <= 0 )
    return none;

  gradient_cache_update(&rdpat_cache, w, h);

  return (gradient_result_t){rdpat_cache.surface, rdpat_cache.version};

} /* gradient_cache_get_overlay() */

/*-----------------------------------------------------------------------*/
