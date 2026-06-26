/*
 *  wl_session.c - Wayland xdg-session-management window restore.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "wl_session.h"
#include "shared.h"

#ifdef HAVE_GDK_WAYLAND_GET_XDG_TOPLEVEL

#include <gdk/gdkwayland.h>
#include "xdg-session-management-v1-client.h"
#include "xx-session-management-v1-client.h"

/* One app-wide session covers every toplevel; each is keyed by a stable
 * name passed to restore_toplevel. The cookie persists in
 * rc_config.session_id across restarts. */

/* xdg_ (staged) variant — preferred */
static struct xdg_session_manager_v1 *xdg_session_manager;
static struct xdg_session_v1         *xdg_session;

/* xx_ (draft/KWin) variant — fallback */
static struct xx_session_manager_v1  *xx_session_manager;
static struct xx_session_v1          *xx_session;

/* -- xdg_ listeners -------------------------------------------------------- */

  static void
on_xdg_session_created( void *data, struct xdg_session_v1 *obj,
    const char *session_id )
{
  (void) data;
  (void) obj;

  g_strlcpy( rc_config.session_id, session_id,
      sizeof(rc_config.session_id) );
}

  static void
on_xdg_session_restored( void *data, struct xdg_session_v1 *obj )
{
  (void) data;
  (void) obj;
}

  static void
on_xdg_session_replaced( void *data, struct xdg_session_v1 *obj )
{
  (void) data;
  (void) obj;
}

static const struct xdg_session_v1_listener xdg_session_listener = {
  .created  = on_xdg_session_created,
  .restored = on_xdg_session_restored,
  .replaced = on_xdg_session_replaced,
};

/* -- xx_ listeners --------------------------------------------------------- */

  static void
on_xx_session_created( void *data, struct xx_session_v1 *obj,
    const char *id )
{
  (void) data;
  (void) obj;

  pr_debug("wl_session: xx created id=%s\n", id ? id : "(null)");
  g_strlcpy( rc_config.session_id, id, sizeof(rc_config.session_id) );
}

  static void
on_xx_session_restored( void *data, struct xx_session_v1 *obj )
{
  (void) data;
  (void) obj;

  pr_debug("wl_session: xx session restored\n");
}

  static void
on_xx_session_replaced( void *data, struct xx_session_v1 *obj )
{
  (void) data;
  (void) obj;
}

static const struct xx_session_v1_listener xx_session_listener = {
  .created  = on_xx_session_created,
  .restored = on_xx_session_restored,
  .replaced = on_xx_session_replaced,
};

/* -- registry -------------------------------------------------------------- */

  static void
registry_global( void *data, struct wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version )
{
  (void) data;
  (void) version;

  if( g_strcmp0(interface, xdg_session_manager_v1_interface.name) == 0 )
    xdg_session_manager = wl_registry_bind( registry, name,
        &xdg_session_manager_v1_interface, 1 );
  else if( g_strcmp0(interface, xx_session_manager_v1_interface.name) == 0 )
    xx_session_manager = wl_registry_bind( registry, name,
        &xx_session_manager_v1_interface, 1 );
}

  static void
registry_global_remove( void *data, struct wl_registry *registry,
    uint32_t name )
{
  (void) data;
  (void) registry;
  (void) name;
}

static const struct wl_registry_listener registry_listener = {
  .global        = registry_global,
  .global_remove = registry_global_remove,
};

/* -- toplevel restore ------------------------------------------------------ */

/* The compositor stores a toplevel's geometry only while its session handle
 * lives: XdgToplevelWindow reads on map and writes on role destroy, both
 * gated on a non-null session pointer. The handle is therefore kept alive
 * for the GdkWindow's lifetime and released through the data destructor
 * below; destroying it early loses the saved state. */
  static void
destroy_xdg_toplevel_session( gpointer data )
{
  xdg_toplevel_session_v1_destroy( data );
}

  static void
destroy_xx_toplevel_session( gpointer data )
{
  xx_toplevel_session_v1_destroy( data );
}

/* The session restore request must precede the toplevel's first commit;
 * the xdg-toplevel-realized signal fires in exactly that window. */
  static void
on_xdg_toplevel_realized( GdkWindow *gdkwin, gpointer user_data )
{
  const char *name = user_data;
  struct xdg_toplevel *toplevel;

  toplevel = gdk_wayland_window_get_xdg_toplevel( gdkwin );
  if( unlikely(toplevel == NULL) )
    return;

  if( xdg_session != NULL )
  {
    struct xdg_toplevel_session_v1 *ts =
        xdg_session_v1_restore_toplevel( xdg_session, toplevel, name );
    g_object_set_data_full( G_OBJECT(gdkwin), "wl-toplevel-session",
        ts, destroy_xdg_toplevel_session );
    pr_debug("wl_session: xdg restore_toplevel name=%s\n", name);
  }
  else if( xx_session != NULL )
  {
    struct xx_toplevel_session_v1 *ts =
        xx_session_v1_restore_toplevel( xx_session, toplevel, name );
    g_object_set_data_full( G_OBJECT(gdkwin), "wl-toplevel-session",
        ts, destroy_xx_toplevel_session );
    pr_debug("wl_session: xx restore_toplevel name=%s\n", name);
  }
}

/* Defer the xdg-toplevel-realized connection until the widget owns a
 * GdkWindow; the name string is a stable literal. */
  static void
on_widget_realize( GtkWidget *widget, gpointer user_data )
{
  GdkWindow *gdkwin = gtk_widget_get_window( widget );

  if( unlikely(gdkwin == NULL) )
  {
    pr_debug("wl_session: realize name=%s gdkwin=NULL\n", (const char *) user_data);
    return;
  }

  pr_debug("wl_session: realize name=%s connecting xdg-toplevel-realized\n",
      (const char *) user_data);
  g_signal_connect( gdkwin, "xdg-toplevel-realized",
      G_CALLBACK(on_xdg_toplevel_realized), user_data );
}

/* -- public API ------------------------------------------------------------ */

  void
wl_session_init( void )
{
  GdkDisplay *display = gdk_display_get_default();
  struct wl_display *wl_display;
  struct wl_registry *registry;
  const char *cookie;

  if( unlikely(display == NULL || !GDK_IS_WAYLAND_DISPLAY(display)) )
    return;

  wl_display = gdk_wayland_display_get_wl_display( display );
  registry = wl_display_get_registry( wl_display );
  wl_registry_add_listener( registry, &registry_listener, NULL );
  wl_display_roundtrip( wl_display );

  cookie = rc_config.session_id[0] ? rc_config.session_id : NULL;
  pr_debug("wl_session: get_session cookie=%s\n", cookie ? cookie : "(null)");

  if( xdg_session_manager != NULL )
  {
    xdg_session = xdg_session_manager_v1_get_session( xdg_session_manager,
        XDG_SESSION_MANAGER_V1_REASON_SESSION_RESTORE, cookie );
    xdg_session_v1_add_listener( xdg_session, &xdg_session_listener, NULL );
  }
  else if( xx_session_manager != NULL )
  {
    xx_session = xx_session_manager_v1_get_session( xx_session_manager,
        XX_SESSION_MANAGER_V1_REASON_SESSION_RESTORE, cookie );
    xx_session_v1_add_listener( xx_session, &xx_session_listener, NULL );
  }
  else
  {
    pr_warn("compositor lacks xdg_session_manager_v1 and xx_session_manager_v1; window positions will not restore\n");
  }
}

/* Registration only wires the realize hook; the session need not be open
 * yet. on_xdg_toplevel_realized resolves the live session when it fires, so
 * windows must register before they realize, independent of init order. */
  void
wl_session_register_window( GtkWidget *window, const char *name )
{
  pr_debug("wl_session: register name=%s realized=%d\n",
      name, gtk_widget_get_realized(window));

  if( gtk_widget_get_realized(window) )
    on_widget_realize( window, (gpointer) name );
  else
    g_signal_connect( window, "realize",
        G_CALLBACK(on_widget_realize), (gpointer) name );
}

#else /* !HAVE_GDK_WAYLAND_GET_XDG_TOPLEVEL */

  void
wl_session_init( void )
{
}

  void
wl_session_register_window( GtkWidget *window, const char *name )
{
  (void) window;
  (void) name;
}

#endif /* HAVE_GDK_WAYLAND_GET_XDG_TOPLEVEL */
