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
static char rcsid[] = "@(#)$RCSfile: head.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 17:08:35 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
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
 * 1.10  com/cmd/scan/head.c, cmdscan, bos320, 9146320d 11/14/91 13:01:20
 */

/*
 *                                                                    
 * FUNCTION:	Displays the first "count" of lines of each of the 
 *		specified files, or of the standard input.  If "count"
 *		is omitted, it defaults to 10.
 *		
 *		Valid usages:
 *			 head [-count] [file...]
 *			 head [-n lines] [-c bytes] [file...]
 *
 *		Note: The -n lines is synonomous to -count, the -n
 *			is a new flag defined by Posix 1003.2.
 *
 */  


#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include        <nl_types.h>
#include        "head_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_HEAD,num,str)  /*MSG*/

nl_catd         catd;

static void copyout();
static int  getnum();
static void  bytesout();
static void usage();

main(argc, argv)
int argc;
char *argv[];
{
	int linecnt = 10;	/* Default number of file lines to print */
	int numfiles = --argc;  /* Number of files to be printed         */
	char *name;             /* Name of file to be printed            */
	int morethanone = 0;    /* Flag to indicate more than one file   */
	int cflag = 0;		/* output in bytes			 */
	int endopt = 0;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_HEAD, NL_CAT_LOCALE);
	argv++;
	do {
		/* retrieve "count" if present */
		while (argc > 0 && argv[0][0] == '-' ) {
			switch (argv[0][1]) {

			case '-':
				if (argv[0][2] == '\0') {
				    endopt++;
				    argv++;
				    argc--;
				    numfiles--;
				} else {
				    fprintf(stderr, MSGSTR(BADNUM, 
					    "Badly formed number\n"));
				    usage();
				}
				break;
			case 'n':
				cflag = 0;
				if (argv[0][2] == '\0') {
					linecnt = getnum(argv[1]);
					argc-=2; argv+=2;
					numfiles-=2;
				} else {
					linecnt = getnum(argv[0] + 2);
					argc-=1; argv+=1;
					numfiles-=1;
				}
				break;
			case 'c':
				cflag++;
				if (argv[0][2] == '\0') {
					linecnt = getnum(argv[1]);
					argc-=2; argv+=2;
					numfiles-=2;
				} else {
					linecnt = getnum(argv[0] + 2);
					argc-=1; argv+=1;
					numfiles-=1;
				}
				break;

			default:
				cflag = 0;
				if (isdigit(argv[0][1])) {
				  linecnt = getnum(argv[0] + 1);
				  argc--;
				  argv++;
				  numfiles--;
				  break;
				} else
				  usage();
			}
			break;

			/* 
			 * since we're not using getopt, 
			 * we need to check for "--" delimter
			 */
			if (endopt) 
				break;
		}
		/* If extra -"count" at end, break (i.e. head -20 file1 -10) */
		if (argc == 0 && morethanone)
			break;

		/* If filename present, substitute file for stdin */
		if (argc > 0) {
			close(0);
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			name = argv[0];
			argc--;
			argv++;
		} else /* use stdin */
			name = 0;

		/* If more than one file to be printed, 
		   put space between files and print name  */
		if (morethanone)
			putchar('\n');
		morethanone++;
		if (numfiles > 1 && name)
			printf("==> %s <==\n", name);

		/* Print out specified number of lines */
		if (cflag)
			bytesout(linecnt);
		else
			copyout(linecnt);
		fflush(stdout);
	} while (argc > 0);
}

/* Print "count" lines of file */
static void
copyout(cnt)
int cnt;
{
	char lbuf[BUFSIZ];

	while (cnt > 0 && fgets(lbuf,(int)sizeof(lbuf), stdin) != 0) {
		fwrite(lbuf,(int)sizeof(char),strlen(lbuf),stdout);
		cnt--;
	}
	fflush(stdout);
}

static void
bytesout(cnt)
int cnt;
{
	int buf;
	while (cnt > 0 && (buf = getc(stdin)) != EOF ) {
		putchar(buf);
		cnt--;
	}
	fflush(stdout);
	if (buf != '\n')
		printf("\n");
}

/* Retrieve from command line number of lines of each file to be printed */
/* Return value: number of lines.   Like atoi but does error checking.   */
static int
getnum(cp)
char *cp;
{
	int i;

	errno = 0;
	i = strtol(cp, &cp, 0);

	if ( !cp || *cp || errno || (i<0) ) {
	  	/*
		 * If the string was anything but a numeric sequence, or
		 * there was any conversion error, or the string was negative
		 */
		fprintf(stderr, MSGSTR(BADNUM, "Badly formed number\n"));
		usage();
	}
	return (i);
}

void
usage()
{
	fprintf(stderr, MSGSTR(USAGE,  "usage: head [-count] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGEP,  "usage: head [-n lines] [-c bytes] [file ...]\n"));
	exit(1);
}
