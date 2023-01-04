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
static char rcsid[] = "@(#)$RCSfile: bsd_idsubwins.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:00:34 $";
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
 * _id_subwins:
 *	Re-sync the pointers to _y for all the subwindows.
 *
 */
_id_subwins(orig)
register WINDOW	*orig;
{
	register WINDOW	*win;
	register int	realy;
	register int	y, oy, x;

	realy = orig->_begy + orig->_cury;
	for (win = orig->_nextp; win != orig; win = win->_nextp) {
		/*
		 * If the window ends before our current position,
		 * don't need to do anything.
		 */
		if (win->_begy + win->_maxy <= realy)
			continue;

		oy = orig->_cury;
		for (y = realy - win->_begy; y < win->_maxy; y++, oy++)
			win->_y[y] = &orig->_y[oy][win->_ch_off];
	}
}
