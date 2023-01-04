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
static char rcsid[] = "@(#)$RCSfile: newterm.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:06:37 $";
#endif
/*
 * HISTORY
 */
/*** "newterm.c  1.6  com/lib/curses,3.1,9008 12/4/89 21:02:23"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   newterm
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
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:        newterm
 */

struct screen *
newterm(type, outfd, infd)
char *type;
FILE *outfd, *infd;
{
	extern void _tstp();
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
	/*
	 * The default is echo, for upward compatibility, but we do
	 * all echoing in curses to avoid problems with the tty driver
	 * echoing things during critical sections.
	 */
	SP->fl_echoit = 1;
	savetty();
	scp = _new_tty(type, outfd);
	if (scp == NULL)
		return NULL;
#ifdef USG
	(cur_term->Nttyb).c_lflag &= ~ECHO;
#else
	(cur_term->Nttyb).sg_flags &= ~ECHO;
#endif
	_reset_prog_mode();					/* 001 */
# ifdef SIGTSTP
	signal(SIGTSTP, ((void (*)(int))_tstp));
# endif
	if (curscr != NULL) {
# ifdef DEBUG
		if(outf) fprintf(outf,
		"INITSCR: non null curscr = 0%o\n", curscr);
# endif
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	LINES =	lines;
	COLS =	columns;
	curscr = makenew(LINES, COLS, 0, 0);
	stdscr = newwin(LINES, COLS, 0, 0);
# ifdef DEBUG
	if(outf) fprintf(outf,
		"SP %x, stdscr %x, curscr %x\n", SP, stdscr, curscr);
# endif
	SP->std_scr = stdscr;
	SP->cur_scr = curscr;
	/* Maybe should use makewin and glue _y's to DesiredScreen. */
	_endwin = FALSE;
	return scp;
}
