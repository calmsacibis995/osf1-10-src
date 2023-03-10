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
static char rcsid[] = "@(#)$RCSfile: miniinit.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 21:57:52 $";
#endif
/*
 * HISTORY
 */
/*** "miniinit.c  1.8  com/lib/curses,3.1,9008 12/4/89 21:01:55"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   m_initscr, m_newterm, makenew
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
# include	<signal.h>

char	*calloc();
extern	char	*getenv();

static WINDOW	*makenew();

struct screen *m_newterm();

/*
 * NAME:        m_initsc
 *
 * FUNCTION:
 *
 *      This routine initializes the current and standard screen.
 */

WINDOW *
m_initscr() {
	register char	*sp;
	
# ifdef DEBUG
	if (outf == NULL) {
		outf = fopen("trace", "w");
		if (outf == NULL) {
			perror("trace");
			exit(-1);
		}
	}
#endif

	if (isatty(2)) {
		if ((sp = getenv("TERM")) == NULL)
			sp = Def_term;
# ifdef DEBUG
		if(outf) fprintf(outf, "INITSCR: term = %s\n", sp);
# endif
	}
	else {
		sp = Def_term;
	}
	(void) m_newterm(sp, stdout, stdin);
	return stdscr;
}

/*
 * NAME:        m_newterm
 */

struct screen *
m_newterm(type, outfd, infd)
char *type;
FILE *outfd, *infd;
{
	int		m_tstp();
	struct screen *scp;
	struct screen *_new_tty();
	extern int _endwin;

#ifdef DEBUG
	if(outf) fprintf(outf, "NEWTERM() isatty(2) %d, getenv %s\n",
		isatty(2), getenv("TERM"));
# endif
	SP = (struct screen *) calloc(1, sizeof (struct screen));
	SP->term_file = outfd;
	SP->input_file = infd;
	savetty();
	scp = _new_tty(type, outfd);
# ifdef SIGTSTP
	signal(SIGTSTP, ((void (*)(int))m_tstp));
# endif

	LINES =	lines;
	COLS =	columns;
# ifdef DEBUG
	if(outf) fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	curscr = makenew(LINES, COLS, 0, 0);
	stdscr = makenew(LINES, COLS, 0, 0);
# ifdef DEBUG
	if(outf) fprintf(outf,
	"SP %x, stdscr %x, curscr %x\n", SP, stdscr, curscr);
# endif
	SP->std_scr = stdscr;
	SP->cur_scr = curscr;
	_endwin = FALSE;
	return scp;
}

/*
 * NAME:        makenew
 *
 * FUNCTION:
 *
 *      This routine sets up a _window buffer and returns a pointer to it.
 */

static WINDOW *
makenew(num_lines, num_cols, begy, begx)
int	num_lines, num_cols, begy, begx;
{
	register WINDOW	*win;
	char *calloc();

# ifdef	DEBUG
	if(outf) fprintf(outf,
	"MAKENEW(%d, %d, %d, %d)\n", num_lines, num_cols, begy, begx);
# endif
	if ((win = (WINDOW *) calloc(1, sizeof (WINDOW))) == NULL)
		return NULL;
# ifdef DEBUG
	if(outf) fprintf(outf, "MAKENEW: num_lines = %d\n", num_lines);
# endif
	win->_cury = win->_curx = 0;
	win->_maxy = num_lines;
	win->_maxx = num_cols;
	win->_begy = begy;
	win->_begx = begx;
	win->_scroll = win->_leave = win->_use_idl = FALSE;
	return win;
}
