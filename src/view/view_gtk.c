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
 * view_gtk: GTK spin-widget readout and Cairo screen-projection for view_t.
 *
 * view_update_spin_display() writes WR/WI back to the bound GtkSpinButton
 * widgets with signal blocking to prevent feedback.  Project_on_Screen()
 * and Set_Gdk_Segment() are the Cairo rendering consumers.
 */
#include <string.h>

#include "view_core.h"

#include "../callbacks.h"

/*-----------------------------------------------------------------------
 * Helpers
 *----------------------------------------------------------------------*/

/**
 * spin_write_blocked() - Write a spin button value with handler signal blocked
 * @spin:    spin button to update
 * @handler: value-changed handler to block; NULL skips block/unblock
 * @value:   value to set
 */
  static void
spin_write_blocked(GtkSpinButton *spin, GCallback handler, double value)
{
  if( spin == NULL )
    return;

  if( handler != NULL )
    SIGNAL_BLOCK(spin, handler);

  gtk_spin_button_set_value(spin, value);

  if( handler != NULL )
    SIGNAL_UNBLOCK(spin, handler);

} /* spin_write_blocked() */

/*-----------------------------------------------------------------------
 * UI readout
 *----------------------------------------------------------------------*/

/**
 * view_update_spin_display() - Write WR/WI values into bound spin widgets
 *
 * Uses gtk_spin_button_set_value() so the widget's numeric value and
 * displayed text stay synchronised.  The bound value-changed handlers
 * (registered via view_set_spin_handlers()) are blocked across the
 * write so the programmatic update does not feed back as a
 * value-changed callback.  At the pole (wr==NAN) the azimuth entry is
 * left untouched so the last valid value stays visible.
 *
 * In constrained drag mode the drag-session accumulators (drag_wr_deg,
 * drag_wi_deg) are used directly so the spinner tracks continuously
 * through the full circle without the asin() fold-back at ±90°.  In
 * free drag mode Extract_View_Angles() is used since the accumulators
 * are not updated during axis-angle pre-multiply.
 */
  void
view_update_spin_display(view_t *v)
{
  double wr, wi;

  if( v == NULL )
    return;

  if( v->drag_mode == VIEW_DRAG_CONSTRAINED )
  {
    wr = v->drag_wr_deg;
    wi = v->drag_wi_deg;
  }
  else
  {
    view_get_angles(v, &wr, &wi);
  }

  /* Normalize to canonical display ranges: WR in [0, 360), WI in [-180, 180).
   * Both accumulators and matrix decomposition produce unbounded values;
   * fmod brings them into one period before the sign adjustment. */
  if( !isnan(wr) )
  {
    wr = fmod(wr, 360.0);
    if( wr < 0.0 )
      wr += 360.0;
  }

  wi = fmod(wi, 360.0);
  if( wi < -180.0 )
    wi += 360.0;
  else if( wi >= 180.0 )
    wi -= 360.0;

  if( v->rotate_spin != NULL && !isnan(wr) )
    spin_write_blocked(v->rotate_spin, v->rotate_spin_handler, wr);

  if( v->incline_spin != NULL )
    spin_write_blocked(v->incline_spin, v->incline_spin_handler, wi);

} /* view_update_spin_display() */

/**
 * view_set_spin_handlers() - Bind spin-handler callback pointers
 * @v:          view receiving the borrowed handlers
 * @rotate_cb:  azimuth value-changed handler
 * @incline_cb: elevation value-changed handler
 *
 * Pointers are borrowed; view_t does not take ownership.  Handlers
 * may be set independently; NULL entries skip signal blocking.
 */
  void
view_set_spin_handlers(view_t *v, GCallback rotate_cb, GCallback incline_cb)
{
  if( v == NULL )
    return;

  v->rotate_spin_handler  = rotate_cb;
  v->incline_spin_handler = incline_cb;

} /* view_set_spin_handlers() */

/**
 * view_apply_fit() - Apply fitted zoom and pan as one view transition
 * @v:   view receiving the fitted state
 * @fit: fitted zoom and screen-space pan
 *
 * Synchronizes the zoom widget, then notifies observers once when the
 * authoritative view state changes.
 */
  void
view_apply_fit(view_t *v, const view_fit_t *fit)
{
  gboolean changed;

  if( v == NULL || fit == NULL )
    return;

  changed = !fl_feq(v->zoom, fit->zoom)
      || !fl_feq(v->pan_offset[0], fit->pan_offset[0])
      || !fl_feq(v->pan_offset[1], fit->pan_offset[1]);

  v->zoom = fit->zoom;
  memcpy(v->pan_offset, fit->pan_offset, sizeof(v->pan_offset));

  if( v->zoom_spin != NULL )
    gtk_spin_button_set_value(v->zoom_spin, (double)fit->zoom * 100.0);

  if( changed )
    view_notify_change(v);

} /* view_apply_fit() */

/*-----------------------------------------------------------------------
 * Cairo consumers
 *----------------------------------------------------------------------*/

/**
 * Project_on_Screen() - Project (x,y,z) via the current rotation matrix
 * @v:  view containing the rotation matrix
 * @x:  world-space X coordinate
 * @y:  world-space Y coordinate
 * @z:  world-space Z coordinate
 * @xs: output screen-space X coordinate
 * @ys: output screen-space Y coordinate
 * @zs: non-NULL output; receives camera-axis depth
 *
 * Reads rows 0 and 1 of view_R(v).  The canonical rotation matrix
 * produced by Build_View_Rotation_Matrix() places the screen-horizontal
 * axis in row 0 and screen-vertical axis in row 1; row 2 is the
 * camera-direction axis consumed by view_get_theta_phi().
 */
  void
Project_on_Screen(view_t *v,
                  double x, double y, double z,
                  double *xs, double *ys,
                  float *zs)
{
  float (*R)[4] = view_R(v);

  *xs = x * (double)R[0][0] + y * (double)R[1][0] + z * (double)R[2][0];
  *ys = x * (double)R[0][1] + y * (double)R[1][1] + z * (double)R[2][1];
  *zs = (float)(x * (double)R[0][2] + y * (double)R[1][2] + z * (double)R[2][2]);

} /* Project_on_Screen() */

/**
 * Set_Gdk_Segment() - Project a line in xyz frame to screen pixel segment
 * @segm:      output screen-space segment
 * @v:         view for projection parameters
 * @scale:     world-to-pixel scale factor
 * @x1..z1:   first endpoint in world space
 * @x2..z2:   second endpoint in world space
 * @z_mid_out: output midpoint camera-axis depth (required)
 *
 * Applies caller-provided scale, view_x_center(), view_y_center() and the
 * pan offset to the two projected endpoints.  Y axis is flipped because
 * Cairo uses top-down coordinates.  Also computes the camera-axis
 * (column 2 of R) midpoint depth for depth sorting.
 */
  void
Set_Gdk_Segment(Segment_t *segm, view_t *v, double scale,
                double x1, double y1, double z1,
                double x2, double y2, double z2,
                float *z_mid_out)
{
  double x, y;
  double xc    = view_x_center(v);
  double yc    = view_y_center(v);
  double px    = (double)v->pan_offset[0];
  double py    = (double)v->pan_offset[1];
  float zp1, zp2;

  Project_on_Screen(v, x1, y1, z1, &x, &y, &zp1);
  segm->x1 = (gint)(xc + px + x * scale);
  segm->y1 = v->height - (gint)(yc + py + y * scale);

  Project_on_Screen(v, x2, y2, z2, &x, &y, &zp2);
  segm->x2 = (gint)(xc + px + x * scale);
  segm->y2 = v->height - (gint)(yc + py + y * scale);

  /* Scale-normalize depth so segments from different coordinate frames
   * (antenna-meters vs gain-pattern units) sort correctly together. */
  *z_mid_out = 0.5f * (zp1 + zp2) * (float)scale;

} /* Set_Gdk_Segment() */

/*-----------------------------------------------------------------------*/
