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

#ifndef	FORK_H
#define	FORK_H		1

#include "common.h"

/* Parent/child commands */
#define FORK_CMNDS { "inpfile", "frqdata", "nearehf" }

/* Indices for parent/child commands */
enum P2CH_COMND
{
  INFILE = 0,
  FRQDATA,
  EHFIELD,
  NUM_FKCMNDS
};

/* Near Field select flags */
#define E_HFIELD	0x01
#define SNAPSHOT	0x02
#define EFIELD		0x04
#define HFIELD		0x08

#endif
