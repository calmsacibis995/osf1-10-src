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
static char	*sccsid = "@(#)$RCSfile: execve.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:42:32 $";
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
 * execve - progam to call execve()
 */

#include <stdio.h>

char usagestring[] =
"usage: execve <file> argv[0] argv[1] ... argv[n]\n\
\n\
       file   = file name of the file execute \n\
       argv   = arguments, including argv[0], to be passed to the command\n\
\n\
                environment passed as is to the loader and command; use\n\
                env(1) to override environment\n";

extern char *environ;

int
main(argc, argv)
	char *argv[];
{
	char *file, **nargv;

	if (argc < 2) {
		(void)fprintf(stderr, "%s", usagestring);
		exit(1);
	}

	file = argv[1];
	nargv = &argv[2];

	if (execve(file, nargv, environ) == -1) {
		perror("execve: execve() failed");
		exit(3);
	}
	exit(0);
	return(0);	
}
