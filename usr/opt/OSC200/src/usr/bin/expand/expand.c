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
static char rcsid[] = "@(#)$RCSfile: expand.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 16:46:54 $";
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
 * FUNCTIONS: expand
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.8  com/cmd/files/expand.c, 9123320, bos320 5/28/91 15:58:25";
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
#include <string.h>
#include <locale.h>
#include "expand_msg.h" 

nl_catd catd;
#define MSGSTR(num,str) catgets(catd, MS_EXPAND, num, str) 

#define	MAX_TABS 100
int	nstops = 0;
int	tabstops[MAX_TABS];

/* Function prototyping. */
void	getstops  ( char * );
void	expand    ( void );
void	expand_mb ( void );
void	usage     ( void );


/*
 * NAME: expand [-tabstop] [-tab1,tab2,...,tabn] [file...]
 *
 * FUNCTION: expand tabs to equivalent spaces
 * NOTE:  Default tab is 8.
 *        If -tabstop is given then the width of each tab is tabstop.
 *        If -tab1,tab2,...,tabn is given then tabs are set at those 
 *        specific columns.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	register int fastpath;
	register int i, c;

	setlocale(LC_ALL, "");
	catd = catopen(MF_EXPAND, NL_CAT_LOCALE);
	fastpath = MB_CUR_MAX == 1;

	/* Preprocess the options to convert the obsolete
	 * option format into a getopt acceptable format.
	 */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && isdigit( (int)argv[i][1] ) ) {
			char	*tmp;

			tmp = (char *)malloc(strlen(&argv[i][1]) + 3);
			if (tmp == NULL) {
				perror(argv[0]);
				exit(1);
			}
			(void) strcpy(tmp, "-t");
			(void) strcpy(&tmp[2], &argv[i][1]);
			argv[i] = tmp;
		}
	}

	/* Parse the tabstops option.
	 */
	while ((c = getopt(argc, argv, "t:")) != -1) {
		switch (c) {
		    case 't':
			getstops(optarg);
			break;
		    default:
			usage();
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	/* Open all input files and expand the tabs.
	 */
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		if (fastpath)
			expand();
		else
			expand_mb();
	} while (argc > 0);
	exit(0);
}

/*
 * NAME: expand
 *
 * FUNCTION: Replace tabs with blanks.
 *	     Use ANSI-C single byte functions.
 */

void
expand()
{
	register int column;
	register int c;
	register int n;

	column = 0;
	while ((c = getchar()) != EOF) {
		switch (c) {

		case '\t':
			if (nstops == 0) {
				do {
					putchar(' ');
					column++;
				} while (column & 07);
			} else if (nstops == 1) {
				do {
					putchar(' ');
					column++;
				} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
			} else {
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putchar(' ');
					column++;
					break;
				}
				while (column < tabstops[n]) {
					putchar(' ');
					column++;
				}
			}
			break;

		case '\b':
			if (column)
				column--;
			putchar('\b');
			break;

		default:
			putchar(c);
			column++;
			break;

		case '\n':
			putchar(c);
			column = 0;
			break;
		}
	}
	return;
}

/*
 * NAME: expand_mb
 *
 * FUNCTION: Replace tabs with blanks.
 *	     Use X/Open multibyte functions.
 */

void
expand_mb()
{
	register int column;
	register wint_t c;
	register int n;

	column = 0;
	while ((c = getwchar()) != WEOF) {
		switch (c) {

		case '\t':
			if (nstops == 0) {
				do {
					putchar(' ');
					column++;
				} while (column & 07);
			} else if (nstops == 1) {
				do {
					putchar(' ');
					column++;
				} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
			} else {
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putchar(' ');
					column++;
					break;
				}
				while (column < tabstops[n]) {
					putchar(' ');
					column++;
				}
			}
			break;

		case '\b':
			if (column)
				column--;
			putchar('\b');
			break;

		default:
			putwchar(c);
			column += wcwidth(c);
			break;

		case '\n':
			putchar(c);
			column = 0;
			break;
		}
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
			 "%s: Specify tabs in increasing order.\n"),"expand");
			usage();
			exit(1);
		} else if (nstops >= MAX_TABS) {
			fprintf(stderr,MSGSTR(TABOVERFLOW,
			 "%s: Too many tabs.\n"),"expand");
			usage();
			exit(1);
		}
		tabstops[nstops++] = i;
		if (*cp == 0)
			break;
		if (*cp != ',' && *cp != ' ') {
			fprintf(stderr,MSGSTR(TABSEP,
			 "%s: Tabs must be separated with \",\" or \" \".\n"),"expand");
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
usage()
{
    fprintf(stderr, MSGSTR(EXPUSAGE,
    "usage: expand [-t TabList] [File...]\n"));
}
