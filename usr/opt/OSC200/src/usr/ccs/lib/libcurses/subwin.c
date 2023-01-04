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
static char rcsid[] = "@(#)$RCSfile: subwin.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:34:16 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "subwin.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:40:40"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   subwin
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
 * NAME:        subwin
 */

WINDOW *
subwin(orig, num_lines, num_cols, begy, begx)
register WINDOW	*orig;
int	num_lines, num_cols, begy, begx;
{

	register int i;
	register WINDOW	*win;
	register int by, bx, nlines, ncols;
	register int j, k;

	by = begy;
	bx = begx;
	nlines = num_lines;
	ncols = num_cols;

	/*
	 * make sure window fits inside the original one
	 */
# ifdef	DEBUG
	if(outf) fprintf(outf,
		"SUBWIN(%0.2o, %d, %d, %d, %d)\n",
		orig, nlines, ncols, by, bx);
# endif
	if (by < orig->_begy || bx < orig->_begx
	    || by + nlines > orig->_begy + orig->_maxy
	    || bx + ncols  > orig->_begx + orig->_maxx)
		return NULL;
	if (nlines == 0)
		nlines = orig->_maxy - orig->_begy - by;
	if (ncols == 0)
		ncols = orig->_maxx - orig->_begx - bx;
	if ((win = makenew(nlines, ncols, by, bx)) == NULL)
		return NULL;
	j = by - orig->_begy;
	k = bx - orig->_begx;
	for (i = 0; i < nlines; i++)
		win->_y[i] = &orig->_y[j++][k];
	win->_flags = _SUBWIN;
	return win;
}
