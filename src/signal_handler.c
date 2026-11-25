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

#include "common.h"
#include "shared.h"
#include "signal_handler.h"

void sig_handler( int signal )
{
  switch( signal )
  {
    case SIGINT:
      if (!CHILD) pr_crit("exiting via user interrupt\n");
      break;

    case SIGSEGV:
      pr_crit("segmentation fault, exiting\n");
      break;

    case SIGFPE:
      pr_crit("floating point exception, exiting\n");
      break;

    case SIGABRT:
      pr_crit("abort signal received, exiting\n");
      break;

    case SIGTERM:
      pr_crit("termination request received, exiting\n");
      break;

    case SIGCHLD:
      {
        int idx;
        pid_t pid = getpid();

        if( !FORKED )
        {
          pr_crit("not forked, ignoring SIGCHLD from pid %d\n", pid);
          return;
        }
        else
        {
          for( idx = 0; idx < calc_data.num_jobs; idx++ )
            if( child_procs[idx]->pid == pid )
            {
              pr_crit("child process pid %d exited\n", pid);
              if( isFlagSet(MAIN_QUIT) ) return;
              else break;
            }
          return;
        }
      }

    default:
      pr_debug("default exit with signal: %d\n", signal);
  } /* switch( signal ) */

  close_child_command_pipes();

  Close_File( &input_fp );

  if( CHILD ) _exit( 0 );
  else exit( signal );

} /* End of sig_handler() */
