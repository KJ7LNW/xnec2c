#include "shared.h"

/* One row per window carrying a frequency-sweep transport.  The list ends on
 * a NULL builder, a value outside the domain of a live window; a row whose
 * builder holds NULL names a window that is currently closed. */
typedef struct {
  GtkBuilder **builder;
  const char  *play;
  const char  *image;
  const char  *rewind;
} sweep_control_row_t;

static const sweep_control_row_t sweep_controls[] = {
  { .builder = &main_window_builder,        .play   = "main_loop_start",
    .image   = "main_loop_play_image",      .rewind = "main_loop_reset" },
  { .builder = &rdpattern_window_builder,   .play   = "rdpatttern_loop_start",
    .image   = "rdpattern_loop_play_image", .rewind = "rdpattern_loop_reset" },
  { .builder = &freqplots_window_builder,   .play   = "plot_loop_start",
    .image   = "plot_loop_play_image",      .rewind = "plot_loop_reset" },
  { .builder = NULL },
};

/**
 * freq_sweep_controls_apply - write the sweep state onto every transport
 *
 * Runs on the GTK main thread as the idle source posted by
 * freq_sweep_controls_refresh().  The play button carries the action its next
 * press performs, so a sweep under way shows the pause face; rewind takes a
 * sweep back to its first step and is offered only once the driver is gone.
 */
static void
freq_sweep_controls_apply( void )
{
  gboolean    active = freq_sweep_active();
  const char *icon   = active ? "media-playback-pause" : "media-playback-start";
  const char *play_tip;
  const char *rewind_tip;

  if( active )
    play_tip = _("Pause the frequency sweep");
  else if( freq_sweep_paused() )
    play_tip = _("Resume the frequency sweep");
  else
    play_tip = _("Run the frequency sweep");

  /* The disabled face names the press that makes rewind available again, and
   * both faces name the results the reset discards. */
  if( active )
    rewind_tip = _("Pause the frequency sweep to reset it and discard every calculated result");
  else
    rewind_tip = _("Reset the frequency sweep to its first step and discard every calculated result");

  for( const sweep_control_row_t *row = sweep_controls; row->builder != NULL; row++ )
  {
    if( *row->builder == NULL )
      continue;

    gtk_image_set_from_icon_name(
        GTK_IMAGE(Builder_Get_Object(*row->builder, row->image)),
        icon, GTK_ICON_SIZE_LARGE_TOOLBAR );

    gtk_widget_set_tooltip_text(
        Builder_Get_Object(*row->builder, row->play), play_tip );

    GtkWidget *rewind = Builder_Get_Object( *row->builder, row->rewind );

    gtk_widget_set_sensitive( rewind, !active );
    gtk_widget_set_tooltip_text( rewind, rewind_tip );
  }
}

/**
 * freq_sweep_controls_refresh - bring every transport up to the sweep state
 *
 * Callable from any thread: the widget work is posted to the GTK main thread,
 * so a sweep worker announcing its own transition reaches the buttons safely.
 */
void
freq_sweep_controls_refresh( void )
{
  g_idle_add_once( (GSourceOnceFunc)freq_sweep_controls_apply, NULL );
}
