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
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include "main.h"
#include "shared.h"
#include "mathlib.h"

void mathlib_mkl_set_threading_intel(mathlib_t *lib);

static mathlib_t mathlibs[] = {
	// In order of preference, xnec2c will use the first one that loads successfully by default:

	// CentOS 7: yum install atlas-devel

	// ATLAS on openSUSE is not officially supported, see https://software.opensuse.org/package/libatlas3
	// and it needs /etc/ld.so.conf.d/atlas-x86_64.conf updated to point at /usr/lib64/atlas-basic/.
	// but it was giving me an Illegal instruction error, so probably not compiled for my CPU.

	// CentOS 7 and openSUSE use the same library names:
	{.type = MATHLIB_ATLAS, .lib = "libtatlas.so.3", .name = "ATLAS, Threaded", .f_prefix = "clapack_"},
	{.type = MATHLIB_ATLAS, .lib = "libsatlas.so.3", .name = "ATLAS, Serial", .f_prefix = "clapack_"},

	// CentOS 7: yum install openblas-devel # or openblas-{serial,threads,openmp}
	// openSUSE OpenBLAS: zypper install libopenblas_*0
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblas.so.0,libopenblas.so,libopenblas_serial.so.0",
		.name = "OpenBLAS+LAPACKe, Serial", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblaso.so.0,libopenblaso.so,libopenblas_openmp.so.0",
		.name = "OpenBLAS+LAPACKe, OpenMP", .f_prefix = "LAPACKE_"},
	{.type = MATHLIB_OPENBLAS, .lib = "libopenblasp.so.0,libopenblasp.so,libopenblas_pthreads.so.0",
		.name = "OpenBLAS+LAPACKe, pthreads", .f_prefix = "LAPACKE_"},

	// Ubuntu / Debian: apt-get install liblapacke libopenblas0-*
	// Tested Ubuntu 16.04, 18.04, 20.04, Debian 9, Debian 11
	//   Note that you may need to use `alternatives` to select your openblas implementation.
	//   Some combinations of BLAS/LAPACK may or may not work together:
	//     ~# update-alternatives --config libblas.so.3-x86_64-linux-gnu
	//     ~# update-alternatives --config liblapack.so.3-x86_64-linux-gnu
	{.type = MATHLIB_OPENBLAS, .lib = "liblapacke.so.3", .name = "Selected LAPACK+BLAS", .f_prefix = "LAPACKE_"},

	// Ubuntu / Debian: apt-get install libatlas3-base
	// Tested Ubuntu 16.04, 18.04, Debian 9, Debian 11)
	// Note: This requires the following in Debian 11 and Ubuntu 20.04:
	//   Note that you may need to use `alternatives` to select your openblas implementation:
	//     update-alternatives --config libblas.so.3-x86_64-linux-gnu
	//   Additionally, you may need to set this (or add the path to /etc/ld.so.conf.d/atlas.conf):
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
	char *libfn0 = NULL, *libfn, *token;
	char fname[40];
	int fidx;

	if (lib == NULL)
	{
		BUG("open_mathlib: lib is NULL\n");
		return 0;
	}

	// Builtin NEC2 Gaussian Elimination isn't a .so, just return success.
	if (lib->type == MATHLIB_NEC2)
		return 1;

	// Set environment if configured:
	if (lib->env[0] != NULL && setenv(lib->env[0], lib->env[1], 1) < 0)
			pr_debug("setenv(%s, %s): %s\n", lib->env[0], lib->env[1], strerror(errno));

	// Clear any error state
	dlerror();

	// Open the .so library, split on a comma (,):
	mem_alloc((void**)&libfn0, strlen(lib->lib)+1, __LOCATION__);
	strcpy(libfn0, lib->lib);
	libfn = libfn0;
	while ((token = strtok_r(libfn, ",", &libfn)) != NULL)
	{
		lib->handle = dlopen(token, RTLD_NOW);
		if (lib->handle == NULL)
		{
			pr_info("%s: %s\n", lib->name, dlerror());
			close_mathlib(lib);
		}
		else
		{
			pr_info("%s: opened %s\n", lib->name, token);
			break;
		}
	}
	free_ptr((void**)&libfn0);

	if (lib->handle == NULL)
	{
		pr_info("%s: library not found or nonfunctional, skipping: %s\n", lib->name, lib->lib);
		return 0;
	}

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
			pr_err("  %s: unable to bind %s: %s\n", lib->lib, mathfuncs[fidx], error);
			close_mathlib(lib);
			break;
		}
	}

	if (lib->handle != NULL)
		return 1;
	else
		return 0;
}


void init_mathlib(void)
{
	int libidx;

	for (libidx = 0; libidx < num_mathlibs; libidx++)
	{
		// Initialization:
		mathlibs[libidx].idx = libidx;
		mathlibs[libidx].functions = NULL;

		// Try to open each library:
		if (!open_mathlib(&mathlibs[libidx]))
		{
			close_mathlib(&mathlibs[libidx]);

			mathlibs[libidx].available = 0;
			continue;
		}
		else
			mathlibs[libidx].available = 1;

		// At this point the library load was successful, provide detail:
		pr_notice("%s: loaded\n", mathlibs[libidx].name);

		// Set the default to the first one we find:
		if (current_mathlib == NULL)
			current_mathlib = &mathlibs[libidx];
		else
			// Otherwise close it for now and re-open on use.
			close_mathlib(&mathlibs[libidx]);
	}

	if (!mathlibs[rc_config.mathlib_idx].available)
		for (libidx = 0; libidx < num_mathlibs; libidx++)
			if (mathlibs[libidx].available)
			{
				set_mathlib_interactive(NULL, &mathlibs[libidx]);
				break;
			}

	if (!mathlibs[rc_config.mathlib_batch_idx].available)
		for (libidx = 0; libidx < num_mathlibs; libidx++)
			if (mathlibs[libidx].available)
			{
				rc_config.mathlib_batch_idx = libidx;
				break;
			}

}


/////////////////////////////////////////////////////////////////////
//                                 SAVE/RESTORE FUNCTIONS (rc_config)
// Restore selection on open:
void mathlib_config_init(rc_config_vars_t *v, char *line)
{
	if (rc_config.mathlib_idx >= num_mathlibs || rc_config.mathlib_idx < 0)
	{
		pr_warn("mathlib_idx out of range, clamping to zero: 0 !< %d !< %d\n",
			rc_config.mathlib_idx, num_mathlibs);
		rc_config.mathlib_idx = 0;
	}

	if (rc_config.mathlib_batch_idx >= num_mathlibs || rc_config.mathlib_batch_idx < 0)
	{
		pr_warn("mathlib_batch_idx out of range, clamping to zero: 0 !< %d !< %d\n",
			rc_config.mathlib_batch_idx, num_mathlibs);
		rc_config.mathlib_batch_idx = 0;
	}

	if (mathlibs[rc_config.mathlib_idx].available)
		set_mathlib_interactive(NULL, &mathlibs[rc_config.mathlib_idx]);
	else if (!rc_config.first_run)
		pr_err("%s was not detected: Unable to set the mathlib index to %d\n",
		       mathlibs[rc_config.mathlib_idx].name, rc_config.mathlib_idx);
}

int mathlib_config_benchmark_parse(rc_config_vars_t *v, char *line)
{
	int i, len = strlen(line);

	for (i = 0; i < num_mathlibs && i < len; i++)
		if (line[i] == '1' && mathlibs[i].available)
			mathlibs[i].benchmark = 1;

	return 1;
}

int mathlib_config_benchmark_save(rc_config_vars_t *v, FILE *fp)
{
	int i;

	for (i = 0; i < num_mathlibs; i++)
		fprintf(fp, "%c", mathlibs[i].benchmark ? '1' : '0');

	return 1;
}



/////////////////////////////////////////////////////////////////////
//                                                      GTK FUNCTIONS

void set_mathlib_interactive(GtkWidget *widget, mathlib_t *lib)
{
	if (widget != NULL && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		return;

	if (lib == NULL)
	{
		BUG("set_mathlib_interactive: lib == NULL\n");
		return;
	}

	if (current_mathlib == lib)
		return;

	if (!lib->available)
	{
		pr_err("set_mathlib_interactive[%d]: mathlib.available=0, skipping: %s\n",
		        getpid(), lib->name);

		// If the library is unavailable for some reason then clear the one that was
		// just set and reset the active one:
		if (!CHILD)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), FALSE);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(current_mathlib->interactive_widget), TRUE);
		}
		return;
	}

	if (g_mutex_trylock(&global_lock))
	{
		close_mathlib(current_mathlib);
		current_mathlib = lib;

		// Save selection on exit:
		rc_config.mathlib_idx = current_mathlib->idx;

		open_mathlib(lib);

		g_mutex_unlock(&global_lock);
	}
	else
		Stop( _("mathlib: You cannot change the math library while the freq loop is running."), ERR_OK );

}

// Set batch mathlib, this will only be used by child processes so
// don't close and reopen the mathlib:
void set_mathlib_batch(GtkWidget *widget, mathlib_t *lib)
{
	if (widget != NULL && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		return;

	if (lib == NULL)
	{
		BUG("set_mathlib_batch: lib == NULL\n");
		return;
	}

	if (!FORKED)
		Notice(_("Batch Math Library"),
			_("Selecting a batched math library has no effect unless -j is specified on the command line. "
			"(However, this selection will be saved for next time xnec2c is opened.)\n"),
			GTK_BUTTONS_OK);

	rc_config.mathlib_batch_idx = lib->idx;
}

void set_mathlib_benchmark(GtkWidget *widget, mathlib_t *lib)
{
	int i, state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));

	// If lib==NULL then this is a select-all/select-none request:
	// Clear any checkboxes that have widgets even if they are disabled (like Intel)
	// but only set benchmark=1 if available.
	if (lib == NULL)
	{
		for (i = 0; i < num_mathlibs; i++)
			if (mathlibs[i].benchmark_widget != NULL &&
				(!state || mathlibs[i].available))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mathlibs[i].benchmark_widget), state);
				mathlibs[i].benchmark = state;
			}
	}
	// Otherwise set the state:
	else
	{
		lib->benchmark = state;

		// Only allow one Intel mathlib to be set, clear all but the selected if activating:
		if (state && lib->type == MATHLIB_INTEL)
			for (i = 0; i < num_mathlibs; i++)
			{
				if (mathlibs[i].available &&
					mathlibs[i].benchmark_widget != NULL &&
					mathlibs[i].type == MATHLIB_INTEL &&
					i != lib->idx)
						gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mathlibs[i].benchmark_widget), FALSE);
			}

		pr_info("%s: benchmark=%d\n", lib->name, lib->benchmark);
	}
}

GSList *add_mathlib_menu(mathlib_t *lib, GtkWidget *menu, GtkWidget *item, int state, void (*callback)(void))
{
		GSList *group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), state);

		// Add the menu item and pass the mathlib_t structure to the callback:
		g_signal_connect(GTK_MENU_ITEM(item), "toggled", G_CALLBACK(callback), lib);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

		return group;
}

void init_mathlib_menu(void)
{
	GSList *interactive_radio_group = NULL;
	GSList *batch_radio_group = NULL;
	GtkWidget *interactive_menu = Builder_Get_Object(main_window_builder, "main_mathlib_interactive_menu_menu");
	GtkWidget *batch_menu = Builder_Get_Object(main_window_builder, "main_mathlib_batch_menu_menu");
	GtkWidget *benchmark_menu = Builder_Get_Object(main_window_builder, "main_mathlib_benchmark_select_menu_menu");

	int libidx;
	for (libidx = 0; libidx < num_mathlibs; libidx++)
	{
		// Skip .so mathlibs that aren't available on the system:
		if (!mathlibs[libidx].available)
			continue;

		// Create the interactive menu item widget:
		mathlibs[libidx].interactive_widget =
			gtk_radio_menu_item_new_with_label(interactive_radio_group, mathlibs[libidx].name);

		interactive_radio_group = add_mathlib_menu(&mathlibs[libidx],
			interactive_menu,
			mathlibs[libidx].interactive_widget,
			libidx == current_mathlib->idx,
			G_CALLBACK(set_mathlib_interactive));

		// Create the batch menu item widget:
		mathlibs[libidx].batch_widget =
			gtk_radio_menu_item_new_with_label(batch_radio_group, mathlibs[libidx].name);

		batch_radio_group = add_mathlib_menu(&mathlibs[libidx],
			batch_menu,
			mathlibs[libidx].batch_widget,
			libidx == rc_config.mathlib_batch_idx,
			G_CALLBACK(set_mathlib_batch)
			);

		// Create the benchmark select widget:
		mathlibs[libidx].benchmark_widget = gtk_check_menu_item_new_with_label(mathlibs[libidx].name);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mathlibs[libidx].benchmark_widget),
			mathlibs[libidx].benchmark);
		g_signal_connect(GTK_MENU_ITEM(mathlibs[libidx].benchmark_widget),
			"toggled",
			G_CALLBACK(set_mathlib_benchmark),
			(gpointer)&mathlibs[libidx]);
		gtk_menu_shell_append(GTK_MENU_SHELL(benchmark_menu), mathlibs[libidx].benchmark_widget);
	}

	gtk_widget_show_all(main_window);
}

void mathlib_help(void)
{
	Notice(_("Mathlib Help"),
		"Accelerated math libraries such as ATLAS, OpenBLAS and Intel MKL can speed up xnec2c EM simulations "
		"if available on your platform. Library detection details are available in the terminal.\n"
		"\n"
		"* The selected \"Interactive\" math library is used when the frequency is "
		"changed in the UI by changing a Frequency MHz +/- widget or (right-)clicking on the frequency plot\n"
		"\n"
		"* The selected \"Batch\" math library is used when -j is specified on the command line to fork "
		"multiple jobs to run in parallel.\n"
		"\n"
		"For Intel libraries, only one library can be selected at a time.  Once one of the Intel MKL "
		"math libraries has been used for EM calculations the other Intel library options will be locked "
		"until restart.  Since xnec2c remembers the previously selected library it may activate when a "
		".NEC file is opened and lock other options.  To avoid this, close xnec2c and re-open it without "
		"opening a .NEC file (and without specifying a .NEC file on the command line) so you can change "
		"the option.\n"
		"\n"
		"\n"
		"== Math Libraries by Distribution ==\n"
		"* CentOS: yum install atlas-devel openblas-devel\n"
		"\n"
		"* Debian/Ubuntu: apt-get install liblapacke libopenblas*-* libatlas3-base\n"
		"    Note that you may need to use `alternatives` to select your\n"
		"    openblas implementation. Some combinations of BLAS/LAPACK may\n"
		"    or may not work together:\n"
		"      ~# update-alternatives --config libblas.so.3-x86_64-linux-gnu\n"
		"      ~# update-alternatives --config liblapack.so.3-x86_64-linux-gnu\n"
		"\n"
		"* openSUSE: zypper install libopenblas_*0\n"
		"  ATLAS is not officially supported by openSUSE so build from source\n"
		"\n"
		"This text is available in the terminal so you can copy-paste.\n"

		,
		GTK_BUTTONS_OK);
}

void mathlib_benchmark_help(void)
{
	Notice(_("Mathlib Benchmark Help"),
		_(
		"Jobs are forked and run in parallel by xnec2c, "
		"whereas threads are used by the mathlib implementation to parallelize the linear "
		"algebra calculation of one job.  For parallel algorithms you can reduce the number of threads "
		"in use by the algorithm with the following environment variables:\n"
		 "  * OPENBLAS_NUM_THREADS=N\t# OpenBLAS thread limit\n"
		 "  * OMP_NUM_THREADS=N\t\t# OpenMP thread limit\n"
		 "  * MKL_NUM_THREADS=N\t\t# Intel MKL thread limit\n"
		"\n"
		"Each benchmark specifies a complexity to indicate approximately how long it will take "
		"to run.  For example, O(N) means the time will scale with N and O(N*J) means it will "
		"scale with N*J:\n"
		"  * N is the number of mathlibs selected here:\n"
		"      File->Mathlib Benchmarks->Selected Mathlibs\n"
		"  * J is the number of jobs specified on the command line with -j (eg, -j 8)\n"
		"\n"
		"Higher complexity options run more benchmarks to provide additional information about how you "
		"might optimize your xnec2c runtime.  The benchmark menu options are as follows:  \n"
		"\n"
		"* Parallel (-j N): O(N/J) time: Benchmark each mathlib in parallel with the number of jobs "
		"specified on the command line with -j.\n"
		"\n"
		"* Single Job (-j 1): O(N) time: Benchmark each mathlib with a single job even if -j was "
		"specified on the command line. This is useful to find the fastest interactive "
		"mathlib that is used when the frequency is changed in the UI by changing a "
		"Frequency MHz +/- widget or (right-)clicking on the frequency plot\n"
		"\n"
		"* Iterate -j N/=2: O(N*log2(J)) time: Benchmark each mathlib in parallel initially with the number of jobs "
		"specified on the command line with -j, and then reduce the number jobs by half for each "
		"subsequent iteration until only one job remains.  This is useful to find a balance between "
		"the number of parallel jobs specified by -j and the number of threads used by the linear algebra "
		"library as modified by the environment variables listed above.\n"
		"\n"
		"* Iterate -j N-=2: O(N*J) time: Same as -j N/=2 above, but instead decrement the number of jobs "
		"by 1.  This takes the longest to run.\n"),
		GTK_BUTTONS_OK);
}

void mathlib_benchmark(int slow)
{
	struct timespec start, end;
	mathlib_t *mathlib_before_benchmark = current_mathlib;
	mathlib_t *mathlib_batch_before_benchmark = &mathlibs[rc_config.mathlib_batch_idx];
	mathlib_t *best_mathlib = NULL, *active_mathlib = NULL;
	int response;
	char m[10240] = {0};
	int i, best_num_jobs = 0;
	double best_elapsed = 0;

	if (isFlagSet(OPTIMIZER_OUTPUT))
	{
		Notice(_("Mathlib Benchmark"),
			_("Benchmarks are disabled while optimization is activated."),
			GTK_BUTTONS_OK);
		return;
	}

	if (calc_data.num_jobs == 1 && slow != MATHLIB_BENCHMARK_SINGLE)
		Notice(_("Mathlib Benchmark"),
			_("Choosing a benchmark other than \"Single Job\" has no effect "
			  "unless -j is specified on the command line."),
			GTK_BUTTONS_OK);


	// Make sure a library is selected.  We assume if .benchmark=1 that the
	// library is available.
	for (i = 0; i < num_mathlibs; i++)
	{
		if (!mathlibs[i].benchmark)
			continue;
		else
			break;
	}

	if (i == num_mathlibs)
    {
		Notice(_("Mathlib Benchmark"),
			_("You must select at least one math library to benchmark"),
			GTK_BUTTONS_CLOSE);
		return;
	}

	response = Notice(_("Mathlib Benchmark"),
		 _("This will run a frequency loop benchmark for each detected linear algebra library and then provide a summary.  It "
		 "may take some time to complete depending on how big and how many frequencies your NEC2 will use. "
		 "Detailed timing will be provided in the terminal.\n"
		 "\n"
		 "Notes:\n"
		 "* Only one Intel MKL library can be tested without restarting xnec2c, so select which one you wish"
		 "to benchmark before proceeding or it will choose the first in the list and skip the rest.\n\n"
		 "* Thread congestion will occur for multi-threaded libraries when using the -j N option if there are not enough "
		 "CPUs available to accommodate the forked processes in combination with library threads.  Consider reducing the value of -j N for "
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

	
	// DRAW_CURRENTS queues up too many New_Frequency requests during a benchmark
	// so disable it during the benchmark and put it back after:
	int orig_draw_currents = isFlagSet(DRAW_CURRENTS);
	ClearFlag(DRAW_CURRENTS);

	int orig_jobs = calc_data.num_jobs;

	for (i = 0; i < num_mathlibs; i++)
	{
		if (!mathlibs[i].benchmark)
			continue;

		if (FORKED)
			set_mathlib_batch(NULL, &mathlibs[i]);
		else
			set_mathlib_interactive(NULL, &mathlibs[i]);

		active_mathlib = get_mathlib_by_idx(i);

		// Sleep to let the threads settle before the next test:
		usleep(100000);

		snprintf(m + strlen(m), sizeof(m)-strlen(m)-1, "%s:\n", mathlibs[i].name);

		double elapsed_prev = 0;

		if (slow == MATHLIB_BENCHMARK_SINGLE)
			calc_data.num_jobs = 1;
		else
			calc_data.num_jobs = orig_jobs;

		while (calc_data.num_jobs >= 1)
		{
			pr_info("Starting %s benchmark (-j %d)\n",
				active_mathlib->name, calc_data.num_jobs);

			New_Frequency_Reset_Prev();
			calc_data.fmhz_save = 0;

			SetFlag(FREQ_LOOP_INIT);

			clock_gettime(CLOCK_MONOTONIC, &start);
			while (Frequency_Loop(NULL));
			clock_gettime(CLOCK_MONOTONIC, &end);

			double elapsed = (end.tv_sec + (double)end.tv_nsec/1e9) - (start.tv_sec + (double)start.tv_nsec/1e9);

			snprintf(m + strlen(m), sizeof(m)-strlen(m)-1, "   %f seconds (-j %2d) %c\n",
				elapsed,
				calc_data.num_jobs,
				(elapsed > elapsed_prev ? ' ' : '<')
				);

			if (best_mathlib == NULL || elapsed < best_elapsed)
			{
				best_mathlib = active_mathlib;
				best_elapsed = elapsed;
				best_num_jobs = calc_data.num_jobs;
			}

			elapsed_prev = elapsed;

			// These only iterate once with the same -j option:
			if (slow == MATHLIB_BENCHMARK_SINGLE || slow == MATHLIB_BENCHMARK_PARALLEL)
				break;

			// These iterate multiple times by reducing the number of jobs each time:
			g_mutex_lock(&global_lock);

			if (slow == MATHLIB_BENCHMARK_NLOG2)
				calc_data.num_jobs >>= 1;

			else if (slow == MATHLIB_BENCHMARK_NJ)
				calc_data.num_jobs--;

			g_mutex_unlock(&global_lock);
		}
	}

	if (FORKED)
		set_mathlib_batch(NULL, mathlib_batch_before_benchmark);
	else
		set_mathlib_interactive(NULL, mathlib_before_benchmark);

	if (orig_draw_currents)
		SetFlag(DRAW_CURRENTS);

	g_mutex_lock(&global_lock);
	calc_data.num_jobs = orig_jobs;
	g_mutex_unlock(&global_lock);

	if (best_mathlib != NULL)
		snprintf(m + strlen(m), sizeof(m)-strlen(m)-1, "\nBest Mathlib: %s (-j %d): %f seconds\n",
			best_mathlib->name, best_num_jobs, best_elapsed);
	else
		snprintf(m + strlen(m), sizeof(m)-strlen(m)-1, "\nNo result found?  This is a bug.");

	Notice(_("Mathlib Benchmark"), m, GTK_BUTTONS_OK);
}

void mathlib_benchmark_parallel(void)
{
	mathlib_benchmark(MATHLIB_BENCHMARK_PARALLEL);
}

void mathlib_benchmark_single(void)
{
	mathlib_benchmark(MATHLIB_BENCHMARK_SINGLE);
}

void mathlib_benchmark_nlog2(void)
{
	mathlib_benchmark(MATHLIB_BENCHMARK_NLOG2);
}

void mathlib_benchmark_nj(void)
{
	mathlib_benchmark(MATHLIB_BENCHMARK_NJ);
}

void mathlib_lock_intel(int locked_idx, int batch)
{
	static int warned = 0;

	mathlib_t *locked_lib;
	int i;

	locked_lib = get_mathlib_by_idx(locked_idx);
	if (locked_lib == NULL)
	{
		BUG("mathlib_lock_intel: Cannot find library for mathlib idx=%d\n",
		        locked_idx);
		return;
	}

	if (locked_lib->type != MATHLIB_INTEL)
		return;

	if (!warned && !CHILD)
	{
		warned = 1;
		pr_notice("\n"
		        "* Notice: \"%s\" was selected, but you can only change to a\n"
		        "* different Intel MKL threading library by restarting xnec2c.\n"
		        "* \n"
		        "* However, you may continue to select any other Linear Algebra library\n"
		        "* without restart, including this same Intel MKL library.  All other\n"
		        "* Intel MKL threading libraries are disabled until restart because of an \n"
		        "* MKL limitation where the MKL threading library can only be dynamically\n"
		        "* linked only once per runtime.\n"
				"* \n"
		        "* See Intel's documentation on mkl_set_threading_layer() for more detail.\n",
		        locked_lib->name);
	}

	// Disable the other MKL threading options.
	for (i = 0; i < num_mathlibs; i++)
		if (mathlibs[i].type == MATHLIB_INTEL && locked_idx != i)
		{
			mathlibs[i].available = 0;

			if (!CHILD)
			{
				if (mathlibs[i].interactive_widget != NULL && !batch)
					gtk_widget_set_sensitive(GTK_WIDGET(mathlibs[i].interactive_widget), FALSE);

				if (mathlibs[i].batch_widget != NULL && batch)
					gtk_widget_set_sensitive(GTK_WIDGET(mathlibs[i].batch_widget), FALSE);

				if (mathlibs[i].benchmark_widget != NULL &&
					((FORKED && batch) || !FORKED))
				{
					gtk_widget_set_sensitive(GTK_WIDGET(mathlibs[i].benchmark_widget), FALSE);
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mathlibs[i].benchmark_widget), FALSE);
				}
			}

		}
}

void mathlib_lock_intel_interactive(int locked_idx)
{
	mathlib_lock_intel(locked_idx, 0);
}

void mathlib_lock_intel_batch(int locked_idx)
{
	mathlib_lock_intel(locked_idx, 1);
}

void *mathlib_get_func(int f_idx)
{
	// Intel libraries can only be set once, so lock it:
	if (current_mathlib->type == MATHLIB_INTEL)
		mathlib_lock_intel_interactive(current_mathlib->idx);

	if (current_mathlib->functions == NULL)
		return NULL;

	return current_mathlib->functions[f_idx];
}


int32_t zgetrf(int32_t order, int32_t m, int32_t n, complex double *a, int32_t ndim, int32_t *ip)
{
	void *f_ptr = mathlib_get_func(MATHLIB_ZGETRF);

	if (current_mathlib == NULL)
	{
		BUG("zgetrf: current_mathlib is NULL, this should never happen.\n");
		return 1;
	}

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrf_atlas_t *f;

		// This is an ugly cast, but see `man dlopen` for why.
		*(void**)(&f) = f_ptr;
		return f(order, m, n, a, ndim, (int32_t*)ip);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS || current_mathlib->type == MATHLIB_INTEL)
	{
		zgetrf_openblas_t *f;
		*(void**)(&f) = f_ptr;
		return f(order, m, n, a, ndim, (int32_t*)ip);
	}
	else if (current_mathlib->type == MATHLIB_NEC2)
	{
		// use the original NEC2 function
		return factr_gauss_elim(n, a, (int32_t*)ip, ndim);
	}
	else
		BUG("%s: unsupported mathlib type %d\n", __func__,
			current_mathlib->type);

	return 1;
}

int32_t zgetrs(int32_t order, int32_t trans, int32_t lda, int32_t nrhs,
	complex double *a, int32_t ndim, int32_t *ip, complex double *b, int32_t ldb)
{
	void *f_ptr = mathlib_get_func(MATHLIB_ZGETRS);

	if (current_mathlib == NULL)
	{
		BUG("zgetrs: current_mathlib is NULL, this should never happen.\n");
		return 1;
	}

	//pr_debug("%s: type=%d typename=%s\n", mathfuncs[f_idx], current_mathlib->type, current_mathlib->name);

	if (current_mathlib->type == MATHLIB_ATLAS)
	{
		zgetrs_atlas_t *f;

		*(void**)(&f) = f_ptr;
		return f(order, trans, lda, nrhs, a, ndim, (int32_t*)ip, b, ldb);
	}
	else if (current_mathlib->type == MATHLIB_OPENBLAS || current_mathlib->type == MATHLIB_INTEL)
	{
		zgetrs_openblas_t *f;

		*(void**)(&f) = f_ptr;
		return f(order,
			(trans) == CblasConjTrans ? 'C' : ((trans) == CblasTrans ? 'T' : 'N'),
			lda, nrhs, a, ndim, (int32_t*)ip, b, ldb);
	}
	else if (current_mathlib->type == MATHLIB_NEC2)
	{
		if (lda != ldb)
			BUG("zgetrs warning: lda(%d) != ldb(%d)\n", lda,
				ldb);

		// use the original NEC2 function
		return solve_gauss_elim(lda, a, (int32_t*)ip, b, ndim);
	}
	else
		BUG("%s: unsupported mathlib type %d\n", __func__,
			current_mathlib->type);

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
		pr_err("dlsym(mkl_set_threading_layer): %s\n", dlerror());
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
