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

#ifndef NEAR_FIELD_ANIM_H
#define NEAR_FIELD_ANIM_H       1

#include "anim_class.h"
#include "../chroma/chroma_nearfield.h"
#include "../prerender/prerender_state.h"

/* Publish the near-field grid class, folding the independently toggled E, H,
 * and Poynting channels that each produce one vector set. */
extern const anim_class_ops_t near_field_anim_ops;

/**
 * near_field_anim_channel_active() - Report whether @channel draws
 * @channel: near-field channel
 *
 * Sole owner of the per-channel drawing condition, pairing the display
 * selection with the NE/NH cards supplying that channel's samples.
 */
gboolean near_field_anim_channel_active(nf_channel_t channel);

/**
 * nf_static_frame_mode() - Resolve the configured static baseline frame
 *
 * Sole owner of the peak and snapshot mapping, serving the consumers that
 * export the baseline whatever the phase context holds.
 */
nf_frame_mode_t nf_static_frame_mode(void);

/**
 * near_field_anim_frame_mode() - Resolve the frame every channel presents
 *
 * Sole site combining the phase context with the configured static
 * baseline: an open context resolves the instantaneous frame, a closed one
 * the selected baseline.
 */
nf_frame_mode_t near_field_anim_frame_mode(void);

#endif /* NEAR_FIELD_ANIM_H */
