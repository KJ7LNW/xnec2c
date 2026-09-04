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

#ifndef ANIM_PHASE_H
#define ANIM_PHASE_H    1

#include "../common.h"

/**
 * anim_phase_open() - Open the animation phase context
 *
 * Openness is the whole liveness condition, so a stopped context still
 * presents its animated selections at the phase it displays.
 */
void anim_phase_open(void);

/**
 * anim_phase_close() - Close the animation phase context
 *
 * Returns the phase to its zero reference as the context closes.
 */
void anim_phase_close(void);

/**
 * anim_phase_set_step() - Set the phase one tick advances
 * @radians_per_tick: phase increment resolved from the playback cadence
 */
void anim_phase_set_step(double radians_per_tick);

/**
 * anim_phase_set() - Set the phase its caller resolved
 * @radians: phase to display
 */
void anim_phase_set(double radians);

/**
 * anim_phase_advance() - Advance the phase by one step
 *
 * Wraps at one cycle so the accumulating phase stays within its readout.
 */
void anim_phase_advance(void);

/**
 * anim_phase_get() - Report the displayed phase
 *
 * Returns the phase in radians.
 */
double anim_phase_get(void);

/**
 * anim_phase_active() - Report whether the phase context is open
 *
 * Returns TRUE while the context is open, independent of playback.
 */
gboolean anim_phase_active(void);

#endif /* ANIM_PHASE_H */
