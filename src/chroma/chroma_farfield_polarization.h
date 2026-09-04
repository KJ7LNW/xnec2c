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

#ifndef CHROMA_FARFIELD_POLARIZATION_H
#define CHROMA_FARFIELD_POLARIZATION_H  1

#include "../common.h"

/* Complex operator carrying the stored pair to the drawn component pair:
 *   drawn theta = a * E_theta + b * E_phi
 *   drawn phi   = c * E_theta + d * E_phi */
typedef struct
{
  complex double a, b, c, d;
} ff_operator_t;

/**
 * ff_compose() - Apply one operator after another
 * @outer: operator applied second
 * @inner: operator applied first
 */
ff_operator_t ff_compose(ff_operator_t outer, ff_operator_t inner);

/**
 * ff_pol_operator() - Operator selecting the drawn polarization component
 * @pol: polarization selection the gain surface under the arrows is scaled by
 * @cs:  cosine of the angle the linear pair is turned by
 * @sn:  sine of the angle the linear pair is turned by
 */
ff_operator_t ff_pol_operator(int pol, double cs, double sn);

/**
 * ff_frame_turns_pol() - Whether the polarization reference reaches the draw
 * @pol: polarization selection the gain surface is scaled by
 *
 * Only the linear pair takes the reference turn; every other selection
 * resolves to a constant operator.  The animate panel greys its reference
 * control by this answer, so the control offers a choice only where the
 * choice reaches the drawing.
 */
gboolean ff_frame_turns_pol(int pol);

#endif /* CHROMA_FARFIELD_POLARIZATION_H */
