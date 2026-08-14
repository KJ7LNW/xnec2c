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

#ifndef OPENGL_VIEW_NOTICE_H
#define OPENGL_VIEW_NOTICE_H 1

#include "common.h"

#ifdef HAVE_OPENGL
#include "opengl_view.h"

/* Milliseconds a transient notice holds before it fades */
#define GL_VIEW_NOTICE_HOLD_MS 2500

/* gl_view_show_notice()
 *
 * Display a notice message at the given position.
 * duration_ms > 0: hold then fade over 500ms.
 * duration_ms == 0: persistent (no fade), replaced by next call.
 */
void gl_view_show_notice(gl_view_state_t *state, const char *text,
    int duration_ms, gl_notice_position_t position);

/* gl_view_hide_notice()
 *
 * Deactivate the current notice and cancel any fade timer.
 */
void gl_view_hide_notice(gl_view_state_t *state);

/* gl_view_sync_status_notice()
 *
 * Synchronize notice state with the frame's status_message.
 * Persistent notices track status_message; transient notices are not
 * interrupted.  Called from the render loop before gl_view_render_notice.
 */
void gl_view_sync_status_notice(gl_view_state_t *state);

/* gl_view_render_notice()
 *
 * Render notice overlay. Called from the render loop when
 * state->notice_active is set.
 */
void gl_view_render_notice(gl_view_state_t *state, int surf_width, int surf_height);

#endif /* HAVE_OPENGL */
#endif /* OPENGL_VIEW_NOTICE_H */
