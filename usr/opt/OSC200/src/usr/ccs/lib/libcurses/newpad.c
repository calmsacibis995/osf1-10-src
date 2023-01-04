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
static char rcsid[] = "@(#)$RCSfile: newpad.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:05:39 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "newpad.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:30:37"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   newpad
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
# include	<signal.h>

char	*calloc();
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:        newpad
 *
 * FUNCTION:
 *
 *      Like newwin, but makes a pad instead of a window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      A pad is not associated with part of the screen, so it can be
 *      bigger.
 */

WINDOW *
newpad(nlines, ncols)
register int	nlines;
{
	register WINDOW	*win;
	register chtype	*sp;
	register int i;
	char *calloc();

	if ((win = makenew(nlines, ncols, 0, 0)) == NULL)
		return NULL;
	win->_flags |= _ISPAD;
	for (i = 0; i < nlines; i++)
		if ((win->_y[i] = (chtype *) calloc(ncols, sizeof (chtype)))
								== NULL) {
			register int j;

			for (j = 0; j < i; j++)
				cfree((char *)win->_y[j]);
			cfree((char *)win->_firstch);
			cfree((char *)win->_lastch);
			cfree((char *)win->_y);
			cfree((char *)win);
			return NULL;
		}
		else
			for (sp = win->_y[i]; sp < win->_y[i] + ncols; )
				*sp++ = ' ';
	return win;
}
