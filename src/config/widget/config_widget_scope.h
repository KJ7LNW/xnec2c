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

#ifndef CONFIG_WIDGET_SCOPE_H
#define CONFIG_WIDGET_SCOPE_H 1

#include "../config_preview.h"
#include "../config_refresh.h"

/*
 * config_widget_scope: the capabilities one selection dispatches through.
 *
 * Exactly one scope exists per selection: the bytes its value lives in, the
 * idempotent render of those bytes, the transition edge a commit adds on top
 * of that render, and the classification that render carries.  Every row of
 * that selection holds the scope by reference, so no widget can disagree
 * with the selection it expresses.
 *
 * A scope outlives every row that dispatches through it.  A registered
 * field's scope is allocated by the registry on its own, because the binding
 * array relocates as it grows and a widget retains the pointer.  A scoped
 * selection embeds its scope in the object owning the state, such as one
 * frequency-plots view, and the scope dies with that object.
 */
typedef struct {
  preview_target_t dest;
  void           (*commit)(void *context);
  refresh_class_t  cls;
} config_widget_scope_t;

#endif /* CONFIG_WIDGET_SCOPE_H */
