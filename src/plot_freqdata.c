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

/* Graph plot bounding rectangle */
static GdkRectangle plot_rect;

/* Frequency scale max, min, num of values */
static double max_fscale, min_fscale;
static int nval_fscale;

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

  /* Limit freq stepping to nfrq */
  fstep = calc_data.fstep;
  if( fstep >= calc_data.nfrq )
	fstep = calc_data.nfrq;

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
  snprintf( txt, 11, "%9.3f", (double)calc_data.fmhz );
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
  static void
Fit_to_Scale( double *max, double *min, int *nval )
{
  /* Acceptable scale values (10/10, 10/5, 10/4, 10/2) */
  /* Intermediate values are geometric mean of pairs */
  double scale_val[] =
  { 1.0, 1.4142, 2.0, 2.2360, 2.5, 3.5355, 5.0, 7.0710, 10.0, 14.142 };
  double subdiv_val, subdiv_order;
  int idx;

  /* Do nothing in this case */
  if( *max <= *min ) return;

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

} /* Fit_to_Scale() */

/*-----------------------------------------------------------------------*/

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
  /* Acceptable scale values (10/10, 10/5, 10/4, 10/2) */
  /* Intermediate values are geometric mean of pairs */
  double scale_val[] = { 10.0, 5.0, 2.5, 2.0, 1.0, 0.5 };

  double subdiv_val1, subdiv_order1, subdiv_val2, subdiv_order2;
  double max_1, min_1, max_2, min_2, range1, range2, min_stretch;
  double max1sv=0.0, min1sv=0.0, max2sv=0.0, min2sv=0.0;
  int idx1, idx2, nval1, nval2, nvalsv=0, mx, i1, i2;

  /* Do nothing in these cases */
  if( *max1 <= *min1 ) return;
  if( *max2 == *min2 ) return;

  /* For each scale */
  /* Find subdivision's lower order of magnitude */
  subdiv_val1 = (*max1 - *min1) / (double)(*nval-1);
  subdiv_order1 = 1.0;
  while( subdiv_order1 < subdiv_val1 )
	subdiv_order1 *= 10.0;
  while( subdiv_order1 > subdiv_val1 )
	subdiv_order1 /= 10.0;

  /* Scale subdivision 1 < subd < 10 */
  subdiv_val1 /= subdiv_order1;

  /* Find nearest prefered subdiv value */
  idx1 = 1;
  while( (scale_val[idx1] > subdiv_val1) && (idx1 <= 4) )
	idx1++;

  /* Find subdivision's lower order of magnitude */
  subdiv_val2 = (*max2 - *min2) / (double)(*nval-1);
  subdiv_order2 = 1.0;
  while( subdiv_order2 < subdiv_val2 )
	subdiv_order2 *= 10.0;
  while( subdiv_order2 > subdiv_val2 )
	subdiv_order2 /= 10.0;

  /* Scale subdivision 1 < subd < 10 */
  subdiv_val2 /= subdiv_order2;

  /* Find nearest prefered subdiv value */
  idx2 = 1;
  while( (scale_val[idx2] > subdiv_val2) && (idx2 <= 4) )
	idx2++;

  /* Search for a compromize in scale stretching */
  range1 = *max1 - *min1;
  range2 = *max2 - *min2;
  min_stretch = 10.0;

  /* Scale prefered subdiv values */
  subdiv_val1 = scale_val[idx1] * subdiv_order1;
  subdiv_val2 = scale_val[idx2] * subdiv_order2;

  /* Recalculate new max and min values */
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

/* Set_Rectangle()
 *
 * Sets the parameters of a GdkRectangle
 */
  static void
Set_Rectangle( GdkRectangle *rect, int x, int y, int w, int h )
{
  rect->x = x;
  rect->y = y;
  rect->width  = w;
  rect->height = h;
}

/*-----------------------------------------------------------------------*/

/* Plot_Horizontal_Scale()
 *
 * Draws out a horizontal scale, between the min
 * and max value of the variable to be plotted
 */
  static void
Plot_Horizontal_Scale(
	cairo_t *cr,
	double red, double grn, double blu,
	int x, int y, int width,
	double max, double min,
	int nval )
{
  int idx, xps, order;
  double hstep = 1.0;
  char value[16], format[8];
  PangoLayout *layout;
  int pl_width, pl_height; /* Layout size */

  /* Abort if not enough values to plot */
  if( nval <= 1 ) return;

  /* Cairo context */
  cairo_set_source_rgb( cr, red, grn, blu );

  /* Calculate step between scale values */
  hstep = (max - min) / (nval - 1);

  /* Determine format for scale values */
  /* Use order of horizontal step to determine format of print */
  double ord = log10( fabs(hstep + 0.0000001) );
  order = (int)ord;
  if( order > 0 )  order = 0;
  if( order < -9 ) order = -9;
  snprintf( format, 7, "%%6.%df", 1-order );

  /* Create a pango layout */
  layout = gtk_widget_create_pango_layout(
	  freqplots_drawingarea, "1234.5" );
  pango_layout_get_pixel_size( layout, &pl_width, &pl_height);

  /* Draw horizontal scale values */
  /* Align with plot box */
  x -= pl_width/2;
  for( idx = 0; idx < nval; idx++ )
  {
	xps = x + (idx * width) / (nval-1);
	snprintf( value, sizeof(value), (const char *)format, min );
	pango_layout_set_text( layout, value, -1 );
	cairo_move_to( cr, xps, y );
	pango_cairo_show_layout( cr, layout );
	min += hstep;
  }

  g_object_unref( layout );

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
  PangoLayout *layout;
  int pl_width, pl_height; /* Layout size */

  /* Abort if not enough values to plot */
  if( nval <= 1 ) return;

  /* Cairo context */
  cairo_set_source_rgb( cr, red, grn, blu );

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
  snprintf( format, 6, "%%6.%df", (3-order) );

  /* Create a pango layout */
  layout = gtk_widget_create_pango_layout(
	  freqplots_drawingarea, "X" );
  pango_layout_get_pixel_size( layout, &pl_width, &pl_height);

  /* Draw vertical scale values */
  /* Align with plot box */
  y += pl_height/4;
  for( idx = 0; idx < nval; idx++ )
  {
	yps = y + (idx * height) / (nval-1);
	snprintf( value, 16, (const char *)format, max );
	pango_layout_set_text( layout, value, -1 );
	cairo_move_to( cr, x, yps );
	pango_cairo_show_layout( cr, layout );
	max -= vstep;
  }

  g_object_unref( layout );

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
  PangoLayout *layout;
  int width0, width1, width2, height; /* Layout size */

  /* Draw titles (left scale, center and right scale) */
  cairo_set_source_rgb( cr, MAGENTA );
  layout = gtk_widget_create_pango_layout(
	  freqplots_drawingarea, title[0] );
  pango_layout_get_pixel_size( layout, &width0, &height );
  cairo_move_to( cr, rect->x, rect->y );
  pango_cairo_show_layout( cr, layout );

  cairo_set_source_rgb( cr, CYAN );
  pango_layout_set_text( layout, title[2], -1 );
  pango_layout_get_pixel_size( layout, &width2, &height );
  xpw = rect->x + rect->width - width2;
  cairo_move_to( cr, xpw, rect->y );
  pango_cairo_show_layout( cr, layout );

  cairo_set_source_rgb( cr, YELLOW );
  pango_layout_set_text( layout, title[1], -1 );
  pango_layout_get_pixel_size( layout, &width1, &height );
  xpw = rect->x + width0/2 + (rect->width-width1-width2)/2;
  cairo_move_to( cr, xpw, rect->y );
  pango_cairo_show_layout( cr, layout );

  /* Move to plot box and divisions */
  rect->y += height;
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

  /* Draw a vertical line to show current freq if it was
   * changed by a user click on the plots drawingarea */
  if( isFlagSet(FREQ_LOOP_DONE) && isFlagSet(PLOT_FREQ_LINE) )
  {
	double fr;

	fr = ((double)calc_data.fmhz - min_fscale) /
	  (max_fscale - min_fscale);
	fr = fr * (double)rect->width + 0.5;

	cairo_set_source_rgb( cr, GREEN );
	Cairo_Draw_Line( cr, rect->x+(int)fr, rect->y, rect->x+(int)fr, yph );
  }

  g_object_unref( layout );

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
  double ra, rb;
  int idx;
  GdkPoint *points = NULL, polygn[4];

  /* Cairo context */
  cairo_set_source_rgb( cr, red, grn, blu );

  /* Range of values to plot */
  ra = amax - amin;
  rb = bmax - bmin;

  /* Calculate points to plot */
  mem_alloc( (void **)&points,
	  (size_t)calc_data.nfrq * sizeof(GdkPoint),
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
	points[idx].x = rect->x + (int)( (double)rect->width  *
		(b[idx]-bmin) / rb + 0.5 );
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

/* Plot_Graph2()
 *
 * Plots graphs of two functions against a common variable
 */
  static void
Plot_Graph2(
	cairo_t *cr,
	double *fa, double *fb, double *fc, int nc,
	char *titles[], int nplt, int posn )
{
  double max_fa, min_fa, max_fb, min_fb;
  static int first_call = TRUE;
  int idx, nval_ab, plot_height, plot_posn;

  /* Pango layout size */
  static int layout_width, layout_height;

  if( first_call )
  {
	/* Create a pango layout to get scale size */
	PangoLayout *layout;
	layout = gtk_widget_create_pango_layout(
		freqplots_drawingarea, "000000" );
	pango_layout_get_pixel_size( layout,
		&layout_width, &layout_height);
	first_call = FALSE;
	g_object_unref( layout );
  }

  /* Available height for each graph.
   * (np=number of graphs to be plotted) */
  plot_height = freqplots_height/nplt;
  plot_posn   = (freqplots_height * (posn-1))/nplt;

  /* Plot box rectangle */
  Set_Rectangle(
	  &plot_rect,
	  layout_width+4, plot_posn+2,
	  freqplots_width-8 - 2*layout_width,
	  plot_height-8 - 2*layout_height );

  /*** Draw horizontal (freq) scale ***/
  Plot_Horizontal_Scale(
	  cr,
	  YELLOW,
	  layout_width+2,
	  plot_posn+plot_height-2 - layout_height,
	  plot_rect.width,
	  max_fscale, min_fscale, nval_fscale );

  /*** Draw left and right scale ***/
  /* Find max and min of fa */
  max_fa = min_fa = fa[0];
  for( idx = 1; idx < nc; idx++ )
  {
	if( max_fa < fa[idx] )
	  max_fa = fa[idx];
	if( min_fa > fa[idx] )
	  min_fa = fa[idx];
  }

  /* Find max and min of fb */
  max_fb = min_fb = fb[0];
  for( idx = 1; idx < nc; idx++ )
  {
	if( max_fb < fb[idx] )
	  max_fb = fb[idx];
	if( min_fb > fb[idx] )
	  min_fb = fb[idx];
  }

  /* Fit ranges to common scale */
  nval_ab = plot_height / 50;
  Fit_to_Scale2( &max_fa, &min_fa, &max_fb, &min_fb, &nval_ab );

  /* Draw left scale */
  Plot_Vertical_Scale(
	  cr,
	  MAGENTA,
	  2, plot_posn+2,
	  plot_rect.height,
	  max_fa, min_fa, nval_ab );

  /* Draw right scale */
  Plot_Vertical_Scale(
	  cr,
	  CYAN,
	  freqplots_width-2 - layout_width, plot_posn+2,
	  plot_rect.height,
	  max_fb, min_fb, nval_ab );

  /* Draw plotting frame */
  Draw_Plotting_Frame( cr, titles, &plot_rect, nval_ab, nval_fscale );

  /* Draw graph */
  Draw_Graph(
	  cr,
	  MAGENTA,
	  &plot_rect, fa, fc,
	  max_fa, min_fa,
	  max_fscale, min_fscale,
	  nc, LEFT );

  /* Draw graph */
  Draw_Graph(
	  cr,
	  CYAN, &plot_rect,
	  fb, fc,
	  max_fb, min_fb,
	  max_fscale, min_fscale,
	  nc, RIGHT );

} /* Plot_Graph2() */

/*-----------------------------------------------------------------------*/

/* Plot_Graph()
 *
 * Plots graph of a function against a variable
 */
  static void
Plot_Graph(
	cairo_t *cr,
	double *fa, double *fb, int nb,
	char *titles[], int nplt, int posn )
{
  double max_fa, min_fa;
  static int first_call = TRUE;
  int idx, nval_fa, plot_height, plot_posn;
  /* Pango layout size */
  static int layout_width, layout_height;


  if( first_call )
  {
	/* Create a pango layout to get scale size */
	PangoLayout *layout;
	layout = gtk_widget_create_pango_layout(
		freqplots_drawingarea, "000000" );
	pango_layout_get_pixel_size(
		layout, &layout_width, &layout_height);
	first_call = FALSE;
	g_object_unref( layout );
  }

  /* Available height for each graph.
   * (np=number of graphs to be plotted) */
  plot_height = freqplots_height/nplt;
  plot_posn   = (freqplots_height * (posn-1))/nplt;

  /* Plot box rectangle */
  Set_Rectangle(
	  &plot_rect,
	  layout_width + 4, plot_posn+2,
	  freqplots_width-8 - 2*layout_width,
	  plot_height-8 - 2*layout_height );

  /*** Draw horizontal (freq) scale ***/
  Plot_Horizontal_Scale(
	  cr,
	  YELLOW,
	  layout_width+2,
	  plot_posn+plot_height-2 - layout_height,
	  plot_rect.width,
	  max_fscale, min_fscale, nval_fscale );

  /*** Draw left scale ***/
  /* Find max and min of fa */
  max_fa = min_fa = fa[0];
  for( idx = 1; idx < nb; idx++ )
  {
	if( max_fa < fa[idx] )
	  max_fa = fa[idx];
	if( min_fa > fa[idx] )
	  min_fa = fa[idx];
  }

  /* Fit fa range to scale */
  nval_fa = plot_height / 50;
  Fit_to_Scale( &max_fa, &min_fa, &nval_fa );

  /* Draw left scale */
  Plot_Vertical_Scale(
	  cr,
	  MAGENTA,
	  2, plot_posn+2,
	  plot_rect.height,
	  max_fa, min_fa, nval_fa );

  /* Draw plotting frame */
  Draw_Plotting_Frame( cr, titles, &plot_rect, nval_fa, nval_fscale );

  /* Draw graph */
  Draw_Graph(
	  cr,
	  MAGENTA,
	  &plot_rect, fa, fb,
	  max_fa, min_fa,
	  max_fscale, min_fscale,
	  nb, LEFT );

} /* Plot_Graph() */

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
  if(
	  isFlagSet(MAIN_QUIT) ||
	  isFlagClear(PLOT_ENABLED) ||
	  isFlagClear(ENABLE_EXCITN) )
	return;

  /* Titles for plots */
  char *titles[3];

  int
	idx,
	posn,  /* Position num of plot in drawingarea */
	fstep; /* Freq step number */

  static double
	*gmax     = NULL, /* Max gain buffer */
	*vgain    = NULL, /* Viewer direction gain buffer */
	*netgain  = NULL, /* Viewer direction net gain buffer */
	*gdir_tht = NULL, /* Direction in theta of gain */
	*gdir_phi = NULL, /* Direction in phi of gain */
	*fbratio  = NULL; /* Front to back ratio */

  /* Used to calculate net gain */
  double Zr, Zo, Zi;


  /* Clear drawingarea */
  cairo_set_source_rgb( cr, BLACK );
  cairo_rectangle(
	  cr, 0.0, 0.0,
	  (double)freqplots_width,
	  (double)freqplots_height );
  cairo_fill( cr );

  /* Abort if plotting is not possible */
  if( (calc_data.fstep < 1)				||
	  isFlagClear(FREQLOOP_READY)		||
	  (isFlagClear(FREQ_LOOP_RUNNING)	&&
	   isFlagClear(FREQ_LOOP_DONE))		||
	  (isFlagClear(PLOT_GMAX)			&&
	   isFlagClear(PLOT_GVIEWER)		&&
	   isFlagClear(PLOT_VSWR)			&&
	   isFlagClear(PLOT_ZREAL_ZIMAG)	&&
	   isFlagClear(PLOT_ZMAG_ZPHASE)) )
  {
	return;
  }

  /* Fit frequency range to scale */
  min_fscale  = (double)save.freq[0];
  if( isFlagSet(FREQ_LOOP_RUNNING) )
	max_fscale = (double)save.freq[calc_data.fstep];
  else
	max_fscale = (double)save.freq[calc_data.lastf];
  nval_fscale = freqplots_width / 75;
  Fit_to_Scale( &max_fscale, &min_fscale, &nval_fscale );

  /* Graph position */
  posn = 0;

  /* Limit freq stepping to last freq step */
  fstep = calc_data.lastf + 1;

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
	  gmax[idx] =
		rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);

	  /* Net gain if selected */
	  if( isFlagSet(PLOT_NETGAIN) )
	  {
		Zr = impedance_data.zreal[idx];
		Zi = impedance_data.zimag[idx];
		netgain[idx] = gmax[idx] + 10*log10(4*Zr*Zo/(pow(Zr+Zo,2)+pow(Zi,2)));
	  }

	  /* Radiation angle/phi where max gain occurs */
	  gdir_tht[idx] = 90.0 - rad_pattern[idx].max_gain_tht[pol];
	  gdir_phi[idx] = rad_pattern[idx].max_gain_phi[pol];

	  /* Find F/B ratio if possible or net gain not required */
	  if( no_fbr || isFlagSet(PLOT_NETGAIN) )
		continue;

	  /* Find F/B direction in theta */
	  fbdir = 180.0 - rad_pattern[idx].max_gain_tht[pol];
	  if( fpat.dth == 0.0 )	nth = 0;
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
		  nth = (int)( fbdir/fpat.dth + 0.5 );
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
		  (rad_pattern[idx].gtot[fbidx] +
		   Polarization_Factor(pol, idx, fbidx)) / 10.0 );
	  fbratio[idx] = 10.0 * log10( fbratio[idx] );

	} /* for( idx = 0; idx < fstep; idx++ ) */

	/*** Plot gain and f/b ratio (if possible) graph(s) */
	if( no_fbr || isFlagSet(PLOT_NETGAIN) )
	{
	  /* Plotting frame titles */
	  titles[0] = _("Raw Gain dbi");
	  if( isFlagSet(PLOT_NETGAIN) )
	  {
		titles[1] = _("Max Gain & Net Gain vs Frequency");
		titles[2] = _("Net Gain dbi");
		if( fstep > 1 )
		  Plot_Graph2( cr, gmax, netgain, save.freq, fstep,
			  titles, calc_data.ngraph, ++posn );
	  }
	  else
	  {
		titles[1] = _("Max Gain & F/B Ratio vs Frequency");
		titles[2] = "        ";
		if( fstep > 1 )
		  Plot_Graph( cr, gmax, save.freq, fstep,
			  titles, calc_data.ngraph, ++posn );
	  }
	}
	else
	{
	  /* Plotting frame titles */
	  titles[0] = _("Raw Gain dbi");
	  titles[1] = _("Max Gain & F/B Ratio vs Frequency");
	  titles[2] = _("F/B Ratio db");
	  if( fstep > 1 )
		Plot_Graph2( cr, gmax, fbratio, save.freq, fstep,
			titles, calc_data.ngraph, ++posn );
	}

	/* Plot max gain direction if enabled */
	if( isFlagSet(PLOT_GAIN_DIR) )
	{
	  /* Plotting frame titles */
	  titles[0] = _("Rad Angle - deg");
	  titles[1] = _("Max Gain Direction vs Frequency");
	  titles[2] = _("Phi - deg");
	  if( fstep > 1 )
		Plot_Graph2( cr, gdir_tht, gdir_phi, save.freq, fstep,
			titles, calc_data.ngraph, ++posn );
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
		  10*log10(4*Zr*Zo/(pow(Zr+Zo,2)+pow(Zi,2)));
	  }

	  /* Plot net gain if selected */
	  titles[2] = _("Net gain dbi");
	  if( fstep > 1 )
		Plot_Graph2( cr, vgain, netgain, save.freq, fstep,
			titles, calc_data.ngraph, ++posn );
	} /* if( isFlagSet(PLOT_NETGAIN) ) */
	else
	{
	  titles[2] = "        ";
	  if( fstep > 1 )
		Plot_Graph( cr, vgain, save.freq, fstep,
			titles, calc_data.ngraph, ++posn );
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
	mem_alloc( (void **)&vswr,
		(size_t)calc_data.nfrq * sizeof(double),
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
	  gamma = sqrt( (zrmro2 + zimag2)/(zrpro2 + zimag2) );
	  vswr[idx] = (1+gamma)/(1-gamma);
	  if( vswr[idx] > 10.0 ) vswr[idx] = 10.0;
	}

	titles[2] = "        ";
	if( fstep > 1 )
	  Plot_Graph( cr, vswr, save.freq, fstep,
		  titles, calc_data.ngraph, ++posn );

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
	  Plot_Graph2( cr,
		  impedance_data.zreal, impedance_data.zimag, save.freq,
		  fstep, titles, calc_data.ngraph, ++posn );

  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  /* Plot z-magn and z-phase */
  if( isFlagSet(PLOT_ZMAG_ZPHASE) )
  {
	/* Plotting frame titles */
	titles[0] = _("Z-magn");
	titles[1] = _("Impedance vs Frequency");
	titles[2] = _("Z-phase");
	if( fstep > 1 )
	  Plot_Graph2( cr, impedance_data.zmagn, impedance_data.zphase,
		  save.freq, fstep, titles, calc_data.ngraph, ++posn );

  } /* if( isFlagSet(PLOT_ZREAL_ZIMAG) ) */

  /* Display freq data in entry widgets */
  Display_Frequency_Data();

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

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

} /* Plots_Window_Killed() */

/*-----------------------------------------------------------------------*/

/* Set_Frequecy_On_Click()
 *
 * Sets the current freq after click by user on plots drawingarea
 */
  void
Set_Frequency_On_Click( GdkEventButton *event )
{
  gdouble fmhz = 0.0;
  gdouble x, w;
  int idx;


  if( isFlagClear(FREQ_LOOP_DONE) )
	return;

  /* Width of plot bounding rectangle */
  w = (double)plot_rect.width;

  /* 'x' posn of click refered to plot bounding rectangle's 'x' */
  x = event->x - (double)plot_rect.x;
  if( x < 0.0 ) x = 0.0;
  else if( x > w ) x = w;

  /* Set freq corresponding to click 'x', to freq spinbuttons */
  idx = calc_data.lastf;
  switch( event->button )
  {
	case 1: /* Calculate frequency corresponding to mouse position in plot */
	  /* Enable drawing of freq line */
	  SetFlag( PLOT_FREQ_LINE );

	  fmhz = max_fscale - min_fscale;
	  fmhz = min_fscale + fmhz * x/w;
	  break;

	case 2: /* Disable drawing of freq line */
	  ClearFlag( PLOT_FREQ_LINE );
	  /* Wait for GTK to complete its tasks */
	  gtk_widget_queue_draw( freqplots_drawingarea );
	  while( g_main_context_iteration(NULL, FALSE) );
	  return;

	case 3: /* Calculate frequency corresponding to mouse position in plot */
	  /* Enable drawing of freq line */
	  SetFlag( PLOT_FREQ_LINE );

	  fmhz = max_fscale - min_fscale;
	  fmhz = min_fscale + fmhz * x/w;

	  /* Find nearest freq step */
	  idx = (int)( (double)idx * (fmhz - save.freq[0]) /
		  (save.freq[idx] - save.freq[0]) + 0.5 );

	  if( idx > calc_data.lastf )
		idx = calc_data.lastf;
	  else if( idx < 0 ) idx = 0;

	  fmhz = save.freq[idx];

  } /* switch( event->button ) */

  /* Set frequency spinbuttons on new freq */
  if( fmhz != gtk_spin_button_get_value(mainwin_frequency) )
  {
	gtk_spin_button_set_value( mainwin_frequency, fmhz );
	if( isFlagSet(DRAW_ENABLED) )
	  gtk_spin_button_set_value( rdpattern_frequency, fmhz );
  }
  else /* Replot data */
  {
	calc_data.fmhz = (double)fmhz;
	g_idle_add( Redo_Currents, NULL );
  }

} /* Set_Freq_On_Click() */

/*-----------------------------------------------------------------------*/

