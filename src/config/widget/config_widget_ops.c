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
 * config_widget_ops: datum-addressed operations over a field's presentations.
 *
 * One dispatcher resolves the datum, walks the live groups its binding
 * declares, applies the requirements those groups place on the named
 * operation, and hands each group to a fixed implementation.  Nothing here
 * knows a window name, a renderer, a panel or a frequency: what a group is
 * allowed to take part in arrives as a predicate the declaration owns.
 */

#include "config_widget_ops.h"
#include "config_widget_ops_target.h"
#include "config_widget_priv.h"

/* How one operation reaches the widgets of one live group. */
typedef void (*config_widget_reach_fn)(const config_widget_operation_t *op,
    const config_widget_group_view_t *gv, const void *operand);

/* One operation: how it reaches the widgets of one live group, and what it
 * does at each widget it reaches.  A whole-group operation names a reach
 * alone; a per-widget operation shares one reach and names its own at. */
struct config_widget_operation_s {
  config_widget_reach_fn reach;
  config_widget_visit_fn at;
};

/* One operation with its operand, so the group walk carries a request
 * rather than a pair of loose arguments. */
typedef struct {
  const config_widget_operation_t *operation;
  const void                      *operand;
} config_widget_request_t;

/*------------------------------------------------------------------------*/

static void config_widget_reach_readout(const config_widget_operation_t *op,
    const config_widget_group_view_t *gv, const void *operand);
static void config_widget_reach_members(const config_widget_operation_t *op,
    const config_widget_group_view_t *gv, const void *operand);

/*------------------------------------------------------------------------*/

const config_widget_operation_t config_widget_readout_operation =
  { .reach = config_widget_reach_readout };
const config_widget_operation_t config_widget_visible_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_visible };
const config_widget_operation_t config_widget_sensitive_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_sensitive };
const config_widget_operation_t config_widget_tooltip_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_tooltip };
const config_widget_operation_t config_widget_connect_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_connect };
const config_widget_operation_t config_widget_flush_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_flush };
const config_widget_operation_t config_widget_capture_operation =
  { .reach = config_widget_reach_members, .at = config_widget_target_capture };

/*------------------------------------------------------------------------*/

/** config_widget_reach_readout - project the datum's readout bytes onto one group
 * @op:      unused; the source is the one the binding declares
 * @gv:      a binding and one of its live groups
 * @operand: unused
 */
static void
config_widget_reach_readout(const config_widget_operation_t *op,
    const config_widget_group_view_t *gv, const void *operand)
{
  (void)op;
  (void)operand;

  if( gv->binding->tree->readout == NULL )
    BUG("a readout reached a binding declaring no readout source\n");
  else
    config_widget_group_project(gv, gv->binding->tree->readout);
}

/** config_widget_reach_members - carry an operation to each widget of one group
 * @op:      the operation, naming what it does at each widget
 * @gv:      a binding and one of its live groups
 * @operand: carried through unread
 */
static void
config_widget_reach_members(const config_widget_operation_t *op,
    const config_widget_group_view_t *gv, const void *operand)
{
  if( op->at == NULL )
    BUG("an operation reaching every widget names nothing to do there\n");
  else
    config_widget_walk_members(gv, op->at, operand);
}

/*------------------------------------------------------------------------*/

/** config_widget_group_admits - whether a group takes part in one operation
 * @g:         the group, which lists a requirement per operation it limits
 * @operation: what the group is being asked to do
 *
 * Return: TRUE when every requirement the group lists for @operation holds.
 */
static gboolean
config_widget_group_admits(const config_widget_group_t *g,
    const config_widget_operation_t *operation)
{
  const config_widget_participation_t *const *p;
  gboolean admits = TRUE;

  for( p = g->participation; (p != NULL) && (*p != NULL) && admits; p++ )
  {
    if( (*p)->operation != operation )
      continue;

    admits = (*p)->allowed();
  }

  return admits;
}

/** config_widget_group_admit - carry one request to one group that allows it
 * @gv:      a binding and one of its live groups
 * @operand: the request naming the operation and its operand
 */
static void
config_widget_group_admit(const config_widget_group_view_t *gv,
    const void *operand)
{
  const config_widget_request_t *request = operand;

  if( !config_widget_group_admits(gv->declaration, request->operation) )
    return;

  request->operation->reach(request->operation, gv, request->operand);
}

/** config_widget_dispatch - carry one operation to every presentation of a datum
 * @field:     address of a registered field
 * @operation: what each live group is asked to do
 * @operand:   carried through to the operation unread
 */
static void
config_widget_dispatch(const void *field,
    const config_widget_operation_t *operation, const void *operand)
{
  const config_widget_binding_t *b = config_widget_find(field);
  config_widget_request_t request =
    { .operation = operation, .operand = operand };

  if( b == NULL )
    BUG("a configuration operation named a field outside the registry\n");
  else
    config_widget_walk_groups(b, config_widget_group_admit, &request);
}

/*------------------------------------------------------------------------*/

void
config_widget_readout(const void *field)
{
  config_widget_dispatch(field, &config_widget_readout_operation, NULL);
}

/*------------------------------------------------------------------------*/

void
config_widget_set_visible(const void *field, gboolean visible)
{
  config_widget_dispatch(field, &config_widget_visible_operation, &visible);
}

/*------------------------------------------------------------------------*/

void
config_widget_set_sensitive(const void *field, gboolean sensitive)
{
  config_widget_dispatch(field, &config_widget_sensitive_operation, &sensitive);
}

/*------------------------------------------------------------------------*/

void
config_widget_set_tooltip(const void *field, const char *text)
{
  config_widget_dispatch(field, &config_widget_tooltip_operation, text);
}

/*------------------------------------------------------------------------*/

void
config_widget_connect(const void *field, const config_widget_signal_t *signal)
{
  config_widget_dispatch(field, &config_widget_connect_operation, signal);
}

/*------------------------------------------------------------------------*/

void
config_widget_flush(const void *field)
{
  config_widget_dispatch(field, &config_widget_flush_operation, NULL);
}

/*------------------------------------------------------------------------*/

void
config_widget_capture(const void *field)
{
  config_widget_dispatch(field, &config_widget_capture_operation, NULL);
}
