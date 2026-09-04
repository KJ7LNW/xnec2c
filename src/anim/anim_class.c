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
 * anim_class: registry of animated subjects and their presentation domains.
 *
 * The registry holds one domain and one immutable operation object per class.
 * It names no channel, effect, widget, target, or configuration field: each
 * class module folds its own rows into the three answers it publishes.
 */
#include "anim_class.h"
#include "anim_phase.h"
#include "far_field_anim.h"
#include "near_field_anim.h"
#include "structure_patch_anim.h"
#include "structure_segment_anim.h"
#include "../console.h"
#include "../render/render_canvas.h"
#include "../shared.h"

/* Bind one animated subject to its domain and its capability object */
typedef struct
{
  anim_domain_t           domain;
  const anim_class_ops_t *ops;
} anim_class_row_t;

typedef struct
{
  canvas_id_t canvas;
  gboolean  (*condition)(void);
} anim_target_t;

typedef struct
{
  const anim_target_t *targets;
} anim_domain_row_t;

static const anim_target_t structure_targets[] = {
  { .canvas = CANVAS_STRUCTURE, .condition = NULL },
  { .canvas = CANVAS_RDPATTERN, .condition = overlay_struct_active },
  { .canvas = CANVAS_NONE,      .condition = NULL }
};

static const anim_target_t rdpattern_targets[] = {
  { .canvas = CANVAS_RDPATTERN, .condition = NULL },
  { .canvas = CANVAS_NONE,      .condition = NULL }
};

static const anim_domain_row_t anim_domain_rows[] = {
  [ANIM_DOMAIN_STRUCTURE] = { .targets = structure_targets },
  [ANIM_DOMAIN_RDPATTERN] = { .targets = rdpattern_targets },
};

static const anim_class_row_t anim_class_rows[] = {
  [ANIM_CLASS_STRUCTURE_SEGMENT] = {
      .domain = ANIM_DOMAIN_STRUCTURE,
      .ops    = &structure_segment_anim_ops },
  [ANIM_CLASS_STRUCTURE_PATCH] = {
      .domain = ANIM_DOMAIN_STRUCTURE,
      .ops    = &structure_patch_anim_ops },
  [ANIM_CLASS_NEAR_FIELD] = {
      .domain = ANIM_DOMAIN_RDPATTERN,
      .ops    = &near_field_anim_ops },
  [ANIM_CLASS_FAR_FIELD] = {
      .domain = ANIM_DOMAIN_RDPATTERN,
      .ops    = &far_field_anim_ops },
};

_Static_assert(G_N_ELEMENTS(anim_domain_rows) == ANIM_DOMAIN_COUNT,
    "Animation domain rows must cover every domain");
_Static_assert(G_N_ELEMENTS(anim_class_rows) == ANIM_CLASS_COUNT,
    "Animation class rows must cover every class");

/**
 * anim_class_row() - Resolve the row registering @cls
 * @cls: animation class
 *
 * Returns the row, or NULL when @cls names no registered class.
 */
  static const anim_class_row_t *
anim_class_row(anim_class_t cls)
{
  if( cls < ANIM_CLASS_STRUCTURE_SEGMENT || cls >= ANIM_CLASS_COUNT )
  {
    BUG("Animation class %d lies outside [0, %d)\n", cls, ANIM_CLASS_COUNT);
    return NULL;
  }

  return &anim_class_rows[cls];
}

/**
 * anim_member_rows_vary() - Report whether any present member carries phase
 * @rows:  member rows of one class
 * @count: members in @rows
 *
 * Returns TRUE when one available member varies with phase.
 */
  gboolean
anim_member_rows_vary(const anim_member_row_t *rows, size_t count)
{
  gboolean varies = FALSE;
  size_t idx;

  for( idx = 0; idx < count && !varies; idx++ )
  {
    const anim_member_row_t *row = &rows[idx];

    varies = (row->available == NULL || row->available())
        && (row->varies_with_phase != NULL) && row->varies_with_phase();
  }

  return varies;
}

/**
 * anim_class_available() - Report whether @cls has authoritative content
 * @cls: animation class
 */
  gboolean
anim_class_available(anim_class_t cls)
{
  const anim_class_row_t *row = anim_class_row(cls);

  return (row != NULL) && row->ops->available();
}

/**
 * anim_class_presented() - Report whether selections present @cls
 * @cls: animation class
 */
  static gboolean
anim_class_presented(anim_class_t cls)
{
  const anim_class_row_t *row = anim_class_row(cls);

  return (row != NULL) && row->ops->presented();
}

/**
 * anim_class_varies_with_phase() - Report whether @cls carries phase
 * @cls: animation class
 */
  static gboolean
anim_class_varies_with_phase(anim_class_t cls)
{
  const anim_class_row_t *row = anim_class_row(cls);

  return (row != NULL) && row->ops->varies_with_phase();
}

/**
 * anim_class_active() - Report presented content under an open context
 * @cls: animation class
 */
  gboolean
anim_class_active(anim_class_t cls)
{
  return anim_class_available(cls) && anim_class_presented(cls)
      && anim_phase_active();
}

/**
 * anim_class_animating() - Report an active class that carries phase
 * @cls: animation class
 */
  static gboolean
anim_class_animating(anim_class_t cls)
{
  return anim_class_active(cls) && anim_class_varies_with_phase(cls);
}

/**
 * anim_class_phase_capable() - Report content that could carry phase
 * @cls: animation class
 *
 * Presentation and liveness stay out, so this answers both the closed-context
 * controls and the redraw a liveness edge owes.
 */
  static gboolean
anim_class_phase_capable(anim_class_t cls)
{
  return anim_class_available(cls) && anim_class_varies_with_phase(cls);
}

/**
 * anim_domain_available() - Report content held by one presentation domain
 * @domain: presentation domain routing classes to canvases
 *
 * Folds the classes the registry binds to @domain, so a caller asking about
 * one window reads class truth rather than restating model state.
 */
  gboolean
anim_domain_available(anim_domain_t domain)
{
  gboolean available = FALSE;
  anim_class_t cls;

  if( domain < ANIM_DOMAIN_STRUCTURE || domain >= ANIM_DOMAIN_COUNT )
  {
    BUG("Animation domain %d lies outside [0, %d)\n",
        domain, ANIM_DOMAIN_COUNT);
    return FALSE;
  }

  for( cls = ANIM_CLASS_STRUCTURE_SEGMENT;
       cls < ANIM_CLASS_COUNT && !available; cls++ )
    available = (anim_class_rows[cls].domain == domain)
        && anim_class_available(cls);

  return available;
}

/**
 * anim_any_available() - Report whether any class holds authoritative content
 *
 * Phase dependence stays out because the selections that carry phase live
 * inside the dialog this answer opens; folding them here would strand a
 * closed dialog whenever a phase-invariant selection is stored.
 */
  gboolean
anim_any_available(void)
{
  gboolean available = FALSE;
  anim_class_t cls;

  for( cls = ANIM_CLASS_STRUCTURE_SEGMENT;
       cls < ANIM_CLASS_COUNT && !available; cls++ )
    available = anim_class_available(cls);

  return available;
}

/**
 * anim_any_animating() - Report whether any class animates now
 */
  gboolean
anim_any_animating(void)
{
  gboolean animating = FALSE;
  anim_class_t cls;

  for( cls = ANIM_CLASS_STRUCTURE_SEGMENT;
       cls < ANIM_CLASS_COUNT && !animating; cls++ )
    animating = anim_class_animating(cls);

  return animating;
}

/**
 * anim_class_phase() - Resolve the phase @cls presents
 * @cls:         animation class
 * @frame_phase: phase the frame reads once
 */
  double
anim_class_phase(anim_class_t cls, double frame_phase)
{
  return anim_class_animating(cls) ? frame_phase : 0.0;
}

/** anim_mark_domain() - Mark the canvases one domain presents into */
static void
anim_mark_domain(anim_domain_t domain, gboolean *queued)
{
  const anim_target_t *target;

  if( domain < ANIM_DOMAIN_STRUCTURE || domain >= ANIM_DOMAIN_COUNT )
  {
    BUG("Animation domain %d lies outside [0, %d)\n", domain,
        ANIM_DOMAIN_COUNT);
    return;
  }

  for( target = anim_domain_rows[domain].targets;
       target->canvas != CANVAS_NONE; target++ )
  {
    gboolean admit = (target->condition == NULL) || target->condition();

    queued[target->canvas] = queued[target->canvas] || admit;
  }
}

/** anim_dispatch_fold() - Queue each selected class domain once */
static void
anim_dispatch_fold(gboolean (*selected)(anim_class_t))
{
  gboolean queued[CANVAS_RESERVED_COUNT] = { 0 };
  anim_class_t cls;
  canvas_id_t id;

  for( cls = ANIM_CLASS_STRUCTURE_SEGMENT; cls < ANIM_CLASS_COUNT; cls++ )
  {
    if( !selected(cls) )
      continue;

    anim_mark_domain(anim_class_rows[cls].domain, queued);
  }

  for( id = CANVAS_STRUCTURE; id < CANVAS_RESERVED_COUNT; id++ )
  {
    if( !queued[id] )
      continue;

    canvas_queue_redraw(id, TRUE);
  }
}

/** anim_dispatch_redraw() - Queue canvases for animating classes */
void
anim_dispatch_redraw(void)
{
  anim_dispatch_fold(anim_class_animating);
}

/** anim_dispatch_context_redraw() - Queue canvases across a liveness edge */
void
anim_dispatch_context_redraw(void)
{
  anim_dispatch_fold(anim_class_phase_capable);
}
