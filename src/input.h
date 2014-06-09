/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *
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

#ifndef	INPUT_H
#define	INPUT_H		1

#include "common.h"
#include "support.h"

/* Command Mnemonics */
enum CMDN_MNM
{
  CM = 0,
  CP,
  EK,
  EN,
  EX,
  FR,
  GD,
  GN,
  KH,
  LD,
  NE,
  NH,
  NT,
  PQ,
  PT,
  RP,
  SY,
  TL,
  XQ,
  NUM_CMNDS
};

/* Geometry Mnemonics */
enum GEOM_MNM
{
  GW = 0,
  GX,
  GR,
  GS,
  GE,
  GM,
  SP,
  SM,
  GA,
  SC,
  GH,
  GF,
  CT,
  NUM_GEOMN
};

#endif

