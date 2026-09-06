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

/* Modifier axis of scroll capabilities.  Dispatch walks it in ordinal order,
 * so a chord holding both modifiers requests the earlier one. */
typedef enum
{
  SURFACE_MOD_CTRL = 0,
  SURFACE_MOD_SHIFT,
  SURFACE_MOD_COUNT
} surface_modifier_t;

/* One scroll capability: the handler applying it bound to the modifier
 * requesting it and the notice advertising it.  The capability owns the
 * guard retiring its notice, so rows borrowing one capability advertise it
 * once between them. */
typedef struct
{
  gboolean (*handler)(GdkEventScroll *event, render_surface_t *surface);

  /* Modifier the capability answers, stated once here so every row offering
   * the capability agrees with the chord its notice advertises */
  surface_modifier_t modifier;

  /* Text advertising the capability, presented on the first frame of the
   * session whose engine resolves the subject the capability acts upon */
  const char *notice;

  /* Subject the capability acts upon, which the presenting engine gates on */
  surface_cap_subject_t subject;

  /* Raised once the notice has been advertised for this session */
  gboolean notice_shown;

} surface_capability_t;

/**
 * surface_notice_capabilities() - Advertise capabilities acting on a subject
 * @surface: surface whose engine has resolved the subject
 * @subject: subject the calling engine brought into a reportable state
 *
 * Presents the notice of every capability the surface offers for @subject,
 * at most once per capability for the session, through the engine the
 * surface binds.
 */
void surface_notice_capabilities(render_surface_t *surface,
    surface_cap_subject_t subject);

/**
 * surface_input_connect() - Wire pointer, scroll and allocation handlers
 * @surface: surface whose presented widget receives the events
 *
 * Every handler mutates the borrowed view, so a surface showing none carries
 * no pointer behavior and is left unwired.
 */
void surface_input_connect(render_surface_t *surface);

#endif /* __RENDER_SURFACE_INPUT_H */
