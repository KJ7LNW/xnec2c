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

#ifndef RENDER_MESSAGE_H
#define RENDER_MESSAGE_H        1

#include "../common.h"
#include "render_engine.h"
#include "gradient_cache.h"

/* Status messages emitted when rendering preconditions are not met.
 * Single source of truth: both Cairo and OpenGL use these constants. */
#define STATUS_MSG_OPEN_FILE        "File ▸ Open to load an NEC file"
#define STATUS_MSG_NO_RP_CARD       "Gain pattern requires an RP card in the NEC file"
#define STATUS_MSG_NO_NEAREH_CARDS  "Near field requires NE or NH cards in the NEC file"
#define STATUS_MSG_NO_RP_NO_NEAREH  "No RP or NE/NH cards in the NEC file"
#define STATUS_MSG_SELECT_NEARFIELD "Select Near Field (no RP card for gain pattern)"
#define STATUS_MSG_SELECT_GAINPAT   "Select Gain Pattern (no NE/NH cards for near field)"
#define STATUS_MSG_SELECT_MODE      "Select Gain Pattern or Near Field"
#define STATUS_MSG_SELECT_NF_FIELD  "Enable E field, H field, or Poynting vector"
#define STATUS_MSG_START_FREQLOOP   "Press ▶ to start the frequency loop"
#define STATUS_MSG_NOT_READY        "Radiation pattern data not ready"
#define STATUS_MSG_NO_RDPAT_DATA    "Waiting for frequency data"

/* Where a block of overlay text lines sits on the drawable */
typedef enum
{
  OVERLAY_TEXT_CENTER = 0,
  OVERLAY_TEXT_BOTTOM_LEFT,
  OVERLAY_TEXT_ANCHOR_COUNT
} overlay_text_anchor_t;

/* What retires an overlay text instance */
typedef enum
{
  OVERLAY_TEXT_PER_FRAME = 0,  /* dropped once the posting frame hands off */
  OVERLAY_TEXT_TIMED,          /* holds, then fades out by the clock */
  OVERLAY_TEXT_LIFETIME_COUNT
} overlay_text_lifetime_t;

/* Placement and retirement shared by every instance one producer posts */
typedef struct
{
  overlay_text_anchor_t   anchor;
  overlay_text_lifetime_t lifetime;
  int                     hold_ms;   /* TIMED rows set these two */
  int                     fade_ms;
} overlay_text_style_t;

/* One posted text and the style row of its producer */
typedef struct
{
  const char                 *text;   /* static lifetime */
  const overlay_text_style_t *style;  /* static lifetime */
} overlay_text_t;

/* One instance a store holds, stamped with its posting time */
typedef struct
{
  overlay_text_t instance;
  gint64         posted_us;           /* g_get_monotonic_time() at post */
} overlay_text_live_t;

/* One line an image presents: the instance and how far it has faded */
typedef struct
{
  overlay_text_t instance;
  int            fade_elapsed_ms;     /* 0 while holding and for PER_FRAME */
} overlay_text_line_t;

/* Inputs an overlay text image is painted from */
typedef struct
{
  int                  width;
  int                  height;
  uint32_t             palette_gen;   /* color_palette_generation() */
  overlay_text_line_t *lines;         /* mem_array, paint order */
} overlay_text_key_t;

/* Overlay text one surface presents: its live instances, the frame request
 * pacing their fades, and the image painted from them */
typedef struct
{
  overlay_text_live_t *live;          /* mem_array, post order */
  overlay_text_line_t *scratch;       /* mem_array, resolved each hand-off */
  overlay_text_key_t   key;           /* inputs image was painted from */
  cairo_surface_t     *image;         /* NULL while no line shows */
  uint64_t             version;       /* advances on each repaint */
  guint                timeout_id;    /* 0 while no frame request is armed */
} overlay_text_store_t;

/**
 * overlay_text_post() - Add an overlay text instance to a surface
 * @surface:  surface presenting the instance
 * @instance: text and style row, both of static lifetime
 *
 * The instance shows from the frame posting it until its lifetime retires it.
 * Posting re-arms the frame request pacing timed instances; it queues no
 * redraw, since the posting frame hands the image off itself.
 */
void overlay_text_post(render_surface_t *surface, const overlay_text_t *instance);

/**
 * overlay_text_frame() - Resolve the overlay text image a frame presents
 * @surface: surface whose store resolves
 * @width:   drawable width in pixels
 * @height:  drawable height in pixels
 *
 * Repaints the image only when its lines, size or palette change, then drops
 * every instance the frame retires.  The image stays valid until the next
 * call on @surface.
 *
 * Returns the image and its version; the image is NULL while no line shows.
 */
render_image_t overlay_text_frame(render_surface_t *surface,
    int width, int height);

/**
 * overlay_text_release() - Release the overlay text a surface holds
 * @surface: surface whose store is released
 *
 * Removes the armed frame request, which carries @surface, so the caller
 * releases the store before freeing the surface.
 */
void overlay_text_release(render_surface_t *surface);

#endif
