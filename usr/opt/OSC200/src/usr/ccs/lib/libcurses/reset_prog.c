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
static char rcsid[] = "@(#)$RCSfile: reset_prog.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:20:53 $";
#endif
/*
 * HISTORY
 */
/*** "reset_prog.c  1.6  com/lib/curses,3.1,8943 10/20/89 11:57:34"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   reset_prog_mode
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
 * NAME:        reset_prog_mode
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

_reset_prog_mode()						/* 001 */
{
	if (BR(Nttyb))
		ioctl(cur_term -> Filedes,
#ifdef USG
			TCSETAW,
#else
			TIOCSETN,
#endif
			&(cur_term->Nttyb));
# ifdef LTILDE
	ioctl(cur_term -> Filedes, TIOCLGET, &oldlmode);
	newlmode = oldlmode & ~LTILDE;
	if (newlmode != oldlmode)
		ioctl(cur_term -> Filedes, TIOCLSET, &newlmode);
# endif
#ifdef DIOCSETT
	if (old.st_termt == 0)
		ioctl(2, DIOCGETT, &old);
	new = old;
	new.st_termt = 0;
	new.st_flgs |= TM_SET;
	ioctl(2, DIOCSETT, &new);
#endif
}								/* 001 */

/*								** 001 **
 * This routine is only called from outside the curses library.  Because
 * this routine is being called, we have presumably lost our current state
 * so that (among other things) the keypad is currently in an unknown state.
 * This routine calls _reset_prog_mode to reset all of our state except the
 * keypad, and then it sets the keypad state itself to a known state.
 */
reset_prog_mode ()						/* 001 */
{								/* 001 */
	_reset_prog_mode ();					/* 001 */

#ifdef KEYPAD
/*								** 001 **
 * We assume the real state of the terminal's keypad is no longer known.
 * Curses assumes the keypad is OFF before a curses progam starts up, so
 * lets force it off here, so we know it is off.  If the intended state was
 * ON for a particular window, stored in the window's _use_keypad flag, a
 * subsequent wgetch() call will reset the terminal's keypad state to ON,
 * because it was forced to OFF here.
 *
 * These actions are necessary in the case of one curses program enabling
 * its keypad and fork(2)'ing and exec(2)'ing a second program.  The second
 * program may have the keypad back into "local" mode (or disabled it) prior
 * to exiting with the result that the first program (after the exec(2)) will
 * no longer see in the keypad keys.  It is recommended that the application
 * call reset_shell_mode() before calling another program, and call
 * reset_prog_mode() upon return to the application.  That is,
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
 */								/* 001 */
	/* if initscr() or newterm() was called, then can reset	** 001 **
	 *	terminal's keypad
	 * else application must use low-level terminfo routines
	 *		to do this.
	 */							/* 001 */
	if (SP) {						/* 001 */
		/* Now, force keypad into a known state (OFF).	** 001 **
		 * __kpmode saves the new state in
		 * SP->kp_state.
		 */						/* 001 */
		__kpmode(0);					/* 001 */
 		fflush(stdout);	/* flush escape sequence out to	** 001 **
				 * the terminal			** 001 **
				 */				/* 001 */
	}							/* 001 */
#endif /* KEYPAD */						/* 001 */
}
