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

#include "presentation_cache_key.h"
#include "../color/color_palette.h"
#include "../shared.h"

/**
 * presentation_cache_key_build() - Build key from current authoritative state
 * @fstep: frequency step index (indexes into rad_pattern)
 *
 * Single builder for all presentation cache consumers.  Adding a new
 * gain-scaling input = add field to presentation_cache_key_t and set
 * it here; all caches invalidate automatically via
 * presentation_cache_key_match.
 */
  presentation_cache_key_t
presentation_cache_key_build(int fstep)
{
  presentation_cache_key_t k = {0};

  k.gain_style         = rc_config.gain_style;
  k.pol_type           = calc_data.pol_type;
  k.ant_temp_sky       = rc_config.ant_temp_sky;
  k.ant_temp_earth     = rc_config.ant_temp_earth;
  k.ant_temp_interp        = rc_config.ant_temp_interp;
  k.ant_temp_elevation     = rc_config.ant_temp_elevation;
  k.ant_temp_custom_t_sky  = rc_config.ant_temp_custom_t_sky;
  k.ant_temp_custom_t_earth = rc_config.ant_temp_custom_t_earth;
  k.freq_mhz               = calc_data.freq_mhz;
  k.palette_gen            = color_palette_generation();

  /* max/min gain detect child-process data updates at same fstep.
   * When rad_pattern is unavailable, zeros from {0} init retained. */
  if( rad_pattern != NULL &&
      fstep >= 0 && fstep <= calc_data.steps_total &&
      rad_pattern[fstep].max_gain != NULL &&
      rad_pattern[fstep].min_gain != NULL )
  {
    k.max_gain = rad_pattern[fstep].max_gain[k.pol_type];
    k.min_gain = rad_pattern[fstep].min_gain[k.pol_type];
  }

  return k;
}
