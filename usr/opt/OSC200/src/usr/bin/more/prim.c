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
static char rcsid[] = "@(#)$RCSfile: prim.c,v $ $Revision: 1.1.5.6 $ (DEC) $Date: 1994/01/29 17:00:40 $";
#endif
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	prim.c,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.4  1992/09/23  16:39:25  tom
 * 	Bug 7801 - Make the regular expression in search static.
 * 	[1992/09/23  16:39:06  tom]
 *
 * Revision 1.1.2.3  1992/09/14  23:42:59  tom
 * 	Bug 7685 - Call pos_clear() after clearing the screen in forw().
 * 	[1992/09/14  23:42:27  tom]
 * 
 * Revision 1.1.2.2  1992/08/24  18:18:52  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:31:12  tom]
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
static char sccsid[] = "@(#)prim.c	5.8 (Berkeley) 6/1/90";
#endif /* not lint */

/*
 * Primitives for displaying the file on the screen.
 */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>
#include <stdlib.h>	/* wcstombs() */
#include "less.h"

int back_scroll = -1;
int hit_eof;		/* keeps track of how many times we hit end of file */
int screen_trashed;

static int squished;

extern int sc_window;					/*GA001*/
extern int sigs;
extern int top_scroll;
extern int sc_width, sc_height;
extern int caseless;
extern int linenums;
extern int tagoption;
extern wchar_t *line;

static off_t find_offset(off_t);
static int badmark(int);
int	screen_line;
int	top_line;
int	actual_line;

/*
 * Check to see if the end of file is currently "displayed".
 */
void
eof_check(void)
{
	off_t pos;

	if (sigs)
		return;
	/*
	 * If the bottom line is empty, we are at EOF.
	 * If the bottom line ends at the file length,
	 * we must be just at EOF.
	 */
	pos = position(BOTTOM_PLUS_ONE);
	if (pos == NULL_POSITION || pos == ch_length())
		hit_eof++;
}

/*
 * If the screen is "squished", repaint it.
 * "Squished" means the first displayed line is not at the top
 * of the screen; this can happen when we display a short file
 * for the first time.
 */
void
squish_check(void)
{
	if (squished) {
		squished = 0;
		repaint();
	}
}

/*
 * Display n lines, scrolling forward, starting at position pos in the
 * input file.  "only_last" means display only the last screenful if
 * n > screen size.
 */
void
forw(register int n, off_t pos, int only_last)
{
	extern int short_file;
	static int first_time = 1;
	int eof = 0, do_repaint;

	squish_check();

	/*
	 * do_repaint tells us not to display anything till the end, 
	 * then just repaint the entire screen.
	 */
	do_repaint = (only_last && n > sc_height-1);

	if (!do_repaint) {
		if (top_scroll && n >= sc_height - 1) {
			/*
			 * Start a new screen.
			 * {{ This is not really desirable if we happen
			 *    to hit eof in the middle of this screen,
			 *    but we don't yet know if that will happen. }}
			 */
			clear();
			pos_clear();
			home();
		} else {
		/* DSS01 Removed lower_left() call. This changes the behavior
		 * on short files (less than a screenful) to print at the
		 * cursor rather than at the lower left corner of the screen.
		 */
                        move_bol();  /* qar 19523-resize window repaint DWD */
			clear_eol();
		}

		/*
		 * This is not contiguous with what is currently displayed.
		 * Clear the screen image (position table) and start a new
		 * screen.
		 */
		if (pos != position(BOTTOM_PLUS_ONE)) {
			pos_clear();
			add_forw_pos(pos);
			if (top_scroll) {
				clear();
				home();
			/* DSS01  Check the short_file var before printing
			 * the "...skipping..." message.
			 */
			} else if (!first_time && !short_file)
				putstr(MSGSTR(SKIP, "...skipping...\n"));
		}
	}

	for (short_file = 0; --n >= 0;) {
		/*
		 * Read the next line of input.
		 */
		pos = forw_line(pos);
		if (pos == NULL_POSITION) {
			/*
			 * end of file; copy the table if the file was
			 * too small for an entire screen.
			 */
			eof = 1;
			if (position(TOP) == NULL_POSITION) {
				copytable();
				if (!position(TOP))
					short_file = 1;
			}
			break;
		}
		/*
		 * Add the position of the next line to the position table.
		 * Display the current line on the screen.
		 */
		add_forw_pos(pos);
		if (do_repaint)
			continue;
		/*
		 * If this is the first screen displayed and we hit an early
		 * EOF (i.e. before the requested number of lines), we
		 * "squish" the display down at the bottom of the screen.
		 * But don't do this if a -t option was given; it can cause
		 * us to start the display after the beginning of the file,
		 * and it is not appropriate to squish in that case.
		 */
		if (first_time && line == NULL && !top_scroll && !tagoption) {
			squished = 1;
			continue;
		}
		put_line();
	}

	if (eof && !sigs)
		hit_eof++;
	else
		eof_check();
	if (do_repaint)
		repaint();
	first_time = 0;
	(void) currline(BOTTOM);
}

/*
 * Display n lines, scrolling backward.
 */
void
back(register int n, off_t pos, int only_last)
{
	int do_repaint;

	squish_check();
	do_repaint = (n > get_back_scroll() || (only_last && n > sc_height-1));
	hit_eof = 0;
	while (--n >= 0)
	{
		/*
		 * Get the previous line of input.
		 */
		pos = back_line(pos);
		if (pos == NULL_POSITION)
			break;
		/*
		 * Add the position of the previous line to the position table.
		 * Display the line on the screen.
		 */
		add_back_pos(pos);
		if (!do_repaint)
		{
			home();
			add_line();
			put_line();
		}
	}

	eof_check();
	if (do_repaint)
		repaint();
	(void) currline(BOTTOM);
}

/*
 * Display n more lines, forward.
 * Start just after the line currently displayed at the bottom of the screen.
 */
void
forward(int n, int only_last)
{
	off_t pos;
	extern char *first_cmd;

	if (hit_eof) {
		/*
		 * If we're trying to go forward from end-of-file,
		 * go on to the next file.
		 */
		next_file(1);
		return;
	}

	pos = position(BOTTOM_PLUS_ONE);
	if (pos == NULL_POSITION)
	{
		hit_eof++;
		return;
	}
	forw(n, pos, only_last);
}

/*
 * Display n more lines, backward.
 * Start just before the line currently displayed at the top of the screen.
 */
void
backward(int n, int only_last)
{
	off_t pos;

	pos = position(TOP);
	/*
	 * This will almost never happen, because the top line is almost
	 * never empty.
	 */
	if (pos == NULL_POSITION)
		return;   
	back(n, pos, only_last);
}

/*
 * Repaint the screen, starting from a specified position.
 */
void
prepaint(off_t pos)
{
	hit_eof = 0;
	if (sc_window > 0)		/* If sc_window is set for   */
	  forw(sc_window, pos, 0);	/* only a certain number of  */
	else				/* lines (by the -n option), */
	  forw(sc_height-1, pos, 0);	/* display just that many.   */
	screen_trashed = 0;		/* Otherwise display a whole */
}					/* screen full.     (GA001)  */

/*
 * Repaint the screen.
 */
void
repaint(void)
{
	/*
	 * Start at the line currently at the top of the screen
	 * and redisplay the screen.
	 */
	prepaint(position(TOP));
}

/*
 * Jump to the end of the file.
 * It is more convenient to paint the screen backward,
 * from the end of the file toward the beginning.
 */
void
jump_forw(void)
{
	off_t pos;

	if (ch_end_seek())
	{
		error(MSGSTR(NOSEEK, "Cannot seek to end of file"));
		return;
	}
	lastmark();
	pos = ch_tell();
	clear();
	pos_clear();
	add_back_pos(pos);
	back(sc_height - 1, pos, 0);
}

/*
 * Jump to line n in the file.
 */
void
jump_back(register int n)
{
	register wint_t c;
	register int nlines;

	/*
	 * This is done the slow way, by starting at the beginning
	 * of the file and counting newlines.
	 *
	 * {{ Now that we have line numbering (in linenum.c),
	 *    we could improve on this by starting at the
	 *    nearest known line rather than at the beginning. }}
	 */
	if (ch_seek((off_t)0)) {
		/* 
		 * Probably a pipe with beginning of file no longer buffered. 
		 * If he wants to go to line 1, we do the best we can, 
		 * by going to the first line which is still buffered.
		 */
		if (n <= 1 && ch_beg_seek() == 0)
			jump_loc(ch_tell());
		error(MSGSTR(NOBEGIN, "Cannot get to beginning of file"));
		return;
	}

	/*
	 * Start counting lines.
	 */
	for (nlines = 1;  nlines < n;  nlines++)
		while ((c = ch_forw_get()) != L'\n')
			if (c == EOI) {
				char message[40];
				(void)sprintf(message, MSGSTR(ONLYN, "File has only %d lines"),
				    nlines - 1);
				error(message);
				return;
			}
	jump_loc(ch_tell());
}

/*
 * Find the line containing the specified byte offset.
 */
static off_t
find_offset(off_t offset)
{
	off_t pos, linepos;
	off_t curr_offset;
	int search_forward;
	int linenum;

	/*
	 * We're currently at the line just after the bottom line on
	 * the screen.  Need to decide where to start searching and
	 * which direction to go.  If the requested offset is beyond
	 * where we are now, search forward.  If we're past it,
	 * either search forward from the beginning of the file, or
	 * search backward from where we are now, whichever is
	 * shortest.
	 */

	if ((pos = position(BOTTOM_PLUS_ONE)) == NULL_POSITION)
		pos = (off_t) 0;

	curr_offset = ch_byte(pos);
	if (offset >= curr_offset) {
		search_forward = 1;
	}
	else {
		if (offset <= (curr_offset - offset)) {
			pos = (off_t) 0;
			search_forward = 1;
		}
		else {
			search_forward = 0;
		}
	}

	linenum = find_linenum(pos);
	for (;;) {
		/*
		 * Get lines until we find a matching one or 
		 * until we hit end-of-file (or beginning-of-file 
		 * if we're going backwards).
		 */
		if (sigs)
			/*
			 * A signal aborts the search.
			 */
			return(NULL_POSITION);

		if (search_forward) {
			/*
			 * Read the next line, and save the 
			 * starting position of that line in linepos.
			 */
			linepos = pos;
			pos = forw_raw_line(pos);
			if (linenum != 0)
				linenum++;

			/*
			 * We hit EOF/BOF.  The last line spans the
			 * requested offset.
			 */
			if (pos == NULL_POSITION)
				break;

			if (ch_byte(pos) > offset)
				break;
		}
		else {
			/*
			 * Read the previous line and save the
			 * starting position of that line in linepos.
			 */
			pos = back_raw_line(pos);
			linepos = pos;
			if (linenum != 0)
				linenum--;

			/*
			 * We hit EOF/BOF.  We must be trying to read
			 * back through piped input that has fallen
			 * off the LRU chain in ch.c.
			 */
			if (pos == NULL_POSITION)
				return(NULL_POSITION);

			if (ch_byte(pos) <= offset)
				break;
		}

		/*
		 * If we're using line numbers, we might as well
		 * remember the information we have now (the position
		 * and line number of the current line).
		 */
		if (linenums)
			add_lnum(linenum, pos);
	}
	return(linepos);
}

/*
 * Jump to a specified percentage into the file.
 * This is a poor compensation for not being able to
 * quickly jump to a specific line number.
 */
void
jump_percent(int percent)
{
	off_t pos, len, offset;
	register wint_t c;

	/*
	 * Determine the position in the file
	 * (the specified percentage of the file's length).
	 */
	if ((len = ch_length()) == NULL_POSITION)
	{
		error(MSGSTR(NOLENGTH, "Don't know length of file"));
		return;
	}
	offset = (percent * len) / 100;

	/*
	 * Try to seek directly to the offset.  If that works, then
	 * back up to the beginning of the line.  It it doesn't, we
	 * need to find the offset ourselves.
	 */
	if (ch_seek_byte(offset) == 0)
	{
		while ((c = ch_back_get()) != L'\n' && c != EOI)
			;
		if (c == L'\n')
			(void) ch_forw_get();
		pos = ch_tell();
	}
	else
	{
		pos = find_offset(offset);
	}

	if (pos != NULL_POSITION)
		jump_loc(pos);
}

/*
 * Jump to a specified position in the file.
 */
void
jump_loc(off_t pos)
{
	register int nline;
	off_t tpos;

	if ((nline = onscreen(pos)) >= 0) {
		/*
		 * The line is currently displayed, so
		 * just scroll there, unless sc_window
		 * has been set by the -n option to
		 * limit the number of lines displayed.
		 */
		if (sc_window > 0)			   /*GA001*/
		  forw(0, position(BOTTOM_PLUS_ONE), 0);
		else
		  forw(nline, position(BOTTOM_PLUS_ONE), 0);
		return;
	}

	/*
	 * Line is not on screen.
	 * Seek to the desired location.
	 */
	if (ch_seek(pos)) {
		error(MSGSTR(CANTSEEK, "Cannot seek to that position"));
		return;
	}

	/*
	 * See if the desired line is BEFORE the currently displayed screen.
	 * If so, then move forward far enough so the line we're on will be
	 * at the bottom of the screen, in order to be able to call back()
	 * to make the screen scroll backwards & put the line at the top of
	 * the screen.
	 * {{ This seems inefficient, but it's not so bad,
	 *    since we can never move forward more than a
	 *    screenful before we stop to redraw the screen. }}
	 */
	tpos = position(TOP);
	if (tpos != NULL_POSITION && pos < tpos) {
		off_t npos = pos;
		/*
		 * Note that we can't forw_line() past tpos here,
		 * so there should be no EOI at this stage.
		 */
		for (nline = 0;  npos < tpos && nline < sc_height - 1;  nline++)
			npos = forw_line(npos);

		if (npos < tpos) {
			/*
			 * More than a screenful back.
			 */
			lastmark();
			clear();
			pos_clear();
			add_back_pos(npos);
		}

		/*
		 * Note that back() will repaint() if nline > back_scroll.
		 */
		back(nline, npos, 0);
		return;
	}
	/*
	 * Remember where we were; clear and paint the screen.
	 */
	lastmark();
	prepaint(pos);
}

/*
 * The table of marks.
 * A mark is simply a position in the file.
 */
#define	NMARKS		(27)		/* 26 for a-z plus one for quote */
#define	LASTMARK	(NMARKS-1)	/* For quote */
static off_t marks[NMARKS];

/*
 * Initialize the mark table to show no marks are set.
 */
void
init_mark(void)
{
	int i;

	for (i = 0;  i < NMARKS;  i++)
		marks[i] = NULL_POSITION;
}

/*
 * See if a mark letter is valid (between a and z).
 */
static int
badmark(int c)
{
	if (c < 'a' || c > 'z')
	{
		error(MSGSTR(BADLET, "Choose a letter between 'a' and 'z'"));
		return (1);
	}
	return (0);
}

/*
 * Set a mark.
 */
void
setmark(int c)
{
	if (badmark(c))
		return;
	marks[c-'a'] = position(TOP);
}

void
lastmark(void)
{
	marks[LASTMARK] = position(TOP);
}

/*
 * Go to a previously set mark.
 */
void
gomark(int c)
{
	off_t pos;

	if (c == '\'') {
		pos = marks[LASTMARK];
		if (pos == NULL_POSITION)
			pos = 0;
	}
	else {
		if (badmark(c))
			return;
		pos = marks[c-'a'];
		if (pos == NULL_POSITION) {
			error(MSGSTR(NOMARK, "mark not set"));
			return;
		}
	}
	jump_loc(pos);
}

/*
 * Get the backwards scroll limit.
 * Must call this function instead of just using the value of
 * back_scroll, because the default case depends on sc_height and
 * top_scroll, as well as back_scroll.
 */
int
get_back_scroll(void)
{
	if (back_scroll >= 0)
		return (back_scroll);
	if (top_scroll)
		return (sc_height - 2);
	return (sc_height - 1);
}

/*
 * Search for the n-th occurence of a specified pattern, 
 * either forward or backward.
 */
int
search( register int search_forward, 
	register char *pattern, 
	register int n, 
	int wantmatch)
{
	off_t pos, linepos;
	register wchar_t *p;
	register wchar_t *q;
	int linenum;
	int linematch;
	int rv;
#ifdef SILVER
	char *errmsg;
	char *re_comp();
#else		
	static regex_t regexp;
#endif
	static int compiled_re = 0;
#define MSGSIZE 80
	char msg[MSGSIZE];
	char mbs[LINE_LENGTH * MB_LEN_MAX];


	if (pattern == NULL || *pattern == '\0')
	{
		/*
		 * A null pattern means use the previous pattern.
		 */
		if (!compiled_re)
		{
			error(MSGSTR(NOREG, "No previous regular expression"));
			return(0);
		}
	} else
	{
		/*
		 * Otherwise compile the given pattern.
		 */
#ifdef SILVER
		if ((errmsg = re_comp(pattern)) != NULL) {
			error(errmsg);
#else
		if ((rv = regcomp(&regexp, pattern, 
                    (caseless ? REG_EXTENDED|REG_ICASE : REG_EXTENDED))) != 0) {
			if(regerror(rv, &regexp, msg, MSGSIZE)) {
				error(msg);
			}
#endif
			compiled_re = 0;	/* Lost it */
			return (0);
		}
		compiled_re = 1;		/* Got it */
	}

	/*
	 * Figure out where to start the search.
	 */

	if (position(TOP) == NULL_POSITION) {
		/*
		 * Nothing is currently displayed.  Start at the beginning
		 * of the file.  (This case is mainly for searches from the
		 * command line.
		 */
		pos = (off_t)0;
	} else if (!search_forward) {
		/*
		 * Backward search: start just before the top line
		 * displayed on the screen.
		 */
		pos = position(TOP);
	} else {
		/*
		 * Start at the second screen line displayed on the screen.
		 */
	/* Adjust the start line position to the actual line position
	   when we search forward to the first long line. */
	top_line = find_linenum(position(TOP));
	screen_line = TOP_PLUS_ONE;
	pos = position(screen_line);
	actual_line = find_linenum(pos);
	while( actual_line == top_line ){
		pos = position(++screen_line);
		if( find_linenum(pos))
		  actual_line = find_linenum(pos);
	}
	}
	if (pos == NULL_POSITION)
	{
		/*
		 * Can't find anyplace to start searching from.
		 */
		error(MSGSTR(NOSEARCH, "Nothing to search"));
		return(0);
	}

	linenum = find_linenum(pos);
	for (;;)
	{
		/*
		 * Get lines until we find a matching one or 
		 * until we hit end-of-file (or beginning-of-file 
		 * if we're going backwards).
		 */
		if (sigs)
			/*
			 * A signal aborts the search.
			 */
			return(0);

		if (search_forward)
		{
			/*
			 * Read the next line, and save the 
			 * starting position of that line in linepos.
			 */
			linepos = pos;
			pos = forw_raw_line(pos);
			if (linenum != 0)
				linenum++;
		} else
		{
			/*
			 * Read the previous line and save the
			 * starting position of that line in linepos.
			 */
			pos = back_raw_line(pos);
			linepos = pos;
			if (linenum != 0)
				linenum--;
		}

		if (pos == NULL_POSITION)
		{
			/*
			 * We hit EOF/BOF without a match.
			 */
			error(MSGSTR(NOPAT, "Pattern not found"));
			return(0);
		}
		/*
		 * If we're using line numbers, we might as well
		 * remember the information we have now (the position
		 * and line number of the current line).
		 */
		if (linenums)
			add_lnum(linenum, pos);

		/*
		 * Remove any backspaces along with the preceeding char.
		 * This allows us to match text which is underlined or
		 * overstruck.
		 */
		for (p = q = line;  *p;  p++, q++)
			if (q > line && *p == L'\b')
				/* Delete BS and preceeding char. */
				q -= 2;
			else
				/* Otherwise, just copy. */
				*q = *p;

		/*
		 * The line buffer is a wchar_t string, but regexec()
		 * needs a multi-byte string, so convert.
		 */
		if (wcstombs(mbs, line, sizeof mbs) == (size_t) -1) {
			perror("more");
			return(0);
		}

		/*
		 * Test the next line to see if we have a match.
		 */
#ifdef SILVER
		linematch = (re_exec(mbs) == 0);
#else
		linematch = (regexec(&regexp, mbs,  (size_t)0, NULL, 0) == 0);
#endif		
		/*
		 * We are successful if wantmatch and linematch are
		 * both true (want a match and got it),
		 * or both false (want a non-match and got it).
		 */
		if (((wantmatch && linematch) || (!wantmatch && !linematch)) &&
		      --n <= 0)
			/*
			 * Found the line.
			 */
			break;
	}
	jump_loc(linepos);
	return(1);
}
