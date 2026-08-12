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
 * render_canvas_lifetime: the token a bound canvas and its queued frame
 * requests share.
 *
 * References are counted atomically because frame requests are queued from
 * the worker threads as well as the main thread.
 */

#include "render_canvas_lifetime.h"
#include "../shared.h"

/* canvas_lifetime_t - the canvas named and the references outstanding. */
struct canvas_lifetime_s
{
  gatomicrefcount refs;
  canvas_id_t id;
};

  canvas_lifetime_t *
canvas_lifetime_new(canvas_id_t id)
{
  canvas_lifetime_t *lifetime = NULL;

  mem_new( &lifetime );
  g_atomic_ref_count_init( &lifetime->refs );
  lifetime->id = id;

  return( lifetime );

} /* canvas_lifetime_new() */

  canvas_lifetime_t *
canvas_lifetime_acquire(canvas_lifetime_t *lifetime)
{
  g_atomic_ref_count_inc( &lifetime->refs );

  return( lifetime );

} /* canvas_lifetime_acquire() */

  void
canvas_lifetime_release(gpointer data)
{
  canvas_lifetime_t *lifetime = data;

  if( lifetime == NULL )
    return;

  if( g_atomic_ref_count_dec(&lifetime->refs) )
    mem_free( &lifetime );

} /* canvas_lifetime_release() */

  canvas_id_t
canvas_lifetime_id(const canvas_lifetime_t *lifetime)
{
  return( lifetime->id );

} /* canvas_lifetime_id() */

/*-----------------------------------------------------------------------*/
