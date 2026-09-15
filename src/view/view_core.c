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
 * view_core: lifecycle, viewport, change notification, and sharing for view_t.
 *
 * Rotation math and angle API live in view_angles.c; drag and pan in
 * view_drag.c; GTK spin-widget readout and Cairo projection in view_gtk.c.
 * All four translation units share view_core.h as the single header.
 */
#include "view_core.h"

#include "../common.h"
#include "../console.h"

/*-----------------------------------------------------------------------
 * Lifecycle
 *----------------------------------------------------------------------*/

/**
 * view_new() - Allocate and initialize a view
 * @type:         drawing-area identity
 * @rotate_spin:  borrowed azimuth spin button
 * @incline_spin: borrowed elevation spin button
 * @zoom_spin:    borrowed zoom spin button
 * @changed_cb:   callback fired on every state change; NULL permitted
 * @changed_user: opaque pointer passed to @changed_cb
 */
  view_t *
view_new(view_type_t type,
         GtkSpinButton *rotate_spin,
         GtkSpinButton *incline_spin,
         GtkSpinButton *zoom_spin,
         view_observer_fn changed_cb,
         gpointer changed_user)
{
  view_t *v = NULL;

  mem_new(&v);

  glm_mat4_identity(v->R);
  v->rotation_master   = NULL;
  v->rotation_follower = NULL;

  glm_vec2_zero(v->pan_offset);
  v->zoom           = 1.0f;
  v->width          = 0;
  v->height         = 0;
  v->type           = type;

  v->last_ptr_x     = 0.0f;
  v->last_ptr_y     = 0.0f;
  v->drag_button    = VIEW_DRAG_NONE;
  v->drag_mode      = VIEW_DRAG_CONSTRAINED;
  v->motion_divisor = (float)VIEW_DRAG_DIVISOR;
  v->drag_wr_deg    = VIEW_DEFAULT_WR;
  v->drag_wi_deg    = VIEW_DEFAULT_WI;

  v->changed_cb     = changed_cb;
  v->changed_user   = changed_user;
  v->in_notify      = FALSE;

  v->rotate_spin    = rotate_spin;
  v->incline_spin   = incline_spin;
  v->zoom_spin      = zoom_spin;

  v->rotate_spin_handler  = NULL;
  v->incline_spin_handler = NULL;
  v->zoom_spin_handler    = NULL;

  Build_View_Rotation_Matrix(v->R, VIEW_DEFAULT_WR, VIEW_DEFAULT_WI);

  return( v );

} /* view_new() */

/** view_free() - Release a view and clear caller's pointer */
  void
view_free(view_t **vp)
{
  if( vp == NULL )
    return;

  mem_free(vp);

} /* view_free() */

/*-----------------------------------------------------------------------
 * Viewport and extent
 *----------------------------------------------------------------------*/

/** view_set_viewport() - Record drawable size and notify observers */
  void
view_set_viewport(view_t *v, int width, int height)
{
  v->width  = width;
  v->height = height;
  view_notify_change(v);

} /* view_set_viewport() */

/** view_set_zoom() - Set zoom factor (1.0 == fit-to-viewport)
 *
 * No-op when the new value matches the current zoom to avoid
 * observer storms from per-frame scene generators that push the
 * spin-widget value back into view_t every render.
 */
  void
view_set_zoom(view_t *v, float zoom)
{
  if( fabsf(v->zoom - zoom) < VIEW_ZOOM_EPS )
    return;

  v->zoom = zoom;
  view_notify_change(v);

} /* view_set_zoom() */

/*-----------------------------------------------------------------------
 * Change notification
 *----------------------------------------------------------------------*/

/**
 * view_notify_change() - Invoke the bound callback and propagate to follower
 *
 * Two-layer re-entry protection: the in_notify flag blocks nested
 * dispatcher entry (for example when a follower callback mutates the
 * master and re-enters here via the rotation_follower link), and the
 * per-mutator idempotence guards in view_set_angles() / view_set_zoom()
 * (VIEW_ANGLE_EPS, VIEW_ZOOM_EPS) ensure the feedback loop converges.
 * When rotation_follower is non-NULL a recursive call fires the
 * follower's own callback after the master's; own callback fires
 * first, follower propagation second.
 */
  void
view_notify_change(view_t *v)
{
  if( v == NULL || v->in_notify )
    return;

  v->in_notify = TRUE;

  if( v->changed_cb != NULL )
    v->changed_cb(v, v->changed_user);

  /* Propagate drag accumulators to the follower so
   * view_update_spin_display reads exact double-precision angles
   * rather than a lossy float matrix roundtrip. */
  view_sync_drag_to_follower(v);
  if( v->rotation_follower != NULL )
    view_notify_change(v->rotation_follower);

  v->in_notify = FALSE;

} /* view_notify_change() */

/*-----------------------------------------------------------------------
 * Sharing
 *----------------------------------------------------------------------*/

/**
 * view_share_master() - Follower borrows master's rotation matrix
 *
 * While shared, view_R(follower) returns master->R.  The master's
 * rotation_follower link is set so a master-side change propagates
 * into the follower's changed_cb.  See view_unshare_master() for
 * release.
 */
  void
view_share_master(view_t *follower, view_t *master)
{
  if( follower == NULL || master == NULL || follower == master )
    return;

  if( follower->rotation_master == master )
    return;

  if( follower->rotation_master != NULL )
    view_unshare_master(follower);

  follower->rotation_master = master;
  master->rotation_follower = follower;

  /* Notify the master so direction-2 propagation in view_notify_change
   * seeds the follower's drag accumulators from the master's
   * authoritative values before firing the follower's changed_cb. */
  view_notify_change(master);

} /* view_share_master() */

/**
 * view_unshare_master() - Follower reverts to its own rotation
 *
 * Clears the rotation_follower link on the master and the
 * rotation_master pointer on the follower.
 */
  void
view_unshare_master(view_t *follower)
{
  view_t *master;
  double wr, wi;

  if( follower == NULL || follower->rotation_master == NULL )
    return;

  master = follower->rotation_master;

  /* Copy master rotation into follower's own store so view_R(follower)
   * returns an up-to-date matrix after the master pointer is cleared.
   * Drag accumulators
   * are reseeded from the extracted angles so a constrained drag
   * started immediately afterwards is continuous; NAN azimuth at the
   * pole substitutes zero. */
  glm_mat4_copy(master->R, follower->R);
  Extract_View_Angles(follower->R, &wr, &wi);
  if( isnan(wr) )
    wr = 0.0;
  follower->drag_wr_deg = wr;
  follower->drag_wi_deg = wi;

  master->rotation_follower = NULL;
  follower->rotation_master = NULL;
  view_notify_change(follower);

} /* view_unshare_master() */

/*-----------------------------------------------------------------------*/
