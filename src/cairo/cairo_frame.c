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

/*
 * cairo_frame: Cairo backend frame orchestration.
 *
 * Owns the Cairo vtables and the per-frame render_cairo() entry point.
 * render_cairo() applies Cairo settings, dispatches through the
 * backend-agnostic render(), flushes the scenebuffer, and paints
 * deferred axis labels.
 */

#include "cairo_frame.h"
#include "cairo_draw.h"
#include "cairo_fit.h"
#include "cairo_scenebuffer.h"
#include "../shared.h"
#include "../render/render_dispatch.h"

/* Per-view scenebuffers for Cairo rendering */
static cairo_scenebuffer_t structure_scenebuffer;
static cairo_scenebuffer_t rdpattern_scenebuffer;

/**
 * cairo_capture_pixbuf() - Capture a Cairo drawing window into a pixbuf
 * @widget: Cairo drawing widget
 * @width: capture width in pixels
 * @height: capture height in pixels
 *
 * Returns a newly allocated pixbuf, or NULL when the widget lacks a window.
 */
  GdkPixbuf *
cairo_capture_pixbuf(GtkWidget *widget, int width, int height)
{
  GdkWindow *window;

  if( widget == NULL || width <= 0 || height <= 0 )
    return NULL;

  window = gtk_widget_get_window(widget);
  if( window == NULL )
    return NULL;

  return gdk_pixbuf_get_from_window(window, 0, 0, width, height);

} /* cairo_capture_pixbuf() */

/*-----------------------------------------------------------------------*/

cairo_scenebuffer_t *
cairo_frame_get_scenebuffer(view_type_t type)
{
  switch( type )
  {
    case VIEW_STRUCTURE:
      return &structure_scenebuffer;

    case VIEW_RDPATTERN:
      return &rdpattern_scenebuffer;

    default:
      BUG("cairo_frame_get_scenebuffer: unknown view_type %d\n", type);
      return &structure_scenebuffer;
  }
}

void
cairo_frame_destroy(void)
{
  scenebuffer_destroy(&structure_scenebuffer);
  scenebuffer_destroy(&rdpattern_scenebuffer);
}

/* Cairo backend operations vtable; render() gates slot calls by mode */
const render_ops_t cairo_ops =
{
  .draw_farfield        = cairo_draw_farfield,
  .draw_nearfield       = cairo_draw_nearfield,
  .draw_structure         = cairo_draw_structure,
  .draw_structure_overlay = cairo_draw_structure,
  .draw_axes              = cairo_draw_axes,
  .init_empty           = NULL,
  .set_status           = cairo_set_status,
  .set_gradient         = cairo_set_gradient,
  .set_colors           = cairo_set_colors,
};

/*-----------------------------------------------------------------------*/

/**
 * render_cairo() - Produce one Cairo frame
 * @ctx: caller-built rendering context with frame resources and resolved colors
 *
 * Applies Cairo settings from rc_config, calls the presentation layer through
 * the Cairo engine's domain protocol, flushes the scenebuffer, and paints
 * deferred axis labels.
 */
  gboolean
render_cairo(cairo_render_ctx_t *ctx)
{
  cairo_t *cr = ctx->cr;
  view_t  *v  = ctx->view;

  if( v == NULL )
    return FALSE;

  cairo_set_antialias(cr, rc_config.cairo_antialias);
  cairo_set_line_cap(cr, rc_config.cairo_line_cap);

  /* Reset scenebuffer; leaf renderer callbacks will deposit segments */
  scenebuffer_reset(ctx->sb);

  cairo_flush_stats_t *stats = NULL;
#if CAIRO_FLUSH_STATS
  cairo_flush_stats_t _stats_buf;
  stats = &_stats_buf;
  gint64 t_frame_start   = g_get_monotonic_time();
#endif
  render((void *)ctx, cairo_engine.render, v);
#if CAIRO_FLUSH_STATS
  gint64 t_deposit_end   = g_get_monotonic_time();
#endif

  /* Clear the surface after render() deposits the active theme colors. */
  cairo_set_source_rgb_f(cr, ctx->background);
  cairo_rectangle(cr, 0.0, 0.0, (double)v->width, (double)v->height);
  cairo_fill(cr);

  /* Flush all accumulated segments in depth-sorted batches */
  scenebuffer_flush(ctx->sb, cr, stats);
#if CAIRO_FLUSH_STATS
  gint64 t_flush_end     = g_get_monotonic_time();
#endif

  /* Draw deferred axis labels on top of flushed segments */
  if( ctx->n_axis_labels > 0 )
  {
    PangoLayout *layout = canvas_pango_layout( canvas_of_view(v->type), NULL );
    int k;

    cairo_set_source_rgb_f(cr, ctx->view_axis_label);
    for( k = 0; k < ctx->n_axis_labels; k++ )
    {
      pango_layout_set_text(layout, ctx->axis_labels[k].text, -1);
      cairo_move_to(cr, (double)ctx->axis_labels[k].x,
          (double)ctx->axis_labels[k].y);
      pango_cairo_show_layout(cr, layout);
    }
    g_object_unref(layout);
  }

  /* Gradient legend surface resolved by render() via set_gradient callback;
   * non-NULL only when farfield mode is active on the rdpattern view. */
  if( ctx->gradient != NULL )
  {
    cairo_set_source_surface(cr, ctx->gradient, 0.0, 0.0);
    cairo_paint(cr);
  }

  /* Paint deferred status message on top of all rendered content */
  if( ctx->status_message != NULL )
    Draw_Centered_Message(cr, v->width, v->height, ctx->status_message);

#if CAIRO_FLUSH_STATS
  gint64 t_frame_end = g_get_monotonic_time();
  double ratio = (stats->batch_groups > 0)
      ? (double)stats->segments / (double)stats->batch_groups : 0.0;
  pr_debug("cairo: segs=%d groups=%d batch=%.1fx"
      " bins=%d deposit=%.2fms sort=%.2fms stroke=%.2fms"
      " flush=%.2fms total=%.2fms cap=%d\n",
      stats->segments, stats->batch_groups, ratio,
      rc_config.cairo_depth_bins,
      (double)(t_deposit_end - t_frame_start) / 1000.0,
      (double)stats->sort_us / 1000.0,
      (double)stats->stroke_us / 1000.0,
      (double)(t_flush_end - t_deposit_end) / 1000.0,
      (double)(t_frame_end - t_frame_start) / 1000.0,
      stats->capacity);
#endif

  return TRUE;
}

/*-----------------------------------------------------------------------*/
