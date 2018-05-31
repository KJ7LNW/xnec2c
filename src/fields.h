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
 */

#ifndef	FIELDS_H
#define	FIELDS_H		1

#include "common.h"

#define	TPJ		(0.0+I*M_2PI)
#define PI8		25.13274123
#define	FPI		12.56637062
#define	CONST2	4.771341188

/* common  /tmi/ */
typedef struct
{
  int ij;

  double
    zpk,
    rkb2;

} tmi_t;

/*common  /tmh/ */
typedef struct
{
  double
    zpka,
    rhks;

} tmh_t;

#endif

