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

#ifndef SY_OVERRIDES_H
#define SY_OVERRIDES_H    1

#include "common.h"
#include "optimizers/opt_simple.h"

/* Initialize the symbol overrides window */
gboolean sy_overrides_init(void);

/* Show the symbol overrides window */
void sy_overrides_show(void);

/* Hide the symbol overrides window */
void sy_overrides_hide(void);

/* Refresh the symbol overrides display from symbol table */
void sy_overrides_refresh(void);

/* Close window if no symbols exist */
void sy_overrides_close_if_empty(void);

/* Cleanup resources */
void sy_overrides_cleanup(void);

/* Signal handlers called from glade */
void on_show_sy_overrides_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_sy_overrides_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_sy_overrides_window_destroy(GObject *object, gpointer user_data);
void on_sy_overrides_apply_clicked(GtkButton *button, gpointer user_data);
void on_sy_overrides_cancel_clicked(GtkButton *button, gpointer user_data);
void on_sy_overrides_close_clicked(GtkButton *button, gpointer user_data);
void on_sy_show_defaults_toggled(GtkToggleButton *button, gpointer user_data);

/**
 * sy_overrides_save_state - save current UI state to .sy file
 *
 * Applies all override values from UI entries to the symbol table
 * and writes the .sy file.  Does not trigger Open_Input_File.
 * Called before starting optimizer to persist non-optimizer overrides.
 */
void sy_overrides_save_state(void);

/* Optimizer variable accessors */

/**
 * sy_overrides_get_opt_vars - build simple_var_t array from Opt-checked rows
 * @vars_out: output pointer, caller frees with sy_overrides_free_opt_vars
 *
 * Returns number of variables, or 0 if none flagged for optimization.
 */
int sy_overrides_get_opt_vars(simple_var_t **vars_out);

/**
 * sy_overrides_free_opt_vars - free array returned by sy_overrides_get_opt_vars
 * @vars: array to free
 * @num_vars: length
 */
void sy_overrides_free_opt_vars(simple_var_t *vars, int num_vars);

/**
 * sy_overrides_set_opt_results - update override entries from optimizer results
 * @vars: result variable array from simple_get_result
 * @num_vars: length
 */
void sy_overrides_set_opt_results(const simple_var_t *vars, int num_vars);

#endif
