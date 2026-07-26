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

#ifndef __OPENGL_VIEW_FIT_H
#define __OPENGL_VIEW_FIT_H 1

#include "../common.h"

#ifdef HAVE_OPENGL
#include "../render/render_engine.h"

/**
 * gl_view_fit_view() - Fit a GL view to its drawn geometry
 * @view: structure or radiation-pattern view
 * @fit:  receives fitted zoom and screen-space pan
 *
 * Folds the GL content batches this view drew (radius scaling and model_scale
 * baked in) into the generic reducer, then inverts the framing into the view's
 * zoom and pan.  Returns FALSE when no geometry is present.
 */
gboolean gl_view_fit_view(view_t *view, view_fit_t *fit);

/* GL engine control-op vtable. */
extern const render_engine_ops_t gl_engine_ops;

#endif /* HAVE_OPENGL */
#endif /* __OPENGL_VIEW_FIT_H */
