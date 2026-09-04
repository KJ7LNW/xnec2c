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

#include "render_check.h"
#include "render_message.h"
#include "../shared.h"

/**
 * render_rdpattern_mode_message() - Resolve the missing-mode status text
 *
 * Returns the status text matching the available radiation-pattern and
 * near-field model content.
 */
  static const char *
render_rdpattern_mode_message(void)
{
  gboolean has_rp = isFlagSet(ENABLE_RDPAT);
  gboolean has_nf = isFlagSet(ENABLE_NEAREH);
  const char *message;

  if( !has_rp && !has_nf )
    message = STATUS_MSG_NO_RP_NO_NEAREH;
  else if( !has_rp )
    message = STATUS_MSG_SELECT_NEARFIELD;
  else if( !has_nf )
    message = STATUS_MSG_SELECT_GAINPAT;
  else
    message = STATUS_MSG_SELECT_MODE;

  return message;
}

/*-----------------------------------------------------------------------*/

/**
 * render_check_nearfield() - Resolve near-field preconditions
 * @result: result carrying the frequency step and receiving the outcome
 *
 * Near E/H field mode is already confirmed active by the caller.
 */
  static void
render_check_nearfield(render_check_result_t *result)
{
  /* Near-field view needs at least one field component selected; with none
   * selected the draw yields nothing, so direct the user to enable one. */
  if( !draw_efield_active() && !draw_hfield_active() &&
      !draw_poynting_active() )
  {
    result->status = RENDER_NO_NF_FIELD;
    result->message = STATUS_MSG_SELECT_NF_FIELD;
    return;
  }

  if( isFlagSet(ENABLE_NEAREH) && NF_FSTEP_AVAILABLE(result->fstep) )
  {
    result->mode = RENDER_MODE_NEARFIELD;
    return;
  }

  if( isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) )
  {
    result->status = RENDER_SUPPRESS;
    return;
  }

  if( isFlagClear(ENABLE_NEAREH) )
  {
    result->status = RENDER_NO_NF_CARD;
    result->message = STATUS_MSG_NO_NEAREH_CARDS;
  }
  else
  {
    result->status = RENDER_NF_NOT_READY;
    result->message = STATUS_MSG_START_FREQLOOP;
  }
}

/*-----------------------------------------------------------------------*/

/**
 * render_check_farfield() - Resolve far-field preconditions
 * @result: result carrying the frequency step and receiving the outcome
 *
 * Far-field gain mode is already confirmed active by the caller.
 */
  static void
render_check_farfield(render_check_result_t *result)
{
  if( isFlagClear(ENABLE_RDPAT) )
  {
    result->status = RENDER_NO_RP_CARD;
    result->message = STATUS_MSG_NO_RP_CARD;
    return;
  }

  if( result->fstep < 0 )
  {
    result->status = RENDER_NO_DATA;
    result->message = STATUS_MSG_NO_RDPAT_DATA;
    return;
  }

  result->mode = RENDER_MODE_FARFIELD;
}

/*-----------------------------------------------------------------------*/

/**
 * render_check_rdpattern() - Resolve radiation-pattern mode and preconditions
 * @result: result carrying the frequency step and receiving the outcome
 *
 * Near-field presentation takes priority over far-field presentation.
 */
  static void
render_check_rdpattern(render_check_result_t *result)
{
  if( rdpat_ehfield_active() )
    render_check_nearfield(result);
  else if( rdpat_gain_active() )
    render_check_farfield(result);
  else
  {
    result->status = RENDER_NO_MODE;
    result->message = render_rdpattern_mode_message();
  }

  result->overlay_active = overlay_struct_active();
}

/*-----------------------------------------------------------------------*/

/**
 * render_check() - Resolve frame preconditions and presentation mode
 * @view_type: view whose content is being presented
 *
 * Returns a result with status RENDER_OK when every precondition passes.
 * On failure, message names the status text the caller presents.
 */
  render_check_result_t
render_check(view_type_t view_type)
{
  render_check_result_t result = { .status = RENDER_OK,
    .mode = RENDER_MODE_NONE, .fstep = -1, .message = NULL,
    .overlay_active = FALSE };

  if( isFlagSet(INPUT_PENDING) )
  {
    result.status = RENDER_NO_GEOMETRY;
    result.message = STATUS_MSG_OPEN_FILE;
    return result;
  }

  result.fstep = calc_data.freq_step;

  if( view_type == VIEW_STRUCTURE )
  {
    if( data.n == 0 && data.m == 0 )
    {
      result.status = RENDER_NO_GEOMETRY;
      result.message = STATUS_MSG_OPEN_FILE;
      return result;
    }

    result.mode = RENDER_MODE_STRUCTURE;
  }
  else
    render_check_rdpattern(&result);

  return result;
}
