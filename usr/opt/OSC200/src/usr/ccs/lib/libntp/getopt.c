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
static char     *sccsid = "@(#)$RCSfile: getopt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:40:31 $";
#endif
/*
 */

/*
 * getopt - get option letter from argv
 *
 * This is a version of the public domain getopt() implementation by
 * Henry Spencer, changed for 4.3BSD compatibility (in addition to System V).
 * It allows rescanning of an option list by setting optind to 0 before
 * calling.  Thanks to Dennis Ferguson for the appropriate modifications.
 *
 * This file is in the Public Domain.
 */

/*LINTLIBRARY*/

#include <stdio.h>

#ifdef	lint
#undef	putc
#define	putc	fputc
#endif	/* lint */

char	*optarg;	/* Global argument pointer. */
int	optind = 0;	/* Global argv index. */

/*
 * N.B. use following at own risk
 */
int	opterr = 1;	/* for compatibility, should error be printed? */
int	optopt;		/* for compatibility, option character checked */

static char	*scan = NULL;	/* Private scan pointer. */

/*
 * Print message about a bad option.  Watch this definition, it's
 * not a single statement.
 */
#define	BADOPT(mess, ch)	if (opterr) { \
					fputs(argv[0], stderr); \
					fputs(mess, stderr); \
					(void) putc(ch, stderr); \
					(void) putc('\n', stderr); \
				} \
				return('?')

int
getopt(argc, argv, optstring)
	int argc;
	char *argv[];
	char *optstring;
{
	register char c;
	register char *place;

	optarg = NULL;

	if (optind == 0) {
		scan = NULL;
		optind++;
	}
	
	if (scan == NULL || *scan == '\0') {
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return EOF;
		if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
			optind++;
			return EOF;
		}
	
		scan = argv[optind]+1;
		optind++;
	}

	c = *scan++;
	optopt = c & 0377;
	for (place = optstring; place != NULL && *place != '\0'; ++place)
		if (*place == c)
			break;

	if (place == NULL || *place == '\0' || c == ':' || c == '?') {
		BADOPT(": unknown option -", c);
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else if (optind >= argc) {
			BADOPT(": option requires argument -", c);
		} else {
			optarg = argv[optind];
			optind++;
		}
	}

	return c&0377;
}
