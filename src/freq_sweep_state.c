#include "shared.h"

/* One row per state, holding that state's full specification: its successor
 * under each of the six transition verbs, the three cross-cutting bits the
 * state answers with, and its diagnostic name.  A successor lives only in the
 * row of the state it leaves, so each transition is described in exactly one
 * place.
 *
 * The three bits are independent readings of one lifecycle: a driver owns the
 * sweep, the result set is full, any results are retained.  The two remaining
 * conditions are state identities rather than bits: armed announces a pending
 * start, stopping announces that the owning driver must retire. */
typedef struct {
  freq_sweep_state_t  arm;
  freq_sweep_state_t  run_begin;
  freq_sweep_state_t  run_end;
  freq_sweep_state_t  publish;
  freq_sweep_state_t  clear;
  freq_sweep_state_t  stop_request;
  gboolean            active;
  gboolean            complete;
  gboolean            has_results;
  const char         *name;
} freq_sweep_row_t;

_Atomic freq_sweep_state_t sweep_state = FREQ_SWEEP_IDLE;

static const freq_sweep_row_t freq_sweep_rows[FREQ_SWEEP_STATE_COUNT] = {
  [FREQ_SWEEP_IDLE] = {
    .arm          = FREQ_SWEEP_ARMED,     .run_begin    = FREQ_SWEEP_ACTIVE,
    .run_end      = FREQ_SWEEP_IDLE,      .publish      = FREQ_SWEEP_COMPLETE,
    .clear        = FREQ_SWEEP_IDLE,      .stop_request = FREQ_SWEEP_IDLE,
    .active       = FALSE,                .complete     = FALSE,
    .has_results  = FALSE,                .name         = "idle" },
  [FREQ_SWEEP_ARMED] = {
    .arm          = FREQ_SWEEP_ARMED,     .run_begin    = FREQ_SWEEP_ACTIVE,
    .run_end      = FREQ_SWEEP_ARMED,     .publish      = FREQ_SWEEP_COMPLETE,
    .clear        = FREQ_SWEEP_IDLE,      .stop_request = FREQ_SWEEP_IDLE,
    .active       = FALSE,                .complete     = FALSE,
    .has_results  = FALSE,                .name         = "armed" },
  [FREQ_SWEEP_ACTIVE] = {
    .arm          = FREQ_SWEEP_ACTIVE,    .run_begin    = FREQ_SWEEP_ACTIVE,
    .run_end      = FREQ_SWEEP_PAUSED,    .publish      = FREQ_SWEEP_FINISHING,
    .clear        = FREQ_SWEEP_ACTIVE,    .stop_request = FREQ_SWEEP_STOPPING,
    .active       = TRUE,                 .complete     = FALSE,
    .has_results  = TRUE,                 .name         = "active" },
  [FREQ_SWEEP_STOPPING] = {
    .arm          = FREQ_SWEEP_STOPPING,  .run_begin    = FREQ_SWEEP_STOPPING,
    .run_end      = FREQ_SWEEP_PAUSED,    .publish      = FREQ_SWEEP_FINISHING,
    .clear        = FREQ_SWEEP_STOPPING,  .stop_request = FREQ_SWEEP_STOPPING,
    .active       = TRUE,                 .complete     = FALSE,
    .has_results  = TRUE,                 .name         = "stopping" },
  [FREQ_SWEEP_FINISHING] = {
    .arm          = FREQ_SWEEP_FINISHING, .run_begin    = FREQ_SWEEP_FINISHING,
    .run_end      = FREQ_SWEEP_COMPLETE,  .publish      = FREQ_SWEEP_FINISHING,
    .clear        = FREQ_SWEEP_ACTIVE,    .stop_request = FREQ_SWEEP_FINISHING,
    .active       = TRUE,                 .complete     = TRUE,
    .has_results  = TRUE,                 .name         = "finishing" },
  [FREQ_SWEEP_PAUSED] = {
    .arm          = FREQ_SWEEP_PAUSED,    .run_begin    = FREQ_SWEEP_ACTIVE,
    .run_end      = FREQ_SWEEP_PAUSED,    .publish      = FREQ_SWEEP_COMPLETE,
    .clear        = FREQ_SWEEP_IDLE,      .stop_request = FREQ_SWEEP_PAUSED,
    .active       = FALSE,                .complete     = FALSE,
    .has_results  = TRUE,                 .name         = "paused" },
  [FREQ_SWEEP_COMPLETE] = {
    .arm          = FREQ_SWEEP_COMPLETE,  .run_begin    = FREQ_SWEEP_FINISHING,
    .run_end      = FREQ_SWEEP_COMPLETE,  .publish      = FREQ_SWEEP_COMPLETE,
    .clear        = FREQ_SWEEP_IDLE,      .stop_request = FREQ_SWEEP_COMPLETE,
    .active       = FALSE,                .complete     = TRUE,
    .has_results  = TRUE,                 .name         = "complete" },
};

/**
 * freq_sweep_row - transition row of the current sweep state
 *
 * Loads the state once so the caller's successor and the reported name come
 * from a single snapshot.  An out-of-range value is reported and answered
 * with the idle row, leaving the caller a defined successor.
 *
 * Returns: row of the state observed by this call
 */
static const freq_sweep_row_t *
freq_sweep_row( void )
{
  freq_sweep_state_t state = sweep_state;

  if( (unsigned)state >= (unsigned)FREQ_SWEEP_STATE_COUNT )
  {
    BUG("freq_sweep_row: invalid sweep_state=%d\n", (int)state);
    return( &freq_sweep_rows[FREQ_SWEEP_IDLE] );
  }

  return( &freq_sweep_rows[state] );
}

/**
 * freq_sweep_arm - announce that a sweep start is pending
 *
 * Raised by the deck load so the frequency hook invoked on the way out does
 * not start a sweep of its own; released by the start it announces or by the
 * explicit clear at the end of the load.
 */
void
freq_sweep_arm( void )
{
  sweep_state = freq_sweep_row()->arm;
}

/**
 * freq_sweep_run_begin - record that a driver owns the sweep
 *
 * A sweep started after a completed sweep retains that result set until the
 * step invalidation that precedes a full restart clears it.
 */
void
freq_sweep_run_begin( void )
{
  sweep_state = freq_sweep_row()->run_begin;
}

/**
 * freq_sweep_run_end - record that the driver has retired
 *
 * A published result set survives, so cancelling a finished sweep leaves its
 * results readable.  A sweep retired with work outstanding becomes paused and
 * retains the steps it computed.
 */
void
freq_sweep_run_end( void )
{
  sweep_state = freq_sweep_row()->run_end;
}

/**
 * freq_sweep_results_publish - mark the result set full
 *
 * Called while the driver still owns the sweep; readers gated on a full
 * result set observe the results from this point on.
 */
void
freq_sweep_results_publish( void )
{
  sweep_state = freq_sweep_row()->publish;
}

/**
 * freq_sweep_results_clear - mark every result gone
 *
 * Called from the site that empties save.fstep[], when a new deck is loaded,
 * and at the end of a load to release an arm no sweep consumed.
 */
void
freq_sweep_results_clear( void )
{
  sweep_state = freq_sweep_row()->clear;
}

/**
 * freq_sweep_stop_request - ask the owning driver to retire
 *
 * A request raised while no driver owns the sweep is a self-transition, so
 * nothing latches and the next sweep starts unencumbered.
 */
void
freq_sweep_stop_request( void )
{
  sweep_state = freq_sweep_row()->stop_request;
}

/**
 * freq_sweep_active - report whether a driver owns the sweep
 *
 * Returns: TRUE while a driver is iterating the sweep
 */
gboolean
freq_sweep_active( void )
{
  return( freq_sweep_row()->active );
}

/**
 * freq_sweep_complete - report whether the full result set is valid
 *
 * Returns: TRUE when every sweep step has been computed
 */
gboolean
freq_sweep_complete( void )
{
  return( freq_sweep_row()->complete );
}

/**
 * freq_sweep_has_results - report whether any results are retained
 *
 * Returns: TRUE when the sweep holds results a presentation layer may draw
 */
gboolean
freq_sweep_has_results( void )
{
  return( freq_sweep_row()->has_results );
}

/**
 * freq_sweep_paused - report a retired sweep with work remaining
 *
 * Derived from the three bits: results are retained, no driver owns them and
 * the set is not full, which is reachable only by retiring a running sweep.
 *
 * Returns: TRUE when a resume has steps left to compute
 */
gboolean
freq_sweep_paused( void )
{
  const freq_sweep_row_t *row = freq_sweep_row();

  return( row->has_results && !row->active && !row->complete );
}

/**
 * freq_sweep_armed - report whether a sweep start is pending
 *
 * Returns: TRUE between the announcement of a start and the start itself
 */
gboolean
freq_sweep_armed( void )
{
  return( sweep_state == FREQ_SWEEP_ARMED );
}

/**
 * freq_sweep_stopping - report whether the owning driver must retire
 *
 * Returns: TRUE once a stop request has reached a driver-owned sweep
 */
gboolean
freq_sweep_stopping( void )
{
  return( sweep_state == FREQ_SWEEP_STOPPING );
}

/**
 * freq_sweep_state_name - diagnostic name of the current sweep state
 *
 * Returns: static string naming the state observed by this call
 */
const char *
freq_sweep_state_name( void )
{
  return( freq_sweep_row()->name );
}
