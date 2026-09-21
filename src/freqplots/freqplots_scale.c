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
 * freqplots_scale: Axis scale fitting and scale-label rendering.
 *
 * Fit_to_Scale/Fit_to_Scale2 round plot ranges to interpolation-friendly
 * subdivisions; Plot_Horizontal_Scale/Plot_Vertical_Scale defer the
 * resulting scale values as text so labels paint above plot geometry.
 */

#include "freqplots_internal.h"
#include "../shared.h"

#include <math.h>

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
double Fit_to_Scale( double *max, double *min, int *nval )
{
  /* Acceptable scale values (10/10, 10/5, 10/4, 10/2) */
  /* Intermediate values are geometric mean of pairs */
  double scale_val[] =
  { 1.0, 1.4142, 2.0, 2.2360, 2.5, 3.5355, 5.0, 7.0710, 10.0, 14.142 };
  double subdiv_val, subdiv_order;
  int idx;

  /* Do nothing in this case */
  if( *max < *min ) return 1;

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

  /* Find nearest preferred subdiv value */
  for( idx = 1; idx <= 9; idx += 2 )
    if( scale_val[idx] >= subdiv_val )
      break;

  /* Scale preferred subdiv value */
  if( idx > 9 ) idx = 9;
  subdiv_val = scale_val[idx-1] * subdiv_order;

  /* Recalculate new max and min value */
  New_Max_Min( max, min, subdiv_val, nval );

  return subdiv_order;
} /* Fit_to_Scale() */

/* fscale_extent_fit()
 *
 * Derive a panel's display extent from its FR card data range.  Seeds
 * *min / *max from the card's frequency limits, then rounds outward via
 * Fit_to_Scale when the X-axis is rounded or the range is degenerate
 * (single-frequency card).  *nval carries the subdivision count in and
 * out: callers with rendered geometry pass the width-derived count;
 * geometry-free callers pass 0 so Fit_to_Scale applies only the
 * single-point expansion.  This is the lone derivation of the extent
 * stored in fr_plot->min_fscale/max_fscale.
 */
void
fscale_extent_fit( const freq_loop_data_t *fld, int round_x_axis,
    double *min, double *max, int *nval )
{
  *min = fld->min_freq;
  *max = fld->max_freq;

  if( round_x_axis || FREQ_EQ(*min, *max) )
    Fit_to_Scale( max, min, nval );
}

/* Fit_to_Scale2()
 *
 * Adjust the max and min value of data to be plotted,
 * as well as the number of scale sub-divisions, so that
 * sub-division values are easier to interpolate between.
 * This is done for two scales (left & right) simultaneously.
 * The chosen scale values are 10, 10/2, 10/4, 10/5 and 1.
 */
  void
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

  /* Provide a made-up range if max = min */

  // Asymmetric expansion for dual single-point case positions left scale
  // at 2/3 from top and right scale at 1/3 from top to prevent overlap
  if( *max1 == *min1 && *max2 == *min2 )
  {
    double delta1, delta2;

    if( *max1 == 0.0 )
      delta1 = 1.0;
    else
      delta1 = fabs( *max1 ) / 10.0;

    if( *max2 == 0.0 )
      delta2 = 1.0;
    else
      delta2 = fabs( *max2 ) / 10.0;

    *max1 = *min1 + delta1 * (4.0/3.0);
    *min1 = *min1 - delta1 * (2.0/3.0);

    *max2 = *min2 + delta2 * (2.0/3.0);
    *min2 = *min2 - delta2 * (4.0/3.0);
  }
  else if( *max1 == *min1 )
  {
    if( *max1 == 0.0 )
    {
      *max1 =  1.0;
      *min1 = -1.0;
    }
    else
    {
      *max1 += fabs( *max1 ) / 10.0;
      *min1 -= fabs( *min1 ) / 10.0;
    }
  }
  else if( *max2 == *min2 )
  {
    if( *max2 == 0.0 )
    {
      *max2 =  1.0;
      *min2 = -1.0;
    }
    else
    {
      *max2 += fabs( *max2 ) / 10.0;
      *min2 -= fabs( *min2 ) / 10.0;
    }
  }

  // Do nothing in these cases
  if( *max1 < *min1 ) return;
  if( *max2 < *min2 ) return;
  if( *nval < 3 ) return;

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

  // Find nearest preferred subdiv value
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

  // Find nearest preferred subdiv value
  idx2 = 1;
  while( (scale_val[idx2] > subdiv_val2) && (idx2 <= 4) )
    idx2++;

  // Search for a compromise in scale stretching
  range1 = *max1 - *min1;
  range2 = *max2 - *min2;
  min_stretch = 10.0;

  // Scale preferred subdiv values
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

      /* Scale preferred subdiv values */
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
  void
Plot_Horizontal_Scale(
    fp_render_t *fp,
    rgb_f_t c,
    int x, int y, int width,
    double max, double min,
    int nval )
{
  int idx, order;
  double hstep = 1.0;
  char value[16], format[16];

  /* Abort if not enough values to plot */
  if( nval <= 1 ) return;

  /* Calculate step between scale values */
  hstep = (max - min) / (nval - 1);

  /* Determine format for scale values */
  /* Use order of horizontal step to determine format of print */
  double ord = log10( fabs(hstep + 0.0000001) );
  order = (int)ord;

  if (!rc_config.freqplots_round_x_axis)
	  order--;

  if( order > 0 )  order = 0;
  if( order < -9 ) order = -9;

  snprintf( format, 16, "%%.%df", 1-order );

  /* Draw horizontal scale values */
  for( idx = 0; idx < nval; idx++ )
  {
		int justify = JUSTIFY_CENTER;
		if (idx == 0) justify = JUSTIFY_LEFT;
		if (idx == nval-1) justify = JUSTIFY_RIGHT;

		snprintf( value, sizeof(value), (const char *)format, min );
		fp_add_text(fp, x + (idx * width) / (nval - 1), y, 1.0f,
                            value, justify, c);

    min += hstep;
  }


} /* Plot_Horizontal_Scale() */

/*-----------------------------------------------------------------------*/

/* Plot_Vertical_Scale()
 *
 * Draws out a vertical scale, between the min
 * and max value of the variable to be plotted
 */
  void
Plot_Vertical_Scale(
    fp_render_t *fp,
    rgb_f_t c,
    int x, int y, int height,
    double max, double min,
    int nval )
{
  int idx, yps;
  int min_order, max_order, order;
  double vstep = 1.0;
  char value[16], format[16];

  /* only plot min/max if there are not enough values to plot */
  if( nval <= 1 )
    nval = 2;

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
  snprintf( format, 16, "%%.%df", (3-order) );

  /* Draw vertical scale values */
  for( idx = 0; idx < nval; idx++ )
  {
    yps = y + (idx * height) / (nval-1);
    snprintf( value, 16, (const char *)format, max );

	// Algin the first value to the top to keep from overlapping the title
	// but otherwise center on the scale line.
	fp_add_text(fp, x, yps, 1.0f, value, JUSTIFY_RIGHT | JUSTIFY_MIDDLE,
                    c);
    max -= vstep;
  }

} /* Plot_Vertical_Scale() */
