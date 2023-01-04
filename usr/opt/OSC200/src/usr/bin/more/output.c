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
static char rcsid[] = "@(#)$RCSfile: output.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/14 17:42:52 $";
#endif
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	output.c,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.2  1992/08/24  18:18:27  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:31:01  tom]
 *
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)output.c	5.10 (Berkeley) 7/24/91";
#endif /* not lint */

/*
 * High level routines dealing with the output to the screen.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>	/* MB_LEN_MAX */
#include <ctype.h>
#include <stdlib.h>	/* wctomb() */
#include "less.h"

int errmsgs;	/* Count of messages displayed by error() */

extern int sigs;
extern int sc_width, sc_height;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern int bo_width, be_width;
extern int tabstop;
extern int screen_trashed;
extern int any_display;
extern wchar_t *line;
extern int show_all_opt, show_opt, bs_mode;
extern int mbcodeset;
extern int null_count;

/* display the line which is in the line buffer. */
void
put_line(void)
{
	register wchar_t *wp;
	register wchar_t wc;
	register int column;
	extern int auto_wrap, ignaw;

	if (sigs)
	{
		/*
		 * Don't output if a signal is pending.
		 */
		screen_trashed = 1;
		return;
	}

	if (line == NULL)
		line = L"";

	column = 0;
	/* check even NULL code */
	for (wp = line;( null_count > 1) ||  ( *wp != L'\0') ;  wp++)
	{
		switch (wc = *wp)
		{
		case ESC_CHAR:
			switch (wc = *++wp)
			{
			case UL_CHAR:
				ul_enter();
				column += ul_width +1;
				break;
			case UE_CHAR:
				ul_exit();
				column += ue_width;
				break;
			case BO_CHAR:
				bo_enter();
				column += bo_width +1;
				break;
			case BE_CHAR:
				bo_exit();
				column += be_width;
				break;
			case ESC_CHAR:
				goto wcshow_it;
				break;
			default:
				error("default case in putline/ESC_CHAR!!");
			}
			break;
		case L'\t':
			if (show_all_opt)
				goto wcshow_it;
			else
				do
				{
					column++;
				} while ((column % tabstop) != 0);
			putchr('\t');
			break;
		case L'\b':
			if (show_all_opt || bs_mode)
				goto wcshow_it;
			else 
			{
				putbs();
				if (column != 0) column--;
			}
			break;
		case L'\r':
			if (show_all_opt)
				goto wcshow_it;
			else
				column = 0;
			break;
		/* If -z option, NULL is ^@ . */
		case L'\0':
			if(show_all_opt){
			 	putstr("^@");
				column +=2;
				null_count--;
			 	break;
			}
			else{ 
				null_count--;
				break;
			}
		default:
wcshow_it:		
			/*
			 * This pretty much only works for ascii
			 * wide characters.  Can we have a 
			 * non-printable multi-byte char? 
			 * If so, how do we show it?
			 */
			if (show_opt) {
				if (!iswprint(wc) && !iswcntrl(wc)) {
					putstr("M-");
					column += 2;
					wc = toascii(wc);
				}
				if (iswcntrl(wc)) {
					putchr('^');
					column++;
					wc = CARAT_CHAR(wc);
				}
			}
			putwchr(wc);
			column++;
			break;

		}
	}
	if (column < sc_width || !auto_wrap || ignaw)
		putchr('\n');
}

static char obuf[1024];
static char *ob = obuf;

/*
 * Flush buffered output.
 */
void
flush(void)
{
	register int n;

	n = ob - obuf;
	if (n == 0)
		return;
	if (write(1, obuf, n) != n)
		screen_trashed = 1;
	ob = obuf;
}

/*
 * Purge any pending output.
 */
void
purge(void)
{

	ob = obuf;
}

/*
 * Output a character.
 */
void
putchr(int c)
{
	if (ob >= &obuf[sizeof(obuf)])
		flush();
	*ob++ = c;
}

/*
 * Output a wide character.
 */
void
putwchr(wchar_t wc)
{
	char 	c[MB_LEN_MAX];
	int	len, i;

	if (mbcodeset) {
		len = wctomb(c, wc);
		if (ob + len >= &obuf[sizeof(obuf)])
			flush(); /* may flush a little early, but thats ok */

		for (i=0; i < len; i++)
			*ob++ = c[i];
	}
	else {
		putchr(wc);
	}
}

/*
 * Output a string.
 */
void
putstr(register char *s)
{
	while (*s != '\0')
		putchr(*s++);
}

int cmdstack;
static char return_to_continue[] = "(press RETURN)";

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */
void
error(char *s)
{
	int ch;
	static char *rets = NULL;

	if (rets == NULL)
		rets = MSGSTR(PRETURN, return_to_continue);

	++errmsgs;
	if (!any_display) {
		/*
		 * Nothing has been displayed yet.  Output this message on
		 * error output (file descriptor 2) and don't wait for a
		 * keystroke to continue.
		 *
		 * This has the desirable effect of producing all error
		 * messages on error output if standard output is directed
		 * to a file.  It also does the same if we never produce
		 * any real output; for example, if the input file(s) cannot
		 * be opened.  If we do eventually produce output, code in
		 * edit() makes sure these messages can be seen before they
		 * are overwritten or scrolled away.
		 */
		(void)write(2, s, strlen(s));
		(void)write(2, "\n", 1);
		return;
	}

	lower_left();
	clear_eol();
	so_enter();
	if (s) {
		putstr(s);
		putstr("  ");
	}
	putstr(rets);
	so_exit();

	if ((ch = getchr()) != '\n') {
		if (ch == 'q')
			quit();
		cmdstack = ch;
	}
	lower_left();

	if ((s ? strlen(s) : 0) + sizeof(rets) + 
		so_width + se_width + 1 > sc_width)
		/*
		 * Printing the message has probably scrolled the screen.
		 * {{ Unless the terminal doesn't have auto margins,
		 *    in which case we just hammered on the right margin. }}
		 */
		repaint();
	flush();
}

static char intr_to_abort[] = "... (interrupt to abort)";

void
ierror(char *s)
{
	lower_left();
	clear_eol();
	so_enter();
	putstr(s);
	putstr(MSGSTR(INTRTOAB, intr_to_abort));
	so_exit();
	flush();
}
