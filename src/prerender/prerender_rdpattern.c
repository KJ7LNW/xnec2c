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
 * prerender_rdpattern: Tier 1 radiation-pattern prerender derivations.
 *
 * Computes excitation_center (centroid of excitation source segments),
 * nf_dr_norm (near-field normalization distance), and radiation
 * pattern trig tables and far-field edge topology.
 * Called once after geometry and excitation are established.
 */
#include "prerender_rdpattern.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/**
 * compute_excitation_center() - Centroid of all excitation source segments
 */
  void
compute_excitation_center(void)
{
  int idx, seg_idx, count;
  double cx, cy, cz;

  cx = cy = cz = 0.0;
  count = 0;

  /* Voltage sources (1-indexed segment numbers) */
  for( idx = 0; idx < vsorc.nsant; idx++ )
  {
    seg_idx = vsorc.isant[idx] - 1;
    if( seg_idx >= 0 && seg_idx < data.n )
    {
      cx += data.segments[seg_idx].x;
      cy += data.segments[seg_idx].y;
      cz += data.segments[seg_idx].z;
      count++;
    }
  }

  /* Current sources */
  for( idx = 0; idx < vsorc.nvqd; idx++ )
  {
    seg_idx = vsorc.ivqd[idx] - 1;
    if( seg_idx >= 0 && seg_idx < data.n )
    {
      cx += data.segments[seg_idx].x;
      cy += data.segments[seg_idx].y;
      cz += data.segments[seg_idx].z;
      count++;
    }
  }

  if( count > 0 )
  {
    geom_pre.excitation_cx = cx / count;
    geom_pre.excitation_cy = cy / count;
    geom_pre.excitation_cz = cz / count;
  }
  else
  {
    geom_pre.excitation_cx = 0.0;
    geom_pre.excitation_cy = 0.0;
    geom_pre.excitation_cz = 0.0;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * compute_nf_dr_norm() - Near-field normalization distance
 *
 * Matches the dr computation from Draw_Near_Field():
 * spherical → dxnr, rectangular → euclidean(dxnr, dynr, dznr)/1.75.
 */
  void
compute_nf_dr_norm(void)
{
  if( fpat.near )
  {
    /* Spherical coordinates */
    geom_pre.nf_dr_norm = (double)fpat.dxnr;
  }
  else
  {
    /* Rectangular coordinates */
    geom_pre.nf_dr_norm = sqrt(
        (double)fpat.dxnr * (double)fpat.dxnr +
        (double)fpat.dynr * (double)fpat.dynr +
        (double)fpat.dznr * (double)fpat.dznr) / 1.75;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * compute_trig_tables() - Precompute sin/cos/solid_angle for radiation grid
 *
 * Allocates and fills geom_pre trig tables from fpat grid parameters.
 * Called once at geometry load; avoids repeated trig per vertex per frame.
 */
  void
compute_trig_tables(void)
{
  int i;
  double dth_rad = (double)fpat.dth * (double)TORAD;
  double dph_rad = (double)fpat.dph * (double)TORAD;

  if( fpat.nth <= 0 || fpat.nph <= 0 )
  {
    mem_array_free(&geom_pre.sin_theta);
    mem_array_free(&geom_pre.cos_theta);
    mem_array_free(&geom_pre.sin_phi);
    mem_array_free(&geom_pre.cos_phi);
    mem_array_free(&geom_pre.solid_angle);
    return;
  }

  mem_array_alloc(&geom_pre.sin_theta, fpat.nth);
  mem_array_alloc(&geom_pre.cos_theta, fpat.nth);
  mem_array_alloc(&geom_pre.solid_angle, fpat.nth);
  mem_array_alloc(&geom_pre.sin_phi, fpat.nph);
  mem_array_alloc(&geom_pre.cos_phi, fpat.nph);

  for( i = 0; i < fpat.nth; i++ )
  {
    double tht = fpat_cell_theta(i) * (double)TORAD;
    geom_pre.sin_theta[i] = sin(tht);
    geom_pre.cos_theta[i] = cos(tht);
    geom_pre.solid_angle[i] = fabs(sin(tht)) * dth_rad * dph_rad;
  }

  for( i = 0; i < fpat.nph; i++ )
  {
    double phi = fpat_cell_phi(i) * (double)TORAD;
    geom_pre.sin_phi[i] = sin(phi);
    geom_pre.cos_phi[i] = cos(phi);
  }
}

/*-----------------------------------------------------------------------*/

/**
 * compute_ff_topology() - Populate geom_pre far-field edge topology tables
 *
 * Writes theta_topo[], phi_topo[], n_theta_edges, n_phi_edges into geom_pre.
 * Topology is frequency-independent (grid connectivity depends only on fpat
 * dimensions).  Called once at RP-card time, parent-only.
 */
  void
compute_ff_topology(void)
{
  int nth, nph, col_idx, pts_idx;

  if( fpat.nth < 2 || fpat.nph < 2 )
  {
    mem_array_free(&geom_pre.theta_topo);
    mem_array_free(&geom_pre.phi_topo);
    geom_pre.n_theta_edges = 0;
    geom_pre.n_phi_edges   = 0;
    return;
  }

  geom_pre.n_theta_edges = (fpat.nth - 1) * fpat.nph;
  mem_array_alloc(&geom_pre.theta_topo, geom_pre.n_theta_edges);

  col_idx = 0;
  pts_idx = 0;
  for( nph = 0; nph < fpat.nph; nph++ )
  {
    for( nth = 1; nth < fpat.nth; nth++ )
    {
      geom_pre.theta_topo[col_idx].va = (uint32_t)pts_idx;
      geom_pre.theta_topo[col_idx].vb = (uint32_t)(pts_idx + 1);
      col_idx++;
      pts_idx++;
    }
    pts_idx++;
  }

  geom_pre.n_phi_edges = fpat.nth * (fpat.nph - 1);
  mem_array_alloc(&geom_pre.phi_topo, geom_pre.n_phi_edges);

  col_idx = 0;
  for( nth = 0; nth < fpat.nth; nth++ )
  {
    pts_idx = nth;
    for( nph = 1; nph < fpat.nph; nph++ )
    {
      geom_pre.phi_topo[col_idx].va = (uint32_t)pts_idx;
      geom_pre.phi_topo[col_idx].vb = (uint32_t)(pts_idx + fpat.nth);
      col_idx++;
      pts_idx += fpat.nth;
    }
  }
}

/*-----------------------------------------------------------------------*/
