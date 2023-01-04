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
static char	*sccsid = "@(#)$RCSfile: soelim.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/09/29 13:08:36 $";
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
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
/*
 * soelim - a filter to process n/troff input eliminating .so's
 *
 * Author: Bill Joy UCB July 8, 1977
 *
 * This program eliminates .so's from a n/troff input stream.
 * It can be used to prepare safe input for submission to the
 * phototypesetter since the software supporting the operator
 * doesn't let him do chdir.
 *
 * This is a kludge and the operator should be given the
 * ability to do chdir.
 *
 * This program is more generally useful, it turns out, because
 * the program tbl doesn't understand ".so" directives.
 *
 * soelim.c	5.1 (Berkeley) 5/31/85
 */
#define	STDIN_NAME	"-"

main(argc, argv)
	int argc;
	char *argv[];
{
	int status=0;	/*DAL001*/

	argc--;
	argv++;
	if (argc <= 0) {			/*DAL001*/
		if(process(STDIN_NAME) < 0)	/*DAL001*/
			status = 1;		/*DAL001*/
		exit(status);			/*DAL001*/
	}
	do {
		if(process(argv[0]) < 0)	/*DAL001*/
			status = 1;		/*DAL001*/
		argv++;
		argc--;
	} while (argc > 0);
	exit(status);				/*DAL001*/
}

int process(file)
	char *file;
{
	register char *cp;
	register int c;
	char fname[BUFSIZ];
	FILE *soee;
	int isfile;
	int status=0;		/*DAL001*/

	if (!strcmp(file, STDIN_NAME)) {
		soee = stdin;
	} else {
		soee = fopen(file, "r");
		if (soee == NULL) {
			perror(file);
			return(-1);
		}
	}
	for (;;) {
		c = getc(soee);
		if (c == EOF)
			break;
		if (c != '.')
			goto simple;
		c = getc(soee);
		if (c != 's') {
			putchar('.');
			goto simple;
		}
		c = getc(soee);
		if (c != 'o') {
			printf(".s");
			goto simple;
		}
		do
			c = getc(soee);
		while (c == ' ' || c == '\t');
		cp = fname;
		isfile = 0;
		for (;;) {
			switch (c) {

			case ' ':
			case '\t':
			case '\n':
			case EOF:
				goto donename;

			default:
				*cp++ = c;
				c = getc(soee);
				isfile++;
				continue;
			}
		}
donename:
		if (cp == fname) {
			printf(".so");
			goto simple;
		}
		*cp = 0;
		if (process(fname) < 0)
			if (isfile)
				printf(".so %s\n", fname);
		continue;
simple:
		if (c == EOF)
			break;
		putchar(c);
		if (c != '\n') {
			c = getc(soee);
			goto simple;
		}
	}
	if(ferror(soee))		/*DAL001*/
		status = -1;		/*DAL001*/
	if (soee != stdin) {
		fclose(soee);
	}
	if(ferror(stdout))		/*DAL001*/
		exit(1);		/*DAL001*/
	return(status);			/*DAL001*/
}
