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
static char	*sccsid = "@(#)$RCSfile: test18.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:44:10 $";
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

char testpurpose[] = "test bad pointer to arguments (EFAULT)";
char usagestring[] = "[-v] [-q]";

extern char **environ;
extern int optind;

#define	LOADER		"failure_loader"
#define	COMMAND		"/bin/ls"
#define	BAD_POINTER	-1

extern int errno;

main(argc, argv)
	char *argv[];
{
	int flags = 0;

	while (ltgetopt(argc, argv, "") != EOF)
		;

	ltprintf("calling exec_with_loader(0, \"%s\", \"%s\", %d, 0)\n",
		LOADER, COMMAND, BAD_POINTER);
	if (exec_with_loader(0, LOADER, (char *)COMMAND,
	    (char **)BAD_POINTER, (char **)0) != -1) {
		lteprintf("exec_with_loader() succeeded and should have failed with EFAULT\n");
		ltexit(1);
	}
	if (errno != EFAULT) {
		lteprintf("exec_with_loader() failed with %d, instead of %d (EFAULT)\n",
			errno, EFAULT);
		ltexit(2);
	}
	ltexit(0);
}
