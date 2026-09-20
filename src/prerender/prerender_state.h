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

#ifndef PRERENDER_STATE_H
#define PRERENDER_STATE_H       1

#include "../common.h"
#include "prerender_color.h"
#include "presentation_cache_key.h"

/*-----------------------------------------------------------------------
 * Shared geometry types
 *----------------------------------------------------------------------*/

/* 3D point with radial distance, used for radiation pattern vertices */
typedef struct
{
  double x, y, z, r;
} point_3d_t;

/*-----------------------------------------------------------------------
 * Tier 1 — Grid topology (frequency-independent)
 *----------------------------------------------------------------------*/

/* Far-field edge topology entry.  Indices into the per-fstep vertex array.
 * Color is stored separately in ff_pre_t.theta_rgb / phi_rgb (Tier 3). */
typedef struct
{
  uint32_t va, vb;
} ff_edge_topo_t;

/*-----------------------------------------------------------------------
 * Tier 1 — Geometry-derived (frequency-independent)
 *----------------------------------------------------------------------*/

/* Patch center and s-scaled tangent axes (s = sqrt(patch_area)/2).
 * Computed once in New_Patch_Data(); consumed by both GL and Cairo renderers.
 *
 * Duplicates data.patches[i].{px,py,pz} and t1/t2 scaled by s; grouped here
 * so all per-frame arrow rendering inputs are co-located in one struct.
 * May also reduce cache pressure in hot paths that would otherwise span
 * data.patches and a separate s computation. */
typedef struct
{
  double cx,  cy,  cz;    /* patch center */
  double st1x, st1y, st1z; /* s * t1 tangent vector */
  double st2x, st2y, st2z; /* s * t2 tangent vector */
} patch_tangent_frame_t;

/* Geometry-derived aggregate state, computed once at file load. */
typedef struct
{
  /* Version of the deck this state describes, advanced by Prerender_Aggregate() */
  uint32_t generation;

  double  scene_radius;
  double  excitation_cx;
  double  excitation_cy;
  double  excitation_cz;
  double  nf_dr_norm;

  /* Radiation pattern grid trig tables [fpat.nth] / [fpat.nph] */
  double *sin_theta;    /* [fpat.nth] */
  double *cos_theta;    /* [fpat.nth] */
  double *sin_phi;      /* [fpat.nph] */
  double *cos_phi;      /* [fpat.nph] */
  double *solid_angle;  /* [fpat.nth] = fabs(sin(theta_i)) * dth_rad * dph_rad */

  /* Far-field edge topology (frequency-independent grid connectivity) */
  ff_edge_topo_t *theta_topo;   /* [(fpat.nth-1) * fpat.nph] */
  ff_edge_topo_t *phi_topo;     /* [fpat.nth * (fpat.nph-1)] */
  int             n_theta_edges;
  int             n_phi_edges;

  /* Precomputed patch rectangle corners [data.m] */
  patch_corners_t *patch_corners;

  /* Precomputed patch tangent frame [data.m]: center and s-scaled axes */
  patch_tangent_frame_t *patch_tangent_frame;
} geom_pre_t;

/*-----------------------------------------------------------------------
 * Tier 3 — Per-frequency prerender
 *----------------------------------------------------------------------*/

/* Noise-mode display rotation the presentation applied to the pattern cells:
 * a Rodrigues rotation about the horizontal axis normal to the max-gain
 * azimuth.  A zero angle leaves the pattern in the untilted basis. */
typedef struct
{
  double axis_phi;  /* NEC phi of the max-gain direction (radians) */
  double angle;     /* Boresight elevation the pattern is tilted to (radians) */
} ff_rotation_t;

/** ff_rotation_tilted() - Whether the display rotation moves the pattern
 * @rot: rotation the presentation recorded
 *
 * The presentation places its vertices and the far-zone resolver places its
 * tangents in one frame, so both read the tilt from this one answer.
 */
static inline gboolean ff_rotation_tilted(const ff_rotation_t *rot)
{
  return rot->angle != 0.0;
}

/* Per-frequency far-field prerender.
 * Grid dimensions: fpat.nth / fpat.nph (Tier 0, not stored).
 * Topology (va/vb) is Tier 1 in geom_pre.theta_topo / phi_topo.
 * Colors are parallel arrays indexed identically to those topo arrays. */
typedef struct
{
  float       pattern_radius;
  float       r_min;
  float       overlay_base_scale; /* Structure-to-pattern scale without scale_adj */
  uint32_t    generation;     /* Incremented by ff_presentation_recompute() */
  ff_rotation_t rotation;     /* Applied to vertices by ff_presentation_recompute() */
  point_3d_t *vertices;       /* [fpat.nth * fpat.nph] */
  rgb_f_t    *theta_rgb;      /* [geom_pre.n_theta_edges] */
  rgb_f_t    *phi_rgb;        /* [geom_pre.n_phi_edges] */
  rgb_f_t    *vertex_rgb;     /* [fpat.nth * fpat.nph] per-vertex colors */

  /* Input snapshot for cache gate (see presentation_cache_key_t) */
  presentation_cache_key_t cache_key;
} ff_pre_t;

/* Field vector displacement from its origin, shared by the near-field sample
 * grid and the far-zone pattern grid.  The origins travel beside it in the
 * vector set its resolver publishes.
 * Geometry only: a forked child holds no palette, so color is resolved
 * in the parent at draw beside this displacement, never stored here. */
typedef struct
{
  float dx, dy, dz;
} field_vector_t;

/* Near-field channel discriminant, one field set per member */
typedef enum { NF_CHAN_E = 0, NF_CHAN_H, NF_CHAN_POV, NF_CHAN_NUM } nf_channel_t;

/*-----------------------------------------------------------------------
 * Global prerender state
 *----------------------------------------------------------------------*/

extern geom_pre_t    geom_pre;
extern ff_pre_t     *ff_pre;

/*-----------------------------------------------------------------------
 * Lifecycle
 *----------------------------------------------------------------------*/

/**
 * prerender_state_alloc() - Allocate per-fstep prerender arrays
 * @steps_total: number of frequency steps
 */
void prerender_state_alloc(int steps_total);

/**
 * prerender_state_free() - Release all prerender storage
 */
void prerender_state_free(void);

#endif
