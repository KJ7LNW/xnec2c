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

#ifndef CONFIG_WIDGET_OPS_TARGET_H
#define CONFIG_WIDGET_OPS_TARGET_H 1

#include "config_widget_ops.h"
#include "config_widget_priv.h"

/*
 * What each operation does at one resolved widget.  Every leaf reads the
 * operand its operation names and touches the widget it is handed alone,
 * so nothing here knows a datum, a group or a window.
 */

/** config_widget_target_visible - show or hide one widget
 * @t:       the widget and the element describing it
 * @operand: the gboolean visible state
 */
void config_widget_target_visible(const config_widget_target_t *t,
    const void *operand);

/** config_widget_target_sensitive - make one widget usable or not
 * @t:       the widget and the element describing it
 * @operand: the gboolean sensitivity
 */
void config_widget_target_sensitive(const config_widget_target_t *t,
    const void *operand);

/** config_widget_target_tooltip - say why one widget reads as it does
 * @t:       the widget and the element describing it
 * @operand: the tooltip text, or NULL to clear it
 */
void config_widget_target_tooltip(const config_widget_target_t *t,
    const void *operand);

/** config_widget_target_connect - wire one event on one widget
 * @t:       the widget and the element describing it
 * @operand: the event name, its handler and the subject that handler acts on
 */
void config_widget_target_connect(const config_widget_target_t *t,
    const void *operand);

/** config_widget_target_flush - parse the text pending in one widget
 * @t:       the widget and the element describing it
 * @operand: unused
 */
void config_widget_target_flush(const config_widget_target_t *t,
    const void *operand);

/** config_widget_target_capture - hand one resolved widget to its declared owner
 * @t:       the widget and the element describing it
 * @operand: unused
 */
void config_widget_target_capture(const config_widget_target_t *t,
    const void *operand);

#endif /* CONFIG_WIDGET_OPS_TARGET_H */
