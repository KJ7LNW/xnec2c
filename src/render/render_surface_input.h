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

#ifndef __RENDER_SURFACE_INPUT_H
#define __RENDER_SURFACE_INPUT_H 1

#include "render_engine.h"

/* Modifier scroll operations the constructing site supplies.  A NULL member
 * declines that modifier, which the generic handler then treats as an
 * unmodified event. */
typedef struct
{
  gboolean (*on_shift_scroll)(GdkEventScroll *event, render_surface_t *surface);
  gboolean (*on_ctrl_scroll)(GdkEventScroll *event, render_surface_t *surface);

  /* Notice advertising the ctrl+scroll capability, presented on the first
   * frame of the session that offers it */
  const char *ctrl_scroll_notice;

  /* Notice advertising the shift+scroll capability, presented on the first
   * frame of the session carrying the overlay geometry it scales */
  const char *shift_scroll_notice;

} surface_input_ops_t;

/**
 * surface_input_connect() - Wire pointer, scroll and allocation handlers
 * @surface: surface whose presented widget receives the events
 *
 * Every handler mutates the borrowed view, so a surface showing none carries
 * no pointer behavior and is left unwired.
 */
void surface_input_connect(render_surface_t *surface);

#endif /* __RENDER_SURFACE_INPUT_H */
