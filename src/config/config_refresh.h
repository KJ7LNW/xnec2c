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

#ifndef CONFIG_REFRESH_H
#define CONFIG_REFRESH_H 1

/*
 * config_refresh: classify idempotent refreshes for hover staging.
 *
 * Hover safety describes the refresh, not the field: one refresh serves many
 * fields, so the answer is carried beside the function and every field naming
 * that function inherits it.
 *
 * A refresh is hover-safe when re-running it with the restored value restores
 * the previous frame: it repaints from current state, dispatches no
 * computation, replaces no canvas engine, and mutates nothing a revert cannot
 * undo.  A bounded resource rebuild keeps that property.
 *
 * A refresh is commit-only when it dispatches computation, replaces a canvas
 * engine, or leaves state behind that restoring the field does not return.
 */

/* Whether a refresh may run on hover; UNSET reports at bind time so a new
 * refresh is classified by its author rather than defaulted. */
typedef enum {
  REFRESH_CLASS_UNSET = 0,
  REFRESH_HOVER_SAFE,
  REFRESH_COMMIT_ONLY,
  REFRESH_CLASS_COUNT
} refresh_class_t;

/* A field refresh, taking the field it renders from the registry. */
typedef struct {
  void          (*fn)(void);
  refresh_class_t cls;
} config_refresh_t;

#endif /* CONFIG_REFRESH_H */
