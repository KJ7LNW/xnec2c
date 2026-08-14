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

#include "opengl_msaa.h"
#include "opengl_rdpattern.h"
#include "opengl_structure.h"
#include "../opengl-engine/opengl_view.h"
#include "../opengl-engine/opengl_view_fit.h"
#include "../opengl-engine/opengl_view_msaa.h"
#include "../opengl-engine/opengl_view_peel.h"
#include "../render/render_canvas.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** msaa_update_view() - Recreate MSAA framebuffer resources for one GL view and queue redraw
 * @surface: GL surface of the target view, or NULL when that canvas has none
 * @samples: new MSAA sample count
 */
  static void
msaa_update_view(render_surface_t *surface, int samples)
{
  gl_view_state_t *state;

  if( !surface )
    return;

  state = gl_view_state(surface);

  gtk_gl_area_make_current(GTK_GL_AREA(state->gl_area));
  gl_view_recreate_msaa(state, samples);

  /* Rebuild peel FBOs with matching MSAA sample count so
   * transparent geometry rasterizes with multisampling */
  if( state->composite_program )
    gl_view_peel_recreate(state, state->msaa_width, state->msaa_height,
        state->msaa_samples);

  gl_view_queue_render(state);

} /* msaa_update_view() */

/*-----------------------------------------------------------------------*/

/** Set_MSAA_Samples() - Change MSAA sample count for all GL views
 * @samples: new sample count (0, 2, 4, 8, or 16)
 */
  void
Set_MSAA_Samples(int samples)
{
  rc_config.opengl_msaa_samples = samples;

  msaa_update_view(canvas_surface_of(CANVAS_STRUCTURE, &gl_engine), samples);
  msaa_update_view(canvas_surface_of(CANVAS_RDPATTERN, &gl_engine), samples);

} /* Set_MSAA_Samples() */

#endif /* HAVE_OPENGL */
