/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2006 N. Kyriazis <neoklis<at>mailspeed.net>
 *
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

/* editors.c
 *
 * Structure/Geometry editor functions for xnec2c
 */

#include "xnec2c.h"
#include "interface.h"
#include "support.h"
#include "editors.h"

/* Global tag number for geometry editors */
gint gbl_tag_num = 0;

/* Tree list stores */
extern GtkListStore
  *cmnt_store,
  *geom_store,
  *cmnd_store;

/* Geometry tree view */
extern GtkTreeView *geom_treeview;

/* Scrolled winodws adjustments */
extern GtkAdjustment
  *geom_adjustment,
  *cmnd_adjustment;

extern GtkWidget
  *wire_editor,	 	 /* Wire designer window  */
  *arc_editor,	 	 /* Arc designer window   */
  *helix_editor, 	 /* Helix designer window */
  *patch_editor,	 /* Patch designer window */
  *reflect_editor,	 /* Reflect design window */
  *scale_editor,	 /* Scale designer window */
  *cylinder_editor,	 /* Cylinder designer window  */
  *transform_editor, /* Transform designer window */
  *gend_editor,		 /* Geom End designer window  */
  *nec2_edit_window; /* NEC2 file editor's window */

/* Various data used by NEC2, stored in this struct */
extern calc_data_t calc_data;

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

  int idx;

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
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

	save = load = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the wire editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(wire_editor, ispin[idx]) );
	iv[idx] = gtk_spin_button_get_value_as_int( spin );
  }
  /* Read float data from the wire editor */
  for( idx = WIRE_X1; idx <= WIRE_LEN; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(wire_editor, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[WIRE_DIA] /= 2.0; fv[WIRE_DIA1] /= 2.0; fv[WIRE_DIAN] /= 2.0;

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
	  iv[SPIN_COL_I1] = ++gbl_tag_num;
	  if( s > 0.0 )	load = TRUE; /* Enable wire loading */
	  iv[SPIN_COL_I3] = iv[SPIN_COL_I4] = 0;

	  if( taper )
	  {
		/* Insert a new blank GC row if tapered wire */
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_gc, "GC" );
		fv[WIRE_DIA]  = 0.0;
	  }

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper );
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
			stop( "GC card preceded by GW card\n"
				"with non-zero wire radius", 0 );

		} /* if( strcmp(name, "GC") == 0 ) */
		else
		  stop( "No GW card before GC card", 0 );
	  }
	  else /*** Editing a GW card ***/
	  {
		taper = FALSE;

		/* Get wire data from tree view */
		Get_Geometry_Data( geom_store, &iter_gw, iv, fv );

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
		  else
			stop( "No GC card after a GW card\n"
				"with a zero wire radius", 0 );

		} /* if( fv[WIRE_DIA] == 0.0 ) */
		else /* If radius != 0, next card should not be GC */
		  if( Check_Card_Name(geom_store, &iter_gc, NEXT, "GC") )
			stop( "GC card follows a GW card\n"
				"with non-zero wire radius", 0 );

	  } /* if( strcmp(name, "GC") == 0 ) */
	  break;

	case WIRE_EDITOR_TAPR: /* Show taper wire data if button checked */
	  {
		/* Wire diameter */
		static double diam;

		/* Read tapered wire checkbutton */
		taper = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(
			  lookup_widget(wire_editor, "wire_taper_checkbutton")) );
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
		  Remove_Row( geom_treeview, geom_store, &iter_gc );
		} /* if( taper ) */
	  }
	  save = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel wire editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gw );
	  if( taper )
		Remove_Row( geom_treeview, geom_store, &iter_gc );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON( lookup_widget(
			wire_editor, fspin[WIRE_RES]) );
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 ) save = load = TRUE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  gbl_tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 )	load = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		if( taper && ( fv[WIRE_RLEN] != 1.0) ) /* Taper ratio < 1 */
		{
		  double cnt = 1.0 / (fv[WIRE_PCL]/100.0) * (1.0-fv[WIRE_RLEN]);
		  if( cnt < 1.0 )
			iv[SPIN_COL_I2] = ceil( log(1.0-cnt) / log(fv[WIRE_RLEN]) );
		}
		else
		  iv[SPIN_COL_I2] = ceil( 100.0 * (fv[WIRE_LEN]/fv[WIRE_PCL]) /
			  (CVEL/calc_data.mxfrq) );
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
  frame = lookup_widget(wire_editor, "wire_taperframe");
  /* Show taper data if appropriate */
  if( taper )
  {
	gtk_widget_show( frame );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  lookup_widget(wire_editor,
			"wire_taper_checkbutton")), TRUE );
  }
  else
  {
	gtk_widget_hide( frame );
	gtk_window_resize( GTK_WINDOW(wire_editor), 10, 10 );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  lookup_widget(wire_editor,
			"wire_taper_checkbutton")), FALSE );
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
  else
	newwln = TRUE;

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	if( taper && (fv[WIRE_RLEN] != 1.0) )
	  fv[WIRE_PCL] =
		100.0 * fv[WIRE_LEN] * (1.0-fv[WIRE_RLEN]) /
		(1.0-pow(fv[WIRE_RLEN], (double)iv[SPIN_COL_I2])) /
		(CVEL/calc_data.mxfrq);
	else
	  fv[WIRE_PCL] = 100.0 * (fv[WIRE_LEN] /
		  (double)iv[SPIN_COL_I2]) / (CVEL/calc_data.mxfrq);
  }
  else
	newpcl = TRUE;

  /*** Calculate radius taper ratio ***/
  if( (iv[SPIN_COL_I2] > 1) && newrdm )
	fv[WIRE_RDIA] = pow( fv[WIRE_DIAN]/fv[WIRE_DIA1],
		1.0/(double)(iv[SPIN_COL_I2]-1) );
  else
	newrdm = TRUE;

  /* Write int data to the wire editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(wire_editor, ispin[idx]) );
	gtk_spin_button_set_value( spin, iv[idx] );
  }

  /* Write float data to the wire editor (F1 to F7 and taper) */
  fv[WIRE_DIA] *= 2.0; fv[WIRE_DIA1] *= 2.0; fv[WIRE_DIAN] *= 2.0;
  for( idx = WIRE_X1; idx <= WIRE_LEN; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(wire_editor, fspin[idx]) );
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

  int idx;

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
	"patch_arbitary_radiobutton",
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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
  {
	/* Clear data not used in SC card */
	iv[SPIN_COL_I3]  = iv[SPIN_COL_I4]  = 0;
	fv[UNUSED_F1] = fv[UNUSED_F2] = 0.0;

	/* Set SP data to treeview */
	Set_Geometry_Data( geom_store, &iter_sp, iv, fv );

	/* Set SC card data to treeview if non arbitary */
	if( ptype != PATCH_ARBT )
	  Set_Geometry_Data(geom_store,
		  &iter_sc, &iv[SPIN_COL_I3], &fv[PATCH_X3]);

	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Set int data from the patch editor (SP card) */
  if( ptype != PATCH_SURF )
  {
	iv[SPIN_COL_I1] = 0; 	  /* Not used in SP */
	iv[SPIN_COL_I2] = ptype; /* Patch type */
  }
  else
	/* Read int data from the patch editor (SM card) */
	for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
	{
	  spin = GTK_SPIN_BUTTON(
		  lookup_widget(patch_editor, ispin[idx]) );
	  iv[idx] = gtk_spin_button_get_value( spin );
	}

  /* Read float data from the patch editor */
  for( idx = PATCH_X1; idx <= PATCH_Z2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(patch_editor, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  for( idx = PATCH_X3; idx <= PATCH_Z4; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(patch_editor, fspin[idx-1]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }

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

	  /* Insert an SC card for non-arbitary patch */
	  if( ptype != PATCH_ARBT )
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_sc, "SC" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper );
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

		  /* Warn user if SP card with arbitary
		   * patch is followed by an SC card */
		  if( ptype == PATCH_ARBT )
			stop( "SC card preceded by SP card\n"
				"with arbitary patch type", 0 );

		} /* if( Check_Card_Name(geom_store, &iter_sp, PREVIOUS, "SP") ) */
		else /* Look for a previous SM card */
		{
		  iter_sp = iter_sc;
		  if( Check_Card_Name(geom_store, &iter_sp, PREVIOUS, "SM") )
		  {
			Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
			ptype = PATCH_SURF;
		  }
		  else
			stop( "No SP or SM card before SC card", 0 );
		}
	  } /* if( strcmp(name, "SC") == 0 ) */
	  else
		/*** Editing an SP|SM card ***/
		if( strcmp(name, "SP") == 0 )
		{
		  /* Get patch data from treeview */
		  Get_Geometry_Data( geom_store, &iter_sp, iv, fv );
		  ptype = iv[SPIN_COL_I2];

		  /*** Get SC card data if patch type non-arbitary ***/
		  if( ptype != PATCH_ARBT )
		  {
			/* If next card is SC, get data */
			if( Check_Card_Name(geom_store, &iter_sc, NEXT, "SC") )
			  Get_Geometry_Data( geom_store, &iter_sc,
				  &iv[SPIN_COL_I3], &fv[PATCH_X3] );
			else
			{
			  ptype = PATCH_ARBT;
			  stop( "No SC card after an SP card\n"
				  "with non-arbitary patch type", 0 );
			}

		  } /* if( ptype != PATCH_ARBT ) */
		  else /* If patch type arbitary, no SC card should follow */
			if( Check_Card_Name(geom_store, &iter_sc, NEXT, "SC") )
			  stop( "SC card follows an SP card\n"
				  "with arbitary patch type", 0 );
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
		  else
			stop( "No SC card after an SM card", 0 );

		} /* if( strcmp(name, "SP") == 0 ) */

	  ptset = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel patch editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_sp );
	  if( ptype != PATCH_ARBT )
		Remove_Row( geom_treeview, geom_store, &iter_sc );
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
	  Remove_Row( geom_treeview, geom_store, &iter_sc );
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

	case PATCH_EDITOR_SCCD: /* Insert SC card (non-arbitary patch case) */
	  if( gtk_list_store_iter_is_valid( geom_store, &iter_sp) )
		Insert_Blank_Geometry_Row(
			geom_treeview, geom_store, &iter_sc, "SC");

  } /* switch( action ) */

  /* Change labels as needed */
  if( ptype == PATCH_ARBT )
  {
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_x1_label")), "Center - X" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_y1_label")), "Center - Y" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_z1_label")), "Center - Z" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_x2_label")), "Normal - Elev." );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_y2_label")), "Normal - Azim." );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_z2_label")), "Patch Area" );
  }
  else
  {
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_x1_label")), "Corner 1 - X" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_y1_label")), "Corner 1 - Y" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_z1_label")), "Corner 1 - Z" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_x2_label")), "Corner 2 - X" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_y2_label")), "Corner 2 - Y" );
	gtk_label_set_text( GTK_LABEL(
		  lookup_widget(patch_editor, "patch_z2_label")), "Corner 2 - Z" );
  }

  /* Hide/Show parts of window as needed */
  switch( ptype )
  {
	case PATCH_ARBT: /* Arbitary shaped patch */
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sc_frame") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_RECT: /* Rectangular patch */
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sc_frame") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sc_table") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_TRIA: /* Triangular patch */
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sc_frame") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sc_table") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_QUAD: /* Quadrilateral patch */
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sc_frame") );
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sc_table") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sm_frame") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );
	  break;

	case PATCH_SURF: /* Multi-patch surface */
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sm_frame") );
	  gtk_widget_show( lookup_widget(patch_editor, "patch_sc_frame") );
	  gtk_widget_hide( lookup_widget(patch_editor, "patch_sc_table") );
	  gtk_window_resize( GTK_WINDOW(patch_editor), 10, 10 );

  } /* switch( ptype ) */

  /* Set patch type radio button */
  if( ptset )
  {
	ptset = FALSE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		  lookup_widget(patch_editor, rbutton[ptype])), TRUE );
  }

  /* Set card name */
  strcpy( name, (ptype == PATCH_SURF ? "SM" : "SP") );
  if( gtk_list_store_iter_is_valid(geom_store, &iter_sp) && save )
	gtk_list_store_set( geom_store, &iter_sp, GEOM_COL_NAME, name, -1 );

  /* Write int data for SM card */
  if( ptype == PATCH_SURF )
	for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
	{
	  spin = GTK_SPIN_BUTTON( lookup_widget(patch_editor, ispin[idx]) );
	  gtk_spin_button_set_value( spin, iv[idx] );
	}

  /* Write float data to the patch editor */
  for( idx = PATCH_X1; idx <= PATCH_Z2; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(patch_editor, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  for( idx = PATCH_X3; idx <= PATCH_Z4; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(patch_editor, fspin[idx-1]) );
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

  int idx;

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
  {
	Set_Geometry_Data( geom_store, &iter_ga, iv, fv );

	/* Set wire conductivity (loading card) */
	if( load )
	  Set_Wire_Conductivity( iv[SPIN_COL_I1], s, cmnd_store );

	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the arc editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(arc_editor, ispin[idx]) );
	iv[idx] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the arc editor */
  for( idx = ARC_RAD; idx <= ARC_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(arc_editor, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[ARC_DIA] /= 2.0;

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
	  iv[SPIN_COL_I1] = ++gbl_tag_num;
	  spin = GTK_SPIN_BUTTON(
		  lookup_widget(arc_editor, ispin[SPIN_COL_I1]));
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
	  break;

	case EDITOR_EDIT:  /* Edit an arc row (GA) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_ga, name );

	  /* Get arc data from tree view */
	  Get_Geometry_Data(
		  geom_store, &iter_ga, iv, fv );
	  break;

	case EDITOR_CANCEL: /* Cancel arc editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_ga );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON( lookup_widget(
			arc_editor, fspin[SPIN_COL_F6]) );
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 ) save = load = TRUE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  gbl_tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 )	load = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		gdouble len = fv[ARC_RAD] *
		  fabs( fv[ARC_END1]-fv[ARC_END2] )/TD;
		iv[SPIN_COL_I2] = ceil(100.0 / fv[ARC_PCL] *
			len /(CVEL/calc_data.mxfrq));
	  }
	  newpcl = FALSE;
	  save = TRUE;

  } /* switch( action ) */

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	gdouble len = fv[ARC_RAD] *
	  fabs( fv[ARC_END1]-fv[ARC_END2] )/TD;
	fv[ARC_PCL] = 100.0 * (len/(gdouble)iv[SPIN_COL_I2]) /
	  (CVEL/calc_data.mxfrq);
  }
  else
	newpcl = TRUE;

  /* Write int data to the arc editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(arc_editor, ispin[idx]) );
	gtk_spin_button_set_value( spin, iv[idx] );
  }

  /* Write float data to the arc editor (F1 to F4 & pcl/resistance) */
  fv[ARC_DIA] *= 2.0;
  for( idx = ARC_RAD; idx <= ARC_RES; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(arc_editor, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  fv[ARC_DIA] /= 2.0;

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

  /* For reading/writing to GH rows */
  static GtkTreeIter iter_gh;

  gint idx;
  gdouble ftmp;

  static gboolean
	load    = FALSE, /* Enable wire loading (conductivity specified) */
	linkall = TRUE,  /* Link all radius spinbuttons  */
	linkzo  = FALSE, /* Link X, Y @ Z=0 spinbuttons  */
	linkzhl = FALSE, /* Link X, Y @ Z=HL spinbuttons */
	helixlh = FALSE, /* Specify a left hand helix */
	newpcl  = TRUE,  /* New percent-of-lambda value  */
	newspc  = TRUE,  /* New percent-of-lambda value  */
	save    = FALSE, /* Enable saving of editor data */
	busy    = FALSE; /* Block callbacks. Must be a better way to do this? */

  /* Float type data, wire conductivity */
  static gdouble fv[9], s = 0.0;

  /* Integer type data */
  static gint iv[2];

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
  {
	/* Change seg/turn to total number of segs */
	gint tmp = iv[SPIN_COL_I2];
	iv[SPIN_COL_I2] = ceil(
		(gdouble)iv[SPIN_COL_I2] * fv[HELIX_NTURN] );

	/* Change to left hand helix */
	if( helixlh )
	  fv[HELIX_LEN] = -fv[HELIX_LEN];

	Set_Geometry_Data( geom_store, &iter_gh, iv, fv );

	/* Change back */
	iv[SPIN_COL_I2] = tmp;
	fv[HELIX_LEN] = fabs( fv[HELIX_LEN] );

	/* Set wire conductivity (loading card) */
	if( load )
	  Set_Wire_Conductivity( iv[SPIN_COL_I1], s, cmnd_store );

	save = load = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the helix editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(helix_editor, ispin[idx]) );
	iv[idx] = gtk_spin_button_get_value_as_int( spin );
  }
  /* Read float data from the helix editor */
  for( idx = HELIX_TSPACE; idx <= HELIX_NTURN; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(helix_editor, fspin[idx]) );
	fv[idx] = gtk_spin_button_get_value( spin );
  }
  fv[HELIX_DIA] /= 2.0;

  /* Link all radius spinbuttons to X @ Z=0 */
  if( linkall )
	fv[HELIX_RYZO] = fv[HELIX_RXZHL] = fv[HELIX_RYZHL] = fv[HELIX_RXZO];

  /* Link X, Y @ Z=0 spinbuttons */
  if( linkzo )
	fv[HELIX_RYZO] = fv[HELIX_RXZO];

  /* Link X, Y @ Z=0 spinbuttons */
  if( linkzhl )
	fv[HELIX_RYZHL] = fv[HELIX_RXZHL];

  /* Respond to user action */
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
	  iv[SPIN_COL_I1] = ++gbl_tag_num;
	  spin = GTK_SPIN_BUTTON( lookup_widget(
			helix_editor, ispin[SPIN_COL_I1]) );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
	  break;

	case EDITOR_EDIT:  /* Edit a helix row (GH) */
	  /* Get selected row */
	  Get_Selected_Row( geom_treeview, geom_store, &iter_gh, name );

	  /* Get helix data from tree view */
	  Get_Geometry_Data( geom_store, &iter_gh, iv, fv );

	  /* Set LH/RH helix check button */
	  {
		GtkToggleButton *toggle =
		  GTK_TOGGLE_BUTTON( lookup_widget(
				helix_editor, "helix_lh_checkbutton") );
		if( fv[HELIX_LEN] < 0.0 )
		  gtk_toggle_button_set_active( toggle, TRUE );
		else
		  gtk_toggle_button_set_active( toggle, FALSE );
	  }

	  /* If left hand helix */
	  fv[HELIX_LEN] = fabs( fv[HELIX_LEN] );

	  /* Change to number of segs/turn */
	  fv[HELIX_NTURN] = fv[HELIX_LEN] / fv[HELIX_TSPACE];
	  iv[SPIN_COL_I2] =
		(gint)((gdouble)iv[SPIN_COL_I2] / fv[HELIX_NTURN]);
	  break;

	case EDITOR_CANCEL: /* Cancel helix editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gh );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;
	  break;

	case EDITOR_LOAD: /* Wire conductivity specified */
	  spin = GTK_SPIN_BUTTON(lookup_widget(
			helix_editor, fspin[HELIX_RES]));
	  s = gtk_spin_button_get_value( spin );
	  if( s > 0.0 ) save = load = TRUE;
	  break;

	case EDITOR_TAGNUM: /* Tag number edited by user */
	  gbl_tag_num = iv[SPIN_COL_I1];
	  save = TRUE;
	  if( s > 0.0 )	load = TRUE;
	  break;

	case HELIX_EDITOR_LH: /* Left hand helix */
	  helixlh = TRUE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_RH: /* Right hand helix */
	  helixlh = FALSE;
	  save = TRUE;
	  break;

	case HELIX_EDITOR_LINKALL: /* Link all radius spinbuttons */
	  linkall = TRUE;
	  linkzo  = linkzhl = FALSE;
	  break;

	case HELIX_EDITOR_LINKZO: /* Link X, Y @ Z=0 spinbuttons */
	  linkzo = TRUE;
	  linkall = linkzhl = FALSE;
	  break;

	case HELIX_EDITOR_LINKZHL: /* Link X, Y @ Z=HL spinbuttons */
	  linkzhl = TRUE;
	  linkzo  = linkall = FALSE;
	  break;

	case HELIX_EDITOR_NTURN: /* New number of turns */
	  fv[HELIX_TSPACE] = fv[HELIX_LEN] / fv[HELIX_NTURN];
	  newspc = FALSE;
	  save = TRUE;
	  break;

	case EDITOR_SEGPC: /* Segment length as % of smallest wavelength */
	  /* Calculate num of segs for given % of lambda */
	  if( calc_data.mxfrq != 0.0 )
	  {
		gdouble len, f;

		/* Pitch angle of helix, assumes untapered helix */
		f = asin( fv[HELIX_TSPACE] / TP / fv[HELIX_RXZO] );

		/* Helix turn length */
		len = TP * fv[HELIX_RXZO] / cos( f );

		/* New number of segments */
		iv[SPIN_COL_I2] = ceil(100.0 / fv[HELIX_PCL]*len /
			(CVEL / calc_data.mxfrq) );
	  }
	  newpcl = FALSE;
	  save = TRUE;

  } /* switch( action ) */

  /*** Calculate seg length as % of smallest wavelength ***/
  if( (calc_data.mxfrq != 0.0) && newpcl )
  {
	gdouble len, f;

	/* Pitch angle of helix, assumes untapered helix */
	f = asin( fv[HELIX_TSPACE]/TP/fv[HELIX_RXZO] );

	/* Helix turn length */
	len = TP * fv[HELIX_RXZO] / cos( f );

	fv[HELIX_PCL] = 100.0 * (len/(gdouble)iv[SPIN_COL_I2]) /
	  (CVEL/calc_data.mxfrq);
  }
  else
	newpcl = TRUE;

  /* Calculate new turn spacing */
  if( newspc )
	fv[HELIX_NTURN] = fv[HELIX_LEN] / fv[HELIX_TSPACE];
  else
	newspc = TRUE;

  /* Write int data to the helix editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(
		  helix_editor, ispin[idx]) );
	gtk_spin_button_set_value( spin, iv[idx] );
  }

  /* Write float data to the helix editor */
  ftmp = fv[HELIX_DIA];
  fv[HELIX_DIA] *= 2.0;
  for( idx = HELIX_TSPACE; idx <= HELIX_NTURN; idx++ )
  {
	spin = GTK_SPIN_BUTTON( lookup_widget(
		  helix_editor, fspin[idx]) );
	gtk_spin_button_set_value( spin, fv[idx] );
  }
  fv[HELIX_DIA] = ftmp;

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Helix_Editor() */

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
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
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
	  break;

	case EDITOR_EDIT:  /* Edit a reflect row (GX) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gx, name );

	  /* Get reflect data from tree view */
	  Get_Geometry_Int_Data( geom_store, &iter_gx, iv );

	  /* Set reflection axes check buttons */
	  {
		gint ck = iv[SPIN_COL_I2];

		for( idx = 0; idx < 3; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  lookup_widget(reflect_editor, ckbutton[idx]) );
		  if( ck & 1 )
			gtk_toggle_button_set_active( toggle, TRUE );
		  else
			gtk_toggle_button_set_active( toggle, FALSE );
		  ck /= 10;
		}
	  }

	  /* Set tag num increment */
	  spin = GTK_SPIN_BUTTON(
		  lookup_widget(reflect_editor, "reflect_taginc_spinbutton") );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I1] );
	  break;

	case EDITOR_CANCEL: /* Cancel reflect editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gx );
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
		lookup_widget(reflect_editor, ckbutton[idx]) );
	if( gtk_toggle_button_get_active(toggle) )
	  iv[SPIN_COL_I2] += ck;
	ck *= 10;
  }

  /* Read tag inc from the reflect editor */
  spin = GTK_SPIN_BUTTON(
	  lookup_widget(reflect_editor,
		"reflect_taginc_spinbutton") );
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

  int idx;

  /* Scale factor */
  static gdouble scale = 1.0;

  /* Card (row) name, strings for convertions */
  gchar name[3], sf[13], *str;

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, scale_editor) ) return;

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
  {
	/* Clear all GS columns */
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_F7; idx++ )
	  gtk_list_store_set( geom_store, &iter_gs, idx, "0", -1 );

	/* Enter tag from-to data */
	Set_Geometry_Int_Data( geom_store, &iter_gs, iv );

	/* Enter scale factor */
	snprintf( sf, 13, "%12.5E", scale );
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
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
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
		scale = atof( str );
		g_free( str );
	  }
	  else
		stop( "Error reading row data\n"
			"Invalid list iterator", 0 );

	  /* Enter tag from-to data to scale editor */
	  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
	  {
		spin = GTK_SPIN_BUTTON(
			lookup_widget(scale_editor, ispin[idx]) );
		gtk_spin_button_set_value( spin, iv[idx] );
	  }

	  /* Set scale factor to scale editor */
	  spin = GTK_SPIN_BUTTON(
		  lookup_widget(scale_editor, "scale_factor_spinbutton") );
	  gtk_spin_button_set_value( spin, scale );
	  break;

	case EDITOR_CANCEL: /* Cancel scale editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gs );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  /* Read int data from the scale editor */
	  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
	  {
		spin = GTK_SPIN_BUTTON(
			lookup_widget(scale_editor, ispin[idx]) );
		iv[idx] = gtk_spin_button_get_value_as_int( spin );
	  }

	  /* Read scale from the scale editor */
	  spin = GTK_SPIN_BUTTON(
		  lookup_widget(scale_editor, "scale_factor_spinbutton") );
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

  int idx;

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
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
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
	  save = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a cylinder row (GR) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(
		  geom_treeview, geom_store, &iter_gr, name);

	  /* Get integer data from cylinder editor */
	  Get_Geometry_Int_Data( geom_store, &iter_gr, iv );

	  /* Write int data to the cylinder editor */
	  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
	  {
		spin = GTK_SPIN_BUTTON(
			lookup_widget(cylinder_editor, ispin[idx]));
		gtk_spin_button_set_value( spin, iv[idx] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel cylinder editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gr );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the cylinder editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I2; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(cylinder_editor, ispin[idx]));
	iv[idx] = gtk_spin_button_get_value_as_int( spin );
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

  int idx;

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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save )
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
	  gtk_adjustment_set_value(
		  geom_adjustment, geom_adjustment->upper);
	  break;

	case EDITOR_EDIT:  /* Edit transform row (GM) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row(geom_treeview, geom_store, &iter_gm, name);

	  /* Get integer data from transform editor */
	  Get_Geometry_Data( geom_store, &iter_gm, iv, fv );

	  /* Write int data to the transform editor */
	  iv[SPIN_COL_I3] = (gint)fv[SPIN_COL_F7];
	  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I3; idx++ )
	  {
		spin = GTK_SPIN_BUTTON(
			lookup_widget(transform_editor, ispin[idx]));
		gtk_spin_button_set_value( spin, iv[idx] );
	  }
	  /* Write float data to the transform editor */
	  for( idx = SPIN_COL_F1; idx <= SPIN_COL_F6; idx++ )
	  {
		spin = GTK_SPIN_BUTTON(
			lookup_widget(transform_editor, fspin[idx]));
		gtk_spin_button_set_value( spin, fv[idx] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel transform editor */
	  /* Remove cards */
	  Remove_Row( geom_treeview, geom_store, &iter_gm );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the transform editor */
  for( idx = SPIN_COL_I1; idx <= SPIN_COL_I3; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(transform_editor, ispin[idx]));
	iv[idx] = gtk_spin_button_get_value_as_int( spin );
  }
  /* Read float data from the transform editor */
  for( idx = 0; idx < 6; idx++ )
  {
	spin = GTK_SPIN_BUTTON(
		lookup_widget(transform_editor, fspin[idx]));
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

  /* Save data to nec2 editor if appropriate */
  if( (action & EDITOR_SAVE) && save &&
	  gtk_list_store_iter_is_valid(geom_store, &iter_ge))
  {
	gtk_list_store_set(
		geom_store, &iter_ge, GEOM_COL_I1, si, -1 );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_EDIT:  /* Edit geom end row (GE) */
	  /* Open GE Editor */
	  if( gend_editor == NULL )
	  {
		gend_editor = create_gend_editor();
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
		toggle = GTK_TOGGLE_BUTTON( lookup_widget(
			  gend_editor, rdbutton[idx]) );
		gtk_toggle_button_set_active( toggle, TRUE );
	  }
	  else
		stop( "Error reading row data\n"
			"Invalid list iterator", 0 );
	  break;

	case EDITOR_CANCEL: /* Cancel transform editor */
	  /* Remove card */
	  Remove_Row( geom_treeview, geom_store, &iter_ge );
	  save = busy = FALSE;
	  return;

	case EDITOR_RDBUTTON: /* Radio button toggled in editor window */
	  /* Test radio buttons */
	  for( idx = 0; idx < GE_RDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON( lookup_widget(
			  gend_editor, rdbutton[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  snprintf( si, 6, "%5d", idx-1 );
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Gend_Editor() */

/*------------------------------------------------------------------------*/

/* Insert_GE_Card()
 *
 * Inserts a default GE card if missing
 */

  void
Insert_GE_Card( GtkListStore *store, GtkTreeIter *iter )
{
  gint idx;

  /* Insert default GE card if list is clear */
  idx = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL );
  if( !idx )
  {
	gtk_list_store_append( store, iter );
	gtk_list_store_set( store, iter, GEOM_COL_NAME, "GE", -1 );
	for( idx = GEOM_COL_I1; idx < GEOM_NUM_COLS; idx++ )
	  gtk_list_store_set( store, iter, idx, "0", -1 );
  }

} /* Insert_GE_Card() */

/*------------------------------------------------------------------------*/

/* Get_Geometry_Data()
 *
 * Gets geometry data from a treeview row
 */

void
Get_Geometry_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gint idx;
  gchar *sv;

  /* Get data from tree view (I1,I2, F1-F7)*/
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_I2; idx++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idx, &sv, -1);
	  iv[idx-GEOM_COL_I1] = atoi(sv);
	  g_free(sv);
	}
	for( idx = GEOM_COL_F1; idx <= GEOM_COL_F7; idx++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idx, &sv, -1);
	  fv[idx-GEOM_COL_F1] = atof(sv);
	  g_free(sv);
	}
  }
  else
	stop( "Error reading row data\n"
		"Invalid list iterator", 0 );

} /* Get_Geometry_Data() */

/*------------------------------------------------------------------------*/

/* Get_Geometry_Int_Data()
 *
 * Gets integer (I1, I2) geometry data from a treeview row
 */

  void
Get_Geometry_Int_Data( GtkListStore *store, GtkTreeIter *iter, int *iv )
{
  gint idx;
  gchar *sv;

  /* Get data from tree view (I1, I2) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_I2; idx++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idx, &sv, -1);
	  iv[idx-GEOM_COL_I1] = atoi(sv);
	  g_free(sv);
	}
  }
  else
	stop( "Error reading row data\n"
		"Invalid list iterator", 0 );

} /* Get_Geometry_Int_Data() */

/*------------------------------------------------------------------------*/

/* Set_Geometry_Data()
 *
 * Sets data into a geometry row
 */

void
Set_Geometry_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gchar str[13];
  gint idx;

  /* Format and set editor data to treeview (I1, I2 & F1-F7) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_I2; idx++ )
	{
	  snprintf( str, 6, "%5d", iv[idx-GEOM_COL_I1] );
	  gtk_list_store_set( store, iter, idx, str, -1 );
	}

	for( idx = GEOM_COL_F1; idx <= GEOM_COL_F7; idx++ )
	{
	  snprintf( str, 13, "%12.5E", fv[idx-GEOM_COL_F1] );
	  gtk_list_store_set( store, iter, idx, str, -1 );
	}
  }
  else
	stop( "Error writing row data\n"
		"Please re-select row", 0 );

  SetFlag( NEC2_EDIT_SAVE );

} /* Set_Geometry_Data() */

/*------------------------------------------------------------------------*/

/* Set_Geometry_Int_Data()
 *
 * Sets integer (I1, I2) data into a geometry row
 */

  void
Set_Geometry_Int_Data( GtkListStore *store, GtkTreeIter *iter, int *iv )
{
  gchar str[6];
  gint idx;

  /* Format and set editor data to treeview (I1, I2) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_I2; idx++ )
	{
	  snprintf( str, 6, "%5d", iv[idx-GEOM_COL_I1] );
	  gtk_list_store_set( store, iter, idx, str, -1 );
	}

	/* Clear unused float columns */
	for( idx = GEOM_COL_F1; idx <= GEOM_COL_F7; idx++ )
	  gtk_list_store_set( store, iter, idx, "0.0", -1 );
  }
  else
	stop( "Error writing row data\n"
		"Please re-select row", 0 );

  SetFlag( NEC2_EDIT_SAVE );

} /* Set_Geometry_Int_Data() */

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
Give_Up( int *busy, GtkWidget *widget )
{
  /* Block callbacks. (Should be a better way to do this) */
  if( *busy ) return( TRUE );
  *busy = TRUE;

  /* Abort if NEC2 editor window is closed */
  if( nec2_edit_window == NULL )
  {
	stop( "NEC2 editor window not open", 0 );
	gtk_widget_destroy( widget );
	*busy = FALSE;
	return( TRUE );
  }

  return( FALSE );

} /* Give_Up() */

/*------------------------------------------------------------------------*/

/* Insert_Blank_Geometry_Row()
 *
 * Inserts a blank row in a tree view with only its name (GW ... )
 */

void
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

/* Remove_Row()
 *
 * Removes a row and selects previous or next row
 */

void
Remove_Row(
	GtkTreeView *view, GtkListStore *store,	GtkTreeIter *iter )
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
  strncpy( name, str, 2 );
  name[2] = '\0';
  g_free( str );

  return( TRUE );

} /* Get_Selected_Row() */

/*------------------------------------------------------------------------*/

/* Set_Wire_Conductivity()
 *
 * Sets the wire conductivity specified in a geometry editor
 * (wire, arc, helix) to a loading card (LD row) in commands treview
 */

  void
Set_Wire_Conductivity( int tag, double s, GtkListStore *store )
{
  int idx, nchld;
  GtkTreeIter iter_ld;
  gchar *str, sv[13];

  /* Find num of rows and first iter, abort if tree empty */
  nchld = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL);
  if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter_ld))
	return;

  /* Look for an LD card with tag number = tag */
  for( idx = 0; idx < nchld; idx++ )
  {
	gtk_tree_model_get( GTK_TREE_MODEL(store),
		&iter_ld, GEOM_COL_NAME, &str, -1 );

	if( strcmp(str, "LD") == 0 )
	{
	  g_free(str);
	  gtk_tree_model_get( GTK_TREE_MODEL(store),
		  &iter_ld, GEOM_COL_I2, &str, -1 );
	  if( atoi(str) == tag )
	  {
		g_free(str);
		break;
	  }
	  else
		g_free(str);
	}
	else
	  g_free(str);

	gtk_tree_model_iter_next(
		GTK_TREE_MODEL(store), &iter_ld);

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
	for( idx = CMND_COL_I1; idx <= CMND_COL_F6; idx++ )
	  gtk_list_store_set( store, &iter_ld, idx, "0", -1 );
  }

  /* Set LD card parameters */
  gtk_list_store_set( store, &iter_ld, CMND_COL_I1, "5", -1 );
  snprintf( sv, 6, "%5d", tag );
  gtk_list_store_set( store, &iter_ld, CMND_COL_I2, sv, -1 );
  snprintf( sv, 13, "%12.5E", s );
  gtk_list_store_set( store, &iter_ld, CMND_COL_F1, sv, -1 );

  /* Scroll tree view to bottom */
  gtk_adjustment_set_value(
	  cmnd_adjustment, cmnd_adjustment->upper );

} /* Set_Wire_Conductivity() */

/*------------------------------------------------------------------------*/

