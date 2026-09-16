/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#ifndef RC_CONFIG_H
#define RC_CONFIG_H 1

#include "common.h"
#include "utils.h"
#include "shared.h"
#include "config/widget/config_widget.h"
#include <sys/stat.h>

#define DEFAULT_CONFIG_FILE     ".xnec2c/xnec2c.conf"

// Compiled-in reset default for the OpenGL renderer toggle; enabled only in
// OpenGL builds.  Consumed by the rc_config_vars row for use_opengl_renderer.
#ifdef HAVE_OPENGL
# define RENDERER_RESET_DEFAULT 1
#else
# define RENDERER_RESET_DEFAULT 0
#endif

// Compiled-in default for one variable, discriminated by the row's format
// string exactly as parse_var/fprint_var dispatch on it.
typedef union {
	int i;
	float f;
	double d;
	const char *s;
} rc_config_default_t;

typedef struct rc_config_vars_t {
	size_t size;
	int ro;          // read-only field like version
	char *desc;
	char *format;
	void *vars[2];
	rc_config_default_t def[2];  // def[1] used only by pair formats (%d,%d)
	void (*init)(struct rc_config_vars_t *, char *);  // call this to initialize if not NULL
	int (*parse)(struct rc_config_vars_t *, char *); // call this to parse, ignore "format"
	int (*save)(struct rc_config_vars_t *, FILE *);  // call this to save, ignore "format"

	// Compute a runtime default in place of def[]; overrides the generic applier.
	void (*set_default)(struct rc_config_vars_t *);

	// True if the variable should not be set when rc_config.batch_mode is true.
	int batch_mode_skip;

	// Row's complete widget projection tree, or NULL when the field has no
	// widget binding.  Consumed by rc_config_register_widgets().
	const config_widget_tree_t *widgets;
} rc_config_vars_t;

extern rc_config_vars_t rc_config_vars[];
extern int num_rc_config_vars;

char *get_conf_dir(char *s, int len);

void rc_config_set_default(rc_config_vars_t *v);
void rc_config_apply_defaults(void);
rc_config_vars_t *rc_config_find_by_field(void *f);
int rc_config_default_int(void *f);
void rc_config_register_widgets(void);
size_t rc_config_field_size(const rc_config_vars_t *v);

#endif
