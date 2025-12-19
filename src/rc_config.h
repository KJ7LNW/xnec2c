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
#include <sys/stat.h>

#define DEFAULT_CONFIG_FILE     ".xnec2c/xnec2c.conf"

typedef struct rc_config_vars_t {
	size_t size;
	int ro;          // read-only field like version
	char *desc;
	char *format;
	void *vars[2];
	void (*init)(struct rc_config_vars_t *, char *);  // call this to initialize if not NULL
	int (*parse)(struct rc_config_vars_t *, char *); // call this to parse, ignore "format"
	int (*save)(struct rc_config_vars_t *, FILE *);  // call this to save, ignore "format"

	// True if the variable should not be set when rc_config.batch_mode is true.
	int batch_mode_skip;

	// GTK UI Integration:

	// A pointer to the builder window, such as &main_window_builder.
	GtkBuilder **builder_window;

	// String id="<id>" name from resources/xnec2c.glade for GtkCheckMenuItem objects:
	char *builder_check_menu_item_id;
} rc_config_vars_t;

extern rc_config_vars_t rc_config_vars[];
extern int num_rc_config_vars;

char *get_conf_dir(char *s, int len);

#endif
