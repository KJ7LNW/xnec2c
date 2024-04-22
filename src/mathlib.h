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
	MATHLIB_NEC2
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
	MATHLIB_BENCHMARK_NJ
};

typedef struct mathlib_t
{
	// Mathlib type:
	int type; 

	// Index into the mathlibs array:
	int idx; 

	// True if it opened successfully upon initialization:
	int available;

	// True if included for benchmarks
	int benchmark;

	// lib: libname.so
	// name: a human readable description
	// f_prefix: the prefix to the function exported by the lib.  For example, "LAPACKE_"
	// f_suffix: the prefix to the function exported by the lib. For example, "_"
	char *lib, *name, *f_prefix, *f_suffix;

	// Handle from dlopen 
	void *handle;

	// Function pointers, one for each function in MATHLIB_FUNCTIONS.
	void **functions;

	// Pointer to environment variables that should be set before dlopen() is called.
	// env[0] is the name, env[1] is the value.
	char *env[2];

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
mathlib_t *get_mathlib_by_idx(int idx);
void set_mathlib_interactive(GtkWidget *widget, mathlib_t *lib);

void mathlib_lock_intel_interactive(int locked_idx);
void mathlib_lock_intel_batch(int locked_idx);

void mathlib_config_init(rc_config_vars_t *v, char *line);
int mathlib_config_benchmark_parse(rc_config_vars_t *v, char *line);
int mathlib_config_benchmark_save(rc_config_vars_t *v, FILE *fp);

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
