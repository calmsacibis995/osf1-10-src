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
 * @(#)$RCSfile: scrn_local.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:38 $
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	scrn_local.h,v $
 * Revision 1.1.1.1  92/05/12  01:45:00  devrcs
 *  *** OSF1_1B29 version ***
 * 
 * Revision 1.1.2.3  1992/04/05  18:20:30  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:45  marquard]
 *
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * @(#)scrn_local.h	1.1 11:18:24 11/8/91 SecureWare
 *
 * Based on OSF version:
 * 	@(#)scrn_local.h	1.2 16:31:21 5/17/91 SecureWare
 */

#ifndef __SCRN_LOCAL_H__
#define __SCRN_LOCAL_H__

/* declarations for screen handling routines */

struct	scrn_ret	getscreen();
void	putscreen();

void help(), helpscrn(), message(), rm_message();

void
   initstates(), downfill(), upfill(),
   leftfill(), rightfill(), delword(), backspace(), tabfill(),
   backtabfill(), delfield(), instoggle(),
   fillinkey(), downmenu(), rightmenu(), upmenu(), leftmenu(),
   wpadstring(), padnumber(),
   putspaces(), movetofield(), clearfield(),
   highlight(), unhighlight(), delchars(), moveright(), findchoice(),
   correctcursor(), scr_rowcol(), cursor(),
   movetoscrollreg(), tabscrollreg(), backtabscrollreg(),
   scrolldownkey(), scrollupkey(), downscrollreg(), upscrollreg(),
   scroll_down(), scroll_up(), drawscrollreg(),
   scr_above_ind(), scr_below_ind(), scr_moveto(),
   settoggle(), unsettoggle(), ExitMemoryError(),
   insfield(), scroll_rshift(), scroll_lshift(),
   off_insert_ind(), on_insert_ind();

int leavefield(), findupitem(), finddownitem(), find_terminal(),
	eighthbit(), reqfillin(), scr_offscreen();

char *fixalpha();

Scrn_struct *buildscrn_struct(), *rebuildscrn_struct();
void freescrn_struct();

/* key definitions - depends on curses */

#ifndef CTRL
#define CTRL(c)	(c & 037)
#endif /* CTRL */

#define BACKSPACE	KEY_BACKSPACE
#define BACKTAB		CTRL('T')
#define DELFIELD	CTRL('F')
#define DELWORD		CTRL('W')
#define DOWN		KEY_DOWN
#define ENTER		KEY_ENTER
#define ESCAPE		27
#define EXECUTE		CTRL('X')
#define HELP		CTRL('Y')
#define KEYS_HELP	CTRL('O')
#define INSFIELD	KEY_IL
#define INSTOGGLE	KEY_IC
#define LEFT		KEY_LEFT
#define NEXTPAGE	KEY_NPAGE
#define PREVPAGE	KEY_PPAGE
#define QUITMENU	CTRL('C')
#define QUITPROG	CTRL('B')
#define REDRAW		CTRL('L')
#define RIGHT		KEY_RIGHT
#define SCROLLDOWN	CTRL('D')
#define SCROLLUP	CTRL('U')
#define SPACE		' '
#define TAB		'\t'
#define UP		KEY_UP

#ifndef DELETE
#define DELETE		0x7f
#endif /* DELETE */

/*
 *	basic justification macros - just stick it wherever
 */

/* left-justify a string at the given line */

#define LEFTS(Y, S)		mvaddstr(Y, 0, S)
#define WLEFTS(W, Y, S)		mvwaddstr(W, Y, 0, S)

/* right-justify a string at the given line */

#define RIGHTS(Y, S)		mvaddstr(Y, (COLS - strlen(S) - 1), S)
#define WRIGHTS(W, Y, S)	{					\
					int I=W->_maxx-strlen(S)-1;	\
					mvwaddstr(W, Y, (I>=0?I:0), S);	\
				}

/* center a string at the given line */

#define CENTERS(Y, S)		mvaddstr (Y, ((COLS - strlen (S)) / 2 - 1), S)
#define WCENTERS(W, Y, S)	{					\
					int I=(W->_maxx-strlen(S))/2-1;	\
					mvwaddstr(W, Y, (I>=0?I:0), S);	\
				}


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


#endif /* __SCRN_LOCAL_H__ */
