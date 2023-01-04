/* 
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: resetshell.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:21:31 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "resetshell.c  1.6  com/lib/curses,3.1,8943 10/20/89 11:57:51";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   reset_shell_mode
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

extern	struct term *cur_term;

#ifdef DIOCSETT

static struct termcb new, old;
#endif
#ifdef LTILDE
static int newlmode, oldlmode;
#endif

#ifdef USG
#define BR(x) (cur_term->x.c_cflag&CBAUD)
#else
#define BR(x) (cur_term->x.sg_ispeed)
#endif

/*
 * NAME:
 *
 * FUNCTION:
 *
 *      Disable CB/UNIX virtual terminals.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Getting the baud rate is different on the two systems.
 *      In either case, a baud rate of 0 hangs up the phone.
 *      Since things are often initialized to 0, getting the phone
 *      hung up on you is a common result of an error in your program.
 *      This is not very friendly, so if the baud rate is 0, we
 *      assume we're doing a reset_xx_mode with no def_xx_mode, and
 *      just don't do anything.
 */

reset_shell_mode()
{
#ifdef DIOCSETT
	/*
	 * Restore any virtual terminal setting.  This must be done
	 * before the TIOCSETN because DIOCSETT will clobber flags like xtabs.
	 */
	old.st_flgs |= TM_SET;
	ioctl(2, DIOCSETT, &old);
#endif
	if ((cur_term != NULL) && BR(Ottyb)) {
		ioctl(cur_term -> Filedes,
#ifdef USG
			TCSETAW,
#else
			TIOCSETN,
#endif
			&(cur_term->Ottyb));
# ifdef LTILDE
	if (newlmode != oldlmode)
		ioctl(cur_term -> Filedes, TIOCLSET, &oldlmode);
# endif
	}

								/* 001 */
#ifdef KEYPAD
	/*							** 001 **
 	 * The _kpmode(0) call below turns off the "keypad_xmit" mode in
 	 * the terminal, in case it had been turned on, so as not to
 	 * affect an application invoked by fork(2) and exec(2) call.
	 * Curses generally assumes the keypad was in "local" mode prior to
	 * startup of the application, so the _kpmode(0) call tries to restore
	 * the initial assumed state.  While the _kpmode(0) call does affect
	 * the current "in curses" state, it is expected that the user's
	 * curses application will call "reset_prog_mode()", prior to any
	 * other curses routines after its fork(2) and exec(2), which will
	 * restore the former keypad state.  That is,
 	 *
 	 *	if (!fork ()) {			# child process
 	 *		reset_shell_mode ();
 	 *		exec (program)
 	 *		exit (-1);
 	 *	}
 	 *	else {
 	 *		wait (&status);	 	# wait for child to exit
 	 *		reset_prog_mode ();
 	 *	}
 	 *
 	 */							/* 001 */
								/* 001 */
	/* if initscr() or newterm() was called, then can reset	** 001 **
	 *	terminal's keypad
	 * else application must use low-level terminfo routines
	 *		to do this.
	 */							/* 001 */
	if (SP) {						/* 001 */
		_kpmode(0);					/* 001 */
 		fflush(stdout);	/* flush escape sequence out to	** 001 **
				 * the terminal			** 001 **
				 */				/* 001 */
	}							/* 001 */
#endif /* KEYPAD */						/* 001 */
}
