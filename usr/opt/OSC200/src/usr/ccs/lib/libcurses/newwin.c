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
static char rcsid[] = "@(#)$RCSfile: newwin.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:07:14 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "newwin.c  1.7  com/lib/curses,3.1,8943 10/20/89 11:57:20"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   newwin
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
 * NAME:
 *
 * FUNCTION:
 *
 *      Allocate space for and set up defaults for a new _window.
 */

WINDOW *
newwin(nlines, ncols, by, bx)
register int	nlines, ncols, by, bx;
{
	register WINDOW	*win;
	register chtype	*sp;
	register int i;
	char *calloc();

	if (by + nlines > LINES)
		nlines = LINES - by;
	if (bx + ncols > COLS)
		ncols = COLS - bx;

	if (nlines == 0)
		nlines = LINES - by;
	if (ncols == 0)
		ncols = COLS - bx;

	if ((win = makenew(nlines, ncols, by, bx)) == NULL)
		return NULL;
	for (i = 0; i < nlines; i++)
#ifdef PHASE2
		if(!(win->_y[i] = (chtype *)calloc(ncols,sizeof(chtype))) ||
		   !(win->_y_atr[i] = (chtype *)calloc(ncols,sizeof(chtype)))){
#else
		if ((win->_y[i] = (chtype *) calloc(ncols, sizeof (chtype)))
								 == NULL) {
#endif
			register int j;

			for (j = 0; j < i; j++){
				cfree((char *)win->_y[j]);
#ifdef PHASE2
				cfree((char *)win->_y_atr[j]);
#endif
			}
			cfree((char *)win->_firstch);
			cfree((char *)win->_lastch);
			cfree((char *)win->_y);
			cfree((char *)win);
#ifdef PHASE2
			cfree((char *)win->_y_atr);
#endif
			return NULL;
		}
		else
			for (sp = win->_y[i]; sp < win->_y[i] + ncols; )
				*sp++ = ' ';
	return win;
}
