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
 */

#ifndef __RENDER_SURFACE_H
#define __RENDER_SURFACE_H 1

#include "../view/view_core.h"
#include "render_engine.h"
#include "render_surface_input.h"

/* One drawing surface.  Every engine extends this through its own first
 * member, so a render_surface_t pointer converts exactly to and from the
 * engine's object. */
struct render_surface_s
{
  /* Presented widget, borrowed from GTK and never released here */
  GtkWidget *widget;

  /* Frame producer, serving both dispatch and canvas selection */
  const render_engine_t *engine;

  /* Borrowed view the surface shows; NULL on a surface showing none */
  view_t *view;

  /* Modifier scroll operations of this engine and domain; NULL declines both */
  const surface_input_ops_t *input;
};

/**
 * render_surface_init() - Initialize one engine-owned drawing surface
 * @surface: surface object to initialize
 * @widget:  GTK-owned widget presented by the surface
 * @engine:  engine producing frames for the surface
 * @view:    borrowed view shown by the surface, or NULL
 * @input:   modifier scroll operations of the presenting domain, or NULL
 *
 * Wires the pointer and allocation handlers once every member is assigned and
 * before the widget is packed, so the first allocation reaches a live surface.
 *
 * Returns FALSE when a required dependency is missing.
 */
static inline gboolean
render_surface_init(render_surface_t *surface, GtkWidget *widget,
    const render_engine_t *engine, view_t *view,
    const surface_input_ops_t *input)
{
  if( surface == NULL || widget == NULL || engine == NULL )
    return FALSE;

  surface->widget = widget;
  surface->engine = engine;
  surface->view = view;
  surface->input = input;

  surface_input_connect(surface);

  return TRUE;
}

#endif /* __RENDER_SURFACE_H */
