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
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include "anim_dialog.h"
#include "anim_class.h"
#include "anim_phase.h"
#include "anim_phase_control.h"
#include "../callbacks.h"
#include "../chroma/chroma_farfield_polarization.h"
#include "../config_hooks.h"
#include "../rdpattern_ui.h"
#include "../shared.h"

typedef struct
{
  const char  *panel_id;
  GtkBuilder **owner_builder;
} anim_panel_owner_t;

typedef gboolean (*anim_gate_test_t)(void);

typedef struct
{
  anim_gate_test_t test;
  const char      *reason;
} anim_guard_t;

typedef struct
{
  const char         *widget_id;
  const char         *reading;
  const anim_guard_t *guards;
} anim_gate_t;

static gboolean gate_rdpattern_window(void);
static gboolean gate_surface_patches(void);
static gboolean gate_farfield_overlay(void);
static gboolean gate_linear_polarization(void);

static const anim_panel_owner_t anim_panel_owners[] = {
  { "anim_efield",   &rdpattern_window_builder },
  { "anim_hfield",   &rdpattern_window_builder },
  { "anim_poynting", &rdpattern_window_builder },
};

static const anim_panel_owner_t anim_menu_items[] = {
  { "main_structure_animate", &main_window_builder },
  { "rdpattern_animate",      &rdpattern_window_builder },
};

static const anim_guard_t guards_flow_dir[] = {
  { gate_surface_patches,
    N_("Choose how current flow is marked on surface patches.\n"
       "This model has no patches: add SP, SC, or SM patch cards to the"
       " NEC input file.") },
  { NULL, NULL }
};

static const anim_guard_t guards_farfield[] = {
  { gate_rdpattern_window,
    N_("Draw the far-zone field as arrows on the gain surface.\n"
       "In the main window choose View, Radiation Pattern; if that item is"
       " disabled, add an RP card to the NEC input file.") },
  { gate_farfield_overlay,
    N_("Draw the far-zone field as arrows on the gain surface.\n"
       "Tick the check box in the Far Field heading just above.") },
  { NULL, NULL }
};

static const anim_guard_t guards_ff_frame[] = {
  { gate_linear_polarization,
    N_("Choose the reference the vertical and horizontal gain are read"
       " against.\n"
       "Under Polarization in this window select Vertical or Horizontal;"
       " Total, Right Hand, and Left Hand do not use this setting.") },
  { NULL, NULL }
};

static const anim_guard_t guards_playback[] = {
  { anim_any_animating,
    N_("Start advancing the animation phase at the cadence set below.\n"
       "Nothing drawn now changes with phase. Under Display choose"
       " Currents or Charges and an Animated colour or segment scale; for"
       " patches choose Currents and an Animated flow mode. For NE/NH cards"
       " show Near Field and tick E, H, or Poynting; for an RP card show Gain"
       " and tick Far Field.") },
  { NULL, NULL }
};

static const anim_gate_t anim_panel_gates[] = {
  { "animation_applybutton",
    N_("Start advancing the animation phase at the cadence set below.\n"
       "Available when drawn content changes with phase."),
    guards_playback },
  { "anim_flow_dir_button",
    N_("Select how the animated patch current flow is rendered.\n"
       "Mirrors the Visualization menu setting in the main window."),
    guards_flow_dir },
  { "anim_farfield_content",
    N_("Resolve the far-zone field into tangent vectors drawn on"
       " the radiation pattern gain surface.\n"
       "Available while the radiation pattern window is open and Far Field"
       " is selected."),
    guards_farfield },
  { "anim_ff_frame_world",
    N_("Read the vertical and horizontal gain selections against the"
       " spherical basis, so vertical is the theta direction and horizontal"
       " is the phi direction.\n"
       "Available for Vertical or Horizontal polarization."),
    guards_ff_frame },
  { "anim_ff_frame_ludwig3",
    N_("Read the vertical and horizontal gain selections against the"
       " Ludwig-3 co-polar and cross-polar directions, which hold one"
       " orientation across the whole pattern.\n"
       "Available for Vertical or Horizontal polarization."),
    guards_ff_frame },
};

/** gate_rdpattern_window() - Report an open radiation-pattern window */
static gboolean
gate_rdpattern_window(void)
{
  return rdpattern_window_builder != NULL;
}

/** gate_surface_patches() - Report patch-class model content */
static gboolean
gate_surface_patches(void)
{
  return anim_class_available(ANIM_CLASS_STRUCTURE_PATCH);
}

/** gate_farfield_overlay() - Report a selected far-field overlay */
static gboolean
gate_farfield_overlay(void)
{
  return rc_config.overlay_farfield != 0;
}

/** gate_linear_polarization() - Report a turnable polarization frame */
static gboolean
gate_linear_polarization(void)
{
  return ff_frame_turns_pol(calc_data.pol_type);
}

/** anim_menu_sensitivity() - Project class capability onto menu items */
static void
anim_menu_sensitivity(void)
{
  const char *reading = _("Open the animation controls for phase-varying content.\n"
      "Available when the model contains supported animation content.");
  const char *reason = _("Open the animation controls for phase-varying content.\n"
      "This model has nothing to animate: load a NEC input file carrying"
      " wire cards (GW, GA, GH), patch cards (SP, SC, SM), near-field cards"
      " (NE, NH), or a radiation pattern card (RP).");
  gboolean sensitive = anim_any_available();
  size_t i;

  for( i = 0; i < G_N_ELEMENTS(anim_menu_items); i++ )
  {
    GtkWidget *widget;

    if( *anim_menu_items[i].owner_builder == NULL )
      continue;

    widget = Builder_Get_Object(*anim_menu_items[i].owner_builder,
        anim_menu_items[i].panel_id);
    gtk_widget_set_sensitive(widget, sensitive);
    gtk_widget_set_tooltip_text(widget, sensitive ? reading : reason);
  }
}

/** anim_dialog_sensitivity() - Project class truth onto dialog controls */
static void
anim_dialog_sensitivity(void)
{
  size_t i;

  if( animate_dialog == NULL )
    return;

  for( i = 0; i < G_N_ELEMENTS(anim_panel_owners); i++ )
  {
    GtkWidget *widget = Builder_Get_Object(animate_dialog_builder,
        anim_panel_owners[i].panel_id);

    gtk_widget_set_sensitive(widget,
        *anim_panel_owners[i].owner_builder != NULL);
  }

  for( i = 0; i < G_N_ELEMENTS(anim_panel_gates); i++ )
  {
    const anim_guard_t *guard;
    const char *reason = NULL;
    GtkWidget *widget;

    for( guard = anim_panel_gates[i].guards;
         guard->test != NULL && reason == NULL; guard++ )
      reason = guard->test() ? NULL : guard->reason;

    widget = Builder_Get_Object(animate_dialog_builder,
        anim_panel_gates[i].widget_id);
    gtk_widget_set_sensitive(widget, reason == NULL);
    gtk_widget_set_tooltip_text(widget,
        reason == NULL ? _(anim_panel_gates[i].reading) : _(reason));
  }
}

/** anim_panel_sensitivity() - Project class truth onto animation controls */
void
anim_panel_sensitivity(void)
{
  anim_menu_sensitivity();
  anim_dialog_sensitivity();
}

/** anim_dialog_readout_update() - Present the displayed phase
 *
 * Updates the phase readout from the phase context without moving the
 * slider, whose position stays static while the timer animation runs.  The
 * readout is decoupled from the slider thumb.  Slider reads degrees; the
 * context stores radians.
 */
  void
anim_dialog_readout_update(void)
{
  GtkLabel *readout;
  gchar *text;

  if( animate_dialog == NULL )
    return;

  readout = GTK_LABEL( Builder_Get_Object(
        animate_dialog_builder, "animate_phase_value") );
  text = g_strdup_printf( "φ %.0f°", anim_phase_get() * TODEG );
  gtk_label_set_text( readout, text );
  g_free( text );
}

/**
 * show_animate_dialog() - Present synchronized animation controls
 *
 * Creates the dialog on first use and wires its static slider handlers.
 */
  static void
show_animate_dialog(void)
{
  if( animate_dialog == NULL )
  {
    animate_dialog = create_animate_dialog( &animate_dialog_builder );

    anim_phase_slider_attach();
    color_tone_marks_attach();
  }
  gtk_widget_show( animate_dialog );
  config_widget_sync_builder( &animate_dialog_builder );
  config_widget_run_hooks( &animate_dialog_builder );

  /* The open dialog is the whole liveness condition, so the phase context
   * opens once the widgets carry their synchronized state. */
  anim_phase_open();
  hook_color_vis();
  nf_static_menu_sync();

  /* Liveness selects the phase-dependent presentation of every class that
   * carries phase, so each one repaints against the open context. */
  anim_dispatch_context_redraw();
}

/* Both menu entries reach the one dialog, whose controls carry the classes
 * the model admits; the near-field quantity is validated where playback
 * starts, so opening the controls selects no single class. */
  void
on_animate_dialog_activate(
    GtkMenuItem     *_menuitem,
    gpointer         _user_data)
{
  show_animate_dialog();
}

  void
on_animation_okbutton_clicked(
    GtkButton       *_button,
    gpointer         _user_data)
{
  gtk_widget_destroy( animate_dialog );
}

/*-----------------------------------------------------------------------*/

  void
on_animate_dialog_destroy(
    GObject       *_object,
    gpointer       _user_data)
{
  /* Stop all animations when dialog closes */
  anim_phase_control_stop();
  anim_phase_close();

  animate_dialog = NULL;
  g_object_unref( animate_dialog_builder );
  animate_dialog_builder = NULL;

  /* Re-enable the near-field static-baseline menu now the phase no longer
   * drives the vectors. */
  nf_static_menu_sync();

  /* Playback ended with the dialog; rebake and redraw under the static
   * selection now that it is gone. */
  if( main_window_builder != NULL )
    hook_color_vis();

  /* The open dialog is the whole condition admitting the far-zone vectors,
   * so the pattern drops them on this edge. */
  anim_dispatch_context_redraw();
}
