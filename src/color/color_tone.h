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

#ifndef COLOR_TONE_H
#define COLOR_TONE_H  1

#include <glib.h>

/* Tone-curve inputs: natural parameter and the memoized dB floor
 * 10^(-R/20).  Transfers read this, never the animation phase. */
typedef struct { double param, floor_ratio; } tone_param_t;

/*-----------------------------------------------------------------------
 * Tone scale-family axis: normalized-magnitude transfer applied before
 * the colorizer.  Each family is driven by one brightness parameter held
 * in rc_config.color_fam_param[family] in slider domain; param_map
 * recovers the natural parameter.  POWER at gamma 1 is the identity and
 * the fall-through baseline.
 *----------------------------------------------------------------------*/
typedef enum
{
  COLOR_TONE_POWER = 0, /* n^gamma; gamma 0.5/1/2 covers old Sqrt/Linear/Squared */
  COLOR_TONE_DB,        /* log with user range R dB; floor 10^(-R/20) */
  COLOR_TONE_ASINH,     /* asinh(n/k) / asinh(1/k) */
  COLOR_TONE_MULAW,     /* log1p(mu*n) / log1p(mu) */
  COLOR_TONE_REINHARD,  /* (1+k)*n / (n+k) */
  COLOR_TONE_SIGMOID,   /* tanh(a*n) / tanh(a) */
  COLOR_TONE_NONE,      /* constant 1: no brightness shaping */
  COLOR_TONE_NUM
} color_tone_t;

/* One scale family: widget ids, persisted slider var, display strings,
 * and the slider-domain-to-natural-parameter and transfer functions. */
typedef struct
{
  const char *row_id;     /* glade slider-row box id, visibility-swapped */
  const char *scale_id;   /* GtkScale id bound to slider_var */
  double     *slider_var; /* &rc_config.color_fam_param[fam], slider domain */
  const char *formula;    /* Pango markup closed form of the transfer */
  const char *value_fmt;  /* printf format rendering the natural parameter */
  const double *marks;    /* slider-domain snap stops, NAN-terminated */
  double    (*param_map)(double v);  /* slider domain -> natural param */
  double    (*transfer)(double n, const tone_param_t *tp);
} color_tone_row_t;

/* Enum-indexed family table; adding a family adds a row, never a branch */
extern const color_tone_row_t color_tones[COLOR_TONE_NUM];

/**
 * color_tone_sanitize() - Bound a persisted family value to the enum
 * @v: raw persisted integer
 *
 * Emits pr_err and returns COLOR_TONE_POWER when out of range.
 */
color_tone_t color_tone_sanitize(int v);

/**
 * color_tone_active() - Resolve the family selection now in effect
 *
 * Reads the committed rc_config selection; a hover stages its candidate
 * into that field, so a preview resolves through the same read.
 */
color_tone_t color_tone_active(void);

/**
 * tone_param_init() - Resolve a family's natural parameter and dB floor
 * @tp:  destination tone parameters
 * @fam: tone family selection
 *
 * Recovers the natural parameter from the persisted slider variable and
 * memoizes the dB floor 10^(-R/20); other families leave floor_ratio 1.
 */
void tone_param_init(tone_param_t *tp, color_tone_t fam);

/**
 * color_tone_transfer_norm() - Normalize a magnitude, then apply the transfer
 * @fam: tone family whose transfer applies
 * @tp:  resolved tone parameters from tone_param_init()
 * @mag: raw magnitude
 * @max: normalization peak; a non-positive peak yields 0
 *
 * Divides mag by the peak under a zero-peak guard, then runs the family
 * transfer on the normalized magnitude.
 */
double color_tone_transfer_norm(color_tone_t fam, const tone_param_t *tp,
    double mag, double max);

/* Slider-domain mappings referenced by the family table */
double map_exp2(double v);      /* 2^v: power gamma */
double map_exp10(double v);     /* 10^v: asinh k, Reinhard k, sigmoid a */
double map_ident(double v);     /* v: dB range R */
double map_mulaw_db(double v);  /* 10^(v/20) - 1: mu from compression dB */

/* Normalized-magnitude transfers referenced by the family table; each is
 * total on n in [0, 1] with the singular point excluded by slider bounds. */
double xfer_power(double n, const tone_param_t *tp);
double xfer_db(double n, const tone_param_t *tp);
double xfer_asinh(double n, const tone_param_t *tp);
double xfer_mulaw(double n, const tone_param_t *tp);
double xfer_reinhard(double n, const tone_param_t *tp);
double xfer_sigmoid(double n, const tone_param_t *tp);
double xfer_none(double n, const tone_param_t *tp);

#endif
