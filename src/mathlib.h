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
			enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
	#endif 
#endif

typedef struct 
{
	int type;
	char *lib, *name, *f_prefix, *f_suffix;
	void *handle;
	void **functions;
} mathlib_t;

// You only need a new enum if the calling convention is different.  For example,
// Intel MKL uses the OpenBLAS calling convention.
enum {
	MATHLIB_ATLAS,
	MATHLIB_OPENBLAS,
	MATHLIB_NONE
};

enum {
	MATHLIB_ZGETRF, 
	MATHLIB_ZGETRS,
};

typedef int32_t (zgetrf_atlas_t)(int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*);
typedef int32_t (zgetrf_openblas_t)(int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*);

typedef int32_t (zgetrs_atlas_t)(int32_t, int32_t, int32_t, int32_t, complex double *, int32_t, int32_t*, complex double *, int32_t);
typedef int32_t (zgetrs_openblas_t)(int32_t, char, int32_t, int32_t, complex double *, int32_t, int32_t*, complex double *, int32_t);


int32_t zgetrf(int32_t order, int32_t m, int32_t n, complex double *a, int32_t ndim, int32_t *ip);
int32_t zgetrs(int32_t order, int32_t trans, int32_t lda, int32_t nrhs, complex double *a, int32_t ndim, int32_t *ip, complex double *b, int32_t ldb);

extern mathlib_t *current_mathlib;
