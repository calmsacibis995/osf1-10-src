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
static char	*sccsid = "@(#)$RCSfile: mailstats.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/08 16:01:48 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
/*
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

*/
#include <sys/file.h>
#include <locale.h>
#include "sendmail.h"
#include "mailstats.h"
#include "pathnames.h"
#include "mailstats_msg.h"

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct statistics stat;
	register int i;
	int ch, fd;
	char *sfile, *ctime();

	catd = catopen(MF_MAILSTATS,NL_CAT_LOCALE);
	(void) setlocale(LC_ALL,"");

	sfile = _PATH_MAILSTATS;
	while ((ch = getopt(argc, argv, "f:")) != EOF)
		switch((char)ch) {
		case 'f':
			sfile = optarg;
			break;
		case '?':
		default:
			fputs(MSGSTR(USAGE, "usage: mailstats [-f file]\n"), stderr);
			exit(EX_USAGE);
		}
	argc -= optind;
	argv += optind;

	if ((fd = open(sfile, O_RDONLY)) < 0) {
		fputs("mailstats: ", stderr);
		perror(sfile);
		exit(EX_NOINPUT);
	}
	if (read(fd, &stat, sizeof(stat)) != sizeof(stat) ||
	    stat.stat_size != sizeof(stat)) {
		fputs(MSGSTR(SIZE, "mailstats: file size changed.\n"), stderr);
		exit(EX_OSERR);
	}

	printf(MSGSTR(STATS, "Statistics from %s"), ctime(&stat.stat_itime));
	printf(MSGSTR(HEADER, " M msgsfr bytes_from  msgsto   bytes_to\n"));
	for (i = 0; i < MAXMAILERS; i++)
		if (stat.stat_nf[i] || stat.stat_nt[i])
			printf("%2d %6ld %10ldK %6ld %10ldK\n", i,
			    stat.stat_nf[i], stat.stat_bf[i],
			    stat.stat_nt[i], stat.stat_bt[i]);
	exit(0);
}
