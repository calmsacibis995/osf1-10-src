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
 * NAME:        winswch
 *
 * FUNCTION:
 *
 *      This routine performs an insert wchar_t char on the line, leaving
 *      (_cury,_curx) unchanged.
 */

#include	<locale.h>
#include	<wchar.h>
#include	<stdlib.h>

extern	int	winswch( register WINDOW *win, chtype c )
{

	register chtype	*temp1, *lastp;
	char	*uctr;
	register chtype	*curp;
	int	wlen, wlen2, back;
	wchar_t	rawc;
	int	i;
#ifdef PHASE2
	chtype	*curatr, *latr, *tmpatr;
	int	last_mod;
#endif

	rawc = (wchar_t)(c & A_CHARTEXT);

	if( rawc == '\t')
	{
		int newx, x;

		x = win->_curx;

		for( newx = x + (8 - (x & 07)); x < newx; x++ )
		{
			if( waddch(win, ' ') == ERR )
			{
				return ERR;
			}
		}
		return OK;
	}

	if( rawc < ' ' || rawc == 0x7f )
	{
		uctr = unctrl(rawc);
		winswch(win, (chtype)uctr[0]|(c&A_ATTRIBUTES));
		winswch(win, (chtype)uctr[1]|(c&A_ATTRIBUTES));
		return OK;
	}

	curp = &win->_y[win->_cury][win->_curx];

#ifdef PHASE2
	curatr = &win->_y_atr[win->_cury][win->_curx];
	/* adjust cursor before insertion */
	while( IS_NEXTATR( *curatr ) ){
		curatr--; curp--;
		win->_curx--;
	}
#else
	/* adjust cursor before insertion */
	while( IS_NEXTCHAR( *curp ) ){
		curp--;
		win->_curx--;
	}
#endif
	wlen = wcwidth( rawc );
	temp1 = &win->_y[win->_cury][win->_maxx - 1];
	lastp = temp1 - wlen;

#ifdef PHASE2
	tmpatr = &win->_y_atr[win->_cury][win->_maxx - 1];
	latr = tmpatr - wlen;
	if( *latr ){
		int	tpos, tlen;
		chtype	*p;

		tlen = get_byteln( *latr );
		tpos = get_pos( *latr );
		if( tlen - tpos > 1 ){
			p = lastp - tpos;
			while( tlen-- )
				*p++ = ' ';
		}
	}

	while (temp1 > curp){
		*temp1-- = *lastp--;
		*tmpatr-- = *latr--;
	}
#else
	for( back = 0; IS_NEXTCHAR( *(lastp-back) ); back++ )
		;
	wlen2 = wcwidth( *(lastp-back) & A_CHARTEXT );
	if( (back+1) < wlen2 ){
		do
			*(lastp-back) = ' ';
		while( back-- );
	}

	while (temp1 > curp){
		*temp1-- = *lastp--;
	}
#endif


	for( i = 0; i < wlen; i++ )
	{
		temp1[i] = (i == 0 ? c : NEXTCHAR);
#ifdef PHASE2
		curatr[i] &= ~ATR_TYPE;
		put_byteln( curatr[i], wlen );
		if( i == 0 ){
			curatr[i] |= IS_ACSCHAR(rawc) ?
			ATR_ACS : ATR_CODE;
		}else{
			curatr[i] |= ATR_NEXT;
		}
		put_pos( curatr[i], i );
#endif
	}

	win->_lastch[win->_cury] = win->_maxx - 1;
	if (win->_firstch[win->_cury] == _NOCHANGE ||
	    win->_firstch[win->_cury] > win->_curx)
		win->_firstch[win->_cury] = win->_curx;
	if (win->_cury == LINES - 1 && win->_y[LINES-1][COLS-1] != ' ')
		if (win->_scroll && !(win->_flags&_ISPAD)) {
			wrefresh(win);
			scroll(win);
			win->_cury--;
		}
		else
			return ERR;
	return OK;
}
