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
static char     *sccsid = "@(#)$RCSfile: stdhosts.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/24 13:41:37 $";
#endif
/*
 */


/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

/* 
 * Filter to convert addresses in /etc/hosts file to standard form
 */

main(argc, argv)
	char **argv;
{
	char line[256];
	char adr[256];
	char *any(), *trailer;
	extern char *inet_ntoa();
	extern u_int inet_addr();
	FILE *fp;
	
	if (argc > 1) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			fprintf(stderr, "stdhosts: can't open %s\n", argv[1]);
			exit(1);
		}
	}
	else
		fp = stdin;
	while (fgets(line, sizeof(line), fp)) {
		struct in_addr in;

		if (line[0] == '#')
			continue;
		if ((trailer = any(line, " \t")) == NULL)
			continue;
		sscanf(line, "%s", adr);
		in.s_addr = inet_addr(adr);
		if (-1 == (int)in.s_addr) {
			fprintf(stderr,
				"%s: Warning: malformed line ignored:\n%s",
				argv[0], line);
		} else {
			fputs(inet_ntoa(in), stdout);
			fputs(trailer, stdout);
		}
	}
	exit(0);
	/* NOTREACHED */
}

/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return (NULL);
}
