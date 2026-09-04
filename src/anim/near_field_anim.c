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
 * near_field_anim: the near-field sample grid animation class.
 *
 * The three channels are independently toggled and each publishes one vector
 * set, so the rows here carry availability alone.  Every channel resolves
 * through the one frame the class selects, so the class answers phase
 * dependence once rather than repeating it per channel.
 */
#include "near_field_anim.h"
#include "anim_phase.h"
#include "../shared.h"

/* Answer availability for one near-field channel; phase dependence belongs
 * to the class, which selects one frame for every channel. */
typedef struct
{
  gboolean (*available)(void);
} nf_channel_row_t;

static gboolean nf_e_available(void);
static gboolean nf_h_available(void);
static gboolean nf_poynting_available(void);

static gboolean near_field_anim_available(void);
static gboolean near_field_anim_presented(void);
static gboolean near_field_anim_varies(void);

/* Each channel pairs its display selection with the NE/NH cards whose
 * samples it composes; Poynting composes both. */
static const nf_channel_row_t nf_channel_rows[] = {
  [NF_CHAN_E]   = { .available = nf_e_available },
  [NF_CHAN_H]   = { .available = nf_h_available },
  [NF_CHAN_POV] = { .available = nf_poynting_available },
};

_Static_assert(G_N_ELEMENTS(nf_channel_rows) == NF_CHAN_NUM,
    "Near-field channel rows must cover every channel");

const anim_class_ops_t near_field_anim_ops = {
  .available         = near_field_anim_available,
  .presented         = near_field_anim_presented,
  .varies_with_phase = near_field_anim_varies,
};

/**
 * nf_e_available() - Report a drawn electric field channel
 */
  static gboolean
nf_e_available(void)
{
  return draw_efield_active() && ((fpat.nfeh & NEAR_EFIELD) != 0);
}

/**
 * nf_h_available() - Report a drawn magnetic field channel
 */
  static gboolean
nf_h_available(void)
{
  return draw_hfield_active() && ((fpat.nfeh & NEAR_HFIELD) != 0);
}

/**
 * nf_poynting_available() - Report a drawn Poynting vector channel
 */
  static gboolean
nf_poynting_available(void)
{
  return draw_poynting_active() && ((fpat.nfeh & NEAR_EFIELD) != 0)
      && ((fpat.nfeh & NEAR_HFIELD) != 0);
}

/**
 * near_field_anim_channel_active() - Report whether @channel draws
 * @channel: near-field channel
 */
  gboolean
near_field_anim_channel_active(nf_channel_t channel)
{
  if( channel < NF_CHAN_E || channel >= NF_CHAN_NUM )
  {
    BUG("Near-field channel %d lies outside [0, %d)", channel, NF_CHAN_NUM);
    return FALSE;
  }

  return nf_channel_rows[channel].available();
}

/** nf_static_frame_mode() - Resolve the configured static baseline frame
 */
  nf_frame_mode_t
nf_static_frame_mode(void)
{
  return (rc_config.nf_static_mode == NF_STATIC_SNAPSHOT)
      ? NF_FRAME_SNAPSHOT : NF_FRAME_PEAK;
}

/**
 * near_field_anim_frame_mode() - Resolve the frame every channel presents
 */
  nf_frame_mode_t
near_field_anim_frame_mode(void)
{
  return anim_phase_active() ? NF_FRAME_INSTANT : nf_static_frame_mode();
}

/**
 * near_field_anim_available() - Report near-field content in the model
 */
  static gboolean
near_field_anim_available(void)
{
  return isFlagSet(ENABLE_NEAREH) != 0;
}

/**
 * near_field_anim_presented() - Report a drawn near-field channel
 */
  static gboolean
near_field_anim_presented(void)
{
  gboolean presented = FALSE;
  nf_channel_t channel;

  for( channel = NF_CHAN_E; channel < NF_CHAN_NUM && !presented; channel++ )
    presented = nf_channel_rows[channel].available();

  return rdpat_ehfield_active() && presented;
}

/**
 * near_field_anim_varies() - Report the near-field frame carrying phase
 *
 * An open phase context resolves every channel at the instantaneous frame,
 * so the class carries phase whenever it is presented.
 */
  static gboolean
near_field_anim_varies(void)
{
  return TRUE;
}
