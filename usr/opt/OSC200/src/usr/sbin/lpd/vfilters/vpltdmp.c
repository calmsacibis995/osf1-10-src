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
static char	*sccsid = "@(#)$RCSfile: vpltdmp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:09:47 $";
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
 * Copyright (c) 1981 Regents of the University of California.
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
"@(#) Copyright (c) 1981 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 *  reads raster file created by vplot and dumps it onto the
 *  Varian or Versatec plotter.
 *  Input comes from file descriptor 0, output is to file descriptor 1.
 */
#include <stdio.h>
#include <sys/vcmd.h>

#define IN	0
#define OUT	1

static	char *Sid = "@(#)vpltdmp.c	5.3\t6/30/88";

int	plotmd[] = { VPLOT };
int	prtmd[]  = { VPRINT };

char	buf[BUFSIZ];		/* output buffer */

int	lines;			/* number of raster lines printed */
int	varian;			/* 0 for versatec, 1 for varian. */
int	BYTES_PER_LINE;		/* number of bytes per raster line. */
int	PAGE_LINES;		/* number of raster lines per page. */

char	*name, *host, *acctfile;

main(argc, argv)
	int argc;
	char *argv[];
{
	register int n;

	while (--argc) {
		if (**++argv == '-') {
			switch (argv[0][1]) {
			case 'x':
				BYTES_PER_LINE = atoi(&argv[0][2]) / 8;
				varian = BYTES_PER_LINE == 264;
				break;

			case 'y':
				PAGE_LINES = atoi(&argv[0][2]);
				break;

			case 'n':
				argc--;
				name = *++argv;
				break;

			case 'h':
				argc--;
				host = *++argv;
			}
		} else
			acctfile = *argv;
	}

	n = putplot();

	ioctl(OUT, VSETSTATE, prtmd);
	if (varian)
		write(OUT, "\f", 2);
	else
		write(OUT, "\n\n\n\n\n", 6);
	account(name, host, *argv);
	exit(n);
}

putplot()
{
	register char *cp;
	register int bytes, n;

	cp = buf;
	bytes = 0;
	ioctl(OUT, VSETSTATE, plotmd);
	while ((n = read(IN, cp, sizeof(buf))) > 0) {
		if (write(OUT, cp, n) != n)
			return(1);
		bytes += n;
	}
	/*
	 * Make sure we send complete raster lines.
	 */
	if ((n = bytes % BYTES_PER_LINE) > 0) {
		n = BYTES_PER_LINE - n;
		for (cp = &buf[n]; cp > buf; )
			*--cp = 0;
		if (write(OUT, cp, n) != n)
			return(1);
		bytes += n;
	}
	lines += bytes / BYTES_PER_LINE;
	return(0);
}

account(who, from, acctfile)
	char *who, *from, *acctfile;
{
	register FILE *a;

	if (who == NULL || acctfile == NULL)
		return;
	if (access(acctfile, 02) || (a = fopen(acctfile, "a")) == NULL)
		return;
	/*
	 * Varian accounting is done by 8.5 inch pages;
	 * Versatec accounting is by the (12 inch) foot.
	 */
	fprintf(a, "t%6.2f\t", (double)lines / (double)PAGE_LINES);
	if (from != NULL)
		fprintf(a, "%s:", from);
	fprintf(a, "%s\n", who);
	fclose(a);
}
