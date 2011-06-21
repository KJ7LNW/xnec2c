/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
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

/* callback_func.c
 *
 * Functions to handle GTK callbacks for xnec2c
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <ctype.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "editors.h"
#include "fork.h"
#include <wait.h>
#include "xnec2c.h"

/* Data for various calculations */
extern calc_data_t calc_data;
extern data_t data;
extern save_t save;

/* Window widgets */
extern GtkWidget
  *main_window,
  *freqplots_window,
  *rdpattern_window,
  *nec2_edit_window;

/* Drawing area widgets */
extern GtkWidget
  *structure_drawingarea,
  *freqplots_drawingarea,
  *rdpattern_drawingarea;

/* Quit dialog widget */
extern GtkWidget *quit_dialog;

/* Used to kill window deleted by user */
extern GtkWidget *kill_window;

/* Frequency loop idle function tag */
extern gint floop_tag;

/* Commands between parent and child processes */
extern char *comnd[];

/* common /fpat/ */
extern fpat_t fpat;

/* Program forked flag */
extern gboolean FORKED;

/* Number of forked child processes & forked flag */
extern int nchild;

/* pointers to input/output files */
extern FILE *input_fp;

/* Forked process data */
extern forkpc_t **forkpc;

/* Magnitude of seg/patch current/charge */
extern double *cmag, *ct1m, *ct2m;

/*-----------------------------------------------------------------------*/

/* Save_Pixmap()
 *
 * Saves pixmaps as png files
 */
  void
Save_Pixmap(
	GdkPixmap *pixmap, int pixmap_width,
	int pixmap_height, char *filename )
{
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  /* Get image from pixmap */
  gtk_widget_grab_focus( structure_drawingarea );
  pixbuf = gdk_pixbuf_get_from_drawable(
	  NULL, pixmap, gdk_drawable_get_colormap(pixmap),
	  0, 0, 0, 0, pixmap_width, pixmap_height );

  /* Save image as PNG file */
  gdk_pixbuf_save( pixbuf, filename, "png", &error, NULL );
  gdk_pixbuf_unref( pixbuf );

} /* Save_Pixmap() */

/*-----------------------------------------------------------------------*/

/* New_Viewer_Angle()
 *
 * Sets parameters for a new viewer angle
 */
void
New_Viewer_Angle(
	double wr, double wi,
	GtkSpinButton *wr_spb,
	GtkSpinButton *wi_spb,
	projection_parameters_t *params )
{
  /* Recalculate projection paramenters */
  params->Wr = wr;
  params->Wi = wi;
  gtk_spin_button_set_value( wr_spb, (gdouble)params->Wr );
  gtk_spin_button_set_value( wi_spb, (gdouble)params->Wi );

} /* New_Viewer_Angle() */

/*-----------------------------------------------------------------------*/

/* Create_Pixmap()
 *
 * Creates or resizes a pixmap after a configure event
 */
void
Create_Pixmap(
	GdkPixmap **pixmap,
	int *pixmap_width,
	int *pixmap_height,
	GtkWidget *widget,
	GdkEventConfigure *event,
	projection_parameters_t *params )
{
  /* Create or resize pixmap */
  if( *pixmap != NULL )
  {
	g_object_unref( *pixmap );
	*pixmap = NULL;
  }

  *pixmap = gdk_pixmap_new(
	  widget->window,
	  event->width,
	  event->height, -1 );
  *pixmap_width  = event->width;
  *pixmap_height = event->height;

  /* Calculate new projection parameters */
  if( params != NULL )
	New_Projection_Parameters( *pixmap_width, *pixmap_height, params );

} /* Create_Pixmap() */

/*-----------------------------------------------------------------------*/

/* Motion_Event()
 *
 * Handles pointer motion event on drawingareas
 */
void
Motion_Event(
	GdkEventMotion  *event, projection_parameters_t *params,
	GtkSpinButton *wr_spb, GtkSpinButton *wi_spb )
{
  /* Save previous pointer position */
  static int x_old=0, y_old=0;
  int x, y;

  x = (int)(event->x/2.0);
  y = (int)(event->y/2.0);

  /* Recalculate projection parameters according to pointer motion.
   * Setting rotate and incline values to spinbuttons triggers redraw */
  if( x > x_old )
  {
	params->Wr -= params->W_incr;
	gtk_spin_button_set_value( wr_spb, (gdouble)params->Wr );
	x_old = x;
  }
  else
	if( x < x_old )
	{
	  params->Wr += params->W_incr;
	  gtk_spin_button_set_value( wr_spb, (gdouble)params->Wr );
	  x_old = x;
	}

  if( y > y_old )
  {
	params->Wi += params->W_incr;
	gtk_spin_button_set_value( wi_spb, (gdouble)params->Wi );
	y_old = y;
  }
  else
	if( y < y_old )
	{
	  params->Wi -= params->W_incr;
	  gtk_spin_button_set_value( wi_spb, (gdouble)params->Wi );
	  y_old = y;
	}

} /* Motion_Event() */

/*-----------------------------------------------------------------------*/

/* Plot_Select()
 *
 * Sets up plotting of requested freq data
 */
  void
Plot_Select( GtkToggleButton *togglebutton, int flag )
{
  if( gtk_toggle_button_get_active(togglebutton) )
  {
	SetFlag( flag | PLOT_SELECT );
	calc_data.ngraph++;
  }
  else
  {
	ClearFlag( flag );
	calc_data.ngraph--;
  }

  /* Trigger a redraw of frequency plots drawingarea */
  if( isFlagSet(PLOT_ENABLED) && isFlagSet(FREQ_LOOP_DONE) )
	Plot_Frequency_Data();

} /* Plot_Select() */

/*-----------------------------------------------------------------------*/

/* Delete_Event()
 *
 * Handles user request to delete a window
 */
  void
Delete_Event( gchar *message )
{
  quit_dialog = create_quit_dialog();
  gtk_widget_show( quit_dialog );

  if( isFlagSet(FREQ_LOOP_RUNNING) )
  {
	if( isFlagSet(MAIN_QUIT) )
	  gtk_label_set_text( GTK_LABEL(lookup_widget(
			  quit_dialog, "quit_label")),
		  "The frequency loop is running\n"
		  "Really end operation?" );
	else
	  gtk_label_set_text( GTK_LABEL(lookup_widget(
			  quit_dialog, "quit_label")),
		  "The frequency loop is running\n"
		  "Really close this window?" );
  }
  else
	gtk_label_set_text( GTK_LABEL(lookup_widget(
			quit_dialog, "quit_label")), message );

} /* Delete_Event() */

/*-----------------------------------------------------------------------*/

/* Set_Pol_Menuitem()
 *
 * Sets the polarization type menuitem to current setting
 */
  void
Set_Pol_Menuitem( GtkMenuItem *menuitem )
{
  gchar *pol_menu[NUM_POL] =
  {
	"rdpattern_total",
	"rdpattern_horizontal",
	"rdpattern_vertical",
	"rdpattern_right_hand",
	"rdpattern_left_hand",
  };

  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(lookup_widget(
		  GTK_WIDGET(menuitem), pol_menu[calc_data.pol_type])), TRUE );

} /* Set_Pol_Menuitem() */

/*-----------------------------------------------------------------------*/

/* Close_Windows()
 *
 * Closes some open windows
 */
  void
Close_Windows( void )
{
  if( isFlagSet(PLOT_ENABLED) )
  {
	gtk_widget_destroy( freqplots_window );
	gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(lookup_widget(
			main_window, "main_freqplots")), FALSE );
  }

  if( isFlagSet(DRAW_ENABLED) )
  {
	gtk_widget_destroy( rdpattern_window );
	gtk_check_menu_item_set_active(
		GTK_CHECK_MENU_ITEM(lookup_widget(
			main_window, "main_rdpattern")), FALSE );
  }

} /* Close_Windows() */

/*-----------------------------------------------------------------------*/

/* Open_Editor()
 *
 * Pops up a Editor window on user
 * right-click on a NEC2 Editor treeview
 */
  gboolean
Open_Editor( GtkTreeView *view )
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *card;
  GtkWidget *button;

  /* Find the selected treeview row */
  selection = gtk_tree_view_get_selection( view );
  if( !gtk_tree_selection_get_selected(selection, &model, &iter) )
	return( FALSE );

  /* Get the "card" name from first column */
  gtk_tree_model_get( model, &iter, 0, &card, -1);

  /* Some "cards" have common editors */
  if( strcmp(card, "GC") == 0 )
	strcpy( card, "GW" );
  else
	if( strcmp(card, "SC") == 0 )
	  strcpy( card, "SP" );
	else
	  if( strcmp(card, "SM") == 0 )
		strcpy( card, "SP" );
	  else
		if( strcmp(card, "NH") == 0 )
		  strcpy( card, "NE" );
		else
		  if( strcmp(card, "GE") == 0 )
		  {
			Gend_Editor( EDITOR_EDIT );
			return( TRUE );
		  }
		  else /* EN Not editable */
			if( strcmp(card, "EN") == 0 )
			  return( TRUE );

  /* Send a "clicked" signal to the appropriate editor button */
  card[0] = tolower(card[0]);
  card[1] = tolower(card[1]);
  button = lookup_widget( GTK_WIDGET(view), card );
  g_free(card);
  if( button != NULL )
	g_signal_emit_by_name( button, "clicked" );
  else
	return( FALSE );

  return( TRUE );

} /* Open_Editor() */

/*-----------------------------------------------------------------------*/

/* Nec2_Apply_Checkbuton()
 *
 * Checks whether the NEC2 editor's "Apply" check button is active
 */

  gboolean
Nec2_Apply_Checkbutton( void )
{
  return( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
		  lookup_widget(nec2_edit_window,
			"nec2_apply_checkbutton" ))) );
}

/*-----------------------------------------------------------------------*/

/* Gtk_Quit()
 *
 * Quits gtk main
 */
  void
Gtk_Quit( void )
{
  int i, k;

  Close_File( &input_fp );
  SetFlag( MAIN_QUIT );

  /* Kill child processes */
  if( FORKED && !CHILD )
	while( nchild )
	  kill( forkpc[--nchild]->chpid, SIGKILL );

  /* Kill possibly nested loops */
  k = gtk_main_level();
  for( i = 0; i < k; i++ )
	gtk_main_quit();

} /* Gtk_Quit() */

/*-----------------------------------------------------------------------*/

/* Pass_EH_Flags
 *
 * Passes near field related flags to child processes
 */
  void
Pass_EH_Flags( void )
{
  char flag;
  size_t cnt;
  int idx;

  /* Abort if not forked */
  if( !FORKED ) return;

  /* Tell child process to calculate near field data */
  cnt = strlen( comnd[EHFIELD] );
  for( idx = 0; idx < calc_data.nfork; idx++ )
	Write_Pipe( idx, comnd[EHFIELD], cnt, TRUE );

  /* Tell child to set near field flags */
  flag = 0;
  if( isFlagSet(DRAW_EHFIELD) )		flag |= 0x01;
  if( isFlagSet(NEAREH_SNAPSHOT) )	flag |= 0x02;
  if( isFlagSet(DRAW_EFIELD) )		flag |= 0x04;
  if( isFlagSet(DRAW_HFIELD) )		flag |= 0x08;

  cnt = sizeof( flag );
  for( idx = 0; idx < calc_data.nfork; idx++ )
	Write_Pipe( idx, &flag, cnt, TRUE );

} /* Pass_EH_Flags */

/*-----------------------------------------------------------------------*/

/* Alloc_Crnt_Buffs()
 *
 * Allocates memory for current/charge draw buffers
 */
  void
Alloc_Crnt_Buffs( void )
{
  size_t mreq = data.m * sizeof( double );
  /* Patch currents buffer */
  if( mreq > 0 )
  {
	mem_realloc( (void *)&ct1m, mreq, "in input.c" );
	mem_realloc( (void *)&ct2m, mreq, "in input.c" );
  }

  /* Segment currents buffer */
  if( data.n > 0 )
	mem_realloc( (void *)&cmag,
		data.n * sizeof(double), "in draw_structure.c" );

} /* Alloc_Crnt_Buffs() */

/*-----------------------------------------------------------------------*/

/* Free_Crnt_Buffs()
 *
 * Frees current/charge draw buffers
 */
  void
Free_Crnt_Buffs( void )
{
  free_ptr( (void *)&ct1m );
  free_ptr( (void *)&ct2m );
  free_ptr( (void *)&cmag );
}

/*-----------------------------------------------------------------------*/

