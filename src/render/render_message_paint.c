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

#include "render_message_paint.h"
#include "render_message.h"
#include "render_surface.h"
#include "render_canvas_surface.h"
#include "../shared.h"
#include "../themes/theme.h"

/* Text size in pixels, absolute so every engine presents the same text */
#define OVERLAY_TEXT_FONT_PX           16

/* Margin between the text and the edges of its box, in pixels */
#define OVERLAY_TEXT_PADDING           12.0

/* Opacity of the box behind the text */
#define OVERLAY_TEXT_BOX_ALPHA         0.7

/* Placement of one anchor's block: its origin is align * (drawable - block) */
typedef struct
{
  double align_x;
  double align_y;
} overlay_text_anchor_row_t;

static const overlay_text_anchor_row_t
overlay_text_anchors[OVERLAY_TEXT_ANCHOR_COUNT] =
{
  [OVERLAY_TEXT_CENTER]      = { .align_x = 0.5, .align_y = 0.5 },
  [OVERLAY_TEXT_BOTTOM_LEFT] = { .align_x = 0.0, .align_y = 1.0 },
};

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

  pango_layout_set_text(layout, _(line->instance.text), -1);
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

  cairo_surface_t *
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
      key->width * key->scale, key->height * key->scale);

  /* Map logical units onto scale times as many pixels, keeping text sharp */
  cairo_surface_set_device_scale(image, key->scale, key->scale);
  cr = cairo_create(image);

  for( anchor = 0; anchor < OVERLAY_TEXT_ANCHOR_COUNT; anchor++ )
    overlay_text_paint_anchor(cr, layout, key, anchor);

  cairo_destroy(cr);
  g_object_unref(layout);

  return( image );

} /* overlay_text_paint() */

/*-----------------------------------------------------------------------*/
