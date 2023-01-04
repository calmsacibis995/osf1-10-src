/*
 *  Title:	process_ultrix
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1987,1988                                                  |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	Ultrix-specific process support code.
 *
 * Modified by:
 *
 * Alfred von Campe     20-Nov-1993	    BL-E
 *      - Clean up and exit if our child process has gone away.
 *
 * Alfred von Campe	15-Oct-1993     BL-E
 *      - Add extern reference for streams structure.
 *
 * Eric Osman          11-June-1992     Sun
 *      - Only exit in SYS_reapchild if the dead child was our user process.
 *
 * Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *	- Don't call destroy() from SYS_reapchild (CLD MUH-1770).
 *
 * Tom Porcher		28-Apr-1988	X0.4-13
 *	- Removed most of the code in this module.  Creating processes
 *	  is now done by p_new_terminal(), calling spawn() from xterm.
 *	- SYS_reapchild() now looks through all streams for dead processes.
 *
 * Tom Porcher		28-Apr-1988	X0.4-13
 *	- Removed code to move around channels; it confuses DRM.
 *
 * Tom Porcher		13-Feb-1988	X0.3-4
 *	- Added code to retrieve columns and rows from terminal widget.
 *	- saved created pid in stm->pid for pty code.
 *
 */

#include "mx.h"
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

globalref char *getty_dev, *slave_pty, run_flag;
extern STREAM *streams[];

/* Procedure to obtain user process for new terminal.
 *
 * This procedure is a no-op on Ultrix.  The process is created
 * by p_new_terminal().
 *
 * Input:
 *	isn			to lookup stream (see STREAM in mx.h)
 *	login_self_flag		1 means login as self, 0 means ask user
 * Output:
 *
 * Value:
 *	true or error code
 *
 */
int u_new_process (isn, login_self_flag)
   int isn, login_self_flag;
{
    return TRUE;
}



extern int pipe_fd[];

void SYS_reapchild()
{
    int		status;
    int		pid;
    int		i;
    STREAM	*stm;
	
    pid  = wait3 (&status, WNOHANG, NULL);
    if (!pid) 
	return;

/*
 * Setting run_flag to 0 will tell main loop we want to exit.  However, we
 * only want to do this if the process that exited is in fact our user process
 * instead of a process used for some other purpose.  But since the toolkit
 * is most likely blocking on something, it will not check run_flag.  Ideally,
 * we would just do our cleanup here, but we can't do any X-calls from signal
 * level.  So we write into a pipe for which we did an XtAppAddInput earlier,
 * and even though the toolkit will still be blocked, it will call our input
 * handler, where we can do our clean up (see pty_ultrix.c).
 */
    for (i = 0; i <= MaxISN; i++)
	if ( (stm = streams[i]) != NULL && stm->pid == pid )
	  {
	    run_flag = 0;
	    write(pipe_fd[1], " ", 1);
	    break;
	  }
}

/*
 * Exit(n)
 *
 * This routine is called from spawn() and other fatal signals.
 */

void Exit( n )
    int n;
{
    int		i;
    STREAM	*stm;

    for (i = 0; i <= MaxISN; i++)
	if ( (stm = streams[i]) != NULL && stm->pid != 0 )
	    un_spawn( stm->pty.chan, stm->pty.tslot, getty_dev, (slave_pty != NULL) );
}
