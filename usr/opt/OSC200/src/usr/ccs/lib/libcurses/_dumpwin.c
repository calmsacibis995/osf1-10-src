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
static char rcsid[] = "@(#)$RCSfile: _dumpwin.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:34:06 $";
#endif
/*
 * HISTORY
 */

/***
 ***  "_dumpwin.c	1.7  com/lib/curses,3.1,9008 12/4/89 21:00:51";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _dumpwin
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

/*
 * NAME:        _dumpwin
 *
 * FUNCTION:
 *
 *      Make the current screen look like "win" over the area covered by
 *      win.
 */

#ifdef DEBUG
_dumpwin(win)
register WINDOW *win;
{
	register int x, y;
	register chtype *nsp;
#ifdef PHASE2
	register chtype *nat;
#endif
	

	if (!outf) {
		return;
	}
	if (win == stdscr)
		fprintf(outf, "_dumpwin(stdscr)--------------\n");
	else if (win == curscr)
		fprintf(outf, "_dumpwin(curscr)--------------\n");
	else
		fprintf(outf, "_dumpwin(%o)----------------\n", win);
	for (y=0; y<win->_maxy; y++) {
		if (y > 76)
			break;
		nsp = &win->_y[y][0];
#ifdef PHASE2
		nat = &win->_y_atr[y][0];
#endif
		fprintf(outf, "%d: ", y);
		for (x=0; x<win->_maxx; x++) {
			_sputc(*nsp, outf);
#ifdef PHASE2
			if( *nsp != ' ' )
				fprintf( outf, "[%04x]", *nat );
			nat++;
#endif
			nsp++;
		}
		fprintf(outf, "\n");
	}
	fprintf(outf, "end of _dumpwin----------------------\n");
}
#endif
