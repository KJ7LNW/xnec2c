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
 * structure_patch_anim: the surface patch animation class.
 *
 * Patches carry the patch current under two effects, colour and flow mark.
 * The charges view fills patches with the static geometry colour and draws
 * no flow, so this class is presented under the currents view alone.
 */
#include "structure_patch_anim.h"
#include "../chroma/chroma.h"
#include "../render/render_patch_flow.h"
#include "../shared.h"

/* Enumerate the effects the patch class folds into its phase answer */
typedef enum
{
  PATCH_EFFECT_COLOR = 0,
  PATCH_EFFECT_FLOW_MARK,
  PATCH_EFFECT_COUNT
} patch_effect_t;

static gboolean patch_color_varies(void);
static gboolean patch_flow_varies(void);

static gboolean structure_patch_anim_available(void);
static gboolean structure_patch_anim_presented(void);
static gboolean structure_patch_anim_varies(void);

/* Both effects apply whenever the class is presented, and each answers
 * phase dependence from the selection it reads. */
static const anim_member_row_t patch_effect_rows[] = {
  [PATCH_EFFECT_COLOR] = {
      .available = NULL, .varies_with_phase = patch_color_varies },
  [PATCH_EFFECT_FLOW_MARK] = {
      .available = NULL, .varies_with_phase = patch_flow_varies },
};

_Static_assert(G_N_ELEMENTS(patch_effect_rows) == PATCH_EFFECT_COUNT,
    "Patch effect rows must cover every effect");

const anim_class_ops_t structure_patch_anim_ops = {
  .available         = structure_patch_anim_available,
  .presented         = structure_patch_anim_presented,
  .varies_with_phase = structure_patch_anim_varies,
};

/**
 * patch_color_varies() - Report a colour selection that carries phase
 *
 * A projection row naming a channel patches lack falls back to the current
 * envelope, which changes the channel and never the encoding, so the one
 * projection classifier answers for patches as it does for segments.
 */
  static gboolean
patch_color_varies(void)
{
  return chroma_proj_animated(chroma_proj_selected());
}

/**
 * patch_flow_varies() - Report a flow selection that carries phase
 */
  static gboolean
patch_flow_varies(void)
{
  return patch_flow_mode_varies_with_phase(patch_flow_mode_active());
}

/**
 * structure_patch_anim_available() - Report surface patches in the model
 */
  static gboolean
structure_patch_anim_available(void)
{
  return data.m > 0;
}

/**
 * structure_patch_anim_presented() - Report a view showing patch currents
 */
  static gboolean
structure_patch_anim_presented(void)
{
  return struct_view_currents();
}

/**
 * structure_patch_anim_varies() - Fold the effect rows into one answer
 */
  static gboolean
structure_patch_anim_varies(void)
{
  return anim_member_rows_vary(patch_effect_rows,
      G_N_ELEMENTS(patch_effect_rows));
}
