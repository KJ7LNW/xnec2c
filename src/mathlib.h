#ifdef HAVE_OPENBLAS_CBLAS_H
	#include <openblas/cblas.h>
	#ifdef HAVE_OPENBLAS_LAPACKE_H
		#include <openblas/lapacke.h>
	#endif
	#ifdef HAVE_LAPACKE_H
		#include <lapacke.h>
	#endif

#else
	#ifdef HAVE_CLAPACK_H
			#include <clapack.h>
	#else
			// Define it ourself if undefined:
			enum CBLAS_TRANSPOSE {CblasColMajor=102, CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
	#endif 
#endif

#include "rc_config.h"

// You only need a new enum if the calling convention is different.  For example,
// Intel MKL uses the OpenBLAS calling convention, but needs special treatment:
enum MATHLIB_TYPES {
	MATHLIB_ATLAS,
	MATHLIB_OPENBLAS,
	MATHLIB_INTEL,
	MATHLIB_NEC2,
	MATHLIB_COUNT
};

enum MATHLIB_FUNCTIONS {
	MATHLIB_ZGETRF, 
	MATHLIB_ZGETRS,
};

enum MATHLIB_BENCHMARKS
{
	MATHLIB_BENCHMARK_PARALLEL,
	MATHLIB_BENCHMARK_SINGLE,
	MATHLIB_BENCHMARK_NLOG2,
	MATHLIB_BENCHMARK_NJ,
	MATHLIB_BENCHMARK_THREADS,
	MATHLIB_BENCHMARK_COUNT
};

enum MATHLIB_VARIED
{
	MATHLIB_VARIED_JOBS,
	MATHLIB_VARIED_THREADS,
	MATHLIB_VARIED_COUNT
};

/**
 * mathlib_varied_t - Engine count a benchmark progression walks
 * @value: Engine field holding the count
 * @label: Command-line option stating @value, named in the summary
 * @needs_setter: The count reaches a library only through the runtime thread
 *                setter its family exposes
 */
typedef struct
{
	int *value;
	const char *label;
	gboolean needs_setter;
} mathlib_varied_t;

/**
 * mathlib_benchmark_spec_t - Count progression of one benchmark mode
 * @varied: Count the progression walks
 * @single_job: Run every pass at one job instead of the -j count
 * @shift: Right-shift applied to the walked count after each sweep
 * @decrement: Amount subtracted from the walked count after each sweep
 *
 * A row advancing the count by neither shift nor decrement runs one sweep and
 * ends the progression.
 */
typedef struct
{
	enum MATHLIB_VARIED varied;
	gboolean single_job;
	int shift;
	int decrement;
} mathlib_benchmark_spec_t;

typedef struct mathlib_t
{
	// Mathlib type:
	int type;

	// True if it opened successfully upon initialization:
	int available;

	// True if included for benchmarks
	int benchmark;

	// Unique stable identifier for configuration persistence.
	// MUST remain unchanged once assigned to maintain backwards compatibility.
	// Used for string-based config storage instead of array indices.
	char *id;

	// lib: libname.so
	// name: a human readable description
	// f_prefix: the prefix to the function exported by the lib.  For example, "LAPACKE_"
	// f_suffix: the prefix to the function exported by the lib. For example, "_"
	char *lib, *name, *f_prefix, *f_suffix;

	// Handle from dlopen 
	void *handle;

	// Function pointers, one for each function in MATHLIB_FUNCTIONS.
	void **functions;

	// Runtime thread-count setter bound from the handle by open_mathlib().  NULL
	// for libraries whose thread count is fixed when they are built.  Cleared by
	// close_mathlib() because it addresses the closed handle.
	void (*set_threads)(int);

	// Reference to the menu item under File->Math Libraries.
	GtkWidget
		*interactive_widget,
		*batch_widget,
		*benchmark_widget;

	// Function pointer to call after dlopen() and is passed the mathlib_t pointer.
	void (*init)(struct mathlib_t*);
} mathlib_t;


void init_mathlib(void);
void init_mathlib_menu(void);
mathlib_t *get_mathlib_by_id(const char *id);
int mathlib_load(mathlib_t *lib);
void set_mathlib_interactive(GtkWidget *widget, mathlib_t *lib);

void mathlib_lock_intel_interactive(const char *locked_id);
void mathlib_lock_intel_batch(const char *locked_id);

void mathlib_config_init(rc_config_vars_t *v, char *line);
int mathlib_config_benchmark_parse(rc_config_vars_t *v, char *line);
int mathlib_config_benchmark_save(rc_config_vars_t *v, FILE *fp);

void mathlib_set_num_threads(mathlib_t *lib, int threads);
const char *mathlib_threads_env_conflict(void);

void mathlib_mkl_set_threading_intel(mathlib_t *lib);
void mathlib_mkl_set_threading_sequential(mathlib_t *lib);
void mathlib_mkl_set_threading_gnu(mathlib_t *lib);
void mathlib_mkl_set_threading_tbb(mathlib_t *lib);

typedef int32_t (zgetrf_atlas_t)(int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*);
typedef int32_t (zgetrf_openblas_t)(int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*);

typedef int32_t (zgetrs_atlas_t)(int32_t, int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*, complex double *, int32_t);
typedef int32_t (zgetrs_openblas_t)(int32_t, char, int32_t, int32_t, complex double *, int32_t, int32_t*, complex double *, int32_t);


int32_t zgetrf(int32_t order, int32_t m, int32_t n, complex double *a, int32_t ndim, int32_t *ip);
int32_t zgetrs(int32_t order, int32_t trans, int32_t lda, int32_t nrhs, complex double *a, int32_t ndim, int32_t *ip, complex double *b, int32_t ldb);

extern mathlib_t *current_mathlib;
