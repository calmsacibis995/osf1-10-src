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
static char rcsid[] = "@(#)$RCSfile: unexpand.c,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1993/10/11 19:26:24 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: unexpand
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.7  com/cmd/files/unexpand.c, 9123320, bos320 5/27/91 11:50:12
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "expand_msg.h" 

nl_catd catd;
#define MSGSTR(num,str) catgets(catd, MS_EXPAND, num, str) 

#define	MAX_TABS 100
int	nstops = 0;
int	tabstops[MAX_TABS];
int	current_tabstop = 0;

/* Function prototyping. */
void	getstops  ( char * );
void	tabify    ( int );
void	tabify_mb ( int );
void	usage     ( void );

extern int optind;


/*
 * NAME: unexpand [-a]
 *
 * FUNCTION: Replace blanks with tabs. Leave trailing blanks/tabs
 *         at the end of a line.
 *
 * NOTE:   If -a is not used then only leading blanks are replaced
 *	   with tabs in each line.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	register int all=0;
	register char *cp;
	nl_catd catd;
	int	c;
	int	fastpath;

	setlocale(LC_ALL, "");
	catd = catopen(MF_EXPAND, NL_CAT_LOCALE);
	fastpath = MB_CUR_MAX == 1;

	while ((c = getopt(argc, argv, "a?t:")) != -1) {
		switch (c) {
			case 'a':
				all = 1;
				break;
			case 't':
				getstops(optarg);
				break;
			case '?':
			default:
				usage();
				exit(1);
		}
	}
	argv += optind;
	argc -= optind;

	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		if (fastpath)
			tabify(all);
		else
			tabify_mb(all);
	} while (argc > 0);
	exit(0);
}

/*
 * NAME: tabify
 *
 * FUNCTION:  read file, add tabs where needed, write file.
 *	      Process text with ANSI-C single byte functions.
 */

void
tabify(all)
	int all;
{
	register int blanks;
	register size_t column;
	register int c;

	blanks = 0;
	column = 0;

	while ((c = getchar()) != EOF) {
		switch (c) {

		/* Count spaces (blanks), print a tab if the
		 * number of spaces is equal to a tab stop.
		 */
		case ' ':
			blanks++; column++;
			if (nstops == 0) {
				if ((column & 7) != 0)
					continue;
			} else if (nstops == 1) {
				if ((column % tabstops[0]) != 0)
					continue;
			} else {
				/* Current tab stop should be greater than
				 * or equal to column, unless no more stops.
				 */
				while (current_tabstop < nstops &&
				       column > tabstops[current_tabstop]) {
					current_tabstop++;
				}
				if (column != tabstops[current_tabstop])
					continue;
				current_tabstop++;
			}
			if (--blanks > 0) {
				c = '\t';
				blanks = 0;
			}
			break;


		/* For tabs, increment column to the next
		 * tab stop and clear the blanks (spaces) count.
		 */
		case '\t':
			if (nstops == 0) {
				column  = (column + 8) & ~7;
			} else if (nstops == 1) {
				column += tabstops[0] - column % tabstops[0];
			} else {
				if (current_tabstop < nstops &&
				    column < tabstops[current_tabstop])
					column = tabstops[current_tabstop++];
			}
			blanks = 0;
			break;


		/* At the end of a line print any counted spaces.
		 */
		case '\n':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = 0;
			current_tabstop = 0;
			break;


		/* Not space, tab, or end of line.  Print out any
		 * counted spaces.  If no options specified then print
		 * out the rest of the line.  Otherwise increment the
		 * column count and continue processing this line.
		 */
		default:
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			if (all || nstops != 0)
				column++;
			else {
				do {
					putchar(c);
				} while ((c = getchar()) != '\n' && c != EOF);
				if (c == EOF)
					return;
				column = 0;
			}
			break;
		}
		putchar(c);
	}
	return;
}

/*
 * NAME: tabify_mb
 *
 * FUNCTION:  read file, add tabs where needed, write file.
 *	      Process text with X/Open multibyte functions.
 */

void
tabify_mb(all)
	int all;
{
	register int blanks;
	register size_t column;
	register wint_t c;

	blanks = 0;
	column = 0;

	while ((c = getwchar()) != WEOF) {
		switch (c) {

		/* Count spaces (blanks), print a tab if the
		 * number of spaces is equal to a tab stop.
		 */
		case ' ':
			blanks++; column++;
			if (nstops == 0) {
				if ((column & 7) != 0)
					continue;
			} else if (nstops == 1) {
				if ((column % tabstops[0]) != 0)
					continue;
			} else {
				/* Current tab stop should be greater than
				 * or equal to column, unless no more stops.
				 */
				while (current_tabstop < nstops &&
				       column > tabstops[current_tabstop]) {
					current_tabstop++;
				}
				if (column != tabstops[current_tabstop])
					continue;
				current_tabstop++;
			}
			if (--blanks > 0) {
				c = '\t';
				blanks = 0;
			}
			break;

		/* For tabs, increment column to the next
		 * tab stop and clear the blanks (spaces) count.
		 */
		case '\t':
			if (nstops == 0) {
				column  = (column + 8) & ~7;
			} else if (nstops == 1) {
				column += tabstops[0] - column % tabstops[0];
			} else {
				if (current_tabstop < nstops &&
				    column < tabstops[current_tabstop])
					column = tabstops[current_tabstop++];
			}
			blanks = 0;
			break;

		/* At the end of a line print any counted spaces.
		 */
		case '\n':
			while (blanks-- > 0)
				putchar(' ');
			blanks = 0;
			column = 0;
			current_tabstop = 0;
			break;

		/* Not space, tab, or end of line.  Print out any
		 * counted spaces.  If no options specified then print
		 * out the rest of the line.  Otherwise increment the
		 * column count and continue processing this line.
		 */
		default:
			while (blanks-- > 0)
				putchar(' ');
			if (all || nstops != 0) {
				column += wcwidth(c);
				blanks = 0;
			} else {
				do {
					putwchar(c);
				} while ((c = getwchar()) != '\n' && c != WEOF);
				if (c == WEOF)
					return;
				blanks = 0;
				column = 0;
			}
			break;
		}
		putwchar(c);
	}
	return;
}


/*
 * NAME: getstops
 *
 * FUNCTION: checks for tabs that are smaller than previous or for
 *	     too many tab stops.
 */
void
getstops(cp)
	register char *cp;
{
	register int i;


	for (;;) {
		i = 0;
		while (*cp >= '0' && *cp <= '9')
			i = i * 10 + *cp++ - '0';
		if (i <= 0 || (nstops > 0 && i <= tabstops[nstops-1])) {
			fprintf(stderr,MSGSTR(TABORDER,
			 "%s: Specify tabs in increasing order.\n"),"unexpand");
			usage();
			exit(1);
		} else if (nstops >= MAX_TABS) {
			fprintf(stderr,MSGSTR(TABOVERFLOW,
			 "%s: Too many tabs.\n"),"unexpand");
			usage();
			exit(1);
		}
		tabstops[nstops++] = i;
		if (*cp == 0)
			break;
		if (*cp != ',' && *cp != ' ') {
			fprintf(stderr,MSGSTR(TABSEP,
			 "%s: Tabs must be separated with \",\" or \" \".\n"),"unexpand");
			usage();
			exit(1);
		}
		cp++;
	}
	return;
}

/* Convenience routine since usage is printed
 * from more than one place.
 */
void
usage( void )
{
    fprintf(stderr, MSGSTR(USAGE,
"usage: unexpand [-a | -t TabList] [File...]\n"));
}
