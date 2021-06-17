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

#include "shared.h"

/*------------------------------------------------------------------------*/

/* Writes out frequency-dependent
 * data for the external Optimizer */
  void
Write_Optimizer_Data( void )
{
  char csv_file[FILENAME_LEN];
  size_t s = sizeof( csv_file );
  
  ClearFlag( SIGHUP_RECEIVED );

  /* Create a file name for the Optimizer csv file */
  Strlcpy( csv_file, rc_config.input_file, s );
  Strlcat( csv_file, ".csv", s );

  /* Open Optimizer csv file */
  FILE *fp = NULL;
  if( !Open_File(&fp, csv_file, "w") )
  {
    if( error_dialog == NULL )
    {
      GtkBuilder *builder;
      error_dialog = create_error_dialog( &builder );
      gtk_label_set_text( GTK_LABEL(Builder_Get_Object(builder, "error_label")),
          "Failed to open Optimizer CSV file for writing" );
      gtk_widget_hide( Builder_Get_Object(builder, "error_okbutton") );
      gtk_widget_show( error_dialog );
      g_object_unref( builder );
    }
    perror( "xnec2c: Open_File()" );

    return;
  } // if( !Open_File(&fp, csv_file, "w") )

  /* Print data column titles */
  fprintf( fp, "%12s,", "Freq-MHz" );
  if( isFlagSet(PLOT_ZREAL_ZIMAG) ) fprintf( fp, "%12s,%12s,", "Z-real", "Z-imag" );
  if( isFlagSet(PLOT_ZMAG_ZPHASE) ) fprintf( fp, "%12s,%12s,", "Z-magn", "Z-phase" );
  if( isFlagSet(PLOT_VSWR) )        fprintf( fp, "%12s,", "VSWR" );
  if( isFlagSet(PLOT_GMAX) )        fprintf( fp, "%12s,", "Gain-max" );
  if( isFlagSet(PLOT_GVIEWER) )     fprintf( fp, "%12s,", "Gain-viewer" );
  if( isFlagSet(PLOT_NETGAIN) )     fprintf( fp, "%12s,", "Gain-net" );
  fprintf( fp, "\n" );

  /* Print frequency-dependent data corresponding
   * to graphs in plot of frequency-dependent data */
  for( int idx = 0; idx < calc_data.nfrq; idx++ )
  {
    /* Print the frequency in MHz */
    fprintf( fp, "%12.4E,", (double)save.freq[idx] );

    /* Print Z-real and Z-imag of input impedance */
    if( isFlagSet(PLOT_ZREAL_ZIMAG) )
    {
      fprintf( fp, "%12.4E,%12.4E,",
          impedance_data.zreal[idx], impedance_data.zimag[idx] );
    }

    /* Plot Z-magnitude and Z-phase */
    if( isFlagSet(PLOT_ZMAG_ZPHASE) )
    {
      fprintf( fp, "%12.4E,%12.4E,",
          impedance_data.zmagn[idx], impedance_data.zphase[idx] );
    }

    /* Print VSWR */
    if( isFlagSet(PLOT_VSWR) )
    {
      double zrpro2 = impedance_data.zreal[idx] + calc_data.zo;
      zrpro2 *= zrpro2;
      double zrmro2 = impedance_data.zreal[idx] - calc_data.zo;
      zrmro2 *= zrmro2;
      double zimag2 = impedance_data.zimag[idx] * impedance_data.zimag[idx];
      double gamma = sqrt( (zrmro2 + zimag2) / (zrpro2 + zimag2) );
      double vswr = (1 + gamma) / (1 - gamma);
      fprintf( fp, "%12.4E,", vswr );
    }

    /* Print Max gain for given polarization type */
    if( isFlagSet(PLOT_GMAX) && isFlagSet(ENABLE_RDPAT) )
    {
      int pol = calc_data.pol_type;
      int mgidx = rad_pattern[idx].max_gain_idx[pol];
      double max_gain = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
      fprintf( fp, "%12.4E,", max_gain );
    }

    /* Print gain in viewer's direction */
    if( isFlagSet(PLOT_GVIEWER) )
    {
      double viewer_gain = Viewer_Gain( structure_proj_params, idx );
      fprintf( fp, "%12.4E,", viewer_gain );
    }

    /* Print Net gain in max gain case */
    if( isFlagSet(PLOT_NETGAIN) )
    {
      int pol = calc_data.pol_type;
      int mgidx = rad_pattern[idx].max_gain_idx[pol];
      double max_gain = rad_pattern[idx].gtot[mgidx] + Polarization_Factor(pol, idx, mgidx);
      double Zr = impedance_data.zreal[idx];
      double Zi = impedance_data.zimag[idx];
      double Zo = calc_data.zo;
      double net_gain = max_gain +
        10.0 * log10( 4.0 * Zr * Zo / (pow(Zr + Zo, 2.0) + pow( Zi, 2.0 )) );
      fprintf( fp, "%12.4E,", net_gain );
    }

    fprintf( fp, "\n" );
  } //for( int idx = 0; idx < calc_data.nfrq; idx++ )

  fclose( fp );
} // Write_Optimizer_Data()

/*------------------------------------------------------------------------*/

/* Response to signal SIGHUP */
  void
Sig_HungUp( void )
{
  SetFlag( SIGHUP_RECEIVED );
  gboolean flag = TRUE;
  g_idle_add( Open_Input_File, (gpointer)&flag );
} // Sig_HungUp()

/*------------------------------------------------------------------------*/

