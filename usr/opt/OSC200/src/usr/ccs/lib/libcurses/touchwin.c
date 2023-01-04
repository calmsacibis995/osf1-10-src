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
static char rcsid[] = "@(#)$RCSfile: touchwin.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:44:32 $";
#endif
/*
 * HISTORY
 */
/*** "touchwin.c  1.8  com/lib/curses,3.1,9008 12/14/89 17:54:07"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   touchwin, touchline
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
 * NAME:        touchwin
 *
 * FUNCTION:
 *
 *      Make it look like the whole window has been changed.
 */

touchwin(win)
register WINDOW	*win;
{
	register int		y, maxy, maxx;

#ifdef DEBUG
	if (outf) fprintf(outf, "touchwin(%x)\n", win);
#endif
	maxy = win->_maxy;
	maxx = win->_maxx - 1;
	for (y = 0; y < maxy; y++)
	    touchline(win, y, 0, maxx);

}

/*
 * NAME:        touchline
 */

touchline(win, y, sx, ex)
register WINDOW *win;
register int    y, sx, ex;
{
	if (win->_firstch[y] == _NOCHANGE) {
		win->_firstch[y] = sx;
		win->_lastch[y] = ex;
	}
	else {
		if (win->_firstch[y] > sx)
			win->_firstch[y] = sx;
		if (win->_lastch[y] < ex)
			win->_lastch[y] = ex;
	}
}


