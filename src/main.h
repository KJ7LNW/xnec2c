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

#ifndef MAIN_H
#define MAIN_H      1

#include "common.h"
#include "callbacks.h"
#include "interface.h"
#include "fork.h"
#include "nec2_model.h"

#ifndef WIN32
#include <sys/wait.h>
#endif

#define OPEN_INPUT_FLAGS \
  RDPAT_FLAGS       | \
  FREQ_LOOP_FLAGS   | \
  PLOT_FREQ_LINE    | \
  ENABLE_EXCITN     | \
  INPUT_OPENED

#endif

