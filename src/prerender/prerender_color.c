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
 * prerender_color: renderer-agnostic color mapping functions.
 *
 * No Cairo, no OpenGL, no GTK dependencies.
 */
#include "prerender_color.h"
#include "../chroma/chroma.h"
#include "../chroma/chroma_nearfield.h"
#include "../chroma/chroma_farfield.h"
#include "../shared.h"

rgb_f_t *seg_rgb   = NULL;
rgb_f_t *patch_rgb = NULL;

struct_colors_t *struct_colors = NULL;

/* Geometry display color per segment classification */
static const rgb_f_t seg_type_rgb[SEG_COLOR_COUNT] = {
  [SEG_COLOR_NORMAL]     = { 0.0f, 0.0f, 1.0f },
  [SEG_COLOR_LOADED]     = { 1.0f, 1.0f, 0.0f },
  [SEG_COLOR_EXCITATION] = { 1.0f, 0.0f, 0.0f },
};

/* Drawn when a classification falls outside the enum, so the fault shows
 * on the model instead of reaching the canvas as an unwritten color */
#define SEG_TYPE_UNKNOWN_RGB  ((rgb_f_t){ 1.0f, 0.0f, 1.0f })

/*-----------------------------------------------------------------------*/

/**
 * get_segment_color_type() - Classify a wire segment
 * @seg_num: 1-indexed segment number (matches vsorc.isant, zload.ldsegn)
 *
 * Priority: excitation > loaded > normal.
 */
  segment_color_type_t
get_segment_color_type(int seg_num)
{
  int idx;

  /* Excitation sources (highest priority) */
  for( idx = 0; idx < vsorc.nsant; idx++ )
  {
    if( vsorc.isant[idx] == seg_num )
      return SEG_COLOR_EXCITATION;
  }

  for( idx = 0; idx < vsorc.nvqd; idx++ )
  {
    if( vsorc.ivqd[idx] == seg_num )
      return SEG_COLOR_EXCITATION;
  }

  for( idx = 0; idx < zload.nldseg; idx++ )
  {
    if( zload.ldsegn[idx] == seg_num )
      return SEG_COLOR_LOADED;
  }

  return SEG_COLOR_NORMAL;
}

/*-----------------------------------------------------------------------*/

/**
 * segment_type_to_rgb() - Map classification to display color
 * @type: segment classification
 * @r:    output red [0..1]
 * @g:    output green [0..1]
 * @b:    output blue [0..1]
 */
  void
segment_type_to_rgb(segment_color_type_t type, float *r, float *g, float *b)
{
  rgb_f_t c = SEG_TYPE_UNKNOWN_RGB;

  if( (unsigned)type < SEG_COLOR_COUNT )
    c = seg_type_rgb[type];
  else
    BUG("unknown segment color classification %d\n", type);

  *r = c.r;
  *g = c.g;
  *b = c.b;
}

/*-----------------------------------------------------------------------*/

/**
 * free_struct_colors_step() - Release one fstep's color sub-buffers
 * @elem: pointer to one struct_colors_t element
 */
static void
free_struct_colors_step(void *elem)
{
  struct_colors_t *c = elem;
  mem_array_free(&c->patch_flow_data);
}

/*-----------------------------------------------------------------------*/

/**
 * alloc_struct_colors() - Allocate per-fstep struct_colors array
 * @nfrq: total frequency steps
 */
  void
alloc_struct_colors(int nfrq)
{
  int i;

  if( nfrq <= 0 )
    return;

  /* Resize the outer array, freeing only the shrink tail; surviving
   * entries keep their sub-buffers for reuse by the inner alloc loop. */
  mem_array_resize(&struct_colors, nfrq, free_struct_colors_step);

  for( i = 0; i < nfrq; i++ )
  {
    if( data.m > 0 )
      mem_array_alloc(&struct_colors[i].patch_flow_data, data.m);
  }

  chroma_proj_alloc();
}

/*-----------------------------------------------------------------------*/

/**
 * free_struct_colors() - Release struct_colors and geometry color arrays
 */
  void
free_struct_colors(void)
{
  int i;
  int nfrq;

  if( struct_colors != NULL )
  {
    nfrq = mem_array_count(struct_colors);
    for( i = 0; i < nfrq; i++ )
      free_struct_colors_step(&struct_colors[i]);
    mem_array_free(&struct_colors);
  }

  mem_array_free(&seg_rgb);
  mem_array_free(&patch_rgb);

  chroma_proj_free();
  chroma_nf_free();
  chroma_ff_free();
}

/*-----------------------------------------------------------------------*/

/**
 * init_geometry_colors() - Compute Tier 1 seg_rgb/patch_rgb at file load
 */
  void
init_geometry_colors(void)
{
  int i;
  float r, g, b;
  segment_color_type_t ctype;

  if( data.n > 0 )
  {
    mem_array_realloc(&seg_rgb, data.n);

    for( i = 0; i < data.n; i++ )
    {
      ctype = get_segment_color_type(i + 1);
      segment_type_to_rgb(ctype, &r, &g, &b);
      seg_rgb[i].r = r;
      seg_rgb[i].g = g;
      seg_rgb[i].b = b;
    }
  }

  if( data.m > 0 )
  {
    mem_array_realloc(&patch_rgb, data.m);

    /* Patches use SEG_COLOR_NORMAL blue constant */
    segment_type_to_rgb(SEG_COLOR_NORMAL, &r, &g, &b);
    for( i = 0; i < data.m; i++ )
    {
      patch_rgb[i].r = r;
      patch_rgb[i].g = g;
      patch_rgb[i].b = b;
    }
  }
}

/*-----------------------------------------------------------------------*/

/**
 * struct_colors_fill_fstep() - Compute Tier 2 wire/patch colors for one fstep
 * @fstep: frequency step index
 *
 * Reads crnt_fstep[fstep] current/charge amplitudes, scans the magnitude
 * ranges, and bakes the patch tangent-flow phasor projections.
 */
  void
struct_colors_fill_fstep(int fstep)
{
  int i;
  double cabs_val, cmax_wire_crnt, cmax_wire_chrg, cmax_patch;
  double cmin_wire_crnt, cmin_wire_chrg, cmin_patch;
  complex double c;

  if( struct_colors == NULL
      || fstep < 0
      || fstep > calc_data.steps_total
      || crnt_fstep == NULL
      || crnt_fstep[fstep].cur == NULL )
    return;

  cmax_wire_crnt = 0.0;
  cmax_wire_chrg = 0.0;
  cmax_patch     = 0.0;
  cmin_wire_crnt = 1e30;
  cmin_wire_chrg = 1e30;
  cmin_patch     = 1e30;

  /* Scan wire current magnitudes (cur array: data.np3m complex doubles;
   * wire segments are the first data.n entries at index i) */
  for( i = 0; i < data.n; i++ )
  {
    c = crnt_fstep[fstep].cur[i];
    cabs_val = cabs(c);
    if( cabs_val > cmax_wire_crnt ) cmax_wire_crnt = cabs_val;
    if( cabs_val < cmin_wire_crnt ) cmin_wire_crnt = cabs_val;
  }

  /* Scan wire charge magnitudes (bir/bii arrays) */
  for( i = 0; i < data.n; i++ )
  {
    cabs_val = hypot(crnt_fstep[fstep].bir[i], crnt_fstep[fstep].bii[i]);
    if( cabs_val > cmax_wire_chrg ) cmax_wire_chrg = cabs_val;
    if( cabs_val < cmin_wire_chrg ) cmin_wire_chrg = cabs_val;
  }

  /* Scan patch current magnitudes (cur array, patches follow wires at
   * index ci = data.n + 3*i) */
  for( i = 0; i < data.m; i++ )
  {
    int ci = data.n + 3 * i;
    double mag = cabs(crnt_fstep[fstep].cur[ci]);
    if( mag > cmax_patch ) cmax_patch = mag;
    if( mag < cmin_patch ) cmin_patch = mag;
  }

  /* Store range scalars */
  struct_colors[fstep].wire_crnt_cmin  = (float)cmin_wire_crnt;
  struct_colors[fstep].wire_crnt_cmax  = (float)cmax_wire_crnt;
  struct_colors[fstep].wire_chrg_cmin  = (float)cmin_wire_chrg;
  struct_colors[fstep].wire_chrg_cmax  = (float)cmax_wire_chrg;
  struct_colors[fstep].patch_crnt_cmin = (float)cmin_patch;
  struct_colors[fstep].patch_crnt_cmax = (float)cmax_patch;

  /* Precompute patch tangent-axis phasor projections for arrow rendering */
  if( struct_colors[fstep].patch_flow_data != NULL )
  {
    if( cmax_patch > 0.0 )
    {
      double scale = 1.0 / cmax_patch;
      for( i = 0; i < data.m; i++ )
      {
        int ci = data.n + 3 * i;
        complex double cur_x = crnt_fstep[fstep].cur[ci];
        complex double cur_y = crnt_fstep[fstep].cur[ci + 1];
        complex double cur_z = crnt_fstep[fstep].cur[ci + 2];
        complex double ct1 = cur_x * data.patches[i].t1x
                           + cur_y * data.patches[i].t1y
                           + cur_z * data.patches[i].t1z;
        complex double ct2 = cur_x * data.patches[i].t2x
                           + cur_y * data.patches[i].t2y
                           + cur_z * data.patches[i].t2z;
        struct_colors[fstep].patch_flow_data[i][0] = (float)(creal(ct1) * scale);
        struct_colors[fstep].patch_flow_data[i][1] = (float)(cimag(ct1) * scale);
        struct_colors[fstep].patch_flow_data[i][2] = (float)(creal(ct2) * scale);
        struct_colors[fstep].patch_flow_data[i][3] = (float)(cimag(ct2) * scale);
      }
    }
    else
    {
      mem_array_zero(struct_colors[fstep].patch_flow_data);
    }
  }
}

/*-----------------------------------------------------------------------*/
