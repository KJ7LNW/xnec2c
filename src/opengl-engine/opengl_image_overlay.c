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

#include "opengl_image_overlay.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** gradient_overlay_new() - Allocate and initialize gradient overlay renderer
 */
  gradient_overlay_t*
gradient_overlay_new(void)
{
  gradient_overlay_t *overlay;

  overlay = g_new0(gradient_overlay_t, 1);

  overlay->base = cairo_gl_overlay_new();
  if( !overlay->base )
  {
    pr_err("Failed to create cairo overlay base\n");
    g_free(overlay);
    return( NULL );
  }

  return( overlay );

} /* gradient_overlay_new() */

/*-----------------------------------------------------------------------*/

/** gradient_overlay_free() - Free gradient overlay resources
 * @overlay: overlay instance to free
 */
  void
gradient_overlay_free(gradient_overlay_t *overlay)
{
  if( !overlay )
    return;

  cairo_gl_overlay_free(overlay->base);
  g_free(overlay);

} /* gradient_overlay_free() */

/*-----------------------------------------------------------------------*/

/** gradient_overlay_set_viewport() - Update viewport dimensions and mark texture for regeneration
 * @overlay: overlay instance
 * @width: viewport width in pixels
 * @height: viewport height in pixels
 */
  void
gradient_overlay_set_viewport(gradient_overlay_t *overlay, int width, int height)
{
  if( !overlay || !overlay->base )
    return;

  if( overlay->base->width != width || overlay->base->height != height )
  {
    cairo_gl_overlay_set_size(overlay->base, width, height);
  }

} /* gradient_overlay_set_viewport() */

/*-----------------------------------------------------------------------*/

/** gradient_overlay_upload_surface() - Upload pre-rendered surface to GL texture
 * @overlay: overlay instance
 * @surface: ARGB32 surface from gradient_cache
 * @version: cache version for staleness detection; skips upload when unchanged
 *
 * Compares version against last upload to detect staleness.  Re-uploads
 * only when the cache version has advanced since the last texture transfer.
 */
  void
gradient_overlay_upload_surface(gradient_overlay_t *overlay,
    cairo_surface_t *surface, uint64_t version)
{
  if( !overlay || !overlay->base || surface == NULL )
    return;

  if( overlay->last_version == version )
    return;

  cairo_gl_overlay_upload(overlay->base, surface);
  overlay->last_version = version;

} /* gradient_overlay_upload_surface() */

/*-----------------------------------------------------------------------*/

/** gradient_overlay_render() - Render gradient overlay in screen space
 * @overlay: overlay instance
 */
  void
gradient_overlay_render(gradient_overlay_t *overlay)
{
  if( !overlay || !overlay->base )
    return;

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  /* Cairo surfaces carry premultiplied alpha, anti-aliased edges included,
   * which the overlay shader passes through to the blend */
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  cairo_gl_overlay_render(overlay->base);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

} /* gradient_overlay_render() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
