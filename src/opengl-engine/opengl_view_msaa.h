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

#ifndef OPENGL_VIEW_MSAA_H
#define OPENGL_VIEW_MSAA_H 1

#include "opengl_view.h"

#ifdef HAVE_OPENGL

/* gl_view_scene_recreate()
 *
 * Recreate scene framebuffer resources without destroying GL widget.
 * A request below 2 samples allocates single-sample scene storage.
 */
void gl_view_scene_recreate(gl_view_state_t *state, int requested_samples);

/* gl_view_scene_free()
 *
 * Release scene framebuffer, color, and depth renderbuffer resources.
 * Zeroes the corresponding fields in state.
 */
void gl_view_scene_free(gl_view_state_t *state);

/* gl_view_scene_resolve()
 *
 * Resolve the scene color buffer into the single-sample resolve framebuffer
 * that capture reads and presentation copies.
 */
void gl_view_scene_resolve(gl_view_state_t *state);

/* gl_view_scene_present()
 *
 * Resolve the scene color buffer to a single-sample texture, then
 * draw that texture into default_fbo.  Splits the multisample resolve from
 * the write into GTK's framebuffer because a direct multisample blit into
 * default_fbo is rejected under Wayland/EGL.
 */
void gl_view_scene_present(gl_view_state_t *state, GLint default_fbo);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_VIEW_MSAA_H */
