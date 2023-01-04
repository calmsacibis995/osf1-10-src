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
static char rcsid[] = "@(#)$RCSfile: getstr.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/12 21:31:20 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "getstr.c  1.6  com/lib/curses,3.1,9008 11/14/89 22:28:43";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wgetnstr
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


/*
 * NAME:        wgetnstr
 *
 * FUNCTION:
 *
 *      This routine gets a string starting at (_cury,_curx)
 */

wgetnstr(win,str,n)
WINDOW	*win; 
char	*str;
int	n;
{
	char myerase, mykill;
	char rownum[256], colnum[256];
	int doecho = SP->fl_echoit;
	int savecb = SP->fl_rawmode;
	register int cpos = 0;
	register int ch;
	register char *cp = str;

#ifdef DEBUG
	if (outf) fprintf(outf, "doecho %d, savecb %d\n", doecho, savecb);
#endif

	myerase = erasechar();
	mykill = killchar();
	noecho(); crmode();

	while( n == INFINI || n-- ){
		rownum[cpos] = win->_cury;
		colnum[cpos] = win->_curx;
		if (! (win->_flags&_ISPAD))
			wrefresh(win);
		ch = wgetch(win);
		if ((int) ch <= 0 || (int) ch == ERR ||
					ch == '\n' || ch == '\r')
			break;
		if (ch == myerase || ch == KEY_LEFT || ch == KEY_BACKSPACE) {
			if (cpos > 0) {
				cp--; cpos--;
				if (doecho) {
					wmove(win, rownum[cpos],
							colnum[cpos]);
					wclrtoeol(win);
				}
			}
		} else if (ch == mykill) {
			cp = str;
			cpos = 0;
			if (doecho) {
				wmove(win, rownum[cpos], colnum[cpos]);
				wclrtoeol(win);
			}
		} else {
			*cp++ = ch;
			cpos++;

			if (doecho) {
				waddch(win, ch);
			}
		}
	}

	*cp = '\0';

	if (doecho)
		echo();
	if (!savecb)
		nocrmode();
	waddch(win, '\n');
	if (win->_flags & _ISPAD);
		wrefresh(win);
	if ((int) ch == ERR)
		return ERR;
	return OK;
}
