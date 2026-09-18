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

#ifndef CONFIG_WIDGET_OPS_H
#define CONFIG_WIDGET_OPS_H 1

#include "config_widget.h"

/*
 * config_widget_ops: what a datum's presentations are asked to do.
 *
 * A caller names the datum and the operands; the datum's binding names the
 * windows, the widgets and the requirements delivery obeys.  One
 * presentation is a collection of one, so a single control, several
 * controls in one window and controls across windows read alike.
 */

/*------------------------------------------------------------------------*/

/* One event wiring carried to every widget a datum reaches.  Every
 * presentation of the datum receives the same specification, so the subject
 * the handler acts on travels in data rather than in a widget name. */
typedef struct {
  const char *name;
  GCallback   handler;
  gpointer    data;
} config_widget_signal_t;

/*------------------------------------------------------------------------*/

/* The operation identities a group's participation list names.  A group
 * listing one of these takes part in it only while that entry's own
 * requirement answers TRUE; an operation the list does not name reaches
 * every live group of the datum. */
extern const config_widget_operation_t config_widget_readout_operation;
extern const config_widget_operation_t config_widget_visible_operation;
extern const config_widget_operation_t config_widget_sensitive_operation;
extern const config_widget_operation_t config_widget_tooltip_operation;
extern const config_widget_operation_t config_widget_connect_operation;
extern const config_widget_operation_t config_widget_flush_operation;
extern const config_widget_operation_t config_widget_capture_operation;

/*------------------------------------------------------------------------*/

/** config_widget_readout - show a datum's declared readout bytes
 * @field: address of a registered field whose tree names a readout source
 *
 * The passive direction of the projection: the widgets display the readout
 * source while the field keeps the bytes a commit writes.  Writes nothing
 * to the field and fires no hook.
 */
void config_widget_readout(const void *field);

/** config_widget_set_visible - show or hide every widget a datum reaches
 * @field:   address of a registered field
 * @visible: the visible state to apply
 */
void config_widget_set_visible(const void *field, gboolean visible);

/** config_widget_set_sensitive - make every widget a datum reaches usable or not
 * @field:     address of a registered field
 * @sensitive: the sensitivity to apply
 */
void config_widget_set_sensitive(const void *field, gboolean sensitive);

/** config_widget_set_tooltip - say why every widget a datum reaches reads as it does
 * @field: address of a registered field
 * @text:  the tooltip, or NULL to clear it
 */
void config_widget_set_tooltip(const void *field, const char *text);

/** config_widget_connect - wire one event on every widget a datum reaches
 * @field:  address of a registered field
 * @signal: the event name, its handler and the subject that handler acts on
 *
 * Repeat-safe: a widget already carrying this handler with this subject is
 * left as it stands, so a rebuilt presentation wires once.
 */
void config_widget_connect(const void *field,
                           const config_widget_signal_t *signal);

/** config_widget_flush - parse the text pending in every widget a datum reaches
 * @field: address of a registered field
 *
 * Synchronous: an entry the user has typed into but not activated reaches
 * its own commit path before this returns.
 */
void config_widget_flush(const void *field);

/** config_widget_capture - hand each resolved widget to its declared owner
 * @field: address of a registered field
 *
 * For the interaction references a presentation's owner holds by borrow.
 * Runs each element's capture callback at the point the owner already
 * acquired that reference; retains nothing and wires nothing.
 */
void config_widget_capture(const void *field);

#endif /* CONFIG_WIDGET_OPS_H */
