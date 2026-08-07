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
 * prerender_farfield: Tier 3 far-field presentation pass (parent).
 *
 * ff_presentation_recompute() reads current rc_config/calc_data and
 * noise_temp[] table; writes vertex positions, edge colors, pattern_radius,
 * and r_min.  Called on frequency step change and UI parameter mutation.
 * Edge topology (va/vb) is Tier 1 in geom_pre.theta_topo / phi_topo,
 * computed once by compute_ff_topology() at RP-card time.
 */
#include "prerender_farfield.h"
#include "prerender_color.h"
#include "../color/color_palette.h"
#include "../shared.h"
#include "../measurements.h"

/* Structure overlay extent as multiple of radiation pattern r_max */
#define FF_OVERLAY_DEFAULT_EXTENT 1.25f

/*-----------------------------------------------------------------------*/

/**
 * color_edges() - Assign themed ramp colors to an edge color array from vertex radii
 * @topo:    topology array with va/vb vertex indices
 * @colors:  output color array, parallel to topo; must be pre-allocated
 * @count:   number of edges
 * @verts:   vertex array indexed by topo[i].va / topo[i].vb
 * @r_min:   minimum radius (maps to first color)
 * @r_range: radius range (r_max - r_min); must be > 0
 */
static void
color_edges(ff_edge_topo_t *topo, rgb_f_t *colors, int count,
    point_3d_t *verts, double r_min, double r_range)
{
  const palette_t *ramp = palette_get(PALETTE_RAMP);
  int i;
  for( i = 0; i < count; i++ )
  {
    double avg_r = (verts[topo[i].va].r + verts[topo[i].vb].r) / 2.0;
    colors[i] = palette_lookup_scaled(ramp, (avg_r - r_min) / r_range, 1.0);
  }
}

/*-----------------------------------------------------------------------*/

  void
ff_presentation_recompute(int fstep)
{
  int nth, nph, pts_idx, pol;
  double r, r_min, r_max;
  ff_pre_t *fp;

  if( ff_pre == NULL || fstep < 0 || !save.fstep[fstep] )
    return;
  int total = fpat.nth * fpat.nph;

  fp = &ff_pre[fstep];

  if( rad_pattern == NULL || total <= 0 )
    return;

  /* Input-snapshot cache gate: skip recomputation when inputs unchanged */
  presentation_cache_key_t cur_key = presentation_cache_key_build(fstep);
  if( fp->vertices &&
      presentation_cache_key_match(&fp->cache_key, &cur_key) )
    return;

  pol = cur_key.pol_type;
  mem_array_realloc(&fp->vertices, total);

  if( geom_pre.n_theta_edges > 0 )
  {
    mem_array_realloc(&fp->theta_rgb, geom_pre.n_theta_edges);
  }

  if( geom_pre.n_phi_edges > 0 )
  {
    mem_array_realloc(&fp->phi_rgb, geom_pre.n_phi_edges);
  }
  mem_array_realloc(&fp->vertex_rgb, total);

  /* Resolve noise temperatures from table */
  double t_sky = 0.0, t_earth = 0.0;
  int noise_mode = IS_NOISE_MODE(rc_config.gain_style);
  if( noise_mode )
    noise_temp_resolve(fstep, &t_sky, &t_earth);

  /* Compute r_max and r_min from gain extremes */
  int idx = rad_pattern[fstep].max_gain_idx[pol];
  double g = rad_pattern[fstep].gtot[idx] +
             Polarization_Factor(pol, fstep, idx);
  r_max = Scale_Gain_Resolved(g, fstep, idx, t_sky, t_earth);
  if( r_max < FF_PATTERN_MIN_RADIUS )
    r_max = 1.0;

  idx = rad_pattern[fstep].min_gain_idx[pol];
  double actual_gain = rad_pattern[fstep].gtot[idx] +
                       Polarization_Factor(pol, fstep, idx);
  double color_gain = (actual_gain < COLOR_MIN_GAIN) ? COLOR_MIN_GAIN : actual_gain;
  r_min = Scale_Gain_Resolved(color_gain, fstep, idx, t_sky, t_earth);

  /* Noise-mode rotation parameters */
  double rot_tht_mg = 0.0, rot_phi_mg = 0.0, rot_elev = 0.0;
  if( noise_mode )
  {
    rot_tht_mg = rad_pattern[fstep].max_gain_tht[pol] * (double)TORAD;
    rot_phi_mg = rad_pattern[fstep].max_gain_phi[pol] * (double)TORAD;
    rot_elev   = rc_config.ant_temp_elevation * (double)TORAD;
  }

  pts_idx = 0;

  for( nph = 0; nph < fpat.nph; nph++ )
  {
    for( nth = 0; nth < fpat.nth; nth++ )
    {
      double gain_v = rad_pattern[fstep].gtot[pts_idx] +
                      Polarization_Factor(pol, fstep, pts_idx);
      r = Scale_Gain_Resolved(gain_v, fstep, pts_idx, t_sky, t_earth);

      fp->vertices[pts_idx].r = r;

      if( noise_mode && rot_elev != 0.0 )
      {
        /* Reconstruct angles from integer step to avoid accumulation drift */
        double theta_r = (fpat.thets + nth * fpat.dth) * (double)TORAD;
        double phi_r   = (fpat.phis  + nph * fpat.dph) * (double)TORAD;
        double xr, yr, zr;
        ant_temp_rotate_point(theta_r, phi_r,
            rot_tht_mg, rot_phi_mg, rot_elev,
            &xr, &yr, &zr);
        fp->vertices[pts_idx].x = r * xr;
        fp->vertices[pts_idx].y = r * yr;
        fp->vertices[pts_idx].z = r * zr;
      }
      else
      {
        double r_xy = r * geom_pre.sin_theta[nth];
        fp->vertices[pts_idx].z = r * geom_pre.cos_theta[nth];
        fp->vertices[pts_idx].x = r_xy * geom_pre.cos_phi[nph];
        fp->vertices[pts_idx].y = r_xy * geom_pre.sin_phi[nph];
      }

      pts_idx++;
    }
  }

  /* Noise-mode: rescan for true scaled min/max */
  if( noise_mode )
  {
    double nmax = fp->vertices[0].r;
    double nmin = fp->vertices[0].r;
    int i;
    for( i = 1; i < total; i++ )
    {
      if( fp->vertices[i].r > nmax ) nmax = fp->vertices[i].r;
      if( fp->vertices[i].r < nmin ) nmin = fp->vertices[i].r;
    }
    rad_pattern[fstep].noise_scaled_max = nmax;
    rad_pattern[fstep].noise_scaled_min = nmin;
    r_max = nmax;
    r_min = nmin;
  }

  fp->pattern_radius = (float)r_max;
  fp->r_min = (float)r_min;

  /* Base scale for structure overlay (without interactive scale_adj) */
  if( geom_pre.scene_radius > 0.001 )
    fp->overlay_base_scale = FF_OVERLAY_DEFAULT_EXTENT
        * (float)r_max / (float)geom_pre.scene_radius;
  else
    fp->overlay_base_scale = 1.0f;

  /* Per-vertex colors for triangle tessellation */
  double r_range = r_max - r_min;
  if( r_range <= 0.0 )
    r_range = 1.0;
  const palette_t *ramp = palette_get(PALETTE_RAMP);
  for( int i = 0; i < total; i++ )
    fp->vertex_rgb[i] = palette_lookup_scaled(ramp,
        (fp->vertices[i].r - r_min) / r_range, 1.0);

  /* Edge colors from averaged vertex radii */
  if( fp->theta_rgb != NULL )
    color_edges(geom_pre.theta_topo, fp->theta_rgb, geom_pre.n_theta_edges,
        fp->vertices, r_min, r_range);
  if( fp->phi_rgb != NULL )
    color_edges(geom_pre.phi_topo, fp->phi_rgb, geom_pre.n_phi_edges,
        fp->vertices, r_min, r_range);

  fp->generation++;

  /* Store input snapshot for cache gate */
  fp->cache_key = cur_key;
}

/*-----------------------------------------------------------------------*/
