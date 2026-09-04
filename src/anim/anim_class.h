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

#ifndef ANIM_CLASS_H
#define ANIM_CLASS_H    1

#include "../common.h"

/* Name each animated subject: the entity owning the phasor phase resolves
 * against.  Channels and effects belong to the class module, never here. */
typedef enum
{
  ANIM_CLASS_STRUCTURE_SEGMENT = 0,
  ANIM_CLASS_STRUCTURE_PATCH,
  ANIM_CLASS_NEAR_FIELD,
  ANIM_CLASS_FAR_FIELD,
  ANIM_CLASS_COUNT
} anim_class_t;

/* Name the presentation domain routing a class to its canvases. */
typedef enum
{
  ANIM_DOMAIN_STRUCTURE = 0,
  ANIM_DOMAIN_RDPATTERN,
  ANIM_DOMAIN_COUNT
} anim_domain_t;

/* Answer the three questions the registry asks of one animated subject.
 * Content existence, current presentation, and phase dependence stay
 * separate answers, each resolved from authoritative model state. */
typedef struct
{
  gboolean (*available)(void);
  gboolean (*presented)(void);
  gboolean (*varies_with_phase)(void);
} anim_class_ops_t;

/* Answer availability and phase dependence for one channel or effect.
 * A NULL availability is unconditional within its class; a NULL phase
 * entry is phase-invariant. */
typedef struct
{
  gboolean (*available)(void);
  gboolean (*varies_with_phase)(void);
} anim_member_row_t;

/**
 * anim_member_rows_vary() - Report whether any present member carries phase
 * @rows:  member rows of one class
 * @count: members in @rows
 *
 * Returns TRUE when one available member varies with phase.
 */
gboolean anim_member_rows_vary(const anim_member_row_t *rows, size_t count);

/**
 * anim_class_available() - Report whether @cls has authoritative content
 * @cls: animation class
 */
gboolean anim_class_available(anim_class_t cls);

/**
 * anim_class_active() - Report presented content under an open context
 * @cls: animation class
 */
gboolean anim_class_active(anim_class_t cls);

/**
 * anim_domain_available() - Report content held by one presentation domain
 * @domain: presentation domain routing classes to canvases
 *
 * Folds the classes the registry binds to @domain, so a caller asking about
 * one window reads class truth rather than restating model state.
 */
gboolean anim_domain_available(anim_domain_t domain);

/**
 * anim_any_available() - Report whether any class holds authoritative content
 *
 * Folds content alone, without presentation, phase dependence, or liveness,
 * so it answers controls evaluated while the context is closed.
 */
gboolean anim_any_available(void);

/**
 * anim_any_animating() - Report whether any class animates now
 */
gboolean anim_any_animating(void);

/**
 * anim_class_phase() - Resolve the phase @cls presents
 * @cls:         animation class
 * @frame_phase: phase the frame reads once
 *
 * Returns @frame_phase for an animating class and zero otherwise, so a
 * quiescent class evaluates at its zero-phase reference.
 */
double anim_class_phase(anim_class_t cls, double frame_phase);

/** anim_dispatch_redraw() - Queue canvases for animating classes */
void anim_dispatch_redraw(void);

/** anim_dispatch_context_redraw() - Queue canvases across a liveness edge */
void anim_dispatch_context_redraw(void);

#endif /* ANIM_CLASS_H */
