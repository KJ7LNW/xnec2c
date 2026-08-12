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

#ifndef __RENDER_CANVAS_LIFETIME_H
#define __RENDER_CANVAS_LIFETIME_H 1

#include "render_canvas.h"

/*
 * canvas_lifetime_t - the token naming one bound canvas lifetime.
 *
 * A canvas takes a token when its first surface binds and drops it when its
 * window closes.  A queued frame request holds a reference of its own, so a
 * request arriving after that window closed still reads a live token, finds
 * the canvas holding a different one, and retires without drawing.  The
 * held reference also keeps the address occupied, so a reused pooled handle
 * cannot be mistaken for the closed window.
 */
typedef struct canvas_lifetime_s canvas_lifetime_t;

/** canvas_lifetime_new() - Take a token for a canvas that has just bound */
canvas_lifetime_t *canvas_lifetime_new(canvas_id_t id);

/** canvas_lifetime_acquire() - Hold a further reference, returning @lifetime */
canvas_lifetime_t *canvas_lifetime_acquire(canvas_lifetime_t *lifetime);

/** canvas_lifetime_release() - Drop one reference, freeing the last
 * @lifetime: token to release, or NULL
 *
 * Shaped as a GDestroyNotify so a queued request releases its reference
 * whichever way it retires.
 */
void canvas_lifetime_release(gpointer lifetime);

/** canvas_lifetime_id() - Report the canvas the token names */
canvas_id_t canvas_lifetime_id(const canvas_lifetime_t *lifetime);

#endif /* __RENDER_CANVAS_LIFETIME_H */
