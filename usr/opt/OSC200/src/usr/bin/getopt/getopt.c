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
static char rcsid[] = "@(#)$RCSfile: getopt.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 17:08:06 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * com/cmd/sh/getopt.c, cmdsh, bos320, 9126320 6/7/91 19:14:52
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<locale.h>

# include	"getopt_msg.h"
nl_catd catd;
# define	MSGSTR(Num, Str)  catgets(catd, MS_GETOPT, Num, Str)
int mb_cur_max;
extern int getopt(), optind;
extern char *optarg;

void check(int);		/* Tests for string overflow and prints error */
#define	STR_LENGTH	5120		/* output string length	*/

/*
 * NAME:	getopt
 *
 * SYNTAX:	getopt legal-args $*
 *
 * FUNCTION:	getopt - parse command line flags and parameters
 *
 * NOTES:	Getopt can be used to break up flags and parameters in
 *		command lines for easy parsing by shell procedures and
 *		to check for valid flags.  "Legal-args" is a string of
 *		recognized flags (see getopt(3)).
 *
 * RETURN VALUE DESCRIPTION:	0 if no errors were found, 2 if command line
 *		errors were found, 1 if non-ascii characters were found (NLS)
 */

void
main(argc, argv)
int argc;
char **argv;
{
	register int	c;
	int	errflg = 0;
	int	rc = 0;
	int	totlen = 0;
	char	tmpstr[4];	/* big enuf for "-c \0"	*/
	char	*goarg;
	char    outstr[STR_LENGTH];

	char *s;

	(void) setlocale (LC_ALL, "");

	mb_cur_max = MB_CUR_MAX;

	catd = catopen(MF_GETOPT, NL_CAT_LOCALE);

	/*
	 * check argument count
	 */
	if (argc < 2) {
		(void) fputs(MSGSTR(USAGE,
			"usage: getopt legal-args $*\n"), stderr);
		exit(2);
	}

        /* Check for any NLS characters in the option string. */
        for (s = argv[1]; *s; s++) {
			rc = mblen(s, mb_cur_max);
	    		if ((rc > 1) || ((rc == 1) && (!isascii((int) *s)))) {
	        		(void) fputs(MSGSTR(NONLSCH,
				"getopt: Only ASCII characters are permitted in option string.\n"),
				stderr);
				exit(1);
	    		}; /* end if */
        }; /* end for */

	*outstr = '\0';         /* initialize the output string */
	goarg = argv[1];
	argv[1] = argv[0];
	argv++;
	argc--;

	/*
	 * process flags
	 */
	while((c=getopt(argc, argv, goarg)) != EOF) {
		if(c=='?') {
			errflg++;
			continue;
		}

		/*
		 * build string based on flag found
		 */
		tmpstr[0] = '-';
		tmpstr[1] = c;
		tmpstr[2] = ' ';
		tmpstr[3] = '\0';

		/*
		 * concatenate to output string
		 */
		totlen +=3;
		check(totlen);
		(void) strcat(outstr, tmpstr);
		
		if(*(strchr(goarg, c)+1) == ':') {
			/*
			 * get optarg for flag
			 */
		        totlen += strlen(optarg) + 1;
			check(totlen);

			(void) strcat(outstr, optarg);
			(void) strcat(outstr, " ");
		}
	}

	/*
	 * exit with status 2 if we found any errors
	 */
	if(errflg) {
		exit(2);
	}

	/*
	 * add "--" to end options
	 */
	totlen +=3;
	check(totlen);
	(void) strcat(outstr, "-- ");

	/*
	 * add rest of arguments
	 */
	while(optind < argc) {
	        totlen+=strlen(argv[optind])+1;
		check(totlen);
		(void) strcat(outstr, argv[optind++]);
		(void) strcat(outstr, " ");
	}

	/*
	 * print it
	 */
	(void) puts(outstr);

	exit(0);
}

void
check(int totlen) {
    if (totlen < STR_LENGTH)
	return;

    fprintf(stderr, MSGSTR(OVERFLOW,"Argument strings too long\n"),STR_LENGTH);
    exit(1);
}
