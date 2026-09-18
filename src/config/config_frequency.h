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

#ifndef CONFIG_FREQUENCY_H
#define CONFIG_FREQUENCY_H 1

#include "widget/config_widget.h"

/* config_frequency: the frequency selection's configuration domain, holding
 * the change-edge hook its rows commit through and the session-only
 * apply-frequency field that has no persistence row. */

/** config_frequency_init - Register the session-only apply-frequency field
 *
 * Called once from main.c startup, before Read_Config(); rc_config_vars
 * rows register themselves separately via rc_config_register_widgets().
 */
void config_frequency_init(void);

void hook_frequency(void);

#endif /* CONFIG_FREQUENCY_H */
