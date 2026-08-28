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

#ifndef RDPATTERN_NOISE_MENU_H
#define RDPATTERN_NOISE_MENU_H 1

#include "config/config_refresh.h"

/**
 * noise_model_menus_populate() - build the noise environment menu
 */
void noise_model_menus_populate(void);

/* Expose the hover-safe refresh shared by generated noise selector rows. */
extern const config_refresh_t hook_noise_env_refresh;

/**
 * hook_noise_sky_commit() - apply committed sky-model dependencies
 */
void hook_noise_sky_commit(void);

/**
 * hook_noise_earth_commit() - apply committed earth-model dependencies
 */
void hook_noise_earth_commit(void);

#endif /* RDPATTERN_NOISE_MENU_H */
