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
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */
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


# include	"cursesext.h"


/*
 * NAME:        wgetwstr
 *
 * FUNCTION:
 *
 *      This routine gets a wchar_t string starting at (_cury,_curx)
 */

/* Include file for wchar_t */
#include        <locale.h>
#include        <wchar.h>
#include        <stdlib.h>

extern	int	wgetnwstr( WINDOW *win, wchar_t *wstr, int n )
{
	char myerase, mykill;
	char rownum[256], colnum[256];
	int doecho = SP->fl_echoit;
	int savecb = SP->fl_rawmode;
	register int cpos = 0;
	register wchar_t wch;
	register wchar_t *wcp = wstr;

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
#ifdef WCHAR
		wch = wgetwch(win);
#else
		wch = wgetch(win);
#endif
		if ((int)wch <= 0 || (int)wch == ERR ||
					wch == KEY_ENTER ||
					wch == '\n' || wch == '\r')
			break;
		if (wch == myerase || wch == KEY_LEFT || wch == KEY_BACKSPACE) {
			if (cpos > 0) {
				wcp--; cpos--;
				if (doecho) {
					wmove(win, rownum[cpos],
							colnum[cpos]);
					wclrtoeol(win);
				}
			}
		} else if (wch == mykill) {
			wcp = wstr;
			cpos = 0;
			if (doecho) {
				wmove(win, rownum[cpos], colnum[cpos]);
				wclrtoeol(win);
			}
		} else {
			*wcp++ = wch;
			cpos++;

			if (doecho) {
				waddwch(win, wch);
			}
		}
	}

	if( n != 0 )
		*wcp = '\0';

	if (doecho)
		echo();
	if (!savecb)
		nocrmode();
	waddch(win, '\n');
	if (win->_flags & _ISPAD);
		wrefresh(win);
	if ((int)wch == ERR)
		return ERR;
	return OK;
}
