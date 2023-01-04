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
static char	*sccsid = "@(#)$RCSfile: test06.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:43:36 $";
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
#include <errno.h>

char testpurpose[] = "test no execute access to loader (EACCES)";
char usagestring[] = "[-v] [-q]";

extern char **environ;
extern int optind;

/* let's hope /etc/passwd doesn't have execute access!!! */

#define	LOADER	"/etc/passwd"
#define	COMMAND	"/bin/ls"

extern int errno;

main(argc, argv)
	char *argv[];
{
	while (ltgetopt(argc, argv, "") != EOF)
		;

	ltprintf("calling exec_with_loader(0, \"%s\", \"%s\", 0, 0)\n",
		LOADER, COMMAND);
	if (exec_with_loader(0, LOADER, COMMAND, (char **)0,
	    (char **)0) != -1) {
		lteprintf("exec_with_loader() succeeded and should have failed with EACCES\n");
		ltexit(1);
	}
	if (errno != EACCES) {
		lteprintf("exec_with_loader() failed with %d, instead of %d (EACCES)\n",
			errno, EACCES);
		ltexit(2);
	}
	ltexit(0);
}
