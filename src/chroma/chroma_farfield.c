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
 * chroma_farfield: parent draw-time far-zone field color and geometry resolver.
 *
 * The far zone joins wire, patch, and near field on one color lifecycle: the
 * immutable phasor pair in rad_pattern[fstep].phasor is the source, this
 * resolver derives the instantaneous field vector, its scaled displacement,
 * and its palette color at draw, and the render backends consume the resolved
 * arrays.  The far-zone field is transverse, so the displacement lies tangent
 * to the pattern sphere and the gain surface it attaches to is untouched.
 */
#include "chroma_farfield.h"
#include "chroma_farfield_polarization.h"
#include "../color/color_edge.h"
#include "../color/color_palette.h"
#include "../color/color_tone.h"
#include "../shared.h"

/* Origin derivation inputs, compared and assigned whole: the step and
 * presentation generation placing the pattern vertices, the cell count
 * spanning them, and the rigid displacement carrying them to the drawn
 * frame. */
typedef struct
{
  int        fstep;
  uint32_t   generation;
  int        total;
  point_3d_t translation;
  gboolean   valid;
} ff_origin_edge_t;

/* Resolved entry buffer and the input edge gating its rebuild, indexed by
 * pattern cell. */
static field_vector_entry_t *ff_entry_buf;

static color_edge_t ff_edge;
static gboolean     ff_edge_valid;

/* What the stored origins were derived from */
static ff_origin_edge_t ff_origin_edge;

/*-----------------------------------------------------------------------*/

/**
 * ff_origin_edge_eq() - Whether two origin snapshots carry identical inputs
 * @a: stored snapshot
 * @b: candidate snapshot
 */
  static gboolean
ff_origin_edge_eq(const ff_origin_edge_t *a, const ff_origin_edge_t *b)
{
  return a->valid && b->valid
      && a->fstep == b->fstep && a->generation == b->generation
      && a->total == b->total
      && dl_feq(a->translation.x, b->translation.x)
      && dl_feq(a->translation.y, b->translation.y)
      && dl_feq(a->translation.z, b->translation.z);

} /* ff_origin_edge_eq() */

/*-----------------------------------------------------------------------*/

/**
 * ff_follow_display_rotation() - Carry resolved tangents into the drawn frame
 * @rot: display rotation the presentation applied to the pattern vertices
 * @entries: records whose displacements rotate in place
 * @total: pattern cell count
 *
 * Noise mode tilts the drawn pattern so the sky and earth boundary reads
 * horizontal, while the resolve pass builds its tangent basis from the
 * untilted spherical angles.  The same rotation carries each displacement
 * onto the surface it attaches to.  An untilted pattern keeps the resolved
 * displacement.
 */
  static void
ff_follow_display_rotation(const ff_rotation_t *rot,
    field_vector_entry_t *entries, int total)
{
  int idx;

  if( !ff_rotation_tilted(rot) )
    return;

  for( idx = 0; idx < total; idx++ )
  {
    field_vector_t *vector = &entries[idx].vector;
    double xr, yr, zr;

    ant_temp_rotate_vector((double)vector->dx, (double)vector->dy,
        (double)vector->dz, rot->axis_phi, rot->angle, &xr, &yr, &zr);

    vector->dx = (float)xr;
    vector->dy = (float)yr;
    vector->dz = (float)zr;
  }

} /* ff_follow_display_rotation() */

/*-----------------------------------------------------------------------*/

/**
 * ff_set_origins() - Attach entries to translated pattern vertices
 * @entries: records receiving their drawn origins
 * @vertices: authoritative pattern surface vertices
 * @want: origin derivation inputs this call records
 *
 * An origin follows the placed vertex and the rigid displacement alone, so a
 * frame repeating those inputs keeps the origins already stored.
 */
static void
ff_set_origins(field_vector_entry_t *entries, const point_3d_t *vertices,
    const ff_origin_edge_t *want)
{
  int idx;

  if( ff_origin_edge_eq(&ff_origin_edge, want) )
    return;

  for( idx = 0; idx < want->total; idx++ )
  {
    entries[idx].origin.x = vertices[idx].x + want->translation.x;
    entries[idx].origin.y = vertices[idx].y + want->translation.y;
    entries[idx].origin.z = vertices[idx].z + want->translation.z;
    entries[idx].origin.r = vertices[idx].r;
  }

  ff_origin_edge = *want;

} /* ff_set_origins() */

/*-----------------------------------------------------------------------*/

  field_vector_set_t
chroma_proj_frame_farfield(int fstep, double phase,
    const point_3d_t *translation)
{
  field_vector_set_t out = { NULL, 0.0 };
  const point_3d_t *verts;
  ff_operator_t quantity;
  color_tone_t fam;
  tone_param_t tp;
  color_edge_t want;
  ff_origin_edge_t origin_want;
  double cos_ph, sin_ph, env_peak, ratio;
  int total, nth, nph, idx;

  if( rad_pattern == NULL || ff_pre == NULL || fstep < 0 || !save.fstep[fstep] )
    return out;

  total = fpat.nth * fpat.nph;
  if( total <= 0 || rad_pattern[fstep].phasor == NULL ||
      ff_pre[fstep].vertices == NULL )
    return out;

  ratio = rc_config.ff_vector_length_ratio;
  fam   = color_tone_active();
  tone_param_init(&tp, fam);
  verts = ff_pre[fstep].vertices;
  origin_want = (ff_origin_edge_t){
      .fstep = fstep,
      .generation = ff_pre[fstep].generation,
      .total = total,
      .translation = *translation,
      .valid = TRUE };

  /* The far-zone magnetic field is r_hat x E over the free-space impedance:
   * a quarter turn in the tangent plane, in time phase with the electric
   * field.  The impedance divides every cell alike while the arrow length
   * follows the surface and the color normalizes against this frame's own
   * peak, so it cancels and the turn is what remains. */
  if( rc_config.ff_quantity == FF_QTY_HFIELD )
    quantity = (ff_operator_t){ 0.0, -1.0, 1.0, 0.0 };
  else
    quantity = (ff_operator_t){ 1.0, 0.0, 0.0, 1.0 };

  /* Each arrow spans this fraction of its own cell radius, so the longest
   * spans the same fraction of the pattern radius */
  out.extent = (double)ff_pre[fstep].pattern_radius * ratio;

  /* The phase enters the edge, so a scrub back to a drawn phase hits the
   * cache.  The presentation generation enters it because the arrows attach
   * to the surface that generation placed, the polarization, reference, and
   * quantity because they select what is drawn, and the length ratio, tone
   * parameter, and dB floor so a slider change invalidates the frame. */
  want = (color_edge_t){ .fstep = fstep, .elem = calc_data.pol_type,
      .chan = rc_config.ff_quantity, .fam = (int)fam,
      .proj = rc_config.ff_frame, .phase = phase, .cmax = ratio,
      .param = tp.param, .flr = tp.floor_ratio,
      .freq_mhz = calc_data.freq_mhz, .gen_a = ff_pre[fstep].generation,
      .palette = color_palette_generation() };

  if( ff_edge_valid && color_edge_eq(&ff_edge, &want) )
  {
    /* Displacement and color stand; the origins follow their own derivation
     * inputs, so a repeated placement keeps the stored origins. */
    ff_set_origins(ff_entry_buf, verts, &origin_want);
    out.entries = ff_entry_buf;
    return out;
  }

  mem_array_realloc(&ff_entry_buf, total);

  cos_ph   = cos(phase);
  sin_ph   = sin(phase);
  env_peak = 0.0;
  idx      = 0;

  /* Geometry pass: the selected component of the stored pair evaluated at the
   * phase and composed as e_th * theta_hat + e_ph * phi_hat, stretched to its
   * own cell's share of the pattern radius so the arrows follow whatever gain
   * style shaped the surface.  Scan the standing envelope peak for the
   * colorize pass; that peak carries no phase, so the color scale holds still
   * while the arrows sweep through it. */
  for( nph = 0; nph < fpat.nph; nph++ )
  {
    ff_operator_t op;
    double cs, sn;

    if( rc_config.ff_frame == FF_FRAME_LUDWIG3 )
    {
      /* Co-polar direction held fixed across the pattern by the cell azimuth */
      cs = geom_pre.cos_phi[nph];
      sn = geom_pre.sin_phi[nph];
    }
    else
    {
      /* World spherical basis, the linear pair left unturned */
      cs = 1.0;
      sn = 0.0;
    }

    op = ff_compose(quantity, ff_pol_operator(calc_data.pol_type, cs, sn));

    for( nth = 0; nth < fpat.nth; nth++ )
    {
      const ff_phasor_t *ph = &rad_pattern[fstep].phasor[idx];
      complex double eth = op.a * ph->eth + op.b * ph->eph;
      complex double eph = op.c * ph->eth + op.d * ph->eph;
      double env = sqrt(creal(eth * conj(eth)) + creal(eph * conj(eph)));
      double e_th = creal(eth) * cos_ph - cimag(eth) * sin_ph;
      double e_ph = creal(eph) * cos_ph - cimag(eph) * sin_ph;
      double len_scale, th_disp, ph_disp, th_xy;

      /* A cell carrying no field draws an arrow of no length */
      if( env > 0.0 )
        len_scale = ratio * verts[idx].r / env;
      else
        len_scale = 0.0;

      th_disp = e_th * len_scale;
      ph_disp = e_ph * len_scale;
      th_xy   = th_disp * geom_pre.cos_theta[nth];

      ff_entry_buf[idx].vector.dx =
        (float)(th_xy * geom_pre.cos_phi[nph] -
                ph_disp * geom_pre.sin_phi[nph]);
      ff_entry_buf[idx].vector.dy =
        (float)(th_xy * geom_pre.sin_phi[nph] +
                ph_disp * geom_pre.cos_phi[nph]);
      ff_entry_buf[idx].vector.dz =
        (float)(-th_disp * geom_pre.sin_theta[nth]);

      ff_entry_buf[idx].magnitude = sqrt(e_th * e_th + e_ph * e_ph);
      if( env > env_peak )
        env_peak = env;

      idx++;
    }
  }

  /* Colorize pass: amplitude ramp of the instantaneous magnitude against the
   * standing envelope peak through the active tone */
  for( idx = 0; idx < total; idx++ )
    ff_entry_buf[idx].color =
      field_ramp_color(fam, &tp, ff_entry_buf[idx].magnitude, env_peak);

  /* Carry each displacement and origin into the drawn pattern frame. */
  ff_follow_display_rotation(&ff_pre[fstep].rotation, ff_entry_buf, total);
  ff_set_origins(ff_entry_buf, verts, &origin_want);

  ff_edge       = want;
  ff_edge_valid = TRUE;

  out.entries = ff_entry_buf;
  return out;

} /* chroma_proj_frame_farfield() */

/*-----------------------------------------------------------------------*/

  void
chroma_ff_free(void)
{
  mem_array_free(&ff_entry_buf);
  ff_edge        = (color_edge_t){ 0 };
  ff_edge_valid  = FALSE;
  ff_origin_edge = (ff_origin_edge_t){ 0 };

} /* chroma_ff_free() */

/*-----------------------------------------------------------------------*/
