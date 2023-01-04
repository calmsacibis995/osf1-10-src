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
static char	*sccsid = "@(#)$RCSfile: getsetid.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:42:45 $";
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
#include <sys/types.h>
#include <sys/stat.h>

main(argc, argv)
	char *argv[];
{
	struct stat buf;

	if (argc < 2) {
		(void)fprintf(stderr, "usage: %s <file-name>\n", argv[0]);
		exit(1);
	}
	if (stat(argv[1], &buf) == -1) {
		(void)fprintf(stderr, "%s: stat(2) failed on file \"%s\": ",
			argv[0], argv[1]);
		perror((char *)0);
		exit(2);
	}
#ifdef	SETUID
	if (buf.st_mode & S_ISUID)
		(void)printf("%d\n", buf.st_uid);
	else {
		(void)fprintf(stderr, "%s: file \"%s\" is not set uid\n",
			argv[0], argv[1]);
		exit(3);
	}
#else
	if (buf.st_mode & S_ISGID)
		(void)printf("%d\n", buf.st_gid);
	else {
		(void)fprintf(stderr, "%s: file \"%s\" is not set gid\n",
			argv[0], argv[1]);
		exit(3);
	}
#endif
	exit(0);
}
