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
static char rcsid[] = "@(#)$RCSfile: box.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:01:05 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "box.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:04:31";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   box
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include       "cursesext.h"

#define DEFVERT '|'
#define DEFHOR  '-'

/*
 * NAME:        box
 *
 * FUNCTION:
 *
 *      This routine draws a box around the given window with "vert"
 *      as the vertical delimiting char, and "hor", as the horizontal one.
 */

box(win, vert, hor)
register WINDOW	*win;
chtype	vert, hor;
{
	register int	i;
	register int	endy, endx;
	register chtype	*fp, *lp;

	if (vert == 0)
		vert = DEFVERT;
	if (hor == 0)
		hor = DEFHOR;
	endx = win->_maxx;
	endy = win->_maxy -  1;
	fp = win->_y[0];
	lp = win->_y[endy];
	for (i = 0; i < endx; i++)
		fp[i] = lp[i] = hor;
	endx--;
	for (i = 0; i <= endy; i++)
		win->_y[i][0] = (win->_y[i][endx] = vert);
	touchwin(win);
}
