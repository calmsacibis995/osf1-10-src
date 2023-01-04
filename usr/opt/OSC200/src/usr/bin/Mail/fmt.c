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
static char rcsid[] = "@(#)$RCSfile: fmt.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/11 15:35:00 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX fmt.c
 * 
 * FUNCTIONS: MSGSTR, Mfmt, fmt, ispref, leadin, oflush, pack, 
 *            prefix, savestr, setout, split, tabulate 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * fmt.c        5.2 (Berkeley) 6/21/85
 */

#include <stdio.h>
#include <ctype.h>
#ifdef ASIAN_I18N
#include <locale.h>
#include <wchar.h>
#endif

#include "Mail_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * fmt -- format the concatenation of input files or standard input
 * onto standard output.  Designed for use with Mail ~|
 *
 * Syntax: fmt [ -width ] [ name ... ]
 * Author: Kurt Shoens (UCB) 12/7/78
 */


#ifdef ASIAN_I18N 
/* Return TRUE if c is a valid first byte of an Asian character */
#define IS_ASIAN_CHAR(c)	((unsigned)(c) > 0x100) 
#define	NULLSTR	((wchar_t *) 0)	/* Null string pointer for lint */
#else
#define	NULLSTR	((char *) 0)	/* Null string pointer for lint */
#endif 

int	pfx;			/* Current leading blank count */
int	lineno;			/* Current input line */
int	mark;			/* Last place we saw a head line */
int	width = 72;		/* Width that we will not exceed */

char	*calloc();		/* for lint . . . */
char	*headnames[] = {"To", "Subject", "Cc", 0};

#ifdef ASIAN_I18N
int	delay=0;
wchar_t	tmp_word[BUFSIZ];
#endif
/*
 * Drive the whole formatter by managing input files.  Also,
 * cause initialization of the output stuff and flush it out
 * at the end.
 */

main(argc, argv)
	char **argv;
{
	register FILE *fi;
	register int errs = 0;
	register char *cp;
	int nofile;

#ifdef ASIAN_I18N
	setlocale(LC_ALL,"");
#endif
	catd = catopen(MF_MAIL, NL_CAT_LOCALE);

	setout();
	lineno = 1;
	mark = -10;
	if (argc < 2) {
single:
		fmt(stdin);
		oflush();
		exit(0);
	}
	nofile = 1;
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			width = atoi(cp+1);
			if (width <= 0 || width >= BUFSIZ-2) {
				fprintf(stderr, MSGSTR(BADWIDTH, "fmt:  bad width: %d\n"), width); /*MSG*/
				exit(1);
			}
			continue;
		}
		nofile = 0;
		if ((fi = fopen(cp, "r")) == NULL) {
			perror(cp);
			errs++;
			continue;
		}
		fmt(fi);
		fclose(fi);
	}
	if (nofile)
		goto single;
	oflush();
	exit(errs);
}

/*
 * Read up characters from the passed input file, forming lines,
 * doing ^H processing, expanding tabs, stripping trailing blanks,
 * and sending each line down for analysis.
 */

fmt(fi)
	FILE *fi;
{
#ifdef ASIAN_I18N
        wint_t  c;
        wchar_t linebuf[BUFSIZ], canonb[BUFSIZ];
        wchar_t *cp, *cp2 ;
        int     c_width;
        int     i;
        int     bs_count; /* Backspace count */
#else  /* ASIAN_I18N */
	char linebuf[BUFSIZ], canonb[BUFSIZ];
	register char *cp, *cp2;
	register int c;
#endif /* ASIAN_I18N */
	register int col;

#ifdef	ASIAN_I18N
	c = fgetwc(fi);
	while (c != WEOF) {
#else	/* ASIAN_I18N */
	c = getc(fi);
	while (c != EOF) {
#endif	/* ASIAN_I18N */
		
		/*
		 * Collect a line, doing ^H processing.
		 * Leave tabs for now.
		 */

		cp = linebuf;
#ifdef	ASIAN_I18N
		while (c != '\n' && c != WEOF && cp-linebuf < BUFSIZ-2) {
			if (c == '\b') {
				bs_count = 1 ;
				while ((c = fgetwc(fi)) == '\b')
					bs_count++ ;
				while (cp > linebuf) {
					cp2 = cp - 1;
					c_width = wcwidth(*cp2);
					if (bs_count >= c_width)
						bs_count -= c_width;
					else /* Finish */
						break ;
					cp = cp2;
				}
				continue ;
			}
			if ((c != '\t') && 
			    ((c < ' ') || ((c >= 0177) && (c <= 0237)) ||
			    (c == 0377))) {
				c = fgetwc(fi);	/* ignore this character and read new one */
				continue;
			}
			*cp++ = c;
			c = fgetwc(fi);
#else	/* ASIAN_I18N */
		while (c != '\n' && c != EOF && cp-linebuf < BUFSIZ-2) {
			if (c == '\b') {
				if (cp > linebuf)
					cp--;
				c = getc(fi);
				continue;
			}
			if ((c < ' ' || c >= 0177) && c != '\t') {
				c = getc(fi);	/* ignore this character and read new one */
				continue;
			}
			*cp++ = c;
			c = getc(fi);
#endif	/* ASIAN_I18N */
		}
		*cp = '\0';

		/*
		 * Toss anything remaining on the input line.
		 */

#ifdef	ASIAN_I18N
		while (c != '\n' && c != WEOF)
			c = fgetwc(fi);
#else	/* ASIAN_I18N */
		while (c != '\n' && c != EOF)
			c = getc(fi);
#endif	/* ASIAN_I18N */
		
		/*
		 * Expand tabs on the way to canonb.
		 */

		col = 0;
		cp = linebuf;
		cp2 = canonb;
#ifdef	ASIAN_I18N
		while (c = *cp++) {
			if (c != '\t') {
				col += wcwidth(c) ;
				if (cp2-canonb < BUFSIZ-2)
				{
				   /* Convert 2-byte space to spaces */
				   if (!isascii(c) && iswspace(c))
					for(i=0; i<wcwidth(c); i++)
						*cp2++ = ' ';
				    else
					   *cp2++ = c ;
				}
#else	/* ASIAN_I18N */
		while (c = *cp++) {
			if (c != '\t') {
				col++;
				if (cp2-canonb < BUFSIZ-2)
					*cp2++ = c;
#endif	/* ASIAN_I18N */
				continue;
			}
			do {
				if (cp2-canonb < BUFSIZ-2)
					*cp2++ = ' ';
				col++;
			} while ((col & 07) != 0);
		}

		/*
		 * Swipe trailing blanks from the line.
		 */

		for (cp2--; cp2 >= canonb && *cp2 == ' '; cp2--)
			;
		*++cp2 = '\0';
		prefix(canonb);
#ifdef	ASIAN_I18N
		if (c != WEOF)
			c = fgetwc(fi);
#else	/* ASIAN_I18N */
		if (c != EOF)
			c = getc(fi);
#endif	/* ASIAN_I18N */
	}
#ifdef	ASIAN_I18N
	split(NULL);
#endif	/* ASIAN_I18N */
}

/*
 * Take a line devoid of tabs and other garbage and determine its
 * blank prefix.  If the indent changes, call for a linebreak.
 * If the input line is blank, echo the blank line on the output.
 * Finally, if the line minus the prefix is a mail header, try to keep
 * it on a line by itself.
 */

prefix(line)
#ifdef	ASIAN_I18N
	wchar_t line[];
#else	/* ASIAN_I18N */
	char line[];
#endif	/* ASIAN_I18N */
{
#ifdef	ASIAN_I18N
	char	mb_line[BUFSIZ];
#endif	/* ASIAN_I18N */

	register char *cp, **hp;
	register int np, h;

#ifdef	ASIAN_I18N
	if ((wcslen(line) == 0) || (wcstombs(mb_line, line, BUFSIZ) == (size_t) -1)) {
		if (delay == 1)
		{
			pack(tmp_word);
			delay = 0;
		}
#else	/* ASIAN_I18N */
	if (strlen(line) == 0) {
#endif	/* ASIAN_I18N */
		oflush();
		putchar('\n');
		return;
	}

#ifdef	ASIAN_I18N
	for (cp = mb_line; *cp == ' '; cp++)
		;
	np = cp - mb_line;
#else	/* ASIAN_I18N */
	for (cp = line; *cp == ' '; cp++)
		;
	np = cp - line;
#endif	/* ASIAN_I18N */

	/*
	 * The following horrible expression attempts to avoid linebreaks
	 * when the indent changes due to a paragraph.
	 */

	if (np != pfx && (np > pfx || abs(pfx-np) > 8))
		oflush();
	if (h = ishead(cp))
		oflush(), mark = lineno;
	if (lineno - mark < 3 && lineno - mark > 0)
		for (hp = &headnames[0]; *hp != (char *) 0; hp++)
			if (ispref(*hp, cp)) {
				h = 1;
				oflush();
				break;
			}
	if (!h && (h = (*cp == '.')))
		oflush();
	pfx = np;
	split(cp);
	if (h)
		oflush();
	lineno++;
}

/*
 * Split up the passed line into output "words" which are
 * maximal strings of non-blanks with the blank separation
 * attached at the end.  Pass these words along to the output
 * line packer.
 */

split(line)
	char line[];
{
#ifdef	ASIAN_I18N
	int	English;
	wchar_t *cp, *cp2;
	wchar_t word[BUFSIZ];
	wchar_t	wc_line[BUFSIZ];
#else	/* ASIAN_I18N */
	register char *cp, *cp2;
	char word[BUFSIZ];
#endif	/* ASIAN_I18N */

#ifdef	ASIAN_I18N
	if (line == NULL) 
	{
		if (delay == 1)
			pack(tmp_word);
		return;
	}

	if (mbstowcs(wc_line, line, BUFSIZ) == (size_t) -1)
		return;
	cp = wc_line;
#else	/* ASIAN_I18N */
	cp = line;
#endif	/* ASIAN_I18N */

	while (*cp) {
		cp2 = word;

		/*
		 * Collect a 'word,' allowing it to contain escaped
		 * white space.
		 */
#ifdef	ASIAN_I18N
		while (*cp && *cp != ' ' && !IS_ASIAN_CHAR(*cp)) {
			English=1;
			if (*cp == '\\' && isascii(cp[1]) && isspace(cp[1]))
#else
		while (*cp && *cp != ' ') {
			if (*cp == '\\' && isspace(cp[1]))
#endif	/* ASIAN_I18N */
				*cp2++ = *cp++;
			*cp2++ = *cp++;
		}

#ifdef	ASIAN_I18N
		/* An Asian character is treated as a single word */
		if (cp2 == word && IS_ASIAN_CHAR(*cp))
		{
			*cp2++ = *cp++;
			English = 0;
		}

		if (delay == 1)
		{
			if (English == 0)
				tmp_word[wcslen(tmp_word) - 1] = '\0';
			pack(tmp_word);
			delay = 0;
		}
#endif	/* ASIAN_I18N */
		/*
		 * Guarantee a space at end of line.
		 * Two spaces after end of sentence punctuation.
		 */

		if (*cp == '\0' && English) {
			*cp2++ = ' ';
#ifdef	ASIAN_I18N
			if (cp[-1] == '.' || cp[-1] == ':' || cp[-1] == '!' ||
			    cp[-1] == '?')
				*cp2++ = ' ';
			else if (English == 0)
			{
				delay = 1;
				*cp2 = '\0';
				wcscpy(tmp_word, word);
				continue;
			}
#else	/* ASIAN_I18N */
			if (any(cp[-1], ".:!?"))
				*cp2++ = ' ';
#endif	/* ASIAN_I18N */
		}
		while (*cp == ' ')
			*cp2++ = *cp++;
		*cp2 = '\0';
		pack(word);
	}
}

/*
 * Output section.
 * Build up line images from the words passed in.  Prefix
 * each line with correct number of blanks.  The buffer "outbuf"
 * contains the current partial line image, including prefixed blanks.
 * "outp" points to the next available space therein.  When outp is NULLSTR,
 * there ain't nothing in there yet.  At the bottom of this whole mess,
 * leading tabs are reinserted.
 */

#ifdef	ASIAN_I18N
wchar_t	outbuf[BUFSIZ];			/* Sandbagged output line image */
wchar_t	*outp;				/* Pointer in above */
#else	/* ASIAN_I18N */
char	outbuf[BUFSIZ];			/* Sandbagged output line image */
char	*outp;				/* Pointer in above */
#endif	/* ASIAN_I18N */

/*
 * Initialize the output section.
 */

setout()
{
	outp = NULLSTR;
}

/*
 * Pack a word onto the output line.  If this is the beginning of
 * the line, push on the appropriately-sized string of blanks first.
 * If the word won't fit on the current line, flush and begin a new
 * line.  If the word is too long to fit all by itself on a line,
 * just give it its own and hope for the best.
 */

pack(word)
#ifdef	ASIAN_I18N
	wchar_t word[];
#else	/* ASIAN_I18N */
	char word[];
#endif	/* ASIAN_I18N */
{
#ifdef	ASIAN_I18N
	wchar_t *cp;
#else	/* ASIAN_I18N */
	register char *cp;
#endif	/* ASIAN_I18N */
	register int s, t;

	if (outp == NULLSTR)
		leadin();
#ifdef	ASIAN_I18N
	/*
	 * Add an extra space between an Asian character followed by an
	 * ASCII character
	 */
	if ((outp != NULLSTR) && (outp > outbuf) && IS_ASIAN_CHAR(outp[-1]) &&
	    isascii(word[0]))
	    *outp++ = ' ' ;
	*outp = '\0' ;
#endif	/* ASIAN_I18N */
#ifdef	ASIAN_I18N
	t = wcswidth(word, BUFSIZ);
	s = wcswidth(outbuf, BUFSIZ);
#else	/* ASIAN_I18N */
	t = strlen(word);
	s = outp-outbuf;
#endif	/* ASIAN_I18N */

	if (t+s <= width) {
		
		/*
		 * In like flint!
		 */

		for (cp = word; *cp; *outp++ = *cp++)
			;
		*outp = '\0';
		return;
	}
	if (s > pfx) {
		oflush();
		leadin();
	}
	for (cp = word; *cp; *outp++ = *cp++)
		;
	*outp = '\0';
}

/*
 * If there is anything on the current output line, send it on
 * its way.  Set outp to NULLSTR to indicate the absence of the current
 * line prefix.
 */

oflush()
{
	if (outp == NULLSTR)
		return;
	*outp = '\0';
	tabulate(outbuf);
	outp = NULLSTR;
}

/*
 * Take the passed line buffer, insert leading tabs where possible, and
 * output on standard output (finally).
 */

tabulate(line)
#ifdef	ASIAN_I18N
	wchar_t line[];
#else	/* ASIAN_I18N */
	char line[];
#endif	/* ASIAN_I18N */
{
#ifdef	ASIAN_I18N
	wchar_t *cp, *cp2;
#else	/* ASIAN_I18N */
	register char *cp, *cp2;
#endif	/* ASIAN_I18N */
	register int b, t;

	/*
	 * Toss trailing blanks in the output line.
	 */

#ifdef	ASIAN_I18N
	cp = line + wcslen(line) - 1;
#else	/* ASIAN_I18N */
	cp = line + strlen(line) - 1;
#endif	/* ASIAN_I18N */
	while (cp >= line && *cp == ' ')
		cp--;
	*++cp = '\0';
	
	/*
	 * Count the leading blank space and tabulate.
	 */

	for (cp = line; *cp == ' '; cp++)
		;
	b = cp-line;
	t = b >> 3;
	b &= 07;
	if (t > 0)
		do
#ifdef	ASIAN_I18N
			fputwc((wchar_t)('\t'), stdout); 
#else	/* ASIAN_I18N */
			putc('\t', stdout);
#endif	/* ASIAN_I18N */
		while (--t);
	if (b > 0)
		do
#ifdef	ASIAN_I18N
			fputwc((wchar_t)(' '), stdout);
#else	/* ASIAN_I18N */
			putc(' ', stdout);
#endif	/* ASIAN_I18N */
		while (--b);
	while (*cp)
#ifdef	ASIAN_I18N
		fputwc((wchar_t)(*cp++), stdout);
	fputwc((wchar_t)('\n'), stdout);
#else	/* ASIAN_I18N */
		putc(*cp++, stdout);
	putc('\n', stdout);
#endif	/* ASIAN_I18N */
}

/*
 * Initialize the output line with the appropriate number of
 * leading blanks.
 */

leadin()
{
	register int b;
#ifdef	ASIAN_I18N
	wchar_t *cp;
#else	/* ASIAN_I18N */
	register char *cp;
#endif	/* ASIAN_I18N */

	for (b = 0, cp = outbuf; b < pfx; b++)
		*cp++ = ' ';
	outp = cp;
	*outp = '\0';
}

/*
 * Save a string in dynamic space.
 * This little goodie is needed for
 * a headline detector in head.c
 */

char *
savestr(str)
	char str[];
{
	register char *top;

	top = calloc(strlen(str) + 1, 1);
	if (!top) {
		fprintf(stderr, catgets(catd, 9, 2,
					"fmt:  Ran out of memory\n"));
		catclose(catd);
		exit(1);
	}
	copy(str, top);
	return(top);
}

/*
 * Is s1 a prefix of s2??
 */

ispref(s1, s2)
	register char *s1, *s2;
{

	while (*s1++ == *s2)
		;
	return(*s1 == '\0');
}

