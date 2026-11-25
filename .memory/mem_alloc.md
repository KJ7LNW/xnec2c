# C memory allocation and structure layout

Apply each line when writing C code that allocates, copies, zeroes, or frees memory, or designing a data structure.

## User-given

- Treat allocation shape as the managed-memory contract:
	- API, closed set:
		- Life: `mem_alloc`, `mem_realloc`, `mem_free`, `mem_new`, `mem_clone`, `mem_strdup`
		- Array: `mem_array_alloc`, `mem_array_realloc`, `mem_array_reserve`, `mem_array_resize`, `mem_array_free`, `mem_array_count`, `mem_array_capacity`, `mem_array_cpy`, `mem_array_zero`
		- Byte: `mem_set`, `mem_zero`
		- Probe: `mem_obj_from_ptr`, `mem_obj_dump`, `mem_backtrace`, `mem_bcmp`
	- One family per pointer: `mem_array_*` for arrays, `mem_*` for bytes and objects; a mismatch calls `BUG()` and leaves the block intact.
	- Null stack pointers before first managed use.
	- Array verbs fold width: pass counts, never bytes or `sizeof`.
	- Copy by shape: object `*dst = *src`; array `mem_array_cpy(dst, src, n)`; string `mem_strdup(src)`; unmanaged bytes `memcpy`.
	- `mem_new(&p)` takes one object, `mem_alloc` raw bytes; array verbs reject void elements.
	- `mem_array_reserve` grows amortized; `mem_array_realloc` fits exact.
	- Read count and capacity from the header; store no mirror field.
	- Hold a fill count only when it differs from the header.
	- `size` is capacity, `used` is length; a smaller request never shrinks.
	- Do not re-zero grown slots; they arrive zeroed.
	- Zero verbs take the allocation start, never stack, static, or interior pointers; `memset` clears subranges.
	- Bound teardown loops with `mem_array_count`.
	- `mem_free(&p)` releases and nulls; NULL is a no-op.
- When considering new structures, consider AoS vs SoA and field order wrt cache-line best practices for performance and high cache hits. Also consider pipelining and structure stride alignments where it is useful (eg AVX/SSE, Compiler optimization friendliness, parallel pragmas, etc).
