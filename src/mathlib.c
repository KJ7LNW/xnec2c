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

#include <dlfcn.h>
#include "main.h"
#include "shared.h"
#include "mathlib.h"

static mathlib_t mathlibs[] = {
	{.type = MATHLIB_ATLAS, .lib = "libtatlas.so", .name = "ATLAS, Threaded"},
	{.type = MATHLIB_ATLAS, .lib = "libsatlas.so", .name = "ATLAS, Serial"},

	{.type = MATHLIB_OPENBLAS, .lib = "libopenblas.so",  .name = "OpenBLAS+LAPACKe, Serial"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblaso.so", .name = "OpenBLAS+LAPACKe, OpenMP"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblasp.so", .name = "OpenBLAS+LAPACKe, pthreads"},

	{.type = MATHLIB_NONE, .lib = "(builtin)", .name = "NEC2 Gaussian Elimination"}

};

mathlib_t *current_mathlib = NULL;

// To add a new mathfunc:
//   * Update the enum in mathlib.h
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

void set_mathlib(GtkWidget * widget, gpointer mathlib)
{
	mathlib_t *lib = (mathlib_t*) mathlib;

	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		return;
	
	if (g_mutex_trylock(&global_lock))
	{
		current_mathlib = lib;
		printf("mathlib: selected %s\n", lib->name);
		g_mutex_unlock(&global_lock);
	}
	else
		Stop( _("mathlib: You cannot change the math library while the freq loop is running."), ERR_OK );

}

int open_mathlib(mathlib_t *lib)
{
	char fname[40];
	int fidx;

	if (lib->type == MATHLIB_NONE)
		return 1;
	
	lib->handle = dlopen(lib->lib, RTLD_NOW);
	if (lib->handle == NULL)
	{
		printf("Unable to open %s: %s\n", lib->lib, dlerror());
		return 0;
	}

	mem_alloc((void **) &lib->functions, sizeof(void *) * num_mathfuncs, __LOCATION__);
	for (fidx = 0; fidx < num_mathfuncs; fidx++)
	{
		// clear any error state
		dlerror();

		snprintf(fname, sizeof(fname) - 1, "%s%s%s", 
			lib->f_prefix ? lib->f_prefix : "",
			mathfuncs[fidx],
			lib->f_suffix ? lib->f_suffix : "");
		lib->functions[fidx] = dlsym(lib->handle, fname);

		char *error = dlerror();

		if (error != NULL)
		{
			printf("%s: unable to bind %s: %s\n", lib->lib, mathfuncs[fidx], error);
			free_ptr((void **) &lib->functions);
			dlclose(lib->handle);
			lib->handle = NULL;
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
		if (mathlibs[libidx].type == MATHLIB_ATLAS)
			mathlibs[libidx].f_prefix = "clapack_";
		else if (mathlibs[libidx].type == MATHLIB_OPENBLAS)
			mathlibs[libidx].f_prefix = "LAPACKE_";

		mathlibs[libidx].functions = NULL;

		printf("Trying %s (%s): ", mathlibs[libidx].name, mathlibs[libidx].lib);
		if (!open_mathlib(&mathlibs[libidx]))
			continue;
		else
			printf("loaded ok.\n");

		// Set the default to the first one we find:
		if (current_mathlib == NULL)
			current_mathlib = &mathlibs[libidx];
	}
}

void init_mathlib_menu()
{
	GSList *math_radio_group = NULL;
	GtkWidget *math_menu = Builder_Get_Object(main_window_builder, "main_mathlib_menu_menu");
	
	int libidx;
	for (libidx = 0; libidx < num_mathlibs; libidx++)
	{
		if (mathlibs[libidx].type != MATHLIB_NONE && mathlibs[libidx].handle == NULL)
			continue;

		GtkWidget *w = gtk_radio_menu_item_new_with_label(math_radio_group,
								  mathlibs[libidx].name);

		math_radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(w));
		if (libidx == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), TRUE);
		else
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), FALSE);

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
	else if (current_mathlib->type == MATHLIB_NONE)
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
	else if (current_mathlib->type == MATHLIB_NONE)
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
