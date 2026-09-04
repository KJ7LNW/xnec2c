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
 * chroma_farfield_polarization: operators selecting the drawn far-zone
 * component.
 *
 * The polarization selection names which part of the stored phasor pair the
 * arrows describe.  These operators express that selection in vector form, so
 * the resolver applies one composed operator per cell and never branches on
 * the selection inside its own loop.
 */
#include "chroma_farfield_polarization.h"
#include "../shared.h"

/*-----------------------------------------------------------------------*/

/**
 * ff_compose() - Apply one operator after another
 * @outer: operator applied second
 * @inner: operator applied first
 */
  ff_operator_t
ff_compose(ff_operator_t outer, ff_operator_t inner)
{
  return (ff_operator_t){
    outer.a * inner.a + outer.b * inner.c,
    outer.a * inner.b + outer.b * inner.d,
    outer.c * inner.a + outer.d * inner.c,
    outer.c * inner.b + outer.d * inner.d };

} /* ff_compose() */

/*-----------------------------------------------------------------------*/

/**
 * ff_pol_operator() - Select the drawn polarization component
 * @pol: polarization selection the gain surface under the arrows is scaled by
 * @cs: cosine of the angle the linear pair is turned by
 * @sn: sine of the angle the linear pair is turned by
 *
 * The surface radius carries Polarization_Factor(), a power fraction taken
 * from the axial ratio and tilt of this same phasor pair.  These operators are
 * that fraction in vector form, so the arrow and the surface beneath it
 * describe one quantity while the arrow keeps the direction and the phase the
 * power fraction discards.
 *
 * The linear pair is turned by (cs, sn): the world basis passes (1, 0) and
 * recovers theta_hat and phi_hat, Ludwig-3 passes the cell azimuth and
 * recovers the co-polar and cross-polar directions.  The circular selections
 * take no turn, since turning their basis multiplies each cell by a phase and
 * breaks the phase relation the animation reads between lobes.
 */
  ff_operator_t
ff_pol_operator(int pol, double cs, double sn)
{
  ff_operator_t op = { 1.0, 0.0, 0.0, 1.0 };

  switch( pol )
  {
    case POL_TOTAL:
      break;

    case POL_VERT:
      /* Co-polar member of the linear pair */
      op = (ff_operator_t){ cs * cs, -cs * sn, -cs * sn, sn * sn };
      break;

    case POL_HORIZ:
      /* Cross-polar member of the linear pair */
      op = (ff_operator_t){ sn * sn, cs * sn, cs * sn, cs * cs };
      break;

    case POL_RHCP:
      /* (E_theta + j E_phi) / sqrt(2), re-expanded on theta_hat and phi_hat.
       * The solver marks right-hand sense where E_phi lags E_theta, so the
       * right-hand direction is theta_hat - j phi_hat and this row keeps the
       * same hand the axial-ratio sign carries to the surface. */
      op = (ff_operator_t){ 0.5, 0.5 * I, -0.5 * I, 0.5 };
      break;

    case POL_LHCP:
      /* (E_theta - j E_phi) / sqrt(2), re-expanded on theta_hat and phi_hat */
      op = (ff_operator_t){ 0.5, -0.5 * I, 0.5 * I, 0.5 };
      break;

    case NUM_POL:
    default:
      BUG("far-zone arrows: unresolved polarization %d\n", pol);
      break;
  }

  return op;

} /* ff_pol_operator() */

/*-----------------------------------------------------------------------*/

  gboolean
ff_frame_turns_pol(int pol)
{
  return (pol == POL_VERT) || (pol == POL_HORIZ);

} /* ff_frame_turns_pol() */
