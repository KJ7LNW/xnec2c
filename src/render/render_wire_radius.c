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
 * render_wire_radius: the user wire radius gesture every structure engine
 * offers.  The gesture writes rc_config.wire_radius_scale; render dispatch
 * reads it from the structure frame.
 */

#include "render_wire_radius.h"
#include "render_surface.h"
#include "../shared.h"
#include "../config_hooks.h"
#include "../gdk_scroll.h"
#include "../settings/render_settings.h"

/*-----------------------------------------------------------------------*/

/** wire_radius_scroll() - Ctrl+scroll handler for adjusting wire radius scale
 * @event: scroll event
 * @surface: surface of the scrolled view
 *
 * Reached through the wire-radius capability, which the structure and
 * rdpattern input rows both offer.
 */
  static gboolean
wire_radius_scroll(
    GdkEventScroll *event, render_surface_t *surface)
{
  const view_t *view;
  double scale, new_scale;
  scroll_step_t s;

  if( surface == NULL )
    return( FALSE );

  view = surface->view;
  s = scroll_step_from_deltas((GdkEvent *)event);

  if( !s.active ||
      (s.direction != GDK_SCROLL_UP && s.direction != GDK_SCROLL_DOWN) )
    return( FALSE );

  /* Compute increment matching zoom scroll feel */
  scale = compute_zoom_scale(view->width, view->height,
      (dl_fgt(rc_config.wire_radius_scale, 0.1) ?
        rc_config.wire_radius_scale : 0.1) * 100.0);

  new_scale = rc_config.wire_radius_scale;

  if( s.direction == GDK_SCROLL_UP )
  {
    /* Scroll up: thicker. If at zero, jump to threshold. */
    if( dl_flt(new_scale, WIRE_FORM_LINE_THRESHOLD) )
    {
      new_scale = WIRE_FORM_LINE_THRESHOLD;
    }
    else
    {
      new_scale *= (1.0 + 0.1 * s.step * scale);
    }
  }
  else if( s.direction == GDK_SCROLL_DOWN )
  {
    /* Scroll down: thinner. Below threshold snaps to zero (line mode). */
    new_scale /= (1.0 + 0.1 * s.step * scale);

    if( dl_flt(new_scale, WIRE_FORM_LINE_THRESHOLD) )
    {
      new_scale = 0.0;
    }
  }

  /* The registered post_apply hook owns the clamp and the repaint of both
   * windows */
  rc_config.wire_radius_scale = new_scale;
  hook_set_radius_scale();

  render_settings_sync_from_config();

  return( TRUE );

} /* wire_radius_scroll() */

/*-----------------------------------------------------------------------*/

/* Ctrl+scroll adjusts the wire radius of structure geometry, which the
 * radiation-pattern overlay presents alongside its own content */
surface_capability_t wire_radius_cap = {
  .handler  = wire_radius_scroll,
  .modifier = SURFACE_MOD_CTRL,
  .notice   = N_("Ctrl+Scroll: Wire Radius"),
  .subject  = SURFACE_CAP_SUBJECT_VIEW
};
