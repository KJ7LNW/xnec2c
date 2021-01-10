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

#include "geom_edit.h"
#include "shared.h"

/*------------------------------------------------------------------------*/

/* Insert_GE_Card()
 *
 * Inserts a default GE card if missing
 */

  static void
Insert_GE_Card( GtkListStore *store, GtkTreeIter *iter )
{
  gint idx, idi;

  /* Insert default GE card if list is clear */
  idx = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL );
  if( !idx )
  {
	gtk_list_store_append( store, iter );
	gtk_list_store_set( store, iter, GEOM_COL_NAME, "GE", -1 );
	for( idi = GEOM_COL_I1; idi < GEOM_NUM_COLS; idi++ )
	  gtk_list_store_set( store, iter, idi, "0", -1 );
  }

} /* Insert_GE_Card() */

/*------------------------------------------------------------------------*/

/* Get_Geometry_Data()
 *
 * Gets geometry data from a treeview row
 */

  static void
Get_Geometry_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gint idi, idf;
  gchar *sv;

  /* Get data from tree view (I1,I2, F1-F7)*/
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idi = GEOM_COL_I1; idi <= GEOM_COL_I2; idi++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idi, &sv, -1);
	  iv[idi-GEOM_COL_I1] = atoi(sv);
	  g_free(sv);
	}
	for( idf = GEOM_COL_F1; idf <= GEOM_COL_F7; idf++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idf, &sv, -1);
	  fv[idf-GEOM_COL_F1] = Strtod( sv, NULL );
	  g_free(sv);
	}
  }
  else Stop( _("Error reading row data\n"
		"Invalid list iterator"), ERR_OK );

} /* Get_Geometry_Data() */

/*------------------------------------------------------------------------*/

/* Set_Geometry_Data()
 *
 * Sets data into a geometry row
 */

  static void
Set_Geometry_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gchar str[13];
  gint idi, idf;

  /* Format and set editor data to treeview (I1, I2 & F1-F7) */
  if( iter && gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idi = GEOM_COL_I1; idi <= GEOM_COL_I2; idi++ )
	{
	  snprintf( str, 6, "%5d", iv[idi-GEOM_COL_I1] );
	  gtk_list_store_set( store, iter, idi, str, -1 );
	}

	for( idf = GEOM_COL_F1; idf <= GEOM_COL_F7; idf++ )
	{
	  snprintf( str, 13, "%12.5E", fv[idf-GEOM_COL_F1] );
	  gtk_list_store_set( store, iter, idf, str, -1 );
	}
  }
  else Stop( _("Error writing row data\n"
		"Please re-select row"), ERR_OK );

  SetFlag( NEC2_EDIT_SAVE );

} /* Set_Geometry_Data() */

/*------------------------------------------------------------------------*/

/* Set_Geometry_Int_Data()
 *
 * Sets integer (I1, I2) data into a geometry row
 */

  static void
Set_Geometry_Int_Data( GtkListStore *store, GtkTreeIter *iter, int *iv )
{
  gchar str[6];
  gint idi, idf;

  /* Format and set editor data to treeview (I1, I2) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idi = GEOM_COL_I1; idi <= GEOM_COL_I2; idi++ )
	{
	  snprintf( str, sizeof(str), "%5d", iv[idi-GEOM_COL_I1] );
	  gtk_list_store_set( store, iter, idi, str, -1 );
	}

	/* Clear unused float columns */
	for( idf = GEOM_COL_F1; idf <= GEOM_COL_F7; idf++ )
	  gtk_list_store_set( store, iter, idf, "0.0", -1 );
  }
  else Stop( _("Error writing row data\n"
		"Please re-select row"), ERR_OK );

  SetFlag( NEC2_EDIT_SAVE );

} /* Set_Geometry_Int_Data() */

/*------------------------------------------------------------------------*/

/* Insert_Blank_Geometry_Row()
 *
 * Inserts a blank row in a tree view with only its name (GW ... )
 */

  static void
Insert_Blank_Geometry_Row(
	GtkTreeView *view, GtkListStore *store,
	GtkTreeIter *iter, const gchar *name )
{
  GtkTreeSelection *selection;
  gboolean retv;
  gint n;
  gchar *str;

  if( nec2_edit_window == NULL )
	return;

  /* Get selected row, if any */
  selection = gtk_tree_view_get_selection( view );
  retv = gtk_tree_selection_get_selected(
	  selection, NULL, iter );

  /* If no selected row, insert new row into list
   * store before last row, else after the selected row,
   * but if this is a GE row, then insert before it */
  if( !retv )
  {
	n = gtk_tree_model_iter_n_children(
		GTK_TREE_MODEL(store), NULL );
	gtk_tree_model_iter_nth_child(
		GTK_TREE_MODEL(store), iter, NULL, n-1 );
	gtk_list_store_insert_before( store, iter, iter );
  }
  else
  {
	gtk_tree_model_get( GTK_TREE_MODEL(store),
		iter, GEOM_COL_NAME, &str, -1 );
	if( strcmp(str, "GE") == 0 )
	  gtk_list_store_insert_before( store, iter, iter );
	else
	  gtk_list_store_insert_after( store, iter, iter );
	g_free(str);
  }

  gtk_list_store_set( store, iter, GEOM_COL_NAME, name, -1 );
  for( n = GEOM_COL_I1; n < GEOM_NUM_COLS; n++ )
	gtk_list_store_set( store, iter, n, "--", -1 );
  gtk_tree_selection_select_iter( selection, iter );

} /* Insert_Blank_Geometry_Row() */

/*------------------------------------------------------------------------*/

/* Set_Wire_Conductivity()
 *
 * Sets the wire conductivity specified in a geometry editor
 * (wire, arc, helix) to a loading card (LD row) in commands treview
 */

  static void
Set_Wire_Conductivity( int tag, double s, GtkListStore *store )
{
  int idx, idi, nchld;
  GtkTreeIter iter_ld;
  gchar *str, sv[13];

  /* Find num of rows and first iter, abort if tree empty */
  nchld = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL);
  if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter_ld))
	return;

  /* Look for an LD card with tag number = tag */
  for( idx = 0; idx < nchld; )
  {
	gtk_tree_model_get( GTK_TREE_MODEL(store),
		&iter_ld, GEOM_COL_NAME, &str, -1 );

	if( strcmp(str, "LD") == 0 )
	{
	  g_free( str );
	  gtk_tree_model_get( GTK_TREE_MODEL(store),
		  &iter_ld, GEOM_COL_I2, &str, -1 );
	  if( atoi( str ) == tag )
	  {
		g_free( str );
		break;
	  }
	  else g_free( str );
	}
	else g_free( str );

	idx++;
	if( !gtk_tree_model_iter_next(
		  GTK_TREE_MODEL(store), &iter_ld) )
	  break;

  } /* for( idx = 0; idx < nchld; idx++ ) */

  /* If not found LD card with tagnum = tag, insert new */
  if( idx >= nchld )
  {
	gtk_tree_model_iter_nth_child(
		GTK_TREE_MODEL(store), &iter_ld, NULL, nchld-1 );
	gtk_list_store_insert_before( store, &iter_ld, &iter_ld );
	gtk_list_store_set(
		store, &iter_ld, CMND_COL_NAME, "LD", -1 );

	/* Clear rest of LD row */
	for( idi = CMND_COL_I1; idi <= CMND_COL_F6; idi++ )
	  gtk_list_store_set( store, &iter_ld, idi, "0", -1 );
  }

  /* Set LD card parameters */
  gtk_list_store_set( store, &iter_ld, CMND_COL_I1, "5", -1 );
  snprintf( sv, 6, "%5d", tag );
  gtk_list_store_set( store, &iter_ld, CMND_COL_I2, sv, -1 );
  snprintf( sv, 13, "%12.5E", s );
  gtk_list_store_set( store, &iter_ld, CMND_COL_F1, sv, -1 );

  /* Scroll tree view to bottom */
  gtk_adjustment_set_value( geom_adjustment,
	  gtk_adjustment_get_upper(geom_adjustment) -
	  gtk_adjustment_get_page_size(geom_adjustment) );

} /* Set_Wire_Conductivity() */

/*------------------------------------------------------------------------*/

/* Get_Wire_Conductivity()
 *
 * Gets the wire conductivity specified in a loading
 * card (LD row) in commands treview for a given tag #
 */

  static gboolean
Get_Wire_Conductivity( int tag, double *s, GtkListStore *store )
{
  int idx, type, nchld;
  GtkTreeIter iter_ld;
  gchar *str;
  static int t = -1;

  /* Find num of rows and first iter, abort if tree empty */
  nchld = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL );
  if( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter_ld) )
	return( FALSE );

  /* Look for an LD card with tag number = tag */
  for( idx = 0; idx < nchld; )
  {
	gtk_tree_model_get( GTK_TREE_MODEL(store),
		&iter_ld, GEOM_COL_NAME, &str, -1 );

	if( strcmp(str, "LD") == 0 )
	{
	  g_free( str );
	  gtk_tree_model_get( GTK_TREE_MODEL(store),
		  &iter_ld, GEOM_COL_I2, &str, -1 );
	  if( atoi(str) == tag )
	  {
		g_free( str );
		break;
	  }
	  else g_free( str );
	}
	else g_free( str );

	idx++;
	if( !gtk_tree_model_iter_next(
		  GTK_TREE_MODEL(store), &iter_ld) )
	  break;

  } /* for( idx = 0; idx < nchld; idx++ ) */

  /* If not found LD card with tagnum = tag, return s=0 */
  if( idx >= nchld )
  {
	*s = 0.0;
	t = tag;
	return( FALSE );
  }

  /* If LD card for given tag is already read, abort */
  if( t == tag ) return( FALSE );
  else t = tag;

  /* Get the loading type (we want LDTYP 5) */
  gtk_tree_model_get( GTK_TREE_MODEL(store),
	  &iter_ld, GEOM_COL_I1, &str, -1 );
  type = atoi( str );
  g_free( str );
  if( type != 5 ) return( FALSE );

  /* Get the wire conductivity S/m */
  gtk_tree_model_get( GTK_TREE_MODEL(store),
	  &iter_ld, CMND_COL_F1, &str, -1 );
  *s = Strtod( str, NULL );
  g_free( str );

  return( TRUE );
} /* Get_Wire_Conductivity() */

/*------------------------------------------------------------------------*/

/* Wire_Editor()
 *
 * Handles all actions of the wire editor window
 */
  void
Wire_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* Frame of tapered wire data */
  GtkWidget *frame;

  /* For reading/writing to GW & GC rows */
  static GtkTreeIter
	iter_gw,
	iter_gc;

  int idx, idi;

  static gboolean
	load   = FALSE, /* Enable wire loading (conductivity specified) */
	taper  = FALSE, /* Editing a tapered wire */
	save   = FALSE, /* Enable saving of editor data */
	busy   = FALSE, /* Block callbacks. Must be a better way to do this? */
	newpcl = TRUE,  /* New percent-of-lambda value */
	newrdm = TRUE,  /* New diameter ratio value */
	newwln = TRUE;  /* New wire length value */

  /* Float type data, wire conductivity */
  static gdouble fv[14], s = 0.0;

  /* Integer type data */
  static gint iv[4];

  /* Wire's projection on xyz axes */
  gdouble dx, dy, dz;

  /* Card (row) name */
  gchar name[3];

  /* Spin button names, int & float data */
  gchar *ispin[2] =
  {
	"wire_tagnum_spinbutton",
	"wire_numseg_spinbutton"
  };

  gchar *fspin[14] =
  {
	"wire_x1_spinbutton",
	"wire_y1_spinbutton",
	"wire_z1_spinbutton",
	"wire_x2_spinbutton",
	"wire_y2_spinbutton",
	"wire_z2_spinbutton",
	"wire_dia_spinbutton",
	"wire_rlen_spinbutton",
	"wire_dia1_spinbutton",
	"wire_dian_spinbutton",
	"wire_rdia_spinbutton",
	"wire_pcl_spinbutton",
	"wire_len_spinbutton",
	"wire_res_spinbutton"
  };


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, wire_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( wire_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( ( action == EDITOR_APPLY) ||
	  ( action == EDITOR_OK)    ||
	  ((action == EDITOR_NEW) && save) )
  {
	/* Clear data not used in GC card */
	iv[SPIN_COL_I3] = iv[SPIN_COL_I4] = 0;
	for( idx = WIRE_RDIA; idx <= WIRE_RES; idx++ )
	  fv[idx] = 0.0;

	/* Set GW data to treeview */
	Set_Geometry_Data( geom_store, &iter_gw, iv, fv );

	/* Set GC card data to treeview if taper */
	if( taper )
	  Set_Geometry_Data( geom_store, &iter_gc,
		  &iv[SPIN_COL_I3], &fv[WIRE_RLEN] );

	/* Set wire conductivity (loading card) */
	if( load )
	  Set_Wire_Conductivity( iv[SPIN_COL_I1], s, cmnd_store );

	if( action == EDITOR_OK ) taper = FALSE;
	save = load = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the wire editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
  {
	spin = GTK_SPIN_BUTTON( Builder_Get_Object(wire_editor_builder, ispin[idi]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the wire editor */
  for( idx = WIRE_X1; idx <= WIRE_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON( Builder_Get_Object(wire_editor_builder, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[WIRE_DIA]  /= 2.0;
  fv[WIRE_DIA1] /= 2.0;
  fv[WIRE_DIAN] /= 2.0;

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New wire row(s) to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gw );

	  /* Insert a new blank GW row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gw, "GW" );

	  /* Some default values */
	  iv[SPIN_COL_I1] = ++tag_num;
	  iv[SPIN_COL_I3] = iv[SPIN_COL_I4] = 0;

	  if( taper )
	  {
		/* Insert a new blank GC row if tapered wire */
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_gc, "GC" );
		fv[WIRE_DIA]  = 0.0;
	  }

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a wire row (GW/GC) */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gw, name );
	  iter_gc = iter_gw;

	  /*** Editing a GC card ***/
	  if( strcmp(name, "GC") == 0 )
	  {
		taper = TRUE;

		/* Get tapered wire data from tree view */
		Get_Geometry_Data( geom_store, &iter_gc,
			&iv[SPIN_COL_I3], &fv[WIRE_RLEN] );
		fv[WIRE_RDIA] = fv[WIRE_RLEN];

		/* Check for a preceding GW card.
		 * If card is GW, get wire data */
		if( Check_Card_Name(geom_store, &iter_gw, PREVIOUS, "GW") )
		{
		  Get_Geometry_Data( geom_store, &iter_gw, iv, fv );

		  /* Warn user if wire radius not 0 */
		  if( fv[WIRE_DIA] != 0.0 )
			Stop( _("GC card preceded by GW card\n"
				  "with non-zero wire radius"), ERR_OK );

		} /* if( strcmp(name, "GC") == 0 ) */
		else Stop( _("No GW card before GC card"), ERR_OK );
	  }
	  else /*** Editing a GW card ***/
	  {
		/* Get wire data from tree view */
		Get_Geometry_Data( geom_store, &iter_gw, iv, fv );

		/* Get wire conductivity if specified in LD card */
		Get_Wire_Conductivity(iv[SPIN_COL_I1], &s, cmnd_store);
		fv[WIRE_RES] = s;

		/*** Get tapered wire data if dia = 0 ***/
		if( fv[WIRE_DIA] == 0.0 )
		{
		  /* If next card is GC, get data */
		  if( Check_Card_Name(geom_store, &iter_gc, NEXT, "GC") )
		  {
			taper = TRUE;

			/* Get wire taper data from tree view */
			Get_Geometry_Data( geom_store, &iter_gc,
				&iv[SPIN_COL_I3], &fv[WIRE_RLEN] );
			fv[WIRE_RDIA] = fv[WIRE_RLEN];
		  }
		  else Stop( _("No GC card after a GW card\n"
				"with a zero wire radius"), ERR_OK );

		} /* if( fv[WIRE_DIA] == 0.0 ) */
		/* If radius != 0, next card should not be GC */
		else /* If radius != 0, next card should not be GC */
		{
		  taper = FALSE;
		  if( Check_Card_Name(geom_store, &iter_gc, NEXT, "GC") )
			Stop( _("GC card follows a GW card\n"
				  "with non-zero wire radius"), ERR_OK );
		}

	  } /* if( strcmp(name, "GC") == 0 ) */
	  break;

	case WIRE_EDITOR_TAPR: /* Show taper wire data if button checked */
	  {
		/* Wire diameter */
		static double diam;

		/* Read tapered wire checkbutton */
		taper = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(wire_editor_builder, "wire_taper_checkbutton")) );
		if( taper )
		{
		  /* Set wire dia to 0 */
		  diam = fv[WIRE_DIA];
		  fv[WIRE_DIA] = 0.0;

		  /* Insert GC card if valid GW iteration */
		  if( gtk_list_store_iter_is_valid(geom_store, &iter_gw) )
			Insert_Blank_Geometry_Row(
				geom_treeview, geom_store, &iter_gc, "GC");
		}
		else
		{
		  /* Restore wire diam */
		  fv[WIRE_DIA] = diam;

		  /* Remove GC card if valid */
		  Remove_Row( geom_store, &iter_gc );
		} /* if( taper ) */
	  }
	  save = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel wire editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gw );
	  if( taper ) Remove_Row( geom_store, &iter_gc );
	  save = busy = taper = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(wire_editor_builder, fspin[WIRE_RES]) );
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 )
	  {
		save = TRUE;
		load = TRUE;
	  }
	  else load = FALSE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 ) load = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		if( taper && ( fv[WIRE_RLEN] != 1.0) ) /* Taper ratio < 1 */
		{
		  double cnt = 1.0 / (fv[WIRE_PCL]/100.0) * (1.0-fv[WIRE_RLEN]);
		  if( cnt < 1.0 )
		  {
			double i = ceil( log(1.0-cnt) / log(fv[WIRE_RLEN]) );
			iv[SPIN_COL_I2] = (gint)i;
		  }
		}
		else
		{
		  double i = ceil( 100.0 * (fv[WIRE_LEN]/fv[WIRE_PCL]) /
			  ((double)CVEL/calc_data.mxfrq) );
		  iv[SPIN_COL_I2] = (gint)i;
		}
	  }
	  newpcl = FALSE;
	  save = TRUE;
	  break;

	  /* Calculate new wire end points on length change */
	case WIRE_EDITOR_WLEN:
	  {
		double l, dl;

		/* Length of wire's projection on axes */
		dx = fv[WIRE_X2]-fv[WIRE_X1];
		dy = fv[WIRE_Y2]-fv[WIRE_Y1];
		dz = fv[WIRE_Z2]-fv[WIRE_Z1];

		/* Wire's length */
		l = (gdouble)sqrt( dx*dx + dy*dy + dz*dz );

		/* 1/2 of change in wire's length / length */
		dl = fv[WIRE_LEN] - l; dl /= 2.0 * l;

		/* Corresponding change in wire end co-ordinates */
		dx *= dl; dy *= dl; dz *= dl;
		fv[WIRE_X1] -= dx; fv[WIRE_X2] += dx;
		fv[WIRE_Y1] -= dy; fv[WIRE_Y2] += dy;
		fv[WIRE_Z1] -= dz; fv[WIRE_Z2] += dz;
	  }
	  newwln = FALSE;
	  save = TRUE;
	  break;

	case WIRE_EDITOR_RLEN: /* Length taper ratio changed */
	  fv[WIRE_DIAN] = fv[WIRE_DIA1] *
		pow(fv[WIRE_RLEN], (double)(iv[SPIN_COL_I2]-1)); /* Nth seg dia */
	  break;

	case WIRE_EDITOR_RDIA: /* New diameter taper ratio */
	  fv[WIRE_DIAN] = fv[WIRE_DIA1] *
		pow(fv[WIRE_RDIA], (double)(iv[SPIN_COL_I2]-1)); /* Nth seg dia */
	  newrdm = FALSE;
	  save = TRUE;

  } /* switch( action ) */

  /* Frame of tapered wire data */
  frame = Builder_Get_Object(wire_editor_builder, "wire_taperframe");
  
  /* Show taper data if appropriate */
  if( taper )
  {
	gtk_widget_show( frame );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(wire_editor_builder, "wire_taper_checkbutton")), TRUE );
  }
  else
  {
	gtk_widget_hide( frame );
	gtk_window_resize( GTK_WINDOW(wire_editor), 10, 10 );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(wire_editor_builder, "wire_taper_checkbutton")), FALSE );
  }

  /*** Calculate wire length ***/
  if( newwln )
  {
	/* Length of wire's projection on axes */
	dx = fv[WIRE_X2]-fv[WIRE_X1];
	dy = fv[WIRE_Y2]-fv[WIRE_Y1];
	dz = fv[WIRE_Z2]-fv[WIRE_Z1];

	/* Wire's length */
	fv[WIRE_LEN] = (gdouble)sqrt( dx*dx + dy*dy + dz*dz );
  }
  else newwln = TRUE;

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	if( taper && (fv[WIRE_RLEN] != 1.0) )
	  fv[WIRE_PCL] =
		100.0 * fv[WIRE_LEN] * (1.0-fv[WIRE_RLEN]) /
		(1.0-(gdouble)pow(fv[WIRE_RLEN], (gdouble)iv[SPIN_COL_I2])) /
		((gdouble)CVEL/calc_data.mxfrq);
	else
	  fv[WIRE_PCL] = 100.0 * (fv[WIRE_LEN] /
		  (gdouble)iv[SPIN_COL_I2]) / ((gdouble)CVEL/calc_data.mxfrq);
  }
  else newpcl = TRUE;

  /*** Calculate radius taper ratio ***/
  if( (iv[SPIN_COL_I2] > 1) && newrdm )
	fv[WIRE_RDIA] = pow( fv[WIRE_DIAN]/fv[WIRE_DIA1],
		1.0/(double)(iv[SPIN_COL_I2]-1) );
  else newrdm = TRUE;

  /* Write int data to the wire editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(wire_editor_builder, ispin[idi]) );
	gtk_spin_button_set_value( spin, iv[idi] );
  }

  /* Write float data to the wire editor (F1 to F7 and taper) */
  fv[WIRE_DIA] *= 2.0; fv[WIRE_DIA1] *= 2.0; fv[WIRE_DIAN] *= 2.0;
  for( idx = WIRE_X1; idx <= WIRE_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(wire_editor_builder, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  fv[WIRE_DIA] /= 2.0; fv[WIRE_DIA1] /= 2.0; fv[WIRE_DIAN] /= 2.0;

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Wire_Editor() */

/*------------------------------------------------------------------------*/

/* Patch_Editor()
 *
 * Handles all actions of the patch editor window
 */
  void
Patch_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to SP/SM & SC rows */
  static GtkTreeIter
	iter_sp,
	iter_sc;

  int idx, idi;

  /* Patch type */
  static int ptype = PATCH_ARBT;

  /* Integer type data */
  static gint iv[4];

  /* Float type data */
  static gdouble fv[14];

  /* Card (row) name */
  static gchar name[3] = "SP";

  /* Spin button names, int data */
  gchar *ispin[2] =
  {
	"patch_nx_spinbutton",
	"patch_ny_spinbutton",
  };

  /* Spin button names, float data */
  gchar *fspin[12] =
  {
	"patch_x1_spinbutton",
	"patch_y1_spinbutton",
	"patch_z1_spinbutton",
	"patch_x2_spinbutton",
	"patch_y2_spinbutton",
	"patch_z2_spinbutton",
	"patch_x3_spinbutton",
	"patch_y3_spinbutton",
	"patch_z3_spinbutton",
	"patch_x4_spinbutton",
	"patch_y4_spinbutton",
	"patch_z4_spinbutton",
  };

  gchar *rbutton[5] =
  {
	"patch_arbitrary_radiobutton",
	"patch_rectangular_radiobutton",
	"patch_triangular_radiobutton",
	"patch_quadrilateral_radiobutton",
	"patch_surface_radiobutton"
  };

  static gboolean
	save  = FALSE,	/* Enable saving of editor data */
    busy  = FALSE,	/* Block callbacks. Must be a better way to do this? */
    ptset = FALSE;	/* Set patch type radio buttons */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, patch_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( patch_editor );
	return;
  }

  /* Set int data from the patch editor (SP card) */
  if( ptype != PATCH_SURF )
  {
	iv[SPIN_COL_I1] = 0; 	 /* Not used in SP */
	iv[SPIN_COL_I2] = ptype; /* Patch type */
  }
  else /* Set int data from the patch editor (SM card) */
	for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
	{
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(patch_editor_builder, ispin[idi]) );
	  double i = gtk_spin_button_get_value( spin );
	  iv[idi] = (gint)i;
	}

  /* Read float data from the patch editor */
  for( idx = PATCH_X1; idx <= PATCH_Z2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(patch_editor_builder, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  for( idx = PATCH_X3; idx <= PATCH_Z4; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(patch_editor_builder, fspin[idx-1]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Clear data not used in SC card */
	iv[SPIN_COL_I3] = iv[SPIN_COL_I4] = 0;
	fv[UNUSED_F1]   = fv[UNUSED_F2]   = 0.0;

	/* Set SP data to treeview */
	Set_Geometry_Data( geom_store, &iter_sp, iv, fv );

	/* Set SC card data to treeview if non arbitrary */
	if( ptype != PATCH_ARBT )
	  Set_Geometry_Data(geom_store,
		  &iter_sc, &iv[SPIN_COL_I3], &fv[PATCH_X3]);

	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New patch to edit, enter some default data */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_sp );

	  /* Insert a new blank SP row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_sp, name );

	  /* Insert an SC card for non-arbitrary patch */
	  if( ptype != PATCH_ARBT )
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_sc, "SC" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  ptset = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a selected geometry row */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_sp, name );
	  iter_sc = iter_sp;

	  /*** Editing an SC card ***/
	  if( strcmp(name, "SC") == 0 )
	  {
		/* Get patch data from SC row in tree view */
		Get_Geometry_Data( geom_store, &iter_sc,
			&iv[SPIN_COL_I3], &fv[PATCH_X3] );

		/* Check for a preceding SP | SM card.
		 * If card is SP, get patch data */
		if( Check_Card_Name(geom_store, &iter_sp, PREVIOUS, "SP") )
		{
		  Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
		  ptype = iv[SPIN_COL_I2];

		  /* Warn user if SP card with arbitrary
		   * patch is followed by an SC card */
		  if( ptype == PATCH_ARBT )
			Stop( _("SC card preceded by SP card\n"
				  "with arbitrary patch type"), ERR_OK );

		} /* if( Check_Card_Name(geom_store, &iter_sp, PREVIOUS, "SP") ) */
		else /* Look for a previous SM card */
		{
		  iter_sp = iter_sc;
		  if( Check_Card_Name(geom_store, &iter_sp, PREVIOUS, "SM") )
		  {
			Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
			ptype = PATCH_SURF;
		  }
		  else Stop( _("No SP or SM card before SC card"), ERR_OK );
		}
	  } /* if( strcmp(name, "SC") == 0 ) */
	  else /*** Editing an SP|SM card ***/
		if( strcmp(name, "SP") == 0 )
		{
		  /* Get patch data from treeview */
		  Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
		  ptype = iv[SPIN_COL_I2];

		  /*** Get SC card data if patch type non-arbitrary ***/
		  if( ptype != PATCH_ARBT )
		  {
			/* If next card is SC, get data */
			if( Check_Card_Name(geom_store, &iter_sc, NEXT, "SC") )
			  Get_Geometry_Data( geom_store, &iter_sc,
				  &iv[SPIN_COL_I3], &fv[PATCH_X3] );
			else
			{
			  ptype = PATCH_ARBT;
			  Stop( _("No SC card after an SP card\n"
					"with non-arbitrary patch type"), ERR_OK );
			}

		  } /* if( ptype != PATCH_ARBT ) */
		  /* If patch type arbitrary, no SC card should follow */
		  else if( Check_Card_Name(geom_store, &iter_sc, NEXT, "SC") )
			Stop( _("SC card follows an SP card\n"
				  "with arbitrary patch type"), ERR_OK );
		} /* if( strcmp(name, "SP") == 0 ) */
		else /* SM card */
		{
		  /* Get patch data from treeview */
		  Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
		  ptype = PATCH_SURF;

		  /* If next card is SC, get data */
		  if( Check_Card_Name(geom_store, &iter_sc, NEXT, "SC") )
			Get_Geometry_Data( geom_store, &iter_sc,
				&iv[SPIN_COL_I3], &fv[PATCH_X3] );
		  else Stop( _("No SC card after an SM card"), ERR_OK );

		} /* if( strcmp(name, "SP") == 0 ) */

	  ptset = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel patch editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_sp );
	  if( ptype != PATCH_ARBT )
		Remove_Row( geom_store, &iter_sc );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some editor data changed */
	  save = TRUE;
	  break;

	case PATCH_EDITOR_ARBT: /* Arbitary-shaped patch */
	  ptype = PATCH_ARBT;
	  ptset = FALSE;
	  save  = TRUE;
	  for( idx = PATCH_X3; idx <= UNUSED_F2; idx++ )
		fv[idx] = 0.0;

	  /* Remove SC card */
	  Remove_Row( geom_store, &iter_sc );
	  break;

	case PATCH_EDITOR_RECT: /* Rectangular patch */
	  ptype = PATCH_RECT;
	  ptset = FALSE;
	  save  = TRUE;
	  for( idx = PATCH_X4; idx <= UNUSED_F2; idx++ )
		fv[idx] = 0.0;
	  break;

	case PATCH_EDITOR_TRIA: /* Triangular patch */
	  ptype = PATCH_TRIA;
	  ptset = FALSE;
	  save  = TRUE;
	  for( idx = PATCH_X4; idx <= UNUSED_F2; idx++ )
		fv[idx] = 0.0;
	  break;

	case PATCH_EDITOR_QUAD: /* Quadrilateral patch */
	  ptype = PATCH_QUAD;
	  ptset = FALSE;
	  save  = TRUE;
	  break;

	case PATCH_EDITOR_SURF: /* Multi-patch surface */
	  iv[SPIN_COL_I1] = iv[SPIN_COL_I2] = 8;
	  ptype = PATCH_SURF;
	  ptset  = FALSE;
	  save   = TRUE;
	  break;

	case PATCH_EDITOR_SCCD: /* Insert SC card (non-arbitrary patch case) */
	  if( gtk_list_store_iter_is_valid( geom_store, &iter_sp) )
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_sc, "SC");

  } /* switch( action ) */

  /* Change labels as needed */
  if( ptype == PATCH_ARBT )
  {
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_x1_label")), _("Center - X") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_y1_label")), _("Center - Y") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_z1_label")), _("Center - Z") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_x2_label")), _("Normal - Elev.") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_y2_label")), _("Normal - Azim.") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_z2_label")), _("Patch Area") );
  }
  else
  {
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_x1_label")), _("Corner 1 - X") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_y1_label")), _("Corner 1 - Y") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_z1_label")), _("Corner 1 - Z") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_x2_label")), _("Corner 2 - X") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_y2_label")), _("Corner 2 - Y") );
	gtk_label_set_text( GTK_LABEL(Builder_Get_Object(
			patch_editor_builder, "patch_z2_label")), _("Corner 2 - Z") );
  }

  /* Hide/Show parts of window as needed */
  switch( ptype )
  {
	case PATCH_ARBT: /* Arbitary shaped patch */
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sc_frame") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_RECT: /* Rectangular patch */
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sc_frame") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sc_grid") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_TRIA: /* Triangular patch */
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sc_frame") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sc_grid") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_QUAD: /* Quadrilateral patch */
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sc_frame") );
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sc_grid") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_SURF: /* Multi-patch surface */
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sm_frame") );
	  gtk_widget_show( Builder_Get_Object(patch_editor_builder, "patch_sc_frame") );
	  gtk_widget_hide( Builder_Get_Object(patch_editor_builder, "patch_sc_grid") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );

  } /* switch( ptype ) */

  /* Set patch type radio button */
  if( ptset )
  {
	ptset = FALSE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(patch_editor_builder, rbutton[ptype])), TRUE );
  }

  /* Set card name */
  Strlcpy( name, (ptype == PATCH_SURF ? "SM" : "SP"), sizeof(name) );
  if( gtk_list_store_iter_is_valid(geom_store, &iter_sp) && save )
	gtk_list_store_set( geom_store, &iter_sp, GEOM_COL_NAME, name, -1 );

  /* Write int data for SM card */
  if( ptype == PATCH_SURF )
	for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
	{
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(patch_editor_builder, ispin[idi]) );
	  gtk_spin_button_set_value( spin, iv[idi] );
	}

  /* Write float data to the patch editor */
  for( idx = PATCH_X1; idx <= PATCH_Z2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(patch_editor_builder, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  for( idx = PATCH_X3; idx <= PATCH_Z4; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(patch_editor_builder, fspin[idx-1]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Patch_Editor() */

/*------------------------------------------------------------------------*/

/* Arc_Editor()
 *
 * Handles all actions of the arc editor window
 */

  void
Arc_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to GA rows */
  static GtkTreeIter iter_ga;

  int idx, idi;

  static gboolean
	load   = FALSE, /* Enable wire loading (conductivity specified) */
	newpcl = TRUE,  /* New percent-of-lambda value  */
	save   = FALSE, /* Enable saving of editor data */
	busy   = FALSE; /* Block callbacks. Must be a better way to do this? */

  /* Float type data */
  static gdouble fv[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  /* Wire conductivity */
  static gdouble s = 0.0;

  /* Integer type data */
  static gint iv[2];

  /* Card (row) name */
  gchar name[3];

  /* Spin button names, int & float data */
  gchar *ispin[2] =
  {
	"arc_tagnum_spinbutton",
	"arc_numseg_spinbutton"
  };

  gchar *fspin[6] =
  {
	"arc_rad_spinbutton",
	"arc_end1_spinbutton",
	"arc_end2_spinbutton",
	"arc_dia_spinbutton",
	"arc_pcl_spinbutton",
	"arc_res_spinbutton"
  };


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, arc_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( arc_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Geometry_Data( geom_store, &iter_ga, iv, fv );

	/* Set wire conductivity (loading card) */
	if( load )
	  Set_Wire_Conductivity( iv[SPIN_COL_I1], s, cmnd_store );

	save = load = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the arc editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(arc_editor_builder, ispin[idi]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the arc editor */
  for( idx = ARC_RAD; idx <= ARC_PCL; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(arc_editor_builder, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[ARC_DIA] /= 2.0;

  /* Get wire conductivity */
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(arc_editor_builder, fspin[idx]) );
  s = gtk_spin_button_get_value( spin );

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New arc row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_ga );

	  /* Insert a new blank GA row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_ga, "GA");

	  /* Some default values */
	  iv[SPIN_COL_I1] = ++tag_num;
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(arc_editor_builder, ispin[SPIN_COL_I1]));
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit an arc row (GA) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_ga, name );

	  /* Get arc data from tree view */
	  Get_Geometry_Data(
		  geom_store, &iter_ga, iv, fv );

	  /* Get wire conductivity if specified in LD card */
	  Get_Wire_Conductivity(iv[SPIN_COL_I1], &s, cmnd_store);
	  break;

	case EDITOR_CANCEL: /* Cancel arc editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_ga );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(arc_editor_builder, fspin[SPIN_COL_F6]) );
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 )
	  {
		save = TRUE;
		load = TRUE;
	  }
	  else load = FALSE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 )	load = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		gdouble len = fv[ARC_RAD] *
		  (gdouble)fabs( fv[ARC_END1] - fv[ARC_END2] ) * (gdouble)TORAD;
		double i = ceil( 100.0 / fv[ARC_PCL] *
			len / ((gdouble)CVEL / (gdouble)calc_data.mxfrq) );
		iv[SPIN_COL_I2] = (gint)i;
	  }
	  newpcl = FALSE;
	  save = TRUE;

  } /* switch( action ) */

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	gdouble len = fv[ARC_RAD] *
	  (gdouble)fabs( fv[ARC_END1] - fv[ARC_END2] ) * (gdouble)TORAD;
	fv[ARC_PCL] = 100.0 * (len/(gdouble)iv[SPIN_COL_I2]) /
	  ((gdouble)CVEL / (gdouble)calc_data.mxfrq);
  }
  else newpcl = TRUE;

  /* Write int data to the arc editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(arc_editor_builder, ispin[idi]) );
	gtk_spin_button_set_value( spin, iv[idi] );
  }

  /* Write float data to the arc editor (F1 to F4 & pcl/resistance) */
  fv[ARC_DIA] *= 2.0;
  for( idx = ARC_RAD; idx <= ARC_PCL; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(arc_editor_builder, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  fv[ARC_DIA] /= 2.0;

  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(arc_editor_builder, fspin[idx]) );
  gtk_spin_button_set_value( spin, s );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Arc_Editor() */

/*------------------------------------------------------------------------*/

/* Helix_Editor()
 *
 * Handles all actions of the helix editor window
 */

  void
Helix_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;
  GtkToggleButton *toggle = NULL;
  GtkImage *img;

  /* For reading/writing to GH rows */
  static GtkTreeIter iter_gh;

  gint idx;
  gdouble ftmp;

  static gboolean
	load      = FALSE, /* Enable wire loading (conductivity specified) */
	helix_rh  = TRUE,  /* Specify a right hand helix */
	helix_lh  = FALSE, /* Specify a left hand helix */
	spiral_rh = FALSE, /* Specify a right hand helix */
	spiral_lh = FALSE, /* Specify a left hand helix */
	newpcl    = TRUE,  /* New percent-of-lambda value  */
	newtsp    = FALSE, /* New turns space value */
	link_a1b1 = TRUE,  /* Link values of a1-b1 radii */
	link_b1a2 = TRUE,  /* Link values of b1-a2 radii */
	link_a2b2 = TRUE,  /* Link values of a2-b2 radii */
	save      = FALSE, /* Enable saving of editor data */
	busy      = FALSE; /* Block callbacks. Must be a better way to do this? */

  /* Float type data, wire conductivity */
  static
	gdouble fv[10],
	s = 0.0,
	helix_len = HELIX_LENGTH;

  /* Integer type data */
  static gint
	iv[2],
	seg_turn = HELIX_SEG_TURN;

  /* Card (row) name */
  gchar name[3];

  /* Spin button names, int & float data */
  gchar *ispin[2] =
  {
	"helix_tagnum_spinbutton",
	"helix_numseg_spinbutton"
  };

  gchar *fspin[10] =
  {
	"helix_tspace_spinbutton",
	"helix_len_spinbutton",
	"helix_radxzo_spinbutton",
	"helix_radyzo_spinbutton",
	"helix_radxzhl_spinbutton",
	"helix_radyzhl_spinbutton",
	"helix_dia_spinbutton",
	"helix_pcl_spinbutton",
	"helix_nturns_spinbutton",
	"helix_res_spinbutton"
  };


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, helix_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( helix_editor );
	return;
  }

  /*** Pass data to nec2 editor if appropriate ***/
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Change seg/turn to total number of segs */
	gdouble n = ceil( (gdouble)seg_turn * fv[HELIX_NTURN] );
	iv[SPIN_COL_I2] = (gint)n;

	/* Change to left/right hand helix */
	if( helix_lh )
	  fv[HELIX_LEN] = -helix_len;
	else if( helix_rh )
	  fv[HELIX_LEN] = helix_len;
	else /* Its a spiral */
	{
	  fv[HELIX_LEN] = 0.0;
	  if( spiral_rh )
		fv[HELIX_TSPACE] = (gdouble)seg_turn;
	  else if( spiral_lh )
		fv[HELIX_TSPACE] = -(gdouble)seg_turn;
	}

	/* Save data to NEC2 editor treeview */
	Set_Geometry_Data( geom_store, &iter_gh, iv, fv );

	/* Set wire conductivity (loading card) */
	if( load ) Set_Wire_Conductivity( iv[SPIN_COL_I1], s, cmnd_store );

	save = load = FALSE;
  } /* if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) */

  /*** Read int data from the helix editor ***/
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object( helix_editor_builder, ispin[SPIN_COL_I1]) );
  iv[SPIN_COL_I1] = gtk_spin_button_get_value_as_int( spin );
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object( helix_editor_builder, ispin[SPIN_COL_I2]) );
  seg_turn = gtk_spin_button_get_value_as_int( spin );

  /*** Read float data from the helix editor ***/
  for( idx = HELIX_TSPACE; idx <= HELIX_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(helix_editor_builder, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[HELIX_DIA] /= 2.0;
  helix_len = fv[SPIN_COL_F2];

  /* Link a1-b1 radius spinbuttons */
  if( link_a1b1 )
	fv[HELIX_RYZO] = fv[HELIX_RXZO];

  /* Link b1-a2 radius spinbuttons */
  if( link_b1a2 )
	fv[HELIX_RXZHL] = fv[HELIX_RYZO];

  /* Link a2-b2 radius spinbuttons */
  if( link_a2b2 )
	fv[HELIX_RYZHL] = fv[HELIX_RXZHL];

  /*** Respond to user action ***/
  switch( action )
  {
	case EDITOR_NEW: /* New helix row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gh );

	  /* Insert a new blank GH row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gh, "GH");

	  /* Some default values */
	  iv[SPIN_COL_I1] = ++tag_num;
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(helix_editor_builder, ispin[SPIN_COL_I1]) );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a helix row (GH) */
	  /* Get selected row */
	  Get_Selected_Row( geom_treeview, geom_store, &iter_gh, name );

	  /* Get helix data from tree view */
	  Get_Geometry_Data( geom_store, &iter_gh, iv, fv );

	  /* Get wire conductivity if specified in LD card */
	  Get_Wire_Conductivity(iv[SPIN_COL_I1], &s, cmnd_store);
	  fv[HELIX_RES] = s;

	  /* Set Right hand or Left hand helix or spiral */
	  helix_len = fv[HELIX_LEN];
	  if( helix_len > 0.0 ) /* Right hand helix */
	  {
		helix_rh  = TRUE;
		helix_lh  = FALSE;
		spiral_rh = FALSE;
		spiral_lh = FALSE;
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(helix_editor_builder, "helix_right_hand_radiobutton") );
	  }
	  else if( helix_len < 0.0 ) /* Left hand helix */
	  {
		helix_rh  = FALSE;
		helix_lh  = TRUE;
		spiral_rh = FALSE;
		spiral_lh = FALSE;
		helix_len = -helix_len;
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(helix_editor_builder, "helix_left_hand_radiobutton") );
	  }
	  else if( fv[HELIX_TSPACE] > 0.0 ) /* Right hand spiral */
	  {
		spiral_rh = TRUE;
		spiral_lh = FALSE;
		helix_rh  = FALSE;
		helix_lh  = FALSE;
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(helix_editor_builder, "spiral_right_hand_radiobutton") );
	  }
	  else if( fv[HELIX_TSPACE] < 0.0 ) /* Left hand spiral */
	  {
		spiral_rh = FALSE;
		spiral_lh = TRUE;
		helix_rh  = FALSE;
		helix_lh  = FALSE;
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(helix_editor_builder, "spiral_left_hand_radiobutton") );
	  }
	  if( toggle ) gtk_toggle_button_set_active( toggle, TRUE );

	  /* Change to number of segs/turn */
	  if( helix_rh || helix_lh )
	  {
		fv[HELIX_NTURN] = helix_len / fv[HELIX_TSPACE];
		seg_turn = (gint)((gdouble)iv[SPIN_COL_I2] / fv[HELIX_NTURN]);
	  }
	  else if( spiral_rh || spiral_lh )
	  {
		seg_turn = (gint)( fabs(fv[SPIN_COL_F1]) );
		fv[HELIX_NTURN] = iv[SPIN_COL_I2] / seg_turn;
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel helix editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gh );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(helix_editor_builder, fspin[HELIX_RES]));
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 )
	  {
		save = TRUE;
		load = TRUE;
	  }
	  else load = FALSE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 )	load = TRUE;
	  break;

	case HELIX_EDITOR_RH_HELIX: /* Right hand helix */
	  helix_rh  = TRUE;
	  helix_lh  = FALSE;
	  spiral_rh = FALSE;
	  spiral_lh = FALSE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_LH_HELIX: /* Left hand helix */
	  helix_rh  = FALSE;
	  helix_lh  = TRUE;
	  spiral_rh = FALSE;
	  spiral_lh = FALSE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_RH_SPIRAL: /* Right hand spiral */
	  spiral_rh = TRUE;
	  spiral_lh = FALSE;
	  helix_rh  = FALSE;
	  helix_lh  = FALSE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_LH_SPIRAL: /* Left hand spiral */
	  spiral_rh = FALSE;
	  spiral_lh = TRUE;
	  helix_rh  = FALSE;
	  helix_lh  = FALSE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_LINK_A1B1: /* Link a1-b1 radius spinbuttons */
	  link_a1b1 = !link_a1b1;
	  img = GTK_IMAGE(
		  Builder_Get_Object(helix_editor_builder, "helix_link_a1b1") );
	  if( link_a1b1 )
		gtk_image_set_from_file( img, "/usr/share/xnec2c/link.svg" );
	  else
		gtk_image_set_from_file( img, "/usr/share/xnec2c/unlink.svg" );
	  break;

	case HELIX_EDITOR_LINK_B1A2: /* Link b1-a2 radius spinbuttons _for_helix_ */
	  if( helix_rh || helix_lh )
	  {
		link_b1a2 = !link_b1a2;
		img = GTK_IMAGE(
			Builder_Get_Object(helix_editor_builder, "helix_link_b1a2") );
		if( link_b1a2 )
		  gtk_image_set_from_file( img, "/usr/share/xnec2c/link.svg" );
		else
		  gtk_image_set_from_file( img, "/usr/share/xnec2c/unlink.svg" );
	  }
	  break;

	case HELIX_EDITOR_LINK_A2B2: /* Link a2-b2 radius spinbuttons */
	  link_a2b2 = !link_a2b2;
	  img = GTK_IMAGE(
		  Builder_Get_Object(helix_editor_builder, "helix_link_a2b2") );
	  if( link_a2b2 )
		gtk_image_set_from_file( img, "/usr/share/xnec2c/link.svg" );
	  else
		gtk_image_set_from_file( img, "/usr/share/xnec2c/unlink.svg" );
	  break;

	case HELIX_EDITOR_NTURN: /* New number of turns _for_helix_ */
	  if( helix_rh || helix_lh )
		fv[HELIX_TSPACE] = helix_len / fv[HELIX_NTURN];
	  save = TRUE;
	  break;

	case HELIX_EDITOR_TSPACE: /* New turns spacing _for_helix_ */
	  if( helix_rh || helix_lh )
	  {
		helix_len = fv[HELIX_TSPACE] * fv[HELIX_NTURN];
		newtsp = TRUE;
	  }
	  save = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		gdouble len, f, ave_rad, n;

		/* Average radius of helix or spiral (approximate) */
		ave_rad =
		  ( fv[HELIX_RXZO]  + fv[HELIX_RYZO] +
		    fv[HELIX_RXZHL] + fv[HELIX_RYZHL] ) / 4.0;

		/* Pitch angle of helix, assumes untapered helix */
		if( helix_rh || helix_lh )
		  f = atan( fv[HELIX_TSPACE] / (gdouble)M_2PI / ave_rad );
		else f = 0.0;

		/* Helix turn length */
		len = (gdouble)M_2PI * ave_rad / (gdouble)cos( f );

		/* New number of segments */
		n = ceil(100.0 / fv[HELIX_PCL] * len /
			((gdouble)CVEL / (gdouble)calc_data.mxfrq) );
		seg_turn = (gint)n;
	  }
	  newpcl = FALSE;
	  save = TRUE;

  } /* switch( action ) */

  /* Separate inner and outer radii of spiral */
  if( spiral_rh || spiral_lh )
  {
	link_b1a2 = FALSE;
	img = GTK_IMAGE(
		Builder_Get_Object(helix_editor_builder, "helix_link_b1a2") );
	gtk_image_set_from_file( img, "/usr/share/xnec2c/unlink.svg" );
  }

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	gdouble len, f, ave_rad;

	/* Average radius of helix or spiral (approximate) */
	ave_rad =
	  ( fv[HELIX_RXZO]  + fv[HELIX_RYZO] +
		fv[HELIX_RXZHL] + fv[HELIX_RYZHL] ) / 4.0;

	/* Pitch angle of helix, assumes untapered helix */
	if( helix_rh || helix_lh )
	  f = atan( fv[HELIX_TSPACE]/(gdouble)M_2PI / ave_rad );
	else f = 0.0;

	/* Helix turn length */
	len = (gdouble)M_2PI * ave_rad / (gdouble)cos( f );

	fv[HELIX_PCL] = 100.0 * (len / (gdouble)seg_turn) /
	  ((gdouble)CVEL / (gdouble)calc_data.mxfrq);
  }
  else newpcl = TRUE;

  /* Calculate new turn spacing for helix */
  if( helix_rh || helix_lh )
  {
	if( !newtsp )
	  fv[HELIX_TSPACE] = helix_len / fv[HELIX_NTURN];
	else newtsp = FALSE;
  }

  /* Write int data to the helix editor */
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(helix_editor_builder, ispin[SPIN_COL_I1]) );
  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(helix_editor_builder, ispin[SPIN_COL_I2]) );
  gtk_spin_button_set_value( spin, seg_turn );

  /* Write float data to the helix editor */
  ftmp = fv[HELIX_DIA];
  fv[HELIX_DIA] *= 2.0;
  fv[HELIX_LEN]  = helix_len;
  if( spiral_rh || spiral_lh )
	fv[HELIX_TSPACE] = 0.0;
  for( idx = HELIX_TSPACE; idx <= HELIX_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(helix_editor_builder, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  fv[HELIX_DIA] = ftmp;

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Helix_Editor() */

/*------------------------------------------------------------------------*/

/* Get_Geometry_Int_Data()
 *
 * Gets integer (I1, I2) geometry data from a treeview row
 */

  static void
Get_Geometry_Int_Data( GtkListStore *store, GtkTreeIter *iter, int *iv )
{
  gint idi;
  gchar *sv;

  /* Get data from tree view (I1, I2) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idi = GEOM_COL_I1; idi <= GEOM_COL_I2; idi++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idi, &sv, -1);
	  iv[idi-GEOM_COL_I1] = atoi(sv);
	  g_free(sv);
	}
  }
  else Stop( _("Error reading row data\n"
		"Invalid list iterator"), ERR_OK );

} /* Get_Geometry_Int_Data() */

/*------------------------------------------------------------------------*/

/* Reflect_Editor()
 *
 * Handles all actions of the reflect editor window
 */
  void
Reflect_Editor( int action )
{
  /* For looking up spinbuttons/checkbuttons */
  GtkSpinButton *spin;
  GtkToggleButton *toggle;

  /* For reading/writing to GX row */
  static GtkTreeIter iter_gx;

  int idx, ck;

  /* Integer type data */
  static gint iv[2];

  /* Check button names */
  gchar *ckbutton[3] =
  {
	"reflect_z_checkbutton",
	"reflect_y_checkbutton",
	"reflect_x_checkbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, reflect_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( reflect_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Geometry_Int_Data( geom_store, &iter_gx, iv );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New reflect row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gx );

	  /* Insert a new blank GX row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gx, "GX");

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a reflect row (GX) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gx, name );

	  /* Get reflect data from tree view */
	  Get_Geometry_Int_Data( geom_store, &iter_gx, iv );

	  /* Set reflection axes check buttons */
	  {
		ck = iv[SPIN_COL_I2];

		for( idx = 0; idx < 3; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(reflect_editor_builder, ckbutton[idx]) );
		  if( ck & 1 )
			gtk_toggle_button_set_active( toggle, TRUE );
		  else
			gtk_toggle_button_set_active( toggle, FALSE );
		  ck /= 10;
		}
	  }

	  /* Set tag num increment */
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(reflect_editor_builder, "reflect_taginc_spinbutton") );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );
	  break;

	case EDITOR_CANCEL: /* Cancel reflect editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gx );
	  save = busy = FALSE;
	  return;

	case REFLECT_EDITOR_TOGGLE: /* Reflect button toggled */
	  save = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Work out the I2 value needed by GX card */
  ck = 1;  iv[SPIN_COL_I2] = 0;
  for( idx = 0; idx < 3; idx++ )
  {
	toggle = GTK_TOGGLE_BUTTON(
		Builder_Get_Object(reflect_editor_builder, ckbutton[idx]) );
	if( gtk_toggle_button_get_active(toggle) )
	  iv[SPIN_COL_I2] += ck;
	ck *= 10;
  }

  /* Read tag inc from the reflect editor */
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(reflect_editor_builder, "reflect_taginc_spinbutton") );
  iv[SPIN_COL_I1] = gtk_spin_button_get_value_as_int( spin );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Reflection_Editor() */

/*------------------------------------------------------------------------*/

/* Scale_Editor()
 *
 * Handles all actions of the scale editor window
 */
  void
Scale_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to GS row */
  static GtkTreeIter iter_gs;

  /* Integer type data */
  static gint iv[2];

  /* Spin button names, int data */
  gchar *ispin[2] =
  {
	"scale_from_spinbutton",
	"scale_to_spinbutton"
  };

  int idx, idi;

  /* Scale factor */
  static gdouble scale = 1.0;

  /* Card (row) name, strings for convertions */
  gchar name[3], sf[13], *str;

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, scale_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( scale_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Clear all GS columns */
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_F7; idx++ )
	  gtk_list_store_set( geom_store, &iter_gs, idx, "0", -1 );

	/* Enter tag from-to data */
	Set_Geometry_Int_Data( geom_store, &iter_gs, iv );

	/* Enter scale factor */
	snprintf( sf, sizeof(sf), "%12.5E", scale );
	gtk_list_store_set(
		geom_store, &iter_gs, GEOM_COL_F1, sf, -1 );

	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New scale row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gs );

	  /* Insert a new blank GS row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gs, "GS");

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  save = FALSE;
	  break;

	case EDITOR_EDIT:  /* Edit a scale row (GS) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gs, name );

	  /* Get tag from-to data from tree view */
	  Get_Geometry_Int_Data( geom_store, &iter_gs, iv );

	  /* Get scale data from tree view */
	  if( gtk_list_store_iter_is_valid(geom_store, &iter_gs) )
	  {
		gtk_tree_model_get(	GTK_TREE_MODEL(geom_store),
			&iter_gs, GEOM_COL_F1, &str, -1);
		scale = Strtod( str, NULL );
		g_free( str );
	  }
	  else
		Stop( _("Error reading row data\n"
			  "Invalid list iterator"), ERR_OK );

	  /* Enter tag from-to data to scale editor */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(scale_editor_builder, ispin[idi]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Set scale factor to scale editor */
	  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
			scale_editor_builder, "scale_factor_spinbutton") );
	  gtk_spin_button_set_value( spin, scale );
	  break;

	case EDITOR_CANCEL: /* Cancel scale editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gs );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  /* Read int data from the scale editor */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(scale_editor_builder, ispin[idi]) );
		iv[idi] = gtk_spin_button_get_value_as_int( spin );
	  }

	  /* Read scale from the scale editor */
	  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
			scale_editor_builder, "scale_factor_spinbutton") );
	  scale = gtk_spin_button_get_value( spin );
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Scale_Editor() */

/*------------------------------------------------------------------------*/

/* Cylinder_Editor()
 *
 * Handles all actions of the cylinder editor window
 */
  void
Cylinder_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to GR row */
  static GtkTreeIter iter_gr;

  int idi;

  /* Integer data (I1 & I2) */
  static gint iv[2];

  /* Cylinder data spinbuttons */
  static gchar *ispin[2] =
  {
	"cylinder_taginc_spinbutton",
	"cylinder_total_spinbutton"
  };

  /* Card (row) name, strings for convertions */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, cylinder_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( cylinder_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Geometry_Int_Data( geom_store, &iter_gr, iv );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New cylinder row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gr );

	  /* Insert a new blank GR row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gr, "GR");

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  save = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a cylinder row (GR) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gr, name);

	  /* Get integer data from cylinder editor */
	  Get_Geometry_Int_Data( geom_store, &iter_gr, iv );

	  /* Write int data to the cylinder editor */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(cylinder_editor_builder, ispin[idi]));
		gtk_spin_button_set_value( spin, iv[idi] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel cylinder editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gr );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the cylinder editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I2; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(cylinder_editor_builder, ispin[idi]));
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Cylinder_Editor() */

/*------------------------------------------------------------------------*/

/* Transform_Editor()
 *
 * Handles all actions of the transform editor window
 */
  void
Transform_Editor( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to GM row */
  static GtkTreeIter iter_gm;

  int idx, idi, idf;

  /* Integer data (I1 to I3) */
  static gint iv[3];

  /* Float data (F1 to F7) */
  static gdouble fv[7];

  /* Transform int data spinbuttons */
  static gchar *ispin[3] =
  {
	"transform_taginc_spinbutton",
	"transform_new_spinbutton",
	"transform_start_spinbutton"
  };

  /* Transform float data spinbuttons */
  static gchar *fspin[6] =
  {
	"transform_rx_spinbutton",
	"transform_ry_spinbutton",
	"transform_rz_spinbutton",
	"transform_mx_spinbutton",
	"transform_my_spinbutton",
	"transform_mz_spinbutton"
  };

  /* Card (row) name, strings for convertions */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, transform_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( transform_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Geometry_Data( geom_store, &iter_gm, iv, fv );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New transform row to create */
	  /* Insert a default GE card if list is empty */
	  Insert_GE_Card( geom_store, &iter_gm );

	  /* Insert a new blank GM row after a selected row,
	   * if any, otherwise before the last (GE) row */
	  Insert_Blank_Geometry_Row(
		  geom_treeview, geom_store, &iter_gm, "GM");

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( geom_adjustment,
		  gtk_adjustment_get_upper(geom_adjustment) -
		  gtk_adjustment_get_page_size(geom_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit transform row (GM) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(geom_treeview, geom_store, &iter_gm, name);

	  /* Get integer data from transform editor */
	  Get_Geometry_Data( geom_store, &iter_gm, iv, fv );

	  /* Write int data to the transform editor */
	  iv[SPIN_COL_I3] = (gint)fv[SPIN_COL_F7];
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I3; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(transform_editor_builder, ispin[idi]));
		gtk_spin_button_set_value( spin, iv[idi] );
	  }
	  /* Write float data to the transform editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(transform_editor_builder, fspin[idf]));
		gtk_spin_button_set_value( spin, fv[idf] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel transform editor */
	  /* Remove cards */
	  Remove_Row( geom_store, &iter_gm );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the transform editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I3; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(transform_editor_builder, ispin[idi]));
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }
  /* Read float data from the transform editor */
  for( idx = 0; idx < 6; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(transform_editor_builder, fspin[idx]));
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[SPIN_COL_F7] = (gdouble)iv[SPIN_COL_I3];

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Transform_Editor() */

/*------------------------------------------------------------------------*/

/* Gend_Editor()
 *
 * Edits the GE card
 */

  void
Gend_Editor( int action )
{
  /* For reading/writing to GE row */
  static GtkTreeIter iter_ge;

  /* For checking radio buttons */
  GtkToggleButton *toggle;

  /* Ground type radio buttons */
#define GE_RDBTN 3
  static gchar *rdbutton[GE_RDBTN] =
  {
	"gend_noimg_radiobutton",
	"gend_nognd_radiobutton",
	"gend_img_radiobutton"
  };

  /* Card (row) name, strings for convertions */
  gchar name[3], *sv;
  static gchar si[6];

  int idx;

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, gend_editor) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( gend_editor );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	if( gtk_list_store_iter_is_valid(geom_store, &iter_ge) )
	{
	  gtk_list_store_set(
		  geom_store, &iter_ge, GEOM_COL_I1, si, -1 );
	  save = FALSE;
	}
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_EDIT:  /* Edit geom end row (GE) */
	  /* Open GE Editor */
	  if( gend_editor == NULL )
	  {
		gend_editor = create_gend_editor( &gend_editor_builder );
		gtk_widget_show( gend_editor );
	  }

	  /* Get selected row */
	  Get_Selected_Row( geom_treeview, geom_store, &iter_ge, name );

	  /* Get integer data from transform editor */
	  if( gtk_list_store_iter_is_valid(geom_store, &iter_ge) )
	  {
		gtk_tree_model_get( GTK_TREE_MODEL(geom_store),
			&iter_ge, GEOM_COL_I1, &sv, -1 );
		idx = atoi(sv) + 1;
		g_free(sv);
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(gend_editor_builder, rdbutton[idx]) );
		gtk_toggle_button_set_active( toggle, TRUE );
	  }
	  else Stop( _("Error reading row data\n"
			"Invalid list iterator"), ERR_OK );
	  break;

	case EDITOR_CANCEL: /* Cancel transform editor */
	  /* Remove card */
	  Remove_Row( geom_store, &iter_ge );
	  save = busy = FALSE;
	  return;

	case EDITOR_RDBUTTON: /* Radio button toggled in editor window */
	  /* Test radio buttons */
	  for( idx = 0; idx < GE_RDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(gend_editor_builder, rdbutton[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  snprintf( si, sizeof(si), "%5d", idx-1 );
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Gend_Editor() */

/*------------------------------------------------------------------------*/

/* Check_Card_Name()
 *
 * Checks previous or next card's name, returns TRUE on match
 */

  gboolean
Check_Card_Name(
	GtkListStore *store,
	GtkTreeIter *iter,
	gboolean next,
	const gchar *name )
{
  gboolean retv;
  char *str;

  retv = FALSE;
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	if( next )
	{
	  if( gtk_tree_model_iter_next(GTK_TREE_MODEL(store), iter) )
	  {
		gtk_tree_model_get( GTK_TREE_MODEL(store),
			iter, GEOM_COL_NAME, &str, -1);
		if( strcmp(name, str) == 0 ) retv = TRUE;
		g_free(str);
	  }
	}
	else
	{
	  if( gtk_tree_model_iter_previous(GTK_TREE_MODEL(store), iter) )
	  {
		gtk_tree_model_get( GTK_TREE_MODEL(store), iter,
			GEOM_COL_NAME, &str, -1);
		if( strcmp(name, str) == 0 ) retv = TRUE;
		g_free(str);
	  }
	}
  }

  return( retv );

} /* Check_Card_Name() */

/*------------------------------------------------------------------------*/

/* Give_Up()
 *
 * Signals functon to abort if busy or no NEC2 editor window
 */

  gboolean
Give_Up( gboolean *busy, GtkWidget *widget )
{
  /* Block callbacks. (Should be a better way to do this) */
  if( *busy ) return( TRUE );
  *busy = TRUE;

  /* Abort if NEC2 editor window is closed */
  if( nec2_edit_window == NULL )
  {
	Stop( _("NEC2 editor window not open"), ERR_OK );
	Gtk_Widget_Destroy( widget );
	*busy = FALSE;
	return( TRUE );
  }

  return( FALSE );

} /* Give_Up() */

/*------------------------------------------------------------------------*/

/* Remove_Row()
 *
 * Removes a row and selects previous or next row
 */

  void
Remove_Row( GtkListStore *store, GtkTreeIter *iter )
{
  if( gtk_list_store_iter_is_valid(store, iter) )
	gtk_list_store_remove( store, iter );

} /* Remove_Row() */

/*------------------------------------------------------------------------*/

/* Get_Selected_Row()
 *
 * Gets the selected row in a tree view
 */

  gboolean
Get_Selected_Row(
	GtkTreeView *view,
	GtkListStore *store,
	GtkTreeIter *iter,
	gchar *name )
{
  gchar *str;
  GtkTreeSelection *selection;

  /* Get selected row, if any */
  selection = gtk_tree_view_get_selection( view );
  if( !gtk_tree_selection_get_selected(selection, NULL, iter) )
	return( FALSE );

  /* Get row name */
  gtk_tree_model_get(
	  GTK_TREE_MODEL(store), iter, GEOM_COL_NAME, &str, -1);
  Strlcpy( name, str, 3 );
  g_free( str );

  return( TRUE );

} /* Get_Selected_Row() */

/*------------------------------------------------------------------------*/

