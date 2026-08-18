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

#include "touchstone.h"

/* Both .s2p variants carry the same nine columns and differ only in the
 * gain measurement feeding S21 and S12. */
#define TOUCHSTONE_S2P_COMMENT \
	"!MHz\tS11(dB)\tS11(Ang)\tS21(dB)\tS21(Ang)" \
	"\tS12(dB)\tS12(Ang)\tS22(dB)\tS22(Ang)\n"

#define TOUCHSTONE_S2P_FORMAT(gain) \
	"{mhz}\t{s11_real}\t{s11_ang}" \
	"\t{" gain "}\t0\t{" gain "}\t0\t-100\t0\n"

/* Column layouts written by Save_FreqPlots_Touchstone().
 *
 * The option line declares "DB" format, so each column pair is
 * (20*log10|S|, angle in degrees).
 *
 * A .s2p file presents the antenna as a two-port: port 1 is the feedpoint
 * and port 2 is the far field along the direction the gain refers to.  We
 * assume they are passive so S21==S12.  S22 is a bit of a mystery, so we
 * assume that all S22 behavior is normalized into S11 and thus S22 is
 * deminimus and set it to -100 dB.
 *
 * Touchstone defines S21 as b2/a1 with a2 = 0, so a1 carries the power
 * available from a source of reference impedance Z0 rather than the power
 * the feedpoint accepts.  NEC normalizes gain to accepted power, which
 * leaves the mismatch factor 1-|S11|^2 out of it.  The S21 column
 * therefore holds realized gain, which xnec2c measures as gain_net and
 * gain_viewer_net; raw gain would overstate delivered power by
 * 1/(1-|S11|^2).  See github issues #80 and #93.
 *
 * The dBi value lands in the S21 dB-magnitude column, making |S21|^2 the
 * realized power gain; S21 thus carries the field-amplitude term.
 */
const touchstone_layout_t touchstone_layouts[TOUCHSTONE_COUNT] = {
	[TOUCHSTONE_S1P] = {
		.comment = "!MHz\tS11(dB)\tS11(Ang)\n",
		.format  = "{mhz}\t{s11_real}\t{s11_ang}\n",
	},

	[TOUCHSTONE_S2P_MAXGAIN] = {
		.comment = TOUCHSTONE_S2P_COMMENT,
		.format  = TOUCHSTONE_S2P_FORMAT("gain_net"),
	},

	[TOUCHSTONE_S2P_VIEWERGAIN] = {
		.comment = TOUCHSTONE_S2P_COMMENT,
		.format  = TOUCHSTONE_S2P_FORMAT("gain_viewer_net"),
	},
};
