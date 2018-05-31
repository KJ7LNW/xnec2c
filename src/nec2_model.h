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

#ifndef	NEC2_MODEL_H
#define	NEC2_MODEL_H	1

#include "common.h"
#include "interface.h"
#include "editors.h"

/* Commands for the NEC2 file editor */
enum NEC2_CMND
{
  NEC2_EDITOR_NEW = 0,
  NEC2_EDITOR_REVERT,
  NEC2_EDITOR_RELOAD,
  NEC2_EDITOR_CLEAR
};

#endif

