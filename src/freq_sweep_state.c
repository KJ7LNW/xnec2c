#include "shared.h"

/* One row per state, holding that state's successor under each of the four
 * transition verbs.  A successor lives only in the row of the state it
 * leaves, so each transition is described in exactly one place.  Every row
 * reproduces the results the two former Flags bits gave for its state:
 * run_begin sets running, run_end clears it, publish sets done and clear
 * clears done. */
typedef struct {
  freq_sweep_state_t  run_begin;
  freq_sweep_state_t  run_end;
  freq_sweep_state_t  publish;
  freq_sweep_state_t  clear;
  const char         *name;
} freq_sweep_row_t;

_Atomic freq_sweep_state_t sweep_state = FREQ_SWEEP_IDLE;

static const freq_sweep_row_t freq_sweep_rows[FREQ_SWEEP_STATE_COUNT] = {
  [FREQ_SWEEP_IDLE] = {
    .run_begin = FREQ_SWEEP_ACTIVE,     .run_end = FREQ_SWEEP_IDLE,
    .publish   = FREQ_SWEEP_COMPLETE,   .clear   = FREQ_SWEEP_IDLE,
    .name      = "idle" },
  [FREQ_SWEEP_ACTIVE] = {
    .run_begin = FREQ_SWEEP_ACTIVE,     .run_end = FREQ_SWEEP_IDLE,
    .publish   = FREQ_SWEEP_FINISHING,  .clear   = FREQ_SWEEP_ACTIVE,
    .name      = "active" },
  [FREQ_SWEEP_FINISHING] = {
    .run_begin = FREQ_SWEEP_FINISHING,  .run_end = FREQ_SWEEP_COMPLETE,
    .publish   = FREQ_SWEEP_FINISHING,  .clear   = FREQ_SWEEP_ACTIVE,
    .name      = "finishing" },
  [FREQ_SWEEP_COMPLETE] = {
    .run_begin = FREQ_SWEEP_FINISHING,  .run_end = FREQ_SWEEP_COMPLETE,
    .publish   = FREQ_SWEEP_COMPLETE,   .clear   = FREQ_SWEEP_IDLE,
    .name      = "complete" },
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
 * freq_sweep_run_begin - record that a driver owns the sweep
 *
 * A sweep started after a completed sweep retains that result set until the
 * new sweep's first iteration clears it.
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
 * results readable.
 */
void
freq_sweep_run_end( void )
{
  sweep_state = freq_sweep_row()->run_end;
}

/**
 * freq_sweep_results_publish - mark the result set valid
 *
 * Called while the driver still owns the sweep; readers gated on the
 * completed state observe the results from this point on.
 */
void
freq_sweep_results_publish( void )
{
  sweep_state = freq_sweep_row()->publish;
}

/**
 * freq_sweep_results_clear - mark the result set invalid
 *
 * Called when a new sweep begins iterating and when a new deck is loaded.
 */
void
freq_sweep_results_clear( void )
{
  sweep_state = freq_sweep_row()->clear;
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
