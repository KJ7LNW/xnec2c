#include <stdbool.h>

#include "common.h"
#include "console.h"
#include "utils.h"
#include "mem_track.h"

_Static_assert(sizeof(mem_obj_t) <= MEM_HEADER_SIZE,
	"mem_obj_t exceeds MEM_HEADER_SIZE");

/**
 * _mem_validate_fail() - report corrupted mem_obj_t header via BUG
 * @ptr: user-facing pointer that failed validation
 * @base: computed base address of the mem_obj_t
 *
 * The header lives at (ptr - MEM_HEADER_SIZE).  Validates that
 * the stored ptr field matches the caller's pointer.
 *
 */
void _mem_validate_fail(void *ptr, void *base)
{
	mem_obj_t *m = (mem_obj_t *)base;

	BUG("mem_obj_from_ptr: ptr %p does not match m->ptr %p (base %p)\n",
		ptr, m->ptr, base);
	abort();
}

/**
 * mem_obj_alloc() - allocate aligned memory, optionally copying from a source
 * @data_src: read-only source for the prior-data copy and element width, or
 *            NULL for a fresh allocation with no data to preserve.
 * @birth_src: birth-identity predecessor supplying serial, birth site, and
 *             backtrace ownership; non-NULL only on a grow-beyond relocation.
 *             NULL records @site and, when reporting is enabled, mints a
 *             fresh serial and captures a backtrace.
 * @req: requested user-data size in bytes
 * @prev_used: bytes of old data to preserve (0 for fresh allocation)
 *
 * Always allocates a new block; never modifies or frees @data_src or
 * @birth_src. Uses posix_memalign with MEM_ALIGNMENT-byte alignment.
 * Total allocation is MEM_HEADER_SIZE + req.
 *
 * Return: pointer to new mem_obj_t, or NULL on failure
 */
static inline mem_obj_t *mem_obj_alloc(const mem_obj_t *data_src,
	const mem_obj_t *birth_src, size_t req, size_t prev_used, const char *site)
{
	void *base = NULL;
	mem_obj_t *m;
	int rc;

	/* Minimum 1-byte user region so m->ptr stays within the allocation */
	if (req == 0)
		req = 1;

	rc = posix_memalign(&base, MEM_ALIGNMENT, MEM_HEADER_SIZE + req);
	if (unlikely(rc != 0 || base == NULL))
		return NULL;

	m = (mem_obj_t *)base;
	m->size = req;
	m->used = req;
	/* Single relocation-persistence point with two predecessor roles:
	 * data_src supplies element width and prior data; birth_src supplies the
	 * carried birth identity (serial, site, backtrace). A grow-beyond
	 * relocation passes the same block as both; a clone passes data_src only,
	 * minting a fresh identity. A fresh allocation passes neither and starts at
	 * the scalar/byte default while the array layer stamps width at birth. */
	m->array_elem_size = (data_src != NULL) ? data_src->array_elem_size : 0;
	/* Birth identity: a grow-beyond block inherits its predecessor's serial,
	 * site, and backtrace; a fresh block records the caller site and, only
	 * while reporting is enabled, mints a serial and captures a backtrace.
	 * When reporting is disabled a fresh block keeps serial 0 and a NULL
	 * backtrace, which the free path null-checks before releasing. Each field
	 * is written once so the registry link below adds no further identity. */
	m->birth_site = (birth_src != NULL) ? birth_src->birth_site : site;
	m->serial = (birth_src != NULL) ? birth_src->serial
		: (unlikely(rc_config.mem_report_enabled) ? mem_track_next_serial() : 0);
	m->backtrace = (birth_src != NULL) ? birth_src->backtrace
		: (unlikely(rc_config.mem_report_enabled) ? _get_backtrace() : NULL);
	m->ptr = (char *)base + MEM_HEADER_SIZE;

	/* Preserve old data when reallocating */
	if (data_src != NULL && prev_used > 0)
	{
		size_t copy_size = (prev_used < req) ? prev_used : req;

		memcpy(m->ptr, data_src->ptr, copy_size);
	}

	/* Zero newly extended region */
	if (req > prev_used)
		memset((char *)m->ptr + prev_used, 0, req - prev_used);

	// Initialize membership before the registry links reported blocks.
	m->registered = false;
	if (unlikely(rc_config.mem_report_enabled))
		mem_track_register(m);

	return m;
}

/**
 * mem_obj_free() - release a managed block and its backtrace
 * @m: header to release
 *
 * Unlink registered blocks from the live registry, then release the optional
 * backtrace and aligned base block.
 */
static inline void mem_obj_free(mem_obj_t *m)
{
	mem_track_unregister(m);
	free(m->backtrace);
	free(m);
}

/**
 * _mem_realloc() - allocate or resize a managed memory buffer
 * @ptr: pointer to caller's void* (set to new user pointer on return)
 * @req: requested size in bytes
 * @str: caller location string from __LOCATION__
 *
 * On fresh allocation (*ptr == NULL), allocates aligned memory.
 * On reallocation, reuses the buffer when capacity suffices,
 * otherwise allocates new aligned memory and copies data.
 *
 * Emits pr_crit on allocation failure.
 *
 * Return: the new user-data pointer (also stored in *ptr)
 */
void *_mem_realloc(void **ptr, size_t req, char *str)
{
	mem_obj_t *m;
	size_t prev_used;

	if (likely(*ptr == NULL))
	{
		/* Fresh allocation */
		m = mem_obj_alloc(NULL, NULL, req, 0, str);
		if (unlikely(m == NULL))
			goto alloc_fail;

		*ptr = m->ptr;
		return m->ptr;
	}

	m = mem_obj_from_ptr(*ptr);

	prev_used = m->used;

	/* Shrink or grow within existing capacity.
	 * On shrink (req < prev_used) the abandoned tail is not zeroed;
	 * grow-within-capacity re-zeroes the exposed region below. */
	if (req <= m->size)
	{
		m->used = req;

		/* Zero newly exposed region on grow-within-capacity */
		if (req > prev_used)
			memset((char *)m->ptr + prev_used, 0, req - prev_used);

		*ptr = m->ptr;
		return m->ptr;
	}

	/* Must reallocate: request exceeds capacity */
	{
		mem_obj_t *old_m = m;

		m = mem_obj_alloc(old_m, old_m, req, prev_used, str);
		if (unlikely(m == NULL))
			goto alloc_fail;

		/* The new block now owns the carried backtrace; null the old
		 * pointer so mem_obj_free does not release the transferred trace. */
		old_m->backtrace = NULL;
		mem_obj_free(old_m);
	}

	*ptr = m->ptr;
	return m->ptr;

alloc_fail:
	pr_crit("Memory allocation denied %s: requested %lu bytes\n",
		str, (unsigned long)req);
	abort();
}

/**
 * _mem_free() - release a managed scalar or byte buffer, rejecting array blocks
 * @ptr: pointer to caller's void* (set to NULL on return)
 * @site: caller location string ("file:line") for the rejection diagnostic
 *
 * The public mem_free macro strips the pointer cast and supplies the caller
 * site before calling here. A block stamped with a non-zero array_elem_size
 * was taken from an array verb and must be released through mem_array_free;
 * freeing it here would mismatch the backing allocation. BUG does not unwind,
 * so the wrongly typed block is left intact and surfaces in the report rather
 * than being silently freed. The site names the offending mem_free call.
 */
void _mem_free(void **ptr, char *site)
{
	if (*ptr != NULL)
	{
		mem_obj_t *m = mem_obj_from_ptr(*ptr);

		if (unlikely(m->array_elem_size != 0))
		{
			BUG("mem_free at %s: array block element size %lu; use mem_array_free\n",
				site, (unsigned long)m->array_elem_size);
			return;
		}

		mem_obj_free(m);
	}

	*ptr = NULL;
}

/**
 * mem_obj_dump() - print debug info for a managed buffer
 * @ptr: user-facing pointer
 */
void mem_obj_dump(void *ptr)
{
	mem_obj_t *m = mem_obj_from_ptr(ptr);

	pr_debug("mem_obj_t at %p: size=%lu used=%lu addr=%p\n",
		(void *)m, (unsigned long)m->size,
		(unsigned long)m->used, m->ptr);
}

/**
 * mem_backtrace() - display allocation backtrace for a managed buffer
 * @ptr: user-facing pointer
 */
void mem_backtrace(void *ptr)
{
	mem_obj_t *m;

	if (ptr == NULL)
	{
		print_backtrace("mem_backtrace(NULL)");
		return;
	}

	m = mem_obj_from_ptr(ptr);

	pr_debug("mem_backtrace(%p):\n", ptr);

	if (m->backtrace == NULL)
		pr_debug("  m->backtrace is null, no backtrace data\n");
	else
		_print_backtrace(m->backtrace);
}

/**
 * _mem_clone() - duplicate a managed memory buffer
 * @ptr: user-facing pointer to an existing managed allocation
 * @site: caller __LOCATION__ forwarded by the mem_clone macro, recorded as
 *        the clone's birth site
 *
 * Allocates a new managed buffer of the same used size and copies
 * the contents of the source buffer into it.
 *
 * Return: user-facing pointer to the new allocation
 */
void *_mem_clone(void *ptr, char *site)
{
	mem_obj_t *src = mem_obj_from_ptr(ptr);
	mem_obj_t *dst;

	/* Source supplies copy data and element width; a NULL birth predecessor
	 * mints the clone a fresh serial and records the caller site. */
	dst = mem_obj_alloc(src, NULL, src->used, src->used, site);
	if (unlikely(dst == NULL))
	{
		pr_crit("mem_clone: allocation failed for %lu bytes\n",
			(unsigned long)src->used);
		abort();
	}

	return dst->ptr;
}

/**
 * mem_bcmp() - compare contents of two managed buffers
 * @p1: user-facing pointer to first managed allocation
 * @p2: user-facing pointer to second managed allocation
 *
 * Compares the used regions of two managed buffers. Buffers with
 * different used sizes are considered unequal.
 *
 * Return: 0 if equal, non-zero otherwise (memcmp semantics)
 */
int mem_bcmp(void *p1, void *p2)
{
	mem_obj_t *m1 = mem_obj_from_ptr(p1);
	mem_obj_t *m2 = mem_obj_from_ptr(p2);

	if (m1->used != m2->used)
		return (m1->used > m2->used) ? 1 : -1;

	return memcmp(m1->ptr, m2->ptr, m1->used);
}

/**
 * mem_set() - fill a managed buffer with a byte value
 * @ptr: user-facing pointer to a managed allocation
 * @c: byte value to fill with
 *
 * Sets every byte in the used region of the managed buffer to c.
 */
void mem_set(void *ptr, int c)
{
	mem_obj_t *m = mem_obj_from_ptr(ptr);

	memset(m->ptr, c, m->used);
}

/**
 * mem_obj_copy_in() - copy into a managed block within its live span
 * @m: header of the destination block
 * @src: readable source buffer, managed or caller-owned
 * @n: number of bytes to copy
 *
 * Single bound-and-overlap point behind the managed copy verbs. The live
 * used span supplies the write bound. Comparing the unsigned address
 * distance against @n detects an overlapping source without forming an
 * overflow-prone range endpoint, preserving the memcpy non-overlap
 * contract; a zero-length copy touches nothing and so is always disjoint.
 * BUG does not unwind, so a rejected request leaves the destination
 * unwritten. The caller establishes source readability.
 */
static void mem_obj_copy_in(mem_obj_t *m, const void *src, size_t n)
{
	uintptr_t dest_addr = (uintptr_t)m->ptr;
	uintptr_t src_addr = (uintptr_t)src;
	uintptr_t distance = (dest_addr > src_addr)
		? dest_addr - src_addr
		: src_addr - dest_addr;

	if (likely(n <= m->used && (n == 0 || distance >= n)))
		memcpy(m->ptr, src, n);
	else if (n > m->used)
		BUG("managed copy: %lu bytes exceed destination used %lu\n",
			(unsigned long)n, (unsigned long)m->used);
	else
		BUG("managed copy: source overlaps destination at distance %lu over %lu bytes\n",
			(unsigned long)distance, (unsigned long)n);
}

/**
 * _mem_strdup() - duplicate a string into a fresh managed byte buffer
 * @src: null-terminated source string
 * @site: caller location from __LOCATION__, recorded as the birth site
 *
 * A string carries its length in its data rather than in its type, so the
 * length is measured once here and the allocation sized to hold the
 * terminator. The copy is born a byte buffer rather than a stamped array,
 * so the array verbs reject it and mem_free releases it.
 *
 * Return: managed copy of @src
 */
char *_mem_strdup(const char *src, char *site)
{
	size_t size = strlen(src) + 1;
	char *dest = NULL;

	_mem_realloc((void **)&dest, size, site);
	mem_obj_copy_in(mem_obj_from_ptr(dest), src, size);

	return dest;
}

/**
 * mem_array_guard() - reject an array verb on a non-array or mismatched block
 * @m: header recovered from an existing managed block
 * @elem_size: element size derived from the caller's typed pointer
 *
 * A live array block carries a non-zero array_elem_size equal to its
 * element size. A zero marker (scalar or byte block) or a size mismatch is
 * a programmer error, reported here and refused by the caller so the
 * mistyped block is left intact and surfaces in the report.
 *
 * Return: true when the block is an array of @elem_size, false otherwise
 */
static bool mem_array_guard(const mem_obj_t *m, size_t elem_size)
{
	if (unlikely(m->array_elem_size == 0 || m->array_elem_size != elem_size))
	{
		BUG("mem_array verb: element size %lu does not match header %lu\n",
			(unsigned long)elem_size, (unsigned long)m->array_elem_size);
		return false;
	}

	return true;
}

/**
 * _mem_array_cpy() - copy a bounded prefix of elements into a managed array
 * @dest: user-facing pointer to a managed array
 * @src: readable source holding at least @n elements of the same type
 * @elem_size: element size folded from the typed pointer at the macro
 * @n: element count to copy
 *
 * The array type guard rejects a scalar or byte block and a width
 * mismatch, so an element count cannot reach a buffer of a different
 * element width. Bounding the count against the destination's element
 * capacity before widening it keeps the byte product from wrapping, and
 * reports the overrun in the elements the caller counts rather than in
 * bytes.
 */
void _mem_array_cpy(void *dest, const void *src, size_t elem_size, size_t n)
{
	mem_obj_t *m = mem_obj_from_ptr(dest);

	if (unlikely(!mem_array_guard(m, elem_size)))
		return;

	if (unlikely(n > m->used / elem_size))
		BUG("mem_array_cpy: %lu elements exceed destination count %lu\n",
			(unsigned long)n, (unsigned long)(m->used / elem_size));
	else
		mem_obj_copy_in(m, src, n * elem_size);
}

/**
 * _mem_array_alloc() - allocate a managed typed array
 * @pp: address of the caller's array pointer
 * @elem_size: element size folded from the typed pointer at the macro
 * @n: element count
 * @site: caller location from __LOCATION__
 *
 * Sizes the request at n * elem_size and stamps the element size into the
 * header so later array verbs can validate and derive count and capacity.
 *
 * Return: the user-data pointer (also stored in *pp), or NULL when an
 * existing block fails the array type guard and *pp is left untouched
 */
void *_mem_array_alloc(void **pp, size_t elem_size, size_t n, char *site)
{
	if (unlikely(*pp != NULL))
	{
		if (unlikely(!mem_array_guard(mem_obj_from_ptr(*pp), elem_size)))
			return NULL;
	}

	void *p = _mem_realloc_fast(pp, n * elem_size, site);

	mem_obj_from_ptr(p)->array_elem_size = elem_size;

	return p;
}

/**
 * _mem_array_realloc() - resize a managed typed array
 * @pp: address of the caller's array pointer
 * @elem_size: element size folded from the typed pointer at the macro
 * @n: new element count
 * @site: caller location from __LOCATION__
 *
 * Validates an existing block against the array type guard, then resizes
 * to n * elem_size and stamps the element size, establishing it when the
 * array is born from a NULL pointer; persistence across a grow-beyond
 * relocation is handled by the allocator's carry-forward point.
 *
 * Return: the user-data pointer (also stored in *pp), or NULL when an
 * existing block fails the array type guard and *pp is left untouched
 */
void *_mem_array_realloc(void **pp, size_t elem_size, size_t n, char *site)
{
	if (likely(*pp != NULL))
	{
		if (unlikely(!mem_array_guard(mem_obj_from_ptr(*pp), elem_size)))
			return NULL;
	}

	void *p = _mem_realloc_fast(pp, n * elem_size, site);

	mem_obj_from_ptr(p)->array_elem_size = elem_size;

	return p;
}

/**
 * _mem_array_free() - release a managed typed array
 * @pp: address of the caller's array pointer (set to NULL on return)
 * @elem_size: element size folded from the typed pointer at the macro
 *
 * Validates the block against the array type guard before release; the
 * drop accounting reads the site stored at allocation, so no site is
 * passed here.
 */
void _mem_array_free(void **pp, size_t elem_size)
{
	if (*pp != NULL)
	{
		mem_obj_t *m = mem_obj_from_ptr(*pp);

		if (unlikely(!mem_array_guard(m, elem_size)))
			return;
		mem_obj_free(m);
	}

	*pp = NULL;
}

/**
 * _mem_zero() - zero a non-array managed buffer, rejecting array blocks
 * @ptr: managed scalar or byte-buffer pointer
 *
 * A non-array block carries a zero array_elem_size; a stamped array is a
 * programmer error, surfaced rather than tolerated. BUG does not unwind, so
 * the wrongly typed block is left untouched.
 */
void _mem_zero(void *ptr)
{
	mem_obj_t *m = mem_obj_from_ptr(ptr);

	if (unlikely(m->array_elem_size != 0))
	{
		BUG("mem_zero: array block element size %lu; use mem_array_zero\n",
			(unsigned long)m->array_elem_size);
		return;
	}

	mem_set(ptr, 0);
}

/**
 * _mem_array_zero() - zero a managed array, rejecting non-array blocks
 * @ptr: managed array pointer
 * @elem_size: element size folded from the typed pointer at the macro
 *
 * Validates the block against the array type guard before clearing the
 * whole live region.
 */
void _mem_array_zero(void *ptr, size_t elem_size)
{
	mem_obj_t *m = mem_obj_from_ptr(ptr);

	if (unlikely(!mem_array_guard(m, elem_size)))
		return;
	mem_set(ptr, 0);
}

/**
 * _mem_array_resize() - resize an array of structs, freeing dropped elements
 * @arr: address of the caller's array pointer
 * @elem_size: size of one array element
 * @new_count: new element count (caller guarantees >= 1)
 * @free_elem: per-element teardown for the shrink tail
 * @site: caller location from __LOCATION__, forwarded to the allocator
 *
 * Reads the prior element count from the allocator header, releases
 * sub-buffers of the dropped tail [new_count, old_count), then reallocates
 * the outer array in place. Surviving entries [0, new_count) keep their
 * pointers for reuse; any grown region is zeroed by the allocator.
 */
void _mem_array_resize(void **arr, size_t elem_size, int new_count,
		      mem_elem_free_fn free_elem, char *site)
{
	int old_count = mem_array_count(*arr);

	for (int i = new_count; i < old_count; i++)
		free_elem((char *)*arr + (size_t)i * elem_size);

	/* Reuse the typed realloc primitive so the element-size stamp and the
	 * type guard apply here exactly as on any other typed growth. */
	_mem_array_realloc(arr, elem_size, new_count, site);
}

/**
 * _mem_array_reserve() - grow a managed array to hold at least @needed elements
 * @arr: address of the caller's array pointer
 * @elem_size: size of one element
 * @needed: minimum element count the array must hold
 * @initial_cap: capacity allocated when the array is empty
 * @site: caller location from __LOCATION__, forwarded to the allocator
 *
 * Doubles the current capacity until it covers @needed, reusing the block
 * in place; an array already large enough is left untouched. The caller
 * tracks its own fill count.
 */
void _mem_array_reserve(void **arr, size_t elem_size, int needed,
		       int initial_cap, char *site)
{
	int cap = mem_array_capacity(*arr);

	if (cap >= needed)
		return;

	int new_cap = cap ? cap : initial_cap;
	while (new_cap < needed)
		new_cap *= 2;

	/* Reuse the typed realloc primitive so the element-size stamp and the
	 * type guard apply here exactly as on any other typed growth. */
	_mem_array_realloc(arr, elem_size, new_cap, site);
}

