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

#ifndef INPUT_H
#define INPUT_H     1

#include "common.h"
#include "sy_expr.h"
#include "expr_edit.h"
#include <ctype.h>

#define COMMANDS \
  "CM", "CP", "EK", "EN", "EX", \
  "FR", "GD", "GN", "KH", "LD", \
  "NE", "NH", "NT", "PQ", "PT", \
  "RP", "SY", "TL", "XQ", "ZO"

/* Command Mnemonics */
enum CMND_MNM
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
  ZO,
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
  SY_GEOM,
  NUM_GEOMN
};


/*
 * readgm_get_sy_line - return raw SY card text from most recent readgm() call
 *
 * Returns the content after the "SY" mnemonic, or NULL when the last card
 * was not an SY card.
 */
const char *readgm_get_sy_line( void );

/*
 * readmn_get_sy_line - return raw SY card text from most recent readmn() call
 *
 * Returns the content after the "SY" mnemonic, or NULL when the last card
 * was not an SY card.
 */
const char *readmn_get_sy_line( void );

/*
 * readgm_get_field_exprs - return expression text from most recent readgm() call
 * @iexprs: char[2][EXPR_FIELD_LEN] — one slot per integer field (I1, I2)
 * @fexprs: char[7][EXPR_FIELD_LEN] — one slot per float field (F1-F7)
 *
 * Each slot contains the original expression text or "" for plain numeric.
 */
void readgm_get_field_exprs( char iexprs[][EXPR_FIELD_LEN],
    char fexprs[][EXPR_FIELD_LEN] );

/*
 * readmn_get_field_exprs - return expression text from most recent readmn() call
 * @iexprs: char[4][EXPR_FIELD_LEN] — one slot per integer field (I1-I4)
 * @fexprs: char[6][EXPR_FIELD_LEN] — one slot per float field (F1-F6)
 *
 * Each slot contains the original expression text or "" for plain numeric.
 */
void readmn_get_field_exprs( char iexprs[][EXPR_FIELD_LEN],
    char fexprs[][EXPR_FIELD_LEN] );

#endif

