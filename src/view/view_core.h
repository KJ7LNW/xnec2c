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

#ifndef VIEW_CORE_H
#define VIEW_CORE_H 1

#include <gtk/gtk.h>
#include <math.h>

#include "../opengl-engine/opengl_math.h"
#include "view_presets.h"

/*
 * view_t is the single owner of per-viewport rotation, pan, zoom,
 * extent, and drag-session state.  Cairo and OpenGL renderers borrow
 * a view_t pointer and read derived quantities on demand.  No datum
 * carried by view_t is mirrored in any renderer-specific structure.
 */

/* view_type_t - which drawing area the view drives. */
typedef enum
{
  VIEW_STRUCTURE = 0,
  VIEW_RDPATTERN = 1

} view_type_t;

/* drag_mode_t - rotation math used by view_apply_drag() */
typedef enum
{
  VIEW_DRAG_CONSTRAINED = 0,   /* decompose R, update WR/WI, rebuild R */
  VIEW_DRAG_FREE        = 1    /* axis-angle pre-multiply */

} drag_mode_t;

/* drag_button_t - active drag button; matches GDK button numbering
 * where 1 == primary.  VIEW_DRAG_NONE means no drag in progress. */
typedef enum
{
  VIEW_DRAG_NONE   = 0,
  VIEW_DRAG_ROTATE = 1,
  VIEW_DRAG_PAN    = 2

} drag_button_t;

struct view_s;
typedef struct view_s view_t;

typedef void (*view_observer_fn)(view_t *v, gpointer user_data);

struct view_s
{
  /* Rotation: canonical store + optional sharing pointers.
   * rotation_master is set on the follower; rotation_follower is the
   * reverse link, set on the master by view_share_master() so a
   * master-side change propagates into the follower's changed_cb. */
  mat4    R;
  view_t *rotation_master;
  view_t *rotation_follower;


  /* Screen-space pan offset in pixels; consumers convert as needed. */
  vec2    pan_offset;

  /* Zoom factor (1.0 == fit-to-viewport). */
  float   zoom;

  /* Viewport geometry. */
  int     width;
  int     height;

  /* Which drawing area this view drives. */
  view_type_t type;

  /* Drag session.  drag_wr_deg/drag_wi_deg are the continuous
   * azimuth/elevation accumulators used by constrained drag; they
   * reset on view_begin_drag via Extract_View_Angles and then track
   * pointer deltas without the [-180,180]/[-90,90] wrap that a fresh
   * matrix decomposition would re-introduce on every event. */
  float       last_ptr_x;
  float       last_ptr_y;
  drag_button_t drag_button;
  drag_mode_t drag_mode;
  float       motion_divisor;
  double      drag_wr_deg;
  double      drag_wi_deg;

  /* Change notification.  Exactly one callback per view is named at
   * view_new(); there is no dynamic registration table.  Follower
   * propagation is handled by rotation_follower above, not by a
   * dispatcher entry. */
  view_observer_fn changed_cb;
  gpointer         changed_user;

  /* Reentrancy guard for view_notify_change().  Set while the
   * callback is firing and checked on entry; prevents a follower
   * callback that mutates the master from recursing back into the
   * dispatcher via the rotation_follower link. */
  gboolean        in_notify;

  /* Borrowed UI bindings; display updater writes through these. */
  GtkSpinButton *rotate_spin;
  GtkSpinButton *incline_spin;
  GtkSpinButton *zoom_spin;

  /* Borrowed spin-handler pointers.  view_update_spin_display() blocks
   * the handler around gtk_spin_button_set_value() so programmatic
   * value writes do not feed back as a value-changed signal. */
  GCallback rotate_spin_handler;
  GCallback incline_spin_handler;
};

typedef struct
{
  float zoom;
  vec2  pan_offset;

} view_fit_t;

/*-----------------------------------------------------------------
 * Lifecycle
 *----------------------------------------------------------------*/

/**
 * view_new() - Allocate and initialize a view
 * @type:         drawing-area identity
 * @rotate_spin:  borrowed azimuth spin button
 * @incline_spin: borrowed elevation spin button
 * @zoom_spin:    borrowed zoom spin button
 * @changed_cb:   callback invoked on every state change; NULL permitted
 * @changed_user: opaque pointer passed to @changed_cb
 *
 * Default rotation is (VIEW_DEFAULT_WR, VIEW_DEFAULT_WI).
 * Drag mode defaults to VIEW_DRAG_CONSTRAINED.
 */
view_t *view_new(view_type_t type,
                 GtkSpinButton *rotate_spin,
                 GtkSpinButton *incline_spin,
                 GtkSpinButton *zoom_spin,
                 view_observer_fn changed_cb,
                 gpointer changed_user);

/** view_free() - Release a view and clear caller's pointer
 * @vp: address of owning pointer; *vp is freed and set to NULL.
 */
void view_free(view_t **vp);

/*-----------------------------------------------------------------
 * Viewport and extent
 *----------------------------------------------------------------*/

/** view_set_viewport() - Record drawable size */
void view_set_viewport(view_t *v, int width, int height);

/** view_set_zoom() - Set zoom factor (1.0 == fit-to-viewport) */
void view_set_zoom(view_t *v, float zoom);

/*-----------------------------------------------------------------
 * Rotation
 *----------------------------------------------------------------*/

/** view_set_angles() - Set rotation from NEC2 (WR, WI) degrees */
void view_set_angles(view_t *v, double wr_deg, double wi_deg);

/** view_get_angles() - Decompose rotation to (WR, WI); writes NAN to wr at pole */
void view_get_angles(view_t *v, double *wr_deg, double *wi_deg);

/** view_get_theta_phi() - Spherical direction of the viewing axis
 * @v:     view
 * @theta: polar angle from +Z (degrees); always defined
 * @phi:   azimuth from +X (degrees); NAN at pole
 */
void view_get_theta_phi(view_t *v, double *theta, double *phi);

/** Build_View_Rotation_Matrix() - Canonical Rz(-90)*Ry(wi-90)*Rz(-wr) */
void Build_View_Rotation_Matrix(mat4 R, double wr_deg, double wi_deg);

/** Extract_View_Angles() - ZYZ decomposition; NAN to *wr_deg at pole */
void Extract_View_Angles(mat4 R, double *wr_deg, double *wi_deg);

/*-----------------------------------------------------------------
 * Drag
 *----------------------------------------------------------------*/

/** view_set_drag_mode() - Select constrained vs free rotation */
void view_set_drag_mode(view_t *v, drag_mode_t mode);

/** view_begin_drag() - Record button and pointer origin */
void view_begin_drag(view_t *v, drag_button_t button, float x, float y);

/** view_end_drag() - Release drag button */
void view_end_drag(view_t *v);

/** view_update_drag() - Dispatch drag or pan from current pointer position */
void view_update_drag(view_t *v, float x, float y);

/** view_apply_drag() - Accumulate rotation delta from pointer delta */
void view_apply_drag(view_t *v, float dx, float dy);

/** view_apply_pan_delta() - Accumulate screen-pixel pan delta */
void view_apply_pan_delta(view_t *v, float dx, float dy);

/** view_reset_pan() - Clear pan offset and notify observers */
void view_reset_pan(view_t *v);

/*-----------------------------------------------------------------
 * Sharing
 *----------------------------------------------------------------*/

/** view_share_master() - Follower borrows master's rotation matrix */
void view_share_master(view_t *follower, view_t *master);

/** view_unshare_master() - Follower reverts to its own rotation */
void view_unshare_master(view_t *follower);

/*-----------------------------------------------------------------
 * Change notification
 *----------------------------------------------------------------*/

/** view_notify_change() - Invoke the bound callback and propagate to follower */
void view_notify_change(view_t *v);

/*-----------------------------------------------------------------
 * UI readout
 *----------------------------------------------------------------*/

/** view_update_spin_display() - Write WR/WI values into bound spin widgets */
void view_update_spin_display(view_t *v);

/** view_set_spin_handlers() - Bind spin-handler callback pointers
 * @v:          view
 * @rotate_cb:  handler connected to rotate_spin's value-changed signal
 * @incline_cb: handler connected to incline_spin's value-changed signal
 *
 * Handlers are blocked around programmatic spin value writes.  Pass
 * NULL to clear.
 */
void view_set_spin_handlers(view_t *v,
                            GCallback rotate_cb,
                            GCallback incline_cb);

/** view_apply_fit() - Apply fitted zoom and pan as one view transition
 * @v:   view receiving the fitted state
 * @fit: fitted zoom and screen-space pan
 */
void view_apply_fit(view_t *v, const view_fit_t *fit);

/*-----------------------------------------------------------------
 * Rotation accessor and derived-at-use inlines
 *----------------------------------------------------------------*/

/**
 * view_rotation_owner() - Return the view that stores the canonical rotation
 *
 * Followers resolve to their master; independent views return themselves.
 * Single accessor for the write path so rotation mutators in view_core.c
 * do not repeat the master-nullness ternary.
 */
static inline view_t *
view_rotation_owner(view_t *v)
{
  return( (v->rotation_master != NULL) ? v->rotation_master : v );
}

/**
 * view_sync_drag() - Copy drag accumulators from one view to another
 * @dst: destination view
 * @src: source view
 *
 * Single definition of the two-field accumulator copy used by both
 * sync directions: mutator-to-owner and owner-to-follower.
 */
static inline void
view_sync_drag(view_t *dst, const view_t *src)
{
  dst->drag_wr_deg = src->drag_wr_deg;
  dst->drag_wi_deg = src->drag_wi_deg;
}

/**
 * view_sync_drag_to_owner() - Propagate drag accumulators from mutator to owner
 * @v: the view that mutated its accumulators
 *
 * When a follower mutates rotation (via view_set_angles or
 * view_apply_drag), the owner's accumulators must mirror the
 * follower's so view_update_spin_display on the owner reads
 * exact double-precision angles rather than a lossy float
 * matrix roundtrip through Extract_View_Angles.
 */
static inline void
view_sync_drag_to_owner(view_t *v)
{
  view_t *owner = view_rotation_owner(v);

  if( owner != v )
    view_sync_drag(owner, v);
}

/**
 * view_sync_drag_to_follower() - Propagate drag accumulators from owner to follower
 * @v: the owner view whose accumulators are authoritative
 *
 * When the owner's rotation changes, the follower's accumulators
 * must mirror the owner's so view_update_spin_display on the
 * follower reads exact double-precision angles rather than a lossy
 * float matrix roundtrip through Extract_View_Angles.
 */
static inline void
view_sync_drag_to_follower(view_t *v)
{
  if( v->rotation_follower != NULL )
    view_sync_drag(v->rotation_follower, v);
}

/**
 * view_R() - Return the canonical rotation matrix pointer.
 *
 * Read path counterpart of view_rotation_owner().  Consumers always
 * call view_R() rather than reading v->R directly.
 */
static inline float (*view_R(view_t *v))[4]
{
  return( view_rotation_owner(v)->R );
}

/**
 * view_projection_scale() - Pixel-per-unit scale for a given extent
 *
 * Computes screen pixels per model unit from viewport half-size, content
 * extent, and zoom factor.  Returns 1.0 when extent is zero (no geometry).
 */
static inline double
view_projection_scale(const view_t *v, float extent, float zoom)
{
  double size2;

  if( v->width < v->height )
    size2 = (double)v->width / 2.0;
  else
    size2 = (double)v->height / 2.0;

  if( extent <= 0.0f )
    return( 1.0 );

  return( (size2 / (double)extent) * (double)zoom );
}

/** view_x_center() - Screen X of the xyz origin */
static inline double
view_x_center(const view_t *v)
{
  return( (double)v->width / 2.0 );
}

/** view_y_center() - Screen Y of the xyz origin (half-pixel bias preserved) */
static inline double
view_y_center(const view_t *v)
{
  return( (double)v->height / 2.0 + 0.5 );
}

/*
 * Cairo consumers Project_on_Screen(), Set_Gdk_Segment(),
 * Viewer_Gain(), and Viewer_Noise_Value() are declared in common.h
 * alongside the Segment_t definition and implemented in view_core.c.
 */

#endif /* VIEW_CORE_H */
