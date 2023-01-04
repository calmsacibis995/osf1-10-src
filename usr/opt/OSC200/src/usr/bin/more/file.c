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
static char	*sccsid = "@(#)$RCSfile: file.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1992/11/25 13:26:52 $";
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
 * file.c : file manipulation routines.
 */

#include <stdio.h>
#include <ctype.h>
#include "globals.h"
#include <limits.h>

extern int print_file_names;

copy_file()
{
	register int c;

 	if (print_file_names) {
 		if (bad_so)
 			erase (0);
 		printf("::::::::::::::\n"); 
 		printf("%s\n::::::::::::::\n", fnames[fnum]); /* 002 */
 		if (lines_to_display > ScreenLength - 4)
 			lines_to_display = ScreenLength - 4;
 	}					/*001*/
  
  	while ((c = getc(curfile)) != EOF)
  		putchar(c);
}

#define LINSIZ	512
unsigned char	Line[LINSIZ];	/* Line buffer */

/*
 * Read a single line from the current input file
 * and write it to the screen.
 */

print_line_from_file()
{
	register int nchars;
	register int prev_line; /* screen line above cursor */
	int top_line;	/* top screen line */
	int prevresid;	/* portion of final char in previous line that could not
				be printed because it would wrap (e.g., ^X) */
	int length;		/* length of current line */
	static int prev_len;	/* length of previous line */

	if (Currline == 0)
		prev_len = 1;
	else if (lineindex != NULL)
		prev_len = lineindex[Currline-1].width;
	/*
	 * If we're in the middle of a long file line, then
	 * see if the previous line ended in a control character
	 * that could only be partially displayed.
	 */
	if (screen_end == ScreenLength - 1)
		top_line = 0;
	else
		top_line = screen_end + 1;
	prevresid = Curresid;
	if (cursor_line == top_line)
		hidden_line.reschar = Curresid;

	/*
	 * Now read lines until we get to one that should be
	 * displayed.  If the ssp flag is not on, then this is
	 * simply the next line.  If ssp is on, and the previous
	 * line was zero length, then we throw away lines until
	 * we get to one that is non-zero length.
	 */
	for (;;) {
		Screen[cursor_line].fline = Currline;
		Screen[cursor_line].fragnum = Currfrag;
		Screen[cursor_line].seek_key = Ftell();
		if ((nchars = getline (&length, fold_opt)) == EOF) {
			if (clreol)
				clreos();
			return EOF;
		}
		if (!(ssp_opt && length == 0 && prev_len == 0))
			break;
	}
	prev_len = length;
	if (bad_so || (Senter && *Senter == ' ') && line_width(cursor_line) > 0)
		erase (0);
	Screen[cursor_line].reschar = Curresid;
	prline (Line, length, nchars, prevresid);
	return 0;
}

/*
 * Get a logical line, returning the number of screen columns
 * that will be occupied when the line is displayed.  If "fold"
 * is 0, then getline ignores the screen width and reads an entire
 * line from the input file.
 */

static int hiwater;
static int highest_line;

int
getline(lengthp, fold)
int *lengthp;
int fold;
{
	register int	c;
	register unsigned char	*p;
	register int	column;
	int		linlen;
	static int	prev_len;	/* length of previous line */

	highest_line = Currline;
	linlen = line_width(cursor_line);
	p = Line;
	column = 0;
	c = Getc();
	if (Currline == 0)
		prev_len = 1;
	else if (lineindex != NULL)
		prev_len = lineindex[Currline-1].width;
	while (p < &Line[LINSIZ - 1]) {
		if (c == EOF) {
			if (p > Line) {
				*p = '\0';
				*lengthp = p - Line;
				return (column);
			}
			*lengthp = p - Line;
			Curresid = Currfrag = 0;
			return (EOF);
		}
		if (c == '\n') {
			if (p == Line && prev_len <= 0 && ssp_opt)
				record_line_width(Currline, -1);
			else
				incr_line_width(Currline, column);
			Currline++;
			Curresid = Currfrag = 0;
			record_line_index(Currline);
			break;
		}
#ifdef whatfor
		if (c & 0200) {
			if (c == ('?' | 0200))
				Ungetc(RUBOUT);
			else
				Ungetc(c & 037);
			Getc();
			c &= 0177;
		}
#endif
		*p++ = c;
		if (c == '\t' && !show_all_opt) {
			/*
			 * Some terminals don't erase what
			 * they tab over, so we'd better clear
			 * to end of line in this case.
			 */
			 /* THIS SHOULDN'T BE DONE HERE */
			if (hardtabs && column < linlen && !hard) {
	/* Look at tahoe code -- it's different */
				if (EraseLineStr && !dumb) {
					column = 1 + (column | 7);
					tputs (EraseLineStr, 1, putch);
					line_width(cursor_line) = linlen = 0;
				}
				else {
					--p;
					do {
						*p++ = ' ';
					} while (++column & 7 && p < &Line[LINSIZ - 1]);
					if (column >= linlen)
						line_width(cursor_line) = linlen = 0;
				}
			}
			else
				column = 1 + (column | 7);
		}
		else if (c == '\b' && !show_all_opt) {
			if (column != 0)
			column--;
		}
		else if (c == '\r' && !show_all_opt)
			column = 0;
		else if (c == EOF) {
			*lengthp = p - Line;
			Curresid = Currfrag = 0;
			return (column);
		}
		else if (isprint(c))
			column++;
		else if (show_opt && !isprint(c) && !iscntrl(c)) {
			if (column == 0 && Curresid) {
				column = Curresid;
				Curresid = 0;
			}
			else
				column += 3;		/* M-X */
			c = toascii(c);
			if (iscntrl(c))
				column++;	/* M-^X */
		}
		else if ((show_opt || (c == '\f' && stop_opt)) && iscntrl(c)) {
			if (column == 0 && Curresid) {
				column = Curresid;
				Curresid = 0;
			}
			else
				column += 2;		/* ^X */
			if (c == '\f' && stop_opt)
				Pause++;
		}
		if (column >= ScreenWidth && fold) break;
		c = Getc();
	}
	if (column > ScreenWidth && fold) {
		/*
		 * Mark that this line has character whose print
		 * rendering straddles two screen lines
		 */
		Curresid = column - ScreenWidth;
		column = ScreenWidth;
		Ungetc(c);	/* Put back character which can only be */
				/* partially printed this time around */
	}
	if (c != '\n')
		incr_line_width(Currline, column);
	if (fold && column >= ScreenWidth && ScreenWidth > 0 && !Wrap)
		*p++ = '\n';
	if (fold && column == ScreenWidth && c != '\n') {
		/* See whether this line really wraps */
		c = Getc();
		if (!Curresid && c == '\n') {
			/*
			 * No, the line went to the last screen
			 * column.  Note that we discard the newline
			 * character.
			 */
			Currline++;
			Curresid = Currfrag = 0;
			record_line_index(Currline);
		}
		else {
			Currfrag++;
			Ungetc(c);
		}
	}
	prev_len = *lengthp = p - Line;
	*p = 0;
	return (column);
}

/*
 * Allocate space for the table that maps file line numbers
 * to seek positions within the file.  Each time we need to
 * grow the table, we allocate NINDEX more entries.
 */
#define NINDEX 10000
static off_t currsize;

grow_line_index(incr)
int incr;
{

	if (lineindex == NULL) {	/* Has never been allocated */
		lineindex = (line_rec *)malloc(NINDEX * sizeof(line_rec));
		currsize = NINDEX;
		if (lineindex != NULL) {
			lineindex[0].seek_key = 0;
			lineindex[0].width = 0;
		}
	}
	else {
		if (incr < NINDEX)
			incr = NINDEX;
		currsize += incr;
		lineindex = (line_rec *)realloc(lineindex, currsize*sizeof(line_rec));
	}
}

init_line_index()
{
	if (lineindex == NULL)
		grow_line_index(0);
	else
		hiwater = 0;
	lineindex[0].width = 0;
	lineindex[0].seek_key = 0;
}

record_line_index(lineno)
int lineno;
{
	if (lineindex == NULL || lineno <= hiwater)
		return;
	if (lineno >= currsize)
		grow_line_index(0);
	if (lineindex != NULL) {
		lineindex[lineno].seek_key = Ftell();
		lineindex[lineno].width = 0;
		hiwater = lineno;
	}
}

record_line_width(lineno, cols)
int lineno;
int cols;
{
	if (lineindex != NULL && lineno >= hiwater)
		lineindex[lineno].width = cols;
}

incr_line_width(lineno, cols)
int lineno;
int cols;
{
	if (lineindex != NULL && lineno >= hiwater)
		lineindex[lineno].width += cols;
}

/*
 * Go to line N in the file.  If we have already read all
 * the lines up to the destination, then this is a simple
 * seek.  Otherwise we need to use getline() to get lines
 * from the file, so that the seek index and width of each
 * line will be properly recorded.
 */

goto_line(lineno)
int lineno;
{
	int length;
	int origpos = Ftell();
	int origline = Currline;
	register int i, nlines;
	extern int force_wait;

	if (lineno <= highest_line) {
		if (ispipe)
			return -1;
		Currline = lineno;
		Curresid = Currfrag = 0;
		Fseek(lineindex[lineno].seek_key);
		return 0;
	}
	Curresid = Currfrag = 0;
	if (!ispipe) {
		Fseek(lineindex[highest_line].seek_key);
		Currline = highest_line;
	}
	while (Currline < lineno) {
		/*
		 * If we got to the end of the file,
		 * back up so that the last screenful
		 * of the file will be displayed.
		 */
		if (getline(&length, 0) == EOF) {
			if (ispipe)
				return 0;
			i = 0;
			lineno = Currline;
			while (i < lines_to_display) {
				if (--lineno < 0)
					break;
				if (lineindex[lineno].width <= ScreenWidth)
					nlines = 1;
				else
					nlines = (lineindex[lineno].width-1)/ScreenWidth + 1;
				i += nlines;
			}
			/*
			 * Get rid of any line fragments that shouldn't
			 * be displayed.
			 */
			Fseek(lineindex[lineno].seek_key);
			Currline = lineno;
			Curresid = Currfrag = 0;
			i -= lines_to_display;
			while (i-- > 0)
				(void)getline(&length, 1);
			break;
		}
	}
	force_wait = 1;
	return 0;
}

/*
 * Skip n lines in the file f
 */

skiplns (n)
register int n;
{
	register int endline = Currline + n;
	int length;
	extern int wait_opt;

	while (Currline < endline)
		if (getline(&length, 0) == EOF) {
			if (wait_opt)
				goto_line(endline);
			return;
		}
}

/*
 * Skip nskip files in the file list (from the command line). Nskip may be
 * negative.
 */

skipf (nskip)
register int nskip;
{
	if (nskip == 0) return;
	if (nskip > 0) {
		if (fnum + nskip > nfiles - 1)
			nskip = nfiles - fnum - 1;
	}
	else if (Currline != 0)
		++fnum;
	fnum += nskip;
	if (fnum < 0)
		fnum = 0;
	printf (MSGSTR(FILESKIP, "\n...Skipping %sto file %s\n\n"), nskip > 0 ? "" : MSGSTR(BACK3, "back "), fnames[(curfile == stdin || ispipe) ? 0 : fnum]); /* paw: qar 1790, 1791 */

	if ((curfile == stdin || ispipe) && nfiles <= 1) /* paw: qar 1789 */
	  fnum=0;
	else
	  --fnum;
}



