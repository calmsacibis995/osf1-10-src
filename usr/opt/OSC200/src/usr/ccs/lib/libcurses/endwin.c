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
static char rcsid[] = "@(#)$RCSfile: endwin.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 21:24:25 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "endwin.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:15:56";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   endwin
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"cursesext.h"
#include	<signal.h>

extern	int	_endwin;
extern	int	_c_clean();
extern	int	_fixdelay();
extern	int	_outch();
extern	int	_pos();
extern	int	doupdate();
extern	int	reset_shell_mode();
extern	int	tputs();

/*
 * NAME:        endwin
 *
 * FUNCTION:
 *
 *      Clean things up before exiting.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      endwin is TRUE if we have called endwin - this avoids calling it
 *      twice.
 */

int
endwin()
{
	int saveci = SP->check_input;

	if (_endwin)
		return;

	/* Flush out any output not output due to typeahead */
	SP->check_input = 9999;
	doupdate();
	SP->check_input = saveci;	/* in case of another initscr */

	_fixdelay(SP->fl_nodelay, FALSE);
	if (stdscr->_use_meta)
		tputs(meta_off, 1, _outch);
	_pos(lines-1, 0);
	_c_clean();
	_endwin = TRUE;
	reset_shell_mode();
#ifdef	SIGTSTP
	signal(SIGTSTP, SIG_DFL);
#endif
	fflush(stdout);
}
