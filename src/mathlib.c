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

void mathlib_mkl_set_threading_intel(mathlib_t *lib);

static mathlib_t mathlibs[] = {
	// In order of preference, xnec2c will use the first one that loads successfully by default:
	
	// CentOS 7 atlas-devel
	{.type = MATHLIB_ATLAS, .lib = "libtatlas.so", .name = "ATLAS, Threaded", .f_prefix = "clapack_"},
	{.type = MATHLIB_ATLAS, .lib = "libsatlas.so", .name = "ATLAS, Serial", .f_prefix = "clapack_"},

	// CentOS 7 openblas-devel
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblas.so",  .name = "OpenBLAS+LAPACKe, Serial", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblaso.so", .name = "OpenBLAS+LAPACKe, OpenMP", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblasp.so", .name = "OpenBLAS+LAPACKe, pthreads", .f_prefix = "LAPACKE_"},
	
	// Ubuntu / Debian (liblapacke libopenblas0-*) (tested Ubuntu 16.04, 18.04, 20.04, Debian 9, Debian 11)
	//   Note that you may need to use `alternatives` to select your openblas implementation.
	//   Some combinations of BLAS/LAPACK may or may not work together:
	//     ~# update-alternatives --config libblas.so.3-x86_64-linux-gnu
	//     ~# update-alternatives --config liblapack.so.3-x86_64-linux-gnu
	{.type = MATHLIB_OPENBLAS, .lib = "liblapacke.so.3", .name = "Selected LAPACK+BLAS", .f_prefix = "LAPACKE_"},

	// Ubuntu / Debian libatlas3-base (tested Ubuntu 16.04, 18.04, Debian 9, Debian 11)
	// Note: This requires the following in Debian 11 and Ubuntu 20.04:
	//   Note that you may need to use `alternatives` to select your openblas implementation:
	//     update-alternatives --config libblas.so.3-x86_64-linux-gnu
	//   Additionally, you should set this:
	//     LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/atlas/
	//   Without it, Ubuntu 20.04 crashes and Debian 11 gives "undefined symbol: clapack_zgetrf"
	{.type = MATHLIB_ATLAS, .lib = "liblapack_atlas.so.3", .name = "ATLAS", .f_prefix = "clapack_"},

	// Intel
	{.type = MATHLIB_INTEL, .lib = "libmkl_rt.so", .name = "Intel MKL, Serial", .f_prefix = "LAPACKE_",
		.init = mathlib_mkl_set_threading_sequential },
	{.type = MATHLIB_INTEL, .lib = "libmkl_rt.so", .name = "Intel MKL, TBB Threads", .f_prefix = "LAPACKE_",
		.init = mathlib_mkl_set_threading_tbb },
	{.type = MATHLIB_INTEL, .lib = "libmkl_rt.so", .name = "Intel MKL, Intel Threads", .f_prefix = "LAPACKE_",
		.init = mathlib_mkl_set_threading_intel },
	{.type = MATHLIB_INTEL, .lib = "libmkl_rt.so", .name = "Intel MKL, GNU Threads", .f_prefix = "LAPACKE_",
		.init = mathlib_mkl_set_threading_gnu },

	// Default implementation if none of the newer libraries are found.
	{.type = MATHLIB_NEC2, .lib = "(builtin)", .name = "NEC2 Gaussian Elimination"},

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


// Restore selection on open:
void mathlib_config_init(rc_config_vars_t *v)
{
	if (rc_config.mathlib_idx < num_mathlibs &&
		rc_config.mathlib_idx >= 0 &&
		mathlibs[rc_config.mathlib_idx].available)
	{
		set_mathlib(NULL, &mathlibs[rc_config.mathlib_idx]);
	}
	else
		printf("Unable to set the preferred mathlib index to %d\n", rc_config.mathlib_idx);
}

void set_mathlib(GtkWidget *widget, mathlib_t *lib)
{
	int i;

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

		// Save selection on exit:
		rc_config.mathlib_idx = current_mathlib->idx;

		open_mathlib(lib);

		// Intel libraries can only be set once:
		if (lib->type == MATHLIB_INTEL)
		{
			if (!CHILD)
				printf( _("\n"
				"* Notice: \"%s\" was selected, but you can only change to a\n"
				"* different Intel MKL threading library by restarting xnec2c.\n"
				"* \n"
				"* However, you may continue to select any other Linear Algebra library\n"
				"* without restart, including this same Intel MKL library.  All other\n"
				"* Intel MKL threading libraries are disabled until restart because of an \n"
				"* MKL limitation where the MKL threading library can only be dynamically\n"
				"* linked only once per runtime.\n"
				"* \n"
				"* See Intel's documentation on mkl_set_threading_layer() for more detail.\n"),
					 lib->name);

			// Disable the other MKL threading options.
			for (i = 0; i < num_mathlibs; i++)
				if (mathlibs[i].available && mathlibs[i].type == MATHLIB_INTEL && lib->idx != i)
				{
					mathlibs[i].available = 0;

					if (!CHILD && mathlibs[i].menu_widget != NULL)
						gtk_widget_set_sensitive(GTK_WIDGET(mathlibs[i].menu_widget), FALSE);
				}
		}

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
		mathlibs[libidx].menu_widget = w;

		math_radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));

		if (libidx == current_mathlib->idx)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), TRUE);
		else
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), FALSE);

		// Add the menu item and pass the mathlib_t structure to the set_mathlib callback:
		g_signal_connect(GTK_MENU_ITEM(w), "toggled", G_CALLBACK(set_mathlib), (gpointer)&mathlibs[libidx]);
		gtk_menu_shell_append(GTK_MENU_SHELL(math_menu), w);

	}

	gtk_widget_show_all(main_window);
}

void mathlib_benchmark()
{
	struct timespec start, end;
	mathlib_t *mathlib_before_benchmark = current_mathlib;
	int response;
	char m[1024] = {0};
	int i;

	response = Notice(_("Mathlib Benchmark"),
		 _("This will run a frequency loop benchmark for each detected linear algebra library and then provide a summary.  It "
		 "may take some time to complete depending on how big and how many frequencies your NEC2 will use. "
		 "Detailed timing will be provided in the terminal.\n"
		 "\n"
		 "Notes:\n"
		 "* Only one Intel MKL library can be tested without restarting xnec2c, so select which one you wish"
		 "to benchmark before proceeding or it will choose the first in the list and skip the rest.\n\n"
		 "* Thread congestion will occur for multi-threaded libraries when using the -j N option if there are not enough "
		 "CPUs available to accomodate the forked processes in combination with library threads.  Consider reducing the value of -j N for "
		 "benchmarking to find what library works best with a mix of forking and threading; you can use `top` to monitor your CPU usage"
		 "and if it reaches 0% idle then you might consider reducing -j N.\n"
		 "\n"
		 "You may wish to experiment setting these environment variables:\n"
		 "  * OPENBLAS_NUM_THREADS=N\t# OpenBLAS thread limit\n"
		 "  * OMP_NUM_THREADS=N\t\t# OpenMP thread limit\n"
		 "  * MKL_NUM_THREADS=N\t\t# Intel MKL thread limit\n"
		 "\n"
		 "Click OK to proceed, this dialog will close when complete."),
		 GTK_BUTTONS_OK_CANCEL);

	if (response != GTK_RESPONSE_OK)
		return;
	
	for (i = 0; i < num_mathlibs; i++)
	{
		if (!mathlibs[i].available)
			continue;

		set_mathlib(NULL, &mathlibs[i]);
		printf("\nStarting %s benchmark\n", current_mathlib->name);


		save.last_freq = 0;
		SetFlag(FREQ_LOOP_INIT);
	
		clock_gettime(CLOCK_MONOTONIC, &start);
		while (Frequency_Loop(NULL));
		clock_gettime(CLOCK_MONOTONIC, &end);

		double elapsed = (end.tv_sec + (double)end.tv_nsec/1e9) - (start.tv_sec + (double)start.tv_nsec/1e9);
		snprintf(m + strlen(m), sizeof(m)-strlen(m)-1, "%s:\n   %f seconds\n\n",
			mathlibs[i].name,
			elapsed);
	}

	set_mathlib(NULL, mathlib_before_benchmark);

	Notice("Mathlib Benchmark", m, GTK_BUTTONS_OK);
	printf("%s", m);
}

int32_t zgetrf(int32_t order, int32_t m, int32_t n, complex double *a, int32_t ndim, int32_t *ip)
{
	int f_idx = MATHLIB_ZGETRF;

	if (current_mathlib == NULL)
	{
		printf("zgetrf: current_mathlib is NULL, this should never happen.\n");
		return 1;
	}

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrf_atlas_t *f;
		
		// This is an ugly cast, but see `man dlopen` for why.
		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order, m, n, a, ndim, (int32_t*)ip);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS || current_mathlib->type == MATHLIB_INTEL)
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

	if (current_mathlib == NULL)
	{
		printf("zgetrs: current_mathlib is NULL, this should never happen.\n");
		return 1;
	}

	//printf("%s: type=%d typename=%s\n", mathfuncs[f_idx], current_mathlib->type, current_mathlib->name);

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrs_atlas_t *f;
		
		*(void**)(&f) = current_mathlib->functions[f_idx];
		return f(order, trans, lda, nrhs, a, ndim, (int32_t*)ip, b, ldb);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS || current_mathlib->type == MATHLIB_INTEL)
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

/* Single Dynamic library threading
 * https://software.intel.com/content/www/us/en/develop/documentation/onemkl-linux-developer-guide/top/linking-your-application-with-the-intel-oneapi-math-kernel-library/linking-in-detail/dynamically-selecting-the-interface-and-threading-layer.html
 */

#define MKL_THREADING_INTEL         0
#define MKL_THREADING_SEQUENTIAL    1
#define MKL_THREADING_PGI           2 // Only for PGI compiler, not implemented here.
#define MKL_THREADING_GNU           3
#define MKL_THREADING_TBB           4

void mathlib_mkl_set_threading(mathlib_t *lib, int code)
{
	int (*mkl_set_threading_layer)(int) = NULL;

	// Skip this if not yet initialized:
	if (!lib->available)
		return;
	
	dlerror();
	*(void **) (&mkl_set_threading_layer) = dlsym(lib->handle, "MKL_Set_Threading_Layer");
	if (mkl_set_threading_layer == NULL)
	{
		printf("dlsym(mkl_set_threading_layer): %s\n", dlerror());
		return;
	}

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
