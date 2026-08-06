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

#ifndef FORK_H
#define FORK_H      1

#include "common.h"

/* Wire width of a parent/child command tag */
#define FORK_CMD_LEN 7

/* Indices for parent/child commands */
enum P2CH_COMND
{
  INFILE = 0,
  FRQDATA,
  NUM_FKCMNDS
};

/* FRQDATA payload: the math library the child adopts, the thread budget it
 * runs that library with, and the frequency it solves.  The widest member
 * sits last so the structure closes within one cache line; the transfer
 * walks each member by its own width, so padding never reaches the wire. */
typedef struct
{
  char   mathlib_id[MATHLIB_ID_LEN];
  int    threads;
  double freq_mhz;
} fork_frqdata_t;

void fork_send_infile( int idx );
void fork_send_frqdata( int idx, fork_frqdata_t *frq );

#endif
