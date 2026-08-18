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

#ifndef TOUCHSTONE_H
#define TOUCHSTONE_H    1

/* Touchstone file variants offered by the frequency plots save dialog. */
typedef enum
{
	TOUCHSTONE_S1P,
	TOUCHSTONE_S2P_MAXGAIN,
	TOUCHSTONE_S2P_VIEWERGAIN,

	TOUCHSTONE_COUNT,
} touchstone_type_t;

/* Column layout of one Touchstone variant.
 *
 * @comment: leading "!" line naming each column for a reader
 * @format:  meas_format() template emitting one data line per frequency
 */
typedef struct
{
	const char *comment;
	const char *format;
} touchstone_layout_t;

extern const touchstone_layout_t touchstone_layouts[TOUCHSTONE_COUNT];

#endif
