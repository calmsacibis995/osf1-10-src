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
static char rcsid[] = "@(#)$RCSfile: erase.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:25:18 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "erase.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:16:23";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   werase
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
 * NAME:        werase
 *
 * FUNCTION:
 *
 *      This routine erases everything on the _window.
 */

werase(win)
register WINDOW	*win; {

	register int		y;
	register chtype	*sp, *end, *start, *maxx;
	register int		minx;

# ifdef DEBUG
	if(outf) fprintf(outf, "WERASE(%0.2o), _maxx %d\n", win, win->_maxx);
# endif
	for (y = 0; y < win->_maxy; y++) {
		minx = _NOCHANGE;
		maxx = NULL;
		start = win->_y[y];
		end = &start[win->_maxx];
		for (sp = start; sp < end; sp++) {
#ifdef DEBUG
			if (y == 23) if(outf) fprintf(outf,
				"sp %x, *sp %c %o\n", sp, *sp, *sp);
#endif
			if (*sp != ' ') {
				maxx = sp;
				if (minx == _NOCHANGE)
					minx = sp - start;
				*sp = ' ';
			}
		}
		if (minx != _NOCHANGE) {
			if (win->_firstch[y] > minx
			     || win->_firstch[y] == _NOCHANGE)
				win->_firstch[y] = minx;
			if (win->_lastch[y] < maxx - win->_y[y])
				win->_lastch[y] = maxx - win->_y[y];
		}
# ifdef DEBUG
if(outf) fprintf(outf,
	"WERASE: minx %d maxx %d _firstch[%d] %d, start %x, end %x\n",
		minx, maxx ? maxx-start : NULL, y, win->_firstch[y],
			start, end);
# endif
	}
	win->_curx = win->_cury = 0;
}
