#include <stdint.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "console.h"
#include "mem_track.h"

/* The registry spine is shared mutable state guarded by one mutex because
 * the parent allocates from both the GTK main thread and the frequency-loop
 * thread. The serial sequence is a lock-free atomic counter, drawn off the
 * registry mutex so a birth stamp never contends with a link or unlink. */
static mem_obj_t *mem_reg_head;
static _Atomic uint64_t mem_serial_seq;
static pthread_mutex_t mem_track_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Report-time snapshot record. mem_report copies the live registry into a
 * private array under the lock, then emits unlocked, so a concurrent free
 * never frees a node mid-report. Backtrace frames are deep-copied because
 * their owning block may be freed once the lock is released. The grouped
 * flag is report-derived state, held here rather than in the block header. */
struct mem_report_rec
{
	const char *birth_site;
	uint64_t serial;
	size_t size;
	size_t used;
	char **frames;
	int grouped;
};

/**
 * mem_track_register() - link a managed block into the live registry
 * @m: header of the freshly allocated block
 *
 * Link under mem_track_mutex and mark the header as registered.
 */
void mem_track_register(mem_obj_t *m)
{
	pthread_mutex_lock(&mem_track_mutex);

	m->reg_prev = NULL;
	m->reg_next = mem_reg_head;
	if (mem_reg_head != NULL)
		mem_reg_head->reg_prev = m;
	mem_reg_head = m;
	m->registered = true;

	pthread_mutex_unlock(&mem_track_mutex);
}

/**
 * mem_track_unregister() - splice a managed block out of the live registry
 * @m: header of the block being freed
 *
 * Ignore blocks that never joined the registry; otherwise splice under
 * mem_track_mutex and clear their membership.
 */
void mem_track_unregister(mem_obj_t *m)
{
	if (likely(!m->registered))
		return;

	pthread_mutex_lock(&mem_track_mutex);

	if (m->reg_prev != NULL)
		m->reg_prev->reg_next = m->reg_next;
	else
		mem_reg_head = m->reg_next;

	if (m->reg_next != NULL)
		m->reg_next->reg_prev = m->reg_prev;

	m->registered = false;
	pthread_mutex_unlock(&mem_track_mutex);
}

/**
 * mem_track_next_serial() - draw the next monotonic birth identity
 *
 * Stamped into a block at birth and carried across relocation, so the
 * serial names a block independent of its reuse-prone address.
 *
 * Return: a serial unique for the program lifetime
 */
uint64_t mem_track_next_serial(void)
{
	return atomic_fetch_add(&mem_serial_seq, 1) + 1;
}

/**
 * mem_report_dup_frames() - deep-copy a backtrace frame array for the snapshot
 * @frames: NULL-terminated birth backtrace owned by a live block, or NULL
 *
 * The owning block may be freed once the registry lock is released, taking
 * its backtrace allocation with it, so the report keeps an independent copy
 * to emit after unlocking. Uses raw allocation because the managed allocator
 * would re-enter the registry mutex held during the snapshot.
 *
 * Return: a freshly allocated NULL-terminated copy, or NULL
 */
static char **mem_report_dup_frames(char *const *frames)
{
	if (frames == NULL)
		return NULL;

	int n = 0;
	while (frames[n] != NULL)
		n++;

	char **copy = malloc((size_t)(n + 1) * sizeof(*copy));
	if (copy == NULL)
		return NULL;

	for (int i = 0; i < n; i++)
		copy[i] = strdup(frames[i]);
	copy[n] = NULL;

	return copy;
}

/**
 * mem_report() - enumerate every live managed block, triaged by birth site
 * @tag: label identifying the report point
 *
 * Snapshots the live registry into a private array under the mutex, releases
 * the lock, then emits, so a concurrent free never frees a node mid-report
 * and no stderr write is held under the allocator lock. Pass one emits one
 * aggregate line per first-seen birth site, grouping later records by
 * birth_site pointer equality and summing survivor count, bytes, and the
 * serial span; a totals line follows with live block, byte, and distinct-site
 * counts. Pass two emits one line per survivor and, when the survivor carries
 * a birth backtrace, one line per call-path frame. The forked compute child
 * runs this identical path.
 */
void mem_report(const char *tag)
{
	mem_obj_t *m;
	struct mem_report_rec *recs = NULL;
	unsigned long long count = 0;
	unsigned long long live_bytes = 0;
	unsigned long long distinct_sites = 0;

	/* Snapshot under the lock: raw allocation avoids re-entering the registry
	 * mutex the managed allocator would take. The list cannot be mutated while
	 * the lock is held, so node and backtrace pointers stay valid across the
	 * copy. */
	pthread_mutex_lock(&mem_track_mutex);

	for (m = mem_reg_head; m != NULL; m = m->reg_next)
		count++;

	if (count > 0)
	{
		recs = malloc((size_t)count * sizeof(*recs));
		if (recs == NULL)
		{
			pthread_mutex_unlock(&mem_track_mutex);
			BUG("mem_report: snapshot allocation of %llu records failed\n",
				count);
			return;
		}

		unsigned long long i = 0;
		for (m = mem_reg_head; m != NULL; m = m->reg_next, i++)
		{
			recs[i].birth_site = m->birth_site;
			recs[i].serial = m->serial;
			recs[i].size = m->size;
			recs[i].used = m->used;
			recs[i].frames = mem_report_dup_frames(m->backtrace);
			recs[i].grouped = 0;
			live_bytes += m->size;
		}
	}

	pthread_mutex_unlock(&mem_track_mutex);

	/* Pass one over the private snapshot: per first-seen birth site, count
	 * survivors, sum bytes, and span the serials. The grouped flag retires
	 * every later record sharing the site so the outer index visits each
	 * site once. */
	for (unsigned long long i = 0; i < count; i++)
	{
		if (recs[i].grouped)
			continue;

		unsigned long long site_count = 0;
		unsigned long long site_bytes = 0;
		uint64_t serial_min = recs[i].serial;
		uint64_t serial_max = recs[i].serial;
		for (unsigned long long j = i; j < count; j++)
		{
			if (recs[j].birth_site != recs[i].birth_site)
				continue;

			recs[j].grouped = 1;
			site_count++;
			site_bytes += recs[j].size;
			if (recs[j].serial < serial_min)
				serial_min = recs[j].serial;
			if (recs[j].serial > serial_max)
				serial_max = recs[j].serial;
		}
		distinct_sites++;

		pr_info("mem-report %s: site=%s count=%llu bytes=%llu serial=[%llu..%llu]\n",
			tag, recs[i].birth_site, site_count, site_bytes,
			(unsigned long long)serial_min, (unsigned long long)serial_max);
	}

	pr_info("mem-report %s: live_blocks=%llu live_bytes=%llu distinct_sites=%llu\n",
		tag, count, live_bytes, distinct_sites);

	/* Pass two: one line per survivor, then its captured birth call path. */
	for (unsigned long long i = 0; i < count; i++)
	{
		pr_info("mem-report %s: serial=%llu site=%s size=%lu used=%lu\n",
			tag, (unsigned long long)recs[i].serial, recs[i].birth_site,
			(unsigned long)recs[i].size, (unsigned long)recs[i].used);

		if (recs[i].frames == NULL)
			continue;

		for (int f = 0; recs[i].frames[f] != NULL; f++)
			pr_info("mem-report %s:   frame %d %s\n", tag, f, recs[i].frames[f]);
	}

	for (unsigned long long i = 0; i < count; i++)
	{
		if (recs[i].frames == NULL)
			continue;

		for (int f = 0; recs[i].frames[f] != NULL; f++)
			free(recs[i].frames[f]);
		free(recs[i].frames);
	}

	free(recs);
}
