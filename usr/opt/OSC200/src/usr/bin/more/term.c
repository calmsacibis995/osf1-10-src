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
static char	*sccsid = "@(#)$RCSfile: term.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/11/25 13:40:09 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 *	Copyright 1990, Eric Shienbrood
 *
 * This software may be freely copied, distributed, or modified, as long
 * this copyright notice is preserved.
 *
 *
 * term.c : Routines that interact with the terminal.
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include "globals.h"

#define TBUFSIZ	1024

unsigned char	obuf[BUFSIZ];	/* stdout buffer */

/* External variables used  by termcap: */

extern char	PC;		/* pad character */
extern short	ospeed;		/* output baud rate */

unsigned char readch();

get_terminal_info ()
{
	char	unsigned buf[TBUFSIZ];
	char	*cp; 
	unsigned char	*padstr;
	int		lmode;
	int		tgrp;
	int             got_term_info;                            /* 001 */
#ifdef TIOCGWINSZ
	struct winsize win;
#endif
	static char	clearbuf[TBUFSIZ];/* holds termcap string capabilities */
	static char	cursorhome[40];	/* cursor motion to home if */
					/*     no termcap home string */
	char		*tgetstr();
	char		*malloc();

	got_term_info = 0;
	if ((ttyfd = open("/dev/tty", O_RDWR)) < 0)
		ttyfd = 2;
    retry:
	if (!(no_tty = ioctl(fileno(stdout), TIOCGETP, &otty))) {
	        got_term_info = 1;                                /* 001 */
		if (ioctl(ttyfd, TIOCLGET, &lmode) < 0) {
			perror("TIOCLGET");
			exit(1);
		}
		docrterase = ((lmode & LCRTERA) != 0);
		docrtkill = ((lmode & LCRTKIL) != 0);
		/*
		 * Wait until we're in the foreground before we save the
		 * terminal modes.
		 */
		if (ioctl(fileno(stdout), TIOCGPGRP, &tgrp) < 0) {
			perror("TIOCGPGRP");
			exit(1);
		}
		if (tgrp != getpgrp(0)) {
			kill(0, SIGTTOU);
			goto retry;
		}
		setbuffer(stdout, (char *)obuf, sizeof obuf);
		ScreenLength = 24;
		ScreenWidth = 80;
		if (!(cp = getenv("TERM")) || tgetent(buf, cp) <= 0) {
			dumb++;
			ul_opt = 0;
		}
		else {
			if (((ScreenLength = tgetnum("li")) < 0) || tgetflag("hc")) {
				hard++;	/* Hard copy terminal */
				ScreenLength = 24;
			}
			if (tailequ (fnames[0], "page") || !hard && tgetflag("ns"))
				noscroll++;
			if ((ScreenWidth = tgetnum("co")) < 0)
				ScreenWidth = 80;
#ifdef TIOCGWINSZ
			if (ioctl(fileno(stdout), TIOCGWINSZ, &win) == 0) {
				if (win.ws_row != 0)
					ScreenLength = win.ws_row;
				if (win.ws_col != 0)
					ScreenWidth = win.ws_col;
			}
#endif
			Wrap = tgetflag("am");
			bad_so = tgetflag ("xs");
			/* Eat newline at last column+1; dec, concept */
			eatnl = tgetflag("xn");
			cp = clearbuf;
			EraseLineStr = tgetstr("ce",&cp);
			ClearScreenStr = tgetstr("cl", &cp);
			Senter = tgetstr("so", &cp);
			Sexit = tgetstr("se", &cp);
			if ((soglitch = tgetnum("sg")) < 0)
				soglitch = 0;
			visible_bell = tgetstr("vb", &cp);

			/*
			 *  Set up for underlining:  some terminals don't need
			 *  it, others have start/stop sequences, still others 
			 *  have an underline char sequence which is assumed to 
			 *  move the cursor forward one character.  If underline
			 *  sequence isn't available, settle for standout
			 *  sequence.
			 */

			if (tgetflag("ul") || tgetflag("os"))
				ul_opt = 0;
			if ((chUL = tgetstr("uc", &cp)) == NULL )
				chUL = "";
			if (((ULenter = tgetstr("us", &cp)) == NULL ||
			     (ULexit = tgetstr("ue", &cp)) == NULL) && !*chUL) {
				if ((ULenter = Senter) == NULL || (ULexit = Sexit) == NULL) {
					ULenter = "";
					ULexit = "";
				}
				else
					ulglitch = soglitch;
			}
			else {
				if ((ulglitch = tgetnum("ug")) < 0)
					ulglitch = 0;
			}

			if (padstr = (unsigned char *)tgetstr("pc", &cp))
				PC = *padstr;

			CursorMotionStr = tgetstr("cm", &cp);
			HomeStr = tgetstr("ho", &cp);
			if (HomeStr == NULL && CursorMotionStr != NULL) {
				strcpy(cursorhome, tgoto(CursorMotionStr, 0, 0));
				HomeStr = cursorhome;
			}
			EodClrStr = tgetstr("cd", &cp);
			if (tgetflag("bs") || (chBS = tgetstr("bc", &cp)) == NULL)
				chBS = "\b";
			ScrollUpStr	= tgetstr("sr", &cp);
			if (ScrollUpStr == NULL)
				ScrollUpStr = tgetstr("al", &cp);
		}
		/*
		 * Allocate the screeninfo array, which gives
		 * the width of each line on the display.
		 * Set the current cursor line, assuming that
		 * we are at the bottom of the screen.
		 */
		Screen = (struct screeninfo *)
				malloc(ScreenLength * sizeof(struct screeninfo));
		if (Screen == NULL) {
			perror(MSGSTR(NOMEM2, "Not enough memory"));
			exit(1);
		}
		forget_screen_state();
		line_width(cursor_line) = 0;
	}
	/* 001 - start of change */
	if (no_intty = ioctl(fileno(stdin), TIOCGETP, &savetty)) {
	    	ioctl(ttyfd, TIOCGETP, &savetty);
	        if (got_term_info == 0)
		       otty = savetty;
		else   savetty = otty;
	}
	else if (got_term_info == 0)
		otty = savetty;
	else    savetty = otty;
	/* 001 - end of change */
	ospeed = obaud(otty);
	slow_tty = ospeed < B1200;
	hardtabs = !xtabs(otty);
	if (!no_tty) {
		echo_off(otty);
		brks_on(otty);
	}
}

unsigned char
readch ()
{
	unsigned char ch;

	errno = 0;
	if (read (ttyfd, &ch, 1) <= 0)
		if (errno != EINTR)
			end_it();
		else
			ch = kill_ch(savetty);
	return (ch);
}

wait_for_keypress()
{
	message(MSGSTR(CONT2, "Press any key to continue"));
	(void)readch();
}

/*
 * Read a line from the keyboard, performing erase and kill processing.
 */

#define ERASEONECHAR() \
	if (docrterase) \
		fputs("\b \b", stdout); \
	else \
		putchar('\b')

read_processed_tty_line (buf, nmax, pchar)
unsigned char buf[];
register int nmax;
unsigned char pchar;
{
	register unsigned char	*sptr;
	register unsigned char	ch;
	register int	quotenext = 0;
	register int	curcol;
	int		maxcol;

	sptr = buf;
	curcol = cursor_column;
	maxcol = curcol;
	while (sptr - buf < nmax) {
		if (curcol > maxcol)
			maxcol = curcol;
		ch = readch ();
		if (ch == '\\')
			quotenext++;
		else if ((ch == erase_ch(savetty)) && !quotenext) {
			if (sptr > buf) {
				--curcol;
				--maxcol;
				ERASEONECHAR();
				--sptr;
				if ((iscntrl((int)*sptr) && *sptr != '\n') || *sptr == RUBOUT) {
					--curcol;
					--maxcol;
					ERASEONECHAR();
				}
				fflush(stdout);
				continue;
			}
			else {
				if (!EraseLineStr)
					cursor_column = maxcol;
				line_width(cursor_line) = cursor_column;
				return 0;
			}
		}
		else if ((ch == kill_ch(savetty)) && !quotenext) {
			if (hard) {
				printchar((unsigned int)ch);
				putchar('\n');
				putchar(pchar);
			}
			else {
				putchar ('\r');
				putchar (pchar);
				line_width(cursor_line) = curcol;
				if (EraseLineStr)
					erase (1);
				else if (docrtkill)
					while (curcol-- > 1)
						ERASEONECHAR();
				curcol = 1;
			}
			sptr = buf;
			fflush (stdout);
			continue;
		}
		if (quotenext && (ch == kill_ch(savetty) || ch == erase_ch(savetty))) {
			ERASEONECHAR();
			--sptr;
		}
		if (ch != '\\')
			quotenext = 0;
		*sptr++ = ch;
		if ((!isprint(ch) && ch != '\n' && ch != ESC) || ch == RUBOUT) {
			ch += ch == RUBOUT ? -0100 : 0100;
			putchar ('^');
			curcol++;
		}
		if (ch != '\n' && ch != ESC) {
			putchar (ch);
			fflush(stdout);
			curcol++;
		}
		else
			break;
	}
	fflush(stdout);
	*--sptr = '\0';
	if (!EraseLineStr)
		curcol = maxcol;
	line_width(cursor_line) = cursor_column = curcol;
	if (sptr - buf >= nmax - 1)
		message(MSGSTR(LINETOLONG, "Line too long"));
	return 1;
}

set_tty ()
{
	ioctl(ttyfd, TIOCSETN, &otty);
}

reset_tty ()
{
	extern int putch();

	if (no_tty)
		return;
	if (pstate) {
		tputs(ULexit, 1, putch);
		fflush(stdout);
		pstate = 0;
	}
	tputs(Mexit, 1, putch); /* paw: QAR 1882 */
	ioctl(ttyfd, TIOCSETN, &savetty);
}

/*
 * A real function, for the tputs routine in termlib
 */

putch (ch)
char ch;
{
	putchar (ch);
}
