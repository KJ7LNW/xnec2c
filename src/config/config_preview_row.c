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
 * config_preview_row: hover edges of one bound row.
 *
 * Adapts the hover edges a row's GTK class supports onto the staging
 * session, reading the destination and the candidate from the binding the
 * row already carries, so a hover stages exactly what a click commits.
 */

#include "widget/config_widget_element.h"
#include "widget/config_widget_row.h"
#include "config_preview.h"
#include "config_preview_row.h"
#include "../console.h"

/*------------------------------------------------------------------------*/

/** preview_row_storage - the address a row stages into
 * @w: a bound row
 *
 * Return: the bytes the row's selection owns.
 */
static void *
preview_row_storage(GtkWidget *w)
{
  return config_widget_row_get(w)->scope->dest.storage;
}

/** preview_row_stage - stage the candidate a row expresses
 * @w: a bound row
 */
static void
preview_row_stage(GtkWidget *w)
{
  const config_widget_row_t *row = config_widget_row_get(w);
  unsigned char candidate[row->scope->dest.size];

  /* Predict the widget's post-click value before staging its hover */
  if( !config_widget_element_candidate(row->elt, w, candidate,
        row->scope->dest.size) )
    return;

  config_preview_stage(&row->scope->dest, candidate);
}

/** preview_row_dismiss - end a hover the dismissing row's own field holds
 * @storage: the address that row stages into
 *
 * A session on any other address belongs to another selector, which ends its
 * own hover on its own edges.
 */
static void
preview_row_dismiss(const void *storage)
{
  if( !config_preview_active(storage) )
    return;

  config_preview_revert();
}

/*------------------------------------------------------------------------*/

/** on_preview_row_select - menu row highlighted
 * @item:      the highlighted row
 * @user_data: unused
 */
static void
on_preview_row_select(GtkMenuItem *item, gpointer user_data)
{
  (void)user_data;

  preview_row_stage(GTK_WIDGET(item));
}

/** on_preview_row_deselect - menu row lost the highlight
 * @item:      the row the pointer left
 * @user_data: unused
 *
 * Activation deselects a row as its menu tears down, so the restore is
 * queued rather than run here; the commit behind it cancels that restore,
 * leaving this edge to end a hover the pointer merely moved off.
 */
static void
on_preview_row_deselect(GtkMenuItem *item, gpointer user_data)
{
  (void)user_data;

  config_preview_queue_dismiss(preview_row_storage(GTK_WIDGET(item)));
}

/** on_preview_shell_closed - a menu shell completed or withdrew its selection
 * @shell:     the shell that closed, unused
 * @user_data: the address its rows stage into
 *
 * A shell tears down ahead of the row it activates, so the restore is queued
 * rather than run here; the commit behind it cancels that restore, leaving
 * this edge to end only a hover the user left without clicking.
 */
static void
on_preview_shell_closed(GtkWidget *shell, gpointer user_data)
{
  (void)shell;

  config_preview_queue_dismiss(user_data);
}

/** on_preview_row_enter - pointer entered a toggle row
 * @w:         the entered row
 * @event:     crossing event
 * @user_data: unused
 *
 * Return: FALSE so the row's own handlers still run.
 */
static gboolean
on_preview_row_enter(GtkWidget *w, GdkEventCrossing *event, gpointer user_data)
{
  (void)user_data;

  /* A crossing onto a child of the row never enters the row itself */
  if( event->detail == GDK_NOTIFY_INFERIOR )
    return FALSE;

  preview_row_stage(w);

  return FALSE;
}

/** on_preview_row_leave - pointer left a toggle row
 * @w:         the row left behind
 * @event:     crossing event
 * @user_data: unused
 *
 * Return: FALSE so the row's own handlers still run.
 */
static gboolean
on_preview_row_leave(GtkWidget *w, GdkEventCrossing *event, gpointer user_data)
{
  (void)user_data;

  if( event->detail == GDK_NOTIFY_INFERIOR )
    return FALSE;

  preview_row_dismiss(preview_row_storage(w));

  return FALSE;
}

/** on_preview_row_unmap - toggle row removed from presentation
 * @w:         the row being unmapped
 * @user_data: unused
 *
 * Hiding or destroying a dialog dismisses its hover without guaranteeing a
 * pointer crossing, so the disappearing row restores the committed value.
 */
static void
on_preview_row_unmap(GtkWidget *w, gpointer user_data)
{
  (void)user_data;

  preview_row_dismiss(preview_row_storage(w));
}

/*------------------------------------------------------------------------*/

/** preview_shell_attach - end this row's hover when its shell closes
 * @item:    a row already appended to its shell
 * @storage: the address this row stages into
 *
 * One connection per row, each ending only its own address, so a shell
 * carrying rows of several selections dismisses every one of them.
 *
 * A shell reports the selections it completes, while a submenu the pointer
 * travels back out of withdraws without completing one, so its withdrawal
 * carries the hovers that completion never reports.
 */
static void
preview_shell_attach(GtkWidget *item, void *storage)
{
  GtkWidget *shell = gtk_widget_get_parent(item);

  if( shell == NULL )
  {
    BUG("hover preview reached a menu row outside any menu shell\n");
    return;
  }

  g_signal_connect(shell, "selection-done",
      G_CALLBACK(on_preview_shell_closed), storage);
  g_signal_connect(shell, "hide",
      G_CALLBACK(on_preview_shell_closed), storage);
}

/*------------------------------------------------------------------------*/

gboolean
config_preview_class_hoverable(GtkWidget *w)
{
  return GTK_IS_MENU_ITEM(w) || GTK_IS_TOGGLE_BUTTON(w);
}

void
config_preview_row_attach(GtkWidget *w)
{
  if( GTK_IS_MENU_ITEM(w) )
  {
    g_signal_connect(w, "select", G_CALLBACK(on_preview_row_select), NULL);
    g_signal_connect(w, "deselect", G_CALLBACK(on_preview_row_deselect), NULL);
    preview_shell_attach(w, preview_row_storage(w));
  }
  else if( GTK_IS_TOGGLE_BUTTON(w) )
  {
    /* A toggle button sits outside any menu, so pointer crossings carry its
     * hover edges and losing presentation ends the hover in their place */
    gtk_widget_add_events(w, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(w, "enter-notify-event",
        G_CALLBACK(on_preview_row_enter), NULL);
    g_signal_connect(w, "leave-notify-event",
        G_CALLBACK(on_preview_row_leave), NULL);
    g_signal_connect(w, "unmap", G_CALLBACK(on_preview_row_unmap), NULL);
  }
  else
    BUG("hover preview reached a class carrying no row-hover edge\n");
}
