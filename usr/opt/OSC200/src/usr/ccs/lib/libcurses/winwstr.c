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
 * NAME:        winnwstr
 *
 *
 */

extern	int	winwchnstr( WINDOW *win, chtype *wchstr, int n )
{
	int	x, y;

	x = win->_curx;
        y = win->_cury;

#ifdef PHASE2
        while( IS_NEXTATR( win->_y_atr[y][x]) )
                x--;
#else
        while( IS_NEXTCHAR( (win->_y[y][x]) ) )
                x--;
#endif

	while( (n == INFINI || n > 0 ) && x < win->_maxx )
	{
#ifdef PHASE2
	        if( !IS_NEXTATR( win->_y_atr[y][x]) ) {
#else
        	if( !IS_NEXTCHAR( win->_y[y][x] )) {
#endif
			*wchstr++ = win->_y[y][x];
			if (n != INFINI) n--;
		}
		x++;
	}
	*wchstr = (chtype) '\0';
	return OK;
}

extern	int	winnwstr( WINDOW *win, wchar_t *wstr, int n )
{
	int	x, y;
	chtype	ch;

	x = win->_curx;
        y = win->_cury;

#ifdef PHASE2
        while( IS_NEXTATR( win->_y_atr[y][x]) )
                x--;
#else
        while( IS_NEXTCHAR( (win->_y[y][x]) ) )
                x--;
#endif

        while( (n == INFINI || n > 0) && x < win->_maxx )
        {
#ifdef PHASE2
                if( !IS_NEXTATR( win->_y_atr[y][x]) )
#else
                if( !IS_NEXTCHAR( win->_y[y][x] ))
#endif
		{
			ch = win->_y[y][x];
			*wstr++ = ch & A_CHARTEXT;
			if (n != INFINI) n--;
		}
		x++;
        }
	*wstr = (wchar_t) '\0';

	return OK;
}
