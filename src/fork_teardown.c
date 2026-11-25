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

#include <poll.h>
#include <sys/wait.h>
#include <time.h>

#include "common.h"
#include "shared.h"

/* Total wall-clock budget the parent spends quiescing every forked child at
 * teardown.  Once spent, any straggler is SIGKILLed so quitting never blocks. */
#define CHILD_REAP_BUDGET_MS 500

/* deadline_remaining_ms()
 *
 * Milliseconds left until the shared teardown deadline, clamped at zero so an
 * elapsed budget yields no remaining time rather than a negative timeout.
 */
  static int
deadline_remaining_ms( const struct timespec *deadline )
{
  struct timespec now;
  clock_gettime( CLOCK_MONOTONIC, &now );

  long ms = (deadline->tv_sec  - now.tv_sec)  * 1000L
          + (deadline->tv_nsec - now.tv_nsec) / 1000000L;

  if( ms < 0 )
    ms = 0;

  return (int)ms;

} /* deadline_remaining_ms() */

/*-----------------------------------------------------------------------*/

/* drain_and_reap()
 *
 * Quiesces one forked child within the shared deadline.  Draining the result
 * pipe lets a child blocked mid-write in Write_Pipe complete, loop to its
 * command read, see the closed command pipe as EOF, and exit through
 * child_exit(); the zombie is then reaped.  When the budget is spent, SIGKILL
 * forces exit and is reaped at once so no zombie survives the teardown pass.
 */
  static void
drain_and_reap( child_proc_t *child, const struct timespec *deadline )
{
  struct pollfd pfd = { .fd = child->from_child[READ], .events = POLLIN };
  char drain[256];
  gboolean draining = TRUE;
  gboolean reaping  = TRUE;

  /* Drain pending result data until EOF, error, or the budget is spent. */
  while( draining )
  {
    int remaining = deadline_remaining_ms( deadline );
    if( remaining == 0 )
    {
      draining = FALSE;
      continue;
    }

    int ready = poll( &pfd, 1, remaining );
    if( ready <= 0 )            /* timeout or poll error: stop draining */
    {
      draining = FALSE;
      continue;
    }

    ssize_t r = read( child->from_child[READ], drain, sizeof(drain) );
    if( r > 0 )
      continue;                 /* more data pending */
    if( r < 0 && errno == EINTR )
      continue;                 /* interrupted: retry */

    draining = FALSE;           /* EOF (r == 0) or read error */
  }

  /* Reap within the remaining budget; force-kill once it is spent. */
  while( reaping )
  {
    pid_t reaped = waitpid( child->pid, NULL, WNOHANG );
    if( reaped == child->pid || reaped < 0 )  /* exited, or no such child */
    {
      reaping = FALSE;
      continue;
    }

    if( deadline_remaining_ms( deadline ) == 0 )
    {
      kill( child->pid, SIGKILL );
      waitpid( child->pid, NULL, 0 );  /* SIGKILL terminates at once */
      reaping = FALSE;
      continue;
    }

    poll( NULL, 0, 5 );         /* brief yield before re-checking exit */
  }

} /* drain_and_reap() */

/*-----------------------------------------------------------------------*/

/* child_procs_free()
 *
 * Reaps each forked child, then releases each child-process slot struct and the
 * child_procs array.
 */
  void
child_procs_free( void )
{
  size_t i, n = mem_array_count( child_procs );

  /* One shared budget bounds the whole teardown pass: the parent spends at most
   * CHILD_REAP_BUDGET_MS quiescing every child combined, then SIGKILLs any
   * straggler so quitting never blocks on a wedged worker. */
  struct timespec deadline;
  clock_gettime( CLOCK_MONOTONIC, &deadline );
  deadline.tv_nsec += CHILD_REAP_BUDGET_MS * 1000000L;
  deadline.tv_sec  += deadline.tv_nsec / 1000000000L;
  deadline.tv_nsec %= 1000000000L;

  /* Only the parent reaps: it owns the forked children (pid > 0), so its report
   * flushes before the slot is released; the serial in-process slot carries
   * pid 0 and is skipped.  The child inherited this array across fork() and
   * frees its slots without reaping siblings it does not own; !CHILD
   * short-circuits before the NULL post-fork slots. */
  for( i = 0; i < n; i++ )
  {
    if( !CHILD && child_procs[i]->pid > 0 )
      drain_and_reap( child_procs[i], &deadline );

    mem_free( &child_procs[i] );
  }

  mem_array_free( &child_procs );

} /* child_procs_free() */
