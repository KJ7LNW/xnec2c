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
 * config_preview: hover staging session lifecycle.
 *
 * The staged candidate lives in the target's own bytes and the committed
 * baseline lives in the session snapshot, so no consumer needs an accessor
 * to see what a hover is showing.
 */

#include <string.h>
#include "config_preview.h"
#include "../console.h"
#include "../mem/mem.h"

/* The one open session.  A NULL target storage is the idle state, and
 * dismiss_id is the restore a closing menu shell queued, zero when the
 * session carries none. */
typedef struct {
  preview_target_t target;
  void            *saved;
  guint            dismiss_id;
} preview_session_t;

static preview_session_t session;

/*------------------------------------------------------------------------*/

/** preview_dismiss_cancel - drop a queued restore that later events overtook */
static void
preview_dismiss_cancel(void)
{
  if( session.dismiss_id == 0 )
    return;

  g_source_remove(session.dismiss_id);
  session.dismiss_id = 0;
}

/** preview_session_close - drop the snapshot and return to idle */
static void
preview_session_close(void)
{
  preview_dismiss_cancel();
  mem_free(&session.saved);
  session.target.storage = NULL;
}

/** preview_session_restore - write the committed bytes back into the target */
static void
preview_session_restore(void)
{
  memcpy(session.target.storage, session.saved, session.target.size);
}

/*------------------------------------------------------------------------*/

void
config_preview_revert(void)
{
  preview_target_t t;
  gboolean staged;

  if( session.target.storage == NULL )
    return;

  t = session.target;
  staged = (memcmp(t.storage, session.saved, t.size) != 0);
  preview_session_restore();
  preview_session_close();

  /* Bytes that never moved already present the committed value */
  if( staged )
    t.refresh(t.context);
}

/*------------------------------------------------------------------------*/

void
config_preview_stage(const preview_target_t *t, const void *candidate)
{
  if( t == NULL || candidate == NULL )
  {
    BUG("hover staging requires a destination and candidate bytes\n");
    return;
  }

  if( t->storage == NULL || t->size == 0 || t->refresh == NULL )
  {
    BUG("hover staging requires storage, positive width, and refresh\n");
    return;
  }

  /* A row taking the hover overtakes the restore a closing shell queued */
  preview_dismiss_cancel();

  /* A candidate for another address ends that address's hover, so leaving one
   * selector for another restores the value the first one was showing; the
   * same address keeps its snapshot, so later rows of one menu stage against
   * the value the menu opened on. */
  if( session.target.storage != t->storage )
  {
    config_preview_revert();

    mem_alloc(&session.saved, t->size);
    memcpy(session.saved, t->storage, t->size);
  }

  session.target = *t;

  /* A candidate the field already holds proposes no transition to paint */
  if( memcmp(t->storage, candidate, t->size) != 0 )
  {
    memcpy(t->storage, candidate, t->size);

    t->refresh(t->context);
  }
}

/*------------------------------------------------------------------------*/

gboolean
config_preview_active(const void *storage)
{
  return (storage != NULL) && (session.target.storage == storage);
}

/*------------------------------------------------------------------------*/

gboolean
config_preview_prepare_commit(void *storage)
{
  if( !config_preview_active(storage) )
    return FALSE;

  preview_session_restore();
  preview_session_close();

  return TRUE;
}

/*------------------------------------------------------------------------*/

void
config_preview_discard(void *storage)
{
  /* No session on this address: the caller's write stands on its own */
  if( !config_preview_active(storage) )
    return;

  preview_session_close();
}

/*------------------------------------------------------------------------*/

/** preview_dismiss_idle - restore a hover that no commit overtook
 * @data: unused
 *
 * Return: G_SOURCE_REMOVE, so a queued restore runs once.
 */
static gboolean
preview_dismiss_idle(gpointer data)
{
  (void)data;

  /* Cleared ahead of the restore, so the close it reaches cancels nothing */
  session.dismiss_id = 0;

  config_preview_revert();

  return G_SOURCE_REMOVE;
}

/*------------------------------------------------------------------------*/

void
config_preview_queue_dismiss(void *storage)
{
  if( !config_preview_active(storage) )
    return;

  /* The first close of this hover already queued the restore */
  if( session.dismiss_id != 0 )
    return;

  session.dismiss_id = g_idle_add(preview_dismiss_idle, NULL);
}
