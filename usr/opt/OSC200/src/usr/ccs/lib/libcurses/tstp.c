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
static char rcsid[] = "@(#)$RCSfile: tstp.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 22:48:54 $";
#endif
/*
 * HISTORY
 */
/*** "tstp.c  1.6  com/lib/curses,3.1,9008 12/4/89 21:03:16"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _tstp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	<signal.h>

# ifdef SIGTSTP

# include	"cursesext.h"

/*
 * NAME:        tstp
 *
 * FUNCTION:
 *
 *      Handle stop and start signals
 */
void
_tstp() 

{
	sigset_t mask;
	extern int _nocursor;
	extern int _endwin;
	extern int _outch();

	if (_nocursor)
		tputs(cursor_visible, 0, _outch);
	_ll_move(lines-1, 0);
# ifdef DEBUG
	if (outf) fflush(outf);
# endif
	endwin();
	fflush(stdout);
	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	signal(SIGTSTP, SIG_DFL);
	sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)0);
	kill(0, SIGTSTP);
	signal(SIGTSTP, ((void (*)(int)) _tstp));
	fixterm();
#ifdef	WCHAR
	/*
	 * Undoing the effect of endwin().
	 */
	_endwin = FALSE ;
	_fixdelay(FALSE, SP->fl_nodelay);
	if (stdscr->_use_meta)
		tputs(meta_on, 1, _outch);
	/* Make sure keypad on is in proper state */
	if (stdscr->_use_keypad != SP->kp_state)
		_kpmode(stdscr->_use_keypad);
	if (_nocursor)
		tputs(cursor_invisible, 0, _outch);
	SP->doclear = 1;
	_ll_refresh(0);		/* Do the actual redraw */
#else
	SP->doclear = 1;
	wrefresh(curscr);
#endif
}
# endif
