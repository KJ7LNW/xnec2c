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

#ifndef PRERENDER_AGGREGATE_H
#define PRERENDER_AGGREGATE_H   1

#include "prerender_state.h"

/**
 * New_Patch_Data() - Precompute patch rectangle corners into geom_pre.patch_corners
 *
 * Allocates geom_pre.patch_corners[data.m], fills all four corners per patch
 * (center ± s*t1 ± s*t2), and returns the maximum distance from origin across
 * all corners. Called by Prerender_Aggregate(); parent-only.
 */
double New_Patch_Data(void);

/**
 * Prerender_Aggregate() - Compute geometry-derived aggregate scalars
 *
 * Populates geom_pre.scene_radius and geom_pre.patch_corners, and advances
 * geom_pre.generation to the version naming this deck's geometry. Called once
 * at GE-card time after geometry is established. Does not require excitation data.
 */
void Prerender_Aggregate(void);

#endif
