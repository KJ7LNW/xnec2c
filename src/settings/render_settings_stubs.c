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
 * render_settings_stubs: non-OpenGL stubs for OpenGL-specific settings
 * symbols that the rc_config_vars rows and reset path reference
 * unconditionally.
 *
 * When HAVE_OPENGL is defined, the real implementations in
 * render_settings_opengl.c provide these symbols instead.
 */

#include "../shared.h"

#ifndef HAVE_OPENGL

#include "render_settings.h"
#include "render_settings_common.h"
#include "render_settings_internal.h"

/*------------------------------------------------------------------------*/

/* Empty OpenGL tab: the entire tab is OpenGL-specific */
void *const opengl_tab_fields[] = {
  NULL,
};

/*------------------------------------------------------------------------*/

/* OpenGL change-edge hooks bound by rc_config_vars rows: no-ops without
 * OpenGL, so the rows link in a Cairo-only build. */

void
hook_set_msaa(void)
{
}

const config_refresh_t hook_set_msaa_refresh =
  { .fn = hook_set_msaa, .cls = REFRESH_HOVER_SAFE };

void
hook_set_radius_scale(void)
{
}

const config_refresh_t hook_set_radius_scale_refresh =
  { .fn = hook_set_radius_scale, .cls = REFRESH_HOVER_SAFE };

/*------------------------------------------------------------------------*/

/* OpenGL tab reset handler: no-op without OpenGL */

void
on_opengl_tab_reset_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
}

#endif /* !HAVE_OPENGL */
