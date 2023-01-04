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
static char rcsid[] = "@(#)$RCSfile: m_tstp.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 21:54:38 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "m_tstp.c  1.6  com/lib/curses,3.1,9008 12/4/89 21:01:42";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_tstp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"
# include	<signal.h>

# ifdef SIGTSTP

/*
 * NAME:        m_tstp
 *
 * FUNCTION:
 *
 *      Handle stop and start signals.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mini.c contains versions of curses routines for minicurses.
 *      They work just like their non-mini counterparts but draw on
 *      std_body rather than stdscr.  This cuts down on overhead but
 *      restricts what you are allowed to do - you can't get stuff back
 *      from the screen and you can't use multiple windows or things
 *      like insert/delete line (the logical ones that affect the screen).
 */

void
m_tstp() 
{
	sigset_t mask;

# ifdef DEBUG
	if (outf) fflush(outf);
# endif
	_ll_move(lines-1, 0);
	endwin();
	fflush(stdout);
	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	signal(SIGTSTP, SIG_DFL);
	sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)0);
	kill(0, SIGTSTP);
	signal(SIGTSTP, ((void (*)(int))m_tstp));
	_reset_prog_mode();					/* 001 */
	SP->doclear = 1;
	_ll_refresh(0);
}
# endif
