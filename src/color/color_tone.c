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
 * color_tone: scale-family table and transfer functions.
 *
 * One enum-indexed row per family owns the widget ids, the persisted
 * slider variable, the Pango formula, the natural-parameter display
 * format, and the param_map/transfer pair.  Consumers evaluate
 * color_tones[fam].transfer(n, tp) with tp filled by tone_param_init().
 */
#include "color_tone.h"
#include "../shared.h"

/* Slider snap stops per family in slider domain, NAN-terminated.  Each
 * set carries the default plus meaningful parameter landmarks: power
 * gamma 2 tracks dissipated power for currents, mu-law 48.1648 dB is the
 * telephony-standard mu = 255. */
static const double marks_power[]    = { -1.0, 0.0, 1.0, NAN };
static const double marks_db[]       = { 20.0, 40.0, 60.0, NAN };
static const double marks_asinh[]    = { -2.0, -1.0, 0.0, NAN };
static const double marks_mulaw[]    = { 20.0, 40.0, 48.164799, NAN };
static const double marks_reinhard[] = { -1.0, 0.0, 1.0, NAN };
static const double marks_sigmoid[]  = { -1.0, 0.0, 1.0, NAN };

const color_tone_row_t color_tones[COLOR_TONE_NUM] =
{
  [COLOR_TONE_POWER] = { .row_id = "anim_fam_row_power",
    .scale_id   = "anim_fam_power",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_POWER],
    .formula    = "v = n<sup>γ</sup>",
    .value_fmt  = "γ = %.2f",
    .param_map  = map_exp2, .transfer = xfer_power,
    .marks      = marks_power },

  [COLOR_TONE_DB] = { .row_id = "anim_fam_row_db",
    .scale_id   = "anim_fam_db",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_DB],
    .formula    = "v = 1 + (20/R)·log₁₀ max(n, 10<sup>−R/20</sup>)",
    .value_fmt  = "%.0f dB",
    .param_map  = map_ident, .transfer = xfer_db,
    .marks      = marks_db },

  [COLOR_TONE_ASINH] = { .row_id = "anim_fam_row_asinh",
    .scale_id   = "anim_fam_asinh",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_ASINH],
    .formula    = "v = asinh(n/k) / asinh(1/k)",
    .value_fmt  = "k = %.3g",
    .param_map  = map_exp10, .transfer = xfer_asinh,
    .marks      = marks_asinh },

  [COLOR_TONE_MULAW] = { .row_id = "anim_fam_row_mulaw",
    .scale_id   = "anim_fam_mulaw",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_MULAW],
    .formula    = "v = ln(1 + μn) / ln(1 + μ)",
    .value_fmt  = "μ = %.3g",
    .param_map  = map_mulaw_db, .transfer = xfer_mulaw,
    .marks      = marks_mulaw },

  [COLOR_TONE_REINHARD] = { .row_id = "anim_fam_row_reinhard",
    .scale_id   = "anim_fam_reinhard",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_REINHARD],
    .formula    = "v = (1+k)·n / (n+k)",
    .value_fmt  = "k = %.3g",
    .param_map  = map_exp10, .transfer = xfer_reinhard,
    .marks      = marks_reinhard },

  [COLOR_TONE_SIGMOID] = { .row_id = "anim_fam_row_sigmoid",
    .scale_id   = "anim_fam_sigmoid",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_SIGMOID],
    .formula    = "v = tanh(a·n) / tanh(a)",
    .value_fmt  = "a = %.2f",
    .param_map  = map_exp10, .transfer = xfer_sigmoid,
    .marks      = marks_sigmoid },

  /* No slider: scale_id and value_fmt stay NULL; the glade row box is
   * empty so the visibility swap shows no parameter control. */
  [COLOR_TONE_NONE] = { .row_id = "anim_fam_row_none",
    .slider_var = &rc_config.color_fam_param[COLOR_TONE_NONE],
    .formula    = "v = 1",
    .param_map  = map_ident, .transfer = xfer_none },
};

/*-----------------------------------------------------------------------*/

  color_tone_t
color_tone_sanitize(int v)
{
  if( v < 0 || v >= COLOR_TONE_NUM )
  {
    pr_err("invalid color tone %d, using power\n", v);
    return COLOR_TONE_POWER;
  }

  return (color_tone_t)v;
}

/*-----------------------------------------------------------------------*/

/** color_tone_active() - Resolve the family selection now in effect */
  color_tone_t
color_tone_active(void)
{
  return color_tone_sanitize(rc_config.color_scale);
}

/*-----------------------------------------------------------------------*/

  void
tone_param_init(tone_param_t *tp, color_tone_t fam)
{
  const color_tone_row_t *row = &color_tones[fam];

  tp->param = row->param_map(*row->slider_var);

  /* Memoize the dB floor 10^(-R/20); other families read param only */
  tp->floor_ratio = (fam == COLOR_TONE_DB)
      ? pow(10.0, -tp->param / 20.0) : 1.0;
}

/*-----------------------------------------------------------------------*/

/**
 * color_tone_transfer_norm() - Normalize a magnitude, then apply the transfer
 * @fam: tone family whose transfer applies
 * @tp:  resolved tone parameters from tone_param_init()
 * @mag: raw magnitude
 * @max: normalization peak; a non-positive peak yields 0
 *
 * Single point of truth for the near-field magnitude-to-coordinate step:
 * divides mag by the peak under a zero-peak guard, then runs the family
 * transfer on the normalized magnitude.
 */
  double
color_tone_transfer_norm(color_tone_t fam, const tone_param_t *tp,
    double mag, double max)
{
  /* Exact zero-peak guard preserves the raw denominator test */
  if( max <= 0.0 )
    return 0.0;

  return color_tones[fam].transfer(mag / max, tp);
}

/*-----------------------------------------------------------------------*/

  double
map_exp2(double v)
{
  return pow(2.0, v);
}

  double
map_exp10(double v)
{
  return pow(10.0, v);
}

  double
map_ident(double v)
{
  return v;
}

  double
map_mulaw_db(double v)
{
  return pow(10.0, v / 20.0) - 1.0;
}

/*-----------------------------------------------------------------------*/

  double
xfer_power(double n, const tone_param_t *tp)
{
  return pow(n, tp->param);
}

  double
xfer_db(double n, const tone_param_t *tp)
{
  /* param is the user range R in dB; floor_ratio memoizes 10^(-R/20) */
  return (log10(fmax(n, tp->floor_ratio)) + tp->param / 20.0)
       / (tp->param / 20.0);
}

  double
xfer_asinh(double n, const tone_param_t *tp)
{
  return asinh(n / tp->param) / asinh(1.0 / tp->param);
}

  double
xfer_mulaw(double n, const tone_param_t *tp)
{
  return log1p(tp->param * n) / log1p(tp->param);
}

  double
xfer_reinhard(double n, const tone_param_t *tp)
{
  return (1.0 + tp->param) * n / (n + tp->param);
}

  double
xfer_sigmoid(double n, const tone_param_t *tp)
{
  return tanh(tp->param * n) / tanh(tp->param);
}

  double
xfer_none(double n, const tone_param_t *tp)
{
  /* Interface parameters unused: the transfer is the constant 1 */
  (void)n;
  (void)tp;
  return 1.0;
}

/*-----------------------------------------------------------------------*/
