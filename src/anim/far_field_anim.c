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
 * far_field_anim: the far-zone pattern surface animation class.
 *
 * The far zone draws one vector set whose quantity is an exclusive electric
 * or magnetic selection, so it holds no member table and answers its three
 * class questions directly.
 */
#include "far_field_anim.h"
#include "../shared.h"

static gboolean far_field_anim_available(void);
static gboolean far_field_anim_presented(void);
static gboolean far_field_anim_varies(void);

const anim_class_ops_t far_field_anim_ops = {
  .available         = far_field_anim_available,
  .presented         = far_field_anim_presented,
  .varies_with_phase = far_field_anim_varies,
};

/**
 * far_field_anim_available() - Report radiation pattern content in the model
 */
  static gboolean
far_field_anim_available(void)
{
  return isFlagSet(ENABLE_RDPAT) != 0;
}

/**
 * far_field_anim_presented() - Report drawn far-zone vectors
 *
 * The vectors attach to the gain surface, so the pattern window must show
 * that surface and select the far-zone overlay on it.
 */
  static gboolean
far_field_anim_presented(void)
{
  return rdpat_gain_active() && (rc_config.overlay_farfield != 0);
}

/**
 * far_field_anim_varies() - Report the far-zone vectors carrying phase
 *
 * The vectors resolve from the stored phasor pair at the frame phase, so
 * they carry phase whenever they are drawn.
 */
  static gboolean
far_field_anim_varies(void)
{
  return TRUE;
}
