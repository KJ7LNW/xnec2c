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
 * anim_phase: sole owner of animation phase, phase step, and liveness.
 *
 * The three scalars here are semantic presentation state.  Playback intent
 * and the timer resources driving it stay with the phase controls, so no
 * widget and no event source crosses this interface.
 */
#include "anim_phase.h"

/* Hold the phase the animated presentations resolve against, in radians.
 * Single precision keeps the width the accumulating tick advances. */
static float anim_phase;

/* Hold the radians one tick adds to the phase */
static double anim_phase_step;

/* Track whether the phase context is open; the dialog edges alone write it */
static gboolean anim_phase_context_open;

/**
 * anim_phase_open() - Open the animation phase context
 */
  void
anim_phase_open(void)
{
  anim_phase_context_open = TRUE;
}

/**
 * anim_phase_close() - Close the animation phase context
 */
  void
anim_phase_close(void)
{
  anim_phase_context_open = FALSE;
  anim_phase = 0.0f;
}

/**
 * anim_phase_set_step() - Set the phase one tick advances
 * @radians_per_tick: phase increment resolved from the playback cadence
 */
  void
anim_phase_set_step(double radians_per_tick)
{
  anim_phase_step = radians_per_tick;
}

/**
 * anim_phase_set() - Set the phase its caller resolved
 * @radians: phase to display
 */
  void
anim_phase_set(double radians)
{
  anim_phase = (float)radians;
}

/**
 * anim_phase_advance() - Advance the phase by one step
 */
  void
anim_phase_advance(void)
{
  anim_phase += (float)anim_phase_step;
  anim_phase -= fl_fge(anim_phase, (float)M_2PI + FL_EPS)
    ? (float)M_2PI : 0.0f;
}

/**
 * anim_phase_get() - Report the displayed phase
 *
 * Returns the phase in radians.
 */
  double
anim_phase_get(void)
{
  return (double)anim_phase;
}

/**
 * anim_phase_active() - Report whether the phase context is open
 *
 * Returns TRUE while the context is open, independent of playback.
 */
  gboolean
anim_phase_active(void)
{
  return anim_phase_context_open;
}
