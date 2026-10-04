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
#include "render_surface.h"
#include "render_canvas_surface.h"
#include "../color/color_palette.h"
#include "../shared.h"
#include "../themes/theme.h"

/* Frame request interval while an instance fades */
#define OVERLAY_TEXT_TICK_MS           16

/* Text size in pixels, absolute so every engine presents the same text */
#define OVERLAY_TEXT_FONT_PX           16

/* Margin between the text and the edges of its box, in pixels */
#define OVERLAY_TEXT_PADDING           12.0

/* Opacity of the box behind the text */
#define OVERLAY_TEXT_BOX_ALPHA         0.7

/* Capacity a store array takes at its first element */
#define OVERLAY_TEXT_INITIAL_CAPACITY  4

/* Delay standing for no later appearance change */
#define OVERLAY_TEXT_NO_CHANGE         G_MAXINT64

/* Placement of one anchor's block: its origin is align * (drawable - block) */
typedef struct
{
  double align_x;
  double align_y;
} overlay_text_anchor_row_t;

/* What one live instance presents at a given time */
typedef struct
{
  gboolean presents;         /* shows a line in this frame */
  gboolean persists;         /* stays in the store after this frame */
  int      fade_elapsed_ms;  /* 0 while holding and for PER_FRAME */
  gint64   next_change_ms;   /* delay to its next appearance change */
} overlay_text_phase_t;

static const overlay_text_anchor_row_t
overlay_text_anchors[OVERLAY_TEXT_ANCHOR_COUNT] =
{
  [OVERLAY_TEXT_CENTER]      = { .align_x = 0.5, .align_y = 0.5 },
  [OVERLAY_TEXT_BOTTOM_LEFT] = { .align_x = 0.0, .align_y = 1.0 },
};

/* Phase of an instance presenting nothing and leaving the store */
static const overlay_text_phase_t overlay_text_retired =
{
  .next_change_ms = OVERLAY_TEXT_NO_CHANGE
};

static gboolean overlay_text_tick(gpointer user_data);

/*-----------------------------------------------------------------------*/

  void
Draw_Centered_Message(cairo_t *cr, int width, int height, const char *msg)
{
  PangoLayout *layout;
  PangoFontDescription *desc;
  int text_w, text_h;

  layout = pango_cairo_create_layout(cr);
  pango_layout_set_text(layout, msg, -1);

  desc = pango_font_description_from_string("Sans 14");
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  pango_layout_get_pixel_size(layout, &text_w, &text_h);

  /* De-emphasized text centered in the drawable */
  cairo_set_source_rgb_f(cr, theme_active()->colors[THEME_ROLE_TEXT_MUTED]);
  cairo_move_to(cr,
      (double)(width  - text_w) / 2.0,
      (double)(height - text_h) / 2.0);
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout);
}

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
 * overlay_text_line_alpha() - Opacity a line paints at
 * @line: line carrying its instance and fade progress
 */
  static double
overlay_text_line_alpha(const overlay_text_line_t *line)
{
  const overlay_text_style_t *style = line->instance.style;
  double alpha = 0.0;

  switch( style->lifetime )
  {
    case OVERLAY_TEXT_PER_FRAME:
      alpha = 1.0;
      break;

    case OVERLAY_TEXT_TIMED:
      /* Fade linearly from opaque to clear across the fade duration */
      alpha = 1.0 - (double)line->fade_elapsed_ms / (double)style->fade_ms;
      break;

    case OVERLAY_TEXT_LIFETIME_COUNT:
      BUG("overlay text lifetime %d names no opacity rule\n",
          (int)style->lifetime);
      break;
  }

  return( alpha );

} /* overlay_text_line_alpha() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_box_measure() - Measure the padded box of one line
 * @layout: layout receiving the line text
 * @line:   line to measure
 *
 * Leaves @layout holding the line text, ready to paint.  Returns the box at
 * the origin for the caller to place.
 */
  static cairo_rectangle_t
overlay_text_box_measure(PangoLayout *layout, const overlay_text_line_t *line)
{
  int text_w, text_h;

  pango_layout_set_text(layout, line->instance.text, -1);
  pango_layout_get_pixel_size(layout, &text_w, &text_h);

  return( (cairo_rectangle_t){
      .width  = (double)text_w + 2.0 * OVERLAY_TEXT_PADDING,
      .height = (double)text_h + 2.0 * OVERLAY_TEXT_PADDING } );

} /* overlay_text_box_measure() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_paint_box() - Paint one line in its box at an opacity
 * @cr:     image context
 * @layout: layout holding the line text
 * @box:    padded box, placed on the image
 * @alpha:  opacity of box and text together
 *
 * Box and text composite as one group, so a fade dims both without the box
 * showing through the glyphs.
 */
  static void
overlay_text_paint_box(cairo_t *cr, PangoLayout *layout,
    const cairo_rectangle_t *box, double alpha)
{
  const theme_t *theme = theme_active();
  rgb_f_t background = theme->colors[THEME_ROLE_BACKGROUND];

  cairo_push_group(cr);

  cairo_set_source_rgba(cr, (double)background.r, (double)background.g,
      (double)background.b, OVERLAY_TEXT_BOX_ALPHA);
  cairo_rectangle(cr, box->x, box->y, box->width, box->height);
  cairo_fill(cr);

  cairo_set_source_rgb_f(cr, theme->colors[THEME_ROLE_TEXT_PRIMARY]);
  cairo_move_to(cr, box->x + OVERLAY_TEXT_PADDING,
      box->y + OVERLAY_TEXT_PADDING);
  pango_cairo_show_layout(cr, layout);

  cairo_pop_group_to_source(cr);
  cairo_paint_with_alpha(cr, alpha);

} /* overlay_text_paint_box() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_paint_anchor() - Paint the block of lines one anchor stacks
 * @cr:     image context
 * @layout: layout the lines measure and paint through
 * @key:    painted inputs: drawable size and lines in post order
 * @anchor: anchor whose lines stack into one block
 *
 * The block sits flush with the anchor edges.  Boxes stack without gaps in
 * post order, newest at the bottom, each aligned within the block as the
 * anchor aligns the block.
 */
  static void
overlay_text_paint_anchor(cairo_t *cr, PangoLayout *layout,
    const overlay_text_key_t *key, overlay_text_anchor_t anchor)
{
  const overlay_text_anchor_row_t *row = &overlay_text_anchors[anchor];
  int count = mem_array_count(key->lines);
  cairo_rectangle_t block = {0};
  cairo_rectangle_t box;
  double y;
  int i;

  /* Measure the block: the widest box by the summed box heights */
  for( i = 0; i < count; i++ )
  {
    if( key->lines[i].instance.style->anchor != anchor )
      continue;

    box = overlay_text_box_measure(layout, &key->lines[i]);
    block.width = MAX(block.width, box.width);
    block.height += box.height;
  }

  block.x = row->align_x * ((double)key->width - block.width);
  block.y = row->align_y * ((double)key->height - block.height);
  y = block.y;

  /* Paint the boxes top to bottom in post order */
  for( i = 0; i < count; i++ )
  {
    if( key->lines[i].instance.style->anchor != anchor )
      continue;

    box = overlay_text_box_measure(layout, &key->lines[i]);
    box.x = block.x + row->align_x * (block.width - box.width);
    box.y = y;
    overlay_text_paint_box(cr, layout, &box,
        overlay_text_line_alpha(&key->lines[i]));
    y += box.height;
  }

} /* overlay_text_paint_anchor() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_layout_new() - Create the layout overlay text paints through
 * @surface: surface whose widget supplies the font family
 *
 * Sets bold at an absolute pixel size, so every engine presents the same
 * text.  Returns NULL when the surface presents no widget.
 */
  static PangoLayout *
overlay_text_layout_new(const render_surface_t *surface)
{
  PangoLayout *layout = canvas_surface_pango_layout(surface, NULL);
  PangoFontDescription *font;

  if( layout == NULL )
  {
    BUG("overlay text surface presents no widget to take a font from\n");
    return( NULL );
  }

  font = pango_font_description_copy(
      pango_context_get_font_description(pango_layout_get_context(layout)));
  pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
  pango_font_description_set_absolute_size(font,
      (double)OVERLAY_TEXT_FONT_PX * (double)PANGO_SCALE);
  pango_layout_set_font_description(layout, font);
  pango_font_description_free(font);

  return( layout );

} /* overlay_text_layout_new() */

/*-----------------------------------------------------------------------*/

/**
 * overlay_text_paint() - Paint the image the painted key describes
 * @surface: surface whose store holds the painted key
 *
 * Returns a drawable-sized image, or NULL when no line shows or the drawable
 * holds no area.
 */
  static cairo_surface_t *
overlay_text_paint(const render_surface_t *surface)
{
  const overlay_text_key_t *key = &surface->overlay_text.key;
  cairo_surface_t *image;
  PangoLayout *layout;
  cairo_t *cr;
  overlay_text_anchor_t anchor;

  if( (mem_array_count(key->lines) == 0) || (key->width <= 0)
      || (key->height <= 0) )
    return( NULL );

  layout = overlay_text_layout_new(surface);
  if( layout == NULL )
    return( NULL );

  image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
      key->width, key->height);
  cr = cairo_create(image);

  for( anchor = 0; anchor < OVERLAY_TEXT_ANCHOR_COUNT; anchor++ )
    overlay_text_paint_anchor(cr, layout, key, anchor);

  cairo_destroy(cr);
  g_object_unref(layout);

  return( image );

} /* overlay_text_paint() */

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

  gradient_result_t
overlay_text_frame(render_surface_t *surface, int width, int height)
{
  gint64 now_us;

  if( surface == NULL )
  {
    BUG("overlay text frame names no surface\n");
    return( (gradient_result_t){ .surface = NULL } );
  }

  now_us = g_get_monotonic_time();
  overlay_text_resolve_lines(&surface->overlay_text, now_us);
  overlay_text_refresh(surface, width, height);
  overlay_text_retire(&surface->overlay_text, now_us);

  return( (gradient_result_t){ .surface = surface->overlay_text.image,
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
