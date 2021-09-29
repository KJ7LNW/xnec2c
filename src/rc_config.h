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
 */

#ifndef RC_CONFIG_H
#define RC_CONFIG_H 1

#include "common.h"
#include "utils.h"
#include "shared.h"
#include <sys/stat.h>

#define CONFIG_FILE     ".xnec2c/xnec2c.conf"

typedef struct rc_config_vars_t {
	size_t size;
	int ro;          // read-only field like version
	char *desc;
	char *format;
	void *vars[2];
	void (*init)(struct rc_config_vars_t *);  // call this to initialize if not NULL
} rc_config_vars_t;

extern rc_config_vars_t rc_config_vars[];
extern int num_rc_config_vars;

#endif
