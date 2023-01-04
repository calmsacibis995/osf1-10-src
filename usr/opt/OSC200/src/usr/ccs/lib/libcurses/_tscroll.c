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
static char rcsid[] = "@(#)$RCSfile: _tscroll.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:54:18 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_tscroll.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:02:28";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _tscroll
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

/*
 * NAME:        _tscroll
 */

_tscroll( win )
register WINDOW	*win;
{

	register chtype	*sp;
	register int		i;
	register chtype	*temp;
	register int	top, bot;

#ifdef DEBUG
	if( win == stdscr )
	{
		if(outf) fprintf( outf, "scroll( stdscr )\n" );
	}
	else
	{
		if( win == curscr)
		{
			if(outf) fprintf( outf, "scroll( curscr )\n" );
		}
		else
		{
			if(outf) fprintf( outf, "scroll( %x )\n", win );
		}
	}
#endif
	if( !win->_scroll )
	{
		return ERR;
	}
	/* scroll the window lines themselves up */
	top = win->_tmarg;
	bot = win->_bmarg;
	temp = win->_y[top];
#ifdef	DEBUG
	if(outf)
	{
		fprintf( outf, "top = %d, bot = %d\n", top, bot );
	}
#endif	/* DEBUG */
	for (i = top; i < bot; i++)
	{
		win->_y[i] = win->_y[i+1];
	}
	/* Put a blank line in the opened up space */
	for (sp = temp; sp - temp < win->_maxx; )
		*sp++ = ' ';
	win->_y[bot] = temp;
#ifdef	DEBUG
	if(outf)
	{
		fprintf(outf,"SCROLL win [0%o], curscr [0%o], top %d, bot %d\n",
				win, curscr, top, bot);
		fprintf( outf, "Doing --> touchwin( 0%o )\n", win );
	}
#endif	/* DEBUG */
	win->_cury--;
	touchwin(win);
	return OK;
}
