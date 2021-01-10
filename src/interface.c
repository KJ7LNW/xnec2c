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

#include "interface.h"
#include "shared.h"

/* Gtk_Builder()
 *
 * Returns a GtkBuilder with required objects from file
 */
  static void
Gtk_Builder( GtkBuilder **builder, gchar **object_ids )
{
  GError *gerror = NULL;
  int ret = 0;

  /* Create a builder from object ids */
  *builder = gtk_builder_new();
  ret = (int)gtk_builder_add_objects_from_file(
	  *builder, xnec2c_glade, object_ids, &gerror );
  if( !ret )
  {
	fprintf( stderr, _("Xnec2c: failed to add objects to builder:\n%s\n"),
		gerror->message );

	fprintf( stderr, _("Xnec2c: Listing Object Ids:\n") );
	int idx = 0;
	while( object_ids[idx] != NULL )
	{
	  fprintf( stderr, "%s\n", object_ids[idx] );
	  idx++;
	}

	exit( -1 );
  }

  /* Connect signals if gmodule is supported */
  if( !g_module_supported() )
  {
	fprintf( stderr, _("Xnec2c: lib gmodule not supported\n") );
	exit( -1 );
  }
  gtk_builder_connect_signals( *builder, NULL );

} /* Gtk_Builder() */

/*------------------------------------------------------------------*/

/* Builder_Get_Object()
 *
 * Gets a named object from the GtkBuilder builder object
 */
  GtkWidget *
Builder_Get_Object( GtkBuilder *builder, gchar *name )
{
  GObject *object = gtk_builder_get_object( builder, name );
  if( object == NULL )
  {
	fprintf( stderr,
		_("!! Xnec2c: builder failed to get named object: %s\n"), name );
	exit( -1 );
  }

  return( GTK_WIDGET(object) );
} /* Builder_Get_Object() */

/*------------------------------------------------------------------*/

  GtkWidget *
create_main_window( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { MAIN_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "main_window" );
  return( ret );
}

  GtkWidget *
create_filechooserdialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { FILECHOOSER_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "filechooserdialog" );
  return( ret );
}

  GtkWidget *
create_freqplots_window( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { FREQPLOTS_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "freqplots_window" );
  return( ret );
}

  GtkWidget *
create_rdpattern_window( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { RDPATTERN_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "rdpattern_window" );
  return( ret );
}

  GtkWidget *
create_quit_dialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { QUIT_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "quit_dialog" );
  return( ret );
}

  GtkWidget *
create_error_dialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { ERROR_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "error_dialog" );
  return( ret );
}

  GtkWidget *
create_animate_dialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { ANIMATE_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "animate_dialog" );
  return( ret );
}

  GtkWidget *
create_nec2_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { NEC2_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "nec2_editor" );
  return( ret );
}

  GtkWidget *
create_wire_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { WIRE_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "wire_editor" );
  return( ret );
}

  GtkWidget *
create_patch_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { PATCH_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "patch_editor" );
  return( ret );
}

  GtkWidget *
create_arc_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { ARC_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "arc_editor" );
  return( ret );
}

  GtkWidget *
create_transform_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { TRANSFORM_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "transform_editor" );
  return( ret );
}

  GtkWidget *
create_helix_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { HELIX_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "helix_editor" );
  return( ret );
}

  GtkWidget *
create_reflect_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { REFLECT_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "reflect_editor" );
  return( ret );
}

  GtkWidget *
create_scale_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { SCALE_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "scale_editor" );
  return( ret );
}

  GtkWidget *
create_cylinder_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { CYLINDER_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "cylinder_editor" );
  return( ret );
}

  GtkWidget *
create_kernel_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { KERNEL_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "kernel_command" );
  return( ret );
}

  GtkWidget *
create_execute_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { EXECUTE_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "execute_command" );
  return( ret );
}

  GtkWidget *
create_intrange_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { INTRANGE_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "intrange_command" );
  return( ret );
}

  GtkWidget *
create_zo_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { ZO_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "zo_command" );
  return( ret );
}

  GtkWidget *
create_ground_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { GROUND_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "ground_command" );
  return( ret );
}

  GtkWidget *
create_nearfield_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { NEARFIELD_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "nearfield_command" );
  return( ret );
}

  GtkWidget *
create_radiation_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { RADIATION_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "radiation_command" );
  return( ret );
}

  GtkWidget *
create_excitation_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { EXCITATION_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "excitation_command" );
  return( ret );
}

  GtkWidget *
create_frequency_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { FREQUENCY_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "frequency_command" );
  return( ret );
}

  GtkWidget *
create_loading_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { LOADING_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "loading_command" );
  return( ret );
}

  GtkWidget *
create_network_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { NETWORK_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "network_command" );
  return( ret );
}

  GtkWidget *
create_txline_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { TXLINE_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "txline_command" );
  return( ret );
}

  GtkWidget *
create_ground2_command( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { GROUND2_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "ground2_command" );
  return( ret );
}

  GtkWidget *
create_gend_editor( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { GEND_EDITOR_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "gend_editor" );
  return( ret );
}

  GtkWidget *
create_aboutdialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { ABOUT_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "aboutdialog" );
  return( ret );
}

  GtkWidget *
create_nec2_save_dialog( GtkBuilder **builder )
{
  GtkWidget *ret = NULL;
  gchar *object_ids[] = { SAVE_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  ret = Builder_Get_Object( *builder, "nec2_save_dialog" );
  return( ret );
}

