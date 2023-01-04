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
static char	*sccsid = "@(#)$RCSfile: ltgetopt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:00 $";
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

#include <stdio.h>

#define OPTARGSIZE 100

int ltpid;
char *ltprogramname;
int ltvflag, ltqflag;

ltgetopt(argc, argv, opts)
	int argc;
	char **argv;
	char *opts;
{
	char optargs[OPTARGSIZE];
	extern int optind, opterr;	/* should be in the test program */
	extern char *optarg;
	extern char testpurpose[];
	char c;

	setbuf(stdout,NULL);		/* force buffering to be turned OFF */

	strcpy(optargs, opts);
	strcat(optargs,"vq");
	if (ltpid == 0) {
		ltpid = getpid();
		ltprogramname = argv[0];
		opterr = 0;
	}
	while (1) {
		c = getopt(argc, argv, optargs);
		if (c == EOF) {
			ltprintf("%s\n",testpurpose);
			return(EOF);
		}
		if (c == 'v' && ltqflag || c == 'q' && ltvflag) {
			lteprintf("-q and -v conflict\n");
			ltprintusage();
		}

		if (c == 'v')
			ltvflag++;
		else if (c == 'q')
			ltqflag++;
		else if (c == '?')
			ltprintusage();
		else return (c);
	}
}
