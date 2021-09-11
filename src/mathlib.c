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
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include "main.h"
#include "shared.h"
#include "mathlib.h"

void warn_restart(mathlib_t *lib);

static mathlib_t mathlibs[] = {
	// In order of preference, xnec2c will use the first one that loads successfully:
	{.type = MATHLIB_ATLAS, .lib = "libtatlas.so", .name = "ATLAS, Threaded", .f_prefix = "clapack_"},
	{.type = MATHLIB_ATLAS, .lib = "libsatlas.so", .name = "ATLAS, Serial", .f_prefix = "clapack_"},

	{.type = MATHLIB_OPENBLAS, .lib = "libopenblas.so",  .name = "OpenBLAS+LAPACKe, Serial", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblaso.so", .name = "OpenBLAS+LAPACKe, OpenMP", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblasp.so", .name = "OpenBLAS+LAPACKe, pthreads", .f_prefix = "LAPACKE_"},

	// These are of type MATHLIB_OPENBLAS because the calling convention is the same:
	{.type = MATHLIB_OPENBLAS, .lib = "libmkl_rt.so", .name = "Intel MKL, Serial", .f_prefix = "LAPACKE_",
		.env = { "MKL_THREADING_LAYER", "SEQUENTIAL" }, .init = warn_restart },
	{.type = MATHLIB_OPENBLAS, .lib = "libmkl_rt.so", .name = "Intel MKL, TBB Threads", .f_prefix = "LAPACKE_",
		.env = { "MKL_THREADING_LAYER", "TBB" }, .init = warn_restart },
	{.type = MATHLIB_OPENBLAS, .lib = "libmkl_rt.so", .name = "Intel MKL, Intel Threads", .f_prefix = "LAPACKE_",
		.env = { "MKL_THREADING_LAYER", "INTEL" }, .init = warn_restart },
	{.type = MATHLIB_OPENBLAS, .lib = "libmkl_rt.so", .name = "Intel MKL, GNU Threads", .f_prefix = "LAPACKE_",
		.env = { "MKL_THREADING_LAYER", "GNU" }, .init = warn_restart },

	// Default implementation if none of the newer libraries are found.  This is
	// before the old implementations below because it has been tested:
	{.type = MATHLIB_NEC2, .lib = "(builtin)", .name = "NEC2 Gaussian Elimination"},

	// Old implementations that may or may not work
	{.type = MATHLIB_ATLAS, .lib = "libatlas.so", .name = "ATLAS, Serial (old)", .f_prefix = "clapack_"},
	{.type = MATHLIB_ATLAS, .lib = "libcblas.so", .name = "CBLAS, Serial (old)", .f_prefix = "clapack_"},
	{.type = MATHLIB_ATLAS, .lib = "libptcblas.so", .name = "CBLAS, Threaded (old)", .f_prefix = "clapack_"}
};

mathlib_t *current_mathlib = NULL;

// To add a new mathfunc:
//   * Update the enum in mathlib.h if the calling convention differs
//   * Add function pointer typedefs in mathlib.h
//   * Add a row here
//   * Implement the new func below to use current_mathlib.
//   * Add a prototype for the new func in mathlib.h
static char *mathfuncs[] = {
	[MATHLIB_ZGETRF] = "zgetrf",
	[MATHLIB_ZGETRS] = "zgetrs"
};

static int num_mathlibs = sizeof(mathlibs) / sizeof(mathlib_t);
static int num_mathfuncs = sizeof(mathfuncs) / sizeof(char *);

mathlib_t *get_mathlib_by_idx(int idx)
{
	if (idx < num_mathlibs)
		return &mathlibs[idx];
	else
		return NULL;
}


void close_mathlib(mathlib_t *lib)
{
	if (lib == NULL)
		return;

	free_ptr((void **) &lib->functions);

	if (lib->handle != NULL)
	{
		dlclose(lib->handle);
		lib->handle = NULL;
	}

	if (lib->env[0] != NULL)
		unsetenv(lib->env[0]);
}

int open_mathlib(mathlib_t *lib)
{
	char fname[40];
	int fidx;

	if (lib == NULL)
	{
		printf("open_mathlib: lib is NULL\n");
		return 0;
	}

	// Builting NEC2 Gaussian Elimination isn't a .so, just return success.
	if (lib->type == MATHLIB_NEC2)
		return 1;

	// Set environment if configured:
	if (lib->env[0] != NULL && setenv(lib->env[0], lib->env[1], 1) < 0)
			printf("setenv(%s, %s): %s\n", lib->env[0], lib->env[1], strerror(errno));
	
	// Clear any error state
	dlerror();

	// Open the .so library
	lib->handle = dlopen(lib->lib, RTLD_NOW);
	//lib->handle = dlmopen(LM_ID_NEWLM, lib->lib, RTLD_NOW);

	if (lib->handle == NULL)
	{
		printf("  Unable to open %s: %s\n", lib->lib, dlerror());
		close_mathlib(lib);
		return 0;
	}

	if (dlinfo(lib->handle, RTLD_DI_LMID, &lib->lmid) == -1)
		printf("dlinfo: %s: %s\n", lib->lib, dlerror());

	// Call the init() function if configured
	if (lib->init != NULL)
		lib->init(lib);

	mem_alloc((void **) &lib->functions, sizeof(void *) * num_mathfuncs, __LOCATION__);
	for (fidx = 0; fidx < num_mathfuncs; fidx++)
	{
		// Clear any error state
		dlerror();

		// Resolve the function symbol
		snprintf(fname, sizeof(fname) - 1, "%s%s%s", 
			lib->f_prefix ? lib->f_prefix : "",
			mathfuncs[fidx],
			lib->f_suffix ? lib->f_suffix : "");
		lib->functions[fidx] = dlsym(lib->handle, fname);

		char *error = dlerror();

		if (error != NULL)
		{
			printf("  %s: unable to bind %s: %s\n", lib->lib, mathfuncs[fidx], error);
			close_mathlib(lib);
			break;
		}
	}

	if (lib->handle != NULL)
		return 1;
	else
		return 0;
}


void init_mathlib()
{
	int libidx;

	for (libidx = 0; libidx < num_mathlibs; libidx++)
	{
		// Initialization:
		mathlibs[libidx].idx = libidx;
		mathlibs[libidx].functions = NULL;

		// Try to open each library:
		printf("\nTrying %s (%s):\n", mathlibs[libidx].name, mathlibs[libidx].lib);
		if (!open_mathlib(&mathlibs[libidx]))
		{
			printf("  skipping.\n");

			close_mathlib(&mathlibs[libidx]);

			mathlibs[libidx].available = 0;
			continue;
		}
		else
			mathlibs[libidx].available = 1;

		// At this point the library load was successful, provide detail:
		if (mathlibs[libidx].handle != NULL)
		{
			char lpath[PATH_MAX];
			dlinfo(mathlibs[libidx].handle, RTLD_DI_ORIGIN, lpath);
			printf("  loaded ok: %s/%s\n", lpath, mathlibs[libidx].lib);
		}
		else
			printf("  loaded ok.\n");

		// Set the default to the first one we find:
		if (current_mathlib == NULL)
			current_mathlib = &mathlibs[libidx];
		else
			// Otherwise close it for now and re-open on use.
			close_mathlib(&mathlibs[libidx]);
	}
}

void set_mathlib(GtkWidget *widget, mathlib_t *lib)
{
	if (widget != NULL && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		return;

	if (lib == NULL)
	{
		printf("set_mathlib: lib == NULL\n");
		return;
	}

	if (current_mathlib == lib)
		return;

	if (g_mutex_trylock(&global_lock))
	{
		close_mathlib(current_mathlib);
		current_mathlib = lib;

		open_mathlib(lib);
		printf("mathlib: selected %s\n", lib->name);
		g_mutex_unlock(&global_lock);
	}
	else
		Stop( _("mathlib: You cannot change the math library while the freq loop is running."), ERR_OK );

}

void init_mathlib_menu()
{
	GSList *math_radio_group = NULL;
	GtkWidget *math_menu = Builder_Get_Object(main_window_builder, "main_mathlib_menu_menu");
	
	int libidx;
	for (libidx = 0; libidx < num_mathlibs; libidx++)
	{
		// Skip .so mathlibs that aren't available on the system:
		if (!mathlibs[libidx].available)
			continue;

		// Create the menu item widget:
		GtkWidget *w = gtk_radio_menu_item_new_with_label(math_radio_group, mathlibs[libidx].name);

		math_radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));

		// This is where we need to read the config:
		if (libidx == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), TRUE);
		else
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), FALSE);

		// Add the menu item and pass the mathlib_t structure to the set_mathlib callback:
		g_signal_connect(GTK_MENU_ITEM(w), "toggled", G_CALLBACK(set_mathlib), (gpointer)&mathlibs[libidx]);
		gtk_menu_shell_append(GTK_MENU_SHELL(math_menu), w);

	}

	gtk_widget_show_all(main_window);
}

int32_t zgetrf(int32_t order, int32_t m, int32_t n, complex double *a, int32_t ndim, int32_t *ip)
{
	int f_idx = MATHLIB_ZGETRF;

	//printf("%s: type=%d typename=%s\n", mathfuncs[f_idx], current_mathlib->type, current_mathlib->name);

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrf_atlas_t *f;
		
		// This is an ugly cast, but see `man dlopen` for why.
		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order, m, n, a, ndim, (int32_t*)ip);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS)
	{
		zgetrf_openblas_t *f;
		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order, m, n, a, ndim, (int32_t*)ip);
	}
	else if (current_mathlib->type == MATHLIB_NEC2)
	{
		// use the original NEC2 function
		return factr_gauss_elim(n, a, (int32_t*)ip, ndim);
	}
	else
		printf("%s: unsupported mathlib type %d\n", mathfuncs[f_idx], current_mathlib->type);

	return 1;
}

int32_t zgetrs(int32_t order, int32_t trans, int32_t lda, int32_t nrhs,
	complex double *a, int32_t ndim, int32_t *ip, complex double *b, int32_t ldb)
{
	int f_idx = MATHLIB_ZGETRS;

	//printf("%s: type=%d typename=%s\n", mathfuncs[f_idx], current_mathlib->type, current_mathlib->name);

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrs_atlas_t *f;
		
		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order, trans, lda, nrhs, a, ndim, (int32_t*)ip, b, ldb);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS)
	{
		zgetrs_openblas_t *f;

		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order,
			(trans) == CblasConjTrans ? 'C' : ((trans) == CblasTrans ? 'T' : 'N'),
			lda, nrhs, a, ndim, (int32_t*)ip, b, ldb);
	}
	else if (current_mathlib->type == MATHLIB_NEC2)
	{
		if (lda != ldb)
			printf("zgetrs warning: lda(%d) != ldb(%d)\n", lda, ldb);

		// use the original NEC2 function
		return solve_gauss_elim(lda, a, (int32_t*)ip, b, ndim);
	}
	else
		printf("%s: unsupported mathlib type %d\n", mathfuncs[f_idx], current_mathlib->type);

	return 1;
}

void warn_restart(mathlib_t *lib)
{
}


// These Intel MKL functions are not being used but I'm leaving them here
// in case they become useful in the future to set the threading type.
// Currently we use setenv(MKL_THREADING_LAYER):

/* Single Dynamic library threading
 * https://software.intel.com/content/www/us/en/develop/documentation/onemkl-linux-developer-guide/top/linking-your-application-with-the-intel-oneapi-math-kernel-library/linking-in-detail/dynamically-selecting-the-interface-and-threading-layer.html
 */

#define MKL_THREADING_INTEL         0
#define MKL_THREADING_SEQUENTIAL    1
#define MKL_THREADING_PGI           2
#define MKL_THREADING_GNU           3
#define MKL_THREADING_TBB           4

void mathlib_mkl_set_threading(mathlib_t *lib, int code)
{
	int (*mkl_set_threading_layer)(int) = NULL;

	*(void **) (&mkl_set_threading_layer) = dlsym(lib->handle, "MKL_Set_Threading_Layer");
	if (mkl_set_threading_layer == NULL)
	{
		printf("dlsym(mkl_set_threading_layer): %s\n", dlerror());
		return;
	}

	printf("mkl_set_threading_layer(%d)\n", code);

	mkl_set_threading_layer(code);
}

void mathlib_mkl_set_threading_intel(mathlib_t *lib)
{
	mathlib_mkl_set_threading(lib, MKL_THREADING_INTEL);
}

void mathlib_mkl_set_threading_sequential(mathlib_t *lib)
{
	mathlib_mkl_set_threading(lib, MKL_THREADING_SEQUENTIAL);
}

void mathlib_mkl_set_threading_gnu(mathlib_t *lib)
{
	mathlib_mkl_set_threading(lib, MKL_THREADING_GNU);
}

void mathlib_mkl_set_threading_tbb(mathlib_t *lib)
{
	mathlib_mkl_set_threading(lib, MKL_THREADING_TBB);
}
