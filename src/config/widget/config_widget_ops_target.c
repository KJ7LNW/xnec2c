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
 * config_widget_ops_target: the widget end of a datum-addressed operation.
 *
 * One leaf per operation, each reading the operand its operation names and
 * touching the one widget the walk handed it.
 */

#include "config_widget_ops_target.h"

/*------------------------------------------------------------------------*/

/** config_widget_target_visible - show or hide one widget
 * @t:       the widget and the element describing it
 * @operand: the gboolean visible state
 */
void
config_widget_target_visible(const config_widget_target_t *t,
    const void *operand)
{
  const gboolean *visible = operand;

  gtk_widget_set_visible(t->widget, *visible);
}

/** config_widget_target_sensitive - make one widget usable or not
 * @t:       the widget and the element describing it
 * @operand: the gboolean sensitivity
 */
void
config_widget_target_sensitive(const config_widget_target_t *t,
    const void *operand)
{
  const gboolean *sensitive = operand;

  gtk_widget_set_sensitive(t->widget, *sensitive);
}

/** config_widget_target_tooltip - say why one widget reads as it does
 * @t:       the widget and the element describing it
 * @operand: the tooltip text, or NULL to clear it
 */
void
config_widget_target_tooltip(const config_widget_target_t *t,
    const void *operand)
{
  const char *text = operand;

  gtk_widget_set_tooltip_text(t->widget, text);
}

/** config_widget_target_connect - wire one event on one widget
 * @t:       the widget and the element describing it
 * @operand: the event name, its handler and the subject that handler acts on
 *
 * The toolkit's own record of connected handlers answers whether this
 * widget is already wired, so a presentation its selector rebuilt wires
 * once.
 */
void
config_widget_target_connect(const config_widget_target_t *t,
    const void *operand)
{
  const config_widget_signal_t *signal = operand;
  union { GCallback cb; gpointer p; } handler = { .cb = signal->handler };

  if( g_signal_handler_find(t->widget,
        G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, 0, 0, NULL,
        handler.p, signal->data) == 0 )
    g_signal_connect(t->widget, signal->name, signal->handler, signal->data);
}

/** config_widget_target_flush - parse the text pending in one widget
 * @t:       the widget and the element describing it
 * @operand: unused
 *
 * An entry the user typed into but never activated reaches its own commit
 * path here, before the caller reads the field.
 */
void
config_widget_target_flush(const config_widget_target_t *t,
    const void *operand)
{
  (void)operand;

  if( GTK_IS_SPIN_BUTTON(t->widget) )
    gtk_spin_button_update(GTK_SPIN_BUTTON(t->widget));
  else
    BUG("a pending-text flush reached a %s, which holds none\n",
        G_OBJECT_TYPE_NAME(t->widget));
}

/** config_widget_target_capture - hand one resolved widget to its declared owner
 * @t:       the widget and the element describing it
 * @operand: unused
 */
void
config_widget_target_capture(const config_widget_target_t *t,
    const void *operand)
{
  (void)operand;

  /* A row the declaration hands to no owner keeps no reference */
  if( t->element->capture != NULL )
    t->element->capture(t->widget);
}
