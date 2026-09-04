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

#include "render_structure_frame.h"
#include "../anim/anim_class.h"
#include "../chroma/chroma.h"
#include "../prerender/prerender_farfield.h"
#include "../shared.h"

/**
 * render_overlay_model_scale() - Resolve the effective overlay model scale
 * @fstep: frequency step index
 *
 * Owns the derived product of the per-fstep prerender base scale and the
 * interactive scale adjustment.
 */
float
render_overlay_model_scale(int fstep)
{
  if( ff_pre == NULL || fstep < 0 )
    return 1.0f;

  return ff_pre[fstep].overlay_base_scale
      * (float)rc_config.rdpattern_overlay_scale_adj;
}

/*-----------------------------------------------------------------------*/

/**
 * render_overlay_excitation_offset() - Resolve the far-field excitation translation
 * @model_scale:    resolved overlay model scale for the frequency step
 * @overlay_active: whether the structure overlay is shown
 * @ff:             receives the pattern-space offset and its length
 *
 * Supplies one translation to the pattern draw and the fit fold.
 */
void
render_overlay_excitation_offset(float model_scale, gboolean overlay_active,
    ff_draw_params_t *ff)
{
  if( !overlay_active || !isFlagSet(ENABLE_EXCITN) )
  {
    ff->x = 0.0f;
    ff->y = 0.0f;
    ff->z = 0.0f;
    ff->off_len = 0.0f;
    return;
  }

  ff->x = (float)geom_pre.excitation_cx * model_scale;
  ff->y = (float)geom_pre.excitation_cy * model_scale;
  ff->z = (float)geom_pre.excitation_cz * model_scale;
  ff->off_len = sqrtf(ff->x * ff->x + ff->y * ff->y + ff->z * ff->z);
}

/*-----------------------------------------------------------------------*/

/**
 * render_structure_frame() - Resolve one structure presentation frame
 * @fstep:       frequency step index
 * @model_scale: structure-to-presentation scale
 * @frame_phase: phase read once by the frame owner
 *
 * Returns the typed wire, patch, glyph, scale, and patch-flow inputs consumed
 * by either structure backend.
 */
  struct_draw_params_t
render_structure_frame(int fstep, float model_scale, double frame_phase)
{
  struct_draw_params_t params;
  double segment_phase = anim_class_phase(ANIM_CLASS_STRUCTURE_SEGMENT,
      frame_phase);
  double patch_phase = anim_class_phase(ANIM_CLASS_STRUCTURE_PATCH,
      frame_phase);
  chroma_proj_t proj = chroma_proj_active();
  color_tone_t fam = color_tone_active();
  seg_scale_enc_t seg_scale_enc = seg_scale_enc_selected();

  params.patch_flow = render_patch_flow_idle();

  if( struct_view_currents() && CRNT_FSTEP_AVAILABLE(fstep) &&
      struct_colors != NULL )
  {
    params.wire_colors = chroma_proj_frame_wire(fstep, segment_phase,
        proj, fam, CHAN_CURRENT);
    params.wire_seg_scale = chroma_proj_frame_seg_scale(fstep,
        segment_phase, proj, seg_scale_enc, fam, CHAN_CURRENT);
    params.patch_colors = chroma_proj_frame_patch(fstep, patch_phase,
        proj, fam);
    params.wire_glyphs = chroma_proj_frame_wire_glyphs(fstep, proj, fam,
        CHAN_CURRENT);

    /* Capture once every projection producer of this frame has published,
     * because a producer publishing later would leave the captured token
     * behind the colors it describes */
    params.color_generation = chroma_proj_generation();
    params.patch_flow = render_patch_flow_frame(fstep, patch_phase,
        params.patch_colors, params.color_generation);
    params.cmax = fmax((double)struct_colors[fstep].wire_crnt_cmax,
        (double)struct_colors[fstep].patch_crnt_cmax);
  }
  else if( struct_view_charges() && CRNT_FSTEP_AVAILABLE(fstep) &&
      struct_colors != NULL )
  {
    /* Keep patches on the static geometry color because they carry no charge. */
    params.wire_colors = chroma_proj_frame_wire(fstep, segment_phase,
        proj, fam, CHAN_CHARGE);
    params.wire_seg_scale = chroma_proj_frame_seg_scale(fstep,
        segment_phase, proj, seg_scale_enc, fam, CHAN_CHARGE);
    params.patch_colors = patch_rgb;
    params.wire_glyphs = chroma_proj_frame_wire_glyphs(fstep, proj, fam,
        CHAN_CHARGE);
    params.cmax = (double)struct_colors[fstep].wire_chrg_cmax;
    params.color_generation = chroma_proj_generation();
  }
  else
  {
    params.wire_colors = seg_rgb;
    params.wire_seg_scale = chroma_proj_seg_scale_identity();
    params.patch_colors = patch_rgb;
    params.wire_glyphs = NULL;
    params.cmax = 0.0;
    params.color_generation = 0;
  }

  params.geometry_extent = (float)geom_pre.scene_radius;
  params.model_scale = model_scale;
  params.fstep = fstep;
  params.freq_mhz = calc_data.freq_mhz;

  return params;
}
