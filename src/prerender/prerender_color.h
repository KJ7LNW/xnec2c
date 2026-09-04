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

#ifndef PRERENDER_COLOR_H
#define PRERENDER_COLOR_H       1

#include "../common.h"

/*-----------------------------------------------------------------------
 * Segment color classification
 *
 * Geometry-color initialization classifies each wire as excitation source,
 * loaded, or normal.
 *----------------------------------------------------------------------*/
typedef enum
{
  SEG_COLOR_NORMAL = 0,
  SEG_COLOR_LOADED,
  SEG_COLOR_EXCITATION,
  SEG_COLOR_COUNT
} segment_color_type_t;

/**
 * segment_type_to_rgb() - Map segment classification to display color
 * @type: segment color classification
 * @r:    output red [0..1]
 * @g:    output green [0..1]
 * @b:    output blue [0..1]
 */
void segment_type_to_rgb(segment_color_type_t type,
    float *r, float *g, float *b);


/**
 * get_segment_color_type() - Classify a wire segment by excitation or load
 * @seg_num: 1-indexed segment number (matches vsorc.isant, zload.ldsegn)
 *
 * Returns the classification for coloring in geometry display mode.
 */
segment_color_type_t get_segment_color_type(int seg_num);

/* rgb_f_t defined in common.h */

/*-----------------------------------------------------------------------
 * Per-fstep magnitude ranges and patch flow projections (Tier 2)
 *
 * Filled by struct_colors_fill_fstep() per frequency step; the color
 * projection layer normalizes its sources against the range scalars.
 *----------------------------------------------------------------------*/
typedef struct
{
  /* Precomputed tangent-axis phasor projections normalized by cmax.
   * [i][0..3] = {Re(ct1), Im(ct1), Re(ct2), Im(ct2)} / cmax_patch.
   * Zeroed when no current data is available for this fstep. */
  float (*patch_flow_data)[4]; /* [data.m][4] */

  float    wire_crnt_cmin;
  float    wire_crnt_cmax;
  float    wire_chrg_cmin;
  float    wire_chrg_cmax;
  float    patch_crnt_cmin;
  float    patch_crnt_cmax;

  /* Publication token of the stored content; identifies one completed fill
   * so a replacement at an unchanged step and frequency differs */
  uint32_t generation;
} struct_colors_t;

/*-----------------------------------------------------------------------
 * Tier 1 geometry color arrays (file-load time, frequency-independent)
 *----------------------------------------------------------------------*/
extern rgb_f_t *seg_rgb;    /* [data.n] blue/yellow/red per segment type */
extern rgb_f_t *patch_rgb;  /* [data.m] blue constant */

/*-----------------------------------------------------------------------
 * Tier 2 per-fstep color arrays
 *----------------------------------------------------------------------*/
extern struct_colors_t *struct_colors;

/**
 * get_precomputed_flow_data() - Copy precomputed patch flow phasors into fd[4]
 * @fstep: frequency step index
 * @idx:   patch index (0-based into data.m)
 * @fd:    output {Re(ct1), Im(ct1), Re(ct2), Im(ct2)} / cmax_patch
 */
static inline void
get_precomputed_flow_data(int fstep, int idx, float fd[4])
{
  const float *pfd = struct_colors[fstep].patch_flow_data[idx];
  fd[0] = pfd[0];
  fd[1] = pfd[1];
  fd[2] = pfd[2];
  fd[3] = pfd[3];
}

/**
 * alloc_struct_colors() - Allocate per-fstep struct_colors array
 * @nfrq: total frequency steps
 */
void alloc_struct_colors(int nfrq);

/**
 * free_struct_colors() - Release struct_colors and geometry color arrays
 */
void free_struct_colors(void);

/**
 * init_geometry_colors() - Compute Tier 1 seg_rgb/patch_rgb at file load
 *
 * Iterates data.n segments via get_segment_color_type() + segment_type_to_rgb().
 * Fills patch_rgb[] with SEG_COLOR_NORMAL blue.
 * Called from Init_Struct_Drawing() after structure_segs is allocated.
 */
void init_geometry_colors(void);

/**
 * struct_colors_fill_fstep() - Compute Tier 2 colors for one fstep
 * @fstep: frequency step index
 *
 * Reads crnt_fstep[fstep] current/charge amplitudes, scans the magnitude
 * ranges, and bakes the patch tangent-flow phasor projections.
 */
void struct_colors_fill_fstep(int fstep);

#endif
