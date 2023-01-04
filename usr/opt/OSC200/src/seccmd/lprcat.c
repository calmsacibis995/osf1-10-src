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
static char	*sccsid = "@(#)$RCSfile: lprcat.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:29:27 $";
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
#if defined(SecureWare) && defined(B1)
/*
 * Copyright (c) 1989 SecureWare, Inc.  All Rights Reserved.
 */



#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <values.h>
#include <errno.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include <protcmd.h>

#ifdef MSG
#include "lprcat_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LPRCAT,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#define	CHAR_SET_SIZE	(1 << BITSPERBYTE)
#define	CHRMAP_SIZE	(WORD_OF_BIT(CHAR_SET_SIZE) + 1)
#define	ESCMAP_SIZE	(WORD_OF_BIT(CHAR_SET_SIZE) + 1)

int	suppress_labeling;
int	suppress_filtering;
int	linelen;
int	pagelen;
int	truncline;

mask_t	chrmap[CHRMAP_SIZE];
mask_t	escmap[ESCMAP_SIZE];

char	*format_label();

struct pr_lp	*prl;

extern char	*optarg;	/* arg pointer for getopt */
extern int	optind;		/* option index for getopt */

extern char	*malloc(), *strchr(), *strrchr();

main(argc, argv)
	int argc;
	char *argv[];
{
	char		*file, *model, *label;
	char		line[BUFSIZ];
	FILE		*in;
	register int	lineno;
	int		ret, option, error = 0;
#if defined(CMW) || defined(SHW)
	ilb_ir_t	*sec_level_ir;
#else
	mand_ir_t	*sec_level_ir;
#endif

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_LPRCAT,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();
#if SEC_MAC
	mand_init();
#endif

	while ((option = getopt(argc, argv, "fl")) != EOF)
		switch (option) {
		case 'f':
			suppress_filtering = 1;
			break;

		case 'l':
			suppress_labeling = 1;
			break;

		case '?':
			error = 1;
			break;
		}

	if (error || argc != optind + 2) {
		fprintf(stderr, MSGSTR(LPRCAT_1, "usage:  %s [-f] [-l] file model\n"),
			command_name);
		fprintf(stderr, MSGSTR(LPRCAT_2, "Where -f suppresses filtering\n"));
		fprintf(stderr, MSGSTR(LPRCAT_3, "      -l suppresses page labeling\n"));
		fprintf(stderr, MSGSTR(LPRCAT_4, "      file is the data file to filter\n"));
		fprintf(stderr, MSGSTR(LPRCAT_5, "      model is the printer model\n"));
		exit(1);
	}

	file = argv[optind];
	model = argv[optind+1];

	/*
	 * Get the characteristics for the printer.  These include
	 * the line length, page length, any special escape/control
	 * sequences, and bold/normal sequences for the label.
	 */

	prl = getprlpnam(model);
	if (prl == (struct pr_lp *) 0) {
		fprintf(stderr, MSGSTR(LPRCAT_6, "%s: %s is not a valid printer model\n"),
			command_name, model);
		exit(1);
	}

	if (!prl->uflg.fg_linelen || !prl->uflg.fg_pagelen) {
		fprintf(stderr,
		    MSGSTR(LPRCAT_7, "%s: descriptor for %s omits page or line length\n"),
		    command_name, model);
		exit(1);
	}

	linelen = prl->ufld.fd_linelen;
	pagelen = prl->ufld.fd_pagelen;

	if (!suppress_filtering || !suppress_labeling) {
		lp_char_map(prl, chrmap, escmap);
		truncline = prl->uflg.fg_truncline && prl->ufld.fd_truncline;
	}

	forcepriv(SEC_ALLOWMACACCESS);
#ifdef ILB
	forcepriv(SEC_ILNOFLOAT);
#endif

	if (!suppress_labeling) {

		/*
		 * Allocate an IR to get the file's label.
		 */
#if defined(CMW) || defined(SHW)
		sec_level_ir = ilb_alloc_ir();
#else
		sec_level_ir = mand_alloc_ir();
#endif
		if (sec_level_ir == 0) {
			fprintf(stderr, MSGSTR(LPRCAT_8, "%s: ran out of space\n"), command_name);
			exit(1);
		}

		/*
		 * Retrieve the label from the appropriate file and format it
		 * for output.  If the label cannot be gotten, either because
		 * it is a WILDCARD or because of an error, assume system high.
		 */

#if defined(CMW) || defined(SHW)
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

	/*
	 * Open the data file to be printed.
	 */
	in = fopen(file, "r");
	if (in == (FILE *) 0)  {
		fprintf(stderr, MSGSTR(LPRCAT_9, "%s: cannot open %s\n"), command_name, file);
		exit(1);
	}

	disablepriv(SEC_ALLOWMACACCESS);

	initialize(stdout);

	lineno = 0;
	line[0] = '\0';

	/*
	 * The main loop.  Each pass outputs a single line.  Large lines
	 * are split (or truncated) and may take multiple passes through
	 * this loop.  Continuation lines are recognized by having a
	 * a non-null character at the start of the line buffer.
	 */

	while (line[0] || fgets(line, sizeof line, in)) {

		if (!suppress_labeling) {

			if (lineno == pagelen - 1) {
				printf("%s\r\f", label);
				lineno = 0;
			}
			if (lineno == 0) {
				printf("%s\n", label);
				lineno++;
			}

			/*
			 * If the printer truncates the line, do so here also
			 * to 1) reduce output, and 2) properly account for
			 * line length in page length calculations below.
			 */
			if (truncline && line[linelen-1] != '\n') {
				line[linelen] = '\n';
				line[linelen + 1] = '\0';
			}
		}

		ret = output(line, stdout);
		lineno = (lineno + 1) % pagelen;

		if (ret && lineno)
			if (suppress_labeling) {
				putchar('\f');
				lineno = 0;
			} else {
				while (lineno < pagelen - 1) {
					putchar('\n');
					++lineno;
				}
			}
	}

	/*
	 * Fill out the final page.
	 */

	if (lineno) {
		if (!suppress_labeling) {
			while (lineno < pagelen-1) {
				putchar('\n');
				++lineno;
			}
			printf("%s\r", label);
		}
		putchar('\f');
	}

	finish(stdout);
	fclose(in);

	exit(0);
}


/*
 * Initialize the printer so that we know the line spacing and other
 * relevant modes.
 */
initialize(file)
	FILE	*file;
{
	if (!suppress_filtering && prl->uflg.fg_initseq)
		fputs(prl->ufld.fd_initseq, file);
}


/*
 * Terminate use of the printer.
 */
finish(file)
	FILE	*file;
{
	if (!suppress_filtering && prl->uflg.fg_termseq)
		fputs(prl->ufld.fd_termseq, file);
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
	register int	count, formfeed, wasescape;

	if (suppress_filtering) {
		cp = strchr(line, '\f');
		if (cp) {
			*cp++ = '\0';
			fputs(line, file);
			fputc('\n', file);
			if (*cp)
				strcpy(line, cp);
			else
				line[0] = '\0';
			return 1;
		}
		fputs(line, file);
		line[0] = '\0';
		return 0;
	}

	cp = line;
	count = formfeed = wasescape = 0;

	/*
	 * Each pass through this loop prints a character.  Some characters
	 * may be filtered here.   If filtered, a <SPACE> is output instead.
	 */
	for (; count < linelen && *cp && *cp != '\n' && !formfeed; ++cp) {

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
			count = 0;
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
			formfeed = 1;
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

	return formfeed;
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
	spaces = (buflen < linelen) ? (linelen - buflen) / 2 : 0;
	buflen += spaces + 1;	/* account for null byte at end of buf */
	if (prl->uflg.fg_emph && prl->uflg.fg_deemph)
		buflen += strlen(prl->ufld.fd_emph) +
			  strlen(prl->ufld.fd_deemph);
	else
		prl->ufld.fd_emph[0] = prl->ufld.fd_deemph[0] = '\0';
	
	buf = malloc(buflen);
	if (buf == (char *) 0) {
		fprintf(stderr, MSGSTR(LPRCAT_8, "%s: ran out of space\n"), command_name);
		exit(1);
	}

	sprintf(buf, "%*s%s%.*s%s", spaces, "", prl->ufld.fd_emph,
		linelen, label, prl->ufld.fd_deemph);

	return buf;
}


/*
 * Determine printable characters.  This handles detection of the start of
 * escape sequences, and control characters that could alter line/page
 * positioning that would throw off or cover the security level stamp.
 */
lp_char_map(prl, chrmap, escmap)
	struct pr_lp	*prl;
	mask_t		*chrmap;
	mask_t		*escmap;
{
	register int	scan;
	int		chrlen;
	int		esclen;

	/*
	 * By default, all characters are allowed unless the strings are
	 * not in the database (in which case, no characters are allowed).
	 */
	for (scan = 0; scan < CHAR_SET_SIZE; scan++)  {
		if (prl->uflg.fg_chrs)
			RMBIT(chrmap, scan);
		else
			ADDBIT(chrmap, scan);

		if (prl->uflg.fg_escs)
			RMBIT(escmap, scan);
		else
			ADDBIT(escmap, scan);
	}
 
	/*
	 * Collect characters never to print.
	 */
	if (prl->uflg.fg_chrs)  {
		if (!prl->uflg.fg_chrslen)  {
			fprintf(stderr,
				MSGSTR(LPRCAT_10, "%s: no character string length specified"),
				command_name);
			if (prl->uflg.fg_name)
				fprintf(stderr, MSGSTR(LPRCAT_11, " for %s"), prl->ufld.fd_name);
			fprintf(stderr, "\n");
			exit(1);
		}
		chrlen = prl->ufld.fd_chrslen;
		for (scan = 0; scan < chrlen; scan++)
			ADDBIT(chrmap, prl->ufld.fd_chrs[scan]);
	}

	/*
	 * Collect characters after <ESC> never to print.
	 */
	if (prl->uflg.fg_escs)  {
		if (!prl->uflg.fg_escslen)  {
			fprintf(stderr, MSGSTR(LPRCAT_12, "%s: no escape string length specified"),
				command_name);
			if (prl->uflg.fg_name)
				fprintf(stderr, MSGSTR(LPRCAT_11, " for %s"), prl->ufld.fd_name);
			fprintf(stderr, "\n");
			exit(1);
		}
		esclen = prl->ufld.fd_escslen;
		for (scan = 0; scan < esclen; scan++)
			ADDBIT(escmap, prl->ufld.fd_escs[scan]);
	}
}

#endif
