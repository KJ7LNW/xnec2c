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
 * The caller resolves the frame and the phase it stands at, so this resolver
 * composes the vectors without reading state or mutating the model.
 */
#include "chroma_nearfield.h"
#include "../color/color_edge.h"
#include "../color/color_palette.h"
#include "../color/color_tone.h"
#include "../shared.h"

/* Per-channel resolved entry buffers and their input-edge gates, indexed by
 * nf_channel_t. */
static field_vector_entry_t *nf_entry_buf[NF_CHAN_NUM];

typedef struct
{
  color_edge_t edge;
  gboolean     valid;
} nf_gate_t;

static nf_gate_t nf_gate[NF_CHAN_NUM];

/*-----------------------------------------------------------------------*/

/** nf_frame_phase() - Resolve the phase a frame mode evaluates at
 * @mode:  frame the vector resolves
 * @phase: animation phase in radians
 *
 * The instantaneous frame alone reads phase; both static baselines stand at
 * their zero reference.
 */
  static double
nf_frame_phase(nf_frame_mode_t mode, double phase)
{
  return (mode == NF_FRAME_INSTANT) ? phase : 0.0;
}

/*-----------------------------------------------------------------------*/

  double
nf_real_vector(const near_field_point_t *p, nf_channel_t chan,
    nf_frame_mode_t mode, double phase, double out[3])
{
  /* Initialize output and phasor scratch because BUG() reports without aborting. */
  double amp[3] = { 0.0 }, ph[3] = { 0.0 }, mag = 0.0, r;
  double eval = nf_frame_phase(mode, phase);

  out[0] = 0.0;
  out[1] = 0.0;
  out[2] = 0.0;

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

  switch( mode )
  {
    case NF_FRAME_INSTANT:
    case NF_FRAME_SNAPSHOT:
      /* Instantaneous real vector; the snapshot stands at phase zero */
      out[0] = amp[0] * cos(eval + ph[0]);
      out[1] = amp[1] * cos(eval + ph[1]);
      out[2] = amp[2] * cos(eval + ph[2]);
      mag = sqrt(out[0]*out[0] + out[1]*out[1] + out[2]*out[2]);
      break;

    case NF_FRAME_PEAK:
      /* Optimal-phase peak envelope */
      Nf_Peak_Vector(amp[0], amp[1], amp[2], ph[0], ph[1], ph[2],
          &out[0], &out[1], &out[2], &r);
      mag = r;
      break;

    case NF_FRAME_COUNT:
      BUG("Near-field frame %d names no resolvable form\n", mode);
      break;
  }

  return mag;
}

/*-----------------------------------------------------------------------*/

  field_vector_set_t
chroma_proj_frame_nearfield(int fstep, nf_channel_t chan,
    nf_frame_mode_t mode, double frame_phase)
{
  field_vector_set_t out = { NULL, 0.0 };
  near_field_t *nf;
  color_tone_t fam;
  tone_param_t tp;
  color_edge_t want;
  double phase, max;
  int npts, i;

  if( !NF_FSTEP_AVAILABLE(fstep) )
    return out;

  npts = fpat.nrx * fpat.nry * fpat.nrz;
  if( npts <= 0 )
    return out;

  nf = &near_field_fstep[fstep];

  /* Every vector is scaled into this bound, so it is the frame's extent */
  out.extent = geom_pre.nf_dr_norm;

  /* Resolve static snapshots at zero without deriving liveness from phase. */
  phase = nf_frame_phase(mode, frame_phase);
  fam   = color_tone_active();
  tone_param_init(&tp, fam);

  /* Gate animation frames with the resolved frame mode and phase; a static
   * frame carries phase zero, so a repeated step and mode hits the cache.
   * Include the tone parameter and dB floor so a slider change invalidates
   * the frame within the active family. */
  want = (color_edge_t){ .fstep = fstep, .chan = (int)chan, .fam = (int)fam,
      .proj = (int)mode, .phase = phase,
      .param = tp.param, .flr = tp.floor_ratio,
      .freq_mhz = calc_data.freq_mhz, .gen_a = nf->content_generation,
      .palette = color_palette_generation() };

  if( nf_gate[chan].valid && color_edge_eq(&nf_gate[chan].edge, &want) )
  {
    out.entries = nf_entry_buf[chan];
    return out;
  }

  mem_array_realloc(&nf_entry_buf[chan], npts);

  max = 0.0;

  /* Geometry pass: the sample position carrying its real vector scaled by the
   * frame extent; scan the frame magnitude maximum for the colorize pass. */
  for( i = 0; i < npts; i++ )
  {
    field_vector_entry_t *entry = &nf_entry_buf[chan][i];
    double px = nf->points[i].px;
    double py = nf->points[i].py;
    double pz = nf->points[i].pz;
    double mag, fscale, pv[3];

    if( chan == NF_CHAN_POV )
    {
      double e[3], h[3];

      /* Poynting composes E-real and H-real at the same phase source */
      nf_real_vector(&nf->points[i], NF_CHAN_E, mode, phase, e);
      nf_real_vector(&nf->points[i], NF_CHAN_H, mode, phase, h);
      mag = nf_poynting(e, h, &pv[0], &pv[1], &pv[2]);
    }
    else
      mag = nf_real_vector(&nf->points[i], chan, mode, phase, pv);

    /* The sample grid is the drawn origin, so one point type reaches the
     * vector capability from either domain */
    entry->origin.x = px;
    entry->origin.y = py;
    entry->origin.z = pz;
    entry->origin.r = sqrt(px * px + py * py + pz * pz);

    fscale = out.extent / mag;
    entry->vector.dx = (float)(pv[0] * fscale);
    entry->vector.dy = (float)(pv[1] * fscale);
    entry->vector.dz = (float)(pv[2] * fscale);
    entry->magnitude = mag;

    if( mag > max )
      max = mag;
  }

  /* Colorize pass: amplitude ramp of mag/max through the active tone */
  for( i = 0; i < npts; i++ )
    nf_entry_buf[chan][i].color =
      field_ramp_color(fam, &tp, nf_entry_buf[chan][i].magnitude, max);

  nf_gate[chan].edge  = want;
  nf_gate[chan].valid = TRUE;

  out.entries = nf_entry_buf[chan];
  return out;
}

/*-----------------------------------------------------------------------*/

  void
chroma_nf_free(void)
{
  int c;

  for( c = 0; c < NF_CHAN_NUM; c++ )
  {
    mem_array_free(&nf_entry_buf[c]);
    nf_gate[c] = (nf_gate_t){ 0 };
  }
}

/*-----------------------------------------------------------------------*/
