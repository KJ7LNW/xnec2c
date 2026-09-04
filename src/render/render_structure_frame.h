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

#ifndef __RENDER_STRUCTURE_FRAME_H
#define __RENDER_STRUCTURE_FRAME_H 1

#include "render_dispatch.h"

/**
 * render_overlay_model_scale() - Resolve the effective overlay model scale
 * @fstep: frequency step index
 *
 * Folds the per-fstep prerender base scale and the interactive
 * rc_config.rdpattern_overlay_scale_adj into the single authoritative product
 * consumed by every engine.  Returns 1.0 when no far-field data exists.
 */
float render_overlay_model_scale(int fstep);

/**
 * render_overlay_excitation_offset() - Resolve the far-field excitation translation
 * @model_scale:    resolved overlay model scale for the fstep
 * @overlay_active: whether the structure overlay is shown
 * @ff:             receives the pattern-space offset in x,y,z and its length
 *                  off_len; all zeroed when no excitation translation applies
 *
 * The pattern draw and the fit fold share this one authoritative translation.
 */
void render_overlay_excitation_offset(float model_scale, gboolean overlay_active,
    ff_draw_params_t *ff);

/**
 * render_structure_frame() - Resolve one structure presentation frame
 * @fstep:       frequency step index
 * @model_scale: structure-to-presentation scale
 * @frame_phase: phase read once by the frame owner
 *
 * Returns the typed wire, patch, glyph, scale, and patch-flow inputs consumed
 * by either structure backend.
 */
struct_draw_params_t render_structure_frame(int fstep, float model_scale,
    double frame_phase);

#endif /* __RENDER_STRUCTURE_FRAME_H */
