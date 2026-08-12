/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

/*
 * freqplots_xy: Dual-axis frequency XY plot type.
 *
 * Plot_Graph lays out one panel per FR card: titles and scales as deferred
 * text, the grid and traces as depth-layered segments, and the current-
 * frequency line on the top segment layer.
 */

#include "freqplots_internal.h"
#include "freqplots_locus.h"
#include "../shared.h"

#include <math.h>

/* Plot_Graph()
 *
 * Plots graphs of two functions against a common variable
 *
 * y_left or y_right may be NULL, in which case it is omitted.
 */
  void
Plot_Graph(
    freqplots_view_t *v, fp_render_t *fp,
    double *y_left, double *y_right, double *x, int nx,
    int *card_nfsteps, char *titles[], int posn, fp_panel_t panel)
{
	// Pointer to the FR card's plot_rect
	GdkRectangle *plot_rect = NULL;

	// Min/max values for the data series
	double max_y_left = 0.0, min_y_left = 0.0, max_y_right = 0.0, min_y_right = 0.0;

	// Values for the fr_plot->plot_rect object below in the FR card loop
	int plot_rect_y, plot_rect_height;

	// Values for the width available for plotting.
	int width_available;

	// Values for pixel padding in the UI:
	int pad_y_px_above_scale, pad_y_bottom_scale_text, pad_y_title_text,
		pad_x_scale_text, pad_x_px_after_scale, pad_x_between_graphs;

	// Values for scale markers:
	int px_per_vert_scale, px_per_horiz_scale,
		n_vert_scale,      n_horiz_scale;

	int i;

	// Reused per-card pixel rail deposited into the click-resolution registry.
	GdkPoint *rail = NULL;

	if (calc_data.freq_step < 0)
		return;

	// Active color theme, per-purpose widths, and per-panel density resolved
	// once at plot entry and passed down to the leaf drawing helpers.
	const theme_t    *th = theme_active();
	const fp_width_t *w  = fp_width_active();
	fp_density_t      density = fp_density_for(v->ngraph);
	fp_style_t        style = { .theme = th, .width = w, .density = density };

	// Get the pixel size of the scale text on left and right of the graph.
	pango_text_size(v->canvas,
		&pad_x_scale_text,
		&pad_y_bottom_scale_text, "1234.5");

	// Configurable pad values:
	// Number of pixels below the graph above the horizontal scale values:
	pad_y_px_above_scale  =  8;

	// Number of pixels to the right (and left) of the vertical scale on each side:
	pad_x_px_after_scale = 8;

	// Number of pixels between each FR card graph:
	pad_x_between_graphs = 10;

	// Show a vertical or horizontal scale line every N pixels. These
	// are scaled based on the size of the text from the call to
	// pango_text_size() above:

	// Space between vertical scale lines across the X axis:
	px_per_vert_scale = 75;

	// Space between horizontal scale lines down the Y axis:
	px_per_horiz_scale = pad_y_bottom_scale_text*3;

	// Title text is the same as the scale text, but change this
	// if the title font-size ever changes!
	pad_y_title_text   = pad_y_bottom_scale_text;

	/* Available height for each graph.
	* (calc_data.ngraph is the number of graphs to be plotted) */
	plot_rect_height =
		v->height / v->ngraph - (
			pad_y_title_text +
			pad_y_px_above_scale +
			pad_y_bottom_scale_text
		);

	// Scales start at the same value, may be modified below.
	// The number of horizontal scale lines (n_horiz_scale) is
	// calculated here. (But n_vert_scale is calculated below in
	// the FR card loop because it can change with the plot size.)
	n_horiz_scale = plot_rect_height / px_per_horiz_scale;

	if (n_horiz_scale < 3)
		n_horiz_scale = 3;

	// Calculate the entire available width for plotting:
	width_available =
		(v->width - (
			// 2x, one for each side, left and right:
			2*pad_x_scale_text +
			2*pad_x_px_after_scale +

			// One less space between graphs than there
			// are graphs:
			(pad_x_between_graphs*(calc_data.FR_cards-1))
		));


	/* Draw titles */
	plot_rect_y = (v->height * posn) / v->ngraph;

	fp_add_text(fp, pad_x_scale_text + pad_x_px_after_scale, plot_rect_y,
		    1.0f, titles[0], JUSTIFY_LEFT, th->colors[THEME_ROLE_LABEL_PRIMARY]);

	fp_add_text(fp, v->width / 2, plot_rect_y, 1.0f, titles[1],
		    JUSTIFY_CENTER, th->colors[THEME_ROLE_LABEL_AXIS]);

	fp_add_text(fp,
		    v->width - (pad_x_scale_text + pad_x_px_after_scale),
		    plot_rect_y, 1.0f, titles[2], JUSTIFY_RIGHT, th->colors[THEME_ROLE_LABEL_SECONDARY]);

	// Increase the y position to account for the title text height above:
	plot_rect_y += pad_y_title_text;

	// Calculate min/max if defined:
	if (y_left != NULL)
	{
		max_y_left = min_y_left = y_left[0];
		for( i = 1; i < nx; i++ )
		{
			if( max_y_left < y_left[i] )
				max_y_left = y_left[i];
			if( min_y_left > y_left[i] )
				min_y_left = y_left[i];

		}
	}

	// Calculate min/max if defined:
	if (y_right != NULL)
	{
		max_y_right = min_y_right = y_right[0];
		for( i = 1; i < nx; i++ )
		{
			if( max_y_right < y_right[i] )
				max_y_right = y_right[i];
			if( min_y_right > y_right[i] )
				min_y_right = y_right[i];

		}
	}

	// We need to fit the scales depending on whether left or right are NULL
	if (y_left != NULL && y_right == NULL)
		Fit_to_Scale(&max_y_left, &min_y_left, &n_horiz_scale);
	else if (y_right != NULL && y_left == NULL)
		Fit_to_Scale(&max_y_right, &min_y_right, &n_horiz_scale);
	else // both are defined
		Fit_to_Scale2(
			&max_y_left, &min_y_left,
			&max_y_right, &min_y_right,
			&n_horiz_scale);


	// Set min/max_y_left and nval for left and right:
	if (y_left != NULL)
		Plot_Vertical_Scale(
			fp,
			th->colors[THEME_ROLE_LABEL_PRIMARY],
			pad_x_scale_text, plot_rect_y,
			plot_rect_height,
			max_y_left, min_y_left, n_horiz_scale);

	if (y_right != NULL)
		Plot_Vertical_Scale(
			fp,
			th->colors[THEME_ROLE_LABEL_SECONDARY],
			v->width-pad_x_px_after_scale,
			plot_rect_y,
			plot_rect_height,
			max_y_right, min_y_right, n_horiz_scale);


	// Plot FR cards:
	int fr, x_offset = 0, offset = 0;

	x_offset = pad_x_scale_text	+ pad_x_px_after_scale;
	for (fr = 0; fr < calc_data.FR_cards; fr++)
	{
		fr_plot_t *fr_plot = get_fr_plot(v, posn, fr);
		fr_plot->panel_type = panel;
		plot_rect = &fr_plot->plot_rect;

		// Set the y/height values from above:
		fr_plot->plot_rect.y = plot_rect_y;
		fr_plot->plot_rect.height = plot_rect_height;

		// Setup the x position for this plot:
		fr_plot->plot_rect.x = x_offset;

		// If fr_plot->plot_rect.width is uninitialized then do it now.  We can't
		// initialize it early because we didn't previously know 'width_available'.
		if (plot_rect->width == 0)
			plot_rect->width = width_available / calc_data.FR_cards;

		// Resize and sync plots if the window size or number of plots have changed.
		if (v->prev_width_available != width_available ||
			plot_rect->width > width_available ||
			plot_rect->width < 0) {
				plot_rect->width = width_available / calc_data.FR_cards;
				fr_plot_sync_widths(v, fr_plot);
		}
		else if (v->prev_ngraphs != v->ngraph) {
			fr_plot_sync_widths(v, fr_plot);
		}

		// Adjust the vertical scale based on the width (assigned above).
		n_vert_scale = plot_rect->width / px_per_vert_scale;

		if (n_vert_scale < 2)
			n_vert_scale = 2;

		// Offset is the offset into the value arrays:
		// Skip plot below if there are no more values to plot because
		// it has plotted all of them or the values are still
		// being calculated:
		int maxidx = nx - offset;

		// Clamp the number of index to be plotted if there
		// are some available to plot in the next FR card which
		// will be done in the next iteration of this loop:
		if (maxidx > card_nfsteps[fr])
			maxidx = card_nfsteps[fr];

		/* Draw plotting frame */

		// Scale the X-axis frequency values to nice round numbers:
		fscale_extent_fit( fr_plot->freq_loop_data,
			rc_config.freqplots_round_x_axis,
			&fr_plot->min_fscale, &fr_plot->max_fscale, &n_vert_scale );

		// local shorthand variables
		double min_fscale = fr_plot->min_fscale;
		double max_fscale = fr_plot->max_fscale;

		Draw_Plotting_Frame( fp, &style, titles,
			plot_rect, n_horiz_scale, n_vert_scale);

		Plot_Horizontal_Scale(
			fp,
			th->colors[THEME_ROLE_LABEL_AXIS],
			plot_rect->x,
			plot_rect->y + plot_rect->height,
			plot_rect->width,
			max_fscale, min_fscale, n_vert_scale);

		if (maxidx > 0 && y_left != NULL)
		{
			Draw_Graph(
				fp,
				&style,
				th->colors[THEME_ROLE_SERIES_PRIMARY],
				plot_rect,
				y_left+offset, x+offset,
				max_y_left, min_y_left,
				max_fscale, min_fscale,
				maxidx, card_nfsteps[fr],
				LEFT );
		}

		if (maxidx > 0 && y_right != NULL)
		{
			Draw_Graph(
				fp,
				&style,
				th->colors[THEME_ROLE_SERIES_SECONDARY],
				plot_rect,
				y_right+offset, x+offset,
				max_y_right, min_y_right,
				max_fscale, min_fscale,
				maxidx, card_nfsteps[fr],
				RIGHT);
		}

		/* Draw a vertical line to show current freq if it was
		* changed by a user click on the plots drawingarea
		* The +/- 0.001 is to adjust for floating-point error,
		* for example: freq_mhz=148.000000 !<= max_fscale=147.999996 */
		if( calc_data.fmhz_save > 0.0
			&& calc_data.fmhz_save >= min_fscale - 1e-6
			&& calc_data.fmhz_save <= max_fscale + 1e-6)
		{
			double freq_x;
			rgb_f_t cursor_c = fp_cursor_color(th);

			freq_x = (calc_data.fmhz_save - min_fscale) / (max_fscale - min_fscale);
			freq_x *= plot_rect->width;

			/* The cursor is a UI marker, not panel data; it keeps the full
			 * w_cursor width unscaled by panel density so a stacked-panel
			 * sub-pixel line never anti-aliases the marker to a dim hue,
			 * matching the unscaled Smith cursor and legacy stroke. */
			fp_add_line(fp,
				(int)(plot_rect->x + freq_x), plot_rect->y,
				(int)(plot_rect->x + freq_x), plot_rect->y+plot_rect->height,
				(fp_stroke_t){ .color = cursor_c, .width = w->widths[FP_W_CURSOR], .z_mid = FP_Z_GREEN });
		}

		/* Deposit this card's click-resolution geometry.  A primary click
		 * lerps an arbitrary frequency across the full rendered x-axis, so the
		 * continuum spans the panel edges (min_fscale at the left, max_fscale
		 * at the right) at a constant y the resolver reduces to pure x.  A
		 * secondary click snaps to the rendered samples, the extra slot among
		 * them; rail and trace share fp_axis_pixel_x, so snap and trace
		 * stay aligned. */
		if (maxidx > 0)
		{
			int rail_y = plot_rect->y + plot_rect->height / 2;
			int s;
			GdkPoint edges[2];
			double   edge_freq[2];

			mem_array_realloc(&rail, maxidx);
			for (s = 0; s < maxidx; s++)
			{
				rail[s].x = fp_axis_pixel_x(plot_rect, x[offset + s],
					min_fscale, max_fscale);
				rail[s].y = rail_y;
			}

			edges[0].x = plot_rect->x;
			edges[0].y = rail_y;
			edge_freq[0] = min_fscale;
			edges[1].x = plot_rect->x + plot_rect->width;
			edges[1].y = rail_y;
			edge_freq[1] = max_fscale;

			fp_locus_add(v, &(fp_locus_input_t){
				.panel = panel, .rect = *plot_rect,
				.cont_pts = edges, .cont_freq = edge_freq, .cont_n = 2,
				.snap_pts = rail, .snap_freq = x + offset, .snap_n = maxidx });
		}

		// Next FR card index:
		x_offset += plot_rect->width + pad_x_between_graphs;
		offset += card_nfsteps[fr];
	}

	mem_array_free(&rail);

	v->prev_width_available = width_available;
	v->prev_ngraphs = v->ngraph;
}
