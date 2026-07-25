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

/* Command Mnemonics */
enum CMND_MNM
{
  CM = 0, // 0
  CP, // 1
  EK, // 2
  EN, // 3
  EX, // 4
  FR, // 5
  GD, // 6
  GN, // 7
  KH, // 8
  LD, // 9
  NE, // 10
  NH, // 11
  NT, // 12
  PQ, // 13
  PT, // 14
  RP, // 15
  SY, // 16
  TL, // 17
  XQ, // 18
  Z0, // 19
  PL, // 20
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

/*
 * cmnd_mnemonic_num - resolve a card mnemonic to its enumerator
 *
 * Consults the canonical mnemonic table first and the legacy alias table
 * second, so both the canonical and legacy spellings resolve. Returns
 * NUM_CMNDS when the mnemonic names no command card.
 */
enum CMND_MNM cmnd_mnemonic_num( const char *mn );

/*
 * cmnd_mnemonic - canonical two-character token for a command enumerator
 *
 * Returns NULL when the enumerator names no command card.
 */
const char *cmnd_mnemonic( enum CMND_MNM num );

#endif

