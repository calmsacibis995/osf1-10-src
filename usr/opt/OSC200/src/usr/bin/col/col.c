
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
static char rcsid[] = "@(#)$RCSfile: col.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/10/11 16:35:48 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Rendell of the Memorial University of Newfoundland.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * col.c	5.2 (Berkeley) 5/24/90
 */

#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "col_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_COL, Num, Str)

nl_catd catd;

#define	BS	'\b'		/* backspace */
#define	TAB	'\t'		/* tab */
#define	SPACE	' '		/* space */
#define	NL	'\n'		/* newline */
#define	CR	'\r'		/* carriage return */
#define	ESC	'\033'		/* escape */
#define	SI	'\017'		/* shift in to normal character set */
#define	SO	'\016'		/* shift out to alternate character set */
#define	VT	'\013'		/* vertical tab (aka reverse line feed) */
#define	RLF	'7'		/* ESC-7 reverse line feed */
#define	RHLF	'8'		/* ESC-8 reverse half-line feed */
#define	FHLF	'9'		/* ESC-9 forward half-line feed */

/* build up at least this many lines before flushing them out */
#define	BUFFER_MARGIN		32

typedef short CSET;

typedef struct char_str {
#define	CS_NORMAL	1
#define	CS_ALTERNATE	2
	short		c_column;	/* column character is in */
	short		c_width;	/* character width */
	CSET		c_set;		/* character set (currently only 2) */
	wchar_t		c_char;		/* character in question */
} CHAR;

typedef struct line_str LINE;
struct line_str {
	CHAR	*l_line;		/* characters on the line */
	LINE	*l_prev;		/* previous line */
	LINE	*l_next;		/* next line */
	int	l_lsize;		/* allocated sizeof l_line */
	int	l_line_len;		/* strlen(l_line) */
	int	l_needs_sort;		/* set if chars went in out of order */
	int	l_max_col;		/* max column in the line */
};

LINE *alloc_line();
void *xmalloc();

CSET last_set;			/* char_set of last char printed */
LINE *lines;
int compress_spaces;		/* if doing space -> tab conversion */
int fine;			/* if `fine' resolution (half lines) */
int max_bufd_lines;		/* max # lines to keep in memory */
int nblank_lines;		/* # blanks after last flushed line */
int no_backspaces;		/* if not to output any backspaces */
int pass_escapes;		/* if passing through all escapes */

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	extern char *optarg;
	register wchar_t ch;
	CHAR *c;
	CSET cur_set;			/* current character set */
	LINE *l;			/* current line */
	int extra_lines;		/* # of lines above first line */
	int cur_col;			/* current column */
	int cur_line;			/* line number of current position */
	int max_line;			/* max value of cur_line */
	int this_line;			/* line l points to */
	int nflushd_lines;		/* number of lines that were flushed */
	int adjust, opt, warned;
	int e;

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_COL, NL_CAT_LOCALE);

	max_bufd_lines = 128;
	compress_spaces = 1;		/* compress spaces into tabs */
	while ((opt = getopt(argc, argv, "bfhl:px")) != EOF)
		switch (opt) {
		case 'b':		/* do not output backspaces */
			no_backspaces = 1;
			break;
		case 'f':		/* allow half forward line feeds */
			fine = 1;
			break;
		case 'h':		/* compress spaces into tabs */
			compress_spaces = 1;
			break;
		case 'l':		/* buffered line count */
			if ((max_bufd_lines = atoi(optarg)) <= 0) {
				(void)fprintf(stderr,
				    MSGSTR(BADL, "col: bad -l argument %s.\n"),
				    optarg);
				exit(1);
			}
			break;
		case 'p':		/* pass through unknown escapes */
			pass_escapes = 1;
			break;
		case 'x':		/* do not compress spaces into tabs */
			compress_spaces = 0;
			break;
		case '?':
		default:
			usage();
		}

	if (optind != argc)
		usage();

	/* this value is in half lines */
	max_bufd_lines *= 2;

	adjust = cur_col = extra_lines = warned = 0;
	cur_line = max_line = nflushd_lines = this_line = 0;
	cur_set = last_set = CS_NORMAL;
	lines = l = alloc_line();

	while ((ch = getwchar()) != WEOF) {
		if (!iswgraph(ch)) {
			switch (ch) {
			case BS:		/* can't go back further */
				if (cur_col == 0)
					continue;
				--cur_col;
				continue;
			case CR:
				cur_col = 0;
				continue;
			case ESC:		/* just ignore EOF */
				switch(e = getwchar()) {
				case RLF:
					cur_line -= 2;
					continue;
				case RHLF:
					cur_line--;
					continue;
				case FHLF:
					cur_line++;
					if (cur_line > max_line)
						max_line = cur_line;
					continue;
				default:
					if (pass_escapes) {
						ungetwc(e, stdin);
						break;
					}
					else
						continue;
				}
				break;
			case NL:
				cur_line += 2;
				if (cur_line > max_line)
					max_line = cur_line;
				cur_col = 0;
				continue;
			case SPACE:
				++cur_col;
				continue;
			case SI:
				cur_set = CS_NORMAL;
				continue;
			case SO:
				cur_set = CS_ALTERNATE;
				continue;
			case TAB:		/* adjust column */
				cur_col |= 7;
				++cur_col;
				continue;
			case VT:
				cur_line -= 2;
				continue;
			default:
				continue;
			}
		}

		/* Must stuff ch in a line - are we at the right one? */
		if (cur_line != this_line - adjust) {
			LINE *lnew;
			int nmove;

			adjust = 0;
			nmove = cur_line - this_line;
			if (!fine) {
				/* round up to next line */
				if (cur_line & 1) {
					adjust = 1;
					nmove++;
				}
			}
			if (nmove < 0) {
				for (; nmove < 0 && l->l_prev; nmove++)
					l = l->l_prev;
				if (nmove) {
					if (nflushd_lines == 0) {
						/*
						 * Allow backup past first
						 * line if nothing has been
						 * flushed yet.
						 */
						for (; nmove < 0; nmove++) {
							lnew = alloc_line();
							l->l_prev = lnew;
							lnew->l_next = l;
							l = lines = lnew;
							extra_lines++;
						}
					} else {
						if (!warned++)
							warn(cur_line);
						cur_line -= nmove;
					}
				}
			} else {
				/* may need to allocate here */
				for (; nmove > 0 && l->l_next; nmove--)
					l = l->l_next;
				for (; nmove > 0; nmove--) {
					lnew = alloc_line();
					lnew->l_prev = l;
					l->l_next = lnew;
					l = lnew;
				}
			}
			this_line = cur_line + adjust;
			nmove = this_line - nflushd_lines;
			if (nmove >= max_bufd_lines + BUFFER_MARGIN) {
				nflushd_lines += nmove - max_bufd_lines;
				flush_lines(nmove - max_bufd_lines);
			}
		}
		/* grow line's buffer? */
		if (l->l_line_len + 1 >= l->l_lsize) {
			int need;

			need = l->l_lsize ? l->l_lsize * 2 : 90;
			l->l_line = (CHAR *)xmalloc((void *) l->l_line,
			    (unsigned) need * sizeof(CHAR));
			l->l_lsize = need;
		}
		c = &l->l_line[l->l_line_len++];
		c->c_char = ch;
		c->c_set = cur_set;
		c->c_column = cur_col;
		c->c_width = wcwidth(ch);
		/*
		 * If things are put in out of order, they will need sorting
		 * when it is flushed.
		 */
		if (cur_col < l->l_max_col)
			l->l_needs_sort = 1;
		else
			l->l_max_col = cur_col + c->c_width ;
		cur_col += c->c_width ;
	}
	/* goto the last line that had a character on it */
	for (; l->l_next; l = l->l_next)
		this_line++;
	flush_lines(this_line - nflushd_lines + extra_lines + 1);

	/* make sure we leave things in a sane state */
	if (last_set != CS_NORMAL)
		PUTC('\017');

	/* flush out the last few blank lines */
	nblank_lines = max_line - this_line;
	if (max_line & 1)
		nblank_lines++;
	else if (!nblank_lines)
		/* missing a \n on the last line? */
		nblank_lines = 2;
	flush_blanks();
	exit(0);
}

flush_lines(nflush)
	int nflush;
{
	LINE *l;

	while (--nflush >= 0) {
		l = lines;
		lines = l->l_next;
		if (l->l_line) {
			flush_blanks();
			flush_line(l);
		}
		nblank_lines++;
		if (l->l_line)
			(void)free((void *)l->l_line);
		free_line(l);
	}
	if (lines)
		lines->l_prev = NULL;
}

/*
 * Print a number of newline/half newlines.  If fine flag is set, nblank_lines
 * is the number of half line feeds, otherwise it is the number of whole line
 * feeds.
 */
flush_blanks()
{
	int half, i, nb;

	half = 0;
	nb = nblank_lines;
	if (nb & 1) {
		if (fine)
			half = 1;
		else
			nb++;
	}
	nb /= 2;
	for (i = nb; --i >= 0;)
		PUTC('\n');
	if (half) {
		PUTC('\033');
		PUTC('9');
		if (!nb)
			PUTC('\r');
	}
	nblank_lines = 0;
}

/*
 * Write a line to stdout taking care of space to tab conversion (-h flag)
 * and character set shifts. In order to process overlapping multibyte 
 * characters, an array colp[] is defined to contain the relative column
 * position of a multibyte character from its beginning. Therefore,
 *	0 => This position should not contain a character or a partial
 *	     character overlapped by the others.
 *	1 => First column position of a multibyte character.
 *	2 => Second column position of a multibyte character.
 *	  :
 */
flush_line(l)
	LINE *l;
{
	CHAR *c, *endc;
	int nchars, last_col, this_col;
	static int colp_size, *colp ;
	wchar_t	last_ch ;
	int	i, width, maxwidth ;

	last_col = 0;
	nchars = l->l_line_len;

	if (l->l_needs_sort) {
		static CHAR *sorted;
		static int count_size, *count, save, sorted_size, tot;

		/*
		 * Do an O(n) sort on l->l_line by column being careful to
		 * preserve the order of characters in the same column.
		 */
		if (l->l_lsize > sorted_size) {
			sorted_size = l->l_lsize;
			sorted = (CHAR *)xmalloc((void *)sorted,
			    (unsigned)sizeof(CHAR) * sorted_size);
		}
		if (l->l_max_col >= count_size) {
			count_size = l->l_max_col + 1;
			colp_size  = l->l_max_col + 1;
			count = (int *)xmalloc((void *)count,
			    (unsigned)sizeof(int) * count_size);
			colp  = (int *)xmalloc((void *)colp,
			    (unsigned)sizeof(int) * colp_size);
		}
		bzero((char *)count, sizeof(int) * l->l_max_col + 1);
		bzero((char *)colp , sizeof(int) * l->l_max_col + 1);
		for (i = nchars, c = l->l_line; --i >= 0; c++) {
			count[c->c_column]++;
			save = c->c_column   ;
			/*
			 * If current column is not 0 or 1, clear the previous
			 * columns until a 1 is found
			 */
			if (colp[save] > 1) {
				while (colp[--save] > 1)
					colp[save] = 0 ;
				colp[save] = 0 ;
				save = c->c_column;
			}
			/*
			 * Set column positions for current character
			 */
			for (tot = 1 ; tot <= c->c_width ; tot++)
				colp[save++] = tot ;
			/*
			 * If next column is not 0 or 1, clear them until a
			 * 0 or 1
			 */
			if ((colp[save] > 1) && (save < l->l_max_col)) {
				do {
					colp[save++] = 0 ;
				} while (colp[save] > 1) ;
			}
		}

		/*
		 * calculate running total (shifted down by 1) to use as
		 * indices into new line.
		 */
		for (tot = 0, i = 0; i <= l->l_max_col; i++) {
			save = count[i];
			count[i] = tot;
			tot += save;
		}

		for (i = nchars, c = l->l_line; --i >= 0; c++)
			sorted[count[c->c_column]++] = *c;
		c = sorted;
	} else
		c = l->l_line;
	while (nchars > 0) {
		this_col = c->c_column;
		endc = c;
		do {
			++endc;
		} while (--nchars > 0 && this_col == endc->c_column);

		/* if -b only print last character */
		if (no_backspaces) {
			/*
			 * Checking of colp[] should be guarded by
			 * l->l_needs_sort in order to prevent access
			 * violation.
			 */
			if (l->l_needs_sort && (colp[this_col] != 1)) {
				/* Skip current column */
				c = endc ;
				continue ;
			}
			c = endc - 1;
		}

		if (this_col > last_col) {
			int nspace = this_col - last_col;

			if (compress_spaces && nspace > 1) {
				int ntabs;

				ntabs = this_col / 8 - last_col / 8;
				if (ntabs > 0) nspace = this_col % 8;
				while (--ntabs >= 0)
					PUTC('\t');
			}
			while (--nspace >= 0)
				PUTC(' ');
			last_col = this_col;
		} else while (this_col < last_col) {
			PUTC('\b') ;
			last_col-- ;
		}

		maxwidth = 1 ;
		for (;;) {
			if (c->c_set != last_set) {
				switch (c->c_set) {
				case CS_NORMAL:
					PUTC('\017');
					break;
				case CS_ALTERNATE:
					PUTC('\016');
				}
				last_set = c->c_set;
			}
			PUTC(c->c_char);
			/*
			 * Save current width and maximum width for later
			 * processing.
			 */
			width = c->c_width ;
			if (width > maxwidth)
				maxwidth = width ;
			if (++c >= endc)
				break;
			while (--width >= 0)
				PUTC('\b');
		}
		/*
		 * This is the last character to be shown at this column
		 */
		if (!l->l_needs_sort || (colp[this_col] == 1)) {
			last_ch   = c[-1].c_char ;
			last_col += width	 ;
			/*
			 * Clear extra text beyond the last character
			 */
			if (maxwidth > width) {
				for (i = maxwidth - width ; i > 0 ; i--)
					PUTC(' ') ;
				for (i = maxwidth - width ; i > 0 ; i--)
					PUTC('\b') ;
			}
		/*
		 * No character should be displayed at this column,
		 * so clear it with spaces.
		 */
		} else if (colp[this_col] == 0) {
			for (i = width ; i > 0 ; i--)
				PUTC('\b') ;
			for (i = maxwidth ; i > 0 ; i--)
				PUTC(' ') ;
			last_col += maxwidth ;
		/*
		 * The last character in this column is not on the top, so
		 * reprints the correct character in a previous column on top.
		 */
		} else {
			for (i = colp[this_col] ; i > 0 ; i--)
				PUTC('\b') ;
			PUTC(last_ch) ;
			for (last_col = this_col + 1 ; colp[last_col] > 1 ;
			     last_col++)
				continue ;
		}
	}
}

#define	NALLOC 64

static LINE *line_freelist;

LINE *
alloc_line()
{
	LINE *l;
	int i;

	if (!line_freelist) {
		l = (LINE *)xmalloc((void *)NULL, sizeof(LINE) * NALLOC);
		line_freelist = l;
		for (i = 1; i < NALLOC; i++, l++)
			l->l_next = l + 1;
		l->l_next = NULL;
	}
	l = line_freelist;
	line_freelist = l->l_next;

	bzero((char *)l, sizeof(LINE));
	return(l);
}

free_line(l)
	LINE *l;
{
	l->l_next = line_freelist;
	line_freelist = l;
}

void *
xmalloc(p, size)
	void *p;
	size_t size;
{
	if (!(p = (void *)realloc(p, size))) {
		(void)fprintf(stderr, "col: %s.\n", strerror(ENOMEM));
		exit(1);
	}
	return(p);
}

usage()
{
	(void)fprintf(stderr, MSGSTR(USAGE,"usage: col [-bfhpx] [-l nline]\n"));
	exit(1);
}

wrerr()
{
	(void)fprintf(stderr, MSGSTR(WRERR, "col: write error.\n"));
	exit(1);
}

warn(line)
	int line;
{
	if (line < 0)
		(void)fprintf(stderr, MSGSTR(BACK1,
		    "col: warning: can't back up past first line.\n"));
	else
		(void)fprintf(stderr, MSGSTR(BACK2,
		    "col: warning: can't back up -- line already flushed.\n"));
}

PUTC(ch)
	wchar_t ch ;
{
	if (putwchar(ch) == WEOF)
		wrerr();
}
