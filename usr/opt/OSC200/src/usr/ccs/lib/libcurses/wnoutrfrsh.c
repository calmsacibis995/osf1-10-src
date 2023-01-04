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
static char rcsid[] = "@(#)$RCSfile: wnoutrfrsh.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/12 23:01:03 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "wnoutrfrsh.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:47:38"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wnoutrefresh
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

#include	"cursesext.h"

extern	WINDOW *lwin;

/*
 * NAME:        wnoutrefresh
 *
 * FUNCTION:
 *
 *      Put out window but don't actually update screen.
 */

wnoutrefresh(win)
register WINDOW	*win;
{
	extern	int	_endwin;
	register int wy, y;
	register chtype	*nsp, *lch;
#ifdef PHASE2
	chtype *nat;
#endif

# ifdef DEBUG
	if( win == stdscr )
	{
		if(outf) fprintf(outf, "REFRESH(stdscr %x)", win);
	}
	else
	{
		if( win == curscr )
		{
			if(outf) fprintf(outf, "REFRESH(curscr %x)", win);
		}
		else
		{
			if(outf) fprintf(outf, "REFRESH(%d)", win);
		}
	}
	if(outf) fprintf(outf,
		" (win == curscr) = %d, maxy %d\n",
		win, (win == curscr), win->_maxy);
	if( win != curscr )
	{
		_dumpwin( win );
	}
	if(outf) fprintf(outf, "REFRESH:\n\tfirstch\tlastch\n");
# endif	/* DEBUG */
	/*
	 * initialize loop parameters
	 */

	if( win->_clear || win == curscr || SP->doclear )
	{
# ifdef DEBUG
		if (outf) fprintf(outf,
			"refresh clears, win->_clear %d, curscr %d\n",
			win->_clear, win == curscr);
# endif	/* DEBUG */
		SP->doclear = 1;
		win->_clear = FALSE;
		if( win != curscr )
		{
			touchwin( win );
		}
	}

	if ( (win == curscr) && (_endwin != TRUE) )
	{
#ifdef	DEBUG
	if(outf) fprintf(outf, "Calling _ll_refresh(FALSE)\n" );
#endif	/* DEBUG */
		_ll_refresh(FALSE);
		return OK;
	}
#ifdef	DEBUG
	if(outf) fprintf(outf, "Didn't do _ll_refresh(FALSE)\n" );
#endif	/* DEBUG */

	for( wy = 0; wy < win->_maxy; wy++ )
	{
		if( win->_firstch[wy] != _NOCHANGE )
		{
			y = wy + win->_begy;
			lch = &win->_y[wy][win->_maxx-1];
			nsp = &win->_y[wy][0];
#ifdef PHASE2
			nat = &win->_y_atr[wy][0];
#endif
			_ll_move(y, win->_begx);
			/*
			 * calculate the terminating address for
			 * updating the current row of the screen.
			 * There are two possiblities:
			 * 1: the screen width
			 * 2: the amount of data to be updated.
			 * calculated which is smaller and set up
			 * the while loop
			 */
			if (lch - nsp + 1 <= columns-SP->virt_x)
			{
				SP->virt_x = SP->virt_x + lch - nsp + 1;
			}
			else
			{
				SP->virt_x = columns;
				lch = nsp + (columns - SP->virt_x - 1);
			}
			/* Update this row of the screen */
			{  register chtype *tmp_SP;
#ifdef PHASE2
			   register chtype *tmp_atr;
			tmp_atr = SP->curatr;
#endif
			tmp_SP = SP->curptr;
			while (nsp <= lch)
			{
				*tmp_SP++ = *nsp++;
#ifdef PHASE2
				*tmp_atr++ = *nat++;
#endif
			}
			  SP->curptr = tmp_SP;
#ifdef PHASE2
			  SP->curatr = tmp_atr; 
#endif
			}
			win->_firstch[wy] = _NOCHANGE;
		}
	}
	lwin = win;
	return OK;
}
