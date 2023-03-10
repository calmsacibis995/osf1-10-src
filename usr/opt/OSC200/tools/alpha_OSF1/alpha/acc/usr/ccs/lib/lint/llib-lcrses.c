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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: llib-lcrses.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1993/08/04 23:14:24 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27; 10
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _NO_PROTO

/*NOTUSED*/
/*NOTDEFINED*/

# include	<curses.h>

static char	*sp;
static WINDOW	*wp;
static struct screen *scp;

	box(win, vert, hor) WINDOW *win; chtype vert, hor; {}
	delwin(win) WINDOW *win; {}
	endwin() {}
	gettmode() {}
char   *getcap(name) char *name; { return name; }
WINDOW *initscr() {}
char   *longname() { return sp; }
	mvcur(ly, lx, y, x) int ly, lx, y, x; {}
	/* VARARGS3 */
int	mvprintw(y, x, fmt, args) int y, x; char *fmt; { return 0; }
	/* VARARGS3 */
int	mvscanw(y, x, fmt, args) int y, x; char *fmt; { return 0; }
int	mvwin(win, by, bx) WINDOW *win; { return 0; }
	/* VARARGS4 */
int	mvwprintw(win, y, x, fmt, args) WINDOW *win; int y, x; char *fmt; { return 0; }
	/* VARARGS4 */
int	mvwscanw(win, y, x, fmt, args) WINDOW *win; int y, x; char *fmt; { return 0; }
WINDOW *newwin(num_lines, num_cols, begy, begx) int num_lines, num_cols, begy, begx; { return wp; }
	overlay(win1, win2) WINDOW *win1, *win2; {}
	overwrite(win1, win2) WINDOW *win1, *win2; {}
int	plod(cnt) int cnt; { return 0; }
	plodput(c) char c; {}
	/* VARARGS1 */
int	printw(fmt, args) char *fmt; { return 0; }
int	scanw(fmt, args) char *fmt; { return 0; }
int	scroll(win) WINDOW *win; { return 0; }
int	setterm(type) char *type; { return 0; }
WINDOW *subwin(orig, num_lines, num_cols, begy, begx) WINDOW *orig; int num_lines, num_cols, begy, begx; { return wp; }
	touchwin(win) WINDOW *win; {}
	tstp() {}
int	waddch(win, c) WINDOW *win; int c; { return 0; }
int	waddwch( win, c ) WINDOW *win; chtype c; { return 0; }
int	waddstr(win, str) WINDOW *win; char *str; { return 0; }
int	waddnwstr( win, str, n ) WINDOW *win; wchar_t *str, n; { return 0; }
int     waddwchnstr( win, chstr, n ) WINDOW *win; chtype *chstr; int n; { return 0; }
int	wclear(win) WINDOW *win; { return 0; }
	wclrtobot(win) WINDOW *win; {}
	wclrtoeol(win) WINDOW *win; {}
	wdeleteln(win) WINDOW *win; {}
	werase(win) WINDOW *win; {}
int	wgetch(win) WINDOW *win; { return 0; }
int	wgetwch( win ) WINDOW *win; { return 0; }
int	wgetnstr( win, str, n ) WINDOW *win; char *str; int n; { return 0; }
int	wgetnwstr( win, str, n ) WINDOW *win; wchar_t *str; int n; { return 0; }
	winsertln(win) WINDOW *win; {}
int	wmove(win, y, x) WINDOW *win; int y, x; { return 0; }
	/* VARARGS2 */
int	wprintw(win, fmt, args) WINDOW *win; char *fmt; { return 0; }
int	wrefresh(win) WINDOW *win; { return 0; }
int	wscanw(win, fmt, args) WINDOW *win; char *fmt; { return 0; }
	wstandend(win) WINDOW *win; {}
	wstandout(win) WINDOW *win; {}
int	baudrate() {return 0;}
	beep() {}
	cbreak() {}
	crmode() {}
	doupdate() {}
	echo() {}
char	erasechar() {return 'a';}
	fixterm() {}
	flash() {}
	flushinp() {}
int	has_ic() {return 0;}
int	has_il() {return 0;}
	idlok(win,bf) WINDOW *win; int bf; {}
	intrflush(win,bf) WINDOW *win; int bf; {}
	keypad(win,bf) WINDOW *win; int bf; {}
char	killchar() {return 'a';}
	leaveok(win,bf) WINDOW *win; int bf; {}
	m_addch(c) chtype c; {}
	m_addstr(str) chtype *str; {}
	m_clear() {}
	m_erase() {}
	m_initscr() {}
	m_move(row, col) int row, col; {}
WINDOW*	m_newterm(type, fd) char *type; FILE *fd; {return wp;}
	m_refresh() {}
	m_tstp() {}
	meta(win,bf) WINDOW *win; int bf; {}
WINDOW*	newpad(nlines, ncols) int nlines, ncols; {return wp;}
struct screen *
	newterm(type, fd) char *type; FILE *fd; {return scp;}
	nl() {}
	nocbreak() {}
	nocrmode() {}
	nodelay(win,bf) WINDOW *win; int bf; {}
	noecho() {}
	nonl() {}
	noraw() {}
	pnoutrefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
		WINDOW *pad;
		int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
		{}
	prefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
		WINDOW *pad;
		int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
		{}
	putp(str) char * str; {}
	raw() {}
	resetterm() {}
	resetty() {}
	saveterm() {}
	savetty() {}
	scrollok(win,bf) WINDOW *win; int bf; {}
SCREEN * set_term(new) char *new; {}
int	setupterm(term, filenum, errret)
		char *term; int filenum; int *errret; 
		{return 0;}
int	tgetent(bp, name) char *bp; char *name; {return 0;}
int	tgetflag(id) char *id; {return 0;}
int	tgetnum(id) char *id; {return 0;}
char *	tgetstr(id, area) char *id; char **area; {return sp;}
char *	tgoto(cap, col, row) char *cap; int col, row; {return sp;}
	/* VARARGS2 */
char *	tparm(instring, parms)
		char *instring;
		int parms;
		{return sp;}
	tputs(cp, affcnt, outc) char *cp; int affcnt; int (*outc)(); {}
	traceoff() {}
	traceon() {}
	vidattr(newmode) int newmode; {}
	wattroff(win, attrs) WINDOW *win; int attrs; {}
	wattron(win, attrs) WINDOW *win; int attrs; {}
	wattrset(win, attrs) WINDOW *win; int attrs; {}
	wdelch(win) WINDOW *win; {}
	winsch(win, c) WINDOW *win; chtype *c; {}
int	winswch(win, c) WINDOW *win; chtype *c; { return 0; }
int     winsnwstr( win, wstr, n ) WINDOW *win; wchar_t *wstr; int n; { return 0; }
chtype  winwch( win ) WINDOW *win; { return 0; }
int     winwchnstr( win, wchstr, n ) WINDOW *win; chtype *wchstr; int n; { return 0; }
int     winnwstr( win, wstr, n ) WINDOW *win; wchar_t *wstr; int n; { return 0; }
	wnoutrefresh(win) WINDOW *win; {}

int	LINES, COLS;

WINDOW	*stdscr, *curscr;

#ifdef lint
/*
 * Various tricks to shut up lint about things that are prefectly fine.
 */
struct screen {
	int _nobody_;
};
char *Def_term, ttytype[1];
char *_unctrl[1];
int	LINES, COLS;
WINDOW *stdscr, *curscr;
static
_dummy_init()
{
	Def_term[0] = ttytype[0] = 0;
	_unctrl[0] = "abc";
	_dummy_init();
	LINES = COLS = 1;
	stdscr = curscr = NULL;
}
#endif

