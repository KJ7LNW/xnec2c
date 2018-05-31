/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#ifndef RC_CONFIG_H
#define RC_CONFIG_H	1

#include "common.h"
#include "utils.h"
#include "shared.h"
#include <sys/stat.h>

#define CONFIG_FILE		".config/xnec2c.conf"

/* Format of rc config file to use in snprintf() */
#define RC_CONFIG_FORMAT \
  _("# Xnec2c configuration file\n"\
  "#\n"\
  "# Application Vesrsion\n"\
  "%s\n"\
  "# Current Working Directory\n"\
  "%s\n"\
  "# Main Window Size, in pixels\n"\
  "%d,%d\n"\
  "# Main Window Position (root x and y)\n"\
  "%d,%d\n"\
  "# Main Window Currents toggle button state\n"\
  "%d\n"\
  "# Main Window Charges toggle button state\n"\
  "%d\n"\
  "# Main Window Polarization menu total state\n"\
  "%d\n"\
  "# Main Window Polarization menu horizontal state\n"\
  "%d\n"\
  "# Main Window Polarization menu vertical state\n"\
  "%d\n"\
  "# Main Window Polarization menu right hand state\n"\
  "%d\n"\
  "# Main Window Polarization menu left hand state\n"\
  "%d\n"\
  "# Main Window Frequency loop start state\n"\
  "%d\n"\
  "# Main Window Rotate spinbutton state\n"\
  "%d\n"\
  "# Main Window Incline spinbutton state\n"\
  "%d\n"\
  "# Main Window Zoom spinbutton state\n"\
  "%d\n"\
  "# Radiation Pattern Window Size, in pixels\n"\
  "%d,%d\n"\
  "# Radiation Pattern Window Position (root x and y)\n"\
  "%d,%d\n"\
  "# Radiation Pattern Window Gain toggle button state\n"\
  "%d\n"\
  "# Radiation Pattern Window EH toggle button state\n"\
  "%d\n"\
  "# Radiation Pattern Window Menu E-field state\n"\
  "%d\n"\
  "# Radiation Pattern Window Menu H-field state\n"\
  "%d\n"\
  "# Radiation Pattern Window Menu Poynting vector state\n"\
  "%d\n"\
  "# Radiation Pattern Window Zoom spinbutton state\n"\
  "%d\n"\
  "# Frequency Plots Window Size, in pixels\n"\
  "%d,%d\n"\
  "# Frequency Plots Window Position (root x and y)\n"\
  "%d,%d\n"\
  "# Frequency Plots Window Max Gain toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window Gain Direction toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window Viewer Direction Gain toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window VSWR toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window Z-real/Z-imag toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window Z-mag/Z-phase toggle button state\n"\
  "%d\n"\
  "# Frequency Plots Window Net Gain checkbutton state\n"\
  "%d\n"\
  "# NEC2 Editor Window Size, in pixels\n"\
  "%d,%d\n"\
  "# NEC2 Editor Window Position (root x and y)\n"\
  "%d,%d\n"\
  "# Structure Projection Center x and y Offset\n"\
  "%d,%d\n"\
  "# Rdpattern Projection Center x and y Offset\n"\
  "%d,%d\n"\
  "# Enable Confirm Quit Dialog\n"\
  "%d\n#")

/* Size of string buffer needed to print config file */
#define RC_FILE_BUF_SIZE	2048

#endif
