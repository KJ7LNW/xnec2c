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

#ifndef PRESENTATION_CACHE_KEY_H
#define PRESENTATION_CACHE_KEY_H 1

#include <stdint.h>
#include <string.h>

/**
 * presentation_cache_key_t - Shared gain-scaling presentation inputs
 *
 * All inputs that determine how raw physics gain maps to scaled
 * radius, color, or legend values.  Embedded by every cache that
 * derives from gain/noise/polarization state.
 *
 * Gate via presentation_cache_key_match against a stored copy: adding
 * a new presentation input = add field here + set it in the builder.
 * All consumers invalidate automatically.
 *
 * Zero-init before populating to avoid padding comparison artifacts.
 */
typedef struct
{
  /* Doubles first to eliminate internal padding (C11 6.2.6.1p6) */
  double ant_temp_elevation;
  double ant_temp_custom_t_sky;
  double ant_temp_custom_t_earth;
  double freq_mhz;
  double max_gain;            /* rad_pattern[fstep].max_gain[pol] */
  double min_gain;            /* rad_pattern[fstep].min_gain[pol] */
  int    gain_style;
  int    pol_type;
  int    ant_temp_sky;
  int    ant_temp_earth;
  int    ant_temp_interp;
  uint32_t palette_gen;       /* color_palette_generation(); re-themes color caches */
} presentation_cache_key_t;

/**
 * presentation_cache_key_build() - Build key from current authoritative state
 * @fstep: frequency step index (indexes into rad_pattern)
 *
 * Reads rc_config, calc_data, and rad_pattern to populate all fields.
 * Returns a zero-initialized struct with only the relevant fields set,
 * safe for memcmp.
 */
presentation_cache_key_t presentation_cache_key_build(int fstep);

/**
 * presentation_cache_key_invalidate() - Return a sentinel key that never matches
 * @key: key to invalidate
 *
 * Fills the key with values guaranteed to mismatch any builder-produced key.
 * Uses the same zero-init + field-set path as the builder to avoid
 * leaking raw memory operations into consumers.
 */
static inline void
presentation_cache_key_invalidate(presentation_cache_key_t *key)
{
  presentation_cache_key_t invalid = {0};
  invalid.gain_style = -1;
  invalid.pol_type   = -1;
  *key = invalid;
}

/**
 * presentation_cache_key_match() - Compare two keys for equality
 * @a: first key
 * @b: second key
 *
 * Returns 1 when all fields match, 0 on any mismatch.
 * Uses memcmp over the full struct; the six trailing 4-byte members
 * fill the 8-byte alignment, leaving no tail pad.  Correctness relies on
 * compilers preserving padding through struct assignment; GCC and
 * clang do so (struct = compiles to memcpy of sizeof).  Both keys
 * must be zero-initialized via = {0} before populating.
 */
static inline int
presentation_cache_key_match(
    const presentation_cache_key_t *a,
    const presentation_cache_key_t *b)
{
  return memcmp(a, b, sizeof(*a)) == 0;
}

#endif /* PRESENTATION_CACHE_KEY_H */
