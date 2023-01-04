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
static char rcsid[] = "@(#)$RCSfile: display.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/08/13 07:40:45 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * COMPONENT_NAME: TCPIP display.c
 * 
 * FUNCTIONS: display, max, readwin, xscroll 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* display.c	1.3  com/sockcmd/talk,3.1,9021 10/8/89 17:25:56 */
/*
 "display.c	5.3 (Berkeley) 6/29/88";
*/

/*
 * The window 'manager', initializes curses and handles the actual
 * displaying of text
 */
#include "talk.h"
#include <stdlib.h>	/* for mbcurmax */

#define	BELL	'\007'	/* == '\a' for ANSI C */

xwin_t	my_win;
xwin_t	his_win;
WINDOW	*line_win;

int	curses_initialized = 0;

/*
 * max HAS to be a function, it is called with
 * a argument of the form --foo at least once.
 */
max(a,b)
	int a, b;
{

	return (a > b ? a : b);
}

/*
 * Display some text on somebody's window, processing some control
 * characters while we are at it.
 */
display(win, text, size)
	register xwin_t *win;
	register char *text;
	int size;
{
	register int i;
	wchar_t    cch;
	int mb_cur_max = MB_CUR_MAX;
	int width ;

	for (i = 0; i < size; i++) {
		if (*text == '\n') {
			xscroll(win, 0);
			text++;
			continue;
		}
		/* erase character */
		if (*text == win->cerase) {
			wmove(win->x_win, win->x_line, max(--win->x_col, 0));
			getyx(win->x_win, win->x_line, win->x_col);
			if (win->x_col > 0) {
				cch   = winwch(win->x_win) ;
				width = wcwidth(cch)	   ;
				if ((width > 1) && (win->x_col >= width - 1)) {
					win->x_col -= width - 1 ;
					wmove(win->x_win, win->x_line,
					      win->x_col);
				}
			}
			waddwch(win->x_win, ' ');
			wmove(win->x_win, win->x_line, win->x_col);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		/*
		 * On word erase search backwards until we find
		 * the beginning of a word or the beginning of
		 * the line.
		 */
		if (*text == win->werase) {
			int endcol, xcol, i, c;

			endcol = win->x_col;
			xcol = endcol - 1;
			while (xcol >= 0) {
				c = readwin(win->x_win, win->x_line, xcol);
				if (c != ' ')
					break;
				xcol--;
			}
			while (xcol >= 0) {
				c = readwin(win->x_win, win->x_line, xcol);
				if (c == ' ')
					break;
				xcol--;
			}
			wmove(win->x_win, win->x_line, xcol + 1);
			for (i = xcol + 1; i < endcol; i++)
				waddwch(win->x_win, ' ');
			wmove(win->x_win, win->x_line, xcol + 1);
			getyx(win->x_win, win->x_line, win->x_col);
			text++  ;
			continue;
		}
		/* line kill */
		if (*text == win->kill) {
			wmove(win->x_win, win->x_line, 0);
			wclrtoeol(win->x_win);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		if (*text == '\f') {
			if (win == &my_win)
				wrefresh(curscr);
			text++;
			continue;
		}
		/* EOF character */
		if (*text == win->eofchar) {
			quit();
		}
		if (*text == BELL) {
			beep();
			text++;
			continue;
		}
		if (win->x_col == COLS-1) {
			/* check for wraparound */
			xscroll(win, 0);
		}
		/*
		 * Handle the multibyte case
		 * We print '?' for nonprintable widechars.
		 */
		if (mb_cur_max > 1 && mblen(text, mb_cur_max) > 1) {
			wchar_t wc;
			int    len;

			len = mbtowc(&wc, text, mb_cur_max);

			if (iswprint(wc) || iswspace(wc)) {
			   /* its printable, put out the bytes */
			   if (win->x_col + wcwidth(wc) >= COLS-1)
				xscroll(win, 0);
			    waddwch(win->x_win, wc) ;
			    getyx(win->x_win, win->x_line, win->x_col);
			    text += len	    ;
			    i    += len - 1 ;
			    continue;
			}
			/*
			 * otherwise, punt and print a question mark.
			 */
			text += len	;
			i    += len - 1 ;
			waddwch(win->x_win, '?');
			getyx(win->x_win, win->x_line, win->x_col);
			continue;
			
		} else if (!isprint(*text & 0xff) && !isspace(*text & 0xff)) {
			waddwch(win->x_win, '^');
			getyx(win->x_win, win->x_line, win->x_col);
			if (win->x_col == COLS-1) /* check for wraparound */
				xscroll(win, 0);
			cch = (*text & 63) + 64;
			waddwch(win->x_win, cch);
		} else
			waddwch(win->x_win, *text & 0xff);
		getyx(win->x_win, win->x_line, win->x_col);
		text++;
	}
	wrefresh(win->x_win);
}

/*
 * Read the character at the indicated position in win
 */
readwin(win, line, col)
	WINDOW *win;
{
	int oldline, oldcol;
	register int c;

	getyx(win, oldline, oldcol);
	wmove(win, line, col);
	c = winwch(win);
	wmove(win, oldline, oldcol);
	return (c);
}

/*
 * Scroll a window, blanking out the line following the current line
 * so that the current position is obvious
 */
xscroll(win, flag)
	register xwin_t *win;
	int flag;
{

	if (flag == -1) {
		wmove(win->x_win, 0, 0);
		win->x_line = 0;
		win->x_col = 0;
		return;
	}
	win->x_line = (win->x_line + 1) % win->x_nlines;
	win->x_col = 0;
	wmove(win->x_win, win->x_line, win->x_col);
	wclrtoeol(win->x_win);
	wmove(win->x_win, (win->x_line + 1) % win->x_nlines, win->x_col);
	wclrtoeol(win->x_win);
	wmove(win->x_win, win->x_line, win->x_col);
}
