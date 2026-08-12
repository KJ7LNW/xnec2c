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
 * opengl_ops: Unified GL backend vtable for render_ops_t dispatch.
 *
 * Assembles leaf renderers from opengl_structure.c and opengl_rdpattern.c
 * into the single vtable consumed by render().  The axes leaf records the
 * extent render() prepared; the axes renderable draws from that record.
 */

#include "../shared.h"

#ifdef HAVE_OPENGL

#include "opengl_structure.h"
#include "opengl_rdpattern.h"
#include "../opengl-engine/opengl_view.h"

/**
 * gl_draw_axes() - Store parent-prepared axes state
 * @ctx: GL view state passed through render_ops_t
 * @extent: primary content extent
 */
  static void
gl_draw_axes(void *ctx, float extent)
{
  gl_view_state_t *state = ctx;

  state->content.axes.active = TRUE;
  state->content.axes.extent = extent;

} /* gl_draw_axes() */

/**
 * gl_set_colors() - Store the frame colors render() resolved
 * @ctx: GL view state passed through render_ops_t
 * @colors: frame colors of the active theme
 */
  static void
gl_set_colors(void *ctx, const render_frame_colors_t *colors)
{
  gl_view_state_t *state = ctx;

  state->content.background = colors->background;
  state->content.view_axis = colors->view_axis;
  state->content.view_axis_label = colors->view_axis_label;

} /* gl_set_colors() */

/* Unified GL backend vtable; render() gates slot calls by mode */
const render_ops_t gl_ops =
{
  .draw_farfield          = gl_rdpat_draw_farfield,
  .draw_nearfield         = gl_rdpat_draw_nearfield,
  .draw_structure         = gl_draw_structure,
  .draw_structure_overlay = gl_draw_structure_overlay,
  .draw_axes              = gl_draw_axes,
  .init_empty     = gl_view_init_empty,
  .set_status     = gl_view_set_status,
  .set_gradient   = gl_view_set_gradient,
  .set_colors     = gl_set_colors,
};

#endif /* HAVE_OPENGL */
