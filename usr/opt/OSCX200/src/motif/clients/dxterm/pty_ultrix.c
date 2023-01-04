/*
 *  Title:	pty_ultrix
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1987,1991.                                                 |
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
 *	Ultrix-specific pseudo-terminal support code.
 *
 * Modified by:
 *
 * Alfred von Campe     20-Nov-1993	BL-E
 *      - Clean up and exit when we are notified through the pipe.
 *
 * Alfred von Campe     08-Jun-1993     DECterm/BL-C
 *      - Added flag word to p_new_terminal() routine.
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *      - Do the ioctl to change the pty's size only if there's been a change.
 *
 * Alfred von Campe     06-Oct-1991     Hercules/1 T0.7
 *      - Fixed cut & paste problem in vi (from Randall Brown).
 *
 * Bob Messenger	20-May-1989	X2.0-11
 *	- Workaround for a problem in spawn(): it expects to use the two
 *	  channels after the display's fd.
 *
 * Bob Messenger	30-Apr-1989	X2.0-9
 *	- Added tt_chars parameters to p_new_terminal, so it will have the
 *	  same number of parameters as on VMS.
 *
 * Eric Osman		26-Sep-1988	BL10.2
 *
 *	- Incorporate Durga rao's changes:  "extern int errno" and only include
 * 	- time.h if _TIMEH isn't defined.
 *
 * Eric Osman (uh oh, is he in here too?)
 *			 9-Sep-1988	BL10.2
 *	- don't allow strlen() of 0, some machines don't like this
 *
 * Tom Porcher		 3-Jul-1988	X0.4-34
 *	- made p_close() not do un_spawn if pty not opened.
 *
 * Tom Porcher		 3-Jul-1988	X0.4-34
 *	- Made p_new_terminal() create process, too, using spawn() from xterm.
 *
 * Tom Porcher		22-Jun-1988	X0.4-32
 *	- changed XtAddInput() to XtAppAddInput().
 *
 * Tom Porcher
 *	- updated to new interface (see pty_vms.c).
 *
 * Tom Porcher		16-Apr-1988	X0.4-10
 *	- Added ioctl for non-blocking I/O on pty channel to fix
 *	  Ctrl-S hang.
 *
 */

#include "mx.h"
#ifndef _TIMEH
#include <sys/time.h>
#endif
#include <sys/file.h>
/* #include <sys/resource.h> */
#include <pwd.h>
#include <sgtty.h>
#include <stdio.h>
#include <errno.h>
	extern int errno;
#include <signal.h>

globalref char *getty_dev,*slave_pty;
globalref char **cmd_to_exec;
globalref char console_flag;
globalref char login_shell_flag;

#ifndef VMS
globalref int dummy_fd_1, dummy_fd_2;
#endif

extern void pc_dead();
extern void pc_read();
extern void pc_resume_write();

Boolean p_write();
void p_stop_read();
void p_resume_read();

static void read_callback();

globaldef int pipe_fd[2];

void exit_handler(stm, source, id)
STREAM    *stm;
int	  *source;
XtInputId *id;
{
    char buffer[1];

    read (pipe_fd[0], buffer, 1);

/* The toolkit is most likely still blocked at this point, so even though
 * run_flag is 0, our main event loop will not finish and pty_fin() and
 * process_exit will not be called.  So we call them from here instead.
 */
    pty_fin();
    process_exit(0);
}

/*
 * ULTRIX Procedure to obtain channel on a new pty.
 *
 * On Ultrix, this procedure also creates the process that runs
 * on the pty.
 * 
 * Input:
 *	
 *
 * Output:
 *	
 *
 * Value:
 *	true or error code
 *
 */
int p_new_terminal( stm, cols, rows, width, height, pty_name, tt_chars, flags )
    STREAM	*stm;
    int		rows, cols, width, height;
    char	*pty_name;
    char	*tt_chars;	/* not used - VMS compatibility */
    int         flags;          /* not used - VMS compatibility */
{
    int am_slave = 0;
    Arg	arglist[2];
    globalref XtAppContext TEA_app_context;

    if (slave_pty) if ( strlen( slave_pty ) >= 3 )
	sscanf( &slave_pty[2], "%d", &am_slave );
    if (am_slave <= 0) {
	am_slave = 0;
	slave_pty = NULL;
    }

/*
 * spawn expects to use the next two channels above the display's fd.
 * To work around this problem we opened two channels to /dev/null right
 * after opening the display, and now we need to free them before calling
 * spawn.  To make sure this only happens once, we use 0 as a code to
 * indicate that the fd's aren't in use.
 */

    if ( dummy_fd_1 != 0 ) close( dummy_fd_1 ), dummy_fd_1 = 0;
    if ( dummy_fd_2 != 0 ) close( dummy_fd_2 ), dummy_fd_2 = 0;

    spawn( getty_dev, am_slave, slave_pty,
	   cmd_to_exec, login_shell_flag, console_flag,
	   stm->display, rows, cols, width, height, XtWindow(stm->parent),
	   &stm->pty.chan, &stm->pid, &stm->pty.tslot );

    pipe(pipe_fd);
    XtAppAddInput(TEA_app_context, pipe_fd[0], (XtPointer)XtInputReadMask,
		    (XtInputCallbackProc)exit_handler, (XtPointer)stm);

    stm->pty.read_id = NULL;
    stm->pty.write_id = NULL;
    stm->pty.write_bufstr = stm->pty.write_buffer;
    stm->pty.write_bufptr = stm->pty.write_buffer;
    stm->pty.write_bufend = &(stm->pty.write_buffer[4096]);

    p_resume_read( stm );

    return TRUE;
}

/*
 * p_set_terminal_size -- set size of terminal in perspective of host
 *
 *
 */
void
p_set_terminal_size( stm, cols, rows, width, height )
    STREAM *stm;
    int cols,rows,width,height;
{
#ifdef TIOCSWINSZ
	struct winsize ws;

	ioctl (stm->pty.chan, TIOCGWINSZ, &ws);

	if(ws.ws_row != rows || ws.ws_col != cols)
	{
	    ws.ws_row = rows;
	    ws.ws_col = cols;
	    ws.ws_xpixel = width;
	    ws.ws_ypixel = height;
	    ioctl (stm->pty.chan, TIOCSWINSZ, (char *)&ws);
#ifdef SIGWINCH
	    if(stm->pid > 1)
		killpg(getpgrp((int)stm->pid), SIGWINCH);
#endif SIGWINCH
	}
#endif TIOCSWINSZ
}

/*
 * pty_write_callback()
 *
 * This callback occurs when the Xtoolkit says there is room for us to
 * write something to the master pty.
 *
 * NOTE:  There appears to be a toolkit bug when attempting to use
 *	  XtAppAddInput() and XtRemoveInput().  This may be because
 *	  the code below was requesting different callbacks for the
 *	  read side and write side of the master pty.  The toolkit
 *	  eventually creates App->OutstandingQueue to be a linked list
 *	  that is linked to itself.  Therefore, when XtRemoveInput is
 *	  is called, the toolkit gets into a endless loop attempting to
 *	  traverse this linked list.
 *
 */
static void
pty_write_callback( stm, source, id_ptr)
    STREAM	*stm;
    int		*source;
    XtInputId	*id_ptr;
{
#ifndef TOOLKIT_FIX
    /* reset write_id so that p_write() will reset timer if needed */
    stm->pty.write_id = NULL;
#endif
    p_write( stm, 0, 0); /* flush buffer */
}

/*
 * p_write()
 *
 * Procedure to send data to a pty.
 *
 * Input:
 *      stm             channel to send to
 *      adr             where they are
 *      count           how many characters
 *
 * Output:
 *      none
 *
 * Value:
 */

Boolean
p_write( stm, adr, count )
    STREAM      *stm;
    char        *adr;
    int         count;
{
        globalref XtAppContext TEA_app_context;
        int 	r;
	int 	c = count;
	int	fd;
	char	*buffer, *bufstr;
	char 	*bufptr, *bufend;

	fd = stm->pty.chan;

	/* setup local pointers to reduce dereferencing */
	buffer = stm->pty.write_buffer;
	bufstr = stm->pty.write_bufstr;
	bufptr = stm->pty.write_bufptr;
	bufend = stm->pty.write_bufend;

	/* check to see if there is any data in the buffer */
	if (bufptr > bufstr) {
	    /* If count > 0, then add new data to buffer */
	    if (count) {
		if (bufend > bufptr + count) {
		    bcopy(adr, bufptr, count);
		    bufptr += count;
		} else {
		    if (bufstr != buffer) {
			bcopy(bufstr, buffer, bufptr - bufstr);
			bufptr -= bufstr - buffer;
			bufstr = buffer;
		    }
		    if (bufend > bufptr + count) {
			bcopy(adr, bufptr, count);
			bufptr += count;
		    } else if (bufptr < bufend) {
			log_message("Out of buffer space\n");
			c = bufend - bufptr;
			bcopy(adr, bufptr, c);
			bufptr = bufend;
		    } else {
			log_message("Out of buffer space\n");
			c = 0;
		    }
		}
	    }
	    if (bufptr > bufstr) {
		if ((r = write(fd, bufstr, bufptr - bufstr)) <= 0) {
		    r = 0;
		    /* these errno's are expected to happen */
		    if ((errno != EWOULDBLOCK) && (errno != EINTR))
			log_message("p_write() : write failed, errno = %d\n", errno);
		}
		/* r contains number of bytes actually written */
		if ((bufstr += r) >= bufptr)
		    bufstr = bufptr = buffer;
	    }
	} else if (count) {
	    if ((r = write(fd, adr, count)) < 0) {
		r = 0;
		/* these errno's are expected to happen */
		if ((errno != EWOULDBLOCK) && (errno != EINTR))
		    log_message("p_write() : write failed, errno = %d\n", errno);
	    }
	    if (count - r) {
		if (count - r > bufend - buffer) {
		    log_message("Truncating to %d\n", bufend - buffer);
		    count = (bufend - buffer) + r;
		}
		bcopy(adr + r, buffer, count - r);
		bufstr = buffer;
		bufptr = buffer + (count - r);
	    }
	}

#ifdef TOOLKIT_FIX
	if (bufptr > bufstr) {
	    if (stm->pty.write_id == NULL)
		stm->pty.write_id = XtAppAddInput( TEA_app_context,
						  stm->pty.chan,
						  XtInputWriteMask,
						  pty_write_callback,
						  stm );
	} else {
	    if (stm->pty.write_id != NULL) {
		XtRemoveInput(stm->pty.write_id);
		stm->pty.write_id = NULL;
	    }
	}
#else
	if (bufptr > bufstr) {
	    if (stm->pty.write_id == NULL)
		stm->pty.write_id = XtAppAddTimeOut(
				    TEA_app_context, 
				    100, 
				    (XtTimerCallbackProc)pty_write_callback, 
				    stm );
	}
#endif

	/* reset pointers in pty structure */
	stm->pty.write_bufstr = bufstr;
	stm->pty.write_bufptr = bufptr;

	return (TRUE);
	
}

/*
 * p_check_write()
 *
 * Returns TRUE if PTY is able to be written to.
 *
 */

Boolean
p_check_write( stm )
    STREAM	*stm;
{
    int		count;

    return ( TRUE );
}


/*
 * p_clear_write()
 *
 * Reset PTY full state (at least from our point-of-view).
 */

void
p_clear_write( stm )
    STREAM	*stm;
{
}

/*
 * read_callback()
 *
 * This callback occurs when the Xtoolkit says there's input on this channel.
 *
 */

static void
read_callback( stm, source, id_ptr )
    STREAM	*stm;
    int		*source;
    XtInputId	*id_ptr;
{
    char	*data;
    int		count;

    count = read( stm->pty.chan,
		  stm->pty.read_buffer,
		  sizeof(stm->pty.read_buffer) );
    if ( count > 0 )
	pc_read( stm, stm->pty.read_buffer, count );
}


/*
 * p_stop_read()
 *
 * Stop calling pc_read().
 */

void
p_stop_read( stm )
    STREAM	*stm;
{
    if ( stm->pty.read_id != NULL ) {
	XtRemoveInput( stm->pty.read_id );
        stm->pty.read_id = NULL;
    }
}


/*
 * p_resume_read()
 *
 * Resume calling pc_read().
 */

void
p_resume_read( stm )
    STREAM	*stm;
{
    globalref XtAppContext TEA_app_context;

    if ( stm->pty.read_id == NULL )
        stm->pty.read_id = XtAppAddInput( TEA_app_context,
				       stm->pty.chan,
				       (XtPointer)XtInputReadMask,
				       (XtInputCallbackProc)read_callback,
				       stm );
}

/*
 * Procedure to interpret status from read on pty
 *
 * Input:
 *	status			status that was returned in iosb
 *	
 * Value:
 *	TRUE if pty disconnected, or FALSE if still active
 *
 */
int u_disconnected_p (status)

{
       if(status == -1)        /* if service failed...*/
            if (errno == EINTR) /* and status is "interrupted" */
	return TRUE;
	else return FALSE;
}

/*
 * Procedure to close a pty channel.
 *
 * Input:
 *	chan			Channel to device.
 *
 * Output:
 *	none
 *
 * Value:
 *	TRUE
 *
 */
void
p_close( stm )
    STREAM	*stm;
{
    p_stop_read( stm );

    if (stm->pty.chan != 0)
	un_spawn( stm->pty.chan, stm->pty.tslot, getty_dev, (slave_pty != NULL) );

}
