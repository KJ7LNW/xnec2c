#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "branch_hints.h"
#include "location.h"

/* Alignment for cache-line and AVX-512 friendliness */
#define MEM_ALIGNMENT  64
/* MEM_HEADER_SIZE must remain a multiple of MEM_ALIGNMENT (64). */
#define MEM_HEADER_SIZE 128

/* mem_obj_t occupies the first cache line of each allocation.
 * User data begins at base + MEM_HEADER_SIZE, also 64-byte aligned.
 *
 * Layout:
 *   |<-- MEM_HEADER_SIZE (128B) -->|<-- req bytes user data -->|
 *   [ mem_obj_t (72B) | pad 56B   ][ user ptr (64-byte aligned)]
 *   ^                              ^
 *   base (64-aligned)              m->ptr = base + MEM_HEADER_SIZE
 *
 * Fields:
 *   size: total allocated user-data capacity
 *   used: amount currently in use by caller
 *     - shrink: req <= used, no realloc
 *     - grow within capacity: used < req <= size, no realloc
 *     - grow beyond capacity: req > size, new posix_memalign
 *   array_elem_size: element size for a managed array; 0 marks a scalar or
 *     byte block. The array layer is its single writer; element count and
 *     capacity derive from used/size divided by this size.
 *   backtrace: birth call path, captured at fresh birth only while reporting
 *     is enabled, carried with birth identity across relocation, released at
 *     free; NULL when reporting was disabled
 *   birth_site: __LOCATION__ of the block's birth allocation, carried across
 *     relocation; names the origin call site of a surviving block for leak
 *     localization, the registry enumerating the live set rather than
 *     adjudicating the leak
 *   ptr: pointer to user data region
 *   serial: monotonic identity stamped at birth and carried across
 *     relocation, naming a block independent of its reuse-prone address
 *   reg_prev, reg_next: live-registry links threaded through the header
 */
typedef struct mem_obj_t
{
	size_t size;
	size_t used;
	size_t array_elem_size;
	char **backtrace;
	const char *birth_site;

	void *ptr;

	uint64_t serial;
	struct mem_obj_t *reg_prev;
	struct mem_obj_t *reg_next;
} mem_obj_t;

void *_mem_realloc(void **ptr, size_t req, char *str);
void _mem_validate_fail(void *ptr, void *base) __attribute__((noreturn));

/**
 * mem_obj_from_ptr() - recover mem_obj_t header from a user pointer
 * @ptr: user-facing pointer returned by _mem_realloc
 *
 * Single point of truth for the header-recovery arithmetic.
 * Calls _mem_validate_fail (which invokes BUG) on corruption.
 *
 * Return: pointer to the mem_obj_t header
 */
static inline mem_obj_t *mem_obj_from_ptr(void *ptr)
{
	mem_obj_t *m = (mem_obj_t *)((char *)ptr - MEM_HEADER_SIZE);

	if (unlikely(m->ptr != ptr))
	{
		_mem_validate_fail(ptr, m);
		return NULL;
	}

	return m;
}

/**
 * _mem_realloc_fast() - inline fast-path for same-size reallocations
 * @ptr: address of caller's pointer (may point to existing allocation)
 * @req: requested size in bytes
 * @str: location string for diagnostics
 *
 * Short-circuits when the existing allocation already satisfies the
 * request (m->used == req), avoiding the full realloc path.
 *
 * Return: user-facing pointer to the allocation
 */
static inline void *_mem_realloc_fast(void **ptr, size_t req, char *str)
{
	if (likely(*ptr != NULL))
	{
		mem_obj_t *m = mem_obj_from_ptr(*ptr);

		if (likely(m->used == req))
			return m->ptr;
	}

	return _mem_realloc(ptr, req, str);
}

#define mem_alloc(ptr, size)   _mem_realloc_fast((void **)(ptr), (size), __LOCATION__)
#define mem_realloc(ptr, size) _mem_realloc_fast((void **)(ptr), (size), __LOCATION__)

void mem_backtrace(void *ptr);
void mem_obj_dump(void *ptr);
void _mem_free(void **ptr, char *site);
#define mem_free(ptr) _mem_free((void **)(ptr), __LOCATION__)

/* Typed array verbs: derive the element size from the pointer so callers
 * pass only a pointer and a quantity. The inner _mem_array_* helpers take
 * the folded size, stamp it into the allocator header, and guard array
 * verbs against non-array or mismatched-element blocks. */
void *_mem_array_alloc(void **pp, size_t elem_size, size_t n, char *site);
void *_mem_array_realloc(void **pp, size_t elem_size, size_t n, char *site);
void _mem_array_free(void **pp, size_t elem_size);

/* MEM_ARRAY_TYPED rejects a void element type at compile time, independent
 * of -Wpointer-arith, so the silent sizeof(void)==1 trap cannot size an
 * array verb at one byte. Arrays of void * pointers pass: their element
 * type is void *, not void. */
/* The guard wraps a struct-member _Static_assert in sizeof so it is a
 * constant expression usable in a comma operator, avoiding a GNU
 * statement-expression (which -Wpedantic rejects). */
#define MEM_ARRAY_TYPED(pp) sizeof(struct { \
	_Static_assert( \
		!__builtin_types_compatible_p(__typeof__(**(pp)), void), \
		"mem_array_*: void element type; use mem_alloc for byte buffers"); \
	int _mem_array_typed_dummy; })

#define mem_array_alloc(pp, n) \
	((void)MEM_ARRAY_TYPED(pp), _mem_array_alloc((void **)(pp), sizeof(**(pp)), (n), __LOCATION__))
#define mem_array_realloc(pp, n) \
	((void)MEM_ARRAY_TYPED(pp), _mem_array_realloc((void **)(pp), sizeof(**(pp)), (n), __LOCATION__))
#define mem_array_free(pp) \
	((void)MEM_ARRAY_TYPED(pp), _mem_array_free((void **)(pp), sizeof(**(pp))))
#define mem_new(pp) \
	((void)MEM_ARRAY_TYPED(pp), mem_alloc((pp), sizeof(**(pp))))

/**
 * mem_array_count() - elements currently allocated in a managed array
 * @ptr: managed pointer, or NULL
 *
 * Reads the live element count from the allocator header, the single
 * source of truth; the element width is the stamped array_elem_size, so
 * callers pass only the pointer.
 *
 * Return: used bytes / element width, or 0 when ptr is NULL
 */
static inline int mem_array_count(void *ptr)
{
	if (ptr == NULL)
		return 0;

	mem_obj_t *m = mem_obj_from_ptr(ptr);

	return (int)(m->used / m->array_elem_size);
}

/**
 * mem_array_capacity() - elements a managed array holds before it must grow
 * @ptr: managed pointer, or NULL
 *
 * Return: capacity bytes / stamped element width, or 0 when ptr is NULL
 */
static inline int mem_array_capacity(void *ptr)
{
	if (ptr == NULL)
		return 0;

	mem_obj_t *m = mem_obj_from_ptr(ptr);

	return (int)(m->size / m->array_elem_size);
}

/**
 * mem_elem_free_fn - per-element teardown callback for mem_array_resize
 * @elem: pointer to one array element to release sub-buffers from
 */
typedef void (*mem_elem_free_fn)(void *elem);

void _mem_array_resize(void **arr, size_t elem_size, int new_count,
		      mem_elem_free_fn free_elem, char *site);

/* Fold the element width from the typed pointer and thread the caller
 * __LOCATION__ to the resize helper; callers pass only the array and count. */
#define mem_array_resize(arr, new_count, free_elem) \
	((void)MEM_ARRAY_TYPED(arr), _mem_array_resize((void **)(arr), sizeof(**(arr)), (new_count), (free_elem), __LOCATION__))

/**
 * mem_array_reserve - grow a managed array to hold at least @needed elements
 * @arr: address of the caller's array pointer
 * @elem_size: size of one element
 * @needed: minimum element count the array must hold
 * @initial_cap: capacity allocated when the array is empty
 */
void _mem_array_reserve(void **arr, size_t elem_size, int needed,
		       int initial_cap, char *site);

#define mem_array_reserve(arr, needed, initial_cap) \
	((void)MEM_ARRAY_TYPED(arr), _mem_array_reserve((void **)(arr), sizeof(**(arr)), (needed), (initial_cap), __LOCATION__))
void *_mem_clone(void *ptr, char *site);
#define mem_clone(p) _mem_clone((p), __LOCATION__)
int mem_bcmp(void *p1, void *p2);
void mem_set(void *ptr, int c);
void _mem_array_cpy(void *dest, const void *src, size_t elem_size, size_t n);
char *_mem_strdup(const char *src, char *site);

/**
 * mem_strdup() - duplicate a string into a fresh managed byte buffer
 * @src: null-terminated source string
 *
 * A string carries its length in its data rather than in its type, so the
 * allocator measures it and sizes the copy to hold the terminator.
 * Ownership passes to the caller, who releases the copy with mem_free.
 *
 * Return: managed copy of @src
 */
#define mem_strdup(src) _mem_strdup((src), __LOCATION__)

/**
 * mem_array_cpy() - copy a bounded prefix of elements into a managed array
 * @dest: managed array pointer
 * @src: readable source holding at least @n elements of the same type
 * @n: element count to copy
 *
 * Folds the element width from @dest so callers pass a quantity rather than
 * a byte size. A destination that is not a stamped array of that width is
 * reported through BUG and left unwritten. The caller supplies a source of
 * the same element type, which may carry different qualifiers.
 */
#define mem_array_cpy(dest, src, n) \
	((void)MEM_ARRAY_TYPED(&(dest)), _mem_array_cpy((dest), (src), sizeof(*(dest)), (n)))

void _mem_zero(void *ptr);
void _mem_array_zero(void *ptr, size_t elem_size);

/**
 * mem_array_zero() - zero the entire used region of a managed array
 * @p: managed array pointer
 *
 * Reports through BUG and returns when @p is not a stamped array block or
 * its element width differs from sizeof(*@p); no partial-range managed zero
 * exists, so the whole live region is cleared.
 */
#define mem_array_zero(p) _mem_array_zero((p), sizeof(*(p)))

/**
 * mem_zero() - zero the entire used region of a non-array managed buffer
 * @p: managed scalar or byte-buffer pointer
 *
 * Reports through BUG and returns when @p is a stamped array block; use
 * mem_array_zero for arrays.
 */
#define mem_zero(p) _mem_zero((p))

#endif /* MEM_H */
