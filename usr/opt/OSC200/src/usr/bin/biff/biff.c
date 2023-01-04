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
static char	*sccsid = "@(#)$RCSfile: biff.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:20:21 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * biff
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

char	*ttyname();
char *progname;

main(argc, argv)
	int argc;
	char **argv;
{
	char *cp = ttyname(2);
	struct stat stb;
	progname = argv[0];

	argc--, argv++;
	if (cp == 0)
		fprintf(stderr, "Where are you?\n"), exit(1);
	if (stat(cp, &stb) < 0)
		perror(cp), exit(1);
	if (argc == 0) {
		printf("is %s\n", stb.st_mode&0100 ? "y" : "n");
		exit((stb.st_mode&0100) ? 0 : 1);
	}
	switch (argv[0][0]) {

	case 'y':
		if (chmod(cp, stb.st_mode|0100) < 0)
			perror(cp);
		break;

	case 'n':
		if (chmod(cp, stb.st_mode&~0100) < 0)
			perror(cp);
		break;

	default:
		fprintf(stderr, "usage: %s [y] [n]\n",progname);
	}
	exit((stb.st_mode&0100) ? 0 : 1);
}
