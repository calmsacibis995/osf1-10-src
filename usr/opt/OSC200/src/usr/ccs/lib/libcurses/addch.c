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
static char rcsid[] = "@(#)$RCSfile: addch.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 20:56:03 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "addch.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:03:02";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   waddch
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
 * NAME:        waddch
 *
 * FUNCTION:
 *
 *      This routine prints the character in the current position.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Think of it as putc.
 */

waddch(win, c)
register WINDOW	*win;
register chtype		c;
{
	register int		x, y;
	char *uctr;
     	register chtype rawc = c & A_CHARTEXT;

	x = win->_curx;
	y = win->_cury;

# ifdef DEBUG
	if (outf)
		if (c == rawc)
			fprintf(outf, "'%c'", rawc);
		else
			fprintf(outf, "'%c' %o, raw %o", c, c, rawc);
# endif
	if (y >= win->_maxy || x >= win->_maxx || y < 0 || x < 0)
	{
# ifdef DEBUG
if(outf)
{
fprintf(outf,"off edge, (%d,%d) not in (%d,%d)\n",y,x,win->_maxy,win->_maxx);
}
# endif
		return ERR;
	}
	switch( rawc )
	{
	case '\t':
		{
			register int newx;

			for( newx = x + (8 - (x & 07)); x < newx; x++ )
			{
				if( waddch(win, ' ') == ERR )
				{
					return ERR;
				}
			}
			return OK;
		}
	  default:
		if( rawc < ' ' || rawc == 0x7f )
		{
			uctr = unctrl(rawc);
			waddch(win, (chtype)uctr[0]|(c&A_ATTRIBUTES));
			waddch(win, (chtype)uctr[1]|(c&A_ATTRIBUTES));
			return OK;
		}
		if( win->_attrs )
		{
#ifdef	DEBUG
if(outf) fprintf(outf,
	"(attrs %o, %o=>%o)", win->_attrs, c, c | win->_attrs);
#endif	/* DEBUG */
			c |= win->_attrs;;
		}
		if( win->_y[y][x] != c )
		{
			if( win->_firstch[y] == _NOCHANGE )
			{
				win->_firstch[y] = win->_lastch[y] = x;
			}
			else
			{
				if( x < win->_firstch[y] )
				{
					win->_firstch[y] = x;
				}
				else
				{
					if( x > win->_lastch[y] )
					{
						win->_lastch[y] = x;
					}
				}
			}
		}
		win->_y[y][x++] = c;

		if (x >= win->_maxx)
		{
			x = 0;
new_line:
			if (++y > win->_bmarg)
			{
				if (win->_scroll && !(win->_flags&_ISPAD))
				{
#ifdef	DEBUG
	if(outf) {
		fprintf( outf, "Calling  _tscroll(  0%o  )\n", win );
	}
#endif	/* DEBUG */
					_tscroll( win );
					--y;
				}
				else
				{
# ifdef DEBUG
					int i;
					if(outf)
					{
					    fprintf(outf,
					    "ERR because (%d,%d) > (%d,%d)\n",
					    x, y, win->_maxx, win->_maxy);
					    fprintf(outf, "line: '");
					    for (i=0; i<win->_maxy; i++)
						fprintf(outf, "%c",
							win->_y[y-1][i]);
					    fprintf(outf, "'\n");
					}
# endif	/* DEBUG */
					return ERR;
				}
			}
		}
# ifdef DEBUG
if(outf) fprintf(outf,
	"ADDCH: 2: y = %d, x = %d, firstch = %d, lastch = %d\n", y, x,
		win->_firstch[y], win->_lastch[y]);
# endif	/* DEBUG */
		break;
	  case '\n':
		wclrtoeol(win);
		x = 0;
		goto new_line;
	  case '\r':
		x = 0;
		break;
	  case '\b':
		if (--x < 0)
			x = 0;
		break;
	}
	win->_curx = x;
	win->_cury = y;
	return OK;
}
