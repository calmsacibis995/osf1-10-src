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
static char rcsid[] = "@(#)$RCSfile: bsd_overlay.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:09:30 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1981 Regents of the University of California.
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

#ifndef lint

#endif /* not lint */

# include	"curses.ext"
# include	<ctype.h>

# define	min(a,b)	(a < b ? a : b)
# define	max(a,b)	(a > b ? a : b)

/*
 *	This routine writes win1 on win2 non-destructively.
 *
 */
overlay(win1, win2)
reg WINDOW	*win1, *win2; {

	reg char	*sp, *end;
	reg int		x, y, endy, endx, starty, startx;
	reg int 	y1,y2;

# ifdef DEBUG
	fprintf(outf, "OVERLAY(%0.2o, %0.2o);\n", win1, win2);
# endif
	starty = max(win1->_begy, win2->_begy);
	startx = max(win1->_begx, win2->_begx);
	endy = min(win1->_maxy + win1->_begy, win2->_maxy + win2->_begx);
	endx = min(win1->_maxx + win1->_begx, win2->_maxx + win2->_begx);
# ifdef DEBUG
	fprintf(outf, "OVERLAY:from (%d,%d) to (%d,%d)\n", starty, startx, endy, endx);
# endif
	if (starty >= endy || startx >= endx)
		return;
	y1 = starty - win1->_begy;
	y2 = starty - win2->_begy;
	for (y = starty; y < endy; y++, y1++, y2++) {
		end = &win1->_y[y1][endx - win1->_begx];
		x = startx - win2->_begx;
		for (sp = &win1->_y[y1][startx - win1->_begx]; sp < end; sp++) {
			if (!isspace(*sp))
				mvwaddch(win2, y2, x, *sp);
			x++;
		}
	}
}
