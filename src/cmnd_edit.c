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

#include "cmnd_edit.h"
#include "shared.h"

/*------------------------------------------------------------------------*/

/* Insert_EN_Card()
 *
 * Inserts a default EN card if missing
 */

  static void
Insert_EN_Card( GtkListStore *store, GtkTreeIter *iter )
{
  gint idx, idc;

  /* Insert default EN card if list is clear */
  idx = gtk_tree_model_iter_n_children(
	  GTK_TREE_MODEL(store), NULL );
  if( !idx )
  {
	gtk_list_store_append( store, iter );
	gtk_list_store_set( store, iter, CMND_COL_NAME, "EN", -1 );
	for( idc = CMND_COL_I1; idc < CMND_NUM_COLS; idc++ )
	  gtk_list_store_set( store, iter, idc, "0", -1 );
  }

} /* Insert_EN_Card() */

/*------------------------------------------------------------------------*/

/* Get_Command_Data()
 *
 * Gets command data from a treeview row
 */

  static void
Get_Command_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gint idc;
  gchar *sv;

  /* Get data from tree view (I1-I4, F1-F6)*/
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idc = CMND_COL_I1; idc <= CMND_COL_I4; idc++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idc, &sv, -1);
	  iv[idc-CMND_COL_I1] = atoi(sv);
	  g_free(sv);
	}
	for( idc = CMND_COL_F1; idc <= CMND_COL_F6; idc++ )
	{
	  gtk_tree_model_get(
		  GTK_TREE_MODEL(store), iter, idc, &sv, -1);
	  fv[idc-CMND_COL_F1] = Strtod( sv, NULL );
	  g_free(sv);
	}
  }
  else Stop( _("Get_Command_Data(): Error reading\n"
		"row data: Invalid list iterator"), ERR_OK );

} /* Get_Command_Data() */

/*------------------------------------------------------------------------*/

/* Set_Command_Data()
 *
 * Sets data into a command row
 */

  static void
Set_Command_Data(
	GtkListStore *store,
	GtkTreeIter *iter,
	int *iv, double *fv )
{
  gchar str[13];
  gint idc;

  /* Format and set editor data to treeview (I1-I4 & F1-F6) */
  if( gtk_list_store_iter_is_valid(store, iter) )
  {
	for( idc = CMND_COL_I1; idc <= CMND_COL_I4; idc++ )
	{
	  snprintf( str, 6, "%5d", iv[idc-CMND_COL_I1] );
	  gtk_list_store_set( store, iter, idc, str, -1 );
	}

	for( idc = CMND_COL_F1; idc <= CMND_COL_F6; idc++ )
	{
	  snprintf( str, 13, "%12.5E", fv[idc-CMND_COL_F1] );
	  gtk_list_store_set( store, iter, idc, str, -1 );
	}
  }
  else Stop( _("Set_Command_Data(): Error writing row data\n"
		"Please re-select row"), ERR_OK );

  SetFlag( NEC2_EDIT_SAVE );

} /* Set_Command_Data() */

/*------------------------------------------------------------------------*/

/* Insert_Blank_Command_Row()
 *
 * Inserts a blank row in a tree view with only its name (GW ... )
 */

  static void
Insert_Blank_Command_Row(
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
  retv = gtk_tree_selection_get_selected( selection, NULL, iter );

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
	gtk_tree_model_get(
		GTK_TREE_MODEL(store), iter, CMND_COL_NAME, &str, -1 );
	if( strcmp(str, "EN") == 0 )
	  gtk_list_store_insert_before( store, iter, iter );
	else
	  gtk_list_store_insert_after( store, iter, iter );
	g_free(str);
  }

  gtk_list_store_set( store, iter, CMND_COL_NAME, name, -1 );
  for( n = CMND_COL_I1; n < CMND_NUM_COLS; n++ )
	gtk_list_store_set( store, iter, n, "--", -1 );
  gtk_tree_selection_select_iter( selection, iter );

} /* Insert_Blank_Command_Row() */

/*------------------------------------------------------------------------*/

/* Set_Labels()
 *
 * Sets labels in an editor window
 */

  static void
Set_Labels(
	GtkBuilder *builder,
	gchar **labels,
	gchar **text,
	gint num )
{
  int idx;
  GtkLabel *label;

  for( idx = 0; idx < num; idx++ )
  {
	label = GTK_LABEL( Builder_Get_Object(builder, labels[idx]) );
	gtk_label_set_text( label, text[idx] );
  }

} /* Set_Labels() */

/*------------------------------------------------------------------------*/

/* Excitation_Command()
 *
 * Edits the Excitation command (EX card) parameters
 */

  void
Excitation_Command( int action )
{
  /* For looking up spinbuttons/toggles */
  GtkSpinButton   *spin;
  GtkToggleButton *toggle;

  /* Spinbutton labels */
#define EX_LABELS 8
  static gchar *labels[EX_LABELS] =
  {
	"excitation_i2_label",
	"excitation_i3_label",
	"excitation_f1_label",
	"excitation_f2_label",
	"excitation_f3_label",
	"excitation_f4_label",
	"excitation_f5_label",
	"excitation_f6_label"
  };

  /* For reading/writing to EX row */
  static GtkTreeIter iter_ex;

  int idx, idi, idf;

  /* Integer data (I1-I4) */
  static gint iv[4] = { 0, 0, 0, 0 };

  /* Float data (F1-F6) */
  static gdouble fv[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  /* Command radio buttons */
#define EX_TYPRDBTN 6

  /* Excitation type */
  static gchar *typrdbtn[EX_TYPRDBTN] =
  {
	"excitation_i10_radiobutton",
	"excitation_i11_radiobutton",
	"excitation_i12_radiobutton",
	"excitation_i13_radiobutton",
	"excitation_i14_radiobutton",
	"excitation_i15_radiobutton"
  };

  /* Command data spinbuttons */
  static gchar *ispin[2] =
  {
	"excitation_i2_spinbutton",
	"excitation_i3_spinbutton"
  };
  static gchar *fspin[6] =
  {
	"excitation_f1_spinbutton",
	"excitation_f2_spinbutton",
	"excitation_f3_spinbutton",
	"excitation_f4_spinbutton",
	"excitation_f5_spinbutton",
	"excitation_f6_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	label = TRUE,  /* Enable setting of spinbutton labels */
		  save  = FALSE, /* Enable saving of editor data */
		  busy  = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, excitation_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
    Gtk_Widget_Destroy( excitation_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_ex, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New excitation row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_ex );

	  /* Insert a new blank EX row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_ex, "EX" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT: /* Edit a command row (EX card) */
	  /* Get selected row */
	  Get_Selected_Row(
		  cmnd_treeview, cmnd_store, &iter_ex, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_ex, iv, fv );

	  /* Write int data to the command editor */
	  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I3; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(excitation_editor_builder, ispin[idi - SPIN_COL_I2]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(excitation_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }

	  /* Set radio buttons */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			excitation_editor_builder, typrdbtn[iv[SPIN_COL_I1]]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Set check buttons */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			excitation_editor_builder, "excitation_i419_checkbutton") );
	  if( iv[SPIN_COL_I4] & 0x0a )
		gtk_toggle_button_set_active( toggle, TRUE );
	  else
		gtk_toggle_button_set_active( toggle, FALSE );

	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			excitation_editor_builder, "excitation_i420_checkbutton") );
	  if( iv[SPIN_COL_I4] & 0x01 )
		gtk_toggle_button_set_active( toggle, TRUE );
	  else
		gtk_toggle_button_set_active( toggle, FALSE );

	  label = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel excitation editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_ex );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled in editor window */
	  /* Find active excitation type radio button */
	  for( idx = 0; idx < EX_TYPRDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(excitation_editor_builder, typrdbtn[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  iv[SPIN_COL_I1] = idx;

	  label = save = TRUE;
	  break;

	case COMMAND_CKBUTTON: /* Check button toggled in editor window */
	  /* Find active print control check button
	   * (matrix asymmetry and segment impedance) */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			excitation_editor_builder, "excitation_i419_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
		iv[SPIN_COL_I4] = 10;
	  else
		iv[SPIN_COL_I4] = 0;

	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			excitation_editor_builder, "excitation_i420_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
		iv[SPIN_COL_I4] += 1;

	  save = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I3; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(excitation_editor_builder, ispin[idi - SPIN_COL_I2]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(excitation_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Set spin button labels */
  if( label )
  {
	switch( iv[SPIN_COL_I1] ) /* Excitation type */
	{
	  case 0: case 5: /* Voltage source */
		{
		  gchar *text[EX_LABELS] =
		  {
			_("Tag Number"),
			_("Segment Number"),
			_("Voltage Real Part"),
			_("Voltage Imaginary Part"),
			_("Normalization Factor"),
			_("  ** NOT USED **"),
			_("  ** NOT USED **"),
			_("  ** NOT USED **"),
		  };

		  /* Clear unused spin buttons */
		  for( idf = SPIN_COL_F4; idf <= SPIN_COL_F6; idf++ )
		  {
			spin = GTK_SPIN_BUTTON(
				Builder_Get_Object(excitation_editor_builder, fspin[idf]) );
			gtk_spin_button_set_value( spin, 0 );
		  }
		  Set_Labels( excitation_editor_builder, labels, text, EX_LABELS );
		}
		break;

	  case 1: case 2: case 3: /* Incident wave */
		{
		  gchar *text[EX_LABELS] =
		  {
			_("Num of Theta Angles"),
			_("Num of Phi Angles"),
			_("Theta Angle (deg)"),
			_("Phi Angle (deg)"),
			_("Eta Angle (deg)"),
			_("Theta Increment (deg)"),
			_("Phi Increment (deg)"),
			_("Minor/Major Axis")
		  };

		  Set_Labels( excitation_editor_builder, labels, text, EX_LABELS );
		}
		break;

	  case 4: /* Current source */
		{
		  gchar *text[EX_LABELS] =
		  {
			_("  ** NOT USED **"),
			_("  ** NOT USED **"),
			_("X Position (m)"),
			_("Y Position (m)"),
			_("Z Position (m)"),
			_("alpha Angle (deg)"),
			_("beta Angle (deg)"),
			_("Current Moment (A.m)")
		  };

		  /* Clear unused spin buttons */
		  for( idx = 0; idx < 2; idx++ )
		  {
			spin = GTK_SPIN_BUTTON(
				Builder_Get_Object(excitation_editor_builder, ispin[idx]) );
			gtk_spin_button_set_value( spin, 0 );
		  }
		  Set_Labels( excitation_editor_builder, labels, text, EX_LABELS );
		}

		label = FALSE;
	} /* switch( iv[SPIN_COL_I1] ) */
  } /* if( label ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Excitation_Command() */

/*------------------------------------------------------------------------*/

/* Frequency_Command()
 *
 * Edits the Frequency command (FR card) parameters
 */

  void
Frequency_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to FR row */
  static GtkTreeIter iter_fr;

  int idf;

  /* Integer data (I1-I4) */
  static gint iv[4] = { 0, 0, 0, 0 };

  /* Float data (F1-F6) */
  static gdouble
	fv[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  /* Command data spinbuttons */
  static gchar *fspin[3] =
  {
	"frequency_start_spinbutton",
	"frequency_step_spinbutton",
	"frequency_end_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save  = FALSE, /* Enable saving of editor data */
	fstep = FALSE, /* Set frequency step to editor */
	busy  = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, frequency_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( frequency_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_fr, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Read int data from the command editor */
  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
		frequency_editor_builder, "frequency_num_spinbutton") );
  iv[SPIN_COL_I2] = gtk_spin_button_get_value_as_int( spin );

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F3; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(frequency_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New frequency row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_fr );

	  /* Insert a new blank FR row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_fr, "FR" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT: /* Edit a command row (FR card) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_fr, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_fr, iv, fv );

	  /* Write int data to the command editor */
	  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
			frequency_editor_builder, "frequency_num_spinbutton") );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I2] );

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F2; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(frequency_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }

	  /* Calculate and set end freq to editor */
	  if( iv[SPIN_COL_I2] > 1 )
	  {
		if( iv[SPIN_COL_I1] == 0 ) /* Additive stepping */
		  fv[SPIN_COL_F3] = fv[SPIN_COL_F1] +
			fv[SPIN_COL_F2]*(gdouble)(iv[SPIN_COL_I2]-1);
		else /* Multiplicative stepping */
		  fv[SPIN_COL_F3] = fv[SPIN_COL_F1]*pow( fv[SPIN_COL_F2],
			  (gdouble)(iv[SPIN_COL_I2]-1) );

		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(frequency_editor_builder, fspin[SPIN_COL_F3]) );
		gtk_spin_button_set_value( spin, fv[SPIN_COL_F3] );
	  } /* if( iv[SPIN_COL_I2] > 1 ) */

	  /* Set stepping type radio button */
	  if( iv[SPIN_COL_I1] == 0 )
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Builder_Get_Object(
				frequency_editor_builder, "frequency_add_radiobutton")),
			TRUE );
	  else
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Builder_Get_Object(
				frequency_editor_builder, "frequency_mul_radiobutton")),
			TRUE );
	  break;

	case EDITOR_CANCEL: /* Cancel frequency editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_fr );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled in editor window */
	  /* Set frequency stepping type */
	  if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Builder_Get_Object(
				frequency_editor_builder, "frequency_add_radiobutton"))) )
		iv[SPIN_COL_I1] = 0;
	  else
		iv[SPIN_COL_I1] = 1;
	  save = TRUE;
	  break;

	case FREQUENCY_EDITOR_FSTEP: /* Frequency step changed */
	  fstep = FALSE;

	  if( iv[SPIN_COL_I1] == 0 ) /* Additive stepping */
		iv[SPIN_COL_I2] = (gint)((fv[SPIN_COL_F3] -
			  fv[SPIN_COL_F1])/fv[SPIN_COL_F2]) + 1;
	  else /* Multiplicative stepping */
		iv[SPIN_COL_I2] = (gint)(log(fv[SPIN_COL_F3] /
			  fv[SPIN_COL_F1])/log(fv[SPIN_COL_F2])) + 1;

	  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
			frequency_editor_builder, "frequency_num_spinbutton") );
	  gtk_spin_button_set_value( spin, iv[SPIN_COL_I2] );
	  save = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Calculate and set freq step to editor */
  if( fstep && (iv[SPIN_COL_I2] > 1 ))
  {
	if( iv[SPIN_COL_I1] == 0 ) /* Additive stepping */
	  fv[SPIN_COL_F2] = (fv[SPIN_COL_F3]-fv[SPIN_COL_F1]) /
		(gdouble)(iv[SPIN_COL_I2]-1);
	else /* Multiplicative stepping */
	  fv[SPIN_COL_F2] = pow(fv[SPIN_COL_F3]/fv[SPIN_COL_F1],
		  1.0/(gdouble)(iv[SPIN_COL_I2]-1));

	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(frequency_editor_builder, fspin[SPIN_COL_F2]) );
	gtk_spin_button_set_value( spin, fv[SPIN_COL_F2] );
  } /* if( fstep ) */
  else fstep = TRUE;

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Frequency_Command() */

/*------------------------------------------------------------------------*/

/* Ground_Command()
 *
 * Edits the Ground command (GN card) parameters
 */

  void
Ground_Command( int action )
{
  /* For looking up spinbuttons/togglebuttons */
  GtkSpinButton   *spin;
  GtkToggleButton *toggle;

  /* For showing/hiding frames */
  GtkFrame *frame;

  /* For reading/writing to GN/GD row */
  static GtkTreeIter iter_gn, iter_gd;

  int idx, idf;

  /* Integer data (I1-I4) */
  static gint iv[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };

  /* Float data (F1-F6) */
  static gdouble fv[12];

  /* Command radio buttons */
#define GN_RDBTN 4
  static gchar *rdbutton[GN_RDBTN] =
  {
	"ground_null_radiobutton",
	"ground_refl_radiobutton",
	"ground_perf_radiobutton",
	"ground_smfld_radiobutton"
  };

  /* Command data spinbuttons */
  static gchar *fspin[8] =
  {
	"ground_diel1_spinbutton",
	"ground_res1_spinbutton",
	"ground_diel2_spinbutton",
	"ground_res2_spinbutton",
	"ground_dist2_spinbutton",
	"ground_below_spinbutton",
	"ground_scrnrd_spinbutton",
	"ground_diam_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	radl = FALSE, /* Radial ground screen present  */
		 scmd = FALSE, /* Second medium present (cliff) */
		 both = FALSE, /* Both above selected */
		 save = FALSE, /* Enable saving of editor data  */
		 show = TRUE,  /* Show/hide frames according to ground type */
		 busy = FALSE; /* Block callbacks. Must be a better way to do this? */

  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, ground_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( ground_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_gn, iv, fv );
	if( both ) Set_Command_Data(
		cmnd_store, &iter_gd, &iv[CMND_NUM_ICOLS], &fv[CMND_NUM_FCOLS] );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New ground row to create */
	  /* Reset to perfect ground */
	  radl = scmd = both = FALSE;
	  show = TRUE;
	  iv[SPIN_COL_I1] = 1;
	  iv[SPIN_COL_I2] = 0;
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_perf_radiobutton") );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_gn );

	  /* Insert a new blank GN row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_gn, "GN" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  save = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a command row (GN) selected in treeview */
	  /* Reset flags and check buttons */
	  radl = scmd = both = FALSE;
	  show = TRUE;
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_radl_checkbutton") );
	  gtk_toggle_button_set_active( toggle, FALSE );
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_secmd_checkbutton") );
	  gtk_toggle_button_set_active( toggle, FALSE );

	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_gn, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_gn, iv, fv );

	  /* Set ground type radio buttons */
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(ground_editor_builder, rdbutton[iv[SPIN_COL_I1]+1]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Write 1st ground data to the command editor */
	  if( (iv[SPIN_COL_I1] != 1) && (iv[SPIN_COL_I1] != -1) )
		for( idf = SPIN_COL_F1; idf <= SPIN_COL_F2; idf++ )
		{
		  spin = GTK_SPIN_BUTTON(
			  Builder_Get_Object(ground_editor_builder, fspin[idf]) );
		  gtk_spin_button_set_value( spin, fv[idf] );
		}

	  /* Radial ground screen specified */
	  if( iv[SPIN_COL_I2] > 0 )
	  {
		radl = TRUE;

		/* Radial ground screen check button */
		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  ground_editor_builder, "ground_radl_checkbutton") );
		gtk_toggle_button_set_active( toggle, TRUE );

		/* Write num of radials to the command editor */
		spin = GTK_SPIN_BUTTON(	Builder_Get_Object(
			  ground_editor_builder, "ground_nrad_spinbutton") );
		gtk_spin_button_set_value( spin, iv[SPIN_COL_I2] );

		/* Write radial data to the command editor */
		for( idf = SPIN_COL_F3; idf <= SPIN_COL_F4; idf++ )
		{
		  spin = GTK_SPIN_BUTTON(
			  Builder_Get_Object(ground_editor_builder, fspin[idf+4]) );
		  gtk_spin_button_set_value( spin, fv[idf] );
		}

		/* Check for a following GD card, read data */
		iter_gd = iter_gn;
		if( Check_Card_Name(cmnd_store, &iter_gd, NEXT, "GD") )
		{
		  scmd = both = TRUE;

		  /* Write radial data to the command editor */
		  Get_Command_Data( cmnd_store, &iter_gd,
			  &iv[CMND_NUM_ICOLS], &fv[CMND_NUM_FCOLS] );
		  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F4; idf++ )
		  {
			spin = GTK_SPIN_BUTTON(
				Builder_Get_Object(ground_editor_builder, fspin[idf+2]) );
			gtk_spin_button_set_value(
				spin, fv[idf+CMND_NUM_FCOLS] );
		  }

		  /* Set 2nd medium check button */
		  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
				ground_editor_builder, "ground_secmd_checkbutton") );
		  gtk_toggle_button_set_active( toggle, TRUE );

		} /* if( Check_Card_Name(cmnd_store, &iter_gd, NEXT, "GD") ) */
	  } /* if( iv[SPIN_COL_I2] > 0 ) */
	  /* 2nd medium data specified */
	  else if( (fv[SPIN_COL_F3] > 0) && (fv[SPIN_COL_F4] > 0) )
	  {
		scmd = TRUE;

		/* Set 2nd medium check button */
		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  ground_editor_builder, "ground_secmd_checkbutton") );
		gtk_toggle_button_set_active( toggle, TRUE );

		/* Write 2nd medium data to command editor */
		for( idf = SPIN_COL_F3; idf <= SPIN_COL_F6; idf++ )
		{
		  spin = GTK_SPIN_BUTTON(
			  Builder_Get_Object(ground_editor_builder, fspin[idf]) );
		  gtk_spin_button_set_value( spin, fv[idf] );
		}

	  } /* if( (fv[SPIN_COL_F3] > 0) && (fv[SPIN_COL_F4] > 0) ) */
	  break;

	case EDITOR_CANCEL: /* Cancel ground editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_gn );
	  if( both )
		Remove_Row( cmnd_store, &iter_gd );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled */
	  /* Find active radio button */
	  for( idx = 0; idx < GN_RDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(ground_editor_builder, rdbutton[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  iv[SPIN_COL_I1] = idx-1;

	  /* Remove GD card if it exists for perfect or null ground */
	  if( (iv[SPIN_COL_I1] == 1) || (iv[SPIN_COL_I1] == -1) )
	  {
		if( both )
		  Remove_Row( cmnd_store, &iter_gd );

		/* Set toggle buttons as needed */
		toggle = GTK_TOGGLE_BUTTON(	Builder_Get_Object(
			  ground_editor_builder, "ground_radl_checkbutton") );
		gtk_toggle_button_set_active( toggle, FALSE );
		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  ground_editor_builder, "ground_secmd_checkbutton") );
		gtk_toggle_button_set_active( toggle, FALSE );

		both = radl = scmd = FALSE;
		iv[SPIN_COL_I2] = 0;
	  }

	  save = show = TRUE;
	  break;

	case COMMAND_CKBUTTON: /* Check button toggled */
	  /* Get number of radial wires if enabled */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_radl_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
		radl = TRUE;
	  else
		radl = FALSE;

	  /* Set 2nd medium flag if enabled */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_secmd_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
		scmd = TRUE;
	  else
		scmd = FALSE;

	  /* If both radials & 2nd medium, insert gd card */
	  if( radl && scmd )
	  {
		Insert_Blank_Command_Row(
			cmnd_treeview, cmnd_store, &iter_gd, "GD" );

		/* Scroll tree view to bottom */
		gtk_adjustment_set_value( cmnd_adjustment,
			gtk_adjustment_get_upper(cmnd_adjustment) -
			gtk_adjustment_get_page_size(cmnd_adjustment) );
		both = TRUE;
	  }
	  else
	  {
		if( both )
		  Remove_Row( cmnd_store, &iter_gd );
		both = FALSE;
	  }

	  save = show = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /*** Calculate ground data ***/

  /* Read num of radials from the command editor */
  if( radl )
  {
	spin = GTK_SPIN_BUTTON( Builder_Get_Object(
		  ground_editor_builder, "ground_nrad_spinbutton") );
	iv[SPIN_COL_I2] = gtk_spin_button_get_value_as_int( spin );
  }
  else iv[SPIN_COL_I2] = 0;

  /*** Read float data from the command editor ***/

  /* Perfect or null ground */
  if( (iv[SPIN_COL_I1] == 1) || (iv[SPIN_COL_I1] == -1) )
  {
	for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  fv[idf] = 0.0;
	iv[SPIN_COL_I2] = 0;
  }
  else /* Finite ground */
  {
	/* Read 1st medium rel dielec const & conductivity */
	for( idf= SPIN_COL_F1; idf <= SPIN_COL_F2; idf++ )
	{
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(ground_editor_builder, fspin[idf]) );
	  fv[idf] = gtk_spin_button_get_value( spin );
	}

	/* If both radial and 2nd medium, GD card is needed */
	if( both )
	{
	  /* Read radial screen parameters */
	  for( idf = SPIN_COL_F3; idf <= SPIN_COL_F4; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(ground_editor_builder, fspin[idf+4]) );
		fv[idf] = gtk_spin_button_get_value( spin );
	  }

	  /* Set 1st medium & radial screen parameters to GN card */
	  for( idf = SPIN_COL_F5; idf <= SPIN_COL_F6; idf++ )
		fv[idf] = 0.0;

	  /* Set 2nd medium parameters to GD card */
	  for( idf = SPIN_COL_F3; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(ground_editor_builder, fspin[idf]) );
		fv[idf - SPIN_COL_F3+CMND_NUM_FCOLS] =
		  gtk_spin_button_get_value( spin );
	  }

	}
	else /* Only radial screen or 2nd medium */
	{
	  if( radl ) /* If radial ground only specified, read params */
	  {
		for( idf = SPIN_COL_F3; idf <= SPIN_COL_F4; idf++ )
		{
		  spin = GTK_SPIN_BUTTON(
			  Builder_Get_Object(ground_editor_builder, fspin[idf+4]) );
		  fv[idf] = gtk_spin_button_get_value( spin );
		}
		for( idf = SPIN_COL_F5; idf <= SPIN_COL_F6; idf++ )
		  fv[idf] = 0.0;
	  } /* if( radl ) */
	  else if( scmd ) /* If second medium only, read parameters */
	  {
		for( idf = SPIN_COL_F3; idf <= SPIN_COL_F6; idf++ )
		{
		  spin = GTK_SPIN_BUTTON(
			  Builder_Get_Object(ground_editor_builder, fspin[idf]) );
		  fv[idf] = gtk_spin_button_get_value( spin );
		}
	  }
	  else for( idf = SPIN_COL_F3; idf <= SPIN_COL_F6; idf++ )
		fv[idf] = 0.0;

	} /* else of if( both ) */

  } /* else (Finite ground) */

  /* Show/hide parts (frames) of editor as needed */
  if( show )
  {
	/* Perfect or no ground */
	if( (iv[SPIN_COL_I1] == 1) || (iv[SPIN_COL_I1] == -1) )
	{
	  /* Show only radio buttons */
	  frame = GTK_FRAME( Builder_Get_Object(
			ground_editor_builder, "ground_med1_frame") );
	  gtk_widget_hide( GTK_WIDGET(frame) );
	  frame = GTK_FRAME( Builder_Get_Object(
			ground_editor_builder, "ground_med2_frame") );
	  gtk_widget_hide( GTK_WIDGET(frame) );
	  frame = GTK_FRAME( Builder_Get_Object(
			ground_editor_builder, "ground_radial_frame") );
	  gtk_widget_hide( GTK_WIDGET(frame) );
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_radl_checkbutton") );
	  gtk_widget_hide( GTK_WIDGET(toggle) );
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_secmd_checkbutton") );
	  gtk_widget_hide( GTK_WIDGET(toggle) );

	} /* if( (iv[SPIN_COL_I1] == 1) || (iv[SPIN_COL_I1] == -1) ) */

	if( (iv[SPIN_COL_I1] == 0) || (iv[SPIN_COL_I1] == 2) ) /* Finite ground */
	{
	  /* Show check buttons */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_radl_checkbutton") );
	  gtk_widget_show( GTK_WIDGET(toggle) );
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			ground_editor_builder, "ground_secmd_checkbutton") );
	  gtk_widget_show( GTK_WIDGET(toggle) );

	  /* Show 1st medium */
	  frame = GTK_FRAME(
		  Builder_Get_Object(ground_editor_builder, "ground_med1_frame") );
	  gtk_widget_show( GTK_WIDGET(frame) );

	  if( scmd ) /* Show 2nd medium */
	  {
		frame = GTK_FRAME( Builder_Get_Object(
			  ground_editor_builder, "ground_med2_frame") );
		gtk_widget_show( GTK_WIDGET(frame) );
	  }
	  else
	  {
		frame = GTK_FRAME( Builder_Get_Object(
			  ground_editor_builder, "ground_med2_frame") );
		gtk_widget_hide( GTK_WIDGET(frame) );
	  }

	  if( radl ) /* Show radial screen */
	  {
		frame = GTK_FRAME( Builder_Get_Object(
			  ground_editor_builder, "ground_radial_frame") );
		gtk_widget_show( GTK_WIDGET(frame) );
	  }
	  else
	  {
		frame = GTK_FRAME( Builder_Get_Object(
			  ground_editor_builder, "ground_radial_frame") );
		gtk_widget_hide( GTK_WIDGET(frame) );
	  }
	} /* if( (iv[SPIN_COL_I1] == 0) || (iv[SPIN_COL_I1] == 2) ) */

	gtk_window_resize( GTK_WINDOW(ground_command), 10, 10 );
  } /* if( show ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Ground_Command() */

/*------------------------------------------------------------------------*/

/* Ground2_Command()
 *
 * Edits the 2nd medium Ground command (GD card) parameters
 */

  void
Ground2_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to GD row */
  static GtkTreeIter iter_gd;

  int idf;

  /* Integer data (I1-I4) */
  static gint iv[4] = { 0, 0, 0, 0 };

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Command data spinbuttons */
  static gchar *fspin[4] =
  {
	"ground2_diel_spinbutton",
	"ground2_res_spinbutton",
	"ground2_dist_spinbutton",
	"ground2_below_spinbutton",
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data  */
		 busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, ground2_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( ground2_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_gd, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New rad pattern row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_gd );

	  /* Insert a new blank GD row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_gd, "GD" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a command row (GD) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_gd, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_gd, iv, fv );

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F4; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(ground2_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel rad pattern editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_gd );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  /* Read float data from the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F4; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(ground2_editor_builder, fspin[idf]) );
		fv[idf] = gtk_spin_button_get_value( spin );
	  }
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Ground2_Command() */

/*------------------------------------------------------------------------*/

/* Radiation_Command()
 *
 * Edits the Radiation command (RP card) parameters
 */

  void
Radiation_Command( int action )
{
  /* For looking up spin/togle buttons */
  GtkSpinButton *spin;
  GtkToggleButton *toggle;

  /* For reading/writing to RP row */
  static GtkTreeIter iter_rp;

  int idx, idi, idf, xnda;

  /* Integer data (I1-I4) */
  static gint iv[4];

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Wave/ground type radio buttons */
#define RP_WRDBTN 7
  static gchar *wrdbtn[RP_WRDBTN] =
  {
	"radiation_i10_radiobutton",
	"radiation_i11_radiobutton",
	"radiation_i12_radiobutton",
	"radiation_i13_radiobutton",
	"radiation_i14_radiobutton",
	"radiation_i15_radiobutton",
	"radiation_i16_radiobutton"
  };

  /* XNDA radio buttons */
#define RP_XRDBTN 2
  static gchar *xrdbtn[RP_XRDBTN] =
  {
	"radiation_x0_radiobutton",
	"radiation_x1_radiobutton"
  };

#define RP_NRDBTN 6
  static gchar *nrdbtn[RP_NRDBTN] =
  {
	"radiation_n0_radiobutton",
	"radiation_n1_radiobutton",
	"radiation_n2_radiobutton",
	"radiation_n3_radiobutton",
	"radiation_n4_radiobutton",
	"radiation_n5_radiobutton"
  };

#define RP_DRDBTN 2
  static gchar *drdbtn[RP_DRDBTN] =
  {
	"radiation_d0_radiobutton",
	"radiation_d1_radiobutton"
  };

#define RP_ARDBTN 3
  static gchar *ardbtn[RP_ARDBTN] =
  {
	"radiation_a0_radiobutton",
	"radiation_a1_radiobutton",
	"radiation_a2_radiobutton"
  };

  /* Command data spinbuttons */
  static gchar *ispin[2] =
  {
	"radiation_i2_spinbutton",
	"radiation_i3_spinbutton"
  };

  static gchar *fspin[6] =
  {
	"radiation_f1_spinbutton",
	"radiation_f2_spinbutton",
	"radiation_f3_spinbutton",
	"radiation_f4_spinbutton",
	"radiation_f5_spinbutton",
	"radiation_f6_spinbutton"
  };

  /* Labels for I1, F1-F3 & F5 spinuttons */
#define RP_LABELS 4
  static gchar *labels[RP_LABELS] =
  {
	"radiation_i1_label",
	"radiation_f1_label",
	"radiation_f3_label",
	"radiation_f5_label"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	norm  = FALSE, /* Indicates normalization is specified */
		  label = TRUE,  /* Enable setting of labels */
		  save  = FALSE, /* Enable saving of editor data */
		  busy  = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, radiation_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( radiation_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_rp, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New rad pattern row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_rp );

	  /* Insert a new blank RP row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_rp, "RP" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  label = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a command row (RP) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_rp, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_rp, iv, fv );

	  /* Set wave type radio buttons */
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(radiation_editor_builder, wrdbtn[iv[SPIN_COL_I1]]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Set X radio buttons */
	  xnda = iv[SPIN_COL_I4];
	  idx = xnda/1000;
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(radiation_editor_builder, xrdbtn[idx]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Set N type radio buttons */
	  xnda -= idx*1000;
	  idx = xnda/100;
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(radiation_editor_builder, nrdbtn[idx]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Set D type radio buttons */
	  xnda -= idx*100;
	  idx = xnda/10;
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(radiation_editor_builder, drdbtn[idx]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Set A type radio buttons */
	  xnda -= idx*10;
	  idx = xnda;
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(radiation_editor_builder, ardbtn[idx]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  /* Write int data to the command editor (i2 & I3) */
	  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I3; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(radiation_editor_builder, ispin[idi-SPIN_COL_I2]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(radiation_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }
	  label = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel rad pattern editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_rp );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled */
	  /* Test wave/ground type radio buttons */
	  for( idx = 0; idx < RP_WRDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(radiation_editor_builder, wrdbtn[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  iv[SPIN_COL_I1] = idx;

	  /* Test XNDA radio buttons */
	  xnda = 0;
	  if( iv[SPIN_COL_I1] != 1 ) /* Surface wave, XNDA not used */
	  {
		/* X radio buttons */
		for( idx = 0; idx < RP_XRDBTN; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(radiation_editor_builder, xrdbtn[idx]) );
		  if( gtk_toggle_button_get_active(toggle) )
			break;
		}
		xnda += 1000*idx;

		/* N radio buttons */
		for( idx = 0; idx < RP_NRDBTN; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(radiation_editor_builder, nrdbtn[idx]) );
		  if( gtk_toggle_button_get_active(toggle) )
			break;
		}
		xnda += 100*idx;
		if( idx )
		  norm = TRUE;
		else
		  norm = FALSE;

		/* D radio buttons */
		for( idx = 0; idx < RP_DRDBTN; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(radiation_editor_builder, drdbtn[idx]) );
		  if( gtk_toggle_button_get_active(toggle) )
			break;
		}
		xnda += 10*idx;

		/* A radio buttons */
		for( idx = 0; idx < RP_ARDBTN; idx++ )
		{
		  toggle = GTK_TOGGLE_BUTTON(
			  Builder_Get_Object(radiation_editor_builder, ardbtn[idx]) );
		  if( gtk_toggle_button_get_active(toggle) )
			break;
		}
		xnda += idx;
	  }

	  iv[SPIN_COL_I4] = xnda;
	  save = label = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I3; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(radiation_editor_builder, ispin[idi-SPIN_COL_I2]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }
  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(radiation_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Set labels according to wave/ground type */
  if( label )
  {
	if( iv[SPIN_COL_I1] == 1 ) /* Space + ground wave case */
	{
	  gchar *text[RP_LABELS] =
	  {
		_("Points in Z"),
		_("Initial Z (m)"),
		_("Increment in Z"),
		_("Field Point R (m)")
	  };
	  Set_Labels( radiation_editor_builder, labels, text, RP_LABELS );
	}
	else /* All other wave/ground cases */
	{
	  gchar *text[RP_LABELS] =
	  {
		_("Points in Phi"),
		_("Initial Theta (deg)"),
		_("Increment in Theta"),
		_("R (m) (Optional)")
	  };
	  Set_Labels( radiation_editor_builder, labels, text, RP_LABELS );
	}

	/* Set normalization factor label */
	if( norm )
	  gtk_label_set_text( GTK_LABEL(
			Builder_Get_Object(radiation_editor_builder, "radiation_f6_label")),
		  _("Normalization Factor") );
	else
	  gtk_label_set_text( GTK_LABEL(
			Builder_Get_Object(radiation_editor_builder, "radiation_f6_label")),
		  _("** NOT USED **") );

	label = FALSE;
  } /* if( label ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Radiation_Command() */

/*------------------------------------------------------------------------*/

/* Loading_Command()
 *
 * Edits the Loading command (LD card) parameters
 */

  void
Loading_Command( int action )
{
  /* For looking up spinbuttons/togglebuttons */
  GtkSpinButton *spin;
  GtkToggleButton *toggle;

  /* For reading/writing to LD row */
  static GtkTreeIter iter_ld;

  int idx, idi, idf;

  /* Integer data (I1-I4) */
  static gint iv[4];

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Labels for F1-F3 spinuttons */
#define LD_LABELS 3
  static gchar *labels[LD_LABELS] =
  {
	"loading_f1_label",
	"loading_f2_label",
	"loading_f3_label"
  };

  /* Command radio buttons */
#define LD_RDBTN 7
  static gchar *rdbutton[7] =
  {
	"loading_null_radiobutton",
	"loading_slrlc_radiobutton",
	"loading_plrlc_radiobutton",
	"loading_sdrlc_radiobutton",
	"loading_pdrlc_radiobutton",
	"loading_rr_radiobutton",
	"loading_wcon_radiobutton"
  };

  /* Command data spinbuttons */
  static gchar *ispin[3] =
  {
	"loading_i2_spinbutton",
	"loading_i3_spinbutton",
	"loading_i4_spinbutton"
  };

  static gchar *fspin[3] =
  {
	"loading_f1_spinbutton",
	"loading_f2_spinbutton",
	"loading_f3_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save  = FALSE, /* Enable saving of editor data */
		  label = TRUE,  /* Show/hide frames according to loading type */
		  busy  = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, loading_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( loading_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_ld, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New loading row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_ld );

	  /* Insert a new blank LD row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_ld, "LD" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  label = TRUE;
	  break;

	case EDITOR_EDIT: /* Edit a command row (LD) selected in treeview */
	  /* Clear buffers */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I4; idi++ )
		iv[idi] = 0;
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F3; idf++ )
		fv[idf] = 0.0;

	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_ld, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_ld, iv, fv );

	  /* Lumped/Distributed loading, convert parameters to right units */
	  if( (iv[SPIN_COL_I1] >= 0) && (iv[SPIN_COL_I1] <= 3) )
	  {
		fv[SPIN_COL_F2] /= 1.0E-6;  /* Convert H to uH */
		fv[SPIN_COL_F3] /= 1.0E-12; /* Convert F to pF */
	  }

	  /* Write int data to the command editor */
	  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I4; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(loading_editor_builder, ispin[idi-SPIN_COL_I2]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F3; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(loading_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }

	  /* Set active radio button */
	  toggle = GTK_TOGGLE_BUTTON(
		  Builder_Get_Object(loading_editor_builder, rdbutton[iv[SPIN_COL_I1]+1]) );
	  gtk_toggle_button_set_active( toggle, TRUE );

	  label = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel loading editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_ld );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled */
	  /* Find active radio button */
	  for( idx = 0; idx < LD_RDBTN; idx++ )
	  {
		toggle = GTK_TOGGLE_BUTTON(
			Builder_Get_Object(loading_editor_builder, rdbutton[idx]) );
		if( gtk_toggle_button_get_active(toggle) )
		  break;
	  }
	  iv[SPIN_COL_I1] = idx-1;
	  save = label = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Change labels and calculate loading
   * values according to loading type */
  if( label )
  {
	gtk_widget_show( GTK_WIDGET(Builder_Get_Object(
			loading_editor_builder, "loading_frame")) );

	switch( iv[SPIN_COL_I1] ) /* Loading type */
	{
	  case -1: /* Short all loads */
		gtk_widget_hide( GTK_WIDGET(Builder_Get_Object(
				loading_editor_builder, "loading_frame")) );
		gtk_window_resize( GTK_WINDOW(loading_command), 10, 10 );
		break;

	  case 0: case 1: /* Lumped loading */
		{
		  gchar *text[LD_LABELS] =
		  {
			_("Resistance Ohm"),
			_("Inductance uH"),
			_("Capacitance pF")
		  };
		  Set_Labels( loading_editor_builder, labels, text, LD_LABELS );
		}
		break;

	  case 2: case 3: /* Distributed loading */
		{
		  gchar *text[LD_LABELS] =
		  {
			_("Resistance Ohm/m"),
			_("Inductance uH/m"),
			_("Capacitance pF/m")
		  };
		  Set_Labels( loading_editor_builder, labels, text, LD_LABELS );
		}
		break;

	  case 4: /* Impedance */
		{
		  gchar *text[LD_LABELS] =
		  {
			_("Resistance Ohm"),
			_("Reactance Ohm"),
			_("  ** NOT USED **")
		  };
		  Set_Labels( loading_editor_builder, labels, text, LD_LABELS );
		}
		break;

	  case 5: /* Wire conductivity */
		{
		  gchar *text[LD_LABELS] =
		  {
			_("Conductivity S/m"),
			_("  ** NOT USED **"),
			_("  ** NOT USED **")
		  };
		  Set_Labels( loading_editor_builder, labels, text, LD_LABELS );
		}

	} /* switch( iv[SPIN_COL_I1] ) */

	label = FALSE;
  } /* if( label ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I4; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(loading_editor_builder, ispin[idi-SPIN_COL_I2]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F3; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(loading_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Clear or convert parameters to right units for NEC2 */
  switch( iv[SPIN_COL_I1] ) /* Loading type */
  {
	case -1: /* Short all loads */
	  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I4; idi++ )
		iv[idi] = 0;
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F3; idf++ )
		fv[idf] = 0.0;
	  break;

	case 0: case 1: case 2: case 3: /* Lumped/Distributed loading */
	  fv[SPIN_COL_F2] *= 1.0E-6;  /* Convert uH to H */
	  fv[SPIN_COL_F3] *= 1.0E-12; /* Convert pF to F */
	  break;

	case 4: /* Impedance R+X, clear F3 */
	  fv[ SPIN_COL_F3] = 0.0;
	  break;

	case 5: /* Wire conductivity, clear F2 & F3 */
	  fv[SPIN_COL_F2] = 0.0;
	  fv[SPIN_COL_F3] = 0.0;

  } /* switch( iv[SPIN_COL_I1] ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Loading_Command() */

/*------------------------------------------------------------------------*/

/* Network_Command()
 *
 * Edits the Network command (NT card) parameters
 */

  void
Network_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to NT row */
  static GtkTreeIter iter_nt;

  int idi, idf;

  /* Integer data (I1-I4) */
  static gint iv[4];

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Command data spinbuttons */
  static gchar *ispin[4] =
  {
	"network_i1_spinbutton",
	"network_i2_spinbutton",
	"network_i3_spinbutton",
	"network_i4_spinbutton"
  };

  static gchar *fspin[6] =
  {
	"network_f1_spinbutton",
	"network_f2_spinbutton",
	"network_f3_spinbutton",
	"network_f4_spinbutton",
	"network_f5_spinbutton",
	"network_f6_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
		 busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, network_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( network_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_nt, iv, fv );
	save  = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New network row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_nt );

	  /* Insert a new blank NT row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_nt, "NT" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a command row (NT) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_nt, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_nt, iv, fv );

	  /* Write int data to the command editor */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I4; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(network_editor_builder, ispin[idi]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(network_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }
	  break;

	case EDITOR_CANCEL: /* Cancel Network editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_nt );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I4; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(network_editor_builder, ispin[idi]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(network_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Network_Command() */

/*------------------------------------------------------------------------*/

/* Txline_Command()
 *
 * Edits the Transmission Line command (TL card) parameters
 */

  void
Txline_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to TL row */
  static GtkTreeIter iter_tl;

  int idi, idf;

  /* Integer data (I1-I4) */
  static gint iv[4];

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Command data spinbuttons */
  static gchar *ispin[4] =
  {
	"txline_i1_spinbutton",
	"txline_i2_spinbutton",
	"txline_i3_spinbutton",
	"txline_i4_spinbutton"
  };

  static gchar *fspin[6] =
  {
	"txline_f1_spinbutton",
	"txline_f2_spinbutton",
	"txline_f3_spinbutton",
	"txline_f4_spinbutton",
	"txline_f5_spinbutton",
	"txline_f6_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	crossed = FALSE, /* Crossed transmission line */
			save = FALSE,	 /* Enable saving of editor data */
			busy = FALSE;	 /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, txline_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( txline_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	Set_Command_Data( cmnd_store, &iter_tl, iv, fv );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New transmission line row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_tl );

	  /* Insert a new blank TL row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_tl, "TL" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a command row (TL) selected in treeview */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_tl, name );

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_tl, iv, fv );

	  /* Set crossed txline checkbutton */
	  if( fv[SPIN_COL_F1] < 0.0 )
	  {
		fv[SPIN_COL_F1] = -fv[SPIN_COL_F1];
		crossed = TRUE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Builder_Get_Object(
				txline_editor_builder, "txline_crossed_checkbutton")),
			TRUE);
	  }
	  else
	  {
		crossed = FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Builder_Get_Object(
				txline_editor_builder, "txline_crossed_checkbutton")),
			FALSE);
	  }

	  /* Write int data to the command editor */
	  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I4; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(txline_editor_builder, ispin[idi]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(txline_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }

	  break;

	case EDITOR_CANCEL: /* Cancel transmission line editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_tl );
	  save = busy = FALSE;
	  return;

	case COMMAND_CKBUTTON: /* Check button toggled */
	  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Builder_Get_Object(
				txline_editor_builder,	"txline_crossed_checkbutton"))) )
		crossed = TRUE;
	  else
		crossed = FALSE;
	  save = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I1; idi <= SPIN_COL_I4; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(txline_editor_builder, ispin[idi]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(txline_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }
  if( crossed )
	fv[SPIN_COL_F1] = -fv[SPIN_COL_F1];

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Txline_Command() */

/*------------------------------------------------------------------------*/

/* Nearfield_Command()
 *
 * Edits the Nearfield command (NE/NF card) parameters
 */

  void
Nearfield_Command( int action )
{
  /* For looking up spin/toggle buttons */
  GtkSpinButton   *spin;
  GtkToggleButton *toggle;

  /* For reading/writing to NE/NH row */
  static GtkTreeIter
	iter_ne,
	iter_nh;

  int idi, idf;

  /* Integer data (I1-I4) */
  static gint iv[4];

  /* Float data (F1-F6) */
  static gdouble fv[6];

  /* Near field editor labels */
#define NF_LABELS 9
  static gchar *labels[NF_LABELS] =
  {
	"nearfield_i1_label",
	"nearfield_i2_label",
	"nearfield_i3_label",
	"nearfield_f1_label",
	"nearfield_f2_label",
	"nearfield_f3_label",
	"nearfield_f4_label",
	"nearfield_f5_label",
	"nearfield_f6_label"
  };

  /* Command data spinbuttons */
  static gchar *ispin[3] =
  {
	"nearfield_nx_spinbutton",
	"nearfield_ny_spinbutton",
	"nearfield_nz_spinbutton"
  };

  static gchar *fspin[6] =
  {
	"nearfield_f1_spinbutton",
	"nearfield_f2_spinbutton",
	"nearfield_f3_spinbutton",
	"nearfield_f4_spinbutton",
	"nearfield_f5_spinbutton",
	"nearfield_f6_spinbutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	label = TRUE,  /* Change spin button labels */
		  nref  = TRUE,  /* Near E field enabled */
		  nrhf  = FALSE, /* Near H field enabled */
		  save  = FALSE, /* Enable saving of editor data */
		  busy  = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up(&busy, nearfield_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( nearfield_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	if( nref )
	  Set_Command_Data( cmnd_store, &iter_ne, iv, fv );
	if( nrhf )
	  Set_Command_Data( cmnd_store, &iter_nh, iv, fv );
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New near field row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_ne );

	  /* Insert a new blank NE row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_ne, "NE" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  label = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a command row (NE/NH) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_ne, name );
	  iter_nh = iter_ne;

	  /* Get data from command editor */
	  Get_Command_Data( cmnd_store, &iter_ne, iv, fv );

	  /* Write int data to the command editor */
	  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I4; idi++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(nearfield_editor_builder, ispin[idi-SPIN_COL_I2]) );
		gtk_spin_button_set_value( spin, iv[idi] );
	  }

	  /* Write float data to the command editor */
	  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
	  {
		spin = GTK_SPIN_BUTTON(
			Builder_Get_Object(nearfield_editor_builder, fspin[idf]) );
		gtk_spin_button_set_value( spin, fv[idf] );
	  }

	  /* Set radio buttons */
	  if( iv[SPIN_COL_I1] == 0 ) /* Rectangular coordinates */
	  {
		toggle = GTK_TOGGLE_BUTTON(	Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_rect_radiobutton") );
		gtk_toggle_button_set_active( toggle, TRUE );
	  }
	  else
	  {
		toggle = GTK_TOGGLE_BUTTON(	Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_sph_radiobutton") );
		gtk_toggle_button_set_active( toggle, TRUE );
	  }

	  /* Set check buttons */
	  if( strcmp(name, "NE") == 0 )
	  {
		nref = TRUE; nrhf = FALSE;
		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_ne_checkbutton") );
		gtk_toggle_button_set_active( toggle, TRUE );

		toggle = GTK_TOGGLE_BUTTON(	Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_nh_checkbutton") );
		gtk_toggle_button_set_active( toggle, FALSE );
	  }

	  if( strcmp(name, "NH") == 0 )
	  {
		nrhf = TRUE; nref = FALSE;
		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_nh_checkbutton") );
		gtk_toggle_button_set_active( toggle, TRUE );

		toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			  nearfield_editor_builder, "nearfield_ne_checkbutton") );
		gtk_toggle_button_set_active( toggle, FALSE );
	  }

	  label = TRUE;
	  break;

	case EDITOR_CANCEL: /* Cancel near field editor */
	  /* Remove card(s) */
	  if( nref )
		Remove_Row( cmnd_store, &iter_ne );
	  if( nrhf )
		Remove_Row( cmnd_store, &iter_nh );
	  save = busy = FALSE;
	  return;

	case NEARFIELD_NE_CKBUTTON: /* E-field check button toggled */
	  /* Test E field check button */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			nearfield_editor_builder, "nearfield_ne_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
	  {
		nref = TRUE;
		Insert_Blank_Command_Row(
			cmnd_treeview, cmnd_store, &iter_ne, "NE" );
	  }
	  else
	  {
		nref = FALSE;
		Remove_Row( cmnd_store, &iter_ne );
	  }

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  save = TRUE;
	  break;

	case NEARFIELD_NH_CKBUTTON: /* H-field check button toggled */
	  /* Test H field check button */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			nearfield_editor_builder, "nearfield_nh_checkbutton") );
	  if( gtk_toggle_button_get_active(toggle) )
	  {
		nrhf = TRUE;
		Insert_Blank_Command_Row(
			cmnd_treeview, cmnd_store, &iter_nh, "NH" );
	  }
	  else
	  {
		nrhf = FALSE;
		Remove_Row( cmnd_store, &iter_nh );
	  }

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  save = TRUE;
	  break;

	case COMMAND_RDBUTTON: /* Radio button toggled */
	  /* Test rectangular coordinates radio button */
	  toggle = GTK_TOGGLE_BUTTON( Builder_Get_Object(
			nearfield_editor_builder, "nearfield_rect_radiobutton") );
	  if( gtk_toggle_button_get_active(toggle) )
		iv[SPIN_COL_I1] = 0; /* Rectangular */
	  else
		iv[SPIN_COL_I1] = 1; /* Spherical */
	  save = label = TRUE;
	  break;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Set labels according to coordinate type */
  if( label )
  {
	if( iv[SPIN_COL_I1] == 0 ) /* Rectangular coordinates */
	{
	  gchar *text[NF_LABELS] =
	  {
		_("Points in X-axis"),
		_("Points in Y-axis"),
		_("Points in Z-axis"),
		_("First Point X (m)"),
		_("First Point Y (m)"),
		_("First Point Z (m)"),
		_("Increment in X (m)"),
		_("Increment in Y (m)"),
		_("Increment in Z (m)")
	  };
	  Set_Labels( nearfield_editor_builder, labels, text, NF_LABELS );
	}
	else /* Spherical coordinates */
	{
	  gchar *text[NF_LABELS] =
	  {
		_("Points along R"),
		_("Points along Phi"),
		_("Points along Theta"),
		_("First Point R (m)"),
		_("First Point Phi"),
		_("First Point Theta"),
		_("Increment in R (m)"),
		_("Increment in Phi"),
		_("Increment in Theta")
	  };
	  Set_Labels( nearfield_editor_builder, labels, text, NF_LABELS );
	  label = FALSE;
	}

  } /* if( label ) */

  /* Read int data from the command editor */
  for( idi = SPIN_COL_I2; idi <= SPIN_COL_I4; idi++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(nearfield_editor_builder, ispin[idi-SPIN_COL_I2]) );
	iv[idi] = gtk_spin_button_get_value_as_int( spin );
  }

  /* Read float data from the command editor */
  for( idf = SPIN_COL_F1; idf <= SPIN_COL_F6; idf++ )
  {
	spin = GTK_SPIN_BUTTON(
		Builder_Get_Object(nearfield_editor_builder, fspin[idf]) );
	fv[idf] = gtk_spin_button_get_value( spin );
  }

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Nearfield_Command() */

/*------------------------------------------------------------------------*/

/* Kernel_Command()
 *
 * Edits the Kernel command (EK card) parameters
 */

  void
Kernel_Command( int action )
{
  /* For testing check button */
  GtkCheckButton *ckbutton;

  /* For reading/writing to EK row */
  static GtkTreeIter iter_ek;

  /* Thin wire kernel status */
  static int ek = 0;
  gchar sek[6];
  gchar *sv;

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
		 busy = FALSE; /* Block callbacks. Must be a better way to do this? */

  int idc;


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, kernel_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( kernel_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Set EK card data */
	if( gtk_list_store_iter_is_valid(cmnd_store, &iter_ek) )
	{
	  /* Set extended kernel data */
	  snprintf( sek, sizeof(sek), "%5d", ek );
	  gtk_list_store_set(
		  cmnd_store, &iter_ek, CMND_COL_I1, sek, -1 );

	  /* Clear row to 0 */
	  for( idc = CMND_COL_I2; idc <= CMND_COL_F6; idc++ )
		gtk_list_store_set( cmnd_store, &iter_ek, idc, "0", -1 );
	}
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New kernel row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_ek );

	  /* Insert a new blank EK row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_ek, "EK" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  save = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a command row (EK) */
	  /* Get selected row */
	  Get_Selected_Row(
		  cmnd_treeview, cmnd_store, &iter_ek, name );

	  /* Get data from command editor */
	  if( gtk_list_store_iter_is_valid(cmnd_store, &iter_ek) )
	  {
		gtk_tree_model_get(
			GTK_TREE_MODEL(cmnd_store),
			&iter_ek, CMND_COL_I1, &sv, -1);
		ek = atoi(sv);
		g_free(sv);
	  }

	  /* Set the kernel check button */
	  ckbutton = GTK_CHECK_BUTTON( Builder_Get_Object(
			kernel_editor_builder, "kernel_checkbutton") );
	  if( ek == 0 )
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(ckbutton), TRUE );
	  else
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(ckbutton), FALSE );

	  break;

	case EDITOR_CANCEL: /* Cancel kernel editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_ek );
	  save = busy = FALSE;
	  return;

	case COMMAND_CKBUTTON: /* Some check button changed in editor window */
	  /* Set kernel status according to checkbutton */
	  ckbutton = GTK_CHECK_BUTTON( Builder_Get_Object(
			kernel_editor_builder, "kernel_checkbutton") );
	  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ckbutton)) )
		ek = 0;
	  else
		ek = -1;
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Kernel_Command() */

/*------------------------------------------------------------------------*/

/* Intrange_Command()
 *
 * Edits the Interaction Approx Range command (KH card) parameters
 */

  void
Intrange_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to KH row */
  static GtkTreeIter iter_kh;
  gchar *sv;

  /* Interaction Approx range */
  static gdouble kh;
  static gchar skh[13];

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */

  int idc;

  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, intrange_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( intrange_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Set KH card data */
	if( gtk_list_store_iter_is_valid(cmnd_store, &iter_kh) )
	{
	  /* Clear row to 0 */
	  for( idc = CMND_COL_I1; idc <= CMND_COL_F6; idc++ )
		gtk_list_store_set(
			cmnd_store, &iter_kh, idc, "0", -1 );

	  /* Set range data */
	  gtk_list_store_set(
		  cmnd_store, &iter_kh, CMND_COL_F1, skh, -1 );
	}
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New inter approx range row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_kh );

	  /* Insert a new blank KH row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_kh, "KH" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a command row (KH) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_kh, name );

	  /* Get data from command editor */
	  if( gtk_list_store_iter_is_valid(cmnd_store, &iter_kh) )
	  {
		gtk_tree_model_get(
			GTK_TREE_MODEL(cmnd_store),
			&iter_kh, CMND_COL_F1, &sv, -1 );
		kh = Strtod( sv, NULL );
		g_free(sv);
	  }

	  /* Set range data to command editor */
	  spin = GTK_SPIN_BUTTON( Builder_Get_Object(
			intrange_editor_builder, "intrange_wlen_spinbutton") );
	  gtk_spin_button_set_value( spin, kh );

	  break;

	case EDITOR_CANCEL: /* Cancel inter range editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_kh );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Get range data from editor */
  spin = GTK_SPIN_BUTTON( Builder_Get_Object
	  (intrange_editor_builder, "intrange_wlen_spinbutton") );
  kh = gtk_spin_button_get_value( spin );
  snprintf( skh, sizeof(skh), "%12.5e", kh );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Intrange_Command() */

/*------------------------------------------------------------------------*/

/* Execute_Command()
 *
 * Edits the Execute command (XQ card) parameters
 */

  void
Execute_Command( int action )
{
  /* For reading/writing to XQ row */
  static GtkTreeIter iter_xq;
  gchar *sv;
  gchar sxq[6];

  int idx, idc;

  /* Execute command status */
  static int xq = 0;

  /* Command data radio buttons */
#define XQ_RDBTN 4
  static gchar *rdbutton[XQ_RDBTN] =
  {
	"execute_none_radiobutton",
	"execute_xz_radiobutton",
	"execute_yz_radiobutton",
	"execute_both_radiobutton"
  };

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
		 busy = FALSE; /* Block callbacks. Must be a better way to do this? */


  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, execute_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( execute_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Set XQ card data */
	if( gtk_list_store_iter_is_valid(cmnd_store, &iter_xq) )
	{
	  snprintf( sxq, sizeof(sxq), "%5d", xq );
	  gtk_list_store_set(
		  cmnd_store, &iter_xq, CMND_COL_I1, sxq, -1 );
	  for( idc = CMND_COL_I2; idc <= CMND_COL_F6; idc++ )
		gtk_list_store_set( cmnd_store, &iter_xq, idc, "0", -1 );
	}
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New execute row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_xq );

	  /* Insert a new blank XQ row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_xq, "XQ" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  save = TRUE;
	  break;

	case EDITOR_EDIT:  /* Edit a command row (XQ) */
	  /* Get selected row */
	  Get_Selected_Row(
		  cmnd_treeview, cmnd_store, &iter_xq, name );

	  /* Get data from command editor */
	  if( gtk_list_store_iter_is_valid(cmnd_store, &iter_xq) )
	  {
		gtk_tree_model_get(
			GTK_TREE_MODEL(cmnd_store),
			&iter_xq, CMND_COL_I1, &sv, -1);
		xq = atoi(sv);
		g_free(sv);
	  }

	  /* Set radio button in command editor */
	  for( idx = 0; idx < XQ_RDBTN; idx++ )
		if( xq == idx )
		{
		  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
				Builder_Get_Object(execute_editor_builder, rdbutton[idx])), TRUE );
		  break;
		}
	  break;

	case EDITOR_CANCEL: /* Cancel execute editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_xq );
	  save = busy = FALSE;
	  return;

	case COMMAND_RDBUTTON: /* Radio button toggled */
	  /* Get active radio button in command editor */
	  for( idx = 0; idx < XQ_RDBTN; idx++ )
		if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				Builder_Get_Object(execute_editor_builder, rdbutton[idx]))) )
		  break;
	  xq = idx;
	  save = TRUE;

  } /* switch( action ) */

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Execute_Command() */

/*------------------------------------------------------------------------*/

/* Zo_Command()
 *
 * Edits the Transmission Line Zo Command (ZO card) parameters
 */

  void
Zo_Command( int action )
{
  /* For looking up spinbuttons */
  GtkSpinButton *spin;

  /* For reading/writing to ZO row */
  static GtkTreeIter iter_zo;
  gchar *sv;

  /* Tx Line Impedance */
  static gint zo;
  static gchar szo[13];

  /* Card (row) name */
  gchar name[3];

  static gboolean
	save = FALSE, /* Enable saving of editor data */
	busy = FALSE; /* Block callbacks. Must be a better way to do this? */

  int idc;

  /* Block callbacks. (Should be a better way to do this) */
  if( Give_Up( &busy, zo_command) ) return;

  /* Quit if forced quit flag set (treeview row removed) */
  if( isFlagSet(EDITOR_QUIT) )
  {
	ClearFlag( EDITOR_QUIT );
	save = busy = FALSE;
	Gtk_Widget_Destroy( zo_command );
	return;
  }

  /* Save data to nec2 editor if appropriate */
  if( (action == EDITOR_APPLY) || ((action == EDITOR_NEW) && save) )
  {
	/* Set KH card data */
	if( gtk_list_store_iter_is_valid(cmnd_store, &iter_zo) )
	{
	  /* Clear row to 0 */
	  for( idc = CMND_COL_I1; idc <= CMND_COL_F6; idc++ )
		gtk_list_store_set(
			cmnd_store, &iter_zo, idc, "0", -1 );

	  /* Set range data */
	  gtk_list_store_set(
		  cmnd_store, &iter_zo, CMND_COL_I1, szo, -1 );
	}
	save = FALSE;
  } /* if( (action & EDITOR_SAVE) && save ) */

  /* Respond to user action */
  switch( action )
  {
	case EDITOR_NEW: /* New inter approx range row to create */
	  /* Insert a default EN card if list is empty */
	  Insert_EN_Card( cmnd_store, &iter_zo );

	  /* Insert a new blank ZO row after a selected row,
	   * if any, otherwise before the last (EN) row */
	  Insert_Blank_Command_Row(
		  cmnd_treeview, cmnd_store, &iter_zo, "ZO" );

	  /* Scroll tree view to bottom */
	  gtk_adjustment_set_value( cmnd_adjustment,
		  gtk_adjustment_get_upper(cmnd_adjustment) -
		  gtk_adjustment_get_page_size(cmnd_adjustment) );
	  break;

	case EDITOR_EDIT:  /* Edit a command row (KH) */
	  /* Get selected row */
	  Get_Selected_Row( cmnd_treeview, cmnd_store, &iter_zo, name );

	  /* Get data from command editor */
	  if( gtk_list_store_iter_is_valid(cmnd_store, &iter_zo) )
	  {
		gtk_tree_model_get(
			GTK_TREE_MODEL(cmnd_store),
			&iter_zo, CMND_COL_I1, &sv, -1 );
		zo = (gint)atoi( sv );
		g_free( sv );
	  }

	  /* Set range data to command editor */
	  spin = GTK_SPIN_BUTTON(
		  Builder_Get_Object(zo_editor_builder, "zo_spinbutton") );
	  gtk_spin_button_set_value( spin, (gdouble)zo );

	  break;

	case EDITOR_CANCEL: /* Cancel inter range editor */
	  /* Remove card(s) */
	  Remove_Row( cmnd_store, &iter_zo );
	  save = busy = FALSE;
	  return;

	case EDITOR_DATA: /* Some data changed in editor window */
	  save = TRUE;

  } /* switch( action ) */

  /* Get range data from editor */
  spin = GTK_SPIN_BUTTON(
	  Builder_Get_Object(zo_editor_builder, "zo_spinbutton") );
  zo = gtk_spin_button_get_value_as_int( spin );
  snprintf( szo, sizeof(szo), "%4d", zo );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
  busy = FALSE;

} /* Intrange_Command() */

/*------------------------------------------------------------------------*/

