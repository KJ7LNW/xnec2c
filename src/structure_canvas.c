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

/*
 * structure_canvas: Structure canvas surfaces and drawing setup.
 *
 * Registers the CANVAS_STRUCTURE surfaces with their scroll
 * capabilities, and readies the 2D projection buffer and viewport
 * after geometry input.
 */
#include "structure_canvas.h"
#include "shared.h"
#include "cairo/cairo_draw.h"
#include "cairo/cairo_frame.h"
#include "opengl/opengl_structure.h"
#include "prerender/prerender_aggregate.h"
#include "render/render_canvas.h"
#include "render/render_wire_radius.h"

/* Scroll capabilities of the structure domain */
surface_capability_t *const structure_input_caps[] = {
  &wire_radius_cap,
  NULL
};

/*-----------------------------------------------------------------------*/

/**
 * structure_surfaces_add() - Register the structure canvas surfaces
 *
 * Adopts the main window structure drawing area as the Cairo surface of
 * CANVAS_STRUCTURE and, in OpenGL builds, adds a GL surface to the same
 * container.  Both surfaces read structure_view, so structure_view_create()
 * runs first.
 */
  void
structure_surfaces_add( void )
{
  /* Get the structure drawing area */
  GtkWidget *structure_cairo_da =
    Builder_Get_Object( main_window_builder, "structure_drawingarea" );

  /* Register the Cairo surface once the view it presents exists. */
  canvas_add_surface( CANVAS_STRUCTURE,
      cairo_surface_adopt(structure_cairo_da, structure_view,
        structure_input_caps) );

  /* Create the GL surface after the view is initialized */
#ifdef HAVE_OPENGL
  canvas_add_surface( CANVAS_STRUCTURE,
      opengl_structure_surface_new(GTK_CONTAINER(
          gtk_widget_get_parent(structure_cairo_da))) );
#endif
}

/*-----------------------------------------------------------------------*/

/*  Init_Struct_Drawing()
 *
 *  Initializes drawing parameters after geometry input
 */
  void
Init_Struct_Drawing( void )
{
  mem_array_realloc(&structure_segs, (data.n + 4 * data.m));
  Prerender_Aggregate();

  /* Viewport only exists in the UI process; children skip this harmlessly.
   * Guard: skip if no geometry loaded (segments or patches required). */
  if( (data.n > 0 || data.m > 0) && structure_view != NULL )
    canvas_sync_viewport( CANVAS_STRUCTURE );
}

/*-----------------------------------------------------------------------*/
