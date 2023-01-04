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
static char rcsid[] = "@(#)$RCSfile: input.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/14 17:42:22 $";
#endif
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	input.c,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.2  1992/08/24  18:17:00  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:30:22  tom]
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
static char sccsid[] = "@(#)input.c	5.4 (Berkeley) 6/1/90";
#endif /* not lint */

/*
 * High level routines dealing with getting lines of input 
 * from the file being viewed.
 *
 * When we speak of "lines" here, we mean PRINTABLE lines;
 * lines processed with respect to the screen width.
 * We use the term "raw line" to refer to lines simply
 * delimited by newlines; not processed with respect to screen width.
 */

#include <sys/types.h>
#include "less.h"

extern int squeeze;
extern int sigs;
extern wchar_t *line;
extern int null_count;
extern int show_opt;
extern int esc_count;

/*
 * Get the next line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the NEXT line.  The line obtained is the line starting at curr_pos.
 */
off_t
forw_line(off_t curr_pos)
{
	off_t new_pos;
	register wint_t c;

	if (curr_pos == NULL_POSITION || ch_seek(curr_pos))
		return (NULL_POSITION);

	c = ch_forw_get();
	if (c == EOI)
		return (NULL_POSITION);

	prewind();
	/* clear NULL code count in linebuf */
	null_count = 0;
	esc_count =0;
	for (;;)
	{
		if (sigs)
			return (NULL_POSITION);
		if (c == L'\n' || c == EOI)
		{
			/*
			 * End of the line.
			 */
			new_pos = ch_tell();
			break;
		}

		/*
		 * Append the char to the line and get the next char.
		 */
		if (pappend(c))
		{
			/*
			 * The char won't fit in the line; the line
			 * is too long to print in the screen width.
			 * End the line here.
			 */
			if(!show_opt && check_line())
				continue; 
			new_pos = ch_tell() - 1;
			break;
		}
		c = ch_forw_get();
	}
	(void) pappend(L'\0');

	if (squeeze && *line == L'\0')
	{
		/*
		 * This line is blank.
		 * Skip down to the last contiguous blank line
		 * and pretend it is the one which we are returning.
		 */
		while ((c = ch_forw_get()) == L'\n')
			if (sigs)
				return (NULL_POSITION);
		if (c != EOI)
			(void) ch_back_get();
		new_pos = ch_tell();
	}

	return (new_pos);
}

/*
 * Get the previous line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the PREVIOUS line.  The line obtained is the one starting at new_pos.
 */
off_t
back_line(off_t curr_pos)
{
	off_t new_pos, begin_new_pos;
	wint_t c;

	if (curr_pos == NULL_POSITION || curr_pos <= (off_t)0 ||
		ch_seek(curr_pos-1))
		return (NULL_POSITION);

	if (squeeze)
	{
		/*
		 * Find out if the "current" line was blank.
		 */
		(void) ch_forw_get();	/* Skip the newline */
		c = ch_forw_get();	/* First char of "current" line */
		(void) ch_back_get();	/* Restore our position */
		(void) ch_back_get();

		if (c == L'\n')
		{
			/*
			 * The "current" line was blank.
			 * Skip over any preceeding blank lines,
			 * since we skipped them in forw_line().
			 */
			while ((c = ch_back_get()) == L'\n')
				if (sigs)
					return (NULL_POSITION);
			if (c == EOI)
				return (NULL_POSITION);
			(void) ch_forw_get();
		}
	}

	/*
	 * Scan backwards until we hit the beginning of the line.
	 */
	for (;;)
	{
		if (sigs)
			return (NULL_POSITION);
		c = ch_back_get();
		if (c == L'\n')
		{
			/*
			 * This is the newline ending the previous line.
			 * We have hit the beginning of the line.
			 */
			new_pos = ch_tell() + 1;
			break;
		}
		if (c == EOI)
		{
			/*
			 * We have hit the beginning of the file.
			 * This must be the first line in the file.
			 * This must, of course, be the beginning of the line.
			 */
			new_pos = ch_tell();
			break;
		}
	}

	/*
	 * Now scan forwards from the beginning of this line.
	 * We keep discarding "printable lines" (based on screen width)
	 * until we reach the curr_pos.
	 *
	 * {{ This algorithm is pretty inefficient if the lines
	 *    are much longer than the screen width, 
	 *    but I don't know of any better way. }}
	 */
	if (ch_seek(new_pos))
		return (NULL_POSITION);
    loop:
	begin_new_pos = new_pos;
	prewind();
	/* clear NULL code count in linebuf */
	null_count =0;
	esc_count =0;

	do
	{
		c = ch_forw_get();
		if (c == EOI || sigs)
			return (NULL_POSITION);
		new_pos++;
		if (c == L'\n')
			break;
		if (pappend(c))
		{
			/*
			 * Got a full printable line, but we haven't
			 * reached our curr_pos yet.  Discard the line
			 * and start a new one.
			 */
			if(!show_opt && check_line())
				continue; 
			(void) pappend(L'\0');
			(void) ch_back_get();
			new_pos--;
			goto loop;
		}
	} while (new_pos < curr_pos);

	(void) pappend(L'\0');

	return (begin_new_pos);
}
