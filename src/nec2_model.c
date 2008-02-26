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

/* nec2_model.c
 *
 * Structure modelling functions for xnec2c
 */

#include "xnec2c.h"
#include "interface.h"
#include "support.h"
#include "editors.h"
#include "nec2_model.h"

/* Tree list stores */
GtkListStore
  *cmnt_store = NULL,
  *geom_store = NULL,
  *cmnd_store = NULL;

/* NEC2 editor's window */
extern GtkWidget *nec2_edit_window;

/* Pointer to input file */
extern FILE *input_fp;
/* Input file name */
extern char infile[];

extern GtkWidget *error_dialog;
extern GtkWidget *nec2_edit_window;	/* NEC2 editor window */
extern GtkWidget *wire_editor;	/* Wire design window */

/* Various data used by NEC2, stored in this struct */
extern calc_data_t calc_data;

/*------------------------------------------------------------------------*/

/* Nec2_Input_File_Treeview()
 *
 * Reads a NEC2 input file and renders it in a tree view
 */
  int
Nec2_Input_File_Treeview( int action )
{
  /* Comments column names */
  char *cmnt_col_name[CMNT_NUM_COLS] =
  { "Card", "Comments" };

  /* Geometry column names */
  char *geom_col_name[GEOM_NUM_COLS] =
  { "Card", "I1", "I2", "F1", "F2", "F3", "F4", "F5", "F6", "F7" };

  /* Command column names */
  char *cmnd_col_name[CMND_NUM_COLS] =
  { "Card", "I1", "I2", "I3", "I4", "F1", "F2", "F3", "F4", "F5", "F6" };

  GtkTreeIter iter;

  /* "Card" mnemonic and line buffer */
  char ain[3], line_buf[LINE_LEN];

  /* int data from cards */
  int iv[4];

  /* float data from cards */
  long double fv[7];

  /* For sprintf */
  char si[4][7], sf[7][13];

  int idx;
  gboolean ret;


  SetFlag( NEC2_EDIT_SAVE );

  /* Implement user action */
  switch( action )
  {
	case NEC2_EDITOR_REVERT: /* Revert editor to file contents */
	  /* Clear all tree views */
	  gtk_list_store_clear( cmnt_store );
	  gtk_list_store_clear( geom_store );
	  gtk_list_store_clear( cmnd_store );
	  break;

	case NEC2_EDITOR_NEW: /* Create new input file */
	  /* Create comments list store */
	  cmnt_store = gtk_list_store_new(
		  CMNT_NUM_COLS, G_TYPE_STRING,
		  G_TYPE_STRING );

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
		  nec2_edit_window, "nec2_cmnt_treeview",
		  cmnt_store, CMNT_NUM_COLS, cmnt_col_name );

	  /* Insert geometry columns */
	  Insert_Columns(
		  nec2_edit_window, "nec2_geom_treeview",
		  geom_store, GEOM_NUM_COLS, geom_col_name );

	  /* Insert command columns */
	  Insert_Columns(
		  nec2_edit_window, "nec2_cmnd_treeview",
		  cmnd_store, CMND_NUM_COLS, cmnd_col_name );

	  /* Set models to treviews */
	  gtk_tree_view_set_model(
		  GTK_TREE_VIEW(lookup_widget(
			  nec2_edit_window, "nec2_cmnt_treeview")),
		  GTK_TREE_MODEL(cmnt_store) );
	  gtk_tree_view_set_model(
		  GTK_TREE_VIEW(lookup_widget(
			  nec2_edit_window, "nec2_geom_treeview")),
		  GTK_TREE_MODEL(geom_store) );
	  gtk_tree_view_set_model(
		  GTK_TREE_VIEW(lookup_widget(
			  nec2_edit_window, "nec2_cmnd_treeview")),
		  GTK_TREE_MODEL(cmnd_store) );
	  break;

  } /* switch( action ) */

  /* If no already-open input file (File->New menu item activated), */
  /* then create default NEC2 input "cards" when opening editor window */
  if( input_fp == NULL )
  {
	char str[55];

	/* Append a default comment row */
	strcpy( str, "--- NEC2 Input File created by " );
	strcat( str, PACKAGE"-"VERSION);
	strcat( str, " ---" );
	gtk_list_store_append( cmnt_store, &iter );
	gtk_list_store_set(
		cmnt_store, &iter,
		CMNT_COL_NAME, "CM",
		CMNT_COL_COMMENT, str, -1 );

	/* Append a default CE card */
	gtk_list_store_append( cmnt_store, &iter );
	gtk_list_store_set(
		cmnt_store, &iter,
		CMNT_COL_NAME, "CE",
		CMNT_COL_COMMENT,
		"--- End Comments ---",
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
		CMND_COL_I4, "0",
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
	for( idx = CMND_COL_I1; idx < CMND_NUM_COLS; idx++ )
	  gtk_list_store_set( cmnd_store, &iter, idx, "0", -1 );

  } /* if( input_fp == NULL ) */
  else
  {
	/*** Read NEC2 input file and show in Treeviews ***/
	/* Rewind NEC2 input file */
	rewind( input_fp );

	/* Clear all tree views */
	gtk_list_store_clear( cmnt_store );
	gtk_list_store_clear( geom_store );
	gtk_list_store_clear( cmnd_store );

	/*** Read comments ***/
	/* Read a line from input file */
	if( load_line(line_buf, input_fp) == EOF )
	  stop( "Error reading input file:\n"
		  "Unexpected EOF (End of File)", 1 );

	/* Separate card's id mnemonic */
	strncpy( ain, line_buf, 2 );
	ain[2] = '\0';

	/* Check that store is empty */
	ret = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(cmnt_store), &iter );

	/* Keep reading till a non "CM" card */
	while( (strcmp(ain, "CM") == 0) ||
		(strcmp(ain, "CE") == 0)  )
	{
	  /* Append a comment row and fill in text if opening call */
	  if( !ret )
		gtk_list_store_append( cmnt_store, &iter );

	  /* If space missing (after blank CE) */
	  if( line_buf[2] == '\0' )
		line_buf[3] = '\0';
	  gtk_list_store_set(
		  cmnt_store, &iter,
		  CMNT_COL_NAME, ain,
		  CMNT_COL_COMMENT,
		  &line_buf[3],
		  -1 );

	  /* Stop after CE card */
	  if( strcmp(ain, "CE") == 0 )
		break;

	  /* Read a line from input file */
	  if( load_line(line_buf, input_fp) == EOF )
		stop( "Error reading input file:\n"
			"Unexpected EOF (End of File)", 1 );

	  /* Separate card's id mnemonic */
	  strncpy( ain, line_buf, 2 );
	  ain[2] = '\0';

	  /* Get new row if available */
	  ret = gtk_tree_model_iter_next(
		  GTK_TREE_MODEL(cmnt_store), &iter);

	} /* while( strcmp(ain, "CM") == 0 ) */

	/* No "ce" card at end of comments */
	if( strcmp(ain, "CE") != 0 )
	{
	  stop( "No CE card at end of Comments\n"
		  "Appending a default CE card", 0 );

	  /* Append a default CE card */
	  gtk_list_store_append( cmnt_store, &iter );
	  gtk_list_store_set(
		  cmnt_store, &iter,
		  CMNT_COL_NAME, "CE",
		  CMNT_COL_COMMENT,
		  "End Comments", -1 );

	  /* Return to start of last line */
	  fseek( input_fp, (long)(-strlen(line_buf)-1), SEEK_CUR );

	} /* if( strcmp(ain, "CE") != 0 ) */

	/*** Read geometry ***/
	/* Check that store is empty */
	ret = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(geom_store), &iter );
	do
	{
	  /* Read a geometry card */
	  readgm(
		  ain, &iv[0], &iv[1],
		  &fv[0], &fv[1], &fv[2], &fv[3],
		  &fv[4], &fv[5], &fv[6], 0 );

	  /* Abort if mnemonic invalid */
	  if( strlen(ain) < 2 ) return(0);

	  /* Ignore in-data (NEC4 style) comments */
	  if( strcmp(ain, "CM") == 0 ) continue;

	  /* Format card data and print to string */
	  snprintf( si[0], 6, "%5d",  iv[0] );
	  snprintf( si[1], 6, "%5d ", iv[1] );
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

	/*** Read Commands ***/
	/* Check that store is empty */
	ret = gtk_tree_model_get_iter_first(
		GTK_TREE_MODEL(cmnd_store), &iter );
	do
	{
	  /* Read a command card */
	  readmn(
		  ain, &iv[0], &iv[1], &iv[2], &iv[3], &fv[0],
		  &fv[1], &fv[2], &fv[3], &fv[4], &fv[5], 0 );

	  /* Abort if command mnemonic invalid */
	  if( strlen(ain) < 2 ) return(0);

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

  } /* End else of if( input_fp == NULL ) */

  SetFlag( NEC2_SAVE );
  return(0);

} /* Nec2_Input_File_Treeview() */

/*------------------------------------------------------------------------*/

/* Inserts columns in a list store */
void
Insert_Columns(
	GtkWidget *window, gchar *treeview,
	GtkListStore* store, int ncols, char *colname[] )
{
  int idx;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;

  GtkWidget *view = lookup_widget( window, treeview );
  for( idx = 0; idx < ncols; idx++ )
  {
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect( renderer, "edited"
		, (GCallback) cell_edited_callback, view );
	g_object_set_data( G_OBJECT(renderer),
		"column", GUINT_TO_POINTER(idx) );
	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view), -1, colname[idx],
		renderer, "text", idx, NULL );
  }
  model = GTK_TREE_MODEL(store);
  gtk_tree_view_set_model( GTK_TREE_VIEW (view), model );
  /* destroy model automatically with view */
  g_object_unref( model );

} /* Insert_Columns() */

/*------------------------------------------------------------------------*/

/* cell_edited_callback()
 *
 * Text cell edited callback
 */
void
cell_edited_callback(
	GtkCellRendererText *cell,
	gchar               *path_string,
	gchar               *new_text,
	gpointer             user_data )
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  guint column;

  column = GPOINTER_TO_UINT(
	  g_object_get_data(G_OBJECT(cell), "column") );
  selection = gtk_tree_view_get_selection(
	  GTK_TREE_VIEW(user_data) );
  gtk_tree_selection_get_selected(
	  selection, &model, &iter );

  /* Blank cells cause problems */
  if( strcmp(new_text, "") == 0 )
  {
	gchar *name;

	gtk_tree_model_get( model, &iter, 0, &name, -1 );
	if( strcmp(name, "CE") == 0 )
	  gtk_list_store_set( GTK_LIST_STORE(model),
		  &iter, column, "End Comments", -1 );
	else
	  gtk_list_store_set( GTK_LIST_STORE(model),
		  &iter, column, "0", -1 );
	g_free(name);
  }
  else
	gtk_list_store_set( GTK_LIST_STORE(model),
		&iter, column, new_text, -1 );

} /* cell_edited_callback() */

/*------------------------------------------------------------------------*/

/* Save_Nec2_Input_File()
 *
 * Saves the data in a NEC2 input treeview to a given filename
 */
  void
Save_Nec2_Input_File( GtkWidget *treeview_window, char *nec2_file )
{
  FILE *nec2_fp = NULL;
  GtkTreeView *tree_view;


  /* Open NEC2 input file for writing */
  Open_File( &nec2_fp, nec2_file, "w" );

  /* Save comments to file */
  tree_view = GTK_TREE_VIEW( lookup_widget(
		treeview_window, "nec2_cmnt_treeview") );
  Save_Treeview_Data( tree_view, CMNT_NUM_COLS, nec2_fp );

  /* Save geometry to file */
  tree_view = GTK_TREE_VIEW( lookup_widget(
		treeview_window, "nec2_geom_treeview") );
  Save_Treeview_Data( tree_view, GEOM_NUM_COLS, nec2_fp );

  /* Save commands to file */
  tree_view = GTK_TREE_VIEW( lookup_widget(
		treeview_window, "nec2_cmnd_treeview") );
  Save_Treeview_Data( tree_view, CMND_NUM_COLS, nec2_fp );

  Close_File( &nec2_fp );

} /* Save_Nec2_Input_File() */

/*------------------------------------------------------------------------*/

/* Save_Treeview_Data()
 *
 * Saves tree view data to an open NEC2 input file
 */
  void
Save_Treeview_Data( GtkTreeView *tree_view, int ncols, FILE *nec2_fp )
{
  GtkTreeModel *list_store;
  GtkTreeIter iter;
  gboolean valid;
  int idx;


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
	fseek( nec2_fp, -1, SEEK_CUR );
	fprintf( nec2_fp, "\n" );

	valid = gtk_tree_model_iter_next( list_store, &iter );
  }

} /* Save_Treeview_Data() */

/*------------------------------------------------------------------------*/

/* Helper function */
  gboolean
gtk_tree_model_iter_previous(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
  GtkTreePath *path;
  gboolean ret;

  path = gtk_tree_model_get_path (tree_model, iter);
  ret = gtk_tree_path_prev (path);
  if (ret == TRUE)
	gtk_tree_model_get_iter (tree_model, iter, path);
  gtk_tree_path_free (path);
  return ret;
}

/*------------------------------------------------------------------------*/

