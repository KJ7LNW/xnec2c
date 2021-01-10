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

#ifndef	EDITORS_H
#define	EDITORS_H		1

#include "common.h"

#define CMND_NUM_FCOLS 6
#define CMND_NUM_ICOLS 4
#define GEOM_NUM_FCOLS 7
#define GEOM_NUM_ICOLS 2

#define NEXT		TRUE
#define PREVIOUS	FALSE

/* Editor actions that imply saving */
enum SAVE_ACTION
{
  EDITOR_NEW = 64,
  EDITOR_EDIT,
  EDITOR_APPLY,
  EDITOR_OK
};

enum EDITOR_FUNC
{
  /* General editor function actions */
  EDITOR_DATA = 0,
  EDITOR_TAGNUM,
  EDITOR_CANCEL,
  EDITOR_SEGPC,
  EDITOR_RDBUTTON,
  EDITOR_LOAD,

  /* Wire editor function actions */
  WIRE_EDITOR_WLEN,
  WIRE_EDITOR_RLEN,
  WIRE_EDITOR_DIAN,
  WIRE_EDITOR_RDIA,
  WIRE_EDITOR_TAPR,

  /* Helix editor function actions */
  HELIX_EDITOR_NTURN,
  HELIX_EDITOR_TSPACE,
  HELIX_EDITOR_RH_HELIX,
  HELIX_EDITOR_LH_HELIX,
  HELIX_EDITOR_RH_SPIRAL,
  HELIX_EDITOR_LH_SPIRAL,
  HELIX_EDITOR_LINK_A1B1,
  HELIX_EDITOR_LINK_B1A2,
  HELIX_EDITOR_LINK_A2B2,

  /* Patch editor actions */
  PATCH_EDITOR_ARBT,
  PATCH_EDITOR_RECT,
  PATCH_EDITOR_TRIA,
  PATCH_EDITOR_QUAD,
  PATCH_EDITOR_SURF,
  PATCH_EDITOR_SCCD,

  /* Reflect editor actions */
  REFLECT_EDITOR_TOGGLE,

  /* Frequency editor actions */
  FREQUENCY_EDITOR_FSTEP,

  /* General command function actions */
  COMMAND_RDBUTTON,
  COMMAND_CKBUTTON,

  /* Nearfield check buttons toggled */
  NEARFIELD_NE_CKBUTTON,
  NEARFIELD_NH_CKBUTTON
};

/* Comment "card" columns */
enum CARDS
{
  CMNT_COL_NAME = 0,
  CMNT_COL_COMMENT,
  CMNT_NUM_COLS
};

/* Editor spinbuttons corresponding to int list columns */
enum SPIN_COL_I
{
  SPIN_COL_I1 = 0,
  SPIN_COL_I2,
  SPIN_COL_I3,
  SPIN_COL_I4
};

/* Editor spinbuttons corresponding to float list columns */
enum SPIN_COL_F
{
  SPIN_COL_F1 = 0,
  SPIN_COL_F2,
  SPIN_COL_F3,
  SPIN_COL_F4,
  SPIN_COL_F5,
  SPIN_COL_F6,
  SPIN_COL_F7
};

/* Geometry data "card" columns */
enum GEOM_CARD
{
  GEOM_COL_NAME = 0,
  GEOM_COL_I1,
  GEOM_COL_I2,
  GEOM_COL_F1,
  GEOM_COL_F2,
  GEOM_COL_F3,
  GEOM_COL_F4,
  GEOM_COL_F5,
  GEOM_COL_F6,
  GEOM_COL_F7,
  GEOM_NUM_COLS
};

/* Control/Command "card" columns */
enum CMND_CARD
{
  CMND_COL_NAME = 0,
  CMND_COL_I1,
  CMND_COL_I2,
  CMND_COL_I3,
  CMND_COL_I4,
  CMND_COL_F1,
  CMND_COL_F2,
  CMND_COL_F3,
  CMND_COL_F4,
  CMND_COL_F5,
  CMND_COL_F6,
  CMND_NUM_COLS
};

/* F1-F7 columns of GW & GC cards */
enum GW_COLS
{
  WIRE_X1 = 0,
  WIRE_Y1,
  WIRE_Z1,
  WIRE_X2,
  WIRE_Y2,
  WIRE_Z2,
  WIRE_DIA,
  WIRE_RLEN,
  WIRE_DIA1,
  WIRE_DIAN,
  WIRE_RDIA,
  WIRE_PCL,
  WIRE_LEN,
  WIRE_RES
};

/* Patch types */
enum PATCH_TYPE
{
  PATCH_ARBT = 0,
  PATCH_RECT,
  PATCH_TRIA,
  PATCH_QUAD,
  PATCH_SURF
};

/* F1-F6 columns of SP & SC cards */
enum SP_COLS
{
  PATCH_X1 = 0,
  PATCH_Y1,
  PATCH_Z1,
  PATCH_X2,
  PATCH_Y2,
  PATCH_Z2,
  UNUSED_F1,
  PATCH_X3,
  PATCH_Y3,
  PATCH_Z3,
  PATCH_X4,
  PATCH_Y4,
  PATCH_Z4,
  UNUSED_F2
};

/* GA card float data */
enum GA_CARD
{
  ARC_RAD = 0,
  ARC_END1,
  ARC_END2,
  ARC_DIA,
  ARC_PCL,
  ARC_RES
};

/* GH card float data */
enum GH_CARD
{
  HELIX_TSPACE = 0,
  HELIX_LEN,
  HELIX_RXZO,
  HELIX_RYZO,
  HELIX_RXZHL,
  HELIX_RYZHL,
  HELIX_DIA,
  HELIX_PCL,
  HELIX_NTURN,
  HELIX_RES
};

#endif

