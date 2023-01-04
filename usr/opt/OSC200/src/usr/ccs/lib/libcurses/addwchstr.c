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
 * NAME:        waddwchnstr
 *
 * FUNCTION:
 *
 *      This routine adds a wchar_t sstring starting at (_cury,_curx)
 *
 */

extern	int	waddwchnstr( WINDOW *win, chtype *chstr, int n )
{
        register int	x, y, tx;
        chtype rawc, ch;
        int     width, width2, i;
        chtype  *curc;
#ifdef PHASE2
        chtype  *curatr;
        int     wlen, pos;
#endif

        x = win->_curx;
        y = win->_cury;

        if (y >= win->_maxy || x >= win->_maxx || y < 0 || x < 0)
        {
                return ERR;
        }

	curc = win->_y[y];
#ifdef PHASE2
	curatr = win->_y_atr[y];
#endif

	tx = x;

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

	while( (n == INFINI || n--) && (ch = *chstr++) ) 
	{
		rawc = ch & A_CHARTEXT;
		if( (width = IS_ACSCHAR(rawc) ? ACS_WIDTH : wcwidth(rawc)) < 0 )
			return ERR;
		if( width  > win->_maxx - x )
		{
			break;
		}

		for( i = 0; i < width; i++ )
		{
			curc[x] = (i == 0 ? ch : NEXTCHAR);
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
		if (x >= win->_maxx)
			break;
	}

	/* Set space for orphaned columns */
#ifdef PHASE2
	if( (pos = get_pos( curatr[x] )) > 0 )
	{
		for( i=0, wlen=get_byteln(curatr[x]) - pos; i < wlen; i++, x++ )
		{
			curc[x] = ' ';
			curatr[x] = 0;
		}
	}
#else
	for( ; IS_NEXTCHAR( curc[x] ); x++ )
	{
		curc[x] = ' ';
	}
#endif
	x--;
	if( win->_firstch[y] == _NOCHANGE || tx < win->_firstch[y] )
		win->_firstch[y] = tx;
	if( win->_firstch[y] == _NOCHANGE || x > win->_lastch[y] )
		win->_lastch[y] = x;
	return OK;
}
