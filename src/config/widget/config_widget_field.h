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

#ifndef CONFIG_WIDGET_FIELD_H
#define CONFIG_WIDGET_FIELD_H 1

#include <string.h>

/* Size-aware field accessors.  Fields may be typed as enums whose storage
 * width is implementation-defined; these helpers read/write through the
 * field's actual width via memcpy, avoiding aliasing and width mismatches
 * from direct pointer casts. */

/** field_read_int - read an int-compatible field at its actual width */
static inline int
field_read_int(const void *field, size_t size)
{
  int val = 0;
  memcpy(&val, field, size);
  return val;
}

/** field_write_int - write an int-compatible field at its actual width */
static inline void
field_write_int(void *field, size_t size, int val)
{
  memcpy(field, &val, size);
}

/** field_read_float - read a float field without aliasing */
static inline float
field_read_float(const void *field)
{
  float val;
  memcpy(&val, field, sizeof(float));
  return val;
}

/** field_write_float - write a float field without aliasing */
static inline void
field_write_float(void *field, float val)
{
  memcpy(field, &val, sizeof(float));
}

/** field_read_double - read a double field without aliasing */
static inline double
field_read_double(const void *field)
{
  double val;
  memcpy(&val, field, sizeof(double));
  return val;
}

/** field_write_double - write a double field without aliasing */
static inline void
field_write_double(void *field, double val)
{
  memcpy(field, &val, sizeof(double));
}

#endif /* CONFIG_WIDGET_FIELD_H */
