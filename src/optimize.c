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

#include "shared.h"

/*------------------------------------------------------------------------*/

// Dispatch table entry for optimizer save operations
typedef struct
{
	int *opt_flag;
	char **filename;
	char *suffix;
	char *debug_name;
	void (*save_fn)(char *);
} optimizer_save_entry_t;

static optimizer_save_entry_t save_entries[] =
{
	{ &rc_config.opt_write_csv, &rc_config.filename_csv,
	  ".csv", NULL, Save_FreqPlots_CSV },
	{ &rc_config.opt_write_s1p, &rc_config.filename_s1p,
	  ".s1p", NULL, Save_FreqPlots_S1P },
	{ &rc_config.opt_write_s2p_max_gain, &rc_config.filename_s2p_max_gain,
	  "-maxgain.s2p", NULL, Save_FreqPlots_S2P_Max_Gain },
	{ &rc_config.opt_write_s2p_viewer_gain, &rc_config.filename_s2p_viewer_gain,
	  "-viewergain.s2p", NULL, Save_FreqPlots_S2P_Viewer_Gain },
	{ &rc_config.opt_write_rdpat, &rc_config.filename_rdpat,
	  "-radpattern.csv", "rdpat", Save_RadPattern_CSV },
	{ &rc_config.opt_write_currents, &rc_config.filename_currents,
	  "-currents.csv", "currents", Save_Currents_CSV },
	{ &rc_config.opt_write_gnuplot_structure, &rc_config.filename_gnuplot_structure,
	  "-structure.gplot", "gnuplot structure", Save_Struct_Gnuplot_Data },
	{ &rc_config.opt_write_patch_currents, &rc_config.filename_patch_currents,
	  "-patch-currents.csv", "patch currents", Save_Patch_Currents_CSV },
	{ NULL, NULL, NULL, NULL, NULL }
};

int opt_have_files_to_save(void)
{
	for (optimizer_save_entry_t *e = save_entries; e->save_fn != NULL; e++)
	{
		if ((e->opt_flag && *e->opt_flag) || (e->filename && *e->filename))
			return 1;
	}
	return 0;
}

/* Writes out frequency-dependent
 * data for the external Optimizer */
void
_Write_Optimizer_Data(void)
{
	char filename[FILENAME_LEN];
	size_t n = sizeof(filename);

	for (optimizer_save_entry_t *e = save_entries; e->save_fn != NULL; e++)
	{
		// Menu checkbox path: auto-generate filename from input_file
		if (e->opt_flag && *e->opt_flag)
			e->save_fn(str_append(filename, rc_config.input_file, e->suffix, n));

		// CLI path: use user-specified filename
		if (e->filename && *e->filename)
		{
			if (e->debug_name)
				pr_debug("saving %s: %s\n", e->debug_name, *e->filename);
			e->save_fn(*e->filename);
		}
	}
} // Write_Optimizer_Data()

void Write_Optimizer_Data( void )
{
	g_rec_mutex_lock(&freq_data_lock);
	_Write_Optimizer_Data();
	g_rec_mutex_unlock(&freq_data_lock);
}


// If inotify was not detected then create stubs:
#ifndef HAVE_INOTIFY

void *Optimizer_Output( void *arg ) { pr_err("xnec2c was built without inotify.\n"); return NULL; }
void optimizer_output_start( void ) { }
void optimizer_output_stop( void ) { }

// Otherwise, normal optimize code:
#else

#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/stat.h>

/* File extensions to watch - both trigger the same reload handler */
static const char *watch_extensions[] = { ".nec", ".sy", NULL };

/* External optimizer watcher thread handle and its private run flag.  The flag
 * is the watcher's sole control: optimizer_output_stop() clears it and joins.
 * The watcher owns SUPPRESS_INTERMEDIATE_REDRAWS while it runs: start sets it,
 * stop clears it, so the render flag tracks the active optimization. */
static pthread_t optimizer_output_thread;
static _Atomic gboolean optimizer_output_running = FALSE;

/* eventfd used to wake the watcher's blocked poll() at stop, so the join does
 * not wait out an inotify quiet period. */
static int optimizer_output_wakefd = -1;

/* optimizer_output_release()
 *
 * Closes the wake eventfd, resets its handle, and clears the render-suppression
 * flag.  Called by stop(), the start() thread-create failure path, and the
 * watcher's own pre-loop abort.
 */
static void
optimizer_output_release( void )
{
  if( optimizer_output_wakefd >= 0 )
    close( optimizer_output_wakefd );
  optimizer_output_wakefd = -1;
  ClearFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
}

/* Derive companion filename by replacing extension
 * Returns static buffer - not thread safe, caller must use immediately
 */
static const char *get_companion_basename(const char *nec_basename, const char *new_ext)
{
  static char companion[PATH_MAX];
  const char *dot = strrchr(nec_basename, '.');

  if (dot == NULL)
  {
    snprintf(companion, sizeof(companion), "%s%s", nec_basename, new_ext);
  }
  else
  {
    size_t base_len = dot - nec_basename;
    snprintf(companion, sizeof(companion), "%.*s%s", (int)base_len, nec_basename, new_ext);
  }

  return companion;
}

/* Check if event filename matches any watched file
 * Returns TRUE if filename matches .nec or .sy variant
 */
static gboolean is_watched_file(const char *event_name, const char *nec_basename)
{
  gboolean matched = FALSE;

  if (event_name == NULL || nec_basename == NULL)
  {
    matched = FALSE;
  }
  else if (strcmp(event_name, nec_basename) == 0)
  {
    matched = TRUE;
  }
  else
  {
    for (const char **ext = watch_extensions; *ext != NULL && !matched; ext++)
    {
      if (strcmp(*ext, ".nec") != 0)
      {
        const char *companion = get_companion_basename(nec_basename, *ext);
        if (strcmp(event_name, companion) == 0)
        {
          pr_debug("inotify: matched companion file: %s\n", companion);
          matched = TRUE;
        }
      }
    }
  }

  return matched;
}

int inotify_open(struct pollfd *pfd)
{
  int wd, fd;

  /* Create the file descriptor for accessing the inotify API. */
  fd = inotify_init1( IN_NONBLOCK );
  if( fd == -1 )
  {
    perror( "inotify_init1" );
    exit( -1 );
  }

  /* Watch directory instead of file to handle atomic writes (editors use temp file + rename).
   * Watching the file inode directly fails when the inode is replaced. */
  char watch_dir[PATH_MAX];
  char *last_slash = strrchr(rc_config.input_file, '/');
  if( last_slash )
  {
    size_t dir_len = last_slash - rc_config.input_file;
    memcpy(watch_dir, rc_config.input_file, dir_len);
    watch_dir[dir_len] = '\0';
  }
  else
  {
    strcpy(watch_dir, ".");
  }

  /* Create a directory watch descriptor. Track MODIFY, CLOSE_WRITE, and MOVED_TO. */
  wd = inotify_add_watch( fd, watch_dir, IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO );
  if( wd == -1 )
  {
    pr_err("Unable to configure file modification detection to %s: %s\n", watch_dir,
           strerror(errno));
    return -1;
  }
  else
	  pr_debug("Monitoring directory for %s: %s\n", rc_config.input_file, watch_dir);

  pfd->fd     = fd;     /* Inotify input */
  pfd->events = POLLIN;

  return fd;
}

/* wait_for_nonempty_file()
 *
 * Polls until the watched input file exists with non-zero size so a reload
 * does not race a momentarily-truncated file mid-save.  Returns FALSE when the
 * run flag clears, abandoning the reload so the watcher exits promptly. */
static gboolean
wait_for_nonempty_file( struct stat *st )
{
  while( optimizer_output_running )
  {
    errno = 0;
    if( stat(rc_config.input_file, st) == 0 && st->st_size > 0 )
      return TRUE;

    pr_warn("inotify: file stat failed or zero bytes (errno=%s), retrying\n", strerror(errno));
    usleep(100000);
  }

  return FALSE;
}

/* Optimizer_Output()
 *
 * Watches the NEC2 (.nec) input file for modifications using inotify and
 * re-reads the file for re-processing when changes are detected.
 *
 * INOTIFY FILE MODIFICATION DETECTION CHALLENGE:
 *
 * Different editors use different save strategies that generate different
 * inotify event sequences. We must handle both atomic writes and direct
 * writes without false positives from intermediate states.
 *
 * EDITOR BEHAVIORS:
 *
 * Pluma/Gedit (atomic write via temp file + rename):
 *   1. CLOSE_WRITE on target (old inode closing - STALE DATA)
 *   2. MOVED_TO on target (rename complete - FRESH DATA)
 *   3. CLOSE_WRITE on target (spurious, skipped by an active sweep)
 *
 * Vi (atomic write via backup + in-place modification):
 *   1. MODIFY on target (content changing)
 *   2. MODIFY on target (continued writes)
 *   3. CLOSE_WRITE on target (write complete - FRESH DATA)
 *
 * Echo/printf (direct write):
 *   1. MODIFY on target (content changing)
 *   2. CLOSE_WRITE on target (write complete - FRESH DATA)
 *
 * THE PROBLEM:
 *
 * IN_CLOSE_WRITE is ambiguous - it fires both when:
 *   a) The old inode closes (before atomic rename) - contains STALE data
 *   b) A direct write completes - contains FRESH data
 *
 * Reading on first CLOSE_WRITE causes reload with stale data when using
 * editors that perform atomic writes (Pluma, Vi, most modern editors).
 *
 * SOLUTION - STATE MACHINE:
 *
 * Track whether IN_MODIFY occurred for the target file. This distinguishes:
 *   - Spurious CLOSE_WRITE (no prior MODIFY) = old inode closing, SKIP
 *   - Real CLOSE_WRITE (after MODIFY) = direct write complete, RELOAD
 *   - IN_MOVED_TO (anytime) = atomic write complete, RELOAD
 *
 * State transitions:
 *   modify_seen = FALSE (initial state)
 *   IN_MODIFY        → modify_seen = TRUE, continue (don't reload yet)
 *   IN_MOVED_TO      → RELOAD, modify_seen = FALSE
 *   IN_CLOSE_WRITE   → if modify_seen: RELOAD, modify_seen = FALSE
 *                       else: SKIP (spurious close from old inode)
 *
 * This ensures we only reload when file content is actually complete and
 * consistent, regardless of which editor or save strategy is used.
 */
  void *
Optimizer_Output( void *arg )
{
  int fd = -1, poll_num;
  struct pollfd pfds[2];
  char buf[256] __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;
  char *ptr;

  /* State machine: track whether target file was modified (vs just closed) */
  gboolean modify_seen = FALSE;

  char prev_input_file[sizeof(rc_config.input_file)] = {0};

  /* Second poll slot watches the wake eventfd so stop() can interrupt poll. */
  pfds[1].fd     = optimizer_output_wakefd;
  pfds[1].events = POLLIN;

  /* Wait for watch events */
  while( TRUE )
  {
	// Re-open the inotify handle if the filename changes.  This works
	// for the first iteration too because prev_input_file is empty:
	if (strcmp(prev_input_file, rc_config.input_file))
    {
      if (fd > 0) close( fd );

      fd = inotify_open( &pfds[0] );
      if (fd < 0)
      {
        /* Watcher aborts before the loop; no stop() will reconcile this exit,
         * so release what optimizer_output_start() allocated. */
        optimizer_output_running = FALSE;
        optimizer_output_release();
        return( NULL );
      }

      Strlcpy(prev_input_file, rc_config.input_file, sizeof(prev_input_file));
	}

    // Exit thread when the watcher run flag has been cleared
    if( !optimizer_output_running )
    {
      pr_debug("Exited optimizer thread.\n");
      break;
    }

    if ( CHILD )
	{
		pr_info("optimize.c: exit because this is a child process.\n");
		break;
	}

    /* Block on inotify and the stop-wake eventfd; the 1 s timeout re-checks
     * rc_config.input_file for a watch-directory change. */
    poll_num = poll( pfds, 2, 1000 );

    if (poll_num == -1)
    {
      if( errno == EINTR ) continue;
      perror( "poll" );
      exit( -1 );
    }

    /* Stop wake: drain the eventfd and re-check the run flag at the loop top. */
    if( poll_num > 0 && (pfds[1].revents & POLLIN) )
    {
      uint64_t wake;
      ssize_t wr = read( optimizer_output_wakefd, &wake, sizeof(wake) );
      (void)wr;
      continue;
    }

    if( poll_num > 0 )
    {
      if( pfds[0].revents & POLLIN )
      {
        /* Inotify events are available. Read some events. */
        len = read( fd, buf, sizeof(buf) );
        if( (len == -1) && (errno != EAGAIN) )
        {
          perror( "read" );
          exit( -1 );
        }

		pr_debug("inotify: read %zd bytes from inotify fd\n", len);

		if ( isFlagSet(INPUT_PENDING | SY_OPTIMIZER_ACTIVE) || sweep_state != FREQ_SWEEP_COMPLETE)
		{
			pr_debug("inotify: SKIP all events (flags) - INPUT_PENDING=%d sweep_state=%s\n",
				isFlagSet(INPUT_PENDING) ? 1 : 0,
				freq_sweep_state_name());
			continue;
		}

		const char *target_filename = strrchr(rc_config.input_file, '/');
		target_filename = target_filename ? target_filename + 1 : rc_config.input_file;

		for (ptr = buf; ptr < buf + len; )
		{
			struct stat st;
			gboolean flag;

			event = (const struct inotify_event *) ptr;

			pr_debug("inotify: EVENT @offset=%ld: mask=0x%08x cookie=%u len=%u name='%s'\n",
				(long)(ptr - buf), event->mask, event->cookie, event->len,
				event->len ? event->name : "(none)");

			pr_debug("inotify: EVENT flags: IN_CLOSE_WRITE=%d IN_MOVED_TO=%d IN_MOVED_FROM=%d IN_CREATE=%d IN_DELETE=%d IN_MODIFY=%d\n",
				(event->mask & IN_CLOSE_WRITE) ? 1 : 0,
				(event->mask & IN_MOVED_TO) ? 1 : 0,
				(event->mask & IN_MOVED_FROM) ? 1 : 0,
				(event->mask & IN_CREATE) ? 1 : 0,
				(event->mask & IN_DELETE) ? 1 : 0,
				(event->mask & IN_MODIFY) ? 1 : 0);

			pr_debug("inotify: FILTER: target='%s' event_name='%s' match=%d\n",
				target_filename,
				event->len ? event->name : "(none)",
				(event->len && is_watched_file(event->name, target_filename)) ? 1 : 0);

			if (!event->len || !is_watched_file(event->name, target_filename))
			{
				pr_debug("inotify: SKIP event (filename mismatch)\n");
				ptr += sizeof(struct inotify_event) + event->len;
				continue;
			}

			if (event->mask & IN_MODIFY)
			{
				pr_debug("inotify: IN_MODIFY on target, setting modify_seen=TRUE\n");
				modify_seen = TRUE;
				ptr += sizeof(struct inotify_event) + event->len;
				continue;
			}

			if (event->mask & IN_MOVED_TO)
			{
				pr_debug("inotify: IN_MOVED_TO on target (atomic write complete), triggering reload\n");

				if (wait_for_nonempty_file(&st))
				{
					flag = FALSE;
					g_idle_add( Open_Input_File, (gpointer) &flag );
				}

				modify_seen = FALSE;
				ptr += sizeof(struct inotify_event) + event->len;
				continue;
			}

			if (event->mask & IN_CLOSE_WRITE)
			{
				if (modify_seen)
				{
					pr_debug("inotify: IN_CLOSE_WRITE after modify_seen=TRUE (direct write complete), triggering reload\n");

					if (wait_for_nonempty_file(&st))
					{
						gboolean flag = FALSE;
						g_idle_add( Open_Input_File, (gpointer) &flag );
					}

					modify_seen = FALSE;
				}
				else
				{
					pr_debug("inotify: IN_CLOSE_WRITE without modify_seen (spurious close, old inode), skipping\n");
				}

				ptr += sizeof(struct inotify_event) + event->len;
				continue;
			}

			pr_debug("inotify: SKIP event (not MODIFY/CLOSE_WRITE/MOVED_TO, mask=0x%08x)\n", event->mask);
			ptr += sizeof(struct inotify_event) + event->len;
		}
      }
    } // if( poll_num > 0 )
  } // while( TRUE )

  /* Close inotify file descriptor. */
  if( fd >= 0) close( fd );
  return( NULL );
} // Optimizer_Output()

/*------------------------------------------------------------------------*/

/* optimizer_output_start()
 *
 * Creates the inotify watcher thread and stores its handle.  Idempotent: a
 * call while the watcher already runs is a no-op.
 */
  void
optimizer_output_start( void )
{
  if( optimizer_output_running )
    return;

  optimizer_output_running = TRUE;
  SetFlag( SUPPRESS_INTERMEDIATE_REDRAWS );

  optimizer_output_wakefd = eventfd( 0, EFD_CLOEXEC );
  if( optimizer_output_wakefd == -1 )
  {
    optimizer_output_running = FALSE;
    ClearFlag( SUPPRESS_INTERMEDIATE_REDRAWS );
    pr_err("failed to create optimizer wake eventfd\n");
    perror( "eventfd()" );
    exit( -1 );
  }

  int ret = pthread_create(
      &optimizer_output_thread, NULL, Optimizer_Output, NULL );
  if( ret != 0 )
  {
    optimizer_output_running = FALSE;
    optimizer_output_release();
    pr_err("failed to create Optimizer Output thread\n");
    perror( "pthread_create()" );
    exit( -1 );
  }

} /* optimizer_output_start() */

/*------------------------------------------------------------------------*/

/* optimizer_output_stop()
 *
 * Clears the run flag, wakes the watcher's blocked poll() via the eventfd, and
 * joins.  Idempotent: a no-op when the watcher is not running.
 */
  void
optimizer_output_stop( void )
{
  if( !optimizer_output_running )
    return;

  optimizer_output_running = FALSE;

  /* Wake the watcher so the join does not wait for an inotify event; the
   * cleared run flag makes it break at the loop top. */
  uint64_t wake = 1;
  ssize_t wr = write( optimizer_output_wakefd, &wake, sizeof(wake) );
  (void)wr;

  pthread_join( optimizer_output_thread, NULL );

  optimizer_output_release();

} /* optimizer_output_stop() */

#endif // HAVE_INOTIFY

/*------------------------------------------------------------------------*/

