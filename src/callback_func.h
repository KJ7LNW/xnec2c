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

#ifndef CALLBACK_FUNC_H
#define CALLBACK_FUNC_H		1

#include <ctype.h>
#include "callbacks.h"
#include "interface.h"
#include "editors.h"
#include "fork.h"
#include "common.h"

typedef struct save_data
{
  GtkWidget *drawingarea;
  int width, height;
  char filename[LINE_LEN];
} save_data_t;

/* Gain colorcode strip size */
#define COLORCODE_WIDTH		96
#define COLORCODE_HEIGHT	24
#define COLORCODE_MAX		768.0  /* Max value, 8 x COLORCODE_WIDTH */

#endif

