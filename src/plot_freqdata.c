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

/*
 * Net gain added by Mark Whitis http://www.freelabs.com/~whitis/
 * References:
 *   http://www.digitalhome.ca/forum/showpost.php?p=744018&postcount=47
 *      NetGain = RawGain+10*log(Feed-pointGain)
 *      where Feed-point Gain = 4*Zr*Zo/((Zr+Zo)^2+Zi^2)
 *   http://www.avsforum.com/avs-vb/showthread.php?p=14086104#post14086104
 *      NetGain = RawGain+10*log(4*Zr*Zo/((Zr+Zo)^2+Zi^2)
 *   Where log is log10.
 */

#include "plot_freqdata.h"
#include "shared.h"

// Move this to be used in all calls to mem_realloc:
#define __S1(x) #x
#define __S2(x) __S1(x)
#define  __LOCATION__ "in " __S2(__FILE__) " on line " __S2(__LINE__)

#define FR_PLOT_T_MAGIC 				0xc2bca3083893e65eULL
#define FR_PLOT_T_IS_VALID(fr_plot_ptr)	((fr_plot_ptr)->valid == FR_PLOT_T_MAGIC)

typedef struct {
	GdkRectangle plot_rect;
	int posn;			// Position in the frequency plots
	int fr;				// index into calc_data.freq_loop_data[fr]

	// Pointer to &calc_data.freq_loop_data[fr]
	freq_loop_data_t *freq_loop_data;

	// Because we are using realloc it is hard to know if the structure has
	// been initialized or if it needs to be set to sane values.  The 
	// value will equal 0xc2bca3083893e65e (just a 64-bit random number) if
	// it is valid.
	uint64_t valid; 
} fr_plot_t;

fr_plot_t *fr_plots = NULL;

// prev_width_available is used if to detect window resize in Plot_Graph.
// It must be global so it can be reset to zero when the window closes.
// Maybe there is a better way to detect that it is the first call to Plot_Graph after
// closing the window, but for now this works:
static int prev_width_available = 0;


/* Graph plot bounding rectangle */
static double Fit_to_Scale( double *max, double *min, int *nval );

/* helper function to get width and height by creating a layout */
static void pango_text_size(GtkWidget* widget, int *width, int *height, char *s)
{
	PangoLayout *layout = NULL;
    layout = gtk_widget_create_pango_layout(freqplots_drawingarea, s);
    pango_layout_get_pixel_size( layout, width, height);
    g_object_unref( layout );
}

fr_plot_t *get_fr_plot(int posn, int fr)
{
	if (posn < 0 || posn >= calc_data.ngraph ||	fr < 0 || fr >= calc_data.FR_cards)
		return NULL;
		
	return &fr_plots[posn*calc_data.FR_cards + fr];
}

GdkRectangle *get_plot_rect(int posn, int fr)
{
	fr_plot_t *p = get_fr_plot(posn, fr);

	if (p == NULL)
		return NULL;
	
	return &p->plot_rect;
}

void fr_plot_sync_widths(fr_plot_t *fr_plot)
{
	GdkRectangle *current, *r;
	int posn, fr;
	// Update all plot widths so they re the same as fr_plot's width.
	for (posn = 0; posn < calc_data.ngraph; posn++)
	{
		// Skip the current one the mouse adjusted:
		if (posn == fr_plot->posn)
			continue;

		for (fr = 0; fr < calc_data.FR_cards; fr++)
{
			current = get_plot_rect(fr_plot->posn, fr);
			r = get_plot_rect(posn, fr);

			r->width = current->width;
		}
	}

}

void print_fr_plot(fr_plot_t *p)
{
	printf("fr_plot[posn=%d, fr=%d] rect[x=%d, y=%d, w=%d, h=%d] freq[min=%f, max=%f, steps=%d]\n",
		p->posn,
		p->fr,
		p->plot_rect.x,
		p->plot_rect.y,
		p->plot_rect.width,
		p->plot_rect.height,
		p->freq_loop_data->min_freq,
		p->freq_loop_data->max_freq,
		p->freq_loop_data->freq_steps
		);
}

/* draw_text:
 * cr:      cairo object
 * widget:  the widget being drawn on
 * x:       the x position
 * y:       the y position
 * text:    the text to draw
 * justify: JUSTIFY_LEFT, JUSTIFY_CENTER, or JUSTIFY_RIGHT
 *          The justification calculates based the 'x' position. For example,
 *          if you choose JUSTIFY_CENTER, then 'x' will be the center of the
 *          text.
 * r, g, b: text color
 * width:   the text width (optional, may be NULL)
 * height:  the text height (optional, may be NULL)
 */

enum { 
	// Horizontal:
	JUSTIFY_LEFT    =  0,
	JUSTIFY_CENTER  =  1,
	JUSTIFY_RIGHT   =  2,
	JUSTIFY_HMASK   =  0x03,

	// Vertical:
	JUSTIFY_BELOW   =  0,     // Draw the text below the y coordinate
	JUSTIFY_MIDDLE  =  1<<2,  // Draw the text centered at the y coordinate
	JUSTIFY_ABOVE   =  2<<2,  // Draw the text above the y coordinate
	JUSTIFY_VMASK   =  0x0c
};

void draw_text(cairo_t *cr, GtkWidget *widget, 
	int x, int y,
	char *text, int justify,
	double r, double g, double b, 
	int *width, int *height)
{
	PangoLayout *layout;
	int w, h;

	cairo_set_source_rgb( cr, r, g, b);
	layout = gtk_widget_create_pango_layout(widget, text);

	pango_layout_get_pixel_size( layout, &w, &h);
	if (width != NULL)
		*width = w;
	
	if (height != NULL)
		*height = h;

	switch (justify & JUSTIFY_HMASK)
	{
		case JUSTIFY_RIGHT:
			x -= w;
			break;

		case JUSTIFY_CENTER:
			x -= w/2;
			break;

		case JUSTIFY_LEFT:
		default:
			break;
	}

	switch (justify & JUSTIFY_VMASK)
	{
		case JUSTIFY_ABOVE:
			y -= h;
			break;

		case JUSTIFY_MIDDLE:
			y -= h/2;
			break;

		case JUSTIFY_BELOW:
		default:
			break;
	}

	cairo_move_to( cr, x, y );

	pango_cairo_show_layout( cr, layout );
	g_object_unref( layout );
}

/*-----------------------------------------------------------------------*/

/* Display_Frequency_Data()
 *
 * Displays freq dependent data (gain, impedance etc)
 * in the entry widgets in the freq plots window
 */
  static void
Display_Frequency_Data( void )
{
  int pol, fstep;
  double vswr, gamma;
  double zrpro2, zrmro2, zimag2;
  char txt[12];

  if( isFlagClear(PLOT_ENABLED) ) return;

  /* Limit freq stepping to freq_steps FIXME */
  fstep = calc_data.freq_step;
  if( fstep >= calc_data.steps_total )
    fstep = calc_data.steps_total;

  /* Polarization type */
  pol = calc_data.pol_type;

  /* Index to gtot buffer where max gain
   * occurs for given polarization type */
  if( isFlagSet(ENABLE_RDPAT) )
  {
    /* Max gain for given polarization type */
    int mgidx = rad_pattern[fstep].max_gain_idx[pol];
    double gmax = rad_pattern[fstep].gtot[mgidx] +
      Polarization_Factor(pol, fstep, mgidx);

    /* Display max gain */
    snprintf( txt, 7, "%5.1f", gmax );
    gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
            freqplots_window_builder, "freqplots_maxgain_entry")), txt );

  } /* isFlagSet(ENABLE_RDPAT) */

  /* Display frequency */
  snprintf( txt, 11, "%9.3f", (double)calc_data.freq_mhz );
  gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
          freqplots_window_builder, "freqplots_fmhz_entry")), txt );

  /* Calculate VSWR */
  zrpro2 = (double)creal( netcx.zped ) + calc_data.zo;
  zrpro2 *= zrpro2;
  zrmro2 = (double)creal( netcx.zped ) - calc_data.zo;
  zrmro2 *= zrmro2;
  zimag2 = (double)cimag( netcx.zped );
  zimag2 *= zimag2;
  gamma = sqrt( (zrmro2 + zimag2)/(zrpro2 + zimag2) );
  vswr = (1+gamma)/(1-gamma);
  if( vswr > 999.0 )
    vswr = 999.0;

  /* Display VSWR */
  snprintf( txt, 7, "%5.1f", vswr );
  gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
          freqplots_window_builder, "freqplots_vswr_entry")), txt );

  /* Display Z real */
  snprintf( txt, 7, "%5.1f", (double)creal( netcx.zped ) );
  gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
          freqplots_window_builder, "freqplots_zreal_entry")), txt );

  /* Display Z imaginary */
  snprintf( txt, 7, "%5.1f", (double)cimag( netcx.zped ) );
  gtk_entry_set_text( GTK_ENTRY(Builder_Get_Object(
          freqplots_window_builder, "freqplots_zimag_entry")), txt );

} /* Display_Frequency_Data() */

/*-----------------------------------------------------------------------*/

/* New_Max_Min()
 *
 * Calculates new max and min scale
 * values and the number of steps
 */
  static void
New_Max_Min( double *max, double *min, double sval, int *nval )
{
  int ix;
  double i;

  i = ceil(*max / sval - 0.000001);
  ix = (int)i;
  *max = (double)ix * sval;
  i = floor(*min / sval + 0.000001);
  ix = (int)i;
  *min = (double)ix * sval;
  *nval = (int)((*max - *min) / sval + 0.5) + 1;

} /* New_Max_Min() */

/*-----------------------------------------------------------------------*/

/* Fit_to_Scale()
 *
 * Adjust the max and min value of data to be plotted,
 * as well as the number of scale sub-divisions, so that
 * sub-division values are easier to interpolate between.
 * The chosen scale values are 10, 10/2, 10/4, 10/5 and 1.
 */
static double Fit_to_Scale( double *max, double *min, int *nval )
{
  /* Acceptable scale values (10/10, 10/5, 10/4, 10/2) */
  /* Intermediate values are geometric mean of pairs */
  double scale_val[] =
  { 1.0, 1.4142, 2.0, 2.2360, 2.5, 3.5355, 5.0, 7.0710, 10.0, 14.142 };
  double subdiv_val, subdiv_order;
  int idx;

  /* Do nothing in this case */
  if( *max < *min ) return;

  /* Provide a made-up range if max = min */
  if( *max == *min )
  {
    if( *max == 0.0 )
    {
      *max =  1.0;
      *min = -1.0;
    }
    else
    {
      *max += fabs( *max ) / 10.0;
      *min -= fabs( *min ) / 10.0;
    }
  }


  /* prevent divide by zero on small screens */
  if(*nval <= 1) return 1;

  /* Find subdivision's lower order of magnitude */
  subdiv_val = (*max - *min) / (double)(*nval-1);
  subdiv_order = 1.0;
  while( subdiv_order < subdiv_val )
    subdiv_order *= 10.0;
  while( subdiv_order > subdiv_val )
    subdiv_order /= 10.0;

  /* Scale subdivision 1 < subd < 10 */
  subdiv_val /= subdiv_order;

  /* Find nearest prefered subdiv value */
  for( idx = 1; idx <= 9; idx += 2 )
    if( scale_val[idx] >= subdiv_val )
      break;

  /* Scale prefered subdiv value */
  if( idx > 9 ) idx = 9;
  subdiv_val = scale_val[idx-1] * subdiv_order;

  /* Recalculate new max and min value */
  New_Max_Min( max, min, subdiv_val, nval );

  return subdiv_order;
} /* Fit_to_Scale() */

/* Fit_to_Scale2()
 *
 * Adjust the max and min value of data to be plotted,
 * as well as the number of scale sub-divisions, so that
 * sub-division values are easier to interpolate between.
 * This is done for two scales (left & right) simultaneously.
 * The chosen scale values are 10, 10/2, 10/4, 10/5 and 1.
 */
  static void
Fit_to_Scale2( double *max1, double *min1,
    double *max2, double *min2, int *nval )
{
  // Acceptable scale values (10/10, 10/5, 10/4, 10/2) 
  // Intermediate values are geometric mean of pairs 
  double scale_val[] = { 10.0, 5.0, 2.5, 2.0, 1.0, 0.5 };

  double subdiv_val1, subdiv_order1, subdiv_val2, subdiv_order2;
  double max_1, min_1, max_2, min_2, range1, range2, min_stretch;
  double max1sv=0.0, min1sv=0.0, max2sv=0.0, min2sv=0.0;
  int idx1, idx2, nval1, nval2, nvalsv=0, mx, i1, i2;

  // Do nothing in these cases 
  if( *max1 <= *min1 ) return;
  if( *max2 == *min2 ) return;

  // For each scale 
  // Find subdivision's lower order of magnitude 
  subdiv_val1 = (*max1 - *min1) / (double)(*nval-1);
  subdiv_order1 = 1.0;
  while( subdiv_order1 < subdiv_val1 )
    subdiv_order1 *= 10.0;
  while( subdiv_order1 > subdiv_val1 )
    subdiv_order1 /= 10.0;

  // Scale subdivision 1 < subd < 10 
  subdiv_val1 /= subdiv_order1;

  // Find nearest prefered subdiv value 
  idx1 = 1;
  while( (scale_val[idx1] > subdiv_val1) && (idx1 <= 4) )
    idx1++;

  // Find subdivision's lower order of magnitude 
  subdiv_val2 = (*max2 - *min2) / (double)(*nval-1);
  subdiv_order2 = 1.0;
  while( subdiv_order2 < subdiv_val2 )
    subdiv_order2 *= 10.0;
  while( subdiv_order2 > subdiv_val2 )
    subdiv_order2 /= 10.0;

  // Scale subdivision 1 < subd < 10 
  subdiv_val2 /= subdiv_order2;

  // Find nearest prefered subdiv value 
  idx2 = 1;
  while( (scale_val[idx2] > subdiv_val2) && (idx2 <= 4) )
    idx2++;

  // Search for a compromize in scale stretching 
  range1 = *max1 - *min1;
  range2 = *max2 - *min2;
  min_stretch = 10.0;

  // Scale prefered subdiv values 
  subdiv_val1 = scale_val[idx1] * subdiv_order1;
  subdiv_val2 = scale_val[idx2] * subdiv_order2;

  // Recalculate new max and min values 
  max_1 = *max1; min_1 = *min1; nval1 = *nval;
  max_2 = *max2; min_2 = *min2; nval2 = *nval;
  New_Max_Min( &max_1, &min_1, subdiv_val1, &nval1 );
  New_Max_Min( &max_2, &min_2, subdiv_val2, &nval2 );

  /* This is a lucky case */
  if( (nval1 == nval2) && (nval1 >= *nval) )
  {
    *max1 = max_1; *min1 = min_1;
    *max2 = max_2; *min2 = min_2;
    *nval = nval1;
    return;
  }

  /* More likely look for a compromise */
  for( i1 = 0; i1 < 2; i1++ )
    for( i2 = 0; i2 < 2; i2++ )
{
      double stretch;

      /* Scale prefered subdiv values */
      subdiv_val1 = scale_val[idx1-i1] * subdiv_order1;
      subdiv_val2 = scale_val[idx2-i2] * subdiv_order2;

      /* Recalculate new max and min values */
      max_1 = *max1; min_1 = *min1; nval1 = *nval;
      max_2 = *max2; min_2 = *min2; nval2 = *nval;
      New_Max_Min( &max_1, &min_1, subdiv_val1, &nval1 );
      New_Max_Min( &max_2, &min_2, subdiv_val2, &nval2 );

      /* This is a lucky case */
      if( nval1 == nval2 )
      {
        *max1 = max_1; *min1 = min_1;
        *max2 = max_2; *min2 = min_2;
        *nval = nval1;
        return;
      }

      /* Stretch scale with the fewer steps */
      if( nval1 > nval2 )
      {
        mx = nval1 - nval2;
        max_2 += ((mx+1)/2) * subdiv_val2;
        min_2 -= (mx/2) * subdiv_val2;
        stretch = (max_2-min_2)/range2;
        if( (stretch < min_stretch) )
        {
          min_stretch = stretch;
          max2sv = max_2; min2sv = min_2;
          max1sv = max_1; min1sv = min_1;
          nvalsv = nval1;
}
      }
      else
      {
        mx = nval2 - nval1;
        max_1 += ((mx+1)/2) * subdiv_val1;
        min_1 -= (mx/2) * subdiv_val1;
        stretch = (max_1-min_1)/range1;
        if( (stretch < min_stretch) )
        {
          min_stretch = stretch;
          max1sv = max_1; min1sv = min_1;
          max2sv = max_2; min2sv = min_2;
          nvalsv = nval2;
        }
      }

    } /* for( i1 = 0; i1 < 3; i1++ ) */

  *max1 = max1sv; *min1 = min1sv;
  *max2 = max2sv; *min2 = min2sv;
  *nval = nvalsv;

} /* Fit_to_Scale2() */

/*-----------------------------------------------------------------------*/

/* Plot_Horizontal_Scale()
 *
 * Draws out a horizontal scale, between the min
 * and max value of the variable to be plotted
 */
  static void
Plot_Horizontal_Scale(
    cairo_t *cr,
    double r, double g, double b,
    int x, int y, int width,
    double max, double min,
    int nval )
{
  int idx, order;
  double hstep = 1.0;
  char value[16], format[8];

  /* Abort if not enough values to plot */
  if( nval <= 1 ) return;

  /* Calculate step between scale values */
  hstep = (max - min) / (nval - 1);

  /* Determine format for scale values */
  /* Use order of horizontal step to determine format of print */
  double ord = log10( fabs(hstep + 0.0000001) );
  order = (int)ord;
  if( order > 0 )  order = 0;
  if( order < -9 ) order = -9;
  snprintf( format, 7, "%%.%df", 1-order );

  /* Draw horizontal scale values */
  for( idx = 0; idx < nval; idx++ )
  {
		int justify = JUSTIFY_CENTER;
		if (idx == 0) justify = JUSTIFY_LEFT;
		if (idx == nval-1) justify = JUSTIFY_RIGHT;

    snprintf( value, sizeof(value), (const char *)format, min );
		draw_text(cr, freqplots_drawingarea,
			x + (idx * width) / (nval-1),
			y,
			value, justify,
			r, g, b, 
			NULL, NULL);

    min += hstep;
  }


} /* Plot_Horizontal_Scale() */

/*-----------------------------------------------------------------------*/

/* Plot_Vertical_Scale()
 *
 * Draws out a vertical scale, between the min
 * and max value of the variable to be plotted
 */
  static void
Plot_Vertical_Scale(
    cairo_t *cr,
    double red, double grn, double blu,
    int x, int y, int height,
    double max, double min,
    int nval )
{
  int idx, yps;
  int min_order, max_order, order;
  double vstep = 1.0;
  char value[16], format[6];

  /* Abort if not enough values to plot */
  if( nval <= 1 ) return;

  /* Calculate step between scale values */
  vstep = (max-min) / (double)(nval-1);

  /* Determine format for scale values */
  /* Find order of magnitude of min and max values */
  if( min != 0.0 )
  {
    double mo = log10( fabs(min) );
    min_order = (int)mo;
  }
  else
    min_order = 0;
  if( max != 0.0 )
  {
    double mo = log10( fabs(max) );
    max_order = (int)mo;
  }
  else max_order = 0;

  /* Use highest order for format */
  order = ( max_order > min_order ? max_order : min_order );
  if( order > 3 ) order = 3;
  if( order < 0 ) order = 0;
  snprintf( format, 6, "%%.%df", (3-order) );

  /* Draw vertical scale values */
  for( idx = 0; idx < nval; idx++ )
  {
    yps = y + (idx * height) / (nval-1);
    snprintf( value, 16, (const char *)format, max );

	// Algin the first value to the top to keep from overlapping the title
	// but otherwise center on the scale line.
	draw_text(cr, freqplots_drawingarea,
		x, yps,
		value, JUSTIFY_RIGHT | JUSTIFY_MIDDLE,
		red, grn, blu, NULL, NULL);
    max -= vstep;
  }

} /* Plot_Vertical_Scale() */

/*-----------------------------------------------------------------------*/


/* Draw_Plotting_Frame()
 *
 * Draws a graph plotting frame, including
 * horizontal and vertical divisions
 */
  static void
Draw_Plotting_Frame(
    cairo_t *cr,
    gchar **title,
    GdkRectangle *rect,
    int nhor, int nvert )
{
  int idx, xpw, xps, yph, yps;

  /* Move to plot box and divisions */
  xpw = rect->x + rect->width;
  yph = rect->y + rect->height;

  /* Draw vertical divisions */
  cairo_set_source_rgb( cr, GREY );
  nvert--;
  for( idx = 1; idx <= nvert; idx++ )
  {
    xps = rect->x + (idx * rect->width) / nvert;
    Cairo_Draw_Line( cr, xps, rect->y, xps, yph );
  }

  /* Draw horizontal divisions */
  nhor--;
  for( idx = 1; idx <= nhor; idx++ )
  {
    yps = rect->y + (idx * rect->height) / nhor;
    Cairo_Draw_Line( cr, rect->x, yps, xpw, yps );
  }

  /* Draw outer box */
  cairo_rectangle(
      cr, rect->x, rect->y, rect->width, rect->height );
  cairo_stroke( cr );

} /* Draw_Plotting_Frame() */

/*-----------------------------------------------------------------------*/

/* Draw_Graph()
 *
 * Plots a graph of a vs b
 */
  static void
Draw_Graph(
    cairo_t *cr,
    double red, double grn, double blu,
    GdkRectangle *rect,
    double *a, double *b,
    double amax, double amin,
    double bmax, double bmin,
    int nval, int side )
{
  double ra;
  int idx;
  GdkPoint *points = NULL, polygn[4];

  /* Cairo context */
  cairo_set_source_rgb( cr, red, grn, blu );

  /* Range of values to plot */
  ra = amax - amin;

  /* Calculate points to plot */
  mem_alloc( (void **) &points, (size_t)calc_data.steps_total * sizeof(GdkPoint),
      "in Draw_Graph()" );
  if( points == NULL )
  {
    fprintf( stderr, _("xnec2c: Draw_Graph():"
        "memory allocation for points failed\n") );
    Stop( _("Draw_Graph():"
          "Memory allocation for points failed"), ERR_OK );
    return;
  }

  for( idx = 0; idx < nval; idx++ )
  {
    points[idx].x = rect->x + (int)((double)rect->width * idx/(nval-1) + 0.5);
    points[idx].y = rect->y + (int)( (double)rect->height *
        (amax-a[idx]) / ra + 0.5 );

    /* Plot a small rectangle (left scale) or polygon (right scale) at point */
    if( side == LEFT )
    {
      cairo_rectangle( cr,
          (double)(points[idx].x-3), (double)(points[idx].y-3),
          6.0, 6.0 );
      cairo_fill( cr );
    }
    else
    {
      polygn[0].x = points[idx].x-4; polygn[0].y = points[idx].y;
      polygn[1].x = points[idx].x;   polygn[1].y = points[idx].y+4;
      polygn[2].x = points[idx].x+4; polygn[2].y = points[idx].y;
      polygn[3].x = points[idx].x;   polygn[3].y = points[idx].y-4;
      Cairo_Draw_Polygon( cr, polygn, 4 );
      cairo_fill( cr );
    }
  }

  /* Draw the graph */
  Cairo_Draw_Lines( cr, points, nval );

  free_ptr( (void **)&points );

} /* Draw_Graph() */

/*-----------------------------------------------------------------------*/
/* Plot_Graph()
 *
 * Plots graphs of two functions against a common variable
 *
 * y_left or y_right may be NULL, in which case it is omitted.
 */
  static void
Plot_Graph(
    cairo_t *cr,
    double *y_left, double *y_right, double *x, int nx,
    char *titles[], int posn)
{
	// Pointer to the FR card's plot_rect
	GdkRectangle *plot_rect = NULL;

	// Min/max values for the data series
	double max_y_left, min_y_left, max_y_right, min_y_right;

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

	// Get the pixel size of the scale text on left and right of the graph.
	pango_text_size(freqplots_drawingarea, 
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
	px_per_vert_scale = pad_x_scale_text*1.5;

	// Space between horizontal scale lines down the Y axis:
	px_per_horiz_scale = pad_y_bottom_scale_text*3;

	// Title text is the same as the scale text, but change this
	// if the title font-size ever changes!
	pad_y_title_text   = pad_y_bottom_scale_text;

	/* Available height for each graph.
	* (calc_data.ngraph is the number of graphs to be plotted) */
	plot_rect_height = 
		freqplots_height / calc_data.ngraph - (
			pad_y_title_text +
			pad_y_px_above_scale +
			pad_y_bottom_scale_text
		);

	// Scales start at the same value, may be modified below.
	// The number of horizontal scale lines (n_horiz_scale) is
	// calculated here. (But n_vert_scale is calculated below in
	// the FR card loop because it can change with the plot size.)
	n_horiz_scale = plot_rect_height / px_per_horiz_scale;

	// Calculate the entire available width for plotting:
	width_available = 
		(freqplots_width - (
			// 2x, one for each side, left and right:
			2*pad_x_scale_text +
			2*pad_x_px_after_scale +

			// One less space between graphs than there
			// are graphs:
			(pad_x_between_graphs*(calc_data.FR_cards-1))
		));


	/* Draw titles */
	plot_rect_y = (freqplots_height * posn) / calc_data.ngraph;

	draw_text(cr, freqplots_drawingarea, 
		pad_x_scale_text+pad_x_px_after_scale,
		plot_rect_y,
		titles[0], JUSTIFY_LEFT, MAGENTA,
		NULL, NULL);

	draw_text(cr, freqplots_drawingarea, 
		freqplots_width/2,
		plot_rect_y,
		titles[1], JUSTIFY_CENTER, YELLOW,
		NULL, NULL);

	draw_text(cr, freqplots_drawingarea, 
		freqplots_width - (pad_x_scale_text+pad_x_px_after_scale),
		plot_rect_y,
		titles[2], JUSTIFY_RIGHT, CYAN,
		NULL, NULL);

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
			cr,
			MAGENTA,
			pad_x_scale_text, plot_rect_y,
			plot_rect_height,
			max_y_left, min_y_left, n_horiz_scale);

	if (y_right != NULL)
		Plot_Vertical_Scale(
			cr,
			CYAN,
			freqplots_width-pad_x_px_after_scale,
			plot_rect_y,
			plot_rect_height,
			max_y_right, min_y_right, n_horiz_scale);


	// Plot FR cards:
	int fr, x_offset = 0, offset = 0;

	x_offset = pad_x_scale_text	+ pad_x_px_after_scale;
	for (fr = 0; fr < calc_data.FR_cards; fr++)
	{
		fr_plot_t *fr_plot = get_fr_plot(posn, fr);
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
		
		// Resize and sync plots if the window size changed.
		if (prev_width_available != width_available) {
			plot_rect->width = width_available / calc_data.FR_cards;
			fr_plot_sync_widths(fr_plot);
		}

		n_vert_scale = plot_rect->width / px_per_vert_scale;


		// Offset is the offset into the value arrays: 
		// Break if there are no more values to plot because
		// it has plotted all of them or the values are still
		// being calculated:
		int maxidx = calc_data.freq_step - offset;
		if (maxidx <= 0)
			break;

		// Clamp the number of index to be plotted if there
		// are some available to plot in the next FR card which
		// will be done in the next iteration of this loop:
		if (maxidx > fr_plot->freq_loop_data->freq_steps)
			maxidx = fr_plot->freq_loop_data->freq_steps;

		/* Draw plotting frame */
		double min_fscale = fr_plot->freq_loop_data->min_freq;
		double max_fscale = fr_plot->freq_loop_data->max_freq;

		Draw_Plotting_Frame( cr, titles,
			plot_rect, n_horiz_scale, n_vert_scale);

		Plot_Horizontal_Scale(
			cr,
			YELLOW,
			plot_rect->x,
			plot_rect->y + plot_rect->height,
			plot_rect->width,
			max_fscale, min_fscale, n_vert_scale);

		if (y_left != NULL)
		{
			Draw_Graph(
				cr,
				MAGENTA,
				plot_rect,
				y_left+offset, x+offset,
				max_y_left, min_y_left,
				max_fscale, min_fscale,
				maxidx,
				LEFT );
		}

		if (y_right != NULL)
		{
			Draw_Graph(
				cr,
				CYAN,
				plot_rect,
				y_right+offset, x+offset,
				max_y_right, min_y_right,
				max_fscale, min_fscale,
				maxidx,
				RIGHT);
		}

		/* Draw a vertical line to show current freq if it was
		* changed by a user click on the plots drawingarea */
		if( isFlagSet(FREQ_LOOP_DONE) && isFlagSet(PLOT_FREQ_LINE)
			&& calc_data.freq_mhz >= min_fscale 
			&& calc_data.freq_mhz <= max_fscale 
			)
		{
			double freq_x;

			freq_x = (calc_data.freq_mhz - min_fscale) / (max_fscale - min_fscale);
			freq_x *= plot_rect->width;

			cairo_set_source_rgb( cr, GREEN );
			Cairo_Draw_Line(cr,
				plot_rect->x+freq_x, plot_rect->y,
				plot_rect->x+freq_x, plot_rect->y+plot_rect->height);
	}

		// Next FR card index:
		x_offset += plot_rect->width + pad_x_between_graphs;
		offset += fr_plot->freq_loop_data->freq_steps;
	}

	prev_width_available = width_available;

}


  static void
Calculate_Smith( double zr, double zi, double z0, double *re, double *im )
{
  complex double z = zr / z0 + I * zi / z0;
  complex double r = ( z - 1.0 ) / ( z + 1.0 );
  *re = creal( r );
  *im = cimag( r );
}

/*-----------------------------------------------------------------------*/

/* Plot_Graph_Smith()
 *
 * Plots graphs of two functions against a common variable
 */
  static void
Plot_Graph_Smith(
    cairo_t *cr,
    double *fa, double *fb, double *fc,
    int nc, int posn )
{
  int plot_height, plot_y_position;
  int idx;
  GdkPoint *points = NULL;
  int scale, x0, y0, x, y, xpw;
  double re, im;

  GdkRectangle plot_rect;

  /* Pango layout size */
  static int layout_width, layout_height, width1, height;

  pango_text_size(freqplots_drawingarea, &layout_width, &layout_height, "000000");

  /* Available height for each graph.
   * (np=number of graphs to be plotted) */
  plot_height = freqplots_height / calc_data.ngraph;
  plot_y_position   = ( freqplots_height * posn) / calc_data.ngraph;

  /* Plot box rectangle */
  plot_rect.x = layout_width + 4;
  plot_rect.y = plot_y_position + 2;
  plot_rect.width = freqplots_width - 8 - 2 * layout_width;
  plot_rect.height = plot_height - 8 - 2 * layout_height;

  cairo_set_source_rgb( cr, YELLOW );
  pango_text_size(freqplots_drawingarea, &width1, &height, _("Smith Chart") );
  xpw = plot_rect.x + ( plot_rect.width - width1 ) / 2;
  cairo_move_to( cr, xpw, plot_rect.y );
  plot_rect.y += height;

  x0 = plot_rect.x + plot_rect.width  / 2;
  y0 = plot_rect.y + plot_rect.height / 2;
  scale = plot_rect.width;
  if( scale > plot_rect.height )
      scale = plot_rect.height;
  cairo_move_to( cr, x0 - scale / 2, y0 );

  /* Draw smith background */
  cairo_set_source_rgb( cr, GREY );
  for( idx = 0; idx < 3; idx++ )
  {
       int div = 2 << idx;
       double angle[] = { 1.0, 1.2, 1.35 };
       cairo_arc (cr, x0 + scale / 2 - scale / div, y0, scale / div, 0, M_2PI);
       cairo_arc (cr, x0 + scale / 2, y0 - scale / div,
           scale / div, M_PI / 2.0, M_PI * angle[idx] );
       cairo_move_to( cr, x0 + scale / 2, y0 );
       cairo_arc_negative( cr, x0 + scale / 2, y0 + scale / div,
           scale / div, M_PI * 1.5, ( 2.0 - angle[idx]) * M_PI );
       cairo_move_to( cr, x0 + scale / 2, y0 );
  }

  cairo_stroke( cr );
  //g_object_unref( layout );

  /* Cairo context */
  cairo_set_source_rgb( cr, MAGENTA );

  /* Calculate points to plot */
  mem_alloc( (void **)&points, (size_t)calc_data.steps_total * sizeof(GdkPoint),
      "in Draw_Graph()" );

  if( points == NULL )
  {
    fprintf( stderr, _("xnec2c: Draw_Graph():"
        "memory allocation for points failed\n") );
    Stop( _("Draw_Graph():"
          "Memory allocation for points failed"), ERR_OK );
    return;
  }

  for( idx = 0; idx < nc; idx++ )
  {
    Calculate_Smith( fa[idx], fb[idx], calc_data.zo, &re, &im );
    points[idx].x = x0 + (gint)( re * scale / 2 );
    points[idx].y = y0 + (gint)( im * scale / 2 );
        cairo_rectangle( cr,
            (double)(points[idx].x - 3),
            (double)(points[idx].y - 3), 6.0, 6.0 );
    cairo_fill( cr );
  }

  /* Draw the graph */
  Cairo_Draw_Lines( cr, points, nc );
  free_ptr( (void **)&points );

  /* Draw a vertical line to show current freq if it was
   * changed by a user click on the plots drawingarea */
  if( isFlagSet(FREQ_LOOP_DONE) && isFlagSet(PLOT_FREQ_LINE) )
  {
    cairo_set_source_rgb( cr, GREEN );
    Calculate_Smith( creal(netcx.zped), cimag(netcx.zped), calc_data.zo, &re, &im );
    x = x0 + (gint)( re * scale / 2 );
    y = y0 + (gint)( im * scale / 2 );
    cairo_rectangle( cr, x - 4, y - 4, 8.0, 8.0 );
    cairo_fill( cr );
  }

} /* Plot_Graph_Smith() */

/*-----------------------------------------------------------------------*/

/* Plot_Frequency_Data()
 *
 * Plots a graph of frequency-dependent parameters
 * (gain, vswr, imoedance etc) against frequency
 */
  void
Plot_Frequency_Data( cairo_t *cr )
{
  /* Abort plotting if main window is to be closed
   * or when plots drawing area not available */
  if( isFlagSet(MAIN_QUIT) ||
      isFlagClear(PLOT_ENABLED) ||
      isFlagClear(ENABLE_EXCITN) )
    return;

  /* Titles for plots */
  char *titles[3];

  int idx,
    posn,  /* Position num of plot in drawingarea */
    fstep; /* Freq step number */

  static double
    *gmax     = NULL, /* Max gain buffer */
    *vgain    = NULL, /* Viewer direction gain buffer */
    *netgain  = NULL, /* Viewer direction net gain buffer */
    *gdir_tht = NULL, /* Direction in theta of gain */
    *gdir_phi = NULL, /* Direction in phi of gain */
    *fbratio  = NULL; /* Direction in phi of gain */

  /* Used to calculate net gain */
  double Zr, Zo, Zi;

  /* 2d array of plot rectangles popluated by the Plot_Graph function */
  if (calc_data.ngraph > 0 && calc_data.FR_cards > 0)
  mem_realloc((void**)&fr_plots,
	sizeof(fr_plot_t) * calc_data.ngraph * calc_data.FR_cards,
		__LOCATION__); 
  else // nothing to do here...
	  return;


  for (idx = 0; idx < calc_data.ngraph * calc_data.FR_cards; idx++)
  {
	  if (FR_PLOT_T_IS_VALID(&fr_plots[idx]))
		  continue;

	  // Set the plot position
	  fr_plots[idx].posn = idx / calc_data.FR_cards;

	  // Point to the freq loop data
	  fr_plots[idx].fr = idx % calc_data.FR_cards;
	  fr_plots[idx].freq_loop_data = &calc_data.freq_loop_data[fr_plots[idx].fr];

      // zero the plot_rect, Plot_Graph() will fill it in.
	  memset(&fr_plots[idx].plot_rect, 0, sizeof(GdkRectangle));

      // Set it as valid:
	  fr_plots[idx].valid = FR_PLOT_T_MAGIC;
  }

  /* Clear drawingarea */
  cairo_set_source_rgb( cr, BLACK );
  cairo_rectangle(
      cr, 0.0, 0.0,
      (double)freqplots_width,
      (double)freqplots_height );
  cairo_fill( cr );

  /* Abort if plotting is not possible FIXME */
  if( (calc_data.freq_step < 1) || isFlagClear(FREQ_LOOP_READY) ||
      (isFlagClear(FREQ_LOOP_RUNNING) && isFlagClear(FREQ_LOOP_DONE)) ||

      (isFlagClear(PLOT_GMAX)         &&
       isFlagClear(PLOT_GVIEWER)      &&
       isFlagClear(PLOT_VSWR)         &&
       isFlagClear(PLOT_ZREAL_ZIMAG)  &&
       isFlagClear(PLOT_ZMAG_ZPHASE)  &&
       isFlagClear(PLOT_SMITH)) )
  {
    return;
  }

  /* Graph position */
  posn = 0;

  /* Limit freq stepping to last freq step */
  fstep = calc_data.last_step + 1;

  /* Plot max gain vs frequency, if possible */
  if( isFlagSet(PLOT_GMAX) && isFlagSet(ENABLE_RDPAT) )
  {
    int nth, nph, pol;
    gboolean no_fbr;

    /* Allocate max gmax and directions */
    size_t mreq = (size_t)fstep * sizeof(double);
    mem_realloc( (void **)&gmax,     mreq, "in plot_freqdata.c" );
    mem_realloc( (void **)&gdir_tht, mreq, "in plot_freqdata.c" );
    mem_realloc( (void **)&gdir_phi, mreq, "in plot_freqdata.c" );
    mem_realloc( (void **)&fbratio,  mreq, "in plot_freqdata.c" );
    if( isFlagSet(PLOT_NETGAIN) )
      mem_realloc( (void **)&netgain, mreq, "in plot_freqdata.c" );

    /* Find max gain and direction, F/B ratio */
    no_fbr = FALSE;

    /* Polarization type and impedance */
    pol = calc_data.pol_type;
    Zo = calc_data.zo;

    /* When freq loop is done, calcs are done for all freq steps */
    for( idx = 0; idx < fstep; idx++ )
    {
      double fbdir;
      int fbidx, mgidx;

      /* Index to gtot buffer where max gain
       * occurs for given polarization type */
      mgidx = rad_pattern[idx].max_gain_idx[pol];

      /* Max gain for given polarization type */
      gmax[idx] = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);

      /* Net gain if selected */
      if( isFlagSet(PLOT_NETGAIN) )
      {
        Zr = impedance_data.zreal[idx];
        Zi = impedance_data.zimag[idx];
        netgain[idx] = gmax[idx] +
          10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );
      }

      /* Radiation angle/phi where max gain occurs */
      gdir_tht[idx] = 90.0 - rad_pattern[idx].max_gain_tht[pol];
      gdir_phi[idx] = rad_pattern[idx].max_gain_phi[pol];

      /* Find F/B ratio if possible or net gain not required */
      if( no_fbr || isFlagSet(PLOT_NETGAIN) )
        continue;

      /* Find F/B direction in theta */
      fbdir = 180.0 - rad_pattern[idx].max_gain_tht[pol];
      if( fpat.dth == 0.0 ) nth = 0;
      else nth = (int)( fbdir/fpat.dth + 0.5 );

      /* If the antenna is modelled over ground, then use the same
       * theta as the max gain direction, relying on phi alone to
       * take us to the back. Patch supplied by Rik van Riel AB1KW
       */
      if( (nth >= fpat.nth) || (nth < 0) )
      {
        fbdir = rad_pattern[idx].max_gain_tht[pol];
        if( fpat.dth == 0.0 )
          nth = 0;
        else
          nth = (int)( fbdir / fpat.dth + 0.5 );
      }

      /* Find F/B direction in phi */
      fbdir = gdir_phi[idx] + 180.0;
      if( fbdir >= 360.0 ) fbdir -= 360.0;
      nph = (int)( fbdir/fpat.dph + 0.5 );

      /* No F/B calc. possible if no phi step at +180 from max gain */
      if( (nph >= fpat.nph) || (nph < 0) )
      {
        no_fbr = TRUE;
        continue;
      }

      /* Index to gtot buffer for gain in back direction */
      fbidx = nth + nph*fpat.nth;

      /* Front to back ratio */
      fbratio[idx]  = pow( 10.0, gmax[idx] / 10.0 );
      fbratio[idx] /= pow( 10.0,
          (rad_pattern[idx].gtot[fbidx] + Polarization_Factor(pol, idx, fbidx)) / 10.0 );
      fbratio[idx] = 10.0 * log10( fbratio[idx] );
      rad_pattern[idx].fbratio = fbratio[idx];
    } /* for( idx = 0; idx < fstep; idx++ ) */

    /*** Plot gain and f/b ratio (if possible) graph(s) */
    if( no_fbr || isFlagSet(PLOT_NETGAIN) )
    {
      /* Plotting frame titles */
      titles[0] = _("Raw Gain dbi");
      if( isFlagSet(PLOT_NETGAIN) )
      {
        titles[1] = _("AA Max Gain & Net Gain vs Frequency");
        titles[2] = _("Net Gain dbi");
        if( fstep > 1 )
          Plot_Graph( cr,
			  gmax, netgain,
			  save.freq,
			  fstep,
              titles, posn++ );
      }
      else
      {
        titles[1] = _("BB Max Gain & F/B Ratio vs Frequency");
        titles[2] = "        ";
        if( fstep > 1 )
          Plot_Graph( cr, gmax, NULL, save.freq, fstep,
              titles, posn++ );
      }
    }
    else
    {
      /* Plotting frame titles */
      titles[0] = _("Raw Gain dbi");
      titles[1] = _("CC Max Gain & F/B Ratio vs Frequency");
      titles[2] = _("F/B Ratio db");
      if( fstep > 1 )
        Plot_Graph( cr, gmax, fbratio, save.freq, fstep,
            titles, posn++ );
    }

    /* Plot max gain direction if enabled */
    if( isFlagSet(PLOT_GAIN_DIR) )
    {
      /* Plotting frame titles */
      titles[0] = _("Rad Angle - deg");
      titles[1] = _("DD Max Gain Direction vs Frequency");
      titles[2] = _("Phi - deg");
      if( fstep > 1 )
        Plot_Graph( cr, gdir_tht, gdir_phi, save.freq, fstep,
            titles, posn++ );
    }

  } /* if( isFlagSet(PLOT_GMAX) && isFlagSet(ENABLE_RDPAT) ) */

  /* Plot gain in direction of viewer vs freq, if possible */
  if( isFlagSet(PLOT_GVIEWER) && isFlagSet(ENABLE_RDPAT) )
  {
    /* Plotting frame titles */
    titles[0] = _("Raw Gain dbi");
    titles[1] = _("Gain in Viewer Direction vs Frequency");

    /* Allocate viewer gain buffer */
    size_t mreq = (size_t)fstep * sizeof(double);
    mem_realloc( (void **)&vgain, mreq, "in plot_freqdata.c" );

    /* Calcs are done for all freq steps */
    for( idx = 0; idx < fstep; idx++ )
      vgain[idx] = Viewer_Gain( structure_proj_params, idx );

    /* Plot net gain if selected */
    if( isFlagSet(PLOT_NETGAIN) )
    {
      mreq = (size_t)fstep * sizeof(double);
      mem_realloc( (void **)&netgain, mreq, "in plot_freqdata.c" );

      Zo = calc_data.zo;
      for( idx = 0; idx < fstep; idx++ )
      {
        Zr = impedance_data.zreal[idx];
        Zi = impedance_data.zimag[idx];
        netgain[idx] = vgain[idx] +
          10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0 ) + pow(Zi, 2.0)) );
      }

      /* Plot net gain if selected */
      titles[2] = _("Net gain dbi");
      if( fstep > 1 )
        Plot_Graph( cr, vgain, netgain, save.freq, fstep,
            titles, posn++ );
    } /* if( isFlagSet(PLOT_NETGAIN) ) */
    else
    {
      titles[2] = "        ";
      if( fstep > 1 )
        Plot_Graph( cr, vgain, NULL, save.freq, fstep,
            titles, posn++ );
    }
  } /* isFlagSet(PLOT_GVIEWER) && isFlagSet(ENABLE_RDPAT) */

  /* Plot VSWR vs freq */
  if( isFlagSet(PLOT_VSWR) )
  {
    double *vswr = NULL, gamma;
    double zrpro2, zrmro2, zimag2;

    /* Plotting frame titles */
    titles[0] = _("VSWR");
    titles[1] = _("VSWR vs Frequency");

    /* Calculate VSWR */
    mem_alloc( (void **) &vswr, (size_t)calc_data.steps_total * sizeof(double),
        "in Plot_Frequency_Data()" );
    if( vswr == NULL )
    {
      fprintf( stderr, _("xnec2c: Plot_Frequency_Data():"
          "memory allocation for vswr failed\n") );
      Stop( _("Plot_Frequency_Data():"
            "Memory allocation for vswr failed"), ERR_OK );
      return;
    }

    for(idx = 0; idx < fstep; idx++ )
    {
      zrpro2 = impedance_data.zreal[idx] + calc_data.zo;
      zrpro2 *= zrpro2;
      zrmro2 = impedance_data.zreal[idx] - calc_data.zo;
      zrmro2 *= zrmro2;
      zimag2 = impedance_data.zimag[idx] * impedance_data.zimag[idx];
      gamma = sqrt( (zrmro2 + zimag2) / (zrpro2 + zimag2) );
      vswr[idx] = (1.0 + gamma) / (1.0 - gamma);
      if( vswr[idx] > 10.0 ) vswr[idx] = 10.0;
    }

    titles[2] = "        ";
    if( fstep > 1 )
      Plot_Graph( cr, vswr, NULL, save.freq, fstep,
          titles, posn++ );

    free_ptr( (void **)&vswr );
  } /* if( isFlagSet(PLOT_VSWR) ) */

  /* Plot z-real and z-imag */
  if( isFlagSet(PLOT_ZREAL_ZIMAG) )
  {
    /* Plotting frame titles */
    titles[0] = _("Z-real");
    titles[1] = _("Impedance vs Frequency");
    titles[2] = _("Z-imag");
    if( fstep > 1 )
      Plot_Graph( cr,
          impedance_data.zreal, impedance_data.zimag, save.freq,
          fstep, titles, posn++ );

  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  /* Plot z-magn and z-phase */
  if( isFlagSet(PLOT_ZMAG_ZPHASE) )
  {
    /* Plotting frame titles */
    titles[0] = _("Z-magn");
    titles[1] = _("Impedance vs Frequency");
    titles[2] = _("Z-phase");
    if( fstep > 1 )
      Plot_Graph( cr, impedance_data.zmagn, impedance_data.zphase,
          save.freq, fstep, titles, posn++ );

  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  /* Plot smith chart */
  if( isFlagSet(PLOT_SMITH) )
  {
    if( fstep > 1 )
      Plot_Graph_Smith( cr,
          impedance_data.zreal, impedance_data.zimag, save.freq,
		  fstep, posn++ );

  } /* if( isFlagSet(PLOT_SMITH) ) */

  /* Display freq data in entry widgets */
  Display_Frequency_Data();

  /* Wait for GTK to complete its tasks */
  //while( g_main_context_iteration(NULL, FALSE) );

} /* Plot_Frequency_Data() */

/*-----------------------------------------------------------------------*/

/* Plots_Window_Killed()
 *
 * Cleans up after the plots window is closed
 */
  void
Plots_Window_Killed( void )
{
  rc_config.freqplots_width  = 0;
  rc_config.freqplots_height = 0;

  // Reset this for next time otherwise width_available rescales in Plot_Graph will not work.
  prev_width_available = 0;

  if( isFlagSet(PLOT_ENABLED) )
  {
    ClearFlag( PLOT_FLAGS );
    freqplots_drawingarea = NULL;
    g_object_unref( freqplots_window_builder );
    freqplots_window_builder = NULL;

    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(
          Builder_Get_Object(main_window_builder, "main_freqplots")), FALSE );
  }
  freqplots_window = NULL;
  kill_window = NULL;

  if (fr_plots != NULL)
  {
	  free(fr_plots);
	  fr_plots = NULL;
  }

} /* Plots_Window_Killed() */

/*-----------------------------------------------------------------------*/

/* Set_Frequecy_On_Click()
 *
 * Sets the current freq after click by user on plots drawingarea
 */
  void
Set_Frequency_On_Click( GdkEvent *e)
{
  double fmhz = 0.0;
  double x, w;
  int button, i;

  GdkEventButton *button_event = (GdkEventButton *)e;
  GdkEventScroll *scroll_event = (GdkEventScroll *)e;
  GdkEventMotion *motion_event = (GdkEventMotion *)e;

  // fr_plot: the plot where the mouse hovered:
  fr_plot_t *fr_plot = NULL;

  // fr_adj: the plot to adjust when using the mouse wheel:
  fr_plot_t *fr_adj = NULL;


  if( isFlagClear(FREQ_LOOP_DONE) )
    return;

  // find the fr_plot structure that the mouse is within:
  for (i = 0; i < calc_data.ngraph * calc_data.FR_cards; i++)
  {
	if (   button_event->x >= fr_plots[i].plot_rect.x
		&& button_event->x <= fr_plots[i].plot_rect.x + fr_plots[i].plot_rect.width
		&& button_event->y >= fr_plots[i].plot_rect.y 
		&& button_event->y <= fr_plots[i].plot_rect.y + fr_plots[i].plot_rect.height)
	{
		fr_plot = &fr_plots[i];
		break;
	}

  }

  if (fr_plot == NULL)
  {
	  // no plot_rect selected for frequency line
	  return;
  }

  double min_fscale = fr_plot->freq_loop_data->min_freq;
  double max_fscale = fr_plot->freq_loop_data->max_freq;

  /* Width of plot bounding rectangle */
  w = fr_plot->plot_rect.width;

  /* 'x' posn of click refered to plot bounding rectangle's 'x' */
  x = button_event->x - fr_plot->plot_rect.x;
  if( x < 0.0 ) x = 0.0;
  else if( x > w ) x = w;

  printf("mouse click[%f,%f button=%d]: ", button_event->x, button_event->y, button_event->button);
  print_fr_plot(fr_plot);

  button = button_event->button;

  // Support holding the button down to drag the green line:
  if (motion_event->state & GDK_BUTTON1_MASK) button = 1;
  if (motion_event->state & GDK_BUTTON2_MASK) button = 2;
  if (motion_event->state & GDK_BUTTON3_MASK) button = 3;

  /* Set freq corresponding to click 'x', to freq spinbuttons FIXME */
  switch( button )
  {
    case 1: /* Calculate frequency corresponding to mouse position in plot */
      /* Enable drawing of frequency line */
      SetFlag( PLOT_FREQ_LINE );

      /* Frequency corresponding to x position of click */
      fmhz = max_fscale - min_fscale;
      fmhz = min_fscale + fmhz * x / w;
      gtk_widget_queue_draw( freqplots_drawingarea );
      break;

    case 2: /* Disable drawing of freq line */
    case 3: 
      ClearFlag( PLOT_FREQ_LINE );
      calc_data.fmhz_save = 0.0;

      /* Wait for GTK to complete its tasks */
      gtk_widget_queue_draw( freqplots_drawingarea );
      return;

	// not a button, is it a scroll?
    case 0: 

		// If its the last plot than shrink/grow the previous:
		if (fr_plot->fr == calc_data.FR_cards-1) 
			fr_adj = get_fr_plot(fr_plot->posn, fr_plot->fr-1);

		// Otherwise shink/grow the next:
		else 
			fr_adj = get_fr_plot(fr_plot->posn, fr_plot->fr+1);

		// Abort if there is only one plot:
		if (fr_adj == NULL)
			return;

		// the amount to adjust on scale:
		int px_adjust = 20;

		if (scroll_event->direction == GDK_SCROLL_UP)
		{
			if (fr_adj->plot_rect.width < 100)
			return;

			fr_adj->plot_rect.width -= px_adjust;
			fr_plot->plot_rect.width += px_adjust;
		}
		else if (scroll_event->direction == GDK_SCROLL_DOWN)
		{
			if (fr_plot->plot_rect.width < 100)
				return;

			fr_adj->plot_rect.width += px_adjust;
			fr_plot->plot_rect.width -= px_adjust;
		}

		// Sync widths for all positions based on fr_plot:
		fr_plot_sync_widths(fr_plot);

        /* Redraw and wait for GTK to complete its tasks */
		gtk_widget_queue_draw( freqplots_drawingarea );
		
		// Just return, we don't want to set fmhz below.
		return;

  } /* switch( button_event->button ) */

  /* Round frequency to nearest 1 kHz */
  int ifmhz = ( fmhz * 1000.0 + 0.5 );
  fmhz = ifmhz / 1000.0;

  /* Save frequency for later use when the graph plots after the NEC2 run */
  calc_data.fmhz_save = fmhz;

  /* Set frequency spinbuttons on new freq */
  if( fmhz != gtk_spin_button_get_value(mainwin_frequency) )
  {
    gtk_spin_button_set_value( mainwin_frequency, fmhz );
    if( isFlagSet(DRAW_ENABLED) )
      gtk_spin_button_set_value( rdpattern_frequency, fmhz );
  }
  else /* Replot data */
  {
    calc_data.freq_mhz = fmhz;
    g_idle_add( Redo_Currents, NULL );
  }

} /* Set_Freq_On_Click() */
/*-----------------------------------------------------------------------*/

