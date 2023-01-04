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
static char rcsid[] = "@(#)$RCSfile: clrtoeol.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:08:02 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "clrtoeol.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:08:18";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wclrtoeol
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
 * NAME:        wclrtoeol
 *
 * FUNCTION:
 *
 *      This routine clears up to the end of line.
 */

wclrtoeol(win)
register WINDOW	*win; {

	register chtype	*sp, *end;
	register int		y, x;
	register chtype	*maxx;
	register int		minx;

	y = win->_cury;
	x = win->_curx;
	end = &win->_y[y][win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_y[y][x];
	for (sp = maxx; sp < end; sp++)
		if (*sp != ' ') {
			maxx = sp;
			if (minx == _NOCHANGE)
				minx = sp - win->_y[y];
			*sp = ' ';
		}
	/*
	 * update firstch and lastch for the line
	 */
# ifdef DEBUG
if(outf) fprintf(outf, "CLRTOEOL: \
line %d minx = %d, maxx = %d, firstch = %d, lastch = %d, next firstch %d\n",
	 y, minx, maxx - win->_y[y], win->_firstch[y], win->_lastch[y],
		win->_firstch[y+1]);
# endif
	if (minx != _NOCHANGE) {
		if (win->_firstch[y] > minx || win->_firstch[y] == _NOCHANGE)
			win->_firstch[y] = minx;
		if (win->_lastch[y] < maxx - win->_y[y])
			win->_lastch[y] = maxx - win->_y[y];
	}
}
