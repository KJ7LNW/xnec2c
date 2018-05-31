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

#include "nec2_model.h"
#include "shared.h"

/*------------------------------------------------------------------------*/

/* Insert_Columns()
 *
 * Inserts columns in a list store
 */
  static void
Insert_Columns(
	GtkTreeView *view,
	GtkListStore* store,
	int ncols,
	char *colname[] )
{
  int idx;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;

  for( idx = 0; idx < ncols; idx++ )
  {
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect( renderer, "edited",
		(GCallback)cell_edited_callback, view );
	g_object_set_data( G_OBJECT(renderer),
		"column", GUINT_TO_POINTER(idx) );
	gtk_tree_view_insert_column_with_attributes(
		view, -1, colname[idx],	renderer, "text", idx, NULL );
  }
  model = GTK_TREE_MODEL(store);
  gtk_tree_view_set_model( view, model );

  /* Destroy model automatically with view */
  g_object_unref( model );

} /* Insert_Columns() */

/*------------------------------------------------------------------------*/

/* Create_List_Stores()
 *
 * Create stores needed for the treeview
 */
  static void
Create_List_Stores( void )
{
  /* Comments column names */
  char *cmnt_col_name[CMNT_NUM_COLS] =
  { _("Card"), _("Comments") };

  /* Geometry column names */
  char *geom_col_name[GEOM_NUM_COLS] =
  { _("Card"), "I1", "I2", "F1", "F2", "F3", "F4", "F5", "F6", "F7" };

  /* Command column names */
  char *cmnd_col_name[CMND_NUM_COLS] =
  { _("Card"), "I1", "I2", "I3", "I4", "F1", "F2", "F3", "F4", "F5", "F6" };


  /* Create list stores only if needed */
  if( cmnt_store != NULL ) return;

  /* Create comments list store */
  cmnt_store = gtk_list_store_new(
	  CMNT_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING );

  /* Create geometry data list store */
  geom_store = gtk_list_store_new(
	  GEOM_NUM_COLS, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING );

  /* Create control commands data list store */
  cmnd_store = gtk_list_store_new(
	  CMND_NUM_COLS, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING,
	  G_TYPE_STRING, G_TYPE_STRING );

  /* Insert comment columns */
  Insert_Columns(
	  cmnt_treeview, cmnt_store, CMNT_NUM_COLS, cmnt_col_name );

  /* Insert geometry columns */
  Insert_Columns(
	  geom_treeview, geom_store, GEOM_NUM_COLS, geom_col_name );

  /* Insert command columns */
  Insert_Columns(
	  cmnd_treeview, cmnd_store, CMND_NUM_COLS, cmnd_col_name );

  /* Set models to treviews */
  gtk_tree_view_set_model( cmnt_treeview, GTK_TREE_MODEL(cmnt_store) );
  gtk_tree_view_set_model( geom_treeview, GTK_TREE_MODEL(geom_store) );
  gtk_tree_view_set_model( cmnd_treeview, GTK_TREE_MODEL(cmnd_store) );

} /* Create_List_Stores() */

/*------------------------------------------------------------------------*/

/* Create_Default_File()
 *
 * Creates a default NEC2 file if needed
 */
  static void
Create_Default_File( void )
{
  GtkTreeIter iter;
  int idx, idi;
  char str[64];
  size_t s = sizeof( str );
  GtkTreeSelection *selection =
	gtk_tree_view_get_selection( cmnt_treeview );


  /* Clear all tree views */
  gtk_list_store_clear( cmnt_store );
  gtk_list_store_clear( geom_store );
  gtk_list_store_clear( cmnd_store );

  /* Append a default comment row */
  Strlcpy( str, _("--- NEC2 Input File created or edited by "), s );
  Strlcat( str, PACKAGE_STRING, s );
  Strlcat( str, " ---", s );
  gtk_list_store_append( cmnt_store, &iter );
  gtk_list_store_set(
	  cmnt_store, &iter,
	  CMNT_COL_NAME, "CM",
	  CMNT_COL_COMMENT, str, -1 );

  gtk_tree_selection_select_iter( selection, &iter );

  /* Append a default CE card */
  gtk_list_store_append( cmnt_store, &iter );
  gtk_list_store_set(
	  cmnt_store, &iter,
	  CMNT_COL_NAME, "CE",
	  CMNT_COL_COMMENT,
	  _("--- End Comments ---"),
	  -1 );

  /* Append a dipole wire (GW) card */
  gtk_list_store_append( geom_store, &iter );
  gtk_list_store_set( geom_store, &iter,
	  GEOM_COL_NAME, "GW",
	  GEOM_COL_I1, "1",
	  GEOM_COL_I2, "15",
	  GEOM_COL_F1, "0.0",
	  GEOM_COL_F2, "0.0",
	  GEOM_COL_F3, "-1.0",
	  GEOM_COL_F4, "0.0",
	  GEOM_COL_F5, "0.0",
	  GEOM_COL_F6, "1.0",
	  GEOM_COL_F7, "0.015",
	  -1 );

  /* Append a geometry end (GE) card */
  gtk_list_store_append( geom_store, &iter );
  gtk_list_store_set( geom_store,
	  &iter, GEOM_COL_NAME, "GE", -1 );
  for( idx = GEOM_COL_I1; idx < GEOM_NUM_COLS; idx++ )
	gtk_list_store_set( geom_store, &iter, idx, "0", -1 );

  /* Append an excitation (EX) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store, &iter,
	  CMND_COL_NAME, "EX",
	  CMND_COL_I1, "0",
	  CMND_COL_I2, "1",
	  CMND_COL_I3, "8",
	  CMND_COL_I4, "0",
	  CMND_COL_F1, "1.0",
	  CMND_COL_F2, "0.0",
	  CMND_COL_F3, "0.0",
	  CMND_COL_F4, "0.0",
	  CMND_COL_F5, "0.0",
	  CMND_COL_F6, "0.0",
	  -1 );

  /* Append a frequency (FR) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store, &iter,
	  CMND_COL_NAME, "FR",
	  CMND_COL_I1, "0",
	  CMND_COL_I2, "11",
	  CMND_COL_I3, "0",
	  CMND_COL_I4, "0",
	  CMND_COL_F1, "50.0",
	  CMND_COL_F2, "5.0",
	  CMND_COL_F3, "0.0",
	  CMND_COL_F4, "0.0",
	  CMND_COL_F5, "0.0",
	  CMND_COL_F6, "0.0",
	  -1 );

  /* Append a near H field (NH) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store, &iter,
	  CMND_COL_NAME, "NH",
	  CMND_COL_I1, "0",
	  CMND_COL_I2, "0",
	  CMND_COL_I3, "0",
	  CMND_COL_I4, "0",
	  CMND_COL_F1, "0.0",
	  CMND_COL_F2, "0.0",
	  CMND_COL_F3, "0.0",
	  CMND_COL_F4, "0.0",
	  CMND_COL_F5, "0.0",
	  CMND_COL_F6, "0.0",
	  -1 );

  /* Append a near E field (NE) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store, &iter,
	  CMND_COL_NAME, "NE",
	  CMND_COL_I1, "0",
	  CMND_COL_I2, "10",
	  CMND_COL_I3, "1",
	  CMND_COL_I4, "10",
	  CMND_COL_F1, "-1.35",
	  CMND_COL_F2, "0.0",
	  CMND_COL_F3, "-1.35",
	  CMND_COL_F4, "0.3",
	  CMND_COL_F5, "0.0",
	  CMND_COL_F6, "0.3",
	  -1 );

  /* Append a radiation pattern (RP) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store, &iter,
	  CMND_COL_NAME, "RP",
	  CMND_COL_I1, "0",
	  CMND_COL_I2, "19",
	  CMND_COL_I3, "37",
	  CMND_COL_I4, "1000",
	  CMND_COL_F1, "0.0",
	  CMND_COL_F2, "0.0",
	  CMND_COL_F3, "10.0",
	  CMND_COL_F4, "10.0",
	  CMND_COL_F5, "0.0",
	  CMND_COL_F6, "0.0",
	  -1 );

  /* Append a file end (EN) card */
  gtk_list_store_append( cmnd_store, &iter );
  gtk_list_store_set( cmnd_store,
	  &iter, CMND_COL_NAME, "EN", -1 );
  for( idi = CMND_COL_I1; idi < CMND_NUM_COLS; idi++ )
	gtk_list_store_set( cmnd_store, &iter, idi, "0", -1 );

} /* Create_Default_File() */

/*------------------------------------------------------------------------*/

/* List_Comments()
 *
 * Reads comments from file and lists in tree view
 */
  static void
List_Comments( void )
{
  GtkTreeIter iter;
  gboolean ret;
  GtkTreeSelection *selection =
	gtk_tree_view_get_selection( cmnt_treeview );

  /* "Card" mnemonic and line buffer */
  char ain[3], line_buf[LINE_LEN];

  /* Check that store is empty */
  ret = gtk_tree_model_get_iter_first(
	  GTK_TREE_MODEL(cmnt_store), &iter );

  /* Keep reading till the CE card */
  do
  {
	/* Read a line from input file */
	if( Load_Line(line_buf, input_fp) == EOF )
	  Stop( _("List_Comments():\n"
			"Error reading input file\n"
			"Unexpected EOF (End of File)"), ERR_OK );

	/* Check for short or missing CM or CE and fix */
	if( strlen(line_buf) < 2 )
	{
	  Stop( _("List_Comments():\n"
			"Error reading input file\n"
			"Comment mnemonic short or missing"), ERR_OK );
	  Strlcpy( line_buf, "XX ", sizeof(line_buf) );
	}

	/* If only mnemonic in card,
	 * "cut" the rest of line buffer */
	if( strlen(line_buf) == 2 ) line_buf[3] = '\0';

	/* Separate card's id mnemonic */
	Strlcpy( ain, line_buf, sizeof(ain) );

	/* Append a comment row and fill in text if opening call */
	if( !ret ) gtk_list_store_append( cmnt_store, &iter );
	gtk_list_store_set(
		cmnt_store, &iter,
		CMNT_COL_NAME, ain,
		CMNT_COL_COMMENT,
		&line_buf[3], -1 );

	/* Get new row if available */
	ret = gtk_tree_model_iter_next(
		GTK_TREE_MODEL(cmnt_store), &iter);

  } /* do */
  while( strcmp(ain, "CE") != 0 );

  /* Select the first row */
  ret = gtk_tree_model_get_iter_first(
	  GTK_TREE_MODEL(cmnt_store), &iter );
  if( ret ) gtk_tree_selection_select_iter( selection, &iter );

} /* List_Comments() */

/*------------------------------------------------------------------------*/

/* List_Geometry()
 *
 * Reads geometry cards from file and lists in tree view
 */
  static void
List_Geometry( void )
{
  GtkTreeIter iter;

  /* "Card" mnemonic */
  char ain[3];

  /* int data from cards */
  int iv[4];

  /* float data from cards */
  double fv[7];

  /* For snprintf */
  char si[4][7], sf[7][13];

  int idx;
  gboolean ret;


  /* Check that store is empty */
  ret = gtk_tree_model_get_iter_first(
	  GTK_TREE_MODEL(geom_store), &iter );
  do
  {
	/* Read a geometry card. Errors are handled in readgm() */
	if( !readgm( ain, &iv[0], &iv[1],
		&fv[0], &fv[1], &fv[2], &fv[3],
		&fv[4], &fv[5], &fv[6]) )
	  break;

	/* Ignore in-data (NEC4 style) comments */
	if( strcmp(ain, "CM") == 0 ) continue;

	/* Format card data and print to string */
	snprintf( si[0], 6, "%5d",  iv[0] );
	snprintf( si[1], 7, "%5d ", iv[1] );
	for( idx = GEOM_COL_F1; idx <= GEOM_COL_F7; idx++ )
	  snprintf( sf[idx-GEOM_COL_F1], 13,
		  "%12.5E", (double)fv[idx-GEOM_COL_F1] );

	/* Append a comment row and fill in text if opening call */
	if( !ret )
	  gtk_list_store_append( geom_store, &iter );

	/* Set data to list store */
	gtk_list_store_set(
		geom_store, &iter, GEOM_COL_NAME, ain, -1 );
	for( idx = GEOM_COL_I1; idx <= GEOM_COL_I2; idx++ )
	  gtk_list_store_set(
		  geom_store, &iter, idx, si[idx-GEOM_COL_I1], -1 );
	for( idx = GEOM_COL_F1; idx <= GEOM_COL_F7; idx++ )
	  gtk_list_store_set(
		  geom_store, &iter, idx, sf[idx-GEOM_COL_F1], -1 );

	/* Get new row if available */
	ret = gtk_tree_model_iter_next(
		GTK_TREE_MODEL(geom_store), &iter);
  }
  while( strcmp(ain, "GE") != 0 );

} /* List_Geometry() */

/*------------------------------------------------------------------------*/

/* List_Commands()
 *
 * Reads command cards from file and lists in tree view
 */
  static void
List_Commands( void )
{
  GtkTreeIter iter;

  /* "Card" mnemonic and line buffer */
  char ain[3];

  /* int data from cards */
  int iv[4];

  /* float data from cards */
  double fv[7];

  /* For snprintf */
  char si[4][7], sf[7][13];

  int idx;
  gboolean ret;


  /* Check that store is empty */
  ret = gtk_tree_model_get_iter_first(
	  GTK_TREE_MODEL(cmnd_store), &iter );
  do
  {
	/* Read a command card. Errors are handled in readmn() */
	readmn(
		ain, &iv[0], &iv[1], &iv[2], &iv[3], &fv[0],
		&fv[1], &fv[2], &fv[3], &fv[4], &fv[5] );

	/* Ignore in-data (NEC4 style) comments */
	if( strcmp(ain, "CM") == 0 ) continue;

	/* Format card data and print to string */
	for( idx = CMND_COL_I1; idx < CMND_COL_I4; idx++ )
	  snprintf( si[idx-CMND_COL_I1], 6, "%5d", iv[idx-CMND_COL_I1] );

	/* For alignment of data printed to NEC2 file */
	snprintf( si[idx-CMND_COL_I1], 7, " %5d", iv[idx-CMND_COL_I1] );
	for( idx = CMND_COL_F1; idx <= CMND_COL_F6; idx++ )
	  snprintf( sf[idx-CMND_COL_F1], 13,
		  "%12.5E", (double)fv[idx-CMND_COL_F1] );

	/* Append a command row and fill in text if opening call */
	if( !ret )
	  gtk_list_store_append( cmnd_store, &iter );

	/* Set data to list store */
	gtk_list_store_set(
		cmnd_store, &iter, CMND_COL_NAME, ain, -1 );
	for( idx = CMND_COL_I1; idx <= CMND_COL_I4; idx++ )
	  gtk_list_store_set(
		  cmnd_store, &iter, idx, si[idx-CMND_COL_I1], -1 );
	for( idx = CMND_COL_F1; idx <= CMND_COL_F6; idx++ )
	  gtk_list_store_set(
		  cmnd_store, &iter, idx, sf[idx-CMND_COL_F1], -1 );

	/* Get new row if available */
	ret = gtk_tree_model_iter_next(
		GTK_TREE_MODEL(cmnd_store), &iter);
  }
  while( strcmp(ain, "EN") != 0 );

} /* List_Commands() */

/*------------------------------------------------------------------------*/

/* Nec2_Input_File_Treeview()
 *
 * Reads a NEC2 input file and renders it in a tree view
 */
  void
Nec2_Input_File_Treeview( int action )
{
  /* Abort if editor window is not opened */
  if( nec2_edit_window == NULL ) return;

  /* Implement user action */
  switch( action )
  {
	case NEC2_EDITOR_REVERT: /* Revert editor to file contents */
	  /* Clear all tree view list stores */
	  gtk_list_store_clear( cmnt_store );
	  gtk_list_store_clear( geom_store );
	  gtk_list_store_clear( cmnd_store );
	  SetFlag( NEC2_EDIT_SAVE );
	  break;

	case NEC2_EDITOR_NEW: /* Create new default input file */
	  /* If tree view stores are already
	   * created, just make the new file */
	  Create_List_Stores(); /* Only done if needed */
	  Create_Default_File();
	  ClearFlag( OPEN_NEW_NEC2 );
	  return;

	case NEC2_EDITOR_RELOAD: /* Just reload input file */
	  Create_List_Stores();  /* Only done if needed */
	  break;

	case NEC2_EDITOR_CLEAR: /* Just clear the tree view */
	  gtk_list_store_clear( cmnt_store );
	  gtk_list_store_clear( geom_store );
	  gtk_list_store_clear( cmnd_store );
	  break;

  } /* switch( action ) */

  /* Rewind NEC2 input file */
  rewind( input_fp );

  /*** List Comment cards ***/
  List_Comments();

  /*** List Geometry cards ***/
  List_Geometry();

  /*** Read Command cards ***/
  List_Commands();

  return;
} /* Nec2_Input_File_Treeview() */

/*------------------------------------------------------------------------*/

/* cell_edited_callback()
 *
 * Text cell edited callback
 */
  void
cell_edited_callback(
	GtkCellRendererText *cell,
	gchar				*path,
	gchar               *new_text,
	gpointer             user_data )
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  guint column;

  column = GPOINTER_TO_UINT(
	  g_object_get_data(G_OBJECT(cell), "column") );
  selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(user_data) );
  gtk_tree_selection_get_selected( selection, &model, &iter );

  /* Blank cells cause problems */
  if( strcmp(new_text, "") == 0 )
  {
	gchar *name;

	gtk_tree_model_get( model, &iter, 0, &name, -1 );
	if( strcmp(name, "CE") == 0 )
	  gtk_list_store_set( GTK_LIST_STORE(model),
		  &iter, column, _("End Comments"), -1 );
	else
	  gtk_list_store_set( GTK_LIST_STORE(model),
		  &iter, column, "0", -1 );
	g_free(name);
  }
  else
	gtk_list_store_set( GTK_LIST_STORE(model),
	  &iter, column, new_text, -1 );

  SetFlag( NEC2_EDIT_SAVE );

} /* cell_edited_callback() */

/*------------------------------------------------------------------------*/

/* Save_Treeview_Data()
 *
 * Saves tree view data to an open NEC2 input file
 */
  static void
Save_Treeview_Data( GtkTreeView *tree_view, int ncols, FILE *nec2_fp )
{
  GtkTreeModel *list_store;
  GtkTreeIter iter;
  gboolean valid;
  int idx;

  /* Abort if no open file to sane to */
  if( nec2_fp == NULL )
  {
	Stop( _("Cannot save treeview data\n"
		  "Please use the Save button\n"
		  "to specify a file path"), ERR_STOP );
	return;
  }

  /* Get the first iter in the list */
  list_store = GTK_TREE_MODEL( gtk_tree_view_get_model(tree_view) );
  valid = gtk_tree_model_get_iter_first( list_store, &iter );

  /* Walk through all rows and print data to file */
  while( valid )
  {
	gchar *str_data;

	for( idx = 0; idx < ncols; idx++ )
	{
	  gtk_tree_model_get( list_store, &iter, idx, &str_data, -1 );
	  fprintf( nec2_fp, "%s ", str_data );
	  g_free( str_data );
	}

	/* Overwrite last space with newline */
	if( fseek(nec2_fp, -1, SEEK_CUR) == 0 )
	  fprintf( nec2_fp, "\n" );

	valid = gtk_tree_model_iter_next( list_store, &iter );
  } /* while( valid ) */

} /* Save_Treeview_Data() */

/*------------------------------------------------------------------------*/

/* Save_Nec2_Input_File()
 *
 * Saves the data in a NEC2 input treeview to a given filename
 */
  void
Save_Nec2_Input_File( GtkWidget *treeview_window, char *nec2_file )
{
  FILE *nec2_fp = NULL;

  /* Abort if editor window is not opened */
  if( nec2_edit_window == NULL ) return;

  /* Open NEC2 input file for writing */
  if( strlen(nec2_file) == 0 ) return;
  if( !Open_File(&nec2_fp, nec2_file, "w") ) return;

  /* Save comments to file */
  Save_Treeview_Data( cmnt_treeview, CMNT_NUM_COLS, nec2_fp );

  /* Save geometry to file */
  Save_Treeview_Data( geom_treeview, GEOM_NUM_COLS, nec2_fp );

  /* Save commands to file */
  Save_Treeview_Data( cmnd_treeview, CMND_NUM_COLS, nec2_fp );

  /* Re-open file in read mode */
  Close_File( &nec2_fp );

  ClearFlag( NEC2_SAVE );
  ClearFlag( NEC2_EDIT_SAVE );
} /* Save_Nec2_Input_File() */

/*------------------------------------------------------------------------*/

