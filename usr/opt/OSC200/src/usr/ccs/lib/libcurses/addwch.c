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
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */
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


# include	"cursesext.h"

/*
 * NAME:        waddwch
 *
 * FUNCTION:
 *
 *      This routine prints the wchar_t character in the current position.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Think of it as putc.
 */

/* Include file for wchar_t */
#include        <locale.h>
#include        <wchar.h>
#include        <stdlib.h>

extern	int	waddwch( register WINDOW *win, register chtype c )
{
	register int		x, y, tx, lastx;
	char *uctr;
	register chtype rawc = c & A_CHARTEXT;
	int	width, width2, i;
	chtype	*curc;
#ifdef PHASE2
	chtype	*curatr;
	int	wlen, pos;
#endif

	if( IS_NEXTCHAR(c) )
		return OK;
	x = win->_curx;
	y = win->_cury;
# ifdef DEBUG
	if (outf)
		if (c == rawc)
			_sputc( rawc, outf );
		else
			fprintf(outf, "'%wc' 0x%03x, raw 0x%03x", c, c, rawc);
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
if(outf) fprintf(outf, "(attrs %x, %x=>%x)", win->_attrs, c, c | win->_attrs);
#endif	DEBUG
				c |= win->_attrs;;
			}

			if((width = IS_ACSCHAR(rawc)?ACS_WIDTH:wcwidth(rawc)) < 0 )
				return ERR;
			if( width  > win->_maxx - x )
			{
				if( addch_wrap( win ) == ERR )
					return ERR;
				y = win->_cury;
				x = win->_curx;
			}

			curc = win->_y[y];
#ifdef PHASE2
			curatr = win->_y_atr[y];
#endif
			tx = x;
			if( curc[x] == c ) /* Not changed */
			{
				win->_curx += width;
				return OK;
			}

			/* Clear for previous multi-column character */
#ifdef PHASE2
			if( curatr[x] && (wlen = get_byteln( curatr[x] ) ) > 1 )
			{
				tx -= get_pos( curatr[x] );
				for( i = 0; i < wlen; i++ ){
					curc[tx+i] = ' ';
					curatr[tx+i] = 0;
				}
			}
#else
			if( IS_NEXTCHAR( curc[x] ) )
			{
				while( --tx >= 0 && IS_NEXTCHAR( curc[tx] ) )
				{
					curc[tx] = ' ';
				}
				curc[tx] = ' ';
			}
#endif
			for( i = 0; i < width; i++ )
			{
				curc[x] = (i == 0 ? c : NEXTCHAR);
#ifdef PHASE2
				curatr[x] &= ~ATR_TYPE;
				put_byteln( curatr[x], width );
				if( i == 0 ){
					curatr[x] |= IS_ACSCHAR(rawc) ? 
							ATR_ACS : ATR_CODE;
				}else{
					curatr[x] |= ATR_NEXT;
				}
				put_pos( curatr[x], i );
#endif
				x++;
			}
			lastx = x - 1;

			/* Set space for orphaned columns */
#ifdef PHASE2
			if( (pos = get_pos( curatr[x] )) > 0 )
			{
				for( i=0, wlen=get_byteln(curatr[x]) - pos;
					i < wlen; i++ )
				{
					curc[x+i] = ' ';
					curatr[x+i] = 0;
				}
				lastx += i;
			}
#else
			for( i = 0; IS_NEXTCHAR( curc[x+i] ); i++ )
			{
				curc[x+i] = ' ';
			}
			lastx += i;
#endif
			if( win->_firstch[y] == _NOCHANGE || tx < win->_firstch[y] )
				win->_firstch[y] = tx;
			if( win->_firstch[y] == _NOCHANGE || lastx > win->_lastch[y] )
				win->_lastch[y] = lastx;
			if (x >= win->_maxx)
			{
				win->_curx = x;
				addch_wrap( win );
				return OK;
			}
# ifdef DEBUG
		if(outf) fprintf(outf, "ADDCH: 2: y = %d, x = %d, firstch = %d, lastch = %d\n", y, x, win->_firstch[y], win->_lastch[y]);
# endif	DEBUG
			break;
		case '\n':
			wclrtoeol(win);
			addch_wrap( win );
			return OK;
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

int	addch_wrap( WINDOW *win )
{
	int	x, y;

	y = win->_cury;
	x = 0;

	if (++y > win->_bmarg)
	{
		if (win->_scroll && !(win->_flags&_ISPAD))
		{
#ifdef	DEBUG
	if(outf)
	{
#ifdef ULTRIX
		fprintf( outf, "Calling wrefresh( 0%o ) & _tscroll(  0%o  )\n",
				win, win );
#else
		fprintf( outf, "Calling wrefresh( 0%o ) & _tscroll(  0%o  )\n",
				win, win );
#endif
	}
#endif	DEBUG
#ifdef ULTRIX
			wrefresh(win);
#endif
			_tscroll( win );
			--y;
		}
		else
		{
# ifdef DEBUG
			int i;
			if(outf)
			{
				fprintf(outf, "ERR because (%d,%d) > (%d,%d)\n",
						x, y, win->_maxx, win->_maxy);
				fprintf(outf, "line: '");
				for (i=0; i<win->_maxy; i++)
					_sputc( win->_y[y-1][i], outf );
					fprintf(outf, "'\n");
			}
# endif	DEBUG
			return ERR;
		}
	}
	win->_curx = x;
	win->_cury = y;
	return OK;
}
