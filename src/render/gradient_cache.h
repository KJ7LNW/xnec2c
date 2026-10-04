/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#ifndef GRADIENT_CACHE_H
#define GRADIENT_CACHE_H 1

#include <cairo.h>
#include <stdint.h>
#include "../prerender/presentation_cache_key.h"

/*
 * gradient_cache: Self-validating gradient legend surface cache.
 *
 * Called by the presentation layer (render_dispatch) during render();
 * the returned surface is forwarded to backends via render_check_result_t.
 * Stores the inputs that produced the cached surface alongside the
 * surface itself.  gradient_cache_update() compares stored inputs against
 * current authoritative state; a mismatch triggers a re-render.
 * No dirty flags — no per-call-site invalidation obligations.
 */
typedef struct
{
  cairo_surface_t *surface; /* cached ARGB32 gradient rendering */
  uint64_t version;         /* monotonic counter; increments on rebuild */
  int    width;             /* viewport width at render time */
  int    height;            /* viewport height at render time */
  presentation_cache_key_t cache_key; /* shared gain-scaling inputs */
} gradient_cache_t;

/*-----------------------------------------------------------------------
 * API
 *----------------------------------------------------------------------*/

/**
 * render_image_t - Painted image an engine composites
 * @surface: ARGB32 image; NULL when no image shows
 * @version: monotonic counter; increments on surface rebuild
 *
 * Couples surface with its version so consumers can gate re-upload
 * without inspecting cache internals or relying on pointer identity.
 */
typedef struct
{
  cairo_surface_t *surface;
  uint64_t         version;
} render_image_t;

/**
 * gradient_cache_get_overlay() - Presentation entry point for gradient legend
 * @w:       current viewport width
 * @h:       current viewport height
 *
 * Checks rc_config.rdpattern_gradient_key; returns {NULL, 0} when disabled
 * or dimensions are invalid.  Otherwise updates the module-internal singleton
 * cache and returns the rendered surface with its version.
 */
render_image_t gradient_cache_get_overlay(int w, int h);

#endif /* GRADIENT_CACHE_H */
