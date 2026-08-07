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

#ifndef OPENGL_VIEW_PEEL_H
#define OPENGL_VIEW_PEEL_H 1

#include "opengl_view.h"

#ifdef HAVE_OPENGL

/* gl_view_peel_recreate()
 *
 * Recreate depth-peel FBO resources at the given dimensions.
 * Creates 2 ping-pong FBOs with depth textures, a shared color
 * texture, and an accumulation FBO for the under-operator.
 * When msaa_samples > 0, also creates multisampled renderbuffer
 * FBOs for rasterization, resolved to single-sample textures
 * for shader reads and compositing.
 */
void gl_view_peel_recreate(gl_view_state_t *state, int width, int height,
    int msaa_samples);

/* gl_view_peel_free()
 *
 * Release depth-peel FBO, texture, and accumulation resources.
 * Zeroes the corresponding fields in state.
 */
void gl_view_peel_free(gl_view_state_t *state);

/* gl_view_peel_locs_init()
 *
 * Look up depth-peel uniform locations for a shader program.
 */
void gl_view_peel_locs_init(gl_peel_uniform_locs_t *locs, GLuint program);

/* gl_view_set_peel_uniforms()
 *
 * Set depth-peel uniforms for the current peel pass.
 */
void gl_view_set_peel_uniforms(const gl_peel_uniform_locs_t *locs,
    const gl_render_params_t *params);

/* gl_view_peel_render()
 *
 * Run depth-peel transparency passes for sorted transparent items.
 * Prepares each renderable, then executes PEEL_PASSES front-to-back
 * peel iterations with under-operator compositing and final blend
 * over the opaque framebuffer bound to active_fbo.
 */
void gl_view_peel_render(gl_view_state_t *state, GLuint active_fbo,
    gl_render_params_t render_params,
    const gl_trans_item_t *items, int count);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_VIEW_PEEL_H */
