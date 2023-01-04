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
static char rcsid[] = "@(#)$RCSfile: toucholap.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:43:41 $";
#endif
/*
 * HISTORY
 */
/*** "toucholap.c  1.4  com/lib/curses,3.1,9008 11/12/89 21:18:56"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   touchoverlap
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

# include       "cursesext.h"

# define	min(a,b)	(a < b ? a : b)
# define	max(a,b)	(a > b ? a : b)

/*
 * NAME:        touchoverlap
 *
 * FUNCTION:
 *
 *      Touch, on win2, the part that overlaps with win1.
 */

touchoverlap(win1, win2)
register WINDOW      *win1, *win2; {

	register int         y, endy, endx, starty, startx;

# ifdef DEBUG
	fprintf(outf, "TOUCHOVERLAP(%0.2o, %0.2o);\n", win1, win2);
# endif
	starty = max(win1->_begy, win2->_begy);
	startx = max(win1->_begx, win2->_begx);
	endy = min(win1->_maxy + win1->_begy, win2->_maxy + win2->_begx);
	endx = min(win1->_maxx + win1->_begx, win2->_maxx + win2->_begx);
# ifdef DEBUG
	fprintf(outf, "TOUCHOVERLAP:from (%d,%d) to (%d,%d)\n",
		starty, startx, endy, endx);
	fprintf(outf, "TOUCHOVERLAP:win1 (%d,%d) to (%d,%d)\n",
		win1->_begy, win1->_begx, win1->_begy + win1->_maxy,
		win1->_begx + win1->_maxx);
	fprintf(outf, "TOUCHOVERLAP:win2 (%d,%d) to (%d,%d)\n",
		win2->_begy, win2->_begx, win2->_begy + win2->_maxy,
		win2->_begx + win2->_maxx);
# endif
	if (starty >= endy || startx >= endx)
		return;
	starty -= win2->_begy;
	startx -= win2->_begx;
	endy -= win2->_begy;
	endx -= win2->_begx;
	endx--;
	for (y = starty; y < endy; y++)
		touchline(win2, y, startx, endx);
}
