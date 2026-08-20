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

#ifndef STRUCTURE_UI_H
#define STRUCTURE_UI_H  1

#include "common.h"
#include "chroma/chroma.h"

void Draw_Structure_UI(void);
void Show_Viewer_Gain(GtkBuilder *builder, gchar *widget, view_t *v);
void Alloc_Crnt_Fstep_Buffers(int nfrq);
void free_crnt_fstep_buffers(void);
void Queue_Structure_Redraw(gboolean force);
void Queue_Structure_Rebuild(gboolean force);
void structure_view_changed_cb(view_t *v, gpointer user_data);
void Init_Struct_Drawing(void);
gboolean Animate_Phase(gpointer udata);
void apply_animation_phase(void);
void reset_animation_phase(void);
void animation_set_scrubbed(void);
gboolean animation_is_active(void);
chroma_proj_t color_proj_active(void);

#endif
