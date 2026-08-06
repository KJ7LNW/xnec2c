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
 * chroma_nearfield: parent draw-time near-field color and geometry resolver.
 *
 * Near field joins wire and patch on one color lifecycle: the immutable phasor
 * in near_field_fstep[fstep].points is the source, this resolver derives the
 * real vectors, their scaled displacement, and their palette colors at draw,
 * and the render backends consume the resolved arrays.  No color and no
 * geometry cross the parent/child pipe; the palette stays a parent concern.
 * The phase source is chosen by state (animation_is_active, flow_phase,
 * nf_static_mode), never by a per-frame model mutation.
 */
#include "chroma_nearfield.h"
#include "../color/color_edge.h"
#include "../color/color_palette.h"
#include "../color/color_tone.h"
#include "../structure_ui.h"
#include "../shared.h"

/* Per-channel resolved-frame buffers and their input-edge gates.  One draw's
 * geometry, color, and scratch magnitude live here, indexed by nf_channel_t. */
static nf_vector_t *nf_vec_buf[NF_CHAN_NUM];
static rgb_f_t     *nf_col_buf[NF_CHAN_NUM];
static double      *nf_mag_buf[NF_CHAN_NUM];

typedef struct
{
  color_edge_t edge;
  gboolean     valid;
} nf_gate_t;

static nf_gate_t nf_gate[NF_CHAN_NUM];

/*-----------------------------------------------------------------------*/

/**
 * nf_ramp_color() - Amplitude-ramp colorize leaf
 * @fam: active tone family
 * @tp:  tone parameter for the family
 * @mag: point magnitude
 * @max: frame magnitude maximum
 *
 * The one near-field colorize: the family transfer of mag/max looked up on
 * the ramp palette, shared with wire and patch and ready for far field.
 */
static inline rgb_f_t
nf_ramp_color(color_tone_t fam, const tone_param_t *tp, double mag, double max)
{
  return palette_lookup_scaled(palette_get(PALETTE_RAMP),
      color_tone_transfer_norm(fam, tp, mag, max), 1.0);
}

/*-----------------------------------------------------------------------*/

  double
nf_real_vector(const near_field_point_t *p, nf_channel_t chan,
    gboolean live, double phase, nf_static_mode_t mode, double out[3])
{
  /* Zero-init gives definite assignment on the unreachable channels below,
   * so the post-switch use needs no default fall-through */
  double amp[3] = { 0.0 }, ph[3] = { 0.0 }, mag, r;

  switch( chan )
  {
    case NF_CHAN_E:
      amp[0] = p->ex;  amp[1] = p->ey;  amp[2] = p->ez;
      ph[0]  = p->fex; ph[1]  = p->fey; ph[2]  = p->fez;
      break;

    case NF_CHAN_H:
      amp[0] = p->hx;  amp[1] = p->hy;  amp[2] = p->hz;
      ph[0]  = p->fhx; ph[1]  = p->fhy; ph[2]  = p->fhz;
      break;

    case NF_CHAN_POV:
    case NF_CHAN_NUM:
      /* Poynting composes E and H externally; it has no own phasor */
      BUG("nf_real_vector: channel %d carries no phasor\n", chan);
      break;
  }

  if( live )
  {
    /* Instantaneous real vector at the animation phase */
    out[0] = amp[0] * cos(phase + ph[0]);
    out[1] = amp[1] * cos(phase + ph[1]);
    out[2] = amp[2] * cos(phase + ph[2]);
    mag = sqrt(out[0]*out[0] + out[1]*out[1] + out[2]*out[2]);
  }
  else if( mode == NF_STATIC_SNAPSHOT )
  {
    /* Phase-zero instantaneous snapshot */
    out[0] = amp[0] * cos(ph[0]);
    out[1] = amp[1] * cos(ph[1]);
    out[2] = amp[2] * cos(ph[2]);
    mag = sqrt(out[0]*out[0] + out[1]*out[1] + out[2]*out[2]);
  }
  else
  {
    /* NF_STATIC_PEAK: optimal-phase peak envelope */
    Nf_Peak_Vector(amp[0], amp[1], amp[2], ph[0], ph[1], ph[2],
        &out[0], &out[1], &out[2], &r);
    mag = r;
  }

  return mag;
}

/*-----------------------------------------------------------------------*/

  nf_frame_t
chroma_proj_frame_nearfield(int fstep, nf_channel_t chan)
{
  nf_frame_t out = { NULL, NULL };
  near_field_t *nf;
  color_tone_t fam;
  tone_param_t tp;
  color_edge_t want;
  gboolean live;
  double phase, dr, max;
  nf_static_mode_t mode;
  int npts, i;

  if( !NF_FSTEP_AVAILABLE(fstep) )
    return out;

  npts = fpat.nrx * fpat.nry * fpat.nrz;
  if( npts <= 0 )
    return out;

  nf = &near_field_fstep[fstep];

  /* State drives the phase source; the resolver never mutates points */
  live  = animation_is_active();
  phase = live ? (double)flow_phase : 0.0;
  mode  = rc_config.nf_static_mode;
  fam   = color_tone_active();
  tone_param_init(&tp, fam);

  /* The live flag with flow_phase gates animation frames; nf_static_mode
   * gates the peak/snapshot toggle; a static frame carries phase 0, so a
   * static scrub at one step and mode hits the cache.  The tone parameter
   * and dB floor enter the edge so a gamma-slider change invalidates the
   * frame within the active family. */
  want = (color_edge_t){ .fstep = fstep, .chan = (int)chan, .fam = (int)fam,
      .proj = (int)mode, .flags = live ? 1u : 0u, .phase = phase,
      .param = tp.param, .flr = tp.floor_ratio,
      .freq_mhz = calc_data.freq_mhz, .gen_a = nf->content_generation,
      .palette = color_palette_generation() };

  if( nf_gate[chan].valid && color_edge_eq(&nf_gate[chan].edge, &want) )
  {
    out.vecs   = nf_vec_buf[chan];
    out.colors = nf_col_buf[chan];
    return out;
  }

  mem_array_realloc(&nf_vec_buf[chan], npts);
  mem_array_realloc(&nf_col_buf[chan], npts);
  mem_array_realloc(&nf_mag_buf[chan], npts);

  dr  = geom_pre.nf_dr_norm;
  max = 0.0;

  /* Geometry pass: real vector to direction·dr; scan the frame magnitude
   * maximum for the colorize pass. */
  for( i = 0; i < npts; i++ )
  {
    double mag, fscale, pv[3];

    if( chan == NF_CHAN_POV )
    {
      double e[3], h[3];

      /* Poynting composes E-real and H-real at the same phase source */
      nf_real_vector(&nf->points[i], NF_CHAN_E, live, phase, mode, e);
      nf_real_vector(&nf->points[i], NF_CHAN_H, live, phase, mode, h);
      mag = nf_poynting(e, h, &pv[0], &pv[1], &pv[2]);
    }
    else
      mag = nf_real_vector(&nf->points[i], chan, live, phase, mode, pv);

    fscale = dr / mag;
    nf_vec_buf[chan][i].dx = (float)(pv[0] * fscale);
    nf_vec_buf[chan][i].dy = (float)(pv[1] * fscale);
    nf_vec_buf[chan][i].dz = (float)(pv[2] * fscale);

    nf_mag_buf[chan][i] = mag;
    if( mag > max )
      max = mag;
  }

  /* Colorize pass: amplitude ramp of mag/max through the active tone */
  for( i = 0; i < npts; i++ )
    nf_col_buf[chan][i] = nf_ramp_color(fam, &tp, nf_mag_buf[chan][i], max);

  nf_gate[chan].edge  = want;
  nf_gate[chan].valid = TRUE;

  out.vecs   = nf_vec_buf[chan];
  out.colors = nf_col_buf[chan];
  return out;
}

/*-----------------------------------------------------------------------*/

  void
chroma_nf_free(void)
{
  int c;

  for( c = 0; c < NF_CHAN_NUM; c++ )
  {
    mem_array_free(&nf_vec_buf[c]);
    mem_array_free(&nf_col_buf[c]);
    mem_array_free(&nf_mag_buf[c]);
    nf_gate[c] = (nf_gate_t){ 0 };
  }
}

/*-----------------------------------------------------------------------*/
