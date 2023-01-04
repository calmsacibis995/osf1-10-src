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
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: curs_supp.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:22 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	curs_supp.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:03:30  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:58:28  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:22  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:49  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 * 
 * Based on OSF version:
 *	@(#)curs_supp.c	2.5 16:30:52 5/17/91 SecureWare
 */

/* #ident "@(#)curs_supp.c	1.1 11:16:05 11/8/91 SecureWare" */

/*
 *	curs_supp.c - curses support routines(low level)
 */

#include <sys/secdefines.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/*
 *	fancy subwindows stuff
 *
 *		CursesStr() - General purpose string placement/highlight
 *			routine
 */

/* clear the given line, place the string (highlight if specified) */

void
CursesStr(w, y, s, j, h)
WINDOW *w;	/* take a wild guess */
int y;		/* y-coord (line) */
char *s;	/* string */
int h;		/* highlighting - _HON or _HOFF */
int j;		/* justification: _LJ, _CJ or _RJ */
{
	register int x;

	switch (j) {
		case _LJ :
			x = 0;
			break;
		case _CJ :
			x = (w->_maxx - strlen(s)) / 2 - 1;
			break;
		case _RJ :
			x = COLS - strlen(s);
			if (w->_maxx < x)
				x = w->_maxx;
			if (x > 0)
				x--;
			break;
	}

	wmove (w, y, 0);
	wclrtoeol (w);
	wmove (w, y, x);
	if (h) {
		wattron (w, A_STANDOUT);
	}
	waddstr (w, s);
	if (h) {
		wattroff (w, A_STANDOUT);
	}
}


/*
 * Veneer routines for BSD or old Xenix (Pre-UNIX System V) releases
 */

#if (! defined OSF && (defined(_BSD)))

wattron(w,a)
WINDOW *w;
int a;
{
	wstandout(w);
}

attron(a)
int a;
{
	standout();
}

wattroff(w,a)	
WINDOW *w;
int a;
{
	wstandend(w);
}

attroff(a)	
int a;
{
	standend();
}

beep()
{
	putc('', stderr);
}

wnoutrefresh(w)
WINDOW *w;
{
	if(w != stdscr)
		touchwin(w);
	wrefresh(w);
}

doupdate()
{
}

putp(s)
char *s;
{
}

keypad(w,f)
WINDOW *w;
int f;
{
}

meta(w,f)
WINDOW *w;
int f;
{
	return 0;
}

flushinp()
{
}

putenv(s)
char *s;
{
}

#endif /* _BSD || OLD_XENIX */
