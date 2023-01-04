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
static char	*sccsid = "@(#)$RCSfile: curs_supp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */



/*
 *	curs_supp.c - curses support routines(low level)
 */

#include "curs_supp.h"

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
			x = MIN (w->_maxx, x);
			x = (x > 0) ? x - 1 : x;
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
 *	system V/OSF-1 fakeouts for Berzerkeley
 */

#if (! defined OSF && (defined(_BSD) || defined(OLD_XENIX)))

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
