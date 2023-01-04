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
static char	*sccsid = "@(#)$RCSfile: lplf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:12:48 $";
#endif 
/*
 */
/*
 * Copyright (c) 1989-1990 SecureWare, Inc.  All Rights Reserved.
 *
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 *
 * OSF/1 Release 1.0
 */



/*

 */

#include <stdio.h>
#include <values.h>

#include <sys/security.h>
#include <mandatory.h>

#define	CHAR_SET_SIZE	(1 << BITSPERBYTE)
#define	CHRMAP_SIZE	(WORD_OF_BIT(CHAR_SET_SIZE) + 1)
#define	ESCMAP_SIZE	(WORD_OF_BIT(CHAR_SET_SIZE) + 1)
#define	DEFWIDTH	80
#define	DEFLENGTH	66
#define	DEFFF		"\f"

char	*Usage1 =
	"usage: lplf -P printer [-c] [-w width] [-l length] [-i indent]\n";
char	*Usage2 =
	"	    [-n user] [-h host] [-V file]\n";

int	literal;		/* don't filter control characters */
int	linelen = -1;		/* characters per line */
int	pagelen = -1;		/* lines per page */
int	indent;			/* columns to indent output */
int	truncline;		/* printer truncates long lines */
char	*printer;		/* name of printer */
char	*user;			/* user name for accounting */
char	*host;			/* host name for accounting */
char	*initseq;		/* character sequence to print first */
char	*termseq;		/* character sequence to print last */
char	*emphstart;		/* character sequence to begin emphasis */
char	*emphstop;		/* character sequence to end emphasis */
char	*formfeed;		/* character sequence to eject page */
char	*acctfile;		/* accounting file */
char	*command_name;

mask_t	chrmap[CHRMAP_SIZE];
mask_t	escmap[ESCMAP_SIZE];

char	*format_label();

extern char	*optarg;	/* arg pointer for getopt */
extern int	optind;		/* option index for getopt */
extern int	opterr;

extern char	*malloc(), *strchr(), *strrchr();

main(argc, argv)
	int argc;
	char *argv[];
{
	char		*file = (char *) 0, *label = (char *) 0;
	char		line[BUFSIZ];
	FILE		*fp;
	register int	eject, lineno, pages;
	int		ret, option, error = 0;
#if SEC_CMW || SEC_SHW
	ilb_ir_t	*sec_level_ir;
#else
	mand_ir_t	*sec_level_ir;
#endif

	command_name = argv[0];

#if SEC_MAC
	mand_init();
#endif

	opterr = 0;
	while ((option = getopt(argc, argv, "cP:w:l:x:y:i:n:h:V:")) != EOF)
		switch (option) {
		    case 'P':
			printer = optarg;
			break;

		    case 'c':
			literal = 1;
			break;
		
		    case 'w':
			linelen = atoi(optarg);
			break;
		
		    case 'l':
			pagelen = atoi(optarg);
			break;

		    case 'i':
			indent = atoi(optarg);
			break;
		
		    case 'n':
			user = optarg;
			break;

		    case 'h':
			host = optarg;
			break;

		    case 'V':
			file = optarg;
			break;
		
		    case 'x':
		    case 'y':
			/* Ignore these arguments */
			break;

		    case '?':
		    default:
			error = 1;
			break;
		}

	if (error || printer == (char *) 0) {
		fprintf(stderr, "%s%s", Usage1, Usage2);
		exit(1);
	}

	/*
	 * Get the characteristics for the printer.  These include
	 * the line length, page length, any special escape/control
	 * sequences, and bold/normal sequences for the label.
	 */

	chkprinter(printer);

	/*
	 * If a label file is provided, retrieve the label and format it
	 * for output.  If the label cannot be gotten, either because it
	 * is a WILDCARD or because of an error, assume system high.
	 */

	if (file) {
#if SEC_CMW || SEC_SHW
		sec_level_ir = ilb_alloc_ir();
#else
		sec_level_ir = mand_alloc_ir();
#endif
		if (sec_level_ir == 0) {
			fprintf(stderr, "%s: ran out of space\n", command_name);
			exit(1);
		}

#if SEC_CMW || SEC_SHW
		ret = statilabel(file, sec_level_ir);
		if (ret != -1)
			label = format_label(ilb_ir_to_er(sec_level_ir));
		else
			label = format_label(ilb_ir_to_er(mand_syshi));
		ilb_free_ir(sec_level_ir);
#else
		ret = statslabel(file, sec_level_ir);
		if (ret != -1)
			label = format_label(mand_ir_to_er(sec_level_ir));
		else
			label = format_label(mand_ir_to_er(mand_syshi));
		mand_free_ir(sec_level_ir);
#endif
	}

	pages = 0;
	lineno = 0;
	line[0] = '\0';

	/*
	 * The main loop.  Each pass prints a single line.  Large lines
	 * are split (or truncated) and may take multiple passes through
	 * this loop.  Continuation lines are recognized by having a
	 * a non-null character at the start of the line buffer.
	 */

	while (line[0] || fgets(line, sizeof line, stdin)) {

		if (label) {

			/*
			 * If at bottom of page, print bottom label.
			 */
			if (lineno == pagelen - 1) {
				printf("%s\r%s", label, formfeed);
				lineno = 0;
				++pages;
			}

			/*
			 * If at top of page, print top label.
			 */
			if (lineno == 0) {
				printf("%s\n", label);
				lineno++;
			}

			/*
			 * If the printer truncates lines, do so here also
			 * to 1) reduce output, and 2) properly account for
			 * line length in page length calculations below.
			 */
			if (truncline && line[linelen-indent-1] != '\n') {
				line[linelen - indent] = '\n';
				line[linelen - indent + 1] = '\0';
			}
		}

		eject = output(line, stdout);
		lineno = (lineno + 1) % pagelen;

		if (eject && lineno)
			if (label) {
				while (lineno < pagelen - 1) {
					putchar('\n');
					++lineno;
				}
			} else {
				fputs(formfeed, stdout);
				lineno = 0;
			}

		if (lineno == 0)
			++pages;
	}

	/*
	 * Fill out the final page.
	 */

	if (lineno) {
		if (label) {
			while (lineno < pagelen-1) {
				putchar('\n');
				++lineno;
			}
			printf("%s\r", label);
		}
		fputs(formfeed, stdout);
		++pages;
	}

	if (acctfile && host && user && (fp = fopen(acctfile, "a"))) {
		fprintf(fp, "%7.2f\t%s:%s\n", (float) pages, host, user);
		fclose(fp);
	}

	exit(0);
}


/*
 * Initialize the printer so that we know the line spacing and other
 * relevant modes.
 */
initialize(file)
	FILE	*file;
{
	if (!literal && initseq)
		fputs(initseq, file);
}


/*
 * Terminate use of the printer.
 */
finish(file)
	FILE	*file;
{
	if (!literal && termseq)
		fputs(termseq, file);
}


/*
 * Output a line of text, filtering out control codes and escape sequences
 * that would upset the line counting.  Returns 1 if line contains a formfeed,
 * otherwise returns 0.
 */
output(line, file)
	char	*line;
	FILE	*file;
{
	register char	*cp;
	register int	doneindent, count, eject, wasescape;

	if (literal) {
		cp = strchr(line, '\f');
		if (cp) {
			*cp++ = '\0';
			if (indent)
				fprintf(file, "%*s", indent, "");
			fputs(line, file);
			fputc('\n', file);
			if (*cp)
				strcpy(line, cp);
			else
				line[0] = '\0';
			return 1;
		}
		if (indent)
			fprintf(file, "%*s", indent, "");
		fputs(line, file);
		line[0] = '\0';
		return 0;
	}

	cp = line;
	count = eject = wasescape = doneindent = 0;

	/*
	 * Each pass through this loop prints a character.  Some characters
	 * may be filtered here.   If filtered, a <SPACE> is output instead.
	 */
	for (; count < linelen - indent && *cp && *cp != '\n' && !eject; ++cp) {

		if (indent && count == 0 && !doneindent)
			printf("%*s", indent, "");

		if (wasescape) {
			/*
			 * Previous character was an unfiltered ESC.
			 */
			if (ISBITSET(escmap, *cp)) {
				fputc(' ', file);
				++count;
			} else {
				fputc('\033', file);
				fputc(*cp, file);
			}
			wasescape = 0;
			continue;
		}

		switch (*cp)  {
		case '\033':
			if (ISBITSET(chrmap, *cp)) {
				fputc(' ', file);
				++count;
			} else
				wasescape = 1;
			break;
		case '\r':
			fputc(*cp, file);
			count = doneindent = 0;
			break;
		case '\b':
			if (count > 0) {
				fputc(*cp, file);
				--count;
			}
			break;
		case '\t':
			do {
				fputc(' ', file);
				++count;
			} while (count % 8);
			break;
		case '\f':
			eject = 1;
			break;
		default:
			if (ISBITSET(chrmap, *cp))
				fputc(' ', file);
			else
				fputc(*cp, file);
			++count;
			break;
		}
	}

	fputc('\n', file);
	if (*cp == '\n')
		++cp;

	/*
	 * If any characters remain in the buffer, shift them down to
	 * be processed by the next call to this function.
	 */

	if (*cp)
		strcpy(line, cp);
	else
		line[0] = '\0';

	return eject;
}


/*
 * Format the label string to center it on the page and add emphasis.
 */
char *
format_label(label)
	char *label;
{
	register char	*buf;
	register int	buflen, spaces;

	buflen = strlen(label);
	if (buflen > linelen)	/* truncate label if it exceeds line length */
		buflen = linelen;

	/*
	 * Compute the number of blanks to prepend to the label to
	 * center it over the text on the page, taking the indentation
	 * value into account, but avoid indenting the label to the
	 * point of overruning the line length.
	 */
	if (buflen > linelen - indent)
		spaces = linelen - buflen;
	else
		spaces = indent + (linelen - indent - buflen) / 2;

	buflen += spaces + 1;	/* account for null byte at end of buf */
	if (emphstart && emphstop)
		buflen += strlen(emphstart) + strlen(emphstop);
	else
		emphstart = emphstop = "";
	
	buf = malloc(buflen);
	if (buf == (char *) 0) {
		fprintf(stderr, "%s: ran out of space\n", command_name);
		exit(1);
	}

	sprintf(buf, "%*s%s%.*s%s", spaces, "", emphstart, linelen,
		label, emphstop);

	return buf;
}


/*
 * Determine printable characters.  This handles detection of the start of
 * escape sequences, and control characters that could alter line/page
 * positioning that would throw off or cover the security level stamp.
 */
lp_char_map(ctlchars, ctllen, escchars, esclen)
	unsigned char	*ctlchars, *escchars;
	int		ctllen, esclen;
{
	register int	scan;

	/*
	 * By default, all characters are allowed unless the strings are
	 * not in the database (in which case, no characters are allowed).
	 */
	memset(chrmap, '\0', sizeof chrmap);
	memset(escmap, '\0', sizeof escmap);
 
	/*
	 * Collect characters never to print.
	 */
	if (ctlchars)
		for (scan = 0; scan < ctllen; ++scan)
			ADDBIT(chrmap, ctlchars[scan]);

	/*
	 * Collect characters after <ESC> never to print.
	 */
	if (escchars)
		for (scan = 0; scan < esclen; ++scan)
			ADDBIT(escmap, escchars[scan]);
}

chkprinter(printer)
	char	*printer;
{
	static char	pcap[BUFSIZ], strbuf[BUFSIZ];
	int		status, ctllen, esclen;
	char		*ctlchars, *escchars, *bp = strbuf;
	extern char	*pgetstr();

	if ((status = pgetent(pcap, printer)) < 0) {
		fprintf(stderr, "%s: cannot open printer description file\n",
			command_name);
		exit(1);
	}
	if (status == 0) {
		fprintf(stderr, "%s: printer %s is not defined\n",
			command_name, printer);
		exit(1);
	}
	if (linelen == -1 && (linelen = pgetnum("pw")) < 0)
		linelen = DEFWIDTH;
	if (pagelen == -1 && (pagelen = pgetnum("pl")) < 0)
		pagelen = DEFLENGTH;
	if ((formfeed = pgetstr("ff", &bp)) == (char *) 0)
		formfeed = DEFFF;

	emphstart = pgetstr("es", &bp);
	emphstop = pgetstr("ee", &bp);
	initseq = pgetstr("is", &bp);
	termseq = pgetstr("ts", &bp);

	if ((ctllen = pgetnum("cn")) < 0) {
		ctlchars = (char *) 0;
		ctllen = 0;
	} else
		ctlchars = pgetstr("cc", &bp);
	if ((esclen = pgetnum("en")) < 0) {
		escchars = (char *) 0;
		esclen = 0;
	} else
		escchars = pgetstr("ec", &bp);
	lp_char_map(ctlchars, ctllen, escchars, esclen);

	truncline = pgetflag("tl");
	acctfile = pgetstr("af", &bp);
}
