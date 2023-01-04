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
static char	*sccsid = "@(#)$RCSfile: checkeq.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:21:14 $";
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
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


#include <stdio.h>
FILE	*fin;
int	delim	= '$';

main(argc, argv) char **argv; {

	if (argc <= 1)
		check(stdin);
	else
		while (--argc > 0) {
			if ((fin = fopen(*++argv, "r")) == NULL) {
				perror(*argv);
				exit(1);
			}
			printf("%s:\n", *argv);
			check(fin);
			fclose(fin);
		}
	exit(0);
}

check(f)
FILE	*f;
{
	int start, line, eq, ndel, totdel;
	char in[600], *p;

	start = eq = line = ndel = totdel = 0;
	while (fgets(in, 600, f) != NULL) {
		line++;
		ndel = 0;
		for (p = in; *p; p++)
			if (*p == delim)
				ndel++;
		if (*in=='.' && *(in+1)=='E' && *(in+2)=='Q') {
			if (eq++)
				printf("   Spurious EQ, line %d\n", line);
			if (totdel)
				printf("   EQ in %c%c, line %d\n", delim, delim, line);
		} else if (*in=='.' && *(in+1)=='E' && *(in+2)=='N') {
			if (eq==0)
				printf("   Spurious EN, line %d\n", line);
			else
				eq = 0;
			if (totdel > 0)
				printf("   EN in %c%c, line %d\n", delim, delim, line);
			start = 0;
		} else if (eq && *in=='d' && *(in+1)=='e' && *(in+2)=='l' && *(in+3)=='i' && *(in+4)=='m') {
			for (p=in+5; *p; p++)
				if (*p != ' ') {
					if (*p == 'o' && *(p+1) == 'f')
						delim = 0;
					else
						delim = *p;
					break;
				}
			if (delim == 0)
				printf("   Delim off, line %d\n", line);
			else
				printf("   New delims %c%c, line %d\n", delim, delim, line);
		}
		if (ndel > 0 && eq > 0)
			printf("   %c%c in EQ, line %d\n", delim, delim, line);
		if (ndel == 0)
			continue;
		totdel += ndel;
		if (totdel%2) {
			if (start == 0)
				start = line;
			else {
				printf("   %d line %c%c, lines %d-%d\n", line-start+1, delim, delim, start, line);
				start = line;
			}
		} else {
			if (start > 0) {
				printf("   %d line %c%c, lines %d-%d\n", line-start+1, delim, delim, start, line);
				start = 0;
			}
			totdel = 0;
		}
	}
	if (totdel)
		printf("   Unfinished %c%c\n", delim, delim);
	if (eq)
		printf("   Unfinished EQ\n");
}
