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

#ifndef __RENDER_CHECK_H
#define __RENDER_CHECK_H 1

#include "../common.h"
#include "../view/view_core.h"

/* Name the content mode resolved from the active view. */
typedef enum
{
  RENDER_MODE_NONE,
  RENDER_MODE_FARFIELD,
  RENDER_MODE_NEARFIELD,
  RENDER_MODE_STRUCTURE,
  RENDER_MODE_COUNT
} render_mode_t;

/* Name each precondition outcome a frame can report. */
typedef enum
{
  RENDER_OK,
  RENDER_SUPPRESS,
  RENDER_NO_RP_CARD,
  RENDER_NO_NF_CARD,
  RENDER_NF_NOT_READY,
  RENDER_NO_NF_FIELD,
  RENDER_NO_DATA,
  RENDER_NO_GEOMETRY,
  RENDER_NO_MODE,
  RENDER_STATUS_COUNT
} render_status_t;

/* Carry the resolved mode, status, frequency step, and overlay routing. */
typedef struct
{
  render_status_t  status;
  render_mode_t    mode;
  int              fstep;
  const char      *message;
  gboolean         overlay_active;
} render_check_result_t;

/**
 * render_check() - Resolve frame preconditions and presentation mode
 * @view_type: view whose content is being presented
 *
 * Returns a result with status RENDER_OK when every precondition passes.
 * On failure, message names the status text the caller presents.
 */
render_check_result_t render_check(view_type_t view_type);

#endif /* __RENDER_CHECK_H */
