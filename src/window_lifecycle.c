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
 * window_lifecycle: Window liveness and the order its resources release.
 *
 * One descriptor per window_t member relates the window to the builder
 * owning its widgets, the canvas presenting its frames, and the main-window
 * menu item tracking it.  The builder pointer carries the liveness: it holds
 * from creation until release, and every widget lookup resolves through it.
 */
#include "window_lifecycle.h"
#include "shared.h"
#include "config/widget/config_widget.h"
#include "render/render_canvas.h"

/* Signature of the main-window check menu item handler a window is tracked
 * by, held so the programmatic deactivation blocks that exact handler. */
typedef void (*window_menu_handler_t)(GtkMenuItem *menuitem, gpointer user_data);

/* The rc_config fields persisting one window's placement and size. */
typedef struct
{
  int *x;
  int *y;
  int *width;
  int *height;

} window_geom_ref_t;

/* Resources one window owns.  A window carrying no main-window menu item
 * leaves both menu members null, and a window presenting no rotatable view
 * leaves the view member null. */
typedef struct
{
  GtkWidget     **window;
  GtkBuilder    **builder;
  canvas_id_t     canvas;
  view_t        **view;
  const char     *menu_item;
  window_menu_handler_t menu_handler;
  window_geom_ref_t geom;

} window_desc_t;

static const window_desc_t window_desc[WINDOW_COUNT] =
{
  /* window_row() rejects WINDOW_NONE, so its row holds no resources. */
  [WINDOW_NONE] = { 0 },

  [MAIN_WINDOW] =
    { .window = &main_window, .builder = &main_window_builder,
      .canvas = CANVAS_STRUCTURE, .view = &structure_view,
      .geom = { .x = &rc_config.main_x, .y = &rc_config.main_y,
                .width = &rc_config.main_width,
                .height = &rc_config.main_height } },
  [FREQPLOTS_WINDOW] =
    { .window = &freqplots_window, .builder = &freqplots_window_builder,
      .canvas = CANVAS_FREQPLOTS, .menu_item = "main_freqplots",
      .menu_handler = on_main_freqplots_activate,
      .geom = { .x = &rc_config.freqplots_x, .y = &rc_config.freqplots_y,
                .width = &rc_config.freqplots_width,
                .height = &rc_config.freqplots_height } },
  [RDPATTERN_WINDOW] =
    { .window = &rdpattern_window, .builder = &rdpattern_window_builder,
      .canvas = CANVAS_RDPATTERN, .view = &rdpattern_view,
      .menu_item = "main_rdpattern",
      .menu_handler = on_main_rdpattern_activate,
      .geom = { .x = &rc_config.rdpattern_x, .y = &rc_config.rdpattern_y,
                .width = &rc_config.rdpattern_width,
                .height = &rc_config.rdpattern_height } },
};

/*-----------------------------------------------------------------------*/

/**
 * window_row() - Resolve the descriptor of a window
 * @type: window whose resources are named
 *
 * Return: the descriptor, or NULL when @type names no window.
 */
static const window_desc_t *
window_row(window_t type)
{
  if( unlikely(type < MAIN_WINDOW || type >= WINDOW_COUNT) )
  {
    BUG("window type %d names no window\n", (int)type);
    return NULL;
  }

  return( &window_desc[type] );

} /* window_row() */

/*-----------------------------------------------------------------------*/

/**
 * window_deactivate_menu() - Clear the menu item tracking a window
 * @row: descriptor of the window being released
 *
 * Writes the item programmatically, so its handler is blocked across the
 * write and the destroy arm it carries serves operator interaction alone.
 */
static void
window_deactivate_menu(const window_desc_t *row)
{
  GtkWidget *item;

  if( row->menu_item == NULL )
    return;

  item = Builder_Get_Object( main_window_builder, row->menu_item );

  SIGNAL_BLOCK( item, row->menu_handler );
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(item), FALSE );
  SIGNAL_UNBLOCK( item, row->menu_handler );

} /* window_deactivate_menu() */

/*-----------------------------------------------------------------------*/

/**
 * window_is_open() - Whether a window holds its widgets
 * @type: window to test
 *
 * Return: TRUE from the moment the builder is created until release.
 */
gboolean
window_is_open(window_t type)
{
  const window_desc_t *row = window_row( type );

  if( row == NULL )
    return FALSE;

  return( *row->builder != NULL );

} /* window_is_open() */

/*-----------------------------------------------------------------------*/

/**
 * window_builder() - Name the builder owning a window's widgets
 * @type: window whose widget lookups are resolved
 *
 * Return: the builder, or NULL while the window holds none.
 */
GtkBuilder *
window_builder(window_t type)
{
  const window_desc_t *row = window_row( type );

  if( row == NULL )
    return NULL;

  return( *row->builder );

} /* window_builder() */

/*-----------------------------------------------------------------------*/

/**
 * window_view() - Name the view a window rotates
 * @type: window whose view is addressed
 *
 * Return: the view, or NULL when the window presents none or holds no
 * view while closed.
 */
view_t *
window_view(window_t type)
{
  const window_desc_t *row = window_row( type );

  if( row == NULL || row->view == NULL )
    return NULL;

  return( *row->view );

} /* window_view() */

/*-----------------------------------------------------------------------*/

/**
 * window_row_of_toplevel() - Resolve the descriptor holding a toplevel
 * @toplevel: toplevel widget matched against the descriptor table
 *
 * Matches by widget identity, so a window entering that table is
 * recognized here without a further edit.
 *
 * Return: the descriptor whose window holds @toplevel, or NULL when no
 * described window holds it.
 */
static const window_desc_t *
window_row_of_toplevel(GtkWidget *toplevel)
{
  const window_desc_t *match = NULL;

  /* A released window holds a null toplevel, as does every row of a closed
   * window, so only a live widget is matched against the table. */
  if( toplevel == NULL )
    return NULL;

  /* The scan stops at the row holding this toplevel; a row holding another
   * leaves the search running to the next. */
  for( window_t type = MAIN_WINDOW;
       (match == NULL) && (type < WINDOW_COUNT); type++ )
    match = ( *window_desc[type].window == toplevel )
      ? &window_desc[type] : NULL;

  return( match );

} /* window_row_of_toplevel() */

/*-----------------------------------------------------------------------*/

/**
 * window_from_widget() - Name the window presenting a widget
 * @widget: widget belonging to one of the described windows
 *
 * Return: the window presenting @widget, or MAIN_WINDOW when its toplevel
 * matches no row, the structure window being the one always present.
 */
window_t
window_from_widget(GtkWidget *widget)
{
  const window_desc_t *row =
      window_row_of_toplevel( gtk_widget_get_toplevel(widget) );

  if( row == NULL )
    return MAIN_WINDOW;

  return( (window_t)(row - window_desc) );

} /* window_from_widget() */

/*-----------------------------------------------------------------------*/

/**
 * window_capture_geometry() - Commit a window's placement and size
 * @toplevel: toplevel whose geometry is persisted
 *
 * Resolves the row by widget identity, so geometry arriving before the
 * window global is assigned, or from a window holding no row, writes no
 * other window's fields.  A closed window matches no row and keeps the
 * geometry it last reported.
 */
void
window_capture_geometry(GtkWidget *toplevel)
{
  const window_desc_t *row = window_row_of_toplevel( toplevel );

  if( row == NULL )
    return;

  Get_Window_Geometry( toplevel, row->geom.x, row->geom.y,
      row->geom.width, row->geom.height );

} /* window_capture_geometry() */

/*-----------------------------------------------------------------------*/

/**
 * window_release() - Release the resources a destroyed window owned
 * @type: window whose toplevel is being destroyed
 *
 * Reports the window closed before releasing anything, so a callback or
 * destructor reached during release finds the builder, the toplevel and the
 * canvas all reporting closed and refreshes no freed presentation.
 */
void
window_release(window_t type)
{
  const window_desc_t *row = window_row( type );
  GtkBuilder *builder;

  if( row == NULL )
    return;

  builder = *row->builder;
  *row->builder = NULL;
  *row->window  = NULL;
  canvas_clear( row->canvas );
  kill_window   = NULL;

  /* Both verbs absorb a second release: a released window unrefs nothing
   * and its menu item already reads inactive. */
  g_clear_object( &builder );
  window_deactivate_menu( row );

} /* window_release() */

/*-----------------------------------------------------------------------*/
