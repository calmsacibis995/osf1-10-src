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
static char rcsid[] = "@(#)$RCSfile: bsd_refresh.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 20:12:16 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

/*
 * make the current screen look like "win" over the area coverd by
 * win.
 */

# include	"curses.ext"

# ifdef DEBUG
# define	STATIC
# else
# define	STATIC	static
# endif

STATIC _SHORT	ly, lx;

STATIC bool	curwin;

WINDOW	*_win = NULL;

wrefresh(win)
reg WINDOW	*win;
{
	reg _SHORT	wy;
	reg int		retval;
	reg WINDOW	*orig;

	/*
	 * make sure were in visual state
	 */
	if (_endwin) {
		_puts(VS);
		_puts(TI);
		_endwin = FALSE;
	}

	/*
	 * initialize loop parameters
	 */

	ly = curscr->_cury;
	lx = curscr->_curx;
	wy = 0;
	_win = win;
	curwin = (win == curscr);

	if (win->_clear || curscr->_clear || curwin) {
		if ((win->_flags & _FULLWIN) || curscr->_clear) {
			_puts(CL);
			ly = 0;
			lx = 0;
			if (!curwin) {
				curscr->_clear = FALSE;
				curscr->_cury = 0;
				curscr->_curx = 0;
				werase(curscr);
			}
			touchwin(win);
		}
		win->_clear = FALSE;
	}
	if (!CA) {
		if (win->_curx != 0)
			_putchar('\n');
		if (!curwin)
			werase(curscr);
	}
# ifdef DEBUG
	fprintf(outf, "REFRESH(%0.2o): curwin = %d\n", win, curwin);
	fprintf(outf, "REFRESH:\n\tfirstch\tlastch\n");
# endif
	for (wy = 0; wy < win->_maxy; wy++) {
# ifdef DEBUG
		fprintf(outf, "%d\t%d\t%d\n", wy, win->_firstch[wy],
			win->_lastch[wy]);
# endif
		if (win->_firstch[wy] != _NOCHANGE)
			if (makech(win, wy) == ERR)
				return ERR;
			else {
				if (win->_firstch[wy] >= win->_ch_off)
					win->_firstch[wy] = win->_maxx +
							    win->_ch_off;
				if (win->_lastch[wy] < win->_maxx +
						       win->_ch_off)
					win->_lastch[wy] = win->_ch_off;
				if (win->_lastch[wy] < win->_firstch[wy])
					win->_firstch[wy] = _NOCHANGE;
			}
# ifdef DEBUG
		fprintf(outf, "\t%d\t%d\n", win->_firstch[wy],
			win->_lastch[wy]);
# endif
	}

	if (win == curscr)
		domvcur(ly, lx, win->_cury, win->_curx);
	else {
		if (win->_leave) {
			curscr->_cury = ly;
			curscr->_curx = lx;
			ly -= win->_begy;
			lx -= win->_begx;
			if (ly >= 0 && ly < win->_maxy && lx >= 0 &&
			    lx < win->_maxx) {
				win->_cury = ly;
				win->_curx = lx;
			}
			else
				win->_cury = win->_curx = 0;
		}
		else {
			domvcur(ly, lx, win->_cury + win->_begy,
				win->_curx + win->_begx);
			curscr->_cury = win->_cury + win->_begy;
			curscr->_curx = win->_curx + win->_begx;
		}
	}
	retval = OK;
ret:
	_win = NULL;
	fflush(stdout);
	return retval;
}

/*
 * make a change on the screen
 */
STATIC
makech(win, wy)
reg WINDOW	*win;
_SHORT		wy;
{
	reg char	*nsp, *csp, *ce;
	reg _SHORT	wx, lch, y;
	reg int		nlsp, clsp;	/* last space in lines		*/

	wx = win->_firstch[wy] - win->_ch_off;
	if (wx >= win->_maxx)
		return OK;
	else if (wx < 0)
		wx = 0;
	lch = win->_lastch[wy] - win->_ch_off;
	if (lch < 0)
		return OK;
	else if (lch >= win->_maxx)
		lch = win->_maxx - 1;;
	y = wy + win->_begy;

	if (curwin)
		csp = " ";
	else
		csp = &curscr->_y[wy + win->_begy][wx + win->_begx];

	nsp = &win->_y[wy][wx];
	if (CE && !curwin) {
		for (ce = &win->_y[wy][win->_maxx - 1]; *ce == ' '; ce--)
			if (ce <= win->_y[wy])
				break;
		nlsp = ce - win->_y[wy];
	}

	if (!curwin)
		ce = CE;
	else
		ce = NULL;

	while (wx <= lch) {
		if (*nsp != *csp) {
			domvcur(ly, lx, y, wx + win->_begx);
# ifdef DEBUG
			fprintf(outf, "MAKECH: 1: wx = %d, lx = %d\n", wx, lx);
# endif	
			ly = y;
			lx = wx + win->_begx;
			while (*nsp != *csp && wx <= lch) {
				if (ce != NULL && wx >= nlsp && *nsp == ' ') {
					/*
					 * check for clear to end-of-line
					 */
					ce = &curscr->_y[ly][COLS - 1];
					while (*ce == ' ')
						if (ce-- <= csp)
							break;
					clsp = ce - curscr->_y[ly] - win->_begx;
# ifdef DEBUG
					fprintf(outf, "MAKECH: clsp = %d, nlsp = %d\n", clsp, nlsp);
# endif
					if (clsp - nlsp >= strlen(CE)
					    && clsp < win->_maxx) {
# ifdef DEBUG
						fprintf(outf, "MAKECH: using CE\n");
# endif
						_puts(CE);
						lx = wx + win->_begx;
						while (wx++ <= clsp)
							*csp++ = ' ';
						return OK;
					}
					ce = NULL;
				}
				/*
				 * enter/exit standout mode as appropriate
				 */
				if (SO && (*nsp&_STANDOUT) != (curscr->_flags&_STANDOUT)) {
					if (*nsp & _STANDOUT) {
						_puts(SO);
						curscr->_flags |= _STANDOUT;
					}
					else {
						_puts(SE);
						curscr->_flags &= ~_STANDOUT;
					}
				}
				wx++;
				if (wx >= win->_maxx && wy == win->_maxy - 1)
					if (win->_scroll) {
					    if ((curscr->_flags&_STANDOUT) &&
					        (win->_flags & _ENDLINE))
						    if (!MS) {
							_puts(SE);
							curscr->_flags &= ~_STANDOUT;
						    }
					    if (!curwin)
						_putchar((*csp = *nsp) & 0177);
					    else
						_putchar(*nsp & 0177);
					    if (win->_flags&_FULLWIN && !curwin)
						scroll(curscr);
					    ly = win->_begy+win->_cury;
					    lx = win->_begx+win->_curx;
					    return OK;
					}
					else if (win->_flags&_SCROLLWIN) {
					    lx = --wx;
					    return ERR;
					}
				if (!curwin)
					_putchar((*csp++ = *nsp) & 0177);
				else
					_putchar(*nsp & 0177);
# ifdef FULLDEBUG
				fprintf(outf,
					"MAKECH:putchar(%c)\n", *nsp & 0177);
# endif
				if (UC && (*nsp & _STANDOUT)) {
					_putchar('\b');
					_puts(UC);
				}
				nsp++;
			}
# ifdef DEBUG
			fprintf(outf, "MAKECH: 2: wx = %d, lx = %d\n", wx, lx);
# endif	
			if (lx == wx + win->_begx)	/* if no change */
				break;
			lx = wx + win->_begx;
			if (lx >= COLS && AM) {
				lx = 0;
				ly++;
				/*
				 * xn glitch: chomps a newline after auto-wrap.
				 * we just feed it now and forget about it.
				 */
				if (XN) {
					_putchar('\n');
					_putchar('\r');
				}
			}
		}
		else if (wx <= lch)
			while (*nsp == *csp && wx <= lch) {
				nsp++;
				if (!curwin)
					csp++;
				++wx;
			}
		else
			break;
# ifdef DEBUG
		fprintf(outf, "MAKECH: 3: wx = %d, lx = %d\n", wx, lx);
# endif	
	}
	return OK;
}

/*
 * perform a mvcur, leaving standout mode if necessary
 */
STATIC
domvcur(oy, ox, ny, nx)
int	oy, ox, ny, nx; {

	if (curscr->_flags & _STANDOUT && !MS) {
		_puts(SE);
		curscr->_flags &= ~_STANDOUT;
	}
	mvcur(oy, ox, ny, nx);
}
