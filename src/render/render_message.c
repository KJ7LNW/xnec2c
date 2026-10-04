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

#include "render_message.h"
#include "render_message_paint.h"
#include "render_surface.h"
#include "render_canvas_surface.h"
#include "../color/color_palette.h"
#include "../shared.h"

/* Frame request interval while an instance fades */
#define OVERLAY_TEXT_TICK_MS           16

/* Capacity a store array takes at its first element */
#define OVERLAY_TEXT_INITIAL_CAPACITY  4

/* Delay standing for no later appearance change */
#define OVERLAY_TEXT_NO_CHANGE         G_MAXINT64

/* What one live instance presents at a given time */
typedef struct
{
  gboolean presents;         /* shows a line in this frame */
  gboolean persists;         /* stays in the store after this frame */
  int      fade_elapsed_ms;  /* 0 while holding and for PER_FRAME */
  gint64   next_change_ms;   /* delay to its next appearance change */
} overlay_text_phase_t;

/* Phase of an instance presenting nothing and leaving the store */
static const overlay_text_phase_t overlay_text_retired =
{
  .next_change_ms = OVERLAY_TEXT_NO_CHANGE
};

static gboolean overlay_text_tick(gpointer user_data);

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_live_phase() - Resolve what a live instance presents at a time
 * @entry:  live instance
 * @now_us: monotonic time the frame or frame request resolves at
 */
  static overlay_text_phase_t
overlay_text_live_phase(const overlay_text_live_t *entry, gint64 now_us)
{
  const overlay_text_style_t *style = entry->instance.style;
  gint64 elapsed_ms = (now_us - entry->posted_us) / G_TIME_SPAN_MILLISECOND;
  overlay_text_phase_t phase = overlay_text_retired;

  switch( style->lifetime )
  {
    case OVERLAY_TEXT_PER_FRAME:
      /* Shown by the posting frame alone */
      phase.presents = TRUE;
      break;

    case OVERLAY_TEXT_TIMED:
      if( elapsed_ms < style->hold_ms )
        phase = (overlay_text_phase_t){ .presents = TRUE, .persists = TRUE,
          .next_change_ms = style->hold_ms - elapsed_ms };
      else if( elapsed_ms < style->hold_ms + style->fade_ms )
        phase = (overlay_text_phase_t){ .presents = TRUE, .persists = TRUE,
          .fade_elapsed_ms = (int)(elapsed_ms - style->hold_ms),
          .next_change_ms = OVERLAY_TEXT_TICK_MS };
      else
      {
        /* Faded out: no line, and the store drops the instance */
        phase = overlay_text_retired;
      }
      break;

    case OVERLAY_TEXT_LIFETIME_COUNT:
      BUG("overlay text lifetime %d names no retirement rule\n",
          (int)style->lifetime);
      break;
  }

  return( phase );

} /* overlay_text_live_phase() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_disarm() - Remove the armed frame request of a store
 * @store: store whose frame request ends
 */
  static void
overlay_text_disarm(overlay_text_store_t *store)
{
  if( store->timeout_id == 0 )
    return;

  g_source_remove(store->timeout_id);
  store->timeout_id = 0;

} /* overlay_text_disarm() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_arm() - Arm one frame request after a delay
 * @surface:  surface the request redraws
 * @delay_ms: delay to the request; OVERLAY_TEXT_NO_CHANGE arms none
 */
  static void
overlay_text_arm(render_surface_t *surface, gint64 delay_ms)
{
  if( delay_ms == OVERLAY_TEXT_NO_CHANGE )
    return;

  surface->overlay_text.timeout_id =
    g_timeout_add((guint)delay_ms, overlay_text_tick, surface);

} /* overlay_text_arm() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_schedule() - Re-arm the request for the next appearance change
 * @surface: surface whose store paces its timed instances
 *
 * Every delay derives from the clock, so re-arming never stretches a hold.
 */
  static void
overlay_text_schedule(render_surface_t *surface)
{
  overlay_text_store_t *store = &surface->overlay_text;
  gint64 now_us = g_get_monotonic_time();
  gint64 delay_ms = OVERLAY_TEXT_NO_CHANGE;
  int count = mem_array_count(store->live);
  int i;

  overlay_text_disarm(store);

  for( i = 0; i < count; i++ )
  {
    gint64 next_change_ms =
      overlay_text_live_phase(&store->live[i], now_us).next_change_ms;

    delay_ms = MIN(delay_ms, next_change_ms);
  }

  overlay_text_arm(surface, delay_ms);

} /* overlay_text_schedule() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_tick() - Request the frame an appearance change is due in
 * @user_data: surface whose store armed the request
 *
 * Returns G_SOURCE_REMOVE; the next request is armed from the clock.
 */
  static gboolean
overlay_text_tick(gpointer user_data)
{
  render_surface_t *surface = (render_surface_t *)user_data;

  /* The source ends with this dispatch, leaving no armed id to remove */
  surface->overlay_text.timeout_id = 0;

  canvas_surface_queue_redraw(surface);
  overlay_text_schedule(surface);

  return( G_SOURCE_REMOVE );

} /* overlay_text_tick() */

/*-----------------------------------------------------------------------*/

  void
overlay_text_post(render_surface_t *surface, const overlay_text_t *instance)
{
  const overlay_text_style_t *style;
  overlay_text_store_t *store;
  int count;

  if( surface == NULL || instance == NULL || instance->text == NULL
      || instance->style == NULL )
  {
    BUG("overlay text post lacks a surface, an instance, its text or its style\n");
    return;
  }

  style = instance->style;
  if( ((int)style->anchor < 0) || (style->anchor >= OVERLAY_TEXT_ANCHOR_COUNT)
      || ((int)style->lifetime < 0)
      || (style->lifetime >= OVERLAY_TEXT_LIFETIME_COUNT) )
  {
    BUG("overlay text style anchor %d or lifetime %d is out of range\n",
        (int)style->anchor, (int)style->lifetime);
    return;
  }

  if( (style->lifetime == OVERLAY_TEXT_TIMED)
      && ((style->hold_ms < 0) || (style->fade_ms <= 0)) )
  {
    BUG("timed overlay text style holds %d ms and fades %d ms;"
        " expected hold >= 0 and fade > 0\n", style->hold_ms, style->fade_ms);
    return;
  }

  store = &surface->overlay_text;
  count = mem_array_count(store->live);

  mem_array_reserve(&store->live, count + 1, OVERLAY_TEXT_INITIAL_CAPACITY);
  mem_array_realloc(&store->live, count + 1);
  store->live[count] = (overlay_text_live_t){ .instance = *instance,
    .posted_us = g_get_monotonic_time() };

  /* Only timed instances contribute a delay, so other posts arm none */
  overlay_text_schedule(surface);

} /* overlay_text_post() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_key_equal() - Compare the painted key with a frame key
 * @painted: key the image was painted from
 * @frame:   key the frame resolved
 *
 * Text compares by content, so one message reached through two literals
 * matches itself.
 */
  static gboolean
overlay_text_key_equal(const overlay_text_key_t *painted,
    const overlay_text_key_t *frame)
{
  int count = mem_array_count(frame->lines);
  gboolean equal = (painted->width == frame->width)
    && (painted->height == frame->height)
    && (painted->palette_gen == frame->palette_gen)
    && (mem_array_count(painted->lines) == count);
  int i;

  for( i = 0; equal && (i < count); i++ )
  {
    const overlay_text_line_t *painted_line = &painted->lines[i];
    const overlay_text_line_t *frame_line = &frame->lines[i];

    equal = (painted_line->instance.style == frame_line->instance.style)
      && (painted_line->fade_elapsed_ms == frame_line->fade_elapsed_ms)
      && (g_strcmp0(painted_line->instance.text,
            frame_line->instance.text) == 0);
  }

  return( equal );

} /* overlay_text_key_equal() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_resolve_lines() - Resolve the lines a frame presents
 * @store:  store whose live instances resolve into its scratch array
 * @now_us: monotonic time of the frame
 *
 * Lines keep post order, so stacked boxes place the newest at the bottom.
 */
  static void
overlay_text_resolve_lines(overlay_text_store_t *store, gint64 now_us)
{
  int count = mem_array_count(store->live);
  int lines = 0;
  int i;

  /* Size scratch before filling it: growing its count re-zeroes slots */
  mem_array_reserve(&store->scratch, count, OVERLAY_TEXT_INITIAL_CAPACITY);
  mem_array_realloc(&store->scratch, count);

  for( i = 0; i < count; i++ )
  {
    overlay_text_phase_t phase =
      overlay_text_live_phase(&store->live[i], now_us);

    if( !phase.presents )
      continue;

    store->scratch[lines] = (overlay_text_line_t){
      .instance = store->live[i].instance,
      .fade_elapsed_ms = phase.fade_elapsed_ms };
    lines++;
  }

  mem_array_realloc(&store->scratch, lines);

} /* overlay_text_resolve_lines() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_refresh() - Repaint the image when the frame key differs
 * @surface: surface whose store holds the resolved lines
 * @width:   drawable width in pixels
 * @height:  drawable height in pixels
 *
 * The painted key takes the resolved lines whole, and its former lines
 * become the next scratch array.
 */
  static void
overlay_text_refresh(render_surface_t *surface, int width, int height)
{
  overlay_text_store_t *store = &surface->overlay_text;
  overlay_text_key_t frame_key = { .width = width, .height = height,
    .palette_gen = color_palette_generation(), .lines = store->scratch };

  if( overlay_text_key_equal(&store->key, &frame_key) )
    return;

  store->scratch = store->key.lines;
  store->key = frame_key;

  cairo_surface_destroy(store->image);
  store->image = overlay_text_paint(surface);
  store->version++;

} /* overlay_text_refresh() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_retire() - Drop the instances a frame retires
 * @store:  store whose live instances compact in place
 * @now_us: monotonic time of the frame
 *
 * PER_FRAME instances leave once their frame hands off and TIMED ones once
 * faded out; survivors keep post order.
 */
  static void
overlay_text_retire(overlay_text_store_t *store, gint64 now_us)
{
  int count = mem_array_count(store->live);
  int kept = 0;
  int i;

  for( i = 0; i < count; i++ )
  {
    if( !overlay_text_live_phase(&store->live[i], now_us).persists )
      continue;

    store->live[kept] = store->live[i];
    kept++;
  }

  mem_array_realloc(&store->live, kept);

} /* overlay_text_retire() */

/*-----------------------------------------------------------------------*/

  render_image_t
overlay_text_frame(render_surface_t *surface, int width, int height)
{
  gint64 now_us;

  if( surface == NULL )
  {
    BUG("overlay text frame names no surface\n");
    return( (render_image_t){ .surface = NULL } );
  }

  now_us = g_get_monotonic_time();
  overlay_text_resolve_lines(&surface->overlay_text, now_us);
  overlay_text_refresh(surface, width, height);
  overlay_text_retire(&surface->overlay_text, now_us);

  return( (render_image_t){ .surface = surface->overlay_text.image,
      .version = surface->overlay_text.version } );

} /* overlay_text_frame() */

/*-----------------------------------------------------------------------*/

  void
overlay_text_release(render_surface_t *surface)
{
  overlay_text_store_t *store;

  if( surface == NULL )
  {
    BUG("overlay text release names no surface\n");
    return;
  }

  store = &surface->overlay_text;

  overlay_text_disarm(store);

  cairo_surface_destroy(store->image);
  store->image = NULL;

  mem_array_free(&store->live);
  mem_array_free(&store->scratch);
  mem_array_free(&store->key.lines);

} /* overlay_text_release() */

/*-----------------------------------------------------------------------*/
