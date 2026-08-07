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
#include "../shared.h"
#include "../themes/theme.h"

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
