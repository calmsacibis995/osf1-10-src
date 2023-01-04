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
static char rcsid[] = "@(#)$RCSfile: bsd_delwin.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 19:54:10 $";
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

/*
 *	This routine deletes a window and releases it back to the system.
 *
 */
delwin(win)
reg WINDOW	*win; {

	reg int		i;
	reg WINDOW	*wp, *np;

	if (win->_orig == NULL) {
		/*
		 * If we are the original window, delete the space for
		 * all the subwindows, and the array of space as well.
		 */
		for (i = 0; i < win->_maxy && win->_y[i]; i++)
			free(win->_y[i]);
		free(win->_firstch);
		free(win->_lastch);
		wp = win->_nextp;
		while (wp != win) {
			np = wp->_nextp;
			delwin(wp);
			wp = np;
		}
	}
	else {
		/*
		 * If we are a subwindow, take ourselves out of the
		 * list.  NOTE: if we are a subwindow, the minimum list
		 * is orig followed by this subwindow, so there are
		 * always at least two windows in the list.
		 */
		for (wp = win->_nextp; wp->_nextp != win; wp = wp->_nextp)
			continue;
		wp->_nextp = win->_nextp;
	}
	free(win->_y);
	free(win);
}
