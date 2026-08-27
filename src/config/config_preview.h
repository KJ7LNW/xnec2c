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

#ifndef CONFIG_PREVIEW_H
#define CONFIG_PREVIEW_H 1

#include <gtk/gtk.h>

/*
 * config_preview: hover staging of a candidate into its authoritative field.
 *
 * A hovered row writes its candidate over the field the selection owns and
 * repaints from it, so every consumer reads the previewed value from the one
 * place the committed value lives.  The committed bytes live in the session
 * snapshot until the hover commits or is dismissed.  At most one session is
 * open: a menu grab is exclusive and pointer focus is singular.
 */

/*------------------------------------------------------------------------*/

/* One staging destination: the bytes a candidate overwrites and the
 * idempotent repaint that renders them.  refresh receives context, so a
 * registry field and a per-view field describe themselves alike. */
typedef struct {
  void   *storage;
  size_t  size;
  void  (*refresh)(void *context);
  void   *context;
} preview_target_t;

/*------------------------------------------------------------------------*/

/** config_preview_stage - write a candidate into its target and repaint
 * @t:         the staging destination
 * @candidate: @t->size bytes the hover proposes
 *
 * Opens a session on the first candidate for a storage address and reuses
 * that snapshot for later candidates, so travelling across the rows of one
 * menu keeps the committed bytes captured once.  A candidate for a different
 * address reverts the open session first, stranding no value.
 */
void config_preview_stage(const preview_target_t *t, const void *candidate);

/** config_preview_active - whether a session is staging into @storage
 * @storage: address the caller owns
 *
 * Return: TRUE while @storage holds a staged candidate.
 */
gboolean config_preview_active(const void *storage);

/** config_preview_prepare_commit - restore the baseline ahead of a commit
 * @storage: address the commit writes
 *
 * Restores the committed bytes and closes the session without repainting,
 * so the caller's write is read as the transition the user made rather than
 * a no-op against the hovered value.
 *
 * Return: TRUE when a session was open on @storage.
 */
gboolean config_preview_prepare_commit(void *storage);

/** config_preview_discard - close a session whose value is already committed
 * @storage: address the caller has written
 */
void config_preview_discard(void *storage);

/** config_preview_revert - restore the committed bytes and repaint
 *
 * Idle when no session is open.
 */
void config_preview_revert(void);

/** config_preview_queue_dismiss - restore once the main loop settles
 * @storage: address whose hover the closing shell ends
 *
 * A menu shell tears itself down before it activates the clicked row, so an
 * immediate restore would repaint the committed value ahead of the commit
 * that follows.  Queuing lets a commit close the session first and repaint
 * once; a close with no commit behind it restores on the next main-loop
 * pass.  Idle when no session is open on @storage.
 */
void config_preview_queue_dismiss(void *storage);

#endif /* CONFIG_PREVIEW_H */
