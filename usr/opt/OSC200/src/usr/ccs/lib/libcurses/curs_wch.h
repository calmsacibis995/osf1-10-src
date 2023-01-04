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
/*
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
 */
/*
 * HISTORY
 */
/* @(#)$RCSfile: curs_wch.h,v $ $Revision: 1.1 $ (DEC I18N) $Date: 1992/10/29 14:17:46 $ */

/*  MNLS functions */
#define	INFINI	-1

#define addwch(ch)			waddwch(stdscr, ch)
#define mvaddwch(y,x,ch)		mvwaddwch(stdscr,y,x,ch)
#define mvwaddwch(win,y,x,ch)		(wmove(win,y,x)==ERR?ERR:waddwch(win,ch))

#define	addwstr(str)			waddnwstr(stdscr,str,INFINI)
#define addnwstr(str,n)			waddnwstr(stdscr,str,n)
#define waddwstr(win,str)		waddnwstr(win,str,INFINI)
#define mvaddwstr(y,x,str)      	mvwaddwstr(stdscr,y,x,str)
#define mvaddnwstr(y,x,str,n)      	mvwaddnwstr(stdscr,y,x,str,n)
#define mvwaddwstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:waddnwstr(win,str,INFINI))
#define mvwaddnwstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:waddnwstr(win,str,n))

#define	addwchstr(str)			waddwchnstr(stdscr,str,INFINI)
#define addwchnstr(str,n)		waddwchnstr(stdscr,str,n)
#define waddwchstr(win,str)		waddwchnstr(win,str,INFINI)
#define mvaddwchstr(y,x,str)      	mvwaddwchstr(stdscr,y,x,str)
#define mvaddwchnstr(y,x,str,n)		mvwaddwchnstr(stdscr,y,x,str,n)
#define mvwaddwchstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:waddwchnstr(win,str,INFINI))
#define mvwaddwchnstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:waddwchnstr(win,str,n))

#define getwch()			wgetwch(stdscr)
#define mvgetwch(y,x)			mvwgetwch(stdscr,y,x)
#define mvwgetwch(win,y,x)		(wmove(win,y,x)==ERR?ERR:wgetwch(win))

#define getwstr(str)			wgetnwstr(stdscr, str,INFINI)
#define getnwstr(str,n)			wgetnwstr(stdscr, str,n)
#define wgetwstr(win,str)		wgetnwstr(win, str,INFINI)
#define mvgetwstr(y,x,str)		mvwgetwstr(stdscr,y,x,str)
#define mvgetnwstr(y,x,str,n)		mvwgetnwstr(stdscr,y,x,str,n)
#define mvwgetwstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:wgetnwstr(win,str,INFINI))
#define mvwgetnwstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:wgetnwstr(win,str,n))

#define inswch(c)			winswch(stdscr,c)
#define mvinswch(y,x,c)			mvwinswch(stdscr,y,x,c)
#define mvwinswch(win,y,x,c)		(wmove(win,y,x)==ERR?ERR:winswch(win,c))

#define inswstr(str)			winsnwstr(stdscr, str,INFINI)
#define insnwstr(str,n)			winsnwstr(stdscr, str,n)
#define winswstr(win,str)		winsnwstr(win, str,INFINI)
#define mvinswstr(y,x,str)		mvwinswstr(stdscr,y,x,str)
#define mvinsnwstr(y,x,str,n)		mvwinsnwstr(stdscr,y,x,str,n)
#define mvwinswstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:winsnwstr(win,str,INIFINI))
#define mvwinsnwstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:winsnwstr(win,str,n))

#define inwch()				winwch(stdscr)
#define mvinwch(y,x)			mvwinwch(stdscr,y,x)
#define mvwinwch(win,y,x)		(wmove(win,y,x)==ERR?ERR:winwch(win))

#define inwchstr(str)			winwchnstr(stdscr, str,INFINI)
#define inwchnstr(str,n)		winwchnstr(stdscr, str,n)
#define winwchstr(win,str)		winwchnstr(win, str,INFINI)
#define mvinwchstr(y,x,str)		mvwinwchstr(stdscr,y,x,str)
#define mvinwchnstr(y,x,str,n)		mvwinwchnstr(stdscr,y,x,str,n)
#define mvwinwchstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:winwchnstr(win,str,INIFINI))
#define mvwinwchnstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:winwchnstr(win,str,n))

#define inwstr(str)			winnwstr(stdscr, str,INFINI)
#define innwstr(str,n)			winnwstr(stdscr, str,n)
#define winwstr(win,str)		winnwstr(win, str,INFINI)
#define mvinwstr(y,x,str)		mvwinwstr(stdscr,y,x,str)
#define mvinnwstr(y,x,str,n)		mvwinnwstr(stdscr,y,x,str,n)
#define mvwinwstr(win,y,x,str)\
			(wmove(win,y,x)==ERR?ERR:winnwstr(win,str,INIFINI))
#define mvwinnwstr(win,y,x,str,n)\
			(wmove(win,y,x)==ERR?ERR:winnwstr(win,str,n))

#define delwch()			wdelwch(stdscr)
#define mvdelwch(y,x)			mvwdelwch(stdscr,y,x)
#define mvwdelwch(win,y,x)		(wmove(win,y,x)==ERR?ERR:wdelwch(win))

/* Support ACS Characters for waddwch */
#define IS_ACSCHAR(c)   (((c) & ACS_MASK) == ACS_CODE)
#define ACS_WIDTH       1       /* Display width of ACS char    */
#define ACS_MBLEN       2       /* Byte length of ACS char      */

/*
 * Define line graphic character in the alternate character set
 *  - DEC Special Graphic Character Set (in G2)
 */
#define	ACS_CODE	0x8e00		/* Single shift 2		*/
#define	ACS_MASK	0xff00		/* Mask for checking ACS	*/
#define SO_CODE		14		/* Locking Shift, G0 -> G1	*/
#define S1_CODE		15		/* Locking Shift, G1 -> G0	*/

#ifndef	NO_ACS
#define	ACS_ULCORNER	('l'|ACS_CODE)	/* Upper left-hand corner	*/
#define	ACS_LLCORNER	('m'|ACS_CODE)	/* Lower left-hand corner	*/
#define	ACS_URCORNER	('k'|ACS_CODE)	/* Upper right-hand corner	*/
#define	ACS_LRCORNER	('j'|ACS_CODE)	/* Lower right-hand corner	*/
#define	ACS_RTEE	('u'|ACS_CODE)	/* Right tee 			*/
#define	ACS_LTEE	('t'|ACS_CODE)	/* Left tee			*/
#define	ACS_BTEE	('v'|ACS_CODE)	/* Bottom tee			*/
#define	ACS_TTEE	('w'|ACS_CODE)	/* Top tee			*/
#define	ACS_HLINE	('q'|ACS_CODE)	/* Horizontal line		*/
#define	ACS_VLINE	('x'|ACS_CODE)	/* Vertical line		*/
#define	ACS_PLUS	('n'|ACS_CODE)	/* Plus				*/
#define	ACS_S1		('o'|ACS_CODE)	/* Scan line 1			*/
#define	ACS_S9		('s'|ACS_CODE)	/* Scan line 9			*/
#define	ACS_DIAMOND	('`'|ACS_CODE)	/* Diamond			*/
#define	ACS_CKBOARD	('a'|ACS_CODE)	/* Checker board		*/
#define	ACS_DEGREE	('f'|ACS_CODE)	/* Degree symbol		*/
#define	ACS_PLMINUS	('g'|ACS_CODE)	/* Plus/minus			*/
#define	ACS_BULLET	('~'|ACS_CODE)	/* Bullet			*/
#else	/* NO_ACS */
#define	ACS_ULCORNER	'+'	/* Upper left-hand corner	*/
#define	ACS_LLCORNER	'+'	/* Lower left-hand corner	*/
#define	ACS_URCORNER	'+'	/* Upper right-hand corner	*/
#define	ACS_LRCORNER	'+'	/* Lower right-hand corner	*/
#define	ACS_RTEE	'+'	/* Right tee 			*/
#define	ACS_LTEE	'+'	/* Left tee			*/
#define	ACS_BTEE	'+'	/* Bottom tee			*/
#define	ACS_TTEE	'+'	/* Top tee			*/
#define	ACS_HLINE	'-'	/* Horizontal line		*/
#define	ACS_VLINE	'|'	/* Vertical line		*/
#define	ACS_PLUS	'+'	/* Plus				*/
#define	ACS_S1		'-'	/* Scan line 1			*/
#define	ACS_S9		'_'	/* Scan line 9			*/
#define	ACS_DIAMOND	' '	/* Diamond			*/
#define	ACS_CKBOARD	' '	/* Checker board		*/
#define	ACS_DEGREE	'o'	/* Degree symbol		*/
#define	ACS_PLMINUS	' '	/* Plus/minus			*/
#define	ACS_BULLET	'o'	/* Bullet			*/
#endif	/* NO_ACS */
