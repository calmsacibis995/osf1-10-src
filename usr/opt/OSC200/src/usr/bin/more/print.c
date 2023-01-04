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
static char	*sccsid = "@(#)$RCSfile: print.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 93/02/23 18:07:09 $";
#endif
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
#if !defined(lint) && !defined(_NOIDENT)
#endif
*/
/*
 * print.c : output routines.
 *
 * 	NOTE: These routines currently assume that they will
 *		not be called with strings long enough to
 *		wrap to a new line.  This needs to be fixed.
 */

#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include "globals.h"

/* Simplified printf function that keeps track of cursor position */

/*VARARGS*/
printf (va_alist)
va_dcl
{
	va_list ap;
	register char ch;
	register char *fmt;

	va_start(ap);
	fmt = va_arg(ap, char *);
	while (*fmt) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0') {
				va_end(ap);
				if (cursor_line >=0)	/*001*/
				if (cursor_column > line_width(cursor_line))
					line_width(cursor_line) = cursor_column;
				return;
			}
			/*
			 * If going to a new line, erase any remaining
			 * text on current line.
			 */
			if (ch == '\n' && !no_tty) {
				erase(cursor_column);
				newline();
			}
			else {
				putchar(ch);
				if (ch != '\r')
					cursor_column++;
			}
		}
		switch (*fmt++) {
		case 'd':
			printd(va_arg(ap, int));
			break;
		case 's':
			pr(va_arg(ap, char *));
			break;
		case '%':
			cursor_column++;
			putchar('%');
			break;
		case '\0':
		default:
			break;
		}
	}
	va_end(ap);
	if (cursor_column > line_width(cursor_line))
		line_width(cursor_line) = cursor_column;
}

/*
 * Print an integer as a string of decimal digits,
 * returning the length of the print representation.
 */

printd (n)
int n;
{
	int a;

	if (a = n/10)
		printd(a);
	putchar(n % 10 + '0');
	cursor_column++;
}

/*
 *  Print string and return number of characters
 */

pr(s1)
char	*s1;
{
	register char	*s;
	register char	c;

	if (s1 == NULL)
		s1 = "(nil)";
	for (s = s1; c = *s++; )
		if (c == '\n') {
			erase(cursor_column);
			newline();
		}
		else {
			cursor_column++;
			putchar(c);
		}
}

static char bell = ctrl('G');

ring_bell()
{
	if (visible_bell) {
		tputs(visible_bell, 1, putch);
		fflush(stdout);
	}
	else
		write (ttyfd, &bell, 1);
}

prompt (filename)
char *filename;
{
	int len;
	extern int force_wait;

	if (line_width(cursor_line) > 0)
		kill_line ();
	if (!hard) {
		putchar('\r');
		if (bstate)  /* if in bold mode, turn off bold */  /* paw: qar 2411 */
		        tputs (Mexit, 1, putch);
		if (Senter && Sexit)
			tputs (Senter, 1, putch);
		printf(MSGSTR(MORE, "--More--"));
		if (filename != NULL)
			printf (MSGSTR(NEXTFILE,"(Next file: %s)"), filename);
		else if (feof(curfile))
			printf ("(EOF)");
		else if (!ispipe) {
			force_wait = 0;
			printf ("(%d%%)", (int)((file_pos * 100) / file_size));
		}
		if (verbose)
			printf(MSGSTR(CONT1, "[Press space to continue, q to quit, h for help]"));
		len = cursor_column + 2*soglitch;
		if (Senter && Sexit)
			tputs (Sexit, 1, putch);
		if ((clreol) && (show_opt == 0))	/* 002 */
			clreos();			/* 002 */
		if (bstate)  /* restore bold mode if appropriate */  /* paw: qar 2411 */
		        tputs (Benter, 1, putch);
		fflush(stdout);
		line_width(cursor_line) = promptlen = len;
	}
	else
		ring_bell();
	inwait++;
}

/*
 * Print a buffer of n characters that occupies 'width' screen columns.
 */

prline (s, n, width, resid)
register unsigned char *s;
register int n;
int width;
int resid;
{
	register unsigned int c;	/* next output character */
	register int state;		/* next output char's UL state */
	int column = cursor_column;

#define wouldul(s,n)	((n) >= 2 && (((s)[0] == '_' && (s)[1] == '\b') || \
					((s)[1] == '\b' && (s)[2] == '_')))

        if (clreol) cleareol(); /* clear to end of line if option is set */ /* paw: QAR 2441 */
	
	while (--n >= 0) {
		c = *s;
		if (!show_opt) {
			if (bstate) {				/* paw: qar 2411 */
				if (strncmp(s, Mexit, strlen(Mexit)) == 0)
					bstate = 0;
			}
			else {
				if (strncmp(s, Benter, strlen(Benter)) == 0)
					bstate = 1;
			}				/* paw: end qar 2411 */	
		}
		if (!ul_opt) {
			show(c, &column, fold_opt, resid);
			s++;
		}
		else {
			if (c == ' ' && pstate == 0 && ulglitch && wouldul(s+1, n-1)) {
				s++;
				continue;
			}
			if (state = wouldul(s, n)) {
				if (c == '_')
					c = s[2];
				n -= 2;
				s += 3;
			} else
				s++;
			if (state != pstate) {
				if (c == ' ' && state == 0 && ulglitch && wouldul(s, n-1))
					state = 1;
				else
					tputs(state ? ULenter : ULexit, 1, putch);
			}
			if (c != ' ' || pstate == 0 || state != 0 || ulglitch == 0)
				show(c, &column, fold_opt, resid);
			if (state && *chUL) {
				pr(chBS);
				tputs(chUL, 1, putch);
			}
			pstate = state;
		}
	}
	/*
	 * Never leave terminal in standout mode
	 * at the end of a line.  Ann Arbor Ambassadors
	 * don't like that.
	 */
	if (pstate) {
		tputs(ULexit, 1, putch);
		pstate = 0;
	}
	if (width < line_width(cursor_line))
		erase (width);
	else
		line_width(cursor_line) = cursor_column = width;
	if (width == ScreenWidth && eatnl)
		printf("\r\n");
	else if (width < ScreenWidth || !fold_opt)
		newline();
	else if (width == ScreenWidth && fold_opt)
		nextline();
}

show(ch, colp, fold, resid)
register unsigned int ch;
int *colp;
int fold;
register int resid;
{
	register int column = *colp;

	if (ch == '\b' && !show_all_opt && column > 0)
		column--;
	else if (ch == '\r' && !show_all_opt)
		column = 0;
	else if (ch == '\t'&& !show_all_opt )
		column = 1 + (column | 7);
	else if (show_opt || (stop_opt && ch == '\f')) {
		if (!isprint(ch) && !iscntrl(ch)) {
			/* XXX    ASCII only    XXX */
			if (column != 0 || !resid) {
				putchar('M');
				column++;
			}
			else
				resid--;
			if (column < ScreenWidth || !fold) {
				if (column != 0 || !resid) {
					putchar('-');
					column++;
				}
				else
					resid--;
			}
			ch = toascii(ch);
		}
		if (iscntrl(ch)) {
			/* XXX    ASCII only    XXX */
			ch = (ch == '\177') ? '?' : ch | 0100;
			if (column != 0 || !resid) {
				putchar('^');
				column++;
			}
			else
				resid--;
		}
	}
	if (column < ScreenWidth || !isprint(ch) || !fold) {
		putchar(ch);
		if (isprint(ch))
			column++;
	}
	*colp = column;
}

printchar(ch)
register unsigned int ch;
{
	show(ch, &cursor_column, 0);
	if (cursor_column > line_width(cursor_line))
		line_width(cursor_line) = cursor_column;
}

/*
 * Print a highlighted message on the prompt line.
 */
message(mess)
char *mess;
{
	kill_line ();
	if (Senter && Sexit) {
		tputs (Senter, 1, putch);
		printf ("%s", mess);
		tputs (Sexit, 1, putch);
		line_width(cursor_line) += 2*soglitch;
	}
	else
		printf ("%s", mess);
	fflush(stdout);
}
