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
static char *sccsid = "@(#)$RCSfile: file.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/29 14:36:31 $";
#endif
/*
 * OSF/1 Release 1.0
 */
/* Derived from the work
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * Modification History: reverse order
 * 25-Sep-1990 - Adrian Thoms (thoms@wessex)
 *	Added -i flag to dump the compiled magic table mtab
 *	in the form of an initialized structure so that the utility
 *	can be recompiled with the magic information built in.
 *
 *	Added -b flag, conditionally compiled which switches to using
 *	builtin magic information in place of the magic file.
 *	This is so that we can use the program to test the new file
 *	guesser for the print system.
 */

/************************************************************************
 *			Modification History				*
 * 001 Richard Hart, Oct. 21, 1987					*
 *     Copied from 4.3 BSD code:					*
 *		file.c 4.12	(Berkeley) 11/17/85			*
 * 002 Richard Hart, Oct. 21, 1987					*
 *     Added named pipes for Sys V support, and other things in the	*
 *     current Ultrix file.c						*
 * 003 Richard Hart, Oct. 22, 1987					*
 *     Added use of /etc/magic, like Sys V filecommand			*
 * 004 Richard Hart, Nov. 5, 1987					*
 *     Now uses sys/exec.h for support of a.out magic numbers		*
 * 005 Richard Hart, Nov. 16, 1987					*
 *     Separated filetype library routine from file command		*
 * 006 Richard Hart, August 17, 1988					*
 *     Switched to parse args with getopt.				*
 * 007 Jon Reeves, November 12, 1988					*
 *     Fixed declaration of optarg					*
 ************************************************************************/

#include <sys/param.h>
#include <stdio.h>
#include "filetype.h"

char *mfile = "/usr/lib/file/magic";

main(argc, argv)
char **argv;
{
	FILE *fl;
	register char *p;
	char ap[MAXPATHLEN + 1], *s;
	char *fname;
	int bflag = 0,
	    cflag = 0,
	    fflag = 0,
	    errflg = 0,
	    iflag = 0;
	extern int optind;
	extern char *optarg;
	char c;


	if (argc < 2) errflg++;

	if (!errflg)
		while((c = getopt(argc, argv, "bcf:im:")) != EOF)
			switch (c) {
			case 'b':
				bflag++;
				break;
			case 'c':
				cflag++;
				break;
			case 'f':
				fflag++;
				fname = optarg;
				break;
			case 'i':
				iflag++;
				break;
			case 'm':
				mfile = optarg;
				break;
			case '?':
				errflg++;
				break;
			default:
				fprintf(stderr, "file: illegal option - %c\n",*s);
				exit(2);
			}
	
	if (errflg) {
		fprintf(stderr, "usage: %s [-c] [-m magic-file] [-f source-file] [file ...]\n", argv[0]);
		exit(3);
	}
#ifdef HAVE_MAGIC
	if (bflag) {
		binary_mkmtab();
	} else
#endif
	mkmtab(1);	/* make the internal table now to catch errors before anything */
			/* else is begun.					       */

	if (cflag) {
		printmtab();
	}
	if (iflag) {
		init_printmtab();
	}
	if (iflag || cflag) exit(0);

	if (fflag) {
		if ((fl = fopen(fname, "r")) == NULL) {
			perror(fname);
			exit(2);
		}
		while ((p = fgets(ap, sizeof ap, fl)) != NULL) {
			int l = strlen(p);
			if (l>0)
				p[l-1] = '\0';
			printf("%s:	", p);
			filetype(p, PRINT);
		}
		exit(1);
	}
	else
		for ( ; optind < argc; optind++) {
			printf("%s:	", argv[optind]);
			filetype(argv[optind], PRINT);
			fflush(stdout);
		}
	exit(0);
}
