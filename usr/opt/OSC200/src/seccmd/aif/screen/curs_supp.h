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
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: curs_supp.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:35 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE
#ifndef __CURS_SUPP__
#define __CURS_SUPP__

/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/*
 * curs_supp.h - curses support macros
 */

#ifdef _BSD
#define BSD_WAS
#undef _BSD
#endif
#include        <curses.h>
#ifdef _BSD_WAS
#define _BSD
#undef BSD_WAS
#endif

#include "kitch_sink.h"

/*
 *	basic justification macros - just stick it wherever
 */

/* left-justify a string at the given line */

#define LEFTS(Y, S)		mvaddstr (Y, 0, S)
#define WLEFTS(W, Y, S)		mvwaddstr (W, Y, 0, S)

/* right-justify a string at the given line */

#define RIGHTS(Y, S)		mvaddstr (Y, (COLS - strlen (S) - 1), S)
#define WRIGHTS(W, Y, S)	{ int I=W->_maxx-strlen(S)-1; mvwaddstr (W, Y, (I>=0?I:0), S); }

/* center a string at the given line */

#define CENTERS(Y, S)		mvaddstr (Y, ((COLS - strlen (S)) / 2 - 1), S)
#define WCENTERS(W, Y, S)	{ int I=(W->_maxx-strlen(S))/2-1; mvwaddstr (W, Y, (I>=0?I:0), S); }


/*
 *	public interfaces to CursesStr () - clear the line,
 *		highlight if specified, and justify the string
 *		within the specified window
 */

void CursesStr();

/* where to justify the string */

#define _LJ	1	/* left justify */
#define _CJ	2	/* center justify */
#define _RJ	3	/* right justify */

/* whether to highlight the string */

#define _HON	1	/* highlight on */
#define _HOFF	0	/* highlight off */

/* left-justify the string */

#define WLEFTH(W, Y, S)		CursesStr(W, Y, S, _LJ, _HON)
#define LEFTH(Y, S)		CursesStr(stdscr, Y, S, _LJ, _HON)
#define WLEFTP(W, Y, S)		CursesStr(W, Y, S, _LJ, _HOFF)
#define LEFTP(Y, S)		CursesStr(stdscr, Y, S, _LJ, _HOFF)

/* center the string */

#define WCENTERH(W, Y, S)	CursesStr(W, Y, S, _CJ, _HON)
#define CENTERH(Y, S)		CursesStr(stdscr, Y, S, _CJ, _HON)
#define WCENTERP(W, Y, S)	CursesStr(W, Y, S, _CJ, _HOFF)
#define CENTERP(Y, S)		CursesStr(stdscr, Y, S, _CJ, _HOFF)

/* right-justify the string */

#define WRIGHTH(W, Y, S)	CursesStr(W, Y, S, _RJ, _HON)
#define RIGHTH(Y, S)		CursesStr(stdscr, Y, S, _RJ, _HON)
#define WRIGHTP(W, Y, S)	CursesStr(W, Y, S, _RJ, _HOFF)
#define RIGHTP(Y, S)		CursesStr(stdscr, Y, S, _RJ, _HOFF)


/*
 *	turn underline on/off if terminal supports it
*/

#define UNDERLINE(W, S)					\
{							\
	if (has_underline)				\
		if (S == ON)				\
			wattron (W, A_UNDERLINE);	\
		else					\
			wattroff (W, A_UNDERLINE);	\
}

/* put the specified character in the window at the current position */

#define WADDCHAR(W, C)	waddch (W, (int) C)

/* Where insert indicator appears in a window */

#define INSROW		(LINES - 1)		/* On bottom line */
#define INSCOL		(COLS - 15)		/* 15th character from left */
#define INSERTIND	"INS"			/* insert indicator */

#define ABOVEIND	"^"			/* more above */
#define BELOWIND	"v"			/* more below */

/* field boundaries for screens without UNDERLINE capability */

#define LEFT_BOUNDARY   '<'
#define RIGHT_BOUNDARY  '>'
#define SINGLE_BOUNDARY '|' /* for scrolling regions with s_spaces == 1 */


#endif /* __CURS_SUPP__ */
#endif /* SEC_BASE */
